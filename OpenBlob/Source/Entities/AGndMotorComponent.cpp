#include "pch.h"
#include "AGndMotorComponent.h"
#include "ABaseEntity.h"
#include "Collision/ACollisionObject.h"
#include "AEntityData.h"
#include "PathFinding/APathfindingComponent.h"
#include "PathFinding/AAINavigator.h"
#include "PathFinding/AIMoveGoal.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Collision/ACollisionManager2.h"
#include "APropPlatform.h"

#include <Math/TQuaternion.h>
#include <Toshi/TSystem.h>
#include <Math/TCollision.h>

#include <Physics/TSurfaceType.h>
#include <Physics/TBoxData.h>
#include <Physics/Havok/TPhysicsManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AGndMotorComponent::Data* AGndMotorComponent::sm_pData;
// The steepest ground that is walkable (cos 50)
// $deBlob: FUNCTION 00541950
TFLOAT       AGndMotorComponent::sm_fMinGroundDot          = 0.64278764f;
const TFLOAT AGndMotorComponent::scm_fMaxSlidableGroundDot = 0.34202009f;

TSTATICASSERT( sizeof( AGndMotorComponent ) == 0x284 );

// $deBlob: FUNCTION 007d96f0
AGndMotorComponent::AGndMotorComponent( ABaseEntity* a_pEntity )
    : AEntityComponent2( a_pEntity )
{
	m_vAcceleration          = TVector4::VEC_ZERO;
	m_vVelocity              = TVector4::VEC_ZERO;
	m_vGravityDir            = TVector4::VEC_NEGY;
	m_vFacing                = TVector4::VEC_POSZ;
	m_vDesiredFacing         = TVector4::VEC_POSZ;
	m_vGroundVelocity        = TVector4::VEC_ZERO;
	m_vUnkAC                 = TVector4::VEC_ZERO;
	m_vUnkBC                 = TVector4::VEC_ZERO;
	m_oGroundContact.uiFlags = 0;

	m_fSpeed              = 3.0f;
	m_fTerminalVelocity   = 200.0f;
	m_fAcceleration       = -1.0f;
	m_fDeceleration       = -1.0f;
	m_iState              = 0;
	m_fStateTime          = 0.0f;
	m_fHeightOffset       = 0.0f;
	m_fNextHeightOffset   = -1.0f;
	m_bHasReactionForce   = TFALSE;
	m_fCurrentSpeed       = 0.0f;
	m_fLaunchTime         = 0.0f;
	m_fLaunchDrag         = 0.0f;
	m_iMoveType           = 0;
	m_fMoveDeltaTime      = 0.0f;
	m_iStuckCount         = 0;
	m_uiFlags             = FLAG_Unk80;
	m_iUnk270             = 3;
	m_pReactionEntity     = TNULL;
	m_apQueryHelpers[ 0 ] = TNULL;
	m_apQueryHelpers[ 1 ] = TNULL;
	m_apQueryHelpers[ 2 ] = TNULL;

	m_pSurface = CollisionSurfaces::GetDefaultSurface();
}

// $deBlob: FUNCTION 007d98d0
// $deBlob: FUNCTION 007dd430
AGndMotorComponent::~AGndMotorComponent()
{
	for ( TINT i = 0; i < 3; i++ )
	{
		if ( m_apQueryHelpers[ i ] != TNULL )
		{
			TPhysicsManager::GetSingleton()->DestroyPhantom( m_apQueryHelpers[ i ] );
			m_apQueryHelpers[ i ] = TNULL;
		}
	}
}

// NOTE: a retail helper of the collision code (ATerrain2Interface::sm_Instance->IsPositionInReadyCell( pos, TTRUE ))
// $deBlob: FUNCTION 005a95c0
static TBOOL IsPositionInReadyCell( const TVector4& a_rPosition )
{
	return ATerrain2Interface::GetInstance()->IsPositionInReadyCell( a_rPosition, TTRUE );
}

// $deBlob: FUNCTION 007d99d0
void AGndMotorComponent::InitialiseData()
{
	sm_pData = TSTATICCAST( Data, AEntityData::GetSingleton()->GetEntityData( "GlobalData_AMovementComponentData_EntityData" ) );

	TASSERT( sm_pData->Gravity > 0 && sm_pData->Gravity < 100.0f );
	TASSERT( sm_pData->AirFriction->IsReasonable() );
	TASSERT( sm_pData->GroundFriction->IsReasonable() );
	TASSERT( sm_pData->AirTraction->IsReasonable() );
	TASSERT( sm_pData->GroundTraction->IsReasonable() );
}

// $deBlob: FUNCTION 007d9b80
void AGndMotorComponent::Tick( TFLOAT a_fDeltaTime )
{

	if ( !IsPositionInReadyCell( m_pEntity->GetMatrix().GetTranslation() ) )
		return;

	if ( m_fHeightOffset != 0.0f )
	{
		m_uiCollisionFlags     = m_pEntity->GetCollisionObject()->GetGroupMask();
		m_uiPathCollisionFlags = TSTATICCAST( APathfindingComponent, m_pEntity->GetPathfinding() )->GetCollisionFlags();
	}

	// The height offset is taken away during the move
	if ( m_fHeightOffset != 0.0f )
	{
		TUINT8 uiCollisionFlags = TUINT8( m_uiCollisionFlags ) & 0xEC;
		m_pEntity->GetCollisionObject()->SetGroupMask( uiCollisionFlags );

		TVector4 vPosition = m_pEntity->GetMatrix().GetTranslation();
		vPosition.y += m_fHeightOffset;
		m_pEntity->SetPosition( vPosition );
	}

	m_oTouchList.Clear();
	m_uiFlags &= ~FLAG_AtRest;

	if ( IsOnGround( 0.03f ) || GetGravity() <= TMath::TFLOAT_EPSILON )
	{
		if ( TMath::Abs( m_vVelocity.x ) < TMath::TFLOAT_EPSILON && TMath::Abs( m_vVelocity.z ) < TMath::TFLOAT_EPSILON &&
		     m_vVelocity.y >= 0.0f && m_fLaunchTime <= 0.0f && m_vAcceleration.MagnitudeSq() <= TMath::TFLOAT_EPSILON &&
		     !HASANYFLAG( m_uiFlags, FLAG_ProbeGround | FLAG_Unk100 | FLAG_Unk400 ) )
		{
			m_vVelocity.y   = 0.0f;
			m_fCurrentSpeed = 0.0f;
			m_uiFlags |= FLAG_AtRest;
		}
	}

	if ( HASANYFLAG( m_uiFlags, FLAG_AtRest ) )
	{
		// Something else moved the entity: it isn't at rest
		TVector4 vMoved = m_vLastPosition - m_pEntity->GetMatrix().GetTranslation();
		vMoved.w        = m_vLastPosition.w;
		vMoved.Abs4();

		if ( TMath::TFLOAT_EPSILON < vMoved.x || TMath::TFLOAT_EPSILON < vMoved.y || TMath::TFLOAT_EPSILON < vMoved.z )
			m_uiFlags &= ~FLAG_AtRest;
	}

	if ( HASANYFLAG( m_uiFlags, FLAG_ProbeGround ) )
		UpdateTouches( a_fDeltaTime );

	if ( !HASANYFLAG( m_uiFlags, FLAG_AtRest ) || m_bHasReactionForce == TTRUE )
		UpdateGroundContact( a_fDeltaTime );

	if ( TSTATICCAST( AEntityComponent2, m_pEntity->GetPathfinding() )->IsEnabled() != TTRUE )
	{
		UpdateCharacterMovement( a_fDeltaTime );
	}
	else
	{
		if ( m_iMoveType == 0 )
			UpdateCharacterMovement( a_fDeltaTime );
		else if ( m_iMoveType == 1 )
			UpdateAIClimb( a_fDeltaTime );
		else if ( m_iMoveType == 2 )
			UpdateAIFly( a_fDeltaTime );

		m_fMoveDeltaTime = 0.0f;
	}

	m_vLastPosition = m_pEntity->GetMatrix().GetTranslation();

	if ( 200.0f < m_vLastPosition.y && !m_pEntity->HandleFallenThroughWorld() )
	{
		TVector4 vPosition = m_pEntity->GetMatrix().GetTranslation();
		vPosition.y        = -200.0f;
		m_pEntity->SetPosition( vPosition );

		m_vVelocity     = TVector4::VEC_ZERO;
		m_vLastPosition = m_pEntity->GetMatrix().GetTranslation();
	}

	// The height offset is given back
	if ( m_fHeightOffset != 0.0f )
	{
		TUINT8 uiCollisionFlags = TUINT8( m_uiCollisionFlags );
		m_pEntity->GetCollisionObject()->SetGroupMask( uiCollisionFlags );

		if ( m_fNextHeightOffset != -1.0f )
		{
			m_fHeightOffset     = m_fNextHeightOffset;
			m_fNextHeightOffset = -1.0f;
		}

		TVector4 vPosition = m_pEntity->GetMatrix().GetTranslation();
		vPosition.y -= m_fHeightOffset;
		m_pEntity->SetPosition( vPosition );
	}

	m_uiFlags &= ~FLAG_ProbeGround;
}

// $deBlob: FUNCTION 007d9fb0
void AGndMotorComponent::ResetComponent()
{
	m_fCurrentSpeed   = 0.0f;
	m_fLaunchTime     = 0.0f;
	m_vVelocity       = TVector4::VEC_ZERO;
	m_uiFlags         = ( m_uiFlags & FLAG_CanCacheGround ) | FLAG_Unk80;
	m_iState          = 0;
	m_vUnkAC          = TVector4::VEC_ZERO;
	m_pReactionEntity = TNULL;
	m_vGroundVelocity = TVector4::VEC_ZERO;
	m_vUnkBC          = TVector4::VEC_ZERO;

	// The height offset is taken back
	if ( m_fHeightOffset != 0.0f )
	{
		TUINT8 uiCollisionFlags = TUINT8( m_uiCollisionFlags );
		m_pEntity->GetCollisionObject()->SetGroupMask( uiCollisionFlags );

		TVector4 vPosition = m_pEntity->GetMatrix().GetTranslation();
		vPosition.y -= m_fHeightOffset;
		m_pEntity->SetPosition( vPosition );
	}

	m_uiFlags |= FLAG_CanCacheGround;
	m_avGroundTriangle[ 0 ]     = TVector4::VEC_ZERO;
	m_avGroundTriangle[ 1 ]     = TVector4::VEC_ZERO;
	m_avGroundTriangle[ 2 ]     = TVector4::VEC_ZERO;
	m_oGroundContact.vNormal    = TVector4::VEC_NEGY;
	m_oGroundContact.fDistance  = 0.0f;
	m_oGroundContact.pObject    = TNULL;
	m_oGroundContact.eGroupFlag = 0;
	m_oGroundContact.uiFlags    = 0;

	m_oTouchList.Clear();
}

// $deBlob: FUNCTION 007dc980
void AGndMotorComponent::UpdateTouches( TFLOAT a_fDeltaTime )
{
	static const TVector4 s_vDown( 0.0f, 1.0f, 0.0f, 1.0f ); // (y is down)

	TVector4 vPosition = m_pEntity->GetBasePosition();

	if ( m_fHeightOffset != 0.0f )
	{
		m_pEntity->GetCollisionObject()->SetGroupMask( TUINT8( m_uiCollisionFlags ) );
		vPosition.y -= m_fHeightOffset;
	}

	// The objects just below and above
	EntityContactFunctor oFunctor( &m_oTouchList );

	TFLOAT   fDistance = 0.05f;
	TVector4 vNormal;
	TFLOAT   fStep;

	ACollisionObject* pObject = ACollisionManager2::GetSingleton()->CollideSweptAABB( vPosition, m_pEntity->GetBBox(), s_vDown, fDistance, vNormal, 0.0f, fStep, m_pEntity->GetCollisionObject(), &oFunctor, 6, m_apQueryHelpers[ 0 ] );

	if ( pObject != TNULL )
		AMoveHelper::AddTouchedObject( &m_oTouchList, pObject, vNormal );

	fDistance = 0.05f;
	pObject   = ACollisionManager2::GetSingleton()->CollideSweptAABB( vPosition, m_pEntity->GetBBox(), TVector4::VEC_NEGY, fDistance, vNormal, 0.0f, fStep, m_pEntity->GetCollisionObject(), &oFunctor, 6, m_apQueryHelpers[ 0 ] );

	if ( pObject != TNULL )
		AMoveHelper::AddTouchedObject( &m_oTouchList, pObject, vNormal );
}

// $deBlob: FUNCTION 007dc5c0
void AGndMotorComponent::UpdateGroundContact( TFLOAT a_fDeltaTime )
{
	GroundContactInfo oContact;
	oContact.uiFlags    = 0;
	oContact.eGroupFlag = 0;

	TUINT32 uiFlags = m_uiFlags;
	m_uiFlags &= ~FLAG_OnWalkableGround;

	if ( !HASANYFLAG( uiFlags, FLAG_Held ) && FindGroundContact( 0.2f, &oContact ) == TTRUE )
	{
		TFLOAT fUp = -oContact.vNormal.y;

		if ( !HASANYFLAG( oContact.pObject->GetFlags(), ACollisionObject::FLAG_OwnerType1 | ACollisionObject::FLAG_OwnerType2 ) )
		{
			// The terrain: 1 walkable, 3 too steep
			m_iState = ( fUp <= sm_fMinGroundDot ) ? 3 : 1;

			if ( sm_fMinGroundDot < fUp && HASANYFLAG( oContact.uiFlags, GroundContactInfo::FLAG_InsideEdges ) )
				m_uiFlags |= FLAG_OnWalkableGround;
		}
		else
		{
			// An entity: 2 walkable, 4 too steep
			m_iState = ( fUp <= sm_fMinGroundDot ) ? 4 : 2;
		}

		m_vGravityDir = oContact.vNormal;
		m_fStateTime  = oContact.fDistance;
		m_pSurface    = CollisionSurfaces::GetCollisionSurface( oContact.eGroupFlag );
		return;
	}

	m_iState     = 0;
	m_fStateTime = TMath::MAXFLOAT;
	m_pSurface   = CollisionSurfaces::GetDefaultSurface();
}

// $deBlob: FUNCTION 007dc6d0
void AGndMotorComponent::UpdateMovement( TFLOAT a_fDeltaTime )
{
	UpdateCharacterMovement( a_fDeltaTime );
}

// $deBlob: FUNCTION 007dc6f0
void AGndMotorComponent::UpdateAIClimb( TFLOAT a_fDeltaTime )
{
	if ( m_JumpParabola.IsActive() && m_JumpParabola.Update( a_fDeltaTime ) )
	{
		TVector4 vPosition;
		m_JumpParabola.GetPosition( vPosition, 0.0f );
		m_JumpParabola.GetVelocity( m_vVelocity );
		m_pEntity->SetPosition( vPosition );
	}
}

// $deBlob: FUNCTION 007dc760
void AGndMotorComponent::UpdateAIFly( TFLOAT a_fDeltaTime )
{
	// The flight moves with the velocity, the touched entities are told, a hit from below stops the climb
	AMoveHelper::TouchList oTouches;

	TVector4 vMove;
	vMove.Multiply( m_vVelocity, a_fDeltaTime );

	TVector4 vNewPosition;
	TVector4 vMoved = AMoveHelper::PerformClampedMove( m_pEntity, vMove, vNewPosition, &oTouches, TFALSE, TVector4::VEC_ZERO, TNULL, TFALSE, TNULL );

	if ( vMoved.MagnitudeSq() <= 0.0f )
	{
		m_vVelocity = TVector4::VEC_ZERO;
		return;
	}

	for ( TINT i = 0; i < oTouches.Size(); i++ )
	{
		AMoveHelper::ObjectTouched& rTouch = oTouches[ i ];

		if ( HASANYFLAG( rTouch.pObject->GetFlags(), ACollisionObject::FLAG_OwnerType1 | ACollisionObject::FLAG_OwnerType2 ) )
		{
			TObject* pOwnerObject = TSTATICCAST( TObject, rTouch.pObject->GetOwner() );
			TASSERT( ( pOwnerObject == TNULL ) || pOwnerObject->IsA( &TGetClass( ABaseEntity ) ) );
			TSTATICCAST( ABaseEntity, pOwnerObject )->OnTouched( m_pEntity, &vNewPosition, &rTouch.vNormal );
		}

		if ( m_vVelocity.y < 0.0f && -rTouch.vNormal.y < 0.0f )
			m_vVelocity.y = 0.0f;
	}

	const TVector4& rcPosition = m_pEntity->GetMatrix().GetTranslation();
	m_vDesiredFacing.Set( vNewPosition.x - rcPosition.x, vNewPosition.y - rcPosition.y, vNewPosition.z - rcPosition.z, vNewPosition.w );
	m_vDesiredFacing.NormaliseXZ();

	m_pEntity->SetPosition( vNewPosition );

	m_vVelocity.Divide( vMoved, a_fDeltaTime );
	m_vVelocity.w = 1.0f;
}

// $deBlob: FUNCTION 007da1c0
void AGndMotorComponent::AddVelocity( const TVector4& a_rcVelocity )
{
	m_vVelocity.x = m_vVelocity.x + a_rcVelocity.x;
	m_vVelocity.y = a_rcVelocity.y + m_vVelocity.y;
	m_vVelocity.z = a_rcVelocity.z + m_vVelocity.z;
}

// $deBlob: FUNCTION 007da200
void AGndMotorComponent::StopImmediate()
{
	TBOOL bAIMove = TSTATICCAST( AEntityComponent2, m_pEntity->GetPathfinding() )->IsEnabled();

	if ( bAIMove && m_iMoveType != 0 )
	{
		if ( m_iMoveType != 1 )
		{
			m_fCurrentSpeed = 0.0f;
			m_vVelocity     = TVector4::VEC_ZERO;
			return;
		}

		m_iMoveType = 0;
	}

	m_vVelocity.z   = 0.0f;
	m_vVelocity.x   = 0.0f;
	m_fCurrentSpeed = 0.0f;
}

// $deBlob: FUNCTION 007da6b0
void AGndMotorComponent::SetQueryHelpersActive( TBOOL a_bActive )
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	if ( !a_bActive )
	{
		if ( m_apQueryHelpers[ 0 ] != TNULL )
			pPhysicsManager->RemovePhantomFromSimulation( m_apQueryHelpers[ 0 ], TNULL );

		if ( m_apQueryHelpers[ 2 ] != TNULL )
			pPhysicsManager->RemovePhantomFromSimulation( m_apQueryHelpers[ 2 ], TNULL );

		if ( m_apQueryHelpers[ 1 ] != TNULL )
			pPhysicsManager->RemovePhantomFromSimulation( m_apQueryHelpers[ 1 ], TNULL );

		return;
	}

	if ( m_apQueryHelpers[ 0 ] == TNULL )
	{
		// The entity's box, and a flat one (0.2 high) for the ground probe
		TBoxData oBox;
		oBox.SetHalfExtents( m_pEntity->GetBBox() );
		m_pEntity->GetBasePosition();

		m_apQueryHelpers[ 0 ] = pPhysicsManager->CreateShapePhantom( &oBox );
		m_apQueryHelpers[ 2 ] = pPhysicsManager->CreateShapePhantom( &oBox );

		TVector4 vFlat = m_pEntity->GetBBox();
		vFlat.y        = 0.2f;
		oBox.SetHalfExtents( vFlat );
		m_apQueryHelpers[ 1 ] = pPhysicsManager->CreateShapePhantom( &oBox );
	}

	if ( m_apQueryHelpers[ 0 ] != TNULL )
	{
		TPhysicsManager::SetCollisionTestFlags( m_apQueryHelpers[ 0 ], 0x6 );
		pPhysicsManager->AddPhantomToSimulation( m_apQueryHelpers[ 0 ], m_pEntity->GetBasePosition(), TNULL );
	}

	if ( m_apQueryHelpers[ 2 ] != TNULL )
	{
		TPhysicsManager::SetCollisionTestFlags( m_apQueryHelpers[ 2 ], 0x87 );
		pPhysicsManager->AddPhantomToSimulation( m_apQueryHelpers[ 2 ], m_pEntity->GetBasePosition(), TNULL );
	}

	if ( m_apQueryHelpers[ 1 ] != TNULL )
	{
		TPhysicsManager::SetCollisionTestFlags( m_apQueryHelpers[ 1 ], 0x87 );
		pPhysicsManager->AddPhantomToSimulation( m_apQueryHelpers[ 1 ], m_pEntity->GetBasePosition(), TNULL );
	}
}

// $deBlob: FUNCTION 007da270
void AGndMotorComponent::SetVelocity( const TVector4& a_rcVelocity, TFLOAT a_fTime, TFLOAT a_fDrag, TBOOL a_bFace )
{
	m_fLaunchDrag = a_fDrag;
	m_fLaunchTime = a_fTime;

	if ( a_bFace )
	{
		m_vFacing = a_rcVelocity;
		m_vFacing.NormaliseXZ();
	}

	m_vVelocity   = a_rcVelocity;
	m_vVelocity.w = 1.0f;

	// Going up (y is down): off the ground
	if ( a_rcVelocity.y < -TMath::TFLOAT_EPSILON )
		m_iState = 0;
}

// $deBlob: FUNCTION 007da360
TINT AGndMotorComponent::MoveToGoal( const AIMoveGoal_t& a_rcGoal, TFLOAT a_fDeltaTime, TINT a_eMoveType )
{
	m_iMoveType      = a_eMoveType;
	m_fMoveDeltaTime = a_fDeltaTime;

	switch ( a_eMoveType )
	{
		case 0:
			if ( m_JumpParabola.IsActive() )
				m_JumpParabola.SetActive( TFALSE );

			return WalkToGoal( a_rcGoal, a_fDeltaTime );
		case 1:
			return JumpToGoal( a_rcGoal, a_fDeltaTime );
		case 2:
			return FlyToGoal( a_rcGoal, a_fDeltaTime );
		default:
			return 0;
	}
}

// $deBlob: FUNCTION 007da4d0
TBOOL AGndMotorComponent::IsStandableCollisionObject( ACollisionObject* a_pObject, const TVector4& a_rcNormal )
{
	if ( a_pObject == TNULL )
		return TFALSE;

	if ( HASANYFLAG( a_pObject->GetFlags(), ACollisionObject::FLAG_OwnerType0 ) )
		return a_rcNormal.IsEqual( TVector4( 0.0f, 0.0f, 0.0f, 1.0f ) ) || -0.64278764f >= a_rcNormal.y;

	if ( HASANYFLAG( a_pObject->GetFlags(), ACollisionObject::FLAG_OwnerType1 | ACollisionObject::FLAG_OwnerType2 ) )
	{
		TObject* pOwner = TSTATICCAST( TObject, a_pObject->GetOwner() );

		if ( pOwner != TNULL && pOwner->IsA( &TGetClass( ABaseEntity ) ) )
		{
			TUINT32 uiFlags = TSTATICCAST( ABaseEntity, pOwner )->GetFlags();
			return !HASANYFLAG( uiFlags, 0x8 ) && !HASANYFLAG( uiFlags, 0x10 ) && !HASANYFLAG( uiFlags, 0x20 ) && HASANYFLAG( uiFlags, ABaseEntity::FLAG_Visible );
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007da590
TVector4 AGndMotorComponent::GetGroundedPosition() const
{
	const TVector4& rcPosition = m_pEntity->GetMatrix().GetTranslation();
	TFLOAT          fY         = rcPosition.y;

	if ( 0.0f < m_fHeightOffset )
		fY = m_fHeightOffset + rcPosition.y;

	return TVector4( rcPosition.x, fY, rcPosition.z, rcPosition.w );
}

// $deBlob: FUNCTION 007da5d0
void AGndMotorComponent::ReactToEntity( ABaseEntity* a_pEntity )
{
	TVector4 vForce = a_pEntity->GetReactionForce( &m_vVelocity, m_pEntity );

	TVector4 vMove;
	vMove.Multiply( vForce, TSystemManager::GetSingleton()->GetTimer().GetDelta() );

	TVector4 vPosition;
	AMoveHelper::PerformClampedMove( m_pEntity, vMove, vPosition, TNULL, TFALSE, TVector4::VEC_ZERO, a_pEntity->GetCollisionObject(), TFALSE, m_apQueryHelpers[ 2 ] );

	const TVector4& rcPosition = m_pEntity->GetTranslation();
	m_vDesiredFacing           = TVector4( vPosition.x - rcPosition.x, vPosition.y - rcPosition.y, vPosition.z - rcPosition.z, vPosition.w );
	m_vDesiredFacing.NormaliseXZ();

	m_pEntity->SetPosition( vPosition );
	m_uiFlags = ( m_uiFlags & ~FLAG_AtRest ) | FLAG_Unk100;
}

// Forgets the cached ground (FLAG_GroundCached) if it is a_pObject
// $deBlob: FUNCTION 007da910
void AGndMotorComponent::OnCollisionObjectDestroyed( ACollisionObject* a_pObject )
{
	if ( m_oGroundContact.pObject == a_pObject )
	{
		m_avGroundTriangle[ 0 ] = TVector4::VEC_ZERO;
		m_avGroundTriangle[ 1 ] = TVector4::VEC_ZERO;
		m_avGroundTriangle[ 2 ] = TVector4::VEC_ZERO;
		m_uiFlags &= ~FLAG_GroundCached;
		m_oGroundContact.fDistance  = 0.0f;
		m_oGroundContact.vNormal    = TVector4::VEC_NEGY;
		m_oGroundContact.pObject    = TNULL;
		m_oGroundContact.eGroupFlag = 0;
		m_oGroundContact.uiFlags    = 0;
	}
}

// $deBlob: FUNCTION 007da9c0
void AGndMotorComponent::MoveTo( const TVector4& a_rcPosition )
{
	const TVector4& rcPosition = m_pEntity->GetTranslation();
	m_vDesiredFacing           = TVector4( a_rcPosition.x - rcPosition.x, a_rcPosition.y - rcPosition.y, a_rcPosition.z - rcPosition.z, a_rcPosition.w );
	m_vDesiredFacing.NormaliseXZ();

	m_pEntity->SetPosition( a_rcPosition );
}

// $deBlob: FUNCTION 007daa30
TINT AGndMotorComponent::WalkToGoal( const AIMoveGoal_t& a_rcGoal, TFLOAT a_fDeltaTime )
{
	const TVector4& rcPosition = m_pEntity->GetMatrix().GetTranslation();
	const TVector4& rcGoal     = a_rcGoal.GetGoal();

	TVector4 vToGoal( rcGoal.x - rcPosition.x, rcGoal.y - rcPosition.y, rcGoal.z - rcPosition.z, 1.0f );
	TFLOAT   fDistance = vToGoal.MagnitudeXZ();

	if ( fDistance <= TMath::TFLOAT_EPSILON )
	{
		// Arrived: the facing of the goal
		if ( a_rcGoal.vFacing.MagnitudeSqXZ() > TMath::TFLOAT_EPSILON )
		{
			m_vFacing = a_rcGoal.vFacing;
			TASSERT( m_vFacing.IsNormalised() );
		}

		return 3;
	}

	m_vVelocity.x = vToGoal.x / fDistance;
	m_vVelocity.z = vToGoal.z / fDistance;
	vToGoal.Normalise();

	if ( a_rcGoal.vFacing.MagnitudeSqXZ() <= TMath::TFLOAT_EPSILON || 0.3f <= fDistance )
	{
		m_vFacing = m_vVelocity;
		m_vFacing.NormaliseXZ();
	}
	else
	{
		m_vFacing = a_rcGoal.vFacing;
		TASSERT( m_vFacing.IsNormalised() );
	}

	if ( m_iState == 2 )
	{
		// Is there ground ahead (a ray from half a unit above the feet, ahead by the box's width)?
		TVector4 vGroundMove;
		vGroundMove.Multiply( m_vGroundVelocity, a_fDeltaTime );
		vGroundMove.y = 0.0f;

		const TVector4& rcBBox     = m_pEntity->GetBBox();
		TFLOAT          fDistance2 = rcBBox.y + 0.5f;

		TVector4 vAhead;
		vAhead.Multiply( vToGoal, rcBBox.x );

		TVector4 vStart( rcGoal.x + ( vAhead.x - vGroundMove.x ), ( rcPosition.y - 0.5f ) + ( vAhead.y - vGroundMove.y ), rcGoal.z + ( vAhead.z - vGroundMove.z ), rcGoal.w );

		ACollisionObject* pGround = ACollisionManager2::CollideRaySolidFast( vStart, TVector4::VEC_POSY, fDistance2, m_pEntity->GetCollisionObject(), TNULL, 0x83, TNULL );

		if ( pGround == TNULL || !IsStandableCollisionObject( pGround, TVector4::VEC_ZERO ) )
		{
			m_fCurrentSpeed = 0.0f;
			return 0;
		}
	}

	if ( m_fDeceleration <= 0.0f || m_fAcceleration <= 0.0f )
	{
		m_fCurrentSpeed = m_fSpeed;
	}
	else
	{
		// Accelerates (m_fAcceleration) until the braking distance (m_fDeceleration) reaches the end of the path
		// NOTE: retail reads its end goal into an uninitialised vector when the navigator has none
		TVector4 vEndGoal = TVector4::VEC_ZERO;
		TSTATICCAST( APathfindingComponent, m_pEntity->GetPathfinding() )->GetNavigator()->GetEndGoal( vEndGoal );

		TFLOAT fTime          = m_fCurrentSpeed / m_fDeceleration;
		TFLOAT fBrakeDistance = m_fDeceleration * fTime * fTime + m_fCurrentSpeed * fTime;

		TVector4 vPosition = rcPosition;

		if ( 0.0f < m_fHeightOffset )
			vPosition.y = m_fHeightOffset + rcPosition.y;

		if ( fBrakeDistance * fBrakeDistance < TVector4::DistanceSq( vEndGoal, vPosition ) )
			m_fCurrentSpeed += TMath::Min( m_fAcceleration * a_fDeltaTime, m_fSpeed - m_fCurrentSpeed );
		else
			m_fCurrentSpeed -= m_fDeceleration * a_fDeltaTime;
	}

	m_vDesiredFacing.x = m_vVelocity.x;
	m_vDesiredFacing.z = m_vVelocity.z;
	m_vDesiredFacing.y = 0.0f;

	TASSERT( TVector2( m_vVelocity.x, m_vVelocity.z ).IsNormalised() );

	TFLOAT fSpeed = m_fCurrentSpeed;
	m_vVelocity.x *= fSpeed;
	m_vVelocity.z *= fSpeed;

	// Not past the goal
	if ( fDistance < fSpeed * a_fDeltaTime )
	{
		TFLOAT fScale = fDistance / ( fSpeed * a_fDeltaTime );
		m_vVelocity.x *= fScale;
		m_vVelocity.z *= fScale;
	}

	return m_iUnk270;
}

// $deBlob: FUNCTION 007daee0
TINT AGndMotorComponent::JumpToGoal( const AIMoveGoal_t& a_rcGoal, TFLOAT a_fDeltaTime )
{
	if ( !m_JumpParabola.IsActive() )
	{
		// Start the jump
		TVector4 vGoal = a_rcGoal.GetGoal();

		if ( !m_JumpParabola.CheckValidJump( m_pEntity, m_pEntity->GetTranslation(), a_rcGoal.GetGoal(), 0, TNULL, AAIJumpTrajectory::JumpType_Auto, TTRUE, TFALSE ) )
			return 0;

		const TVector4& rcPosition = m_pEntity->GetTranslation();
		m_vFacing                  = TVector4( vGoal.x - rcPosition.x, vGoal.y - rcPosition.y, vGoal.z - rcPosition.z, vGoal.w );
		m_vFacing.NormaliseXZ();
	}
	else
	{
		TASSERT( m_JumpParabola.HasValidTrajectory() );
		m_iState = 0;

		// Landed
		if ( m_JumpParabola.GetTotalTime() < m_JumpParabola.GetTime() )
		{
			m_JumpParabola.SetActive( TFALSE );
			return 3;
		}

		if ( m_JumpParabola.CheckJumpCollision( 0.0f ) )
		{
			m_JumpParabola.SetActive( TFALSE );
			return 0;
		}
	}

	return 3;
}

// $deBlob: FUNCTION 007db030
TINT AGndMotorComponent::FlyToGoal( const AIMoveGoal_t& a_rcGoal, TFLOAT a_fDeltaTime )
{
	const TVector4& rcPosition = m_pEntity->GetTranslation();
	const TVector4& rcGoal     = a_rcGoal.GetGoal();

	TVector4 vToGoal( rcGoal.x - rcPosition.x, rcGoal.y - rcPosition.y, rcGoal.z - rcPosition.z, 1.0f );
	TFLOAT   fDistance = vToGoal.Magnitude();

	if ( fDistance > TMath::TFLOAT_EPSILON )
	{
		vToGoal.Divide( fDistance );
		m_vVelocity   = vToGoal;
		m_vVelocity.w = 1.0f;

		// Face the goal's facing, or where it flies when there's none
		m_vFacing = ( a_rcGoal.vFacing.MagnitudeSqXZ() > TMath::TFLOAT_EPSILON ) ? a_rcGoal.vFacing : m_vVelocity;
		m_vFacing.NormaliseXZ();

		// Accelerate up to m_fSpeed
		if ( m_fCurrentSpeed < m_fSpeed )
		{
			m_pEntity->GetPathfinding();

			TFLOAT fDifference = m_fSpeed - m_fCurrentSpeed;
			TFLOAT fMax        = a_fDeltaTime * 10000.0f;
			m_fCurrentSpeed += TMath::Min( fMax, fDifference );
		}

		m_vDesiredFacing = m_vVelocity;
		TASSERT( m_vVelocity.IsNormalised() );
		m_vVelocity.Multiply( m_fCurrentSpeed );

		// Don't overshoot
		TFLOAT fMove = m_fCurrentSpeed * a_fDeltaTime;

		if ( fDistance < fMove )
			m_vVelocity.Multiply( fDistance / fMove );
	}

	return 3;
}

// $deBlob: FUNCTION 007db1f0
TBOOL AGndMotorComponent::FindGroundContact( TFLOAT a_fTolerance, GroundContactInfo* a_pContact )
{
	TASSERT( a_fTolerance > 0 );

	GroundProbeCollideFunctor2 oFunctor( &m_oTouchList );

	TFLOAT                    fDistance = a_fTolerance + 1.0f;
	ACollisionObject*         pHit      = TNULL;
	TPhysicsPhantomContainer* pPhantom  = m_apQueryHelpers[ 1 ];

	if ( pPhantom != TNULL )
		TPhysicsManager::SetCollisionTestFlags( pPhantom, ( m_fHeightOffset == 0.0f ) ? 0x87 : 0x83 );

	// A flat box (0.2 high) from 1.2 above the bottom of the entity, swept down (y is down)
	TVector4 vHalfExtents = m_pEntity->GetBBox();
	TVector4 vStart       = m_pEntity->GetBasePosition();
	vStart.y              = ( vStart.y + vHalfExtents.y ) - 1.2f;
	vHalfExtents.y        = 0.2f;

	static const TVector4 s_vDown( 0.0f, 1.0f, 0.0f, 1.0f );

	TVector4 vNormal;
	TVector4 avTriangle[ 3 ];

	if ( HASANYFLAG( m_uiFlags, FLAG_GroundCached ) )
	{
		// The cached ground triangle is tried first
		TVector4* apTriangle[ 3 ] = { &m_avGroundTriangle[ 0 ], &m_avGroundTriangle[ 1 ], &m_avGroundTriangle[ 2 ] };
		fDistance                 = a_fTolerance + 1.0f;

		TBOOL bHitCached = TCollision::AABBSweptvsTriangle( vStart, vHalfExtents, s_vDown, apTriangle, fDistance );

		if ( bHitCached )
		{
			vNormal         = m_oGroundContact.vNormal;
			pHit            = m_oGroundContact.pObject;
			avTriangle[ 0 ] = m_avGroundTriangle[ 0 ];
			avTriangle[ 1 ] = m_avGroundTriangle[ 1 ];
			avTriangle[ 2 ] = m_avGroundTriangle[ 2 ];
		}
		else
		{
			m_uiFlags &= ~FLAG_GroundCached;
		}
	}

	if ( !HASANYFLAG( m_uiFlags, FLAG_GroundCached ) )
	{
		TUINT32 uiLayer = ( m_fHeightOffset != 0.0f ) ? 0x83 : 0x87;
		TFLOAT  fStep;

		pHit = ACollisionManager2::GetSingleton()->CollideSweptAABB( vStart, vHalfExtents, s_vDown, fDistance, vNormal, 0.0f, fStep, m_pEntity->GetCollisionObject(), &oFunctor, uiLayer, pPhantom );

		if ( pHit != TNULL && oFunctor.m_bHasTriangle )
		{
			avTriangle[ 0 ] = oFunctor.m_avTriangle[ 0 ];
			avTriangle[ 1 ] = oFunctor.m_avTriangle[ 1 ];
			avTriangle[ 2 ] = oFunctor.m_avTriangle[ 2 ];
		}
	}

	// The distance from the bottom of the entity (the box started 1 above it)
	fDistance           = fDistance - 1.0f;
	TFLOAT fPenetration = fDistance;

	if ( fDistance > -1.0f && fPenetration < 0.0f )
	{
		if ( IsStandableCollisionObject( pHit, TVector4( 0.0f, 0.0f, 0.0f, 1.0f ) ) == TTRUE )
		{
			// In the ground: moved up onto it
			TVector4 vPosition = m_pEntity->GetMatrix().GetTranslation();
			vPosition.y        = vPosition.y + fDistance;
			m_pEntity->SetPosition( vPosition );

			fPenetration = 0.0f;
			fDistance    = 0.0f;
		}
		else
		{
			fPenetration = fDistance;
		}
	}

	if ( pHit == TNULL )
	{
		m_uiFlags &= ~FLAG_GroundCached;
		return TFALSE;
	}

	if ( a_pContact != TNULL )
	{
		a_pContact->vNormal    = vNormal;
		a_pContact->pObject    = pHit;
		a_pContact->fDistance  = ( fPenetration >= 0.0f ) ? fDistance : 0.0f;
		a_pContact->eGroupFlag = oFunctor.m_eGroupFlag;
		TASSERT( a_pContact->fDistance >= 0 );

		a_pContact->uiFlags &= ~( GroundContactInfo::FLAG_HasTriangle | GroundContactInfo::FLAG_InsideEdges );

		if ( oFunctor.m_bHasTriangle )
		{
			TUINT32 uiFlags         = a_pContact->uiFlags | GroundContactInfo::FLAG_HasTriangle;
			a_pContact->uiFlags     = uiFlags;
			const TVector4& rPos    = m_pEntity->GetMatrix().GetTranslation();
			TBOOL           bInside = TTRUE;

			// Is the entity (xz) above the triangle and at least 0.2 away from each edge?
			for ( TINT i = 0; i < 3; i++ )
			{
				const TVector4& rA = avTriangle[ i ];
				const TVector4& rB = avTriangle[ ( i + 1 ) % 3 ];

				TFLOAT fToX   = rPos.x - rA.x;
				TFLOAT fToZ   = rPos.z - rA.z;
				TFLOAT fEdgeX = rB.x - rA.x;
				TFLOAT fEdgeZ = rB.z - rA.z;

				if ( fEdgeZ * fEdgeZ + fEdgeX * fEdgeX <= TMath::TFLOAT_EPSILON )
				{
					bInside = TFALSE;
					break;
				}

				TFLOAT fDot = fEdgeZ * fToZ + fEdgeX * fToX;

				if ( fDot < 0.0f )
				{
					bInside = TFALSE;
					break;
				}

				TFLOAT fEdgeLength = TMath::Sqrt( fEdgeZ * fEdgeZ + fEdgeX * fEdgeX );
				TFLOAT fAlong      = ( 1.0f / fEdgeLength ) * fDot;

				if ( fAlong < 0.0f || fEdgeLength < fAlong )
				{
					bInside = TFALSE;
					break;
				}

				TFLOAT fClosestX = rPos.x - ( fEdgeX * ( 1.0f / fEdgeLength ) * fAlong + rA.x );
				TFLOAT fClosestZ = rPos.z - ( fEdgeZ * ( 1.0f / fEdgeLength ) * fAlong + rA.z );

				if ( fClosestZ * fClosestZ + fClosestX * fClosestX < 0.040000003f )
				{
					bInside = TFALSE;
					break;
				}
			}

			if ( bInside )
				a_pContact->uiFlags = uiFlags | GroundContactInfo::FLAG_InsideEdges;

			a_pContact->uiFlags |= GroundContactInfo::FLAG_HasTriangle;

			// Walkable terrain is cached for the next probes
			if ( HASANYFLAG( m_uiFlags, FLAG_CanCacheGround ) && !HASANYFLAG( m_uiFlags, FLAG_GroundCached ) &&
			     HASANYFLAG( pHit->GetFlags(), ACollisionObject::FLAG_OwnerType0 ) && sm_fMinGroundDot < -vNormal.y )
			{
				m_avGroundTriangle[ 0 ] = avTriangle[ 0 ];
				m_avGroundTriangle[ 1 ] = avTriangle[ 1 ];
				m_avGroundTriangle[ 2 ] = avTriangle[ 2 ];
				m_oGroundContact        = *a_pContact;
				m_uiFlags |= FLAG_GroundCached;
			}
		}
	}

	if ( fDistance < 0.15000001f && !HASANYFLAG( pHit->GetFlags(), ACollisionObject::FLAG_OwnerType0 ) )
		AMoveHelper::AddTouchedObject( &m_oTouchList, pHit, vNormal );

	TASSERT( vNormal.y <= -scm_fMaxSlidableGroundDot );
	return TTRUE;
}

// $deBlob: FUNCTION 007dd6d0
TBOOL AGndMotorComponent::GroundProbeCollideFunctor2::PreFilterSweptAABBPoly( const TVector4& a_rNormal, const TVector4* a_pTriangle )
{
	if ( -a_rNormal.y < scm_fMaxSlidableGroundDot )
		return TTRUE;

	if ( a_pTriangle != TNULL )
	{
		m_bHasTriangle    = TTRUE;
		m_avTriangle[ 0 ] = a_pTriangle[ 0 ];
		m_avTriangle[ 1 ] = a_pTriangle[ 1 ];
		m_avTriangle[ 2 ] = a_pTriangle[ 2 ];
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007dd720
TBOOL AGndMotorComponent::GroundProbeCollideFunctor2::PreFilterSweptSpherePoly( const TVector4& a_rNormal )
{
	if ( -a_rNormal.y < scm_fMaxSlidableGroundDot )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 007dd6a0
TBOOL AGndMotorComponent::GroundProbeCollideFunctor2::PreFilterSpherePoly( const TVector4& a_rNormal )
{
	if ( -a_rNormal.y < scm_fMaxSlidableGroundDot )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 007dd670
TBOOL AGndMotorComponent::GroundProbeCollideFunctor2::PreFilterAABBPoly( const TVector4& a_rNormal )
{
	if ( -a_rNormal.y < scm_fMaxSlidableGroundDot )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 007dd630
TBOOL AGndMotorComponent::GroundProbeCollideFunctor2::HitNonSolidObject( ACollisionObject* a_pObject )
{
	if ( m_pTouches != TNULL )
		AMoveHelper::AddTouchedObject( m_pTouches, a_pObject, TVector4::VEC_ZERO );

	return TFALSE;
}

// $deBlob: FUNCTION 007dd600
void AGndMotorComponent::GroundProbeCollideFunctor2::HitGroup( const TCHAR* a_szGroupName, TUINT32 a_eGroupFlag )
{
	m_eGroupFlag = a_eGroupFlag;
}

// $deBlob: FUNCTION 007da410
const TVector4& AGndMotorComponent::GetTraction() const
{
	if ( TUINT8( m_uiFlags >> 1 ) & 1 )
		return m_vCustomTraction;

	if ( ( ( m_iState == 2 || m_iState == 1 ) && m_fStateTime < 0.03f ) ||
	     ( ( TUINT8( m_pEntity->GetFlags() >> 4 ) & 1 ) && ( m_iState == 4 || m_iState == 3 ) && m_fStateTime < 0.03f ) )
		return *sm_pData->GroundTraction;

	return *sm_pData->AirTraction;
}

// $deBlob: FUNCTION 007da480
const TVector4& AGndMotorComponent::GetFriction() const
{
	if ( TUINT8( m_uiFlags >> 2 ) & 1 )
		return m_vCustomFriction;

	if ( ( m_iState == 2 || m_iState == 1 ) && m_fStateTime < 0.5f )
		return *sm_pData->GroundFriction;

	return *sm_pData->AirFriction;
}

// $deBlob: FUNCTION 007db850
void AGndMotorComponent::UpdateCharacterMovement( TFLOAT a_fDeltaTime )
{
	APathfindingComponent* pPathfinding = TSTATICCAST( APathfindingComponent, m_pEntity->GetPathfinding() );
	pPathfinding->GetObstruction().Reset();
	m_iUnk270 = 3;

	// Without a path to follow the entity faces where it's pushed
	TBOOL bFreeMove;

	if ( pPathfinding->IsEnabled() == TFALSE )
	{
		bFreeMove = m_vAcceleration.MagnitudeSq() < TMath::TFLOAT_EPSILON;
	}
	else
	{
		TINT iNavigatorState = pPathfinding->GetNavigator()->GetState();
		bFreeMove            = !( iNavigatorState == AAINavigator::STATE_Moving || iNavigatorState == AAINavigator::STATE_Finished );
	}

	TINT  iState         = m_iState;
	TBOOL bEntityFlag10  = HASANYFLAG( m_pEntity->GetFlags(), 0x10 );
	TBOOL bOnGround      = ( iState == 2 || iState == 1 ) && m_fStateTime < 0.03f;
	TBOOL bOnSteepGround = ( iState == 4 || iState == 3 ) && m_fStateTime < 0.03f;

	if ( bOnGround || ( bEntityFlag10 && bOnSteepGround ) )
	{
		if ( -TMath::TFLOAT_EPSILON <= m_vUnkAC.y + m_vVelocity.y && ( !bEntityFlag10 || ( iState != 4 && iState != 3 ) ) &&
		     HASANYFLAG( m_uiFlags, FLAG_Unk80 ) )
			m_vVelocity.y = 0.0f;
	}
	else
	{
		m_vAcceleration.y = GetGravity() + m_vAcceleration.y;
	}

	if ( -TMath::TFLOAT_EPSILON <= m_vVelocity.y && ( bOnSteepGround || bOnGround ) )
		m_vUnkAC = TVector4::VEC_ZERO;

	if ( m_fLaunchTime <= 0.0f )
	{
		TVector4 vAcceleration;
		vAcceleration.Multiply( m_vAcceleration, GetTraction() );
		vAcceleration.Multiply( a_fDeltaTime );
		m_vVelocity.x = vAcceleration.x + m_vVelocity.x;
		m_vVelocity.y = m_vVelocity.y + vAcceleration.y;
		m_vVelocity.z = m_vVelocity.z + vAcceleration.z;

		if ( bFreeMove == TTRUE )
		{
			if ( 0.0f < m_vVelocity.MagnitudeSq() )
			{
				// Friction: slows down, never reverses
				TVector4 vFriction = m_vVelocity;
				vFriction.Normalise();

				TVector4 vScaledFriction;
				vScaledFriction.Multiply( GetFriction(), a_fDeltaTime );
				vFriction.Multiply( vScaledFriction );

				if ( TMath::Abs( m_vVelocity.x ) <= TMath::Abs( vFriction.x ) )
				{
					if ( 0.0f < TMath::Abs( vFriction.x ) )
						m_vVelocity.x = 0.0f;
				}
				else
					m_vVelocity.x = m_vVelocity.x - vFriction.x;

				if ( TMath::Abs( m_vVelocity.y ) <= TMath::Abs( vFriction.y ) )
				{
					if ( 0.0f < TMath::Abs( vFriction.y ) )
						m_vVelocity.y = 0.0f;
				}
				else
					m_vVelocity.y = m_vVelocity.y - vFriction.y;

				if ( TMath::Abs( m_vVelocity.z ) <= TMath::Abs( vFriction.z ) )
				{
					if ( 0.0f < TMath::Abs( vFriction.z ) )
						m_vVelocity.z = 0.0f;
				}
				else
					m_vVelocity.z = m_vVelocity.z - vFriction.z;
			}
		}
		else if ( pPathfinding->IsEnabled() == TFALSE )
		{
			m_vFacing = m_vVelocity;
			m_vFacing.NormaliseXZ();
		}

		if ( !HASANYFLAG( m_uiFlags, FLAG_Unk8 ) )
			m_vVelocity.ClampLengthXZ( m_fSpeed );
		else
			m_vVelocity.ClampLength( m_fSpeed );
	}
	else
	{
		m_fLaunchTime = m_fLaunchTime - a_fDeltaTime;

		if ( !bOnGround )
			m_vVelocity.y = GetTraction().y * GetGravity() * a_fDeltaTime + m_vVelocity.y;

		if ( 0.0f < m_fLaunchDrag )
		{
			TVector4 vDrag = m_vVelocity;
			vDrag.Normalise();
			vDrag.Multiply( m_fLaunchDrag * a_fDeltaTime );

			if ( TMath::Abs( vDrag.x ) < TMath::Abs( m_vVelocity.x ) )
				m_vVelocity.x = m_vVelocity.x - vDrag.x;

			if ( TMath::Abs( vDrag.y ) < TMath::Abs( m_vVelocity.y ) )
				m_vVelocity.y = m_vVelocity.y - vDrag.y;

			if ( TMath::Abs( vDrag.z ) < TMath::Abs( m_vVelocity.z ) )
				m_vVelocity.z = m_vVelocity.z - vDrag.z;
		}
	}

	if ( m_fTerminalVelocity < m_vVelocity.y )
		m_vVelocity.y = m_fTerminalVelocity;

	// The entities touched push this one (their reaction force)
	TBOOL bOnPlatform   = TFALSE;
	m_vAcceleration     = TVector4::VEC_ZERO;
	m_bHasReactionForce = TFALSE;
	m_vGroundVelocity   = TVector4::VEC_ZERO;

	TUINT8 uiEntityMask = TUINT8( m_pEntity->GetCollisionGroupAndType() );

	for ( TINT i = 0; i < m_oTouchList.Size(); i++ )
	{
		AMoveHelper::ObjectTouched& rTouch = m_oTouchList[ i ];

		if ( HASANYFLAG( rTouch.pObject->GetFlags(), ACollisionObject::FLAG_OwnerType1 | ACollisionObject::FLAG_OwnerType2 ) )
		{
			TObject* pOwnerObject = TSTATICCAST( TObject, rTouch.pObject->GetOwner() );
			TASSERT( pOwnerObject->IsA( &TGetClass( ABaseEntity ) ) );
			ABaseEntity* pOwner = TDYNAMICCAST( ABaseEntity, pOwnerObject );

			if ( pOwner != m_pReactionEntity )
			{
				if ( ( rTouch.pObject->GetGroupMask() & uiEntityMask ) == 0 )
				{
					m_bHasReactionForce = TTRUE;
					TVector4 vForce     = pOwner->GetReactionForce( &m_vVelocity, m_pEntity );
					m_vGroundVelocity.x += vForce.x;
					m_vGroundVelocity.y += vForce.y;
					m_vGroundVelocity.z += vForce.z;
				}
				else
				{
					// Standing on it
					if ( m_fStateTime <= 0.15f )
					{
						TVector4 vForce = pOwner->GetReactionForce( &m_vVelocity, m_pEntity );
						m_vGroundVelocity.x += vForce.x;
						m_vGroundVelocity.y += vForce.y;
						m_vGroundVelocity.z += vForce.z;

						if ( bFreeMove == TTRUE )
						{
							TVector4 vRotation = pOwner->GetReactionRotation( m_pEntity );
							TQuaternion::RotateVector( m_vFacing, *TREINTERPRETCAST( TQuaternion*, &vRotation ), m_vFacing );
							m_vFacing.NormaliseXZ();
						}
					}

					bOnPlatform = TTRUE;
				}
			}
		}
	}

	if ( m_pReactionEntity != TNULL )
	{
		TVector4 vForce = m_pReactionEntity->GetReactionForce( &m_vVelocity, m_pEntity );
		m_vGroundVelocity.x += vForce.x;
		m_vGroundVelocity.y += vForce.y;
		m_vGroundVelocity.z += vForce.z;

		if ( bFreeMove == TTRUE )
		{
			TVector4 vRotation = m_pReactionEntity->GetReactionRotation( m_pEntity );
			TQuaternion::RotateVector( m_vFacing, *TREINTERPRETCAST( TQuaternion*, &vRotation ), m_vFacing );
			m_vFacing.NormaliseXZ();
		}
	}

	TVector4 vMove( m_vUnkAC.x + m_vVelocity.x, m_vUnkAC.y + m_vVelocity.y, m_vUnkAC.z + m_vVelocity.z, m_vVelocity.w );
	vMove.ClampLengthXZ( m_fMaxSpeed );
	vMove.x = m_vGroundVelocity.x + vMove.x;
	vMove.y = m_vGroundVelocity.y + vMove.y;
	vMove.z = m_vGroundVelocity.z + vMove.z;

	// Resting on the ground: nothing to do
	if ( ( m_iState == 1 && m_fStateTime < 0.03f ) || GetGravity() <= TMath::TFLOAT_EPSILON )
	{
		if ( TMath::Abs( vMove.x ) < TMath::TFLOAT_EPSILON && TMath::Abs( vMove.z ) < TMath::TFLOAT_EPSILON &&
		     TMath::Abs( vMove.y ) < TMath::TFLOAT_EPSILON && m_fLaunchTime <= 0.0f && !HASANYFLAG( m_uiFlags, FLAG_ProbeGround ) &&
		     m_bHasReactionForce == TFALSE && !HASANYFLAG( m_uiFlags, FLAG_Unk400 ) )
		{
			m_vVelocity.y   = 0.0f;
			m_fCurrentSpeed = 0.0f;
			return;
		}
	}

	// The height offset is given back during the move
	if ( m_fHeightOffset != 0.0f )
	{
		m_pEntity->GetCollisionObject()->SetGroupMask( TUINT8( m_uiCollisionFlags ) );

		TVector4 vPosition = m_pEntity->GetMatrix().GetTranslation();
		vPosition.y        = vPosition.y + -m_fHeightOffset;
		m_pEntity->SetPosition( vPosition );
	}

	TVector4 vFrameMove;
	vFrameMove.Multiply( vMove, a_fDeltaTime );
	TASSERT( vFrameMove.IsReasonable() );

	TBOOL    bProject = TMath::TFLOAT_EPSILON < m_vUnkBC.MagnitudeSq();
	TVector4 vNewPosition;
	TVector4 vMoved = AMoveHelper::PerformClampedMove( m_pEntity, vFrameMove, vNewPosition, &m_oTouchList, bProject, m_vUnkBC, TNULL, HASANYFLAG( m_uiFlags, FLAG_Held ), m_apQueryHelpers[ 2 ] );

	if ( vMoved.MagnitudeSq() <= 0.0f )
	{
		if ( m_fHeightOffset != 0.0f )
		{
			m_pEntity->GetCollisionObject()->SetGroupMask( TUINT8( m_uiCollisionFlags ) & 0xEC );

			TVector4 vPosition = m_pEntity->GetMatrix().GetTranslation();
			vPosition.y        = vPosition.y + m_fHeightOffset;
			m_pEntity->SetPosition( vPosition );
		}

		m_fCurrentSpeed = 0.0f;
		m_vVelocity     = TVector4::VEC_ZERO;
	}
	else
	{
		if ( m_fHeightOffset != 0.0f )
		{
			m_pEntity->GetCollisionObject()->SetGroupMask( TUINT8( m_uiCollisionFlags ) & 0xEC );
			vNewPosition.y = vNewPosition.y + m_fHeightOffset;
		}

		const TVector4& rcPosition = m_pEntity->GetMatrix().GetTranslation();
		m_vDesiredFacing.Set( vNewPosition.x - rcPosition.x, vNewPosition.y - rcPosition.y, vNewPosition.z - rcPosition.z, vNewPosition.w );
		m_vDesiredFacing.NormaliseXZ();

		m_pEntity->SetPosition( vNewPosition );

		// The velocity of what was really done
		TVector4 vVelocity;
		vVelocity.Divide( vMoved, a_fDeltaTime );
		m_vVelocity.Set( ( vVelocity.x - m_vGroundVelocity.x ) - m_vUnkAC.x, ( vVelocity.y - m_vGroundVelocity.y ) - m_vUnkAC.y, ( vVelocity.z - m_vGroundVelocity.z ) - m_vUnkAC.z, vVelocity.w );
		m_vVelocity.w   = 1.0f;
		m_fCurrentSpeed = m_vVelocity.MagnitudeXZ();
	}

	// The obstruction of the pathfinding: a wall or an entity in the way
	TBOOL                               bPathfinding = pPathfinding->IsEnabled();
	APathfindingComponent::Obstruction& rObstruction = pPathfinding->GetObstruction();

	for ( TINT i = 0; i < m_oTouchList.Size(); i++ )
	{
		AMoveHelper::ObjectTouched& rTouch  = m_oTouchList[ i ];
		ACollisionObject*           pObject = rTouch.pObject;

		if ( !HASANYFLAG( pObject->GetFlags(), ACollisionObject::FLAG_OwnerType1 | ACollisionObject::FLAG_OwnerType2 ) )
		{
			TASSERT( HASANYFLAG( pObject->GetFlags(), ACollisionObject::FLAG_OwnerType0 ) );

			if ( bPathfinding != TFALSE && -0.64278764f < rTouch.vNormal.y )
			{
				m_iUnk270            = 0;
				rObstruction.pObject = pObject;
				rObstruction.vNormal = rTouch.vNormal;
			}
		}
		else
		{
			TObject* pOwner = TSTATICCAST( TObject, pObject->GetOwner() );

			if ( ( pObject->GetGroupMask() & uiEntityMask ) != 0 && bPathfinding != TFALSE && -0.64278764f < rTouch.vNormal.y )
			{
				m_iUnk270            = 2;
				rObstruction.pObject = pObject;
				rObstruction.vNormal = rTouch.vNormal;
			}

			// Pushed off a platform: looks for the ground again
			if ( !IsOnGround( 0.03f ) && pOwner->IsA( &TGetClass( APropPlatform ) ) )
				UpdateGroundContact( a_fDeltaTime );
		}
	}

	if ( bOnPlatform )
	{
		ResolveAABBCollisions();
	}
}

// The axes ResolveAABBCollisions pushes along
static const TVector4 s_avResolveAxes[ 3 ] = {
	TVector4( 1.0f, 0.0f, 0.0f, 1.0f ),
	TVector4( 0.0f, 1.0f, 0.0f, 1.0f ),
	TVector4( 0.0f, 0.0f, 1.0f, 1.0f ),
};

// Pushes the entity out of an entity it overlaps (on a platform), along the axis of the smallest overlap
// $deBlob: FUNCTION 007dcb50
void AGndMotorComponent::ResolveAABBCollisions()
{
	TVector4 vPosition = m_pEntity->GetBasePosition();
	TVector4 vBBox     = m_pEntity->GetBBox();

	if ( m_fHeightOffset != 0.0f )
	{
		m_pEntity->GetCollisionObject()->SetGroupMask( TUINT8( m_uiCollisionFlags ) );

		// NOTE: retail checks the address of its local position for TNULL here (the other branch, which moves the
		// entity down instead, can never run)
		TVector4* pPosition = &vPosition;

		if ( pPosition != TNULL )
		{
			pPosition->y -= m_fHeightOffset;
		}
		else
		{
			TVector4 vMoved = m_pEntity->GetMatrix().GetTranslation();
			vMoved.y += -m_fHeightOffset;
			m_pEntity->SetPosition( vMoved );
		}
	}

	TVector4          vNormal;
	ACollisionObject* pHit = ACollisionManager2::CollideAABB( vPosition, vBBox, vNormal, m_pEntity->GetCollisionObject(), TNULL, 6 );

	if ( pHit == TNULL || HASANYFLAG( pHit->GetFlags(), ACollisionObject::FLAG_Created ) )
	{
		m_iStuckCount = -1;
		return;
	}

	TObject* pOwnerObject = TSTATICCAST( TObject, pHit->GetOwner() );
	TASSERT( ( pOwnerObject == TNULL ) || pOwnerObject->IsA( &TGetClass( ABaseEntity ) ) );
	ABaseEntity* pOther = TSTATICCAST( ABaseEntity, pOwnerObject );

	TVector4        vOwnPosition   = m_pEntity->GetBasePosition();
	TVector4        vOtherPosition = pOther->GetBasePosition();
	const TVector4& rOtherBBox     = pOther->GetBBox();

	// The smallest overlap of the boxes (its axis and the direction to push in)
	TINT   iAxis = -1;
	TINT   iDir  = 0;
	TFLOAT fBest = TMath::MAXFLOAT;
	TFLOAT fDist;

	fDist = TMath::Abs( ( vBBox.x + vOwnPosition.x ) - ( vOtherPosition.x - rOtherBBox.x ) );
	if ( 0.0f < fDist && fDist < fBest )
	{
		iAxis = 0;
		iDir  = -1;
		fBest = fDist;
	}

	fDist = TMath::Abs( ( vOwnPosition.x - vBBox.x ) - ( vOtherPosition.x + rOtherBBox.x ) );
	if ( 0.0f < fDist && fDist < fBest )
	{
		iAxis = 0;
		iDir  = 1;
		fBest = fDist;
	}

	fDist = TMath::Abs( ( vOwnPosition.y + vBBox.y ) - ( vOtherPosition.y - rOtherBBox.y ) );
	if ( 0.0f < fDist && fDist < fBest )
	{
		iAxis = 1;
		iDir  = -1;
		fBest = fDist;
	}

	fDist = TMath::Abs( ( vOwnPosition.y - vBBox.y ) - ( vOtherPosition.y + rOtherBBox.y ) );
	if ( 0.0f < fDist && fDist < fBest )
	{
		iAxis = 1;
		iDir  = 1;
		fBest = fDist;
	}

	fDist = TMath::Abs( ( vOwnPosition.z + vBBox.z ) - ( vOtherPosition.z - rOtherBBox.z ) );
	if ( 0.0f < fDist && fDist < fBest )
	{
		iAxis = 2;
		iDir  = -1;
		fBest = fDist;
	}

	fDist = TMath::Abs( ( vOwnPosition.z - vBBox.z ) - ( vOtherPosition.z + rOtherBBox.z ) );

	if ( 0.0f < fDist && fDist < fBest )
	{
		iAxis = 2;
		iDir  = 1;
		fBest = fDist;
	}
	else if ( iAxis < 0 )
	{
		return;
	}

	TASSERT( iDir != 0 );

	// Pushes 10% further than the overlap (at least 0.001)
	TVector4 vPush;
	vPush.Multiply( s_avResolveAxes[ iAxis ], TMath::Max( fBest, 0.001f ) );
	vPush.Multiply( 1.1f );
	vPush.Multiply( TFLOAT( iDir ) );

	TVector4 vPushed = m_pEntity->GetBasePosition();
	vPushed.x += vPush.x;
	vPushed.y += vPush.y;
	vPushed.z += vPush.z;

	TVector4 vNewPosition = vPushed;
	vNewPosition.y        = m_pEntity->GetBBox().y + vPushed.y;

	if ( ACollisionManager2::CollideAABB( vPushed, m_pEntity->GetBBox(), vNormal, m_pEntity->GetCollisionObject(), TNULL, 0x87 ) == TNULL )
	{
		m_pEntity->SetPosition( vNewPosition );
		m_iStuckCount = -1;
	}
	else
	{
		m_iStuckCount++;
	}

	if ( m_iStuckCount >= 0 )
		TryUnStick( m_iStuckCount );
}

// The directions and the distances TryUnStick tries
static TCONSTEXPR_VAR TINT s_UNSTICK_COUNT = 16;

static const TVector4 s_avUnStickDirs[ s_UNSTICK_COUNT ] = {
	TVector4( -1.0f, 0.0f, 0.0f, 1.0f ),
	TVector4( 0.0f, 0.0f, -1.0f, 1.0f ),
	TVector4( 1.0f, 0.0f, 0.0f, 1.0f ),
	TVector4( 0.0f, 0.0f, 1.0f, 1.0f ),
	TVector4( -1.0f, 0.0f, -1.0f, 1.0f ),
	TVector4( -1.0f, 0.0f, 1.0f, 1.0f ),
	TVector4( 1.0f, 0.0f, -1.0f, 1.0f ),
	TVector4( 1.0f, 0.0f, 1.0f, 1.0f ),
	TVector4( -1.0f, -1.0f, 0.0f, 1.0f ),
	TVector4( 0.0f, -1.0f, -1.0f, 1.0f ),
	TVector4( 1.0f, -1.0f, 0.0f, 1.0f ),
	TVector4( 0.0f, -1.0f, 1.0f, 1.0f ),
	TVector4( -1.0f, -1.0f, -1.0f, 1.0f ),
	TVector4( -1.0f, -1.0f, 1.0f, 1.0f ),
	TVector4( 1.0f, -1.0f, -1.0f, 1.0f ),
	TVector4( 1.0f, -1.0f, 1.0f, 1.0f ),
};

static const TFLOAT s_afUnStickDists[ 6 ] = { 0.1f, 0.2f, 0.5f, 1.0f, 2.0f, 4.0f };

// Tries the a_riAttempt-th offset (the directions, then the next distance) to get the entity out of what it's stuck
// in; a_riAttempt becomes -1 when the entity is free
// $deBlob: FUNCTION 007dcfe0
void AGndMotorComponent::TryUnStick( TINT& a_riAttempt )
{
	TINT iIndex = TMath::FastMod( a_riAttempt, s_UNSTICK_COUNT );
	TASSERT( iIndex >= 0 && iIndex < s_UNSTICK_COUNT );

	TINT iDist = TMath::FloorToInt( TFLOAT( a_riAttempt ) * 0.0625f );

	if ( iDist >= 6 )
	{
		iDist       = 0;
		a_riAttempt = 0;
		iIndex      = 0;
	}

	TVector4 vOffset;
	vOffset.Multiply( s_avUnStickDirs[ iIndex ], s_afUnStickDists[ iDist ] );

	TVector4 vCentre = m_pEntity->GetBasePosition();
	vCentre.x += vOffset.x;
	vCentre.y += vOffset.y;
	vCentre.z += vOffset.z;

	TVector4 vNewPosition = vCentre;
	vNewPosition.y        = m_pEntity->GetBBox().y + vCentre.y;

	TVector4 vNormal;

	if ( ACollisionManager2::CollideAABB( vCentre, m_pEntity->GetBBox(), vNormal, m_pEntity->GetCollisionObject(), TNULL, 0x87 ) == TNULL )
	{
		m_pEntity->SetPosition( vNewPosition );
		a_riAttempt = -1;
	}
}

// $deBlob: FUNCTION 007dd610
TBOOL EntityContactFunctor::HitNonSolidObject( ACollisionObject* a_pObject )
{
	if ( m_pTouches != TNULL )
		AMoveHelper::AddTouchedObject( m_pTouches, a_pObject, TVector4::VEC_ZERO );

	return TFALSE;
}

// $deBlob: FUNCTION 007dd590
void EntityContactFunctor::HitGroup( const TCHAR* a_szGroupName, TUINT32 a_eGroupFlag )
{
	TASSERT( TStringManager::String8Length( a_szGroupName ) < 64 );
	TStringManager::String8Copy( m_szGroupName, a_szGroupName );
}

// $deBlob: FUNCTION 007da150
void AGndMotorComponent::SetAcceleration( const TVector4& a_rcValue )
{
	m_vAcceleration = a_rcValue;
}

// $deBlob: FUNCTION 007da170
void AGndMotorComponent::SetVelocity( const TVector4& a_rcVelocity )
{
	m_vVelocity   = a_rcVelocity;
	m_vVelocity.w = 1.0f;

	if ( a_rcVelocity.y < -TMath::TFLOAT_EPSILON )
		m_iState = 0;
}

// $deBlob: FUNCTION 007dd140
void AGndMotorComponent::ApplyHeightOffset( TINT a_iMode, TVector4* a_pPosition, TBOOL a_bApplyNext )
{
	if ( m_fHeightOffset == 0.0f )
		return;

	if ( a_iMode == 1 )
		m_pEntity->GetCollisionObject()->SetGroupMask( TUINT8( m_uiCollisionFlags ) );
	else
		m_pEntity->GetCollisionObject()->SetGroupMask( TUINT8( m_uiCollisionFlags ) & 0xEC );

	if ( a_iMode != 0 && a_bApplyNext && m_fNextHeightOffset != -1.0f )
	{
		m_fHeightOffset     = m_fNextHeightOffset;
		m_fNextHeightOffset = -1.0f;
	}

	TFLOAT fOffset = m_fHeightOffset;

	if ( a_iMode == 1 )
		fOffset = -fOffset;

	if ( a_pPosition != TNULL )
	{
		a_pPosition->y += fOffset;
	}
	else
	{
		TVector4 vPosition = m_pEntity->GetMatrix().GetTranslation();
		vPosition.y += fOffset;
		m_pEntity->SetPosition( vPosition );
	}
}

// $deBlob: FUNCTION 007dd230
TBOOL AGndMotorComponent::GetClosestPointOnSegment2D( const TFLOAT* a_pStart, const TFLOAT* a_pEnd, const TFLOAT* a_pPoint, TFLOAT* a_pOut )
{
	TFLOAT fDX = a_pEnd[ 0 ] - a_pStart[ 0 ];
	TFLOAT fDY = a_pEnd[ 1 ] - a_pStart[ 1 ];

	if ( fDY * fDY + fDX * fDX <= TMath::TFLOAT_EPSILON )
		return TFALSE;

	TFLOAT fDot = fDX * ( a_pPoint[ 0 ] - a_pStart[ 0 ] ) + fDY * ( a_pPoint[ 1 ] - a_pStart[ 1 ] );

	if ( fDot < 0.0f )
		return TFALSE;

	TFLOAT fLength    = TFLOAT( sqrt( fDY * fDY + fDX * fDX ) );
	TFLOAT fInvLength = 1.0f / fLength;
	TFLOAT fT         = fInvLength * fDot;

	if ( fT < 0.0f || fLength < fT )
		return TFALSE;

	TFLOAT fY   = fDY * fInvLength * fT + a_pStart[ 1 ];
	a_pOut[ 0 ] = fDX * fInvLength * fT + a_pStart[ 0 ];
	a_pOut[ 1 ] = fY;
	return TTRUE;
}
