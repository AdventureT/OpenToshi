#include "pch.h"
#include "APickupManager.h"
#include "APickup.h"
#include "AEntityHash.h"
#include "GameController/AGameStatus.h"
#include "APooledEntity.h"
#include "APooledEntityManager.h"
#include "ABlobEntity/ABlobEntity.h"
#include "GameInterface/APlayerManager.h"
#include "Animation/AModelInstance2.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include <Physics/Havok/TPhysicsManager.h>
#include <Math/TMath.h>

#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TParticleEffect* APickupManager::m_pParticleEffects[ PICKUPEFFECT_COUNT ];

TSTATICASSERT( sizeof( APickupManager ) == 0x214 );

const TCHAR* APickupManager::ms_aszEffectNames[ PICKUPEFFECT_COUNT ] = {
	"Blob_Pickup_Life",
	"Blob_Pickup_Pattern",
	"Blob_Pickup_Time",
	"Blob_Pickup_Energy",
	"Blob_Pickup_TagA",
	"Blob_Pickup_TagB",
	"Blob_Pickup_TagC",
	"Blob_Pickup_TagD",
	"Blob_Pickup_TagE",
	"Blob_Pickup_TagF",
	"Blob_Pickup_TagG",
	"Blob_Pickup_TagH",
};

// $deBlob: FUNCTION 0075b770
APickupManager::APickupManager()
{
	TFLOAT fAngle = 0.0f;

	for ( TINT i = 0; i < ROTATION_COUNT; i++ )
	{
		m_aRotations[ i ] = TMatrix44::IDENTITY;
		m_aRotations[ i ].RotateY( fAngle );
		m_aReversedRotations[ i ] = TMatrix44::IDENTITY;
		m_aReversedRotations[ i ].RotateY( fAngle );
		fAngle += TMath::TWO_PI * 0.25f;
	}

	m_iNumPatternPickups = 0;
	m_iNumTagPickups     = 0;
}

// $deBlob: FUNCTION 0075b8f0
// $deBlob: FUNCTION 0075c550
APickupManager::~APickupManager()
{
	TASSERT( m_lPickupList.IsEmpty() );
	m_lPickupList.Clear();
}

// $deBlob: FUNCTION 0075b9e0
void APickupManager::AddPickup( APickup& a_rPickup )
{
	TASSERT( !a_rPickup.GetListNode().IsLinked() );
	a_rPickup.GetListNode().InsertBefore( m_lPickupList.End() );

	if ( a_rPickup.GetPickupType() == 3 )
		m_iNumPatternPickups++;
	else if ( a_rPickup.GetPickupType() == 1 )
		m_iNumTagPickups++;
}

// $deBlob: FUNCTION 0075ba60
void APickupManager::RemovePickup( APickup& a_rPickup )
{
	TASSERT( m_lPickupList.IsInList( &a_rPickup.GetListNode() ) );

	if ( a_rPickup.GetPickupType() == 3 )
		m_iNumPatternPickups--;
	else if ( a_rPickup.GetPickupType() == 1 )
		m_iNumTagPickups--;

	a_rPickup.GetListNode().Remove();
}

// $deBlob: FUNCTION 0075baf0
APickup* APickupManager::CreatePickup( TINT a_eType, const TVector4& a_rPosition, TFLOAT a_fHiddenTime, TFLOAT a_fLifeTime )
{
	TINT eMode = AGameStatus::GetSingleton()->GetLevelMode();

	if ( eMode != AGameStatus::Mode_Story && eMode != -1 )
		return TNULL;

	static const TVector4 s_vFacing( 1.0f, 0.0f, 0.0f, 1.0f );

	APooledEntity* pPooled = APooledEntityManager::GetSingleton()->Acquire( a_eType, a_rPosition, s_vFacing, 0 );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail doesn't check for an empty pool
	if ( pPooled == TNULL )
		return TNULL;
#endif

	APickup* pPickup = TSTATICCAST( APickup, pPooled->m_pEntity );
	pPickup->SetPooled( pPooled );
	pPickup->Spawn( a_rPosition, a_fHiddenTime, a_fLifeTime );

	return pPickup;
}

// Spawns a pickup of a_eType 8 away from the player's blob, behind it or at its sides (the first free spot with ground
// below it), else on the ground under the blob
// $deBlob: FUNCTION 0075bb60
APickup* APickupManager::SpawnPickupNearPlayer( TINT a_eType, TINT a_iPlayer )
{
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( a_iPlayer ) ) );

	TMatrix44 oTransform;
	pBlob->GetAnimationComponent()->GetBallEarsModelInstance()->GetTransform( oTransform );

	TVector4 vPosition = pBlob->GetBasePosition();
	TVector4 vBack( -oTransform.m_f31, -oTransform.m_f32, -oTransform.m_f33, oTransform.m_f34 );
	vBack.Normalise();

	TPhysicsManager*     pPM = TPhysicsManager::GetSingleton();
	TPhysicsShapeContact oContact;
	TPhysicsRay          oRay;
	oRay.m_uiCollisionLayer = 0xE3;

	TFLOAT fHiddenTime = -1.0f;
	TBOOL  bFound      = TFALSE;

	for ( TINT i = 0; i < 4; i++ )
	{
		TVector4 vDirection = vBack;
		vDirection.RotateY( TFLOAT( i ) * 1.5707964f );

		TVector4 vOffset;
		vOffset.Multiply( vDirection, 8.0f );

		TVector4 vSpot( vPosition.x + vOffset.x, vPosition.y + vOffset.y, vPosition.z + vOffset.z, vPosition.w );

		// Nothing between the blob and the spot...
		oRay.m_vFrom = vPosition;
		oRay.m_vTo   = vSpot;

		if ( !pPM->CollideRay( oRay, oContact, TNULL ) )
		{
			// ...and ground below it (+Y is down)
			oRay.m_vFrom = TVector4( vSpot.x, vSpot.y - 1.0f, vSpot.z, vSpot.w );
			oRay.m_vTo   = TVector4( vSpot.x, vSpot.y - 1.0f + 100.0f, vSpot.z, vSpot.w );

			if ( pPM->CollideRay( oRay, oContact, TNULL ) )
			{
				vPosition = oContact.m_vPosition;
				bFound    = TTRUE;
				break;
			}
		}
	}

	if ( !bFound )
	{
		// The ground under the blob, or the blob's position
		fHiddenTime = 1.0f;

		TVector4 vBase = pBlob->GetBasePosition();
		oRay.m_vFrom   = TVector4( vBase.x, vBase.y - 1.0f, vBase.z, vBase.w );
		oRay.m_vTo     = TVector4( vBase.x, vBase.y - 1.0f + 100.0f, vBase.z, vBase.w );

		if ( pPM->CollideRay( oRay, oContact, TNULL ) )
			vPosition = oContact.m_vPosition;
		else
			vPosition = pBlob->GetBasePosition();
	}

	return CreatePickup( a_eType, vPosition, fHiddenTime, 30.0f );
}

// $deBlob: FUNCTION 0075be80
void APickupManager::UpdateRotations( TFLOAT a_fDeltaTime )
{
	for ( TINT i = ROTATION_COUNT - 1; i >= 0; i-- )
	{
		m_aRotations[ i ].RotateY( a_fDeltaTime );
		m_aReversedRotations[ i ].RotateY( -a_fDeltaTime );
	}

	// Each player's Blob collects the pickups its body's sphere touches (the blob's sphere is 1 larger than its body)
	if ( APlayerManager::GetSingleton()->GetNumPlayers() != 0 )
	{
		for ( TINT iPlayer = 3; iPlayer >= 0; iPlayer-- )
		{
			ABaseEntity* pBlobEntity = APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( iPlayer ) );

			if ( pBlobEntity == TNULL )
				continue;

			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pBlobEntity );

			TVector4 vPosition = pBlob->GetBodyPosition();
			TFLOAT   fRadius   = ABlobEntity::GetProperties( pBlob->GetSize() )->m_fRadius + 1.0f;

			TINT          iCount  = 0;
			ABaseEntity** ppFound = AEntityHash::GetSingleton()->Query( vPosition, fRadius + 10.0f, &iCount, TNULL, TTRUE, 500, TFALSE, TFALSE, TTRUE );

			for ( TINT i = 0; i < iCount; i++ )
			{
				ABaseEntity* pEntity = ppFound[ i ];

				TASSERT( ( pEntity->GetFlags() & ABaseEntity::FLAG_Pickup ) != 0 );
				TASSERT( pEntity->IsA( &TGetClass( APickup ) ) );

				APickup* pPickup = TSTATICCAST( APickup, pEntity );

				// Idle or flashing, and the sphere reaches the pickup
				APickup::PICKUPSTATE eState = pPickup->GetPickupState();

				if ( ( eState == APickup::PICKUPSTATE_Idle || eState == APickup::PICKUPSTATE_Flashing ) &&
				     TMath::SphereVsSphere( vPosition, fRadius, pPickup->GetTranslation(), pPickup->GetBoundingSphereRadius() ) )
				{
					pPickup->OnCollected( pBlobEntity );
				}
			}
		}
	}
}

// $deBlob: FUNCTION 0075c0b0
void APickupManager::GetPickupRotation( TMatrix44& a_rOutMatrix, TINT a_RotationIndex, TBOOL a_bReversed )
{
	TASSERT( a_RotationIndex >= 0 && a_RotationIndex < ROTATION_COUNT );

	const TMatrix44& rRotation = a_bReversed ? m_aReversedRotations[ a_RotationIndex ] : m_aRotations[ a_RotationIndex ];
	a_rOutMatrix.SetBasisVector4( BASISVECTOR_RIGHT, rRotation.AsBasisVector4( BASISVECTOR_RIGHT ) );
	a_rOutMatrix.SetBasisVector4( BASISVECTOR_UP, rRotation.AsBasisVector4( BASISVECTOR_UP ) );
	a_rOutMatrix.SetBasisVector4( BASISVECTOR_FORWARD, rRotation.AsBasisVector4( BASISVECTOR_FORWARD ) );
}

// $deBlob: FUNCTION 0075c1f0
const TMatrix44* APickupManager::GetPickupRotation( TINT a_RotationIndex, TBOOL a_bReversed )
{
	TASSERT( a_RotationIndex >= 0 && a_RotationIndex < ROTATION_COUNT );
	return a_bReversed ? &m_aReversedRotations[ a_RotationIndex ] : &m_aRotations[ a_RotationIndex ];
}

// $deBlob: FUNCTION 0075c250
void APickupManager::LoadEffects()
{
	for ( TINT i = 0; i < PICKUPEFFECT_COUNT; i++ )
	{
		TASSERT( TNULL == m_pParticleEffects[ i ] );
		m_pParticleEffects[ i ] = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( ms_aszEffectNames[ i ] );
	}
}

// $deBlob: FUNCTION 0075c2c0
void APickupManager::ClearEffects()
{
	for ( TINT i = 0; i < PICKUPEFFECT_COUNT; i++ )
		m_pParticleEffects[ i ] = TNULL;
}

// $deBlob: FUNCTION 0075c2e0
void APickupManager::StartEffect( TINT a_iEffect, const TVector4& a_rPosition )
{
	TASSERT( a_iEffect >= 0 && a_iEffect < PICKUPEFFECT_COUNT );
	TParticleEffectInstance* pInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pParticleEffects[ a_iEffect ] );

	if ( pInstance != TNULL )
	{
		pInstance->GetTransform() = TMatrix44::IDENTITY;
		pInstance->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, a_rPosition );
	}
}

// $deBlob: FUNCTION 0075c390
void APickupManager::CheckEmpty()
{
	TASSERT( m_lPickupList.IsEmpty() );
	TASSERT( m_iNumPatternPickups == 0 );
	TASSERT( m_iNumTagPickups == 0 );
}
