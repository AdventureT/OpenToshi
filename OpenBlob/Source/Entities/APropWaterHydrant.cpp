#include "pch.h"
#include "Sound/APositionalSoundManager.h"
#include "APropWaterHydrant.h"
#include "ABlobEntity/ABlobEntity.h"
#include "AEntityModelComponent.h"
#include "AEntityParams.h"
#include "AEntityPhysicsMover.h"
#include <Physics/TCapsuleData.h>
#include "Animation/AModelInstance2.h"
#include "Sound/ASoundInterface.h"
#include "Fx/AParticleEffectHelper.h"
#include "GameInterface/APlayerManager.h"
#include "AEntityIOPacket.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include <Render/TModel.h>

#include <TParticleSystem/TParticleEffectInstance.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffect.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TParticleEffect* APropWaterHydrant::ms_pWaterHydrantEffect;
Toshi::TParticleEffect* APropWaterHydrant::ms_pLeechBotDestroyEffect;

TSTATICASSERT( sizeof( APropWaterHydrant ) == 0xC0 );

// $deBlob: FUNCTION 0077d2e0
// $deBlob: FUNCTION 0077d2f0
// $deBlob: FUNCTION 0077d390
// $deBlob: FUNCTION 0077d430
// $deBlob: FUNCTION 0077d440
// $deBlob: FUNCTION 00540a60
// $deBlob: FUNCTION 00859d10
TDEFINE_CLASS( APropWaterHydrant );

// $deBlob: FUNCTION 0077d450
APropWaterHydrant::APropWaterHydrant()
{
	m_bUsed         = TFALSE;
	m_bSoundPlaying = TFALSE;
	m_pMesh2        = TNULL;
	m_pParticles    = TNULL;
	TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType )
	[1] = 0;
}

// $deBlob: FUNCTION 0077d490
// $deBlob: FUNCTION 0077e0a7
// $deBlob: FUNCTION 0077e0c0
APropWaterHydrant::~APropWaterHydrant()
{
	if ( m_bSoundPlaying )
	{
		m_bSoundPlaying = TFALSE;
		APositionalSoundManager::GetSingleton()->RemoveSound( this, 8, 0x96 );
	}

	if ( m_pParticles )
	{
		m_pParticles->StopInstant();
		m_pParticles = TNULL;
	}

	if ( m_pMesh2 )
	{
		delete m_pMesh2;
		m_pMesh2 = TNULL;
	}
}

// $deBlob: FUNCTION 0077d550
void APropWaterHydrant::Tick( TFLOAT a_fDeltaTime )
{
	AProp::Tick( a_fDeltaTime );

	if ( m_bUsed )
	{
		// The bounding sphere of the model (LOD 0) at the hydrant, scaled by the largest scale
		TModelInstance* pModelInstance = m_pModelComponent->GetModelInstance()->GetModelInstance();
		T2ModelPtr      pModel         = pModelInstance->GetModel();
		TFLOAT          fRadius        = pModel->GetLOD( 0 ).m_RenderVolume.GetRadius();

		const TVector3& rcScale = pModelInstance->GetTransform().GetScale();
		const TFLOAT*   pfScale = &rcScale.x;

		if ( rcScale.y > *pfScale )
			pfScale = &rcScale.y;

		if ( rcScale.z > *pfScale )
			pfScale = &rcScale.z;

		TSphere oSphere( GetTranslation().AsVector3(), *pfScale * fRadius );

		if ( !ARenderer::GetSingleton()->GetViewportManager()->IsSphereVisible( oSphere ) )
		{
			// Out of sight: switched off
			SetVisible( TTRUE );
			SetUsed( TFALSE );
			SetTicking( TFALSE );
		}

		// The blobs near the water are washed
		APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
		TINT            iNumPlayers    = pPlayerManager->GetNumPlayers();

		for ( TINT i = 0; i < iNumPlayers; i++ )
		{
			ABlobEntity* pBlob       = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( i ) );
			TFLOAT       fBlobRadius = ABlobEntity::GetProperties( pBlob->GetSize() )->m_fRadius;
			TFLOAT       fDistance   = TVector4::DistanceXZ( pBlob->GetBodyPosition(), GetTranslation() ) - fBlobRadius;

			if ( fDistance < 3.5f && GetTranslation().y - 7.0f < pBlob->GetBodyPosition().y + fBlobRadius && GetTranslation().y > pBlob->GetBodyPosition().y - fBlobRadius )
				WashBlob( pBlob, a_fDeltaTime );
		}
	}
}

// Switches between the two meshes (the used one is "Mesh2"), plays the water (sound and "Blob_env_WaterHydrant")
// while used
// $deBlob: FUNCTION 0077dd30
void APropWaterHydrant::SetUsed( TBOOL a_bUsed )
{
	if ( m_bUsed != a_bUsed )
	{
		AEntityModelComponent* pMesh = m_pMesh2;
		m_pMesh2                     = m_pModelComponent;
		m_pMesh2->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		m_pModelComponent = pMesh;
		m_pModelComponent->SetTransform( GetMatrix() );
		m_pModelComponent->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );

		if ( a_bUsed )
		{
			ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 8 ), ASoundMap::EVENTID( 0x97 ), GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), FMOD_EVENT_MODE( 0 ) );

			if ( !m_bSoundPlaying )
			{
				m_bSoundPlaying = TTRUE;
				APositionalSoundManager::GetSingleton()->AddEntitySound( this, 8, 0x96 );
			}
		}
		else if ( m_bSoundPlaying )
		{
			m_bSoundPlaying = TFALSE;
			APositionalSoundManager::GetSingleton()->RemoveSound( this, 8, 0x96 );
		}
	}

	TModelInstance* pModelInstance = m_pModelComponent->GetModelInstance()->GetModelInstance();
	pModelInstance->SetForceFade( TTRUE );
	pModelInstance->SetFade( 1.0f );

	pModelInstance = m_pMesh2->GetModelInstance()->GetModelInstance();
	pModelInstance->SetForceFade( TTRUE );
	pModelInstance->SetFade( 1.0f );

	if ( a_bUsed )
	{
		if ( m_pParticles == TNULL )
		{
			m_pParticles = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pWaterHydrantEffect );

			if ( m_pParticles != TNULL )
				m_pParticles->GetTransform() = GetMatrix();
		}
	}
	else if ( m_pParticles != TNULL )
	{
		m_pParticles->StopInstant();
		m_pParticles = TNULL;
	}

	m_bUsed = a_bUsed;
}

// $deBlob: FUNCTION 0077dec0
void APropWaterHydrant::OnLODChanging( TINT a_iLOD )
{
	if ( !( a_iLOD & FLAG_HighLOD ) && ( a_iLOD & FLAG_LowLOD ) && m_bSoundPlaying )
	{
		m_bSoundPlaying = TFALSE;
		APositionalSoundManager::GetSingleton()->RemoveSound( this, 8, 0x96 );
	}

	AVisibleEntity::OnLODChanging( a_iLOD );
}

// $deBlob: FUNCTION 0077dc50
// $deBlob: FUNCTION 0077e13b
void APropWaterHydrant::PostCreate()
{
	AProp::PostCreate();
	SetVisible( TTRUE );
	SetUsed( TFALSE );
	SetTicking( TFALSE );
}

// $deBlob: FUNCTION 0077d920
// $deBlob: FUNCTION 0077e146
void APropWaterHydrant::ProcessParams( const AEntityParams& a_rParams )
{
	AProp::ProcessParams( a_rParams );

	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	m_fAmbientScale                 = 1.0f;
	pModelInstance->SetAmbientScale( 1.0f );

	const TCHAR* szMesh2 = TNULL;
	a_rParams.GetString( szMesh2, "Mesh2" );

	m_pMesh2 = new AEntityModelComponent( this, szMesh2 );
	m_pMesh2->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

	m_pMesh2->GetModelInstance()->SetAmbientScale( m_fAmbientScale );

	const AEntityTypeData::TRBData* pTRBData    = m_pTypeData->m_pTRBData;
	AEntityParam::Collision**       ppCollision = TNULL;

	if ( a_rParams.GetCollision( ppCollision ) )
	{
		CreatePhysics( pTRBData, *ppCollision, TNULL, TFALSE );
		SetPhysicsMover( new AEntityPhysicsMover( this ) );
		return;
	}

	// A capsule (height 3.0, radius 1.5, along z turned upright) centred half its height below the origin
	TCapsuleData oCapsule;
	oCapsule.SetHeight( 3.0f );
	oCapsule.SetRadius( 1.5f );
	TShapeBaseClass* pShape = &oCapsule;

	TMatrix44 matOffset = TMatrix44::IDENTITY;
	matOffset.RotateX( TMath::PI / 2 );

	matOffset.SetTranslation( TVector4( 0.0f, 3.0f * -0.5f, 0.0f, 1.0f ) );

	CreatePhysics( pTRBData, &pShape, &matOffset, 1, TNULL, TFALSE );
	SetPhysicsMover( new AEntityPhysicsMover( this ) );
}

// $deBlob: FUNCTION 0077d750
void APropWaterHydrant::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	// Touched or attacked by Blob: the water runs and washes it
	if ( ( TINT( a_rPacket.m_eMessage ) == 7 || TINT( a_rPacket.m_eMessage ) == 4 ) && ( TUINT8( a_pCaller->GetFlags() >> 4 ) & 1 ) )
	{
		TASSERT( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) );
		ABlobEntity* pBlob = a_pCaller->CastTo<ABlobEntity>();

		if ( !m_bUsed )
		{
			SetTicking( TTRUE );
			SetUsed( TTRUE );
			RemoveFromPhysics();

			TUINT32 uiColour = pBlob->GetColourComponent()->GetCurrentColour();

			if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
				uiColour = ABlobColourComponent::COLOUR_BLACK;

			if ( uiColour != 0 )
			{
				// A splash of Blob's colour
				TUINT32 uiARGB = ABlobColourComponent::ms_auiColours[ uiColour ];
				AParticleEffectHelper::SetColour( ms_pLeechBotDestroyEffect, uiARGB, "Blob_InkPod_Paint" );
				AParticleEffectHelper::SetColour( ms_pLeechBotDestroyEffect, uiARGB, "Blob_InkPod_Spray" );

				TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pLeechBotDestroyEffect );

				if ( pEffect != TNULL )
				{
					TMatrix44 mTransform = TMatrix44::IDENTITY;

					const TVector4& rcTranslation = GetTranslation();
					TASSERT( rcTranslation.w == 1.0f );
					mTransform.SetTranslation( rcTranslation );

					pEffect->GetTransform() = mTransform;
				}
			}
		}

		WashBlob( pBlob, 0.0f );
	}

	AProp::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 0077dc80
void APropWaterHydrant::Reset( ABaseEntity* a_pEntity )
{
	SetVisible( TTRUE );
	SetUsed( TFALSE );
	SetTicking( TFALSE );

	if ( m_bSoundPlaying )
	{
		m_bSoundPlaying = TFALSE;
		APositionalSoundManager::GetSingleton()->RemoveSound( this, 8, 0x96 );
	}

	AProp::Reset( a_pEntity );
}

// NOTE: no function of its own in retail: LoadEntityProperties ends with a jump to this code
void APropWaterHydrant::LoadEffects()
{
	TParticleEffectManager* pEffectManager = TParticleSystem::GetSingleton()->GetEffectManager();

	ms_pWaterHydrantEffect    = pEffectManager->FindEffect( "Blob_env_WaterHydrant" );
	ms_pLeechBotDestroyEffect = pEffectManager->FindEffect( "Blob_LeechBotDestroy" )->Copy();
}

// $deBlob: FUNCTION 0077dc00
void APropWaterHydrant::ClearEffects()
{
	ms_pWaterHydrantEffect = TNULL;

	if ( ms_pLeechBotDestroyEffect != TNULL )
	{
		delete ms_pLeechBotDestroyEffect;
		ms_pLeechBotDestroyEffect = TNULL;
	}
}

// The loop sound of the hydrant
// $deBlob: FUNCTION 0077df10
void APropWaterHydrant::StartLoopSound()
{
	if ( !m_bSoundPlaying )
	{
		m_bSoundPlaying = TTRUE;
		APositionalSoundManager::GetSingleton()->AddEntitySound( this, 8, 0x96 );
	}
}

// $deBlob: FUNCTION 0077df40
void APropWaterHydrant::StopLoopSound()
{
	if ( m_bSoundPlaying )
	{
		m_bSoundPlaying = TFALSE;
		APositionalSoundManager::GetSingleton()->RemoveSound( this, 8, 0x96 );
	}
}

// $deBlob: FUNCTION 0077df70
void APropWaterHydrant::WashBlob( ABlobEntity* a_pBlob, TFLOAT a_fDeltaTime )
{
	if ( a_pBlob->IsEvilBlob() || ( TUINT8( a_pBlob->m_uiFlags >> 21 ) & 1 ) || ( TUINT8( a_pBlob->m_uiFlags >> 10 ) & 1 ) )
		return;

	a_pBlob->m_uiFlags |= 0x400;
	a_pBlob->WashOff();

	TFLOAT&                        rfDrainTime     = *TREINTERPRETCAST( TFLOAT*, &a_pBlob->m_auiUnk5A8[ 2 ] ); // 0x5B0
	TFLOAT&                        rfDrainInterval = *TREINTERPRETCAST( TFLOAT*, &a_pBlob->m_auiUnk5A8[ 3 ] ); // 0x5B4
	ABlobEntity::CommonProperties* pProperties     = ABlobEntity::ms_pBlobCommonProperties;

	if ( pProperties->m_uiWaterPoints < TUINT32( a_pBlob->m_iPaintPoints ) )
	{
		rfDrainTime += a_fDeltaTime;

		if ( rfDrainInterval <= rfDrainTime && rfDrainTime != rfDrainInterval )
		{
			a_pBlob->RemovePaintPoints( 1, TFALSE );

			// The first drain in the water waits m_fFirstDrainDelay, then the interval shrinks
			if ( !( TUINT8( a_pBlob->m_uiPrevFlags >> 10 ) & 1 ) )
			{
				rfDrainInterval = pProperties->m_fFirstDrainDelay;
				rfDrainTime     = 0.0f;
				return;
			}

			TFLOAT fInterval = rfDrainInterval - pProperties->m_fDrainIntervalStep;
			rfDrainInterval  = TMath::Max( pProperties->m_fMinDrainInterval, fInterval );
			rfDrainTime      = 0.0f;
		}
	}
	else if ( TUINT32( a_pBlob->m_iPaintPoints ) < pProperties->m_uiWaterPoints )
	{
		rfDrainTime += a_fDeltaTime;

		if ( rfDrainInterval <= rfDrainTime && rfDrainTime != rfDrainInterval )
		{
			a_pBlob->AddPaintPoints( 1 );
			rfDrainInterval = 0.1f;
			rfDrainTime     = 0.0f;
		}
	}
}
