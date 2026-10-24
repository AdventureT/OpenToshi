#include "pch.h"
#include "APickupableComponent.h"
#include "ABaseEntity.h"
#include "AProp.h"
#include "AEntityData.h"
#include "AEntityHash.h"
#include "AEntityIOPacket.h"
#include "AAttackMessage.h"
#include "Collision/ACollisionObject.h"
#include "Collision/ACollisionManager2.h"

#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TFLOAT APickupableComponent::scm_fMaxThrownTime = 4.0f;

APickupableComponent::Data* APickupableComponent::sm_pData = TNULL;

// $deBlob: FUNCTION 007dec60
void APickupableComponent::LoadData()
{
	sm_pData = TSTATICCAST( Data, AEntityData::GetSingleton()->GetEntityData( "GlobalData_APickupableComponentData_EntityData" ) );
}

// $deBlob: FUNCTION 007dec80
void APickupableComponent::ReloadData()
{
	sm_pData = TNULL;
	sm_pData = TSTATICCAST( Data, AEntityData::GetSingleton()->GetEntityData( "GlobalData_APickupableComponentData_EntityData" ) );
}

// $deBlob: FUNCTION 007deca0
void APickupableComponent::UnloadData()
{
	sm_pData = TNULL;
}

// $deBlob: FUNCTION 007deb80
APickupableComponent::APickupableComponent( ABaseEntity* a_pOwner, TUINT a_uiAttackDamage, TUINT a_uiAttackUnk2, TFLOAT a_fDamageRadius )
    : AEntityComponent2( a_pOwner )
{
	// NOTE: retail leaves m_qOrientation uninitialised (TQuaternion's constructor sets the identity here)
	m_eState         = STATE_Idle;
	m_pThrower       = TNULL;
	m_vVelocity      = TVector4::VEC_ZERO;
	m_uiAttackDamage = a_uiAttackDamage;
	m_fThrownTime    = 0.0f;
	m_uiAttackUnk2   = a_uiAttackUnk2;
	m_fDamageRadius  = a_fDamageRadius;

	TASSERT( a_pOwner->IsA( &TGetClass( AProp ) ) );
}

// $deBlob: FUNCTION 007df200
// $deBlob: FUNCTION 007dec50
APickupableComponent::~APickupableComponent()
{
}

// $deBlob: FUNCTION 007decb0
void APickupableComponent::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_eState == STATE_Thrown )
	{
		UpdatePosition( a_fDeltaTime );
		UpdateOrientation( a_fDeltaTime );

		m_fThrownTime += a_fDeltaTime;

		if ( m_fThrownTime > scm_fMaxThrownTime )
		{
			m_eState = STATE_Dropped;
			m_pEntity->GetCollisionObject()->SetGroupMask( 0 );
		}
	}
}

// $deBlob: FUNCTION 007ded10
void APickupableComponent::ResetComponent()
{
	m_eState      = STATE_Idle;
	m_pThrower    = TNULL;
	m_fThrownTime = 0.0f;
}

// $deBlob: FUNCTION 007ded30
void APickupableComponent::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_PLAYERINTERACT:
			// Picked up: the picker gets MESSAGE_PICKEDUP back
			if ( m_eState == STATE_Idle )
			{
				AEntityIOPacket oReply;
				oReply.m_eMessage = AEntityIOPacket::MESSAGE_PICKEDUP;
				oReply.m_iData    = 0;
				a_pCaller->HandleEntityIO( m_pEntity, oReply );

				m_pThrower = a_pCaller;
				m_eState   = STATE_PickedUp;
				m_pEntity->GetCollisionObject()->SetGroupMask( 0 );
			}
			break;
		case AEntityIOPacket::MESSAGE_DEATHZONE:
			m_eState = STATE_Dropped;
			break;
		case AEntityIOPacket::MESSAGE_THROW:
		case AEntityIOPacket::MESSAGE_THROW2:
		{
			TASSERT( STATE_PickedUp == m_eState );

			const ThrowData* pThrow = TREINTERPRETCAST( const ThrowData*, a_rPacket.m_iData );

			if ( pThrow->bThrow )
			{
				TMatrix44::MatToQuat( m_qOrientation, m_pEntity->GetMatrix() );
				m_eState = STATE_Thrown;

				// The direction is pitched up by the throw angle
				TVector4 vDirection = pThrow->vDirection;
				TVector4 vAxis;
				vAxis.CrossProduct( TVector4::VEC_POSY, vDirection );
				vAxis.Normalise();

				TVector4 vRotated;
				TVector4::RotateArbitaryVector( vRotated, vDirection, vAxis, sm_pData->fThrowAngle );
				vDirection = vRotated;
				vDirection.Normalise();

				TVector4 vVelocity;
				vVelocity.Multiply( vDirection, sm_pData->fThrowSpeed );
				m_vVelocity = vVelocity;

				m_pEntity->GetCollisionObject()->SetGroupMask( 0xEB );
			}
			else
			{
				m_eState = STATE_Dropped;
			}
			break;
		}
	}
}

// $deBlob: FUNCTION 007deec0
void APickupableComponent::UpdatePosition( TFLOAT a_fDeltaTime )
{
	TVector4 vOldPosition = m_pEntity->GetTranslation();
	TVector4 vBase        = m_pEntity->GetBasePosition();

	m_vVelocity.y += sm_pData->fGravity * a_fDeltaTime;

	TVector4 vMove;
	vMove.Multiply( m_vVelocity, a_fDeltaTime );

	TFLOAT   fDistance = vMove.Magnitude();
	TVector4 vDirection;
	vDirection.Divide( vMove, fDistance );

	ACollisionObject*        pCollision = m_pEntity->GetCollisionObject();
	HittableCollisionFunctor oFunctor( m_pThrower );

	if ( pCollision->GetFlags() & ACollisionObject::FLAG_AABBDirty )
		pCollision->UpdateAABBExtents();

	TVector4          vNormal;
	TFLOAT            fStep;
	ACollisionObject* pHit = ACollisionManager2::CollideSweptSphere( vBase, 1.0f, vDirection, fDistance, vNormal, 0.0f, fStep, pCollision, &oFunctor, 0x85 );

	TVector4 vMoved;
	vMoved.Multiply( vDirection, fDistance );

	TVector4 vNewPosition( vOldPosition.x + vMoved.x, vOldPosition.y + vMoved.y, vOldPosition.z + vMoved.z, vOldPosition.w );
	m_pEntity->SetPosition( vNewPosition );

	if ( pHit != TNULL )
	{
		if ( m_fDamageRadius > 0.0f )
			DamageNearbyEntities( vDirection );

		m_eState = STATE_Dropped;
		m_pEntity->GetCollisionObject()->SetGroupMask( 0 );
	}
}

// $deBlob: FUNCTION 007df040
void APickupableComponent::UpdateOrientation( TFLOAT a_fDeltaTime )
{
	// Spins around the horizontal axis perpendicular to the velocity
	TVector4 vAxis;
	vAxis.CrossProduct( TVector4::VEC_NEGY, m_vVelocity );
	vAxis.Normalise();

	m_qOrientation.RotateAroundAxis( vAxis.AsVector3(), sm_pData->fSpinSpeed * a_fDeltaTime );

	TMatrix44 oMatrix;
	oMatrix.SetFromQuaternion( m_qOrientation );
	oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = m_pEntity->GetTranslation();

	m_pEntity->SetMatrix( oMatrix );
}

// $deBlob: FUNCTION 007df0b0
void APickupableComponent::DamageNearbyEntities( Toshi::TVector4 a_vDirection )
{
	AAttackMessage  oAttack( AAttackMessage::ATTACKTYPE( 6 ), m_pEntity->GetTranslation(), m_uiAttackDamage, m_uiAttackUnk2, a_vDirection, TTRUE );
	AEntityIOPacket oPacket;
	oPacket.m_eMessage = AEntityIOPacket::MESSAGE_PROPATTACK;
	oPacket.m_iData    = TINT( TREINTERPRETCAST( TUINTPTR, &oAttack ) );

	ACollisionObject* pCollision = m_pEntity->GetCollisionObject();

	if ( pCollision->GetFlags() & ACollisionObject::FLAG_AABBDirty )
		pCollision->UpdateAABBExtents();

	TFLOAT fRadius = ( 1.0f <= m_fDamageRadius ? m_fDamageRadius : 1.0f ) + 0.5f;

	TINT          iNumEntities;
	ABaseEntity** ppEntities = AEntityHash::GetSingleton()->Query( m_pEntity->GetBasePosition(), fRadius, &iNumEntities, TNULL, TTRUE, 500, TFALSE, TTRUE, TFALSE );

	for ( TUINT i = 0; i < TUINT( iNumEntities ); i++ )
	{
		ABaseEntity* pEntity = ppEntities[ i ];

		if ( m_pEntity != pEntity && pEntity->GetCollisionObject() != TNULL )
		{
			TVector4 vSphere = m_pEntity->GetBasePosition();
			vSphere.w        = fRadius;

			TFLOAT   fDistance;
			TVector4 vNormal;

			if ( pEntity->GetCollisionObject()->CollideSphere( vSphere, fDistance, vNormal, 0xFF, TNULL ) )
				pEntity->HandleEntityIO( m_pEntity, oPacket );
		}
	}
}

// $deBlob: FUNCTION 007df240
TBOOL HittableCollisionFunctor::HitIgnorableObject( ACollisionObject* a_pObject )
{
	if ( ( a_pObject->GetFlags() & ( ACollisionObject::FLAG_OwnerType1 | ACollisionObject::FLAG_OwnerType2 ) ) == 0 )
		return a_pObject == m_pThrower->GetCollisionObject();

	ABaseEntity* pOwner = TSTATICCAST( TObject, a_pObject->GetOwner() )->CastTo<ABaseEntity>();
	return HASANYFLAG( pOwner->GetFlags(), ABaseEntity::FLAG_Player ) || a_pObject == m_pThrower->GetCollisionObject();
}
