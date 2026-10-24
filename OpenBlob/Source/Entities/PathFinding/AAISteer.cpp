#include "pch.h"
#include "AAISteer.h"
#include "AAIMoveProbe.h"
#include "AAIWaypoint.h"
#include "APathfindingComponent.h"
#include "Collision/ACollisionObject.h"
#include "Entities/ABaseEntity.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/AGndMotorComponent.h"

#include <Physics/Havok/TPhysicsManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AAISteer ) == 0xC4 );

// The half extents of a blocker (also used by ACharEntity::CanOccupyPosition)
// NOTE: retail calls the blocker's GetCollisionObject() first and ignores the result
static const TVector4 s_vBlockerExtents( 1.0f, 1.0f, 1.0f, 1.0f );

static APathfindingComponent* GetPathfinding( ABaseEntity* a_pEntity )
{
	return TSTATICCAST( APathfindingComponent, a_pEntity->GetPathfinding() );
}

// Returns the entity that owns a_pObject (an entity collision object)
static ABaseEntity* GetOwnerEntity( ACollisionObject* a_pObject )
{
	TObject* pOwner = TSTATICCAST( TObject, a_pObject->GetOwner() );
	TASSERT( ( pOwner == TNULL ) || pOwner->IsA( &TGetClass( ABaseEntity ) ) );

	return TSTATICCAST( ABaseEntity, pOwner );
}

// Returns which side of a_rcDirection a_rcOther is on (AAIMoveSolver::SIDE, 0 if straight)
static TUINT8 GetSide( const TVector4& a_rcDirection, const TVector4& a_rcOther )
{
	TVector4 vCross;
	vCross.CrossProduct( a_rcDirection, a_rcOther );

	if ( vCross.y < 0.0f )
		return AAIMoveSolver::SIDE_Right;

	if ( vCross.y > 0.0f )
		return AAIMoveSolver::SIDE_Left;

	return 0;
}

// $deBlob: FUNCTION 007e86a0
AAISteer::AAISteer( ABaseEntity* a_pEntity )
    : AEntityComponent2( a_pEntity )
    , m_lSuggestions( GetGlobalAllocator(), 4, 4 )
{
	m_pBlocker      = TNULL;
	m_pLastBlocker  = TNULL;
	m_fLastSolution = -10000.0f;
}

// $deBlob: FUNCTION 007e8730
// $deBlob: FUNCTION 007e9660
AAISteer::~AAISteer()
{
}

// $deBlob: FUNCTION 007e87a0
AAIWaypoint* AAISteer::AttemptSteer( TFLOAT a_fDeltaTime, const AIMoveGoal_t& a_rcGoal )
{
	m_oSolver.Reset();
	m_lSuggestions.Clear();

	const TVector4& rcPosition = m_pEntity->GetMatrix().GetTranslation();

	// NOTE: retail normalises the XZ direction to the goal here and never uses it
	TVector4 vDirection;
	vDirection.Substract( a_rcGoal.GetGoal(), rcPosition );
	vDirection.w   = a_rcGoal.GetGoal().w;
	TFLOAT fLength = vDirection.MagnitudeXZ();
	vDirection.x /= fLength;
	vDirection.z /= fLength;

	m_vToGoal.Substract( a_rcGoal.GetGoal(), rcPosition );
	m_vToGoal.w = a_rcGoal.GetGoal().w;

	const TVector4&   rcBBox    = m_pEntity->GetBBox();
	TFLOAT            fDistance = TMath::Max( rcBBox.x, rcBBox.z ) * 1.414f;
	ACollisionObject* pObject   = TSTATICCAST( ACollisionObject, GetPathfinding( m_pEntity )->GetObstruction().pObject );

	m_uiBlockerSide = 0;
	m_pBlocker      = TNULL;

	if ( pObject != TNULL && HASANYFLAG( pObject->GetFlags(), ACollisionObject::FLAG_OwnerType1 | ACollisionObject::FLAG_OwnerType2 ) )
	{
		ABaseEntity* pBlocker = TSTATICCAST( ABaseEntity, pObject->GetOwner() );

		if ( !IsSolveable( pBlocker, a_rcGoal ) )
		{
			ResetComponent();
			return TNULL;
		}

		pBlocker->GetCollisionObject();
		pBlocker->GetCollisionObject();
		fDistance += TMath::Max( s_vBlockerExtents.z, s_vBlockerExtents.x ) * 1.414f;

		TVector4 vToBlocker;
		vToBlocker.Substract( pBlocker->GetMatrix().GetTranslation(), rcPosition );
		vToBlocker.w = pBlocker->GetMatrix().GetTranslation().w;

		m_uiBlockerSide |= GetSide( m_vToGoal, vToBlocker );
		m_pBlocker = pBlocker;
	}

	m_lMovingBlockers.Reset();
	AddMoveConstraints( fDistance );
	AdjustSuggestions();

	TFLOAT fSolution;

	if ( m_oSolver.Solve( m_lSuggestions, fSolution ) )
	{
		TVector4 vMove;
		vMove.Multiply( AAIMoveSolver::YawToDirection( TINT( fSolution ) ), fDistance );

		AIMoveGoal_t oGoal;
		oGoal.vGoal.x = rcPosition.x + vMove.x;
		oGoal.vGoal.y = rcPosition.y + vMove.y;
		oGoal.vGoal.z = rcPosition.z + vMove.z;
		oGoal.vGoal.w = rcPosition.w;
		oGoal.fRadius = 0.0f;

		AAIWaypoint* pWaypoint = AAIWaypoint::GetFreeWaypoint();
		pWaypoint->m_fRadius   = 0.01f;
		pWaypoint->m_pNode     = TNULL;
		pWaypoint->m_vPosition = oGoal.GetGoal();
		pWaypoint->m_uiFlags   = AAIWaypoint::FLAG_NoShortcut | AAIWaypoint::FLAG_Steer;

		m_pLastBlocker  = m_pBlocker;
		m_fLastSolution = fSolution;

		return pWaypoint;
	}

	ResetComponent();
	return TNULL;
}

// $deBlob: FUNCTION 007e8ad0
void AAISteer::ResetComponent()
{
	m_pLastBlocker  = TNULL;
	m_pBlocker      = TNULL;
	m_fLastSolution = -10000.0f;
}

// $deBlob: FUNCTION 007e8af0
void AAISteer::AddMoveConstraints( TFLOAT a_fDistance )
{
	TVector4 vFacing = GetPathfinding( m_pEntity )->GetMotor()->GetDesiredFacing();

	if ( vFacing.MagnitudeSqXZ() > 0.0f )
	{
		GetPathfinding( m_pEntity );
		const TVector4& rcPosition = m_pEntity->GetMatrix().GetTranslation();

		static TFLOAT cfObstructionArc = 30.0f;

		TFLOAT fYawLeft  = TFLOAT( AAIMoveSolver::DirectionToYaw( vFacing ) );
		TFLOAT fYawRight = fYawLeft;
		TFLOAT fWeight   = 1.0f;
		TFLOAT fProbed   = 0.0f;
		TBOOL  bFound    = TFALSE;

		if ( !HASANYFLAG( m_uiBlockerSide, AAIMoveSolver::SIDE_Left ) )
			fWeight = 0.5f;

		// Probe to one side until a free direction is found
		do
		{
			if ( bFound ) break;

			bFound = ProbeYaw( fYawLeft, cfObstructionArc, rcPosition, a_fDistance, fWeight );
			fYawLeft -= 30.0f;
			fProbed += 30.0f;

			if ( fYawLeft < 0.0f )
				fYawLeft += 360.0f;

		} while ( fProbed < 180.0f );

		fWeight = HASANYFLAG( m_uiBlockerSide, AAIMoveSolver::SIDE_Right ) ? 1.0f : 0.5f;
		fProbed = 0.0f;
		bFound  = TFALSE;

		// ... and to the other one
		do
		{
			if ( bFound ) return;

			bFound = ProbeYaw( fYawRight, cfObstructionArc, rcPosition, a_fDistance, fWeight );
			fYawRight += 30.0f;
			fProbed += 30.0f;

			if ( fYawRight >= 360.0f )
				fYawRight -= 360.0f;

		} while ( fProbed < 180.0f );
	}
}

// $deBlob: FUNCTION 007e8cf0
void AAISteer::AddNPCConstraint( TFLOAT a_fYaw, TFLOAT a_fSpan, TFLOAT a_fDistance )
{
	APathfindingComponent::Obstruction& rObstruction = GetPathfinding( m_pEntity )->GetObstruction();
	ACollisionObject*                   pObject      = TSTATICCAST( ACollisionObject, rObstruction.pObject );
	TASSERT( HASANYFLAG( pObject->GetFlags(), ACollisionObject::FLAG_OwnerType1 | ACollisionObject::FLAG_OwnerType2 ) && "obstruction.pCollisionObj->IsEntity()" );

	ABaseEntity* pBlocker = GetOwnerEntity( pObject );

	TVector4 vToBlocker;
	vToBlocker.Substract( pBlocker->GetMatrix().GetTranslation(), m_pEntity->GetMatrix().GetTranslation() );
	vToBlocker.w = pBlocker->GetMatrix().GetTranslation().w;

	AAIMoveSolver::Constraint oConstraint;
	oConstraint.m_fWeight        = 1.0f;
	oConstraint.m_eType          = AAIMoveSolver::CONSTRAINTTYPE_NPC;
	oConstraint.m_oArc.m_fCenter = a_fYaw;
	oConstraint.m_oArc.m_fSpan   = a_fSpan;
	oConstraint.m_uiSide         = GetSide( m_vToGoal, vToBlocker );
	oConstraint.m_fDistance      = a_fDistance;
	m_oSolver.AddConstraint( oConstraint );

	// Remember the ones that move (AdjustSuggestions)
	if ( HASANYFLAG( pObject->GetFlags(), ACollisionObject::FLAG_OwnerType2 ) && m_lMovingBlockers.Size() - 1 < 19 )
		m_lMovingBlockers.Push( pBlocker );
}

// $deBlob: FUNCTION 007e8e60
void AAISteer::AddWorldConstraint( TFLOAT a_fYaw, TFLOAT a_fSpan, TFLOAT a_fDistance )
{
	TUINT8                              uiSide       = 0;
	APathfindingComponent::Obstruction& rObstruction = GetPathfinding( m_pEntity )->GetObstruction();

	if ( !rObstruction.vNormal.IsEqual( TVector4( 0.0f, 0.0f, 0.0f, 1.0f ) ) )
		uiSide = GetSide( rObstruction.vNormal, m_vToGoal );

	AAIMoveSolver::Constraint oConstraint;
	oConstraint.m_fWeight        = 1.0f;
	oConstraint.m_eType          = AAIMoveSolver::CONSTRAINTTYPE_World;
	oConstraint.m_oArc.m_fCenter = a_fYaw;
	oConstraint.m_oArc.m_fSpan   = a_fSpan;
	oConstraint.m_uiSide         = uiSide;
	oConstraint.m_fDistance      = a_fDistance;
	m_oSolver.AddConstraint( oConstraint );
}

// $deBlob: FUNCTION 007e8f00
TBOOL AAISteer::ProbeYaw( TFLOAT a_fYaw, TFLOAT a_fSpan, const TVector4& a_rcPosition, TFLOAT a_fDistance, TFLOAT a_fWeight )
{
	TVector4 vMove;
	vMove.Multiply( AAIMoveSolver::YawToDirection( a_fYaw ), a_fDistance );

	TVector4 vGoal;
	vGoal.x = a_rcPosition.x + vMove.x;
	vGoal.y = a_rcPosition.y + vMove.y;
	vGoal.z = a_rcPosition.z + vMove.z;
	vGoal.w = a_rcPosition.w;

	switch ( GetPathfinding( m_pEntity )->GetMoveProbe()->CheckGroundMove( a_rcPosition, vGoal, 0 ) )
	{
		case 0:
		case AAIMoveProbe::PROBERESULT_Wall:
		case AAIMoveProbe::PROBERESULT_NoGround:
			AddWorldConstraint( a_fYaw, a_fSpan, a_fDistance );
			return TFALSE;
		case AAIMoveProbe::PROBERESULT_PushableObject:
		case AAIMoveProbe::PROBERESULT_Object:
			AddNPCConstraint( a_fYaw, a_fSpan, a_fDistance );
			return TFALSE;
	}

	AAIMoveSolver::Constraint oSuggestion;
	oSuggestion.m_fWeight        = a_fWeight;
	oSuggestion.m_eType          = AAIMoveSolver::CONSTRAINTTYPE_Suggestion;
	oSuggestion.m_oArc.m_fCenter = a_fYaw;
	oSuggestion.m_oArc.m_fSpan   = a_fSpan;
	oSuggestion.m_uiSide         = 0;
	oSuggestion.m_fDistance      = a_fDistance;
	m_lSuggestions.PushBack( oSuggestion );

	return TTRUE;
}

// $deBlob: FUNCTION 007e9090
void AAISteer::AdjustSuggestions()
{
	TBOOL bSameBlocker = TFALSE;

	if ( m_pBlocker != TNULL && m_pBlocker == m_pLastBlocker )
	{
		TASSERT( m_fLastSolution >= 0 && m_fLastSolution <= 360 );
		bSameBlocker = TTRUE;
	}

	TFLOAT fSpeed = GetPathfinding( m_pEntity )->GetMotor()->GetCurrentSpeed();

	for ( TINT i = 0; i < m_lSuggestions.Size(); i++ )
	{
		AAIMoveSolver::Constraint* pSuggestion = &m_lSuggestions[ i ];

		// Don't turn back on the last solution while the same entity blocks
		if ( bSameBlocker && AAIMoveSolver::YawDelta( pSuggestion->m_oArc.m_fCenter, m_fLastSolution ) > 100.0f )
			pSuggestion->m_fWeight *= 0.1f;

		if ( pSuggestion->m_fWeight > 0.0f )
		{
			TVector4 vDirection = AAIMoveSolver::YawToDirection( pSuggestion->m_oArc.m_fCenter );

			// Don't follow what moves the same way
			for ( TINT j = 0; j < m_lMovingBlockers.Size(); j++ )
			{
				TVector4 vVelocity;
				TFLOAT   fBlockerSpeed;

				if ( !m_lMovingBlockers[ j ]->IsFlagged( ABaseEntity::FLAG_Player ) )
				{
					AGndMotorComponent* pMotor = GetPathfinding( m_lMovingBlockers[ j ] )->GetMotor();
					vVelocity                  = pMotor->GetDesiredFacing();
					fBlockerSpeed              = pMotor->GetCurrentSpeed();
				}
				else
				{
					TASSERT( m_lMovingBlockers[ j ]->IsA( &TGetClass( ABlobEntity ) ) );

					TPhysicsManager::GetSingleton()->GetLinearVelocity( TSTATICCAST( ABlobEntity, m_lMovingBlockers[ j ] )->m_pBody, vVelocity );
					fBlockerSpeed = vVelocity.NormaliseXZ();
				}

				if ( fBlockerSpeed > 0.0f )
				{
					TFLOAT fDot = vVelocity.x * vDirection.x + vVelocity.y * vDirection.y + vDirection.z * vVelocity.z;

					if ( fDot <= 0.85f )
					{
						if ( fDot > 0.0f )
							pSuggestion->m_fWeight *= 0.5f;
					}
					else if ( TMath::Abs( fBlockerSpeed - fSpeed ) >= 0.2f )
					{
						pSuggestion->m_fWeight *= 0.2f;
					}
					else
					{
						pSuggestion->m_fWeight *= 0.1f;
					}
				}
			}
		}
	}
}

// $deBlob: FUNCTION 007e93e0
TBOOL AAISteer::IsSolveable( ABaseEntity* a_pBlocker, const AIMoveGoal_t& a_rcGoal )
{
	TFLOAT fX = a_rcGoal.GetGoal().x - a_pBlocker->GetMatrix().GetTranslation().x;
	TFLOAT fZ = a_rcGoal.GetGoal().z - a_pBlocker->GetMatrix().GetTranslation().z;

	a_pBlocker->GetCollisionObject();

	// Solveable when the goal isn't inside both of us
	const TVector4& rcBBox   = GetPathfinding( m_pEntity )->GetEntity()->GetBBox();
	TFLOAT          fMinDist = TMath::Max( rcBBox.x, rcBBox.z ) * 1.414f + TMath::Max( s_vBlockerExtents.z, s_vBlockerExtents.x ) * 1.414f;

	if ( fMinDist * fMinDist <= fZ * fZ + fX * fX )
		return TTRUE;

	return TFALSE;
}
