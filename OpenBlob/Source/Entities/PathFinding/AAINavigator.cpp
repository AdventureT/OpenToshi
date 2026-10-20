#include "pch.h"
#include "AAINavigator.h"
#include "Console/AStaticCVars.h"
#include "AAINode.h"
#include "AAIPath.h"
#include "AAIPathFinder.h"
#include "ABuildPathJobQueue.h"
#include "AAISteer.h"
#include "AAIWaypoint.h"
#include "APathfindingComponent.h"
#include "AApplication.h"
#include "Console/AConsoleVar.h"
#include "Entities/ABaseEntity.h"
#include "Entities/AGndMotorComponent.h"
#include "Collision/ACollisionManager2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TVector4 AIMoveGoal_t::scm_vInvalidGoal = TVector4( 0.0f, -10000.0f, 0.0f, -10000.0f );

TINT   AAINavigator::sm_iNumOptimisedPaths;
TFLOAT AAINavigator::sm_fPredictTime;

static APathfindingComponent* GetPathfinding( ABaseEntity* a_pEntity )
{
	return TSTATICCAST( APathfindingComponent, a_pEntity->GetPathfinding() );
}

// $deBlob: FUNCTION 007d2af0
AAINavigator::AAINavigator( ABaseEntity* a_pEntity )
    : AEntityComponent2( a_pEntity )
{
	m_eState             = STATE_None;
	m_pCurrentWaypoint   = TNULL;
	m_pCurrentLink       = TNULL;
	m_eResult            = RESULT_None;
	m_pPathFinder        = TNULL;
	m_pMotor             = TNULL;
	m_iUnk84             = 0;
	m_fSteerEndTime      = 0.0f;
	m_uiFlags            = 0;
	m_vLastPosition      = TVector4::VEC_ZERO;
	m_fStuckTime         = 0.0f;
	m_iCurrentPath       = 0;
	m_apPaths[ 0 ]       = new AAIPath( a_pEntity );
	m_apPaths[ 1 ]       = new AAIPath( a_pEntity );
	m_aiPathInvalid[ 0 ] = 1;
	m_aiPathInvalid[ 1 ] = 1;

	if ( sm_fPredictTime < TMath::TFLOAT_EPSILON )
		sm_fPredictTime = 1.0f / 30.0f;
}

// $deBlob: FUNCTION 007d4210
// $deBlob: FUNCTION 007d2c40
AAINavigator::~AAINavigator()
{
	if ( m_apPaths[ 0 ] != TNULL )
		delete m_apPaths[ 0 ];

	if ( m_apPaths[ 1 ] != TNULL )
		delete m_apPaths[ 1 ];
}

// $deBlob: FUNCTION 007d2cc0
void AAINavigator::ResetComponent()
{
	m_pPathFinder = GetPathfinding( m_pEntity )->GetPathFinder();
	m_pMotor      = GetPathfinding( m_pEntity )->GetMotor();

	m_apPaths[ 0 ]->ResetComponent();
	m_apPaths[ 1 ]->ResetComponent();

	m_CurrentGoal.Reset();
	m_uiFlags &= ~1;
	m_eState                          = STATE_None;
	m_pCurrentWaypoint                = TNULL;
	m_pCurrentLink                    = TNULL;
	m_eResult                         = RESULT_None;
	m_fSteerEndTime                   = 0.0f;
	m_fStuckTime                      = 0.0f;
	m_aiPathInvalid[ m_iCurrentPath ] = 1;

	m_pMotor->StopImmediate();
}

// $deBlob: FUNCTION 007d2d60
void AAINavigator::Tick( TFLOAT a_fDeltaTime )
{
	const TVector4& rcPosition = m_pEntity->GetMatrix().GetTranslation();

	if ( m_eState == STATE_Moving )
	{
		TASSERT( !( m_CurrentGoal.GetGoal() == AIMoveGoal_t::scm_vInvalidGoal ) );

		// The goal moves with the ground under the owner
		if ( m_pMotor->GetState() == 2 && m_CurrentGoal.eMoveType == 0 )
		{
			TVector4 vMove;
			vMove.Multiply( m_pMotor->GetGroundVelocity(), a_fDeltaTime );
			m_CurrentGoal.vGoal.x += vMove.x;
			m_CurrentGoal.vGoal.y += vMove.y;
			m_CurrentGoal.vGoal.z += vMove.z;
		}


		CheckProgress();

		if ( m_eState != STATE_Moving )
			return;

		TINT eMoveResult = m_pMotor->MoveToGoal( m_CurrentGoal, a_fDeltaTime, m_CurrentGoal.eMoveType );

		// Stuck in place for 1.5 seconds: give up
		m_fStuckTime += a_fDeltaTime;

		if ( TVector4::DistanceSq( rcPosition, m_vLastPosition ) > 0.0001f )
			m_fStuckTime = 0.0f;

		if ( m_fStuckTime >= 1.5f )
		{
			m_eResult = RESULT_Failed;
			m_eState  = STATE_Finished;
			m_pMotor->StopImmediate();
			m_fStuckTime                      = 0.0f;
			m_aiPathInvalid[ m_iCurrentPath ] = 1;
			return;
		}

		TBOOL bFailed = TFALSE;

		switch ( eMoveResult )
		{
			case 0:
			case 1:
				bFailed = TTRUE;
				break;
			case 2:
				bFailed = m_CurrentGoal.eMoveType == 1 || !AttemptSteer( a_fDeltaTime );
				break;
			case 3:
				break;
			default:
				TASSERT( !"Unhandled move result" );
				break;
		}

		if ( bFailed )
		{
			// The link of the network can't be used for a while
			if ( m_pCurrentLink != TNULL )
				m_pCurrentLink->SetStale( 4.0f );

			m_eResult = RESULT_Failed;
			m_eState  = STATE_Finished;
			m_pMotor->StopImmediate();
			m_aiPathInvalid[ m_iCurrentPath ] = 1;
		}
	}

	m_vLastPosition = rcPosition;
}

// $deBlob: FUNCTION 007d3020
void AAINavigator::ClearGoal( TBOOL a_bStopMotor )
{
	m_CurrentGoal.Reset();
	m_uiFlags &= ~1;
	m_eState                          = STATE_None;
	m_pCurrentWaypoint                = TNULL;
	m_pCurrentLink                    = TNULL;
	m_eResult                         = RESULT_None;
	m_fSteerEndTime                   = 0.0f;
	m_fStuckTime                      = 0.0f;
	m_aiPathInvalid[ m_iCurrentPath ] = 1;

	if ( a_bStopMotor )
		m_pMotor->StopImmediate();
}

// $deBlob: FUNCTION 007d3090
TBOOL AAINavigator::GetEndGoal( TVector4& a_rvOutGoal )
{
	if ( m_eState == STATE_Moving || m_eState == STATE_Finished )
	{
		AAIPath* pPath = CurrentPath();

		if ( !pPath->m_Path.IsEmpty() )
		{
			// NOTE: retail reads a TNULL waypoint's position if the tail is TNULL
			a_rvOutGoal = pPath->m_Path.Back()->m_vPosition;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007d3130
TINT AAINavigator::GetNumWaypointsLeft() const
{
	AAIWaypoint* pWaypoint = m_pCurrentWaypoint;

	if ( pWaypoint == TNULL )
		return 0;

	AAIWaypoint* pLast = CurrentPath()->m_Path.Back();
	TINT         iNum  = 1;

	while ( pWaypoint != pLast )
	{
		pWaypoint = TSTATICCAST( AAIWaypoint, pWaypoint->Next() );
		iNum++;
	}

	return iNum;
}

// $deBlob: FUNCTION 007d31b0
TBOOL AAINavigator::IsNearEndGoal( TFLOAT a_fDistance )
{
	if ( m_eState != STATE_Moving )
		return TTRUE;

	TASSERT( !( m_CurrentGoal.GetGoal() == AIMoveGoal_t::scm_vInvalidGoal ) );

	if ( CurrentPath()->m_Path.Size() <= 0 )
		return TFALSE;

	if ( a_fDistance * a_fDistance >= TVector4::DistanceSq( CurrentPath()->m_Path.Back()->m_vPosition, m_pEntity->GetMatrix().GetTranslation() ) )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 007d32b0
TBOOL AAINavigator::FaceDirection( const TVector4& a_rvFacing )
{
	if ( m_eState == STATE_Moving )
		return TTRUE;

	TASSERT( a_rvFacing.IsNormalised() );
	TASSERT( a_rvFacing.y == 0 );
	m_pMotor->SetFacing( a_rvFacing );

	return TFALSE;
}

// The path finder's DestinationValid
// $deBlob: FUNCTION 007d33b0
TBOOL AAINavigator::DestinationValid( const TVector4& a_rvGoal, TVector4* a_pOutNodePosition )
{
	return m_pPathFinder->DestinationValid( a_rvGoal, a_pOutNodePosition );
}

// $deBlob: FUNCTION 007d33c0
ABuildPathJob* AAINavigator::BuildPath( const TVector4& a_rvStart, const TVector4& a_rvGoal, TFLOAT a_fRadius, TBOOL a_bUnk, const TVector4* a_pFacing, eGroundGoal a_eGroundGoal, eDoubleBuffer a_eDoubleBuffer )
{
	return ABuildPathJobQueue::GetBuildPathJobQueue()->AddNewJob( this, a_rvStart, a_rvGoal, a_fRadius, a_bUnk, a_pFacing, a_eGroundGoal, a_eDoubleBuffer );
}

// $deBlob: FUNCTION 007d35c0
TBOOL AAINavigator::BuildPathCheckGround( BuildPathState& a_rState, const TVector4& a_rvGoal, eGroundGoal a_eGroundGoal )
{
	AGndMotorComponent* pMotor = GetPathfinding( m_pEntity )->GetMotor();

	if ( !pMotor->IsOnGround( 0.03f ) )
		return TFALSE;

	a_rState.vGoal = a_rvGoal;

	if ( a_eGroundGoal != GROUNDGOAL_Ground )
		return TTRUE;

	// The ground under the goal, a little ahead of the owner's ground move
	TVector4 vPredicted = pMotor->GetGroundVelocity();
	vPredicted.Multiply( sm_fPredictTime );
	vPredicted.y = 0.0f;

	TVector4 vDirection( a_rState.vGoal.x - a_rState.vStart.x, a_rState.vGoal.y - a_rState.vStart.y, a_rState.vGoal.z - a_rState.vStart.z, a_rState.vGoal.w );
	vDirection.Normalise();
	vDirection.y = 0.0f;
	vDirection.Multiply( m_pEntity->GetBBox().x );

	TFLOAT   fGoalY = a_rvGoal.y - 1.0f;
	TVector4 vStart( ( a_rvGoal.x - vPredicted.x ) + vDirection.x, ( fGoalY - vPredicted.y ) + vDirection.y, ( a_rvGoal.z - vPredicted.z ) + vDirection.z, a_rvGoal.w );

	TFLOAT  fDistance  = 4.0f;
	TUINT32 uiMaterial = 0;

	ACollisionObject* pGround = ACollisionManager2::CollideRaySolidFast( vStart, TVector4::VEC_POSY, fDistance, m_pEntity->GetCollisionObject(), TNULL, 163, &uiMaterial );

	if ( ( uiMaterial & 0x2010 ) == 0 && pGround != TNULL )
	{
		TVector4 vUp;
		vUp.Multiply( TVector4::VEC_POSY, fDistance );
		a_rState.vGoal = TVector4( vUp.x + a_rvGoal.x, vUp.y + fGoalY, vUp.z + a_rvGoal.z, a_rvGoal.w );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007d33f0
void AAINavigator::ActivateAuxPath()
{
	TASSERT( HasValidAuxPath() );

	m_CurrentGoal.Reset();
	m_uiFlags &= ~1;
	m_eState                          = STATE_None;
	m_pCurrentWaypoint                = TNULL;
	m_pCurrentLink                    = TNULL;
	m_eResult                         = RESULT_None;
	m_fSteerEndTime                   = 0.0f;
	m_fStuckTime                      = 0.0f;
	m_aiPathInvalid[ m_iCurrentPath ] = 1;
	m_pMotor->StopImmediate();

	// The other path becomes the current one
	m_iCurrentPath                         = ( m_iCurrentPath == 0 );
	m_aiPathInvalid[ m_iCurrentPath == 0 ] = 1;

	TASSERT( TFALSE == CurrentPath()->m_Path.IsEmpty() );
	AAIPath* pPath = CurrentPath();
	SetGoalWaypoint( pPath->m_Path.Head(), pPath, TNULL );
	m_eState = STATE_Moving;
	TASSERT( !( m_CurrentGoal.GetGoal() == AIMoveGoal_t::scm_vInvalidGoal ) );
}

// $deBlob: FUNCTION 007d37b0
TBOOL AAINavigator::BuildPathLocal( BuildPathState& a_rState, TFLOAT a_fRadius, const TVector4* a_pFacing, eDoubleBuffer a_eDoubleBuffer )
{
	AAIPath* pPath;

	if ( a_eDoubleBuffer != DOUBLEBUFFER_Current )
	{
		pPath = m_apPaths[ m_iCurrentPath == 0 ];
	}
	else
	{
		ClearGoal( TFALSE );
		pPath = CurrentPath();
	}

	TINT* pPathInvalid = ( a_eDoubleBuffer != DOUBLEBUFFER_Current ) ? &m_aiPathInvalid[ m_iCurrentPath == 0 ] : &m_aiPathInvalid[ m_iCurrentPath ];
	*pPathInvalid      = 1;

	if ( TVector4::DistanceSq( a_rState.vGoal, a_rState.vStart ) > 100.0f )
		return TFALSE;

	TINT eResult = m_pPathFinder->PathFindLocal( a_rState.vStart, a_rState.vGoal, pPath, a_fRadius );

	if ( eResult == 1 )
	{
		*pPathInvalid = 0;

		if ( a_eDoubleBuffer == DOUBLEBUFFER_Current )
		{
			TASSERT( TFALSE == CurrentPath()->m_Path.IsEmpty() );
			SetGoalWaypoint( pPath->m_Path.Head(), pPath, a_pFacing );
			m_eState = STATE_Moving;
			TASSERT( !( m_CurrentGoal.GetGoal() == AIMoveGoal_t::scm_vInvalidGoal ) );
		}

		return TTRUE;
	}

	if ( eResult != 2 )
		return TFALSE;

	// Already there
	if ( a_eDoubleBuffer != DOUBLEBUFFER_Current )
		return TTRUE;

	m_pMotor->StopImmediate();
	m_eState  = STATE_Finished;
	m_eResult = RESULT_Reached;

	if ( a_pFacing != TNULL )
		FaceDirection( *a_pFacing );

	return TTRUE;
}

// $deBlob: FUNCTION 007d39d0
TBOOL AAINavigator::BuildPathNetwork( BuildPathState& a_rState, TFLOAT a_fRadius, const TVector4* a_pFacing, eDoubleBuffer a_eDoubleBuffer )
{
	TINT     iPath        = ( a_eDoubleBuffer != DOUBLEBUFFER_Current ) ? ( m_iCurrentPath == 0 ) : m_iCurrentPath;
	AAIPath* pPath        = m_apPaths[ iPath ];
	TINT*    pPathInvalid = &m_aiPathInvalid[ iPath ];
	*pPathInvalid         = 1;

	if ( !( GetPathfinding( m_pEntity )->GetFlags() & APathfindingComponent::FLAG_Network ) || m_pPathFinder->PathFindNetwork( a_rState.vStart, a_rState.vGoal, pPath, a_fRadius ) != 1 )
		return TFALSE;

	*pPathInvalid = 0;

	if ( a_eDoubleBuffer == DOUBLEBUFFER_Current )
	{
		SetGoalWaypoint( pPath->m_Path.Head(), pPath, TNULL );
		m_eState = STATE_Moving;
		TASSERT( !( m_CurrentGoal.GetGoal() == AIMoveGoal_t::scm_vInvalidGoal ) );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 007d3af0
void AAINavigator::SetGoalWaypoint( AAIWaypoint* a_pWaypoint, AAIPath* a_pPath, const TVector4* a_pFacing )
{
	m_pCurrentLink     = TNULL;
	m_pCurrentWaypoint = a_pWaypoint;

	if ( a_pWaypoint == TNULL )
	{
		m_CurrentGoal.Reset();
		return;
	}

	TASSERT( a_pPath->m_Path.IsInList( m_pCurrentWaypoint ) );

	// Shortcuts the path (ai_optimise_path, up to 4 entities a frame, high LOD ones only), never from a jump
	if ( sm_iNumOptimisedPaths < 4 && AStaticCVars::cvOptimisePath.GetBool() && m_pEntity->GetLOD() == ABaseEntity::FLAG_HighLOD && !( m_pCurrentWaypoint->m_uiFlags & AAIWaypoint::FLAG_NoShortcut ) )
	{
		AAIWaypoint* pCurrent = m_pCurrentWaypoint;
		AAILink*     pLink    = TNULL;

		if ( pCurrent->m_pNode != TNULL && pCurrent->Next() != TNULL )
		{
			AAINode* pNextNode = TSTATICCAST( AAIWaypoint, pCurrent->Next() )->m_pNode;

			if ( pNextNode != TNULL )
				pLink = pCurrent->m_pNode->GetLinkTo( pNextNode );
		}

		if ( pLink == TNULL || pLink->m_uiMoveType != AAILink::MOVETYPE_Jump )
		{
			sm_iNumOptimisedPaths++;
			m_pCurrentWaypoint = a_pPath->OptimisePath( m_pCurrentWaypoint );
		}
	}

	m_CurrentGoal.vGoal  = m_pCurrentWaypoint->m_vPosition;
	m_CurrentGoal.iUnk20 = 0;

	TBOOL bLinkMoveType = TFALSE;

	if ( m_pCurrentWaypoint->m_pNode != TNULL )
	{
		// The move type of the network link from the previous waypoint
		if ( m_pCurrentWaypoint != a_pPath->m_Path.Head().Get() )
		{
			AAIWaypoint* pPrevious = TSTATICCAST( AAIWaypoint, m_pCurrentWaypoint->Prev() );

			if ( pPrevious->m_pNode != TNULL )
			{
				AAILink* pLink = m_pCurrentWaypoint->m_pNode->GetLinkTo( pPrevious->m_pNode );

				if ( pLink != TNULL )
				{
					m_CurrentGoal.eMoveType = TINT8( pLink->m_uiMoveType );
					m_pCurrentLink          = pLink;
					bLinkMoveType           = TTRUE;
				}
			}
		}
	}

	if ( !bLinkMoveType )
	{
		TUINT8 uiFlags = m_pCurrentWaypoint->m_uiFlags;

		if ( uiFlags & AAIWaypoint::FLAG_Jump )
			m_CurrentGoal.eMoveType = 1;
		else if ( uiFlags & AAIWaypoint::FLAG_Fly )
			m_CurrentGoal.eMoveType = 2;
		else if ( uiFlags & AAIWaypoint::FLAG_Climb )
			m_CurrentGoal.eMoveType = 3;
		else
			m_CurrentGoal.eMoveType = 0;
	}

	// Can't walk: fly
	if ( m_CurrentGoal.eMoveType == 0 && !( GetPathfinding( m_pEntity )->GetFlags() & APathfindingComponent::FLAG_Walk ) )
	{
		TASSERT( GetPathfinding( m_pEntity )->AbleToFly() );
		m_CurrentGoal.eMoveType = 2;
	}

	m_CurrentGoal.fRadius = m_pCurrentWaypoint->m_fRadius;

	if ( a_pFacing != TNULL )
		m_CurrentGoal.vFacing = *a_pFacing;
}

// $deBlob: FUNCTION 007d3da0
void AAINavigator::OnReachedGoal()
{
	TASSERT( CurrentPath()->m_Path.IsInList( m_pCurrentWaypoint ) );

	if ( m_pCurrentWaypoint == CurrentPath()->m_Path.Back() )
	{
		// The end of the path
		m_eResult = RESULT_Reached;
		m_eState  = STATE_Finished;
		m_pMotor->StopImmediate();
		m_uiFlags &= ~1;
		m_pCurrentWaypoint                = TNULL;
		m_pCurrentLink                    = TNULL;
		m_aiPathInvalid[ m_iCurrentPath ] = 1;
		return;
	}

	SetGoalWaypoint( TSTATICCAST( AAIWaypoint, m_pCurrentWaypoint->Next() ), CurrentPath(), TNULL );

	if ( ( m_uiFlags & 1 ) && !( m_pCurrentWaypoint->m_uiFlags & AAIWaypoint::FLAG_Steer ) )
		m_uiFlags &= ~1;
}

// $deBlob: FUNCTION 007d3ee0
TBOOL AAINavigator::AttemptSteer( TFLOAT a_fDeltaTime )
{
	if ( !( m_uiFlags & 1 ) )
	{
		// Start steering, for 1.2 seconds at most
		GetPathfinding( m_pEntity );
		m_uiFlags |= 1;
		m_fSteerEndTime = g_oTheApp.m_fTotalTime + 1.2f;
		m_SteerGoal     = m_CurrentGoal;
		GetPathfinding( m_pEntity )->GetSteer()->ResetComponent();
	}
	else if ( m_fSteerEndTime < g_oTheApp.m_fTotalTime )
	{
		m_fSteerEndTime = 0.0f;
		m_uiFlags &= ~1;
		return TFALSE;
	}

	GetPathfinding( m_pEntity );
	AAIWaypoint* pWaypoint = GetPathfinding( m_pEntity )->GetSteer()->AttemptSteer( a_fDeltaTime, m_SteerGoal );

	if ( AStaticCVars::cvShowSteering.GetBool() )
		GetPathfinding( m_pEntity )->GetSteer()->RenderConstraints();

	if ( pWaypoint != TNULL )
	{
		// The steering waypoint goes before the current one
		pWaypoint->InsertBefore( m_pCurrentWaypoint );

		// NOTE: retail computes the facing from the steering waypoint to the current one and never uses it
		TVector4 vFacing;
		vFacing.Substract( m_pCurrentWaypoint->m_vPosition, pWaypoint->m_vPosition );
		vFacing.w = m_pCurrentWaypoint->m_vPosition.w;
		vFacing.NormaliseXZ();

		SetGoalWaypoint( pWaypoint, CurrentPath(), TNULL );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007d40a0
void AAINavigator::CheckProgress()
{
	const TVector4& rcPosition = m_pEntity->GetMatrix().GetTranslation();
	const TVector4& rcGoal     = m_CurrentGoal.GetGoal();

	TBOOL bFar;

	switch ( m_CurrentGoal.eMoveType )
	{
		case 0:
		{
			// NOTE: retail compares the squared XZ distance with the radius itself
			TFLOAT fDistanceSq = ( rcPosition.z - rcGoal.z ) * ( rcPosition.z - rcGoal.z ) + ( rcPosition.x - rcGoal.x ) * ( rcPosition.x - rcGoal.x );
			bFar               = m_CurrentGoal.fRadius < fDistanceSq;
			break;
		}
		case 1:
		case 3:
			bFar = TMath::Max( 0.01f, m_CurrentGoal.fRadius ) < TVector4::DistanceSq( rcGoal, rcPosition );
			break;
		case 2:
		{
			TFLOAT fDistanceSq = TVector4::DistanceSq( rcGoal, rcPosition );

			if ( m_CurrentGoal.fRadius < fDistanceSq )
				bFar = 0.3f < fDistanceSq;
			else
				bFar = TFALSE;

			break;
		}
		default:
			return;
	}

	if ( !bFar )
		OnReachedGoal();
}
