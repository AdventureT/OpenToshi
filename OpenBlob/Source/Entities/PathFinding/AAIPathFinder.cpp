#include "pch.h"
#include "AAIPathFinder.h"
#include "AAIPath.h"
#include "AAIMoveProbe.h"
#include "AAINetwork.h"
#include "AAINode.h"
#include "AAIWaypoint.h"
#include "APathfindingComponent.h"
#include "Entities/ABaseEntity.h"
#include "Entities/AGndMotorComponent.h"
#include "Collision/ACollisionObject.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include <Physics/Havok/TPhysicsManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL AAIPathFinder::sm_bDebug = TFALSE;

TSTATICASSERT( sizeof( AAIPathFinder ) == 0x54 );

static APathfindingComponent* GetPathfinding( ABaseEntity* a_pEntity )
{
	return TSTATICCAST( APathfindingComponent, a_pEntity->GetPathfinding() );
}

// $deBlob: FUNCTION 007e6cd0
AAIPathFinder::AAIPathFinder( ABaseEntity* a_pEntity )
    : AEntityComponent2( a_pEntity )
{
	m_oStartCache.m_iFilledSize = 0;
	m_oEndCache.m_iFilledSize   = 0;
}

// $deBlob: FUNCTION 007e6d10
TINT AAIPathFinder::PathFindLocal( const TVector4& a_rvStart, const TVector4& a_rvGoal, AAIPath* a_pPath, TFLOAT a_fRadius )
{
	a_pPath->ClearPath();

	if ( TVector4::DistanceSq( a_rvStart, a_rvGoal ) < a_fRadius * a_fRadius )
		return 2;

	APathfindingComponent* pPathfinding = GetPathfinding( m_pEntity );
	TUINT8                 uiFlags      = pPathfinding->GetFlags();
	TBOOL                  bWalk        = uiFlags & APathfindingComponent::FLAG_Walk;
	TBOOL                  bFly         = ( uiFlags >> 3 ) & 1;
	TBOOL                  bJump        = ( uiFlags & APathfindingComponent::FLAG_Jump ) && !( uiFlags & 0x80 );
	TBOOL                  bOnGround    = pPathfinding->GetMotor()->IsOnGround( 0.03f );

	TINT eResult = -1;

	if ( bWalk && bOnGround && ( eResult = PathFindLocal_Imp( a_rvStart, a_rvGoal, a_pPath, a_fRadius, MOVEKIND_Ground ), 0 < eResult ) )
		return eResult;

	if ( bJump && bOnGround && ( eResult = PathFindLocal_Imp( a_rvStart, a_rvGoal, a_pPath, a_fRadius, MOVEKIND_Jump ), 0 < eResult ) )
		return eResult;

	if ( !bFly )
		return eResult;

	// The fly move
	a_pPath->ClearPath();

	TVector4 vStart = a_rvStart;
	TVector4 vGoal  = a_rvGoal;

	// NOTE: retail gets the pathfinding (for the probe) and calls it once more for the flags
	AAIMoveProbe* pMoveProbe = GetPathfinding( m_pEntity )->GetMoveProbe();
	eResult                  = pMoveProbe->CheckFlyMove( vStart, vGoal, GetPathfinding( m_pEntity )->GetCollisionFlags() );

	if ( !IsBlocked( eResult ) )
	{
		AAIWaypoint* pWaypoint = AAIWaypoint::GetFreeWaypoint();
		pWaypoint->m_pNode     = TNULL;
		pWaypoint->m_uiFlags   = AAIWaypoint::FLAG_Fly;
		pWaypoint->m_vPosition = a_rvGoal;
		pWaypoint->m_fRadius   = a_fRadius;
		a_pPath->AddWaypoint( pWaypoint );
		return 1;
	}

	APathfindingComponent* pFlyPathfinding = GetPathfinding( m_pEntity );

	if ( eResult == AAIMoveProbe::PROBERESULT_Clear )
		return 1;

	if ( eResult < 0 || eResult > AAIMoveProbe::PROBERESULT_Clear )
		TASSERT( !"Unknown probe result" );

	if ( GetPathfinding( m_pEntity )->GetFlags() & APathfindingComponent::FLAG_Triangulate )
	{
		// NOTE: Triangulate only works for MOVEKIND_Ground, so this always fails
		TVector4 vMove( vGoal.x - a_rvStart.x, vGoal.y - a_rvStart.y, vGoal.z - a_rvStart.z, vGoal.w );
		TVector4 vApex;

		if ( Triangulate( a_rvStart, a_rvGoal, vApex, vMove.Magnitude(), MOVEKIND_Fly ) )
		{
			AAIWaypoint* pGoal = AAIWaypoint::GetFreeWaypoint();
			AAIWaypoint* pApex = AAIWaypoint::GetFreeWaypoint();
			pGoal->m_fRadius   = 0.01f;
			pGoal->m_vPosition = a_rvGoal;
			pGoal->m_pNode     = TNULL;
			pGoal->m_uiFlags   = 0;
			pApex->m_fRadius   = 0.01f;
			pApex->m_vPosition = vApex;
			pApex->m_pNode     = TNULL;
			pApex->m_uiFlags   = 0;
			a_pPath->AddWaypoint( pApex );
			a_pPath->AddWaypoint( pGoal );
			return 1;
		}
	}

	APathfindingComponent::Obstruction& rObstruction = pFlyPathfinding->GetObstruction();

	if ( rObstruction.pObject != TNULL && ( TSTATICCAST( ACollisionObject, rObstruction.pObject )->GetFlags() & 0x30 ) )
		return -2;

	return -1;
}

// The first waypoint of a path
// $deBlob: FUNCTION 007e84c0
static AAIWaypoint* GetFrontWaypoint( AAIPath* a_pPath )
{
	TASSERT( !a_pPath->m_Path.IsEmpty() );
	return a_pPath->m_Path.Front();
}

// $deBlob: FUNCTION 007e7020
TINT AAIPathFinder::PathFindNetwork( const TVector4& a_rvStart, const TVector4& a_rvGoal, AAIPath* a_pPath, TFLOAT a_fRadius )
{
	TINT     iStartMoveKind;
	AAINode* pStartNode = GetClosestReachableNode( a_rvStart, m_oStartCache, iStartMoveKind, TFALSE, GetPathfinding( m_pEntity )->GetCollisionFlags() );

	if ( pStartNode == TNULL )
		return -1;

	TINT     iEndMoveKind;
	AAINode* pEndNode = GetClosestReachableNode( a_rvGoal, m_oEndCache, iEndMoveKind, TTRUE, GetPathfinding( m_pEntity )->GetCollisionFlags() );

	TASSERT( TNULL == pEndNode || pEndNode->m_uiSegmentID != pStartNode->m_uiSegmentID || PathFindLocal( pEndNode->GetPosition(), a_rvGoal, a_pPath, a_fRadius ) > 0, "Shouldn't be able to get in here because GetClosestReachableNode checks this\nDuring Pathfind from (%f, %f, %f, %f) to (%f, %f, %f, %f)" );

	if ( pEndNode == TNULL || pEndNode->m_uiSegmentID != pStartNode->m_uiSegmentID || PathFindNetwork( pStartNode, pEndNode, a_pPath ) <= 0 )
		return -1;

	AAIWaypoint* pWaypoint = AAIWaypoint::GetFreeWaypoint();
	pWaypoint->m_fRadius   = 0.01f;
	pWaypoint->m_pNode     = TNULL;
	pWaypoint->m_vPosition = a_rvGoal;
	pWaypoint->m_uiFlags   = 0;
	a_pPath->AddWaypoint( pWaypoint );

	// The move from the last node to the goal
	if ( iStartMoveKind == MOVEKIND_Jump )
		pWaypoint->m_uiFlags |= AAIWaypoint::FLAG_Jump;
	else if ( iStartMoveKind == MOVEKIND_Fly )
		pWaypoint->m_uiFlags |= AAIWaypoint::FLAG_Fly;

	// The move from the start to the first node
	if ( iEndMoveKind == MOVEKIND_Jump )
		GetFrontWaypoint( a_pPath )->m_uiFlags |= AAIWaypoint::FLAG_Jump;
	else if ( iEndMoveKind == MOVEKIND_Fly )
		GetFrontWaypoint( a_pPath )->m_uiFlags |= AAIWaypoint::FLAG_Fly;

	return 1;
}

// $deBlob: FUNCTION 007e7210
TINT AAIPathFinder::PathFindNetwork( AAINode* a_pStart, AAINode* a_pEnd, AAIPath* a_pPath )
{
	TINT      iPathLength;
	AAINode** ppPath = AAINetwork::GetSingleton()->FindPath( a_pStart, a_pEnd, m_pEntity, iPathLength, TNULL );

	if ( ppPath == TNULL )
		return -1;

	a_pPath->ClearPath();

	AAIWaypoint* pPrevious = TNULL;

	while ( --iPathLength >= 0 )
	{
		AAINode* pNode = ppPath[ iPathLength ];

		AAIWaypoint* pWaypoint = AAIWaypoint::GetFreeWaypoint();
		pWaypoint->m_fRadius   = 0.01f;
		pWaypoint->m_pNode     = pNode;
		pWaypoint->m_uiFlags   = 0;
		pWaypoint->m_vPosition = pNode->m_vPosition;
		a_pPath->m_Path.PushBack( pWaypoint );

		// The waypoints of a jump link get a wider radius
		if ( pPrevious != TNULL && pPrevious->m_pNode->GetLinkTo( pNode )->m_uiMoveType == AAILink::MOVETYPE_Jump )
		{
			pPrevious->m_fRadius = 0.05f;
			pWaypoint->m_fRadius = 0.05f;
		}

		pPrevious = pWaypoint;
	}

	return 1;
}

// $deBlob: FUNCTION 007e72f0
TBOOL AAIPathFinder::DestinationValid( const TVector4& a_rvGoal, TVector4* a_pOutNodePosition )
{
	TINT     iMoveKind;
	AAINode* pNode = GetClosestReachableNode( a_rvGoal, m_oEndCache, iMoveKind, TTRUE, 2 );

	if ( pNode == TNULL )
		return TFALSE;

	if ( a_pOutNodePosition != TNULL )
		*a_pOutNodePosition = pNode->m_vPosition;

	return TTRUE;
}

// $deBlob: FUNCTION 007e7330
void AAIPathFinder::ResetComponent()
{
}

// $deBlob: FUNCTION 007e7340
TBOOL AAIPathFinder::ToggleDebug( void* a_pCommand )
{
	sm_bDebug = !sm_bDebug;
	return TTRUE;
}

// $deBlob: FUNCTION 007e76c0
AAINode* AAIPathFinder::GetClosestReachableNode( const TVector4& a_rvPosition, AAINodeCache& a_rCache, TINT& a_rMoveKind, TBOOL a_bFromNode, TUINT32 a_uiProbeFlags )
{
	// The nodes around the position (only looked up again when it moved)
	if ( a_rCache.m_iFilledSize == 0 || TVector4::DistanceSq( a_rCache.m_vPosition, a_rvPosition ) > 0.04f )
	{
		AAINetwork::ClosestNodes& rNodes = AAINetwork::GetSingleton()->GetClosestNodes2( a_rvPosition, AAINodeCache::CACHE_SIZE, 10.0f, 0xFF );

		for ( TINT i = 0; i < rNodes.Size(); i++ )
			a_rCache.m_apNodes[ i ] = rNodes.At( i ).pNode;

		a_rCache.SetFilledSize( rNodes.Size() );
		a_rCache.m_vPosition = a_rvPosition;
	}

	AAIMoveProbe* pMoveProbe = GetPathfinding( m_pEntity )->GetMoveProbe();

	// NOTE: retail calls GetPathfinding() once more here and drops the result
	GetPathfinding( m_pEntity );

	TINT     iNumNodes = a_rCache.m_iFilledSize;
	TVector4 vEnd      = a_rvPosition;
	TVector4 vStart    = a_rvPosition;

	TUINT8 uiFlags = GetPathfinding( m_pEntity )->GetFlags();
	TBOOL  bJump   = ( uiFlags & APathfindingComponent::FLAG_Jump ) && !( uiFlags & 0x80 );
	TBOOL  bFly    = ( uiFlags & APathfindingComponent::FLAG_Fly ) != 0;
	TINT   i;

	if ( uiFlags & APathfindingComponent::FLAG_Walk )
	{
		for ( i = 0; i < iNumNodes; i++ )
		{
			if ( !a_rCache.m_apNodes[ i ]->IsFlagged( AAINode::FLAG_Unk1 ) )
				continue;

			TASSERT( TNULL != a_rCache.GetNode( i ) );

			if ( a_bFromNode )
			{
				vStart = a_rCache.m_apNodes[ i ]->GetPosition();
				vEnd   = a_rvPosition;
			}
			else
			{
				vEnd = a_rCache.m_apNodes[ i ]->GetPosition();
				TASSERT( vStart.IsEqual( a_rvPosition ) );
			}

			if ( pMoveProbe->CheckGroundMove( vStart, vEnd, a_uiProbeFlags | 0x10 ) == AAIMoveProbe::PROBERESULT_Clear )
			{
				a_rMoveKind = MOVEKIND_Ground;
				return a_rCache.m_apNodes[ i ];
			}
		}
	}

	if ( bJump )
	{
		vEnd   = a_rvPosition;
		vStart = a_rvPosition;

		for ( i = 0; i < iNumNodes; i++ )
		{
			if ( !a_rCache.m_apNodes[ i ]->IsFlagged( AAINode::FLAG_Unk1 ) )
				continue;

			TASSERT( TNULL != a_rCache.GetNode( i ) );

			if ( a_bFromNode )
			{
				vStart = a_rCache.m_apNodes[ i ]->GetPosition();
				TASSERT( vEnd.IsEqual( a_rvPosition ) );
			}
			else
			{
				vEnd = a_rCache.m_apNodes[ i ]->GetPosition();
				TASSERT( vStart.IsEqual( a_rvPosition ) );
			}

			// NOTE: retail calls GetPathfinding() here and drops the result
			GetPathfinding( m_pEntity );

			if ( pMoveProbe->CheckJumpMove( vStart, vEnd, a_uiProbeFlags ) == AAIMoveProbe::PROBERESULT_Clear )
			{
				a_rMoveKind = MOVEKIND_Jump;
				return a_rCache.m_apNodes[ i ];
			}
		}
	}

	if ( !bFly )
		return TNULL;

	vEnd   = a_rvPosition;
	vStart = a_rvPosition;

	for ( i = 0; i < iNumNodes; i++ )
	{
		if ( !a_rCache.m_apNodes[ i ]->IsFlagged( AAINode::FLAG_Unk1 ) )
			continue;

		TASSERT( TNULL != a_rCache.GetNode( i ) );

		if ( a_bFromNode )
		{
			vStart = a_rCache.m_apNodes[ i ]->GetPosition();
			TASSERT( vEnd.IsEqual( a_rvPosition ) );
		}
		else
		{
			vEnd = a_rCache.m_apNodes[ i ]->GetPosition();
			TASSERT( vStart.IsEqual( a_rvPosition ) );
		}

		if ( pMoveProbe->CheckFlyMove( vStart, vEnd, a_uiProbeFlags ) == AAIMoveProbe::PROBERESULT_Clear )
		{
			a_rMoveKind = MOVEKIND_Fly;
			return a_rCache.m_apNodes[ i ];
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 007e7360
TBOOL AAIPathFinder::Triangulate( const TVector4& a_rvStart, const TVector4& a_rvGoal, TVector4& a_rvOutApex, TFLOAT a_fDistance, TINT a_eMoveKind )
{
	static TCONSTEXPR_VAR TINT APEX_STEPS = 1;

	if ( a_eMoveKind != MOVEKIND_Ground )
		return TFALSE;

	TVector4 vDirection( a_rvGoal.x - a_rvStart.x, a_rvGoal.y - a_rvStart.y, a_rvGoal.z - a_rvStart.z, a_rvGoal.w );
	vDirection.NormaliseXZ();

	TVector4 vSide;
	vSide.CrossProduct( vDirection, TVector4::VEC_NEGY );

	// The point a_fDistance along the line
	TVector4 vAlong;
	vAlong.Multiply( vDirection, a_fDistance );

	TFLOAT fCentreX = a_rvStart.x + vAlong.x;
	TFLOAT fCentreY = a_rvStart.y + vAlong.y;
	TFLOAT fCentreZ = a_rvStart.z + vAlong.z;
	TFLOAT fCentreW = a_rvStart.w;

	// Four box widths to either side of it
	TFLOAT fStep = GetPathfinding( m_pEntity )->GetEntity()->GetBBox().x * 4.0f;

	TVector4 vStep;
	vStep.Multiply( vSide, fStep );

	TVector4 vOffset;
	vOffset.Multiply( vStep, TFLOAT( APEX_STEPS ) );

	TVector4 vApex( fCentreX - vOffset.x, fCentreY - vOffset.y, fCentreZ - vOffset.z, fCentreW );

	TVector4 avApexes[ APEX_STEPS * 2 ];
	TINT     iNumApexes = 0;

	for ( TINT iApex = -APEX_STEPS; iApex <= APEX_STEPS; iApex++ )
	{
		if ( iApex != 0 )
		{
			TASSERT( iNumApexes < APEX_STEPS * 2 );
			avApexes[ iNumApexes++ ] = vApex;
		}

		vStep.Multiply( vSide, fStep );
		vApex.x += vStep.x;
		vApex.y += vStep.y;
		vApex.z += vStep.z;
	}

	AAIMoveProbe* pMoveProbe = GetPathfinding( m_pEntity )->GetMoveProbe();

	for ( TINT i = 0; i < iNumApexes; i++ )
	{
		TASSERT( i < APEX_STEPS * 2 );

		a_rvOutApex = avApexes[ i ];

		TINT eResult = pMoveProbe->CheckGroundMove( a_rvStart, a_rvOutApex, 0x12 );

		if ( eResult >= 0 && eResult < AAIMoveProbe::PROBERESULT_Clear )
			continue;

		if ( eResult == AAIMoveProbe::PROBERESULT_Clear )
		{
			TVector4 vGoal = a_rvGoal;
			eResult        = pMoveProbe->CheckGroundMove( a_rvOutApex, vGoal, 0x12 );

			if ( eResult >= 0 && eResult < AAIMoveProbe::PROBERESULT_Clear )
				continue;

			if ( eResult == AAIMoveProbe::PROBERESULT_Clear )
				return TTRUE;
		}

		TASSERT( !"Unknown probe result" );
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007e7660
TBOOL AAIPathFinder::IsBlocked( TINT a_eProbeResult )
{
	if ( a_eProbeResult >= 0 )
	{
		if ( a_eProbeResult < AAIMoveProbe::PROBERESULT_Clear )
			return TTRUE;

		if ( a_eProbeResult == AAIMoveProbe::PROBERESULT_Clear )
			return TFALSE;
	}

	TASSERT( !"Unknown probe result" );
	return TTRUE;
}

// $deBlob: FUNCTION 007e7e80
TINT AAIPathFinder::PathFindLocal_Imp( const TVector4& a_rvStart, const TVector4& a_rvGoal, AAIPath* a_pPath, TFLOAT a_fRadius, MOVEKIND a_eMoveKind )
{
	a_pPath->ClearPath();

	TINT     eResult = 3;
	TVector4 vStart  = a_rvStart;
	TVector4 vGoal   = a_rvGoal;

	APathfindingComponent* pPathfinding = GetPathfinding( m_pEntity );
	AAIMoveProbe*          pMoveProbe   = pPathfinding->GetMoveProbe();
	TBOOL                  bAddGround   = TFALSE;

	switch ( a_eMoveKind )
	{
		case MOVEKIND_Ground:
			eResult = pMoveProbe->CheckGroundMove( vStart, vGoal, pPathfinding->GetCollisionFlags() | 0x10 );

			if ( !IsBlocked( eResult ) )
				bAddGround = TTRUE;

			break;
		case MOVEKIND_Jump:
		{
			GetPathfinding( m_pEntity );

			eResult = pMoveProbe->CheckJumpMove( vStart, vGoal, GetPathfinding( m_pEntity )->GetCollisionFlags() );

			if ( !IsBlocked( eResult ) )
			{
				AAIWaypoint* pWaypoint = AAIWaypoint::GetFreeWaypoint();
				pWaypoint->m_vPosition = vGoal;
				pWaypoint->m_pNode     = TNULL;
				pWaypoint->m_fRadius   = a_fRadius;
				pWaypoint->m_uiFlags   = AAIWaypoint::FLAG_Jump;
				a_pPath->AddWaypoint( pWaypoint );
				return 1;
			}

			break;
		}
		case MOVEKIND_Fly:
		{
			eResult = pMoveProbe->CheckFlyMove( vStart, vGoal, GetPathfinding( m_pEntity )->GetCollisionFlags() );

			if ( !IsBlocked( eResult ) )
			{
				AAIWaypoint* pWaypoint = AAIWaypoint::GetFreeWaypoint();
				pWaypoint->m_pNode     = TNULL;
				pWaypoint->m_uiFlags   = AAIWaypoint::FLAG_Fly;
				pWaypoint->m_vPosition = a_rvGoal;
				pWaypoint->m_fRadius   = a_fRadius;
				a_pPath->AddWaypoint( pWaypoint );
				return 1;
			}

			break;
		}
		case MOVEKIND_Climb:
		{
			eResult = pMoveProbe->CheckClimbMove( vStart, vGoal, GetPathfinding( m_pEntity )->GetCollisionFlags() );

			if ( !IsBlocked( eResult ) )
			{
				AAIWaypoint* pWaypoint = AAIWaypoint::GetFreeWaypoint();
				pWaypoint->m_pNode     = TNULL;
				pWaypoint->m_uiFlags   = AAIWaypoint::FLAG_Climb;
				pWaypoint->m_vPosition = a_rvGoal;
				pWaypoint->m_fRadius   = a_fRadius;
				a_pPath->AddWaypoint( pWaypoint );
				return 1;
			}

			break;
		}
	}

	if ( !bAddGround )
	{
		APathfindingComponent::Obstruction& rObstruction = pPathfinding->GetObstruction();

		if ( eResult < 0 || eResult > AAIMoveProbe::PROBERESULT_Clear )
		{
			TASSERT( !"Unknown probe result" );
		}
		else if ( eResult == AAIMoveProbe::PROBERESULT_Clear )
		{
			return 1;
		}

		if ( a_eMoveKind == MOVEKIND_Ground && rObstruction.pObject != TNULL && ( TSTATICCAST( ACollisionObject, rObstruction.pObject )->GetFlags() & 0x20 ) )
		{
			// A pushable object: a short ground move without it (the blob can be pushed out of the way)
			if ( 4.0f < ( vGoal.z - vStart.z ) * ( vGoal.z - vStart.z ) + ( vGoal.x - vStart.x ) * ( vGoal.x - vStart.x ) )
			{
				eResult = pMoveProbe->CheckGroundMove( vStart, vGoal, 0x12 );
			}
			else
			{
				// Moving along with the obstruction (same direction, not slower): the ground move to the goal itself
				ABaseEntity*        pOther = TSTATICCAST( ABaseEntity, TSTATICCAST( ACollisionObject, rObstruction.pObject )->GetOwner() );
				AGndMotorComponent* pMotor = GetPathfinding( m_pEntity )->GetMotor();
				TVector4            vOtherVelocity;
				TFLOAT              fOtherSpeed;

				if ( !pOther->IsFlagged( ABaseEntity::FLAG_Player ) )
				{
					AGndMotorComponent* pOtherMotor = GetPathfinding( pOther )->GetMotor();
					vOtherVelocity                  = pOtherMotor->GetDesiredFacing();
					fOtherSpeed                     = pOtherMotor->GetCurrentSpeed();
				}
				else
				{
					TASSERT( pOther->IsA( &TGetClass( ABlobEntity ) ) );
					TPhysicsManager::GetSingleton()->GetLinearVelocity( TSTATICCAST( ABlobEntity, pOther )->GetBody(), vOtherVelocity );
					fOtherSpeed = vOtherVelocity.NormaliseXZ();
				}

				const TVector4& rFacing = pMotor->GetDesiredFacing();
				TFLOAT          fSpeed  = pMotor->GetCurrentSpeed();

				if ( 0.0f < rFacing.y * vOtherVelocity.y + rFacing.x * vOtherVelocity.x + rFacing.z * vOtherVelocity.z && 0.1f < fSpeed &&
				     ( TMath::Abs( fSpeed - fOtherSpeed ) < 0.2f || fSpeed < fOtherSpeed ) )
				{
					vGoal   = a_rvGoal;
					eResult = pMoveProbe->CheckGroundMove( vStart, vGoal, 0x12 );
				}
			}

			if ( !IsBlocked( eResult ) )
				bAddGround = TTRUE;
		}
	}

	if ( bAddGround )
	{
		AAIWaypoint* pWaypoint = AAIWaypoint::GetFreeWaypoint();
		pWaypoint->m_vPosition = vGoal;
		pWaypoint->m_fRadius   = a_fRadius;
		pWaypoint->m_pNode     = TNULL;
		pWaypoint->m_uiFlags   = 0;
		a_pPath->AddWaypoint( pWaypoint );
		return 1;
	}

	if ( eResult >= 0 && eResult == AAIMoveProbe::PROBERESULT_Clear )
		return 1;

	if ( eResult < 0 || eResult > AAIMoveProbe::PROBERESULT_Clear )
	{
		TASSERT( !"Unknown probe result" );
	}

	if ( pPathfinding->GetFlags() & APathfindingComponent::FLAG_Triangulate )
	{
		// Around the obstruction
		TVector4 vMove( vGoal.x - a_rvStart.x, vGoal.y - a_rvStart.y, vGoal.z - a_rvStart.z, vGoal.w );
		TVector4 vApex;

		if ( Triangulate( a_rvStart, a_rvGoal, vApex, vMove.Magnitude(), a_eMoveKind ) )
		{
			AAIWaypoint* pGoal = AAIWaypoint::GetFreeWaypoint();
			AAIWaypoint* pApex = AAIWaypoint::GetFreeWaypoint();
			pGoal->m_fRadius   = 0.01f;
			pGoal->m_vPosition = a_rvGoal;
			pGoal->m_pNode     = TNULL;
			pGoal->m_uiFlags   = 0;
			pApex->m_fRadius   = 0.01f;
			pApex->m_vPosition = vApex;
			pApex->m_pNode     = TNULL;
			pApex->m_uiFlags   = 0;
			a_pPath->AddWaypoint( pApex );
			a_pPath->AddWaypoint( pGoal );
			return 1;
		}
	}

	APathfindingComponent::Obstruction& rObstruction = pPathfinding->GetObstruction();

	if ( rObstruction.pObject != TNULL && ( TSTATICCAST( ACollisionObject, rObstruction.pObject )->GetFlags() & 0x30 ) )
		return -2;

	return -1;
}

// $deBlob: FUNCTION 007e8420
// $deBlob: FUNCTION 007e6d00
AAIPathFinder::~AAIPathFinder()
{
}
