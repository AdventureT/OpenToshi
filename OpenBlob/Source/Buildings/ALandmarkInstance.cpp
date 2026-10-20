#include "pch.h"
#include "ALandmarkInstance.h"
#include "ParticleSystem/AEnvParticles.h"
#include "ABuildingManager.h"
#include "Entities/ABlobEntity/ABlobZJumpComponent.h"
#include "Animation/AModel2.h"
#include "Animation/AModelFactory.h"
#include "Animation/AToshiAnimInterface2.h"
#include "Cameras/ACameraManager.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include "Challenges/AChallengeManager.h"
#include "Challenges/ALandmarkChallenge.h"
#include "XUI/InGame/HUD/ABlobChallengeStatusHud.h"
#include "Collision/ACollisionInstance2.h"
#include "Entities/AFXBreakpointComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ABlobEntity/ABlobColourMaps.h"
#include "GameController/AGameStatus.h"
#include "Entities/ACiv.h"
#include "GameInterface/APlayerManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "Render/Lighting/ALightingData.h"
#include "Render/Material/AMaterialInfo.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include "Sound/AMusicManager.h"
#include "Terrain2/ATerrain2Interface.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "XUI/InGame/HUD/ABlobSizeHud.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <Shaders/WorldShader/TWorldShader.h>
#include <Render/TMesh.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include <TParticleSystem/TParticleSystem.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIScene.h>
#include <XUI/TXUIFigure.h>
#include <XUI/TXUIVisual.h>
#include <XUI/TXUITimelineControl.h>
#include <Render/TRenderContext.h>
#include <Math/TMathInline.h>

// Note: Should be the last include!
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TParticleEffect* ALandmarkInstance::ms_pRingsEffect;
Toshi::TParticleEffect* ALandmarkInstance::ms_pGraphicEffect;
Toshi::TParticleEffect* ALandmarkInstance::ms_pTargetEffect;

static TCONSTEXPR_VAR TFLOAT s_cfFadeInTime   = 0.5f;
static TCONSTEXPR_VAR TFLOAT s_cfFadeOutTime  = 1.5f;
static TCONSTEXPR_VAR TFLOAT s_cfFadeHoldTime = 1.0f;
static TCONSTEXPR_VAR TFLOAT s_cfBounceTime   = 0.55f; // the scale bounce of a completed landmark

// $deBlob: FUNCTION 0053b710
static TFLOAT s_fFadeTime = s_cfFadeOutTime + s_cfFadeInTime + s_cfFadeHoldTime;

// The cell instance was added to the world (flag 0x1000000)
static TBOOL IsInstanceInWorld( const TerrainTypes::CellInstance* a_pInstance )
{
	return ( a_pInstance->m_uiFlags & 0x1000000 ) != 0;
}

static ACollisionInstance2* GetCollision( AModelInstance2* a_pInstance )
{
	return TSTATICCAST( ACollisionInstance2, a_pInstance->GetCollisionInstance() );
}

static TPhysicsObjectContainer* GetBody( ACollisionInstance2* a_pCollision, TINT a_iIndex )
{
	return a_pCollision->GetPhysicsInfo( a_iIndex ).m_pBody;
}

static TUINT32 GetBodyLayer( TPhysicsObjectContainer* a_pBody )
{
	return a_pBody ? ( a_pBody->m_uiCollisionFilterInfo & 0xFFF ) : 0;
}

// Blob's colour (the ink colours count as ink)
static TUINT32 GetBlobColour( ABlobEntity* a_pBlob )
{
	TUINT32 uiColour = a_pBlob->GetPaintColour();

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
		uiColour = ABlobColourComponent::COLOUR_BLACK;

	return uiColour;
}

static void SetFigureColour( TXUIFigure* a_pFigure, TUINT32 a_uiColour )
{
	a_pFigure->SetFillColour( a_uiColour );
}

static TBOOL IsHudIdle()
{
	TXUI* pXUI = TXUI::GetSingleton();

	return pXUI->GetActiveScene() != TNULL && pXUI->GetActiveScene()->IsA( &TGetClass( ABlobHud ) ) &&
	    pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked();
}

//-----------------------------------------------------------------------------
// ALandmarkInstance::Trigger
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 005800a0
ALandmarkInstance::Trigger::Trigger()
    : m_oMatrix( TMatrix44::IDENTITY )
    , m_oCollisionHandler( this )
{
	m_iBody              = -1;
	m_uiBodyLayer        = 0;
	m_bPainted           = TFALSE;
	m_oModel.m_pOwner    = TNULL;
	m_oModel.m_pInstance = TNULL;
	m_pLandmark          = TNULL;
	m_pScene             = TNULL;
	m_iColour            = 4;
	m_pEmitter           = TNULL;
	m_pDropletLow        = TNULL;
	m_pDropletHigh       = TNULL;

	// NOTE: retail leaves m_iPaintLeft, m_wszPaintLeft and the text presenters uninitialised
	m_iPaintLeft        = 0;
	m_wszPaintLeft[ 0 ] = L'\0';
	m_pTextPresenter1   = TNULL;
	m_pTextPresenter2   = TNULL;
}

// $deBlob: FUNCTION 00580170
// $deBlob: FUNCTION 0057bfc0
ALandmarkInstance::Trigger::~Trigger()
{
	GetCollision( m_oModel.m_pInstance )->RemoveFromWorld( 0x60 );
	AModelFactory::GetSingleton()->DestroyInstance( m_oModel.m_pInstance );

	if ( m_pEmitter )
	{
		AEnvParticles::GetSingleton()->RemoveEmitter( m_pEmitter, TTRUE );
		m_pEmitter = TNULL;
	}
}

// $deBlob: FUNCTION 0057ba70
void ALandmarkInstance::Trigger::CollisionHandler::OnCollisionConfirmed( TPhysicsCollisionInfo* a_pInfo )
{
	TPhysicsObjectContainer* pOther = a_pInfo->m_other.m_pBody;

	if ( !( GetBodyLayer( pOther ) & 4 ) || pOther == TNULL || pOther->m_pUserData == TNULL )
		return;

	ABaseEntity* pEntity = TSTATICCAST( ABaseEntity, TSTATICCAST( ACollisionInstance2::OwnerInfo, pOther->m_pUserData )->m_pOwner );
	TASSERT( pEntity->IsA( &TGetClass( ABlobEntity ) ) );

	m_pTrigger->OnBlobContact( TSTATICCAST( ABlobEntity, pEntity ) );
}

// $deBlob: FUNCTION 0057bb00
void ALandmarkInstance::Trigger::OnBlobContact( ABlobEntity* a_pBlob )
{
	if ( m_bPainted || a_pBlob->m_iPaintPoints == 0 )
		return;

	if ( ( m_pLandmark->m_uiFlags & Flags_TriggersHidden ) || m_pLandmark->m_fBodyTimer > 0.0f || a_pBlob->m_eState == ABlobEntity::STATE_LMARK )
		return;

	TUINT32 uiColour = GetBlobColour( a_pBlob );

	if ( m_iColour == 0 )
	{
		// Any colour but white and ink
		if ( uiColour - 1 > 0xE )
			return;
	}
	else if ( uiColour != TUINT32( m_iColour ) && !( a_pBlob->m_uiFlags & 0x200000 ) )
	{
		return;
	}

	a_pBlob->m_pZJump->OnLanded();

	TVector4 vFacing( -m_oMatrix.m_f11, -m_oMatrix.m_f12, -m_oMatrix.m_f13, 1.0f );
	a_pBlob->SetFacing( vFacing );

	TPhysicsManager::StopBody( a_pBlob->GetBody() );
	a_pBlob->EnterLandmark( m_pLandmark, m_pLandmark->GetTriggerIndex( this ) );
}

// $deBlob: FUNCTION 0057bc30
void ALandmarkInstance::Trigger::OnPainted( ABlobEntity* a_pBlob )
{
	Disable();

	AChallengeManager::GetSingleton()->OnLandmarkEvent( m_pLandmark );

	ALandmarkInstance* pLandmark = m_pLandmark;

	if ( pLandmark->GetNumTriggersLeft() != 0 )
		return;

	// The challenge status HUD of a sprint level's landmark challenge
	AChallengeManager* pManager   = AChallengeManager::GetSingleton();
	AChallenge*        pChallenge = pManager->GetCurrentChallenge();

	if ( AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel != 0 && pChallenge != TNULL &&
	     pChallenge->GetType() == AChallenge::Type_Landmark && TSTATICCAST( ALandmarkChallenge, pChallenge )->GetTriggersLeft() == 0 )
	{
		TINT iTrigger = pLandmark->GetTriggerIndex( this );

		if ( iTrigger != -1 && pManager->GetHudElement() != TNULL )
			TSTATICCAST( ABlobChallengeStatusHud, pManager->GetHudElement() )->SetTriggerPaintLeft( pLandmark, iTrigger, m_iPaintLeft );
	}

	if ( APlayerManager::GetSingleton()->GetNumPlayers() != 1 || !IsHudIdle() )
		pLandmark->m_uiFlags |= Flags_WaitForLetterbox;
	else
		pLandmark->ShowLetterbox();
}

// $deBlob: FUNCTION 0057bd70
void ALandmarkInstance::Trigger::Hide()
{
	m_bPainted = TFALSE;
	SetPaintLeft( m_pLandmark->m_iPaintPerTrigger );

	AModelInstance2* pModel = m_oModel.m_pInstance;
	pModel->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
	pModel->SetAmbientScale( ALightingData::GetSingleton()->GetInstanceAmbientScale( m_pLandmark->m_pTerrainInstance->m_iAmbientScaleIndex ) );
	pModel->GetModelInstance()->SetForceFade( TTRUE );
	pModel->GetModelInstance()->SetFade( 0.25f );

	if ( m_pEmitter )
	{
		AEnvParticles::GetSingleton()->RemoveEmitter( m_pEmitter, TFALSE );
		m_pEmitter = TNULL;
	}

	if ( IsInstanceInWorld( m_pLandmark->m_pTerrainInstance ) )
		GetCollision( pModel )->RemoveFromWorld( 0x60 );
}

// $deBlob: FUNCTION 0057be10
void ALandmarkInstance::Trigger::Show()
{
	m_bPainted = TFALSE;
	SetPaintLeft( m_pLandmark->m_iPaintPerTrigger );

	AModelInstance2* pModel = m_oModel.m_pInstance;
	pModel->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
	pModel->SetAmbientScale( ALightingData::GetSingleton()->GetInstanceAmbientScale( m_pLandmark->m_pTerrainInstance->m_iAmbientScaleIndex ) );
	pModel->GetModelInstance()->SetForceFade( TTRUE );
	pModel->GetModelInstance()->SetFade( 1.0f );

	if ( m_pEmitter == TNULL && ms_pTargetEffect != TNULL )
	{
		TMatrix44 oMatrix = m_oMatrix;
		TVector4  vPosition( 1.0f, 0.0f, 0.0f, 1.0f );
		TMatrix44::TransformVector( vPosition, oMatrix, vPosition );
		oMatrix.SetTranslation( vPosition );
		m_pEmitter = AEnvParticles::GetSingleton()->AddEmitter( ms_pTargetEffect, oMatrix, -1.0f, 0 );
	}

	if ( IsInstanceInWorld( m_pLandmark->m_pTerrainInstance ) )
	{
		ACollisionInstance2* pCollision = GetCollision( pModel );

		if ( m_iBody > -1 && m_iBody < pCollision->GetNumPhysicsObjects() )
			TPhysicsManager::GetSingleton()->SetCollisionLayer( GetBody( pCollision, m_iBody ), m_uiBodyLayer );

		pCollision->AddToWorld( 0x60 );
	}
}

// $deBlob: FUNCTION 0057bf50
void ALandmarkInstance::Trigger::Disable()
{
	m_bPainted = TTRUE;
	SetPaintLeft( 0 );

	m_oModel.m_pInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	GetCollision( m_oModel.m_pInstance )->RemoveFromWorld( 0x60 );

	if ( m_pEmitter )
	{
		AEnvParticles::GetSingleton()->RemoveEmitter( m_pEmitter, TFALSE );
		m_pEmitter = TNULL;
	}

	m_pScene->SetVisible( TFALSE );
}

//-----------------------------------------------------------------------------
// ALandmarkInstance
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0057c040
void ALandmarkInstance::FindEffects()
{
	TParticleEffectManager* pManager = TParticleSystem::GetSingleton()->GetEffectManager();

	ms_pRingsEffect   = pManager->FindEffect( "Blob_LandmarkRings" );
	ms_pGraphicEffect = pManager->FindEffect( "Blob_LandmarkGraphic" );
	ms_pTargetEffect  = pManager->FindEffect( "Blob_LandmarkTarget" );
}

// $deBlob: FUNCTION 0057c080
void ALandmarkInstance::OnCreated( TerrainTypes::CellInstance* a_pInstance )
{
	ALandmarkInstance* pLandmark  = TSTATICCAST( ALandmarkInstance, a_pInstance->m_pModelInstance2 );
	pLandmark->m_pTerrainInstance = a_pInstance;
	pLandmark->Init();

	ABuildingManager::GetSingleton()->AddBuilding( pLandmark );

	if ( !pLandmark->IsABillBoard() )
		pLandmark->SetCollisionLayers( TFALSE );
}

// $deBlob: FUNCTION 0057c0d0
void ALandmarkInstance::OnBlobTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const TVector4* a_pPosition )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Landmark );

	ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, a_pInstance->m_pModelInstance2 );

	if ( ( pLandmark->m_uiFlags & Flags_Completed ) || !pLandmark->IsABillBoard() )
		return;

	if ( !a_pBlob->IsA( &TGetClass( ABlobEntity ) ) )
		return;

	TUINT32 uiColour = GetBlobColour( a_pBlob );

	if ( ( a_pBlob->m_uiFlags & 0x200000 ) || ( uiColour != ABlobColourComponent::COLOUR_WHITE && uiColour != ABlobColourComponent::COLOUR_BLACK ) )
		pLandmark->Complete( a_pBlob );
}

// $deBlob: FUNCTION 0057c180
ALandmarkInstance::ALandmarkInstance( AModel2* a_pModel, TBOOL a_bCreateSoundBreakpoints )
    : ABuildingInstance( a_pModel, a_bCreateSoundBreakpoints )
{
	m_pIdleFX          = TNULL;
	m_pTransformFX     = TNULL;
	m_aTriggers        = TNULL;
	m_pParentLandmark  = TNULL;
	m_fBounceTimer     = 0.0f;
	m_fFadeTimer       = -1.0f;
	m_fTransformTimer  = -1.0f;
	m_fBodyTimer       = 0.0f;
	m_iPaintPerTrigger = 0;
	m_iNumTriggers     = 0;

	for ( TINT i = 0; i < MAX_LANDMARK_ACCESSORY_MESHES; i++ )
		m_aTransformAccessoryMats[ i ] = TNULL;

	// NOTE: retail leaves the other members uninitialised (Init sets them)
	m_oIdleModel.m_pOwner         = TNULL;
	m_oIdleModel.m_pInstance      = TNULL;
	m_oTransformModel.m_pOwner    = TNULL;
	m_oTransformModel.m_pInstance = TNULL;
	m_iIdleId                     = -1;
	m_iTransformId                = -1;
	m_iTransformIdleId            = -1;

	for ( TINT i = 0; i < MAX_LANDMARK_MESHES; i++ )
	{
		m_apOriginalMaterials[ i ]    = TNULL;
		m_apMaterials[ i ]            = TNULL;
		m_apLODOriginalMaterials[ i ] = TNULL;
		m_apLODMaterials[ i ]         = TNULL;
		m_apTextures[ i ]             = TNULL;
		m_aiMeshMaterial[ i ]         = -1;
	}
}

// $deBlob: FUNCTION 00580250
// $deBlob: FUNCTION 0057c250
ALandmarkInstance::~ALandmarkInstance()
{
	if ( m_aTriggers )
		delete[] m_aTriggers;

	if ( m_oIdleModel.m_pInstance )
	{
		GetCollision( m_oIdleModel.m_pInstance )->RemoveFromWorld( 0x60 );
		AModelFactory::GetSingleton()->DestroyInstance( m_oIdleModel.m_pInstance );

		if ( m_pIdleFX )
			delete m_pIdleFX;
	}

	if ( m_oTransformModel.m_pInstance )
	{
		GetCollision( m_oTransformModel.m_pInstance )->RemoveFromWorld( 0x60 );
		AModelFactory::GetSingleton()->DestroyInstance( m_oTransformModel.m_pInstance );

		if ( m_pTransformFX )
			delete m_pTransformFX;
	}

	if ( TSTATICCAST( LandmarkData, m_pTerrainInstance->m_pTypeData )->m_uiNumMeshes != 0 )
	{
		for ( TINT i = 0; i < MAX_LANDMARK_MESHES; i++ )
		{
			m_apMaterials[ i ]->SetUVAnimation( TNULL );
			m_apMaterials[ i ]->Destroy();
		}

		if ( m_pTerrainInstance->m_pLowLODInstance != TNULL )
		{
			for ( TINT i = 0; i < MAX_LANDMARK_MESHES; i++ )
			{
				if ( m_apLODMaterials[ i ] )
				{
					m_apLODMaterials[ i ]->SetUVAnimation( TNULL );
					m_apLODMaterials[ i ]->Destroy();
				}
			}
		}
	}

	for ( TINT i = 0; i < MAX_LANDMARK_ACCESSORY_MESHES; i++ )
	{
		if ( m_aTransformAccessoryMats[ i ] )
		{
			m_aTransformAccessoryMats[ i ]->SetUVAnimation( TNULL );
			m_aTransformAccessoryMats[ i ]->Destroy();
			m_aTransformAccessoryMats[ i ] = TNULL;
		}
	}
}

// $deBlob: FUNCTION 0057c410
void ALandmarkInstance::UpdateImp( TFLOAT a_fDeltaTime )
{
	if ( ( m_uiFlags & Flags_WaitForLetterbox ) && APlayerManager::GetSingleton()->GetNumPlayers() == 1 && IsHudIdle() )
		ShowLetterbox();

	// The trigger bodies come back
	if ( m_fBodyTimer > 0.0f )
	{
		m_fBodyTimer -= a_fDeltaTime;

		if ( m_fBodyTimer < 0.0f )
		{
			m_fBodyTimer = 0.0f;

			for ( TINT i = 0; i < m_iNumTriggers; i++ )
			{
				ACollisionInstance2* pCollision = GetCollision( m_aTriggers[ i ].m_oModel.m_pInstance );

				for ( TINT j = 0; j < pCollision->GetNumPhysicsObjects(); j++ )
				{
					TPhysicsObjectContainer* pBody = GetBody( pCollision, j );

					if ( !( GetBodyLayer( pBody ) & 8 ) )
						TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, 0x90 );
				}
			}
		}
	}

	UpdateTriggers( a_fDeltaTime );

	if ( m_fBounceTimer <= 0.0f )
	{
		TFLOAT fTransformTimer = m_fTransformTimer;

		if ( fTransformTimer > 0.0f )
		{
			m_fTransformTimer = fTransformTimer - a_fDeltaTime;

			if ( m_fTransformTimer < 0.0f )
			{
				m_fTransformTimer = -1.0f;

				if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
				{
					ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

					if ( !APlayerManager::GetSingleton()->GetBlobDataOf( pBlob )->IsKeyFrameCameraActive() )
					{
						TXUI*      pXUI   = TXUI::GetSingleton();
						TXUIScene* pScene = pXUI->GetActiveScene();

						if ( pScene == TNULL || pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL ||
						     pXUI->IsInputLocked() || T2String16::Compare( pScene->GetID(), L"LetterBox" ) != 0 )
						{
							// Waits for the letterbox
							m_fTransformTimer = fTransformTimer;
						}
						else
						{
							pXUI->TransitionBack( pScene );

							if ( pBlob->m_eState == ABlobEntity::STATE_LMARK )
								m_uiFlags |= Flags_LetterboxEnded;
						}
					}
				}
			}
		}
	}
	else
	{
		TFLOAT fScale       = 1.0f;
		TFLOAT fBounceTimer = m_fBounceTimer;
		m_fBounceTimer      = fBounceTimer - a_fDeltaTime;

		if ( m_fBounceTimer > 0.0f )
		{
			if ( s_cfBounceTime >= m_fBounceTimer )
			{
				if ( fBounceTimer > s_cfBounceTime && !IsABillBoard() )
				{
					StartTransform();
					MatchMeshes();
					AddPaintPass();
				}

				// $deBlob: 0x00990d14 (computed once)
				static TFLOAT s_fBounceSlope = ( s_cfBounceTime * 2.0f ) * 0.8666667f;

				TFLOAT fT  = m_fBounceTimer / s_cfBounceTime;
				TFLOAT fSq = fT * fT;
				TFLOAT fX  = ( fT > 0.5f / s_cfBounceTime ) ? 1.0f - ( 1.0f - fT ) * s_fBounceSlope : s_fBounceSlope * fT;

				fScale = 1.0f - TFLOAT( cos( ( 1.0f - fX ) * ( TMath::PI * 7.5f ) ) ) * ( fSq * 0.3f );
			}
		}
		else
		{
			m_fBounceTimer    = 0.0f;
			m_pParentLandmark = TNULL;
		}

		if ( IsABillBoard() )
		{
			SetScale( fScale );

			TVector4 vCentre = *m_pTerrainInstance->m_ppCentre[ 0 ];
			vCentre.Multiply( 1.0f - fScale );

			TVector4 vPosition = m_pTerrainInstance->m_pMatrix->GetTranslation();
			vPosition.Multiply( fScale );

			SetPosition( TVector4( vCentre.x + vPosition.x, vCentre.y + vPosition.y, vCentre.z + vPosition.z, vPosition.w ) );
		}
	}

	if ( m_uiFlags & Flags_LetterboxEnded )
	{
		TXUIScene* pScene = TXUI::GetSingleton()->GetActiveScene();

		if ( pScene != TNULL && pScene->IsA( &TGetClass( ABlobHud ) ) )
		{
			m_uiFlags &= ~Flags_LetterboxEnded;

			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
			TASSERT( pBlob->m_eState == ABlobEntity::STATE_LMARK );

			if ( pBlob->m_eState == ABlobEntity::STATE_LMARK )
			{
				pBlob->ExitLandmark( TTRUE );
			}
		}
	}

	// The paint glow fade
	if ( m_fFadeTimer >= 0.0f )
	{
		TFLOAT fPrev = m_fFadeTimer;
		m_fFadeTimer = fPrev + a_fDeltaTime;

		if ( m_fFadeTimer >= s_fFadeTime )
		{
			RemovePaintPass();
		}
		else
		{
			// $deBlob: 0x00990d20 (computed once)
			static TFLOAT s_fFadeOutStart = s_cfFadeHoldTime + s_cfFadeInTime;

			TFLOAT fT;

			if ( m_fFadeTimer <= s_fFadeOutStart )
			{
				fT = m_fFadeTimer / s_cfFadeInTime;
			}
			else
			{
				if ( s_fFadeOutStart >= fPrev )
					SetPaintTextures();

				fT = 1.0f - ( ( m_fFadeTimer - s_fFadeOutStart ) / s_cfFadeOutTime );
			}

			TFLOAT fFade = TMath::LERP( 0.0f, 1.0f, fT );
			TMath::Clip( fFade, 0.0f, 1.0f );

			TFLOAT fAlpha = TMath::Max( fFade, 0.003921569f );

			TWorldMaterialHAL::s_vLerpConstColour.Set( fFade, fFade, fFade, fAlpha );
			ASkinShaderHAL::s_vTevMode16Const.Set( fFade, fFade, fFade, fAlpha );

			TUINT32 uiGlow;

			if ( m_iNumTriggers != 0 )
			{
				uiGlow = ( m_iNumTriggers <= 1 ) ? ABlobColourMaps::ms_pLandmarkGlowColourMap[ m_aTriggers[ 0 ].m_iColour ] : ABlobColourMaps::ms_pLandmarkGlowColourMap[ 0 ];
			}
			else if ( m_pParentLandmark != TNULL && m_pParentLandmark->m_iNumTriggers > 0 )
			{
				uiGlow = ABlobColourMaps::ms_pLandmarkGlowColourMap[ m_pParentLandmark->m_aTriggers[ 0 ].m_iColour ];
			}
			else
			{
				uiGlow = ABlobColourMaps::ms_pLandmarkGlowColourMap[ 0 ];
			}

			TVector4 vGlow;
			UnpackColour( uiGlow, vGlow );

			TWorldMaterialHAL::s_vMultiPassColour.Set( vGlow.x, vGlow.y, vGlow.z, 0.5f );
			ASkinShaderHAL::s_vTevMode16Colour.Set( vGlow.x, vGlow.y, vGlow.z, 0.5f );
		}
	}

	// The breakpoint FX of the visible model
	TFLOAT fFXDistanceSq = ( m_uiFlags & Flags_PowerGen ) ? ATerrain2Interface::GetInstanceLODDistanceSq( 2 ) : 30625.0f;

	AModelInstance2* pIdle = m_oIdleModel.m_pInstance;

	if ( pIdle != TNULL && ( pIdle->GetFlags() & AModelInstance2::FLAG_Visible ) )
	{
		pIdle->UpdateImp( a_fDeltaTime );

		if ( m_pIdleFX )
		{
			TVector4 vCamera   = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
			TFLOAT   fDistance = TVector4::DistanceXZ( *m_pTerrainInstance->m_ppCentre[ 0 ], vCamera ) - m_pTerrainInstance->m_ppCentre[ 1 ]->MagnitudeXZ();

			if ( fFXDistanceSq > fDistance * fDistance )
			{
				m_pIdleFX->SetVisible( TTRUE );
				m_pIdleFX->Tick( a_fDeltaTime );
				m_uiFlags |= Flags_Update;
				return;
			}

			if ( m_pIdleFX->IsEnabled() )
			{
				m_pIdleFX->SetVisible( TFALSE );
				m_uiFlags |= Flags_Update;
				return;
			}
		}
	}
	else
	{
		AModelInstance2* pTransform = m_oTransformModel.m_pInstance;

		if ( pTransform != TNULL && ( pTransform->GetFlags() & AModelInstance2::FLAG_Visible ) )
		{
			pTransform->UpdateImp( a_fDeltaTime );

			if ( m_pTransformFX )
			{
				TVector4 vPosition;
				pTransform->GetPosition( vPosition );

				if ( fFXDistanceSq <= GetDistanceSqToViewports( vPosition ) )
				{
					if ( m_pTransformFX->IsEnabled() )
						m_pTransformFX->SetVisible( TFALSE );
				}
				else
				{
					m_pTransformFX->SetVisible( TTRUE );
					m_pTransformFX->Tick( a_fDeltaTime );
				}
			}

			AToshiAnimInterface2* pAnim = pTransform->GetAnimInterface();

			if ( pAnim->GetSkeletonInstance() != TNULL && pAnim->GetSkeletonInstance()->IsAnimating( TINT16( m_iTransformId ) ) &&
			     pAnim->IsAnimationFinished( TINT16( m_iTransformId ) ) )
			{
				pAnim->StopAnim( TINT16( m_iTransformId ), 0.0f );

				if ( m_iTransformIdleId != -1 )
					pAnim->AddAnim( TINT16( m_iTransformIdleId ), 0.0f, 1.0f, 1.0f );
			}
		}
	}

	m_uiFlags |= Flags_Update;
}

// $deBlob: FUNCTION 0057cd50
TINT ALandmarkInstance::Reset()
{
	RestoreMaterials();

	if ( !IsABillBoard() )
		SetCollisionLayers( TFALSE );

	for ( TINT i = 0; i < m_iNumTriggers; i++ )
		m_aTriggers[ i ].Show();

	if ( m_oIdleModel.m_pInstance )
	{
		m_oIdleModel.m_pInstance->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );

		if ( m_pIdleFX )
		{
			m_pIdleFX->ResetComponent();
			m_pIdleFX->SetVisible( TFALSE );
		}

		if ( m_iIdleId != -1 )
		{
			m_oIdleModel.m_pInstance->GetAnimInterface()->StopAllAnims( 0.0f );
			m_oIdleModel.m_pInstance->GetAnimInterface()->AddAnim( TINT16( m_iIdleId ), 0.0f, 1.0f, 1.0f );
		}

		if ( IsInstanceInWorld( m_pTerrainInstance ) )
			GetCollision( m_oIdleModel.m_pInstance )->AddToWorld( 0x60 );
	}

	if ( m_oTransformModel.m_pInstance )
	{
		m_oTransformModel.m_pInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

		if ( m_pTransformFX )
		{
			m_pTransformFX->ResetComponent();
			m_pTransformFX->SetVisible( TFALSE );
		}

		m_oTransformModel.m_pInstance->GetAnimInterface()->StopAllAnims( 0.0f );
		GetCollision( m_oTransformModel.m_pInstance )->RemoveFromWorld( 0x60 );
	}

	TUINT32 uiFlags   = m_uiFlags;
	m_pParentLandmark = TNULL;
	m_fBounceTimer    = 0.0f;
	m_fTransformTimer = -1.0f;
	m_fBodyTimer      = 0.0f;

	TINT iResult = ABuildingInstance::Reset();
	m_uiFlags    = uiFlags & ( Flags_PowerGen | Flags_MeshesMatched );

	return iResult;
}

// $deBlob: FUNCTION 0057cec0
TBOOL ALandmarkInstance::IsABillBoard() const
{
	return m_pTerrainInstance != TNULL && ( TSTATICCAST( LandmarkData, m_pTerrainInstance->m_pTypeData )->m_uiFlags & LandmarkData::Flags_BillBoard );
}

// $deBlob: FUNCTION 0057cee0
void ALandmarkInstance::Init()
{
	LandmarkData* pData = TSTATICCAST( LandmarkData, m_pTerrainInstance->m_pTypeData );
	m_iPaintPerTrigger  = pData->m_uiPaintPerTrigger;

	if ( TStringManager::String8Compare( m_pTerrainInstance->m_szModel, "powergen_main" ) == 0 )
		m_uiFlags |= Flags_PowerGen;

	TCHAR szName[ 128 ];

	AModelInstance2* pIdle = CreateModel( &m_oIdleModel, pData->m_szIdleModel, *m_pTerrainInstance->m_pMatrix, TTRUE, TNULL );

	if ( pIdle != TNULL )
	{
		pIdle->SetFlag( 0x4000, TTRUE );

		AToshiAnimInterface2* pAnim = pIdle->GetAnimInterface();
		TINT16                iId   = TINT16( pAnim->GetAnimID( "idle_loop" ) );
		m_iIdleId                   = iId;

		if ( iId != -1 )
			pAnim->AddAnim( iId, 0.0f, 1.0f, 1.0f );

		TStringManager::String8Copy( szName, pData->m_szIdleModel, 128 );
		TINT iLen = TStringManager::String8Length( szName );
		TASSERT( iLen + 4 < 128 );
		TStringManager::String8Copy( szName + iLen, ".trb", 128 - iLen );

		m_pIdleFX = AFXBreakpointComponent::Create( szName, TMemory_GetGlobalHeap(), TNULL, pIdle );

		if ( m_pIdleFX )
			m_pIdleFX->SetVisible( TFALSE );
	}

	AModelInstance2* pTransform = CreateModel( &m_oTransformModel, pData->m_szTransformModel, *m_pTerrainInstance->m_pMatrix, TTRUE, TNULL );

	if ( pTransform != TNULL )
	{
		pTransform->SetFlag( 0x4000, TTRUE );
		m_oTransformModel.m_pInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

		m_iTransformId     = TINT16( pTransform->GetAnimInterface()->GetAnimID( "transform" ) );
		m_iTransformIdleId = TINT16( pTransform->GetAnimInterface()->GetAnimID( "idle_loop" ) );

		TStringManager::String8Copy( szName, pData->m_szTransformModel, 128 );
		TINT iLen = TStringManager::String8Length( szName );
		TASSERT( iLen + 4 < 128 );
		TStringManager::String8Copy( szName + iLen, ".trb", 128 - iLen );

		m_pTransformFX = AFXBreakpointComponent::Create( szName, TMemory_GetGlobalHeap(), TNULL, pTransform );

		if ( m_pTransformFX )
			m_pTransformFX->SetVisible( TFALSE );
	}

	if ( pData->m_uiNumTriggers != 0 )
	{
		m_iNumTriggers = pData->m_uiNumTriggers;
		m_aTriggers    = new Trigger[ m_iNumTriggers ];

		TPhysicsManager* pPhysics = TPhysicsManager::GetSingleton();
		TXUIResource*    pHud     = TXUI::GetSingleton()->FindResource( "hud.xur" );

		for ( TINT i = 0; i < m_iNumTriggers; i++ )
		{
			Trigger& rTrigger    = m_aTriggers[ i ];
			rTrigger.m_pLandmark = this;
			rTrigger.m_iColour   = pData->m_pTriggers[ i ].m_iColour;
			rTrigger.SetPaintLeft( m_iPaintPerTrigger );

			// NOTE: retail takes the scale out of the data's matrix itself
			TMatrix44* pMatrix = pData->m_pTriggers[ i ].m_pMatrix;

			TVector4 vScale;
			pMatrix->GetScale( vScale );

			TVector4 vInvScale;
			vInvScale.x = ( vScale.x <= 0.00001f ) ? 1.0f : 1.0f / vScale.x;
			vInvScale.y = ( vScale.y <= 0.00001f ) ? 1.0f : 1.0f / vScale.y;
			vInvScale.z = ( vScale.z <= 0.00001f ) ? 1.0f : 1.0f / vScale.z;
			pMatrix->Scale( vInvScale.x, vInvScale.y, vInvScale.z );

			rTrigger.m_oMatrix = *pMatrix;

			if ( pHud != TNULL )
			{
				rTrigger.m_pScene = TXUI::GetSingleton()->CreateScene( pHud, L"LandmarkTarget", TFALSE );
				rTrigger.m_pScene->SetVisible( TFALSE );
				rTrigger.m_pScene->SetPresenterText( rTrigger.m_wszPaintLeft, 0, -1 );
				TXUIVisual* pVisual = rTrigger.m_pScene->GetVisual();
				pVisual->GetTimelineControl()->StopAnimation();

				// NOTE: retail swaps the bytes of the colour around and back (no change)
				TINT    iColour = rTrigger.m_iColour;
				TUINT32 uiFill  = ABlobColourComponent::ms_auiColours[ iColour ];

				TXUIElement* pShape = pVisual->FindChildXUIElement( L"DropletLow" );
				TASSERT( pShape->IsA( &TGetClass( TXUIFigure ) ) );
				rTrigger.m_pDropletLow = TSTATICCAST( TXUIFigure, pShape );
				SetFigureColour( rTrigger.m_pDropletLow, uiFill );

				pShape = pVisual->FindChildXUIElement( L"DropletHigh" );
				TASSERT( pShape->IsA( &TGetClass( TXUIFigure ) ) );
				rTrigger.m_pDropletHigh = TSTATICCAST( TXUIFigure, pShape );
				SetFigureColour( rTrigger.m_pDropletHigh, uiFill );

				// The light colours get the dark text
				TBOOL bLight = ( iColour == 1 || iColour == 6 || iColour == 5 );

				rTrigger.m_pTextPresenter1 = pVisual->FindChildXUIElement( L"XuiTextPresenter1" );
				rTrigger.m_pTextPresenter1->SetVisible( bLight );
				rTrigger.m_pTextPresenter2 = pVisual->FindChildXUIElement( L"XuiTextPresenter2" );
				rTrigger.m_pTextPresenter2->SetVisible( !bLight );
			}

			CreateModel( &rTrigger.m_oModel, "Landmark_Target_New", rTrigger.m_oMatrix, TTRUE, &rTrigger.m_oCollisionHandler );

			ACollisionInstance2* pCollision = GetCollision( rTrigger.m_oModel.m_pInstance );

			for ( TINT j = 0; j < pCollision->GetNumPhysicsObjects(); j++ )
			{
				TPhysicsObjectContainer* pBody   = GetBody( pCollision, j );
				TUINT32                  uiLayer = GetBodyLayer( pBody ) & ~8;
				pPhysics->SetCollisionLayer( pBody, uiLayer );

				if ( uiLayer & 0x81 )
				{
					rTrigger.m_uiBodyLayer = TUINT16( uiLayer );
					rTrigger.m_iBody       = TINT16( j );
				}
			}

			rTrigger.Show();
		}
	}

	if ( pData->m_uiNumMeshes != 0 )
	{
		for ( TINT i = 0; i < pData->m_uiNumMeshes; i++ )
			m_apTextures[ i ] = TTextureManager::GetSingleton()->FindTexture( pData->m_ppTextureNames[ i ] );

		TWorldShader* pWorldShader = TWorldShader::GetSingleton();

		for ( TINT i = 0; i < MAX_LANDMARK_MESHES; i++ )
		{
			TWorldMaterial* pMaterial = pWorldShader->CreateMaterial( "landmark" );
			TASSERT( pMaterial == TNULL || pMaterial->IsA( &TGetClass( TWorldMaterialHAL ) ) );
			pMaterial->Create( 0 );
			m_apMaterials[ i ] = pMaterial;
		}

		if ( m_pTerrainInstance->m_pLowLODInstance != TNULL )
		{
			T2ModelPtr pModel = m_pTerrainInstance->m_pLowLODInstance->GetModel();
			TModelLOD& rLOD   = pModel->GetLOD( 0 );

			TINT iMeshCount = rLOD.GetMeshCount();
			TASSERT( iMeshCount < MAX_LANDMARK_MESHES );

			for ( TINT i = 0; i < MAX_LANDMARK_MESHES; i++ )
			{
				m_apLODOriginalMaterials[ i ] = TNULL;
				m_apLODMaterials[ i ]         = TNULL;
			}

			for ( TINT i = 0; i < iMeshCount; i++ )
			{
				TMaterial* pOriginal = rLOD.GetMeshes()[ i ]->GetMaterial();
				TTexture*  pTexture  = pOriginal->GetTexture( 0 );

				for ( TINT j = 0; j < TSTATICCAST( LandmarkData, m_pTerrainInstance->m_pTypeData )->m_uiNumMeshes; j++ )
				{
					if ( TStringManager::String8Compare( pTexture->GetName(), pData->m_ppMeshNames[ j ] ) != 0 )
						continue;

					ASkinMaterial* pMaterial = TSTATICCAST( ASkinMaterial, ASkinShader::GetSingleton()->CreateMaterial( TNULL ) );
					pMaterial->Create( 0 );

					m_apLODMaterials[ i ]         = pMaterial;
					m_apLODOriginalMaterials[ i ] = pOriginal;

					pMaterial->SetTextureNum( pOriginal->GetTextureNum() );

					for ( TUINT k = 0; k < pOriginal->GetTextureNum(); k++ )
						pMaterial->SetTexture( k, pOriginal->GetTexture( k ) );

					pMaterial->SetName( pOriginal->GetName() );
					pMaterial->SetTexture( 0, m_apTextures[ j ] );
					break;
				}
			}
		}
	}

	if ( m_oTransformModel.m_pInstance != TNULL )
	{
		T2ModelPtr pModel  = m_oTransformModel.m_pInstance->GetModelInstance()->GetModel();
		TModelLOD& rAccLOD = pModel->GetLOD( 0 );
		TASSERT( rAccLOD.GetMeshCount() < MAX_LANDMARK_ACCESSORY_MESHES );

		TINT iNumMeshes = TMath::Min( rAccLOD.GetMeshCount(), MAX_LANDMARK_ACCESSORY_MESHES );

		for ( TINT i = 0; i < iNumMeshes; i++ )
		{
			m_aTransformAccessoryMats[ i ] = TNULL;

			ASkinMaterialHAL* pCurMat = ASkinMaterialHAL::Cast( ASkinShader::GetSingleton()->CreateMaterial( TNULL ) );
			pCurMat->Create( 0 );
			m_aTransformAccessoryMats[ i ] = pCurMat;

			ASkinMaterialHAL* pSkinMat = ASkinMaterialHAL::Cast( rAccLOD.GetMeshes()[ i ]->GetMaterial() );
			AMaterialInfo::CopyMaterial( pSkinMat, pCurMat );
			TASSERT( pSkinMat->GetTextureNum() == pCurMat->GetTextureNum() );

			for ( TUINT j = 0; j < pSkinMat->GetTextureNum(); j++ )
			{
				pCurMat->SetTevMode( j, pSkinMat->GetTevMode( j ) );
				pCurMat->SetPassMode( j, pSkinMat->GetPassMode( j ) );
			}

			// The paint pass (tev mode 0x16, ASkinShaderHAL::s_vTevMode16Colour/s_vTevMode16Const)
			TUINT uiStage = pCurMat->GetTextureNum();
			pCurMat->SetTextureNum( uiStage + 1 );
			pCurMat->SetTexture( uiStage, TTextureManager::GetSingleton()->GetWhiteTexture() );
			pCurMat->SetBlendMode( 25, 0 );
			pCurMat->SetTevMode( uiStage, 0x16 );
			pCurMat->SetFlags( pSkinMat->GetFlags(), TTRUE );
			pCurMat->SetOrderTable( pSkinMat->GetOrderTable(), TFALSE );

			TASSERT( m_aTransformAccessoryMats[ i ]->IsCreated() );
		}
	}
}

// $deBlob: FUNCTION 0057deb0
void ALandmarkInstance::OnAddedToWorld( TerrainTypes::CellInstance* a_pInstance )
{
	ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, a_pInstance->m_pModelInstance2 );

	pLandmark->MatchMeshes();
	pLandmark->SetPaintTextures();

	AModelInstance2* pModel = ( pLandmark->m_uiFlags & Flags_Completed ) ? pLandmark->m_oTransformModel.m_pInstance : pLandmark->m_oIdleModel.m_pInstance;

	if ( pModel != TNULL )
		GetCollision( pModel )->AddToWorld( 0x60 );

	if ( !( pLandmark->m_uiFlags & ( Flags_TriggersHidden | Flags_Completed ) ) )
	{
		for ( TINT i = 0; i < pLandmark->m_iNumTriggers; i++ )
		{
			Trigger&             rTrigger   = pLandmark->m_aTriggers[ i ];
			ACollisionInstance2* pCollision = GetCollision( rTrigger.m_oModel.m_pInstance );

			if ( rTrigger.m_iBody > -1 && rTrigger.m_iBody < pCollision->GetNumPhysicsObjects() )
				TPhysicsManager::GetSingleton()->SetCollisionLayer( GetBody( pCollision, rTrigger.m_iBody ), rTrigger.m_uiBodyLayer );

			if ( !rTrigger.m_bPainted )
				pCollision->AddToWorld( 0x60 );
		}
	}
}

// $deBlob: FUNCTION 0057df90
void ALandmarkInstance::OnRemovedFromWorld( TerrainTypes::CellInstance* a_pInstance )
{
	ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, a_pInstance->m_pModelInstance2 );

	pLandmark->RestoreMaterials();

	if ( s_fFadeTime > pLandmark->m_fFadeTimer && pLandmark->m_fFadeTimer >= 0.0f )
		pLandmark->RemovePaintPass();

	AModelInstance2* pModel = ( pLandmark->m_uiFlags & Flags_Completed ) ? pLandmark->m_oTransformModel.m_pInstance : pLandmark->m_oIdleModel.m_pInstance;

	if ( pModel != TNULL )
		GetCollision( pModel )->RemoveFromWorld( 0x60 );
}

// $deBlob: FUNCTION 0057e000
void ALandmarkInstance::SetLODMeshMaterials()
{
	LandmarkData* pData = TSTATICCAST( LandmarkData, m_pTerrainInstance->m_pTypeData );

	if ( !( m_uiFlags & Flags_Completed ) || pData->m_uiNumMeshes == 0 )
		return;

	T2ModelPtr pModel = m_pTerrainInstance->m_pLowLODInstance->GetModel();
	TModelLOD& rLOD   = pModel->GetLOD( 0 );

	TINT iMeshCount = rLOD.GetMeshCount();
	TASSERT( iMeshCount < MAX_LANDMARK_MESHES );

	for ( TINT i = 0; i < iMeshCount; i++ )
	{
		if ( m_apLODMaterials[ i ] )
			rLOD.GetMeshes()[ i ]->SetMaterial( m_apLODMaterials[ i ] );
	}
}

// $deBlob: FUNCTION 0057e130
void ALandmarkInstance::RestoreLODMeshMaterials()
{
	LandmarkData* pData = TSTATICCAST( LandmarkData, m_pTerrainInstance->m_pTypeData );

	if ( !( m_uiFlags & Flags_Completed ) || pData->m_uiNumMeshes == 0 )
		return;

	T2ModelPtr pModel = m_pTerrainInstance->m_pLowLODInstance->GetModel();
	TModelLOD& rLOD   = pModel->GetLOD( 0 );

	TINT iMeshCount = rLOD.GetMeshCount();
	TASSERT( iMeshCount < MAX_LANDMARK_MESHES );

	for ( TINT i = 0; i < iMeshCount; i++ )
	{
		if ( m_apLODMaterials[ i ] )
			rLOD.GetMeshes()[ i ]->SetMaterial( m_apLODOriginalMaterials[ i ] );
	}
}

// $deBlob: FUNCTION 0057e260
void ALandmarkInstance::OnTable97b978( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Landmark );

	TSTATICCAST( ALandmarkInstance, a_pInstance->m_pModelInstance2 )->Complete( a_pBlob );
}

// $deBlob: FUNCTION 0057e2b0
void ALandmarkInstance::Complete( ABlobEntity* a_pBlob )
{
	if ( IsABillBoard() )
	{
		AGameStatus* pGameStatus = AGameStatus::GetSingleton();
		pGameStatus->OnBuildingPaintChanged( this );
		AChallengeManager::GetSingleton()->OnPaintEvent( this );

		m_uiFlags |= Flags_Completed;
		m_fBounceTimer = s_cfBounceTime;

		const TVector4* pCentre = m_pTerrainInstance->m_ppCentre[ 0 ];
		TINT            iPlayer = APlayerManager::GetSingleton()->GetPlayerIndex( a_pBlob );

		// The billboard score (BlobScoringProperties 0x80, 0x84 in multiplayer)
		TINT iScore = ( pGameStatus->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE ) ? TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x80 / 4 ] : TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x84 / 4 ];

		pGameStatus->AddBonusScore( iPlayer, iScore, pCentre );
		AMusicManager::GetSingleton()->PlayMusicSFX( "billboard_transform", 2, pCentre, TNULL );

		MatchMeshes();
		SetPaintTextures();

		// The billboards transformed
		pGameStatus->GetLevelData().m_aPlayerData[ 0 ].m_auiProgress[ 5 ]++;
	}
	else
	{
		StartTransform();
		MatchMeshes();
		AddPaintPass();

		m_fBounceTimer    = s_cfBounceTime;
		m_fTransformTimer = -1.0f;
	}
}

// $deBlob: FUNCTION 0057e3a0
TINT ALandmarkInstance::GetNumTriggersLeft() const
{
	TINT iNumLeft = 0;

	for ( TINT i = 0; i < m_iNumTriggers; i++ )
	{
		if ( !m_aTriggers[ i ].m_bPainted )
			iNumLeft++;
	}

	return iNumLeft;
}

// $deBlob: FUNCTION 0057e3d0
void ALandmarkInstance::GetTriggerPosition( TINT a_iIndex, TVector4& a_rOutPosition ) const
{
	TASSERT( a_iIndex < m_iNumTriggers );

	a_rOutPosition.Set( 0.5f, 0.0f, 0.0f, 1.0f );
	TMatrix44::TransformVector( a_rOutPosition, m_aTriggers[ a_iIndex ].m_oMatrix, a_rOutPosition );
}

// $deBlob: FUNCTION 0057e450
void ALandmarkInstance::GetTriggerMatrix( TINT a_iIndex, TMatrix44& a_rOutMatrix ) const
{
	TASSERT( a_iIndex < m_iNumTriggers );

	a_rOutMatrix = m_aTriggers[ a_iIndex ].m_oMatrix;
}

// $deBlob: FUNCTION 0057e4d0
void ALandmarkInstance::PaintTrigger( TINT a_iIndex, ABlobEntity* a_pBlob )
{
	TASSERT( a_iIndex <= m_iNumTriggers );

	Trigger& rTrigger = m_aTriggers[ a_iIndex ];
	rTrigger.m_iPaintLeft--;
	T2String16::Format( rTrigger.m_wszPaintLeft, 8, L"%d", rTrigger.m_iPaintLeft );

	if ( a_pBlob->GetSizeHud() != TNULL )
		a_pBlob->GetSizeHud()->SetLandmarkProgress( rTrigger.m_iPaintLeft, m_iPaintPerTrigger );

	if ( rTrigger.m_iPaintLeft <= 0 )
		rTrigger.OnPainted( a_pBlob );

	AChallengeManager* pManager = AChallengeManager::GetSingleton();

	if ( pManager->GetCurrentChallenge() != TNULL && pManager->GetHudElement() != TNULL )
		TSTATICCAST( ABlobChallengeStatusHud, pManager->GetHudElement() )->SetTriggerPaintLeft( this, a_iIndex, m_aTriggers[ a_iIndex ].m_iPaintLeft );

	ABlobHud::GetSingleton()->ShowRandomSplat();
}

// $deBlob: FUNCTION 0057e5b0
TBOOL ALandmarkInstance::GetTriggerExtents( TINT a_iIndex, TVector4& a_rOutMin, TVector4& a_rOutMax ) const
{
	TASSERT( a_iIndex < m_iNumTriggers );

	const TVector4* pExtents = TSTATICCAST( LandmarkData, m_pTerrainInstance->m_pTypeData )->m_pTriggers[ a_iIndex ].m_pExtents;

	if ( pExtents == TNULL )
		return TFALSE;

	const TFLOAT* pValues = TREINTERPRETCAST( const TFLOAT*, pExtents );
	a_rOutMin.Set( pValues[ 0 ], pValues[ 1 ], pValues[ 2 ], 1.0f );
	a_rOutMax.Set( pValues[ 4 ], pValues[ 5 ], pValues[ 6 ], 1.0f );

	return TTRUE;
}

// $deBlob: FUNCTION 0057e650
void ALandmarkInstance::DisableTriggerBodies()
{
	m_fBodyTimer = 0.5f;

	for ( TINT i = 0; i < m_iNumTriggers; i++ )
	{
		Trigger&             rTrigger   = m_aTriggers[ i ];
		ACollisionInstance2* pCollision = GetCollision( rTrigger.m_oModel.m_pInstance );

		if ( rTrigger.m_iBody > -1 && rTrigger.m_iBody < pCollision->GetNumPhysicsObjects() )
			TPhysicsManager::GetSingleton()->SetCollisionLayer( GetBody( pCollision, rTrigger.m_iBody ), 0 );
	}
}

// $deBlob: FUNCTION 0057e6e0
void ALandmarkInstance::SetTriggersHidden( TBOOL a_bHide )
{
	if ( a_bHide )
	{
		TASSERT( !( m_uiFlags & Flags_Completed ) );
		m_uiFlags = ( m_uiFlags & ~( Flags_WaitForLetterbox | Flags_TriggersHidden ) ) | Flags_TriggersHidden;

		for ( TINT i = 0; i < m_iNumTriggers; i++ )
			m_aTriggers[ i ].Hide();
	}
	else
	{
		m_uiFlags &= ~Flags_TriggersHidden;

		for ( TINT i = 0; i < m_iNumTriggers; i++ )
		{
			if ( !m_aTriggers[ i ].m_bPainted )
				m_aTriggers[ i ].Show();
		}
	}
}

// $deBlob: FUNCTION 0057fe70
void ALandmarkInstance::SetAllTriggersHidden( TBOOL a_bHide )
{
	for ( TINT i = 0; i < m_iNumTriggers; i++ )
	{
		if ( a_bHide )
			m_aTriggers[ i ].Hide();
		else
			m_aTriggers[ i ].Show();
	}
}

// $deBlob: FUNCTION 0057e7b0
void ALandmarkInstance::SetTargetScenesVisible( TBOOL a_bVisible )
{
	for ( TINT i = 0; i < m_iNumTriggers; i++ )
	{
		if ( m_aTriggers[ i ].m_pScene != TNULL && !m_aTriggers[ i ].m_bPainted )
			m_aTriggers[ i ].m_pScene->SetVisible( a_bVisible );
	}
}

// $deBlob: FUNCTION 0057e800
void ALandmarkInstance::UncountCivs()
{
	// The civilians linked to the instance no longer count for the level
	Links* pLinks = TSTATICCAST( Links, m_pTerrainInstance->m_pLinks );

	if ( pLinks == TNULL )
		return;

	for ( TINT i = 0; i < pLinks->m_iNumEntities; i++ )
	{
		ABaseEntity* pEntity = ABaseEntity::FindIDEntity( pLinks->m_piEntityIDs[ i ] );

		if ( pEntity != TNULL && pEntity->IsA( &TGetClass( ACiv ) ) && TSTATICCAST( ACiv, pEntity )->CanBeLiberated() )
			AGameStatus::GetSingleton()->GetLevelData().RemoveCivilian();
	}
}

// $deBlob: FUNCTION 0057e8b0
TINT ALandmarkInstance::GetTriggerIndex( const Trigger* a_pTrigger ) const
{
	for ( TINT i = 0; i < m_iNumTriggers; i++ )
	{
		if ( &m_aTriggers[ i ] == a_pTrigger )
			return i;
	}

	return -1;
}

// $deBlob: FUNCTION 0057e8f0
void ALandmarkInstance::MatchMeshes()
{
	if ( !IsInstanceInWorld( m_pTerrainInstance ) )
		return;

	LandmarkData* pData     = TSTATICCAST( LandmarkData, m_pTerrainInstance->m_pTypeData );
	TINT          iNumNames = pData->m_uiNumMeshes;

	if ( iNumNames == 0 )
		return;

	TerrainTypes::InstanceMeshes* pMeshes = m_pTerrainInstance->m_pSymbolData;

	if ( !( m_uiFlags & Flags_MeshesMatched ) )
	{
		TINT iNumMeshes = pMeshes->m_uiNumMeshes;
		TASSERT( iNumMeshes < MAX_LANDMARK_MESHES );

		for ( TINT i = 0; i < iNumMeshes; i++ )
		{
			m_aiMeshMaterial[ i ] = -1;

			TWorldMeshInstanceData::Source& rSource  = pMeshes->m_pMeshes[ i ];
			TTexture*                       pTexture = rSource.m_pMaterial->GetTexture( rSource.m_szLightmap != TNULL );

			for ( TINT j = 0; j < iNumNames; j++ )
			{
				if ( TStringManager::String8Compare( pTexture->GetName(), pData->m_ppMeshNames[ j ] ) == 0 )
				{
					m_aiMeshMaterial[ i ] = TINT8( j );
					break;
				}
			}
		}

		m_uiFlags |= Flags_MeshesMatched;
	}

	if ( m_uiFlags & Flags_Completed )
	{
		TINT iNumMeshes = pMeshes->m_uiNumMeshes;
		TASSERT( iNumMeshes < MAX_LANDMARK_MESHES );

		for ( TINT i = 0; i < iNumMeshes; i++ )
		{
			if ( m_aiMeshMaterial[ i ] < 0 )
				continue;

			TWorldMaterialHAL* pCopy   = TSTATICCAST( TWorldMaterialHAL, m_apMaterials[ i ] );
			TWorldMaterialHAL* pSource = pMeshes->m_pMeshes[ i ].m_pMaterial;

			AMaterialInfo::CopyMaterial( pSource, pCopy );
			pCopy->SetFlags( pSource->GetFlags(), TTRUE );
			pCopy->SetOrderTable( pSource->GetOrderTable(), 0, TTRUE );

			m_apOriginalMaterials[ i ]          = pSource;
			pMeshes->m_pMeshes[ i ].m_pMaterial = pCopy;
		}
	}
}

// $deBlob: FUNCTION 0057eb20
void ALandmarkInstance::RestoreMaterials()
{
	LandmarkData* pData = TSTATICCAST( LandmarkData, m_pTerrainInstance->m_pTypeData );

	if ( pData->m_uiNumMeshes == 0 || !( m_uiFlags & Flags_Completed ) || !IsInstanceInWorld( m_pTerrainInstance ) )
		return;

	TerrainTypes::InstanceMeshes* pMeshes    = m_pTerrainInstance->m_pSymbolData;
	TINT                          iNumMeshes = pMeshes->m_uiNumMeshes;
	TASSERT( iNumMeshes < MAX_LANDMARK_MESHES );

	for ( TINT i = 0; i < iNumMeshes; i++ )
	{
		if ( m_aiMeshMaterial[ i ] >= 0 )
			pMeshes->m_pMeshes[ i ].m_pMaterial = TSTATICCAST( TWorldMaterialHAL, m_apOriginalMaterials[ i ] );
	}

	for ( TINT i = 0; i < MAX_LANDMARK_MESHES; i++ )
	{
		TMaterial* pMaterial = m_apMaterials[ i ];

		if ( pMaterial )
		{
			for ( TUINT j = 0; j < pMaterial->GetTextureNum(); j++ )
				pMaterial->SetTexture( j, TNULL );
		}
	}
}

// $deBlob: FUNCTION 0057ec50
void ALandmarkInstance::AddPaintPass()
{
	if ( !( m_uiFlags & Flags_Completed ) || !( m_uiFlags & Flags_MeshesMatched ) )
		return;

	TerrainTypes::InstanceMeshes* pMeshes = m_pTerrainInstance->m_pSymbolData;

	if ( pMeshes != TNULL )
	{
		TINT iNumMeshes = pMeshes->m_uiNumMeshes;
		TASSERT( iNumMeshes < MAX_LANDMARK_MESHES );

		for ( TINT i = 0; i < iNumMeshes; i++ )
		{
			if ( m_aiMeshMaterial[ i ] < 0 )
				continue;

			TWorldMaterialHAL* pMaterial = TSTATICCAST( TWorldMaterialHAL, m_apMaterials[ i ] );
			const TCHAR*       szName    = pMaterial->GetMaterialName();

			if ( szName == TNULL || ( TStringManager::String8Compare( szName, "window", 6 ) != 0 && TStringManager::String8Compare( szName, "sticker", 7 ) != 0 ) )
			{
				TUINT uiStage = pMaterial->GetTextureNum();
				pMaterial->SetTextureNum( uiStage + 1 );
				pMaterial->SetTexture( uiStage, TTextureManager::GetSingleton()->GetWhiteTexture() );
				pMaterial->SetBlendMode( 25 );
				pMaterial->SetMultiPassBlendType( uiStage, TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPCONST );
			}
		}
	}

	if ( m_oTransformModel.m_pInstance != TNULL )
	{
		T2ModelPtr pModel  = m_oTransformModel.m_pInstance->GetModelInstance()->GetModel();
		TModelLOD& rAccLOD = pModel->GetLOD( 0 );

		m_oTransformModel.m_pInstance->SetFlag( 0x100, TTRUE );
		TASSERT( rAccLOD.GetMeshCount() < MAX_LANDMARK_ACCESSORY_MESHES );

		TINT iNumMeshes = TMath::Min( rAccLOD.GetMeshCount(), MAX_LANDMARK_ACCESSORY_MESHES );

		for ( TINT i = 0; i < iNumMeshes; i++ )
		{
			TMesh* pMesh = rAccLOD.GetMeshes()[ i ];
			TASSERT( pMesh == TNULL || pMesh->IsA( &TGetClass( ASkinMesh ) ) );
			TSTATICCAST( ASkinMesh, pMesh )->SetMaterial2( m_aTransformAccessoryMats[ i ] );
		}
	}

	TWorldMaterialHAL::s_vLerpConstColour.Set( 0.0f, 0.0f, 0.0f, 0.003921569f );
	ASkinShaderHAL::s_vTevMode16Const.Set( 0.0f, 0.0f, 0.0f, 0.003921569f );
	TWorldMaterialHAL::s_vMultiPassColour.Set( 0.0f, 0.0f, 0.0f, 0.5f );
	ASkinShaderHAL::s_vTevMode16Colour.Set( 0.0f, 0.0f, 0.0f, 0.5f );
}

// $deBlob: FUNCTION 0057ef20
void ALandmarkInstance::RemovePaintPass()
{
	m_fFadeTimer = -1.0f;

	if ( !( m_uiFlags & Flags_Completed ) || !( m_uiFlags & Flags_MeshesMatched ) )
		return;

	TerrainTypes::InstanceMeshes* pMeshes = m_pTerrainInstance->m_pSymbolData;

	if ( pMeshes != TNULL )
	{
		TINT iNumMeshes = pMeshes->m_uiNumMeshes;
		TASSERT( iNumMeshes < MAX_LANDMARK_MESHES );

		for ( TINT i = 0; i < iNumMeshes; i++ )
		{
			if ( m_aiMeshMaterial[ i ] < 0 )
				continue;

			TWorldMaterialHAL* pMaterial = TSTATICCAST( TWorldMaterialHAL, m_apMaterials[ i ] );
			TUINT              uiNumTex  = pMaterial->GetTextureNum();

			if ( pMaterial->GetMultiPassBlendType( uiNumTex - 1 ) == TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPCONST )
			{
				pMaterial->SetTexture( uiNumTex - 1, TNULL );
				pMaterial->SetBlendMode( 0 );
				pMaterial->SetMultiPassBlendType( uiNumTex - 1, TWorldMaterialHAL::MULTIPASSBLENDTYPE_MODULATE );
				pMaterial->SetTextureNum( uiNumTex - 1 );
			}
		}
	}

	if ( m_oTransformModel.m_pInstance != TNULL )
	{
		T2ModelPtr pModel  = m_oTransformModel.m_pInstance->GetModelInstance()->GetModel();
		TModelLOD& rAccLOD = pModel->GetLOD( 0 );

		m_oTransformModel.m_pInstance->SetFlag( 0x100, TFALSE );
		TASSERT( rAccLOD.GetMeshCount() < MAX_LANDMARK_ACCESSORY_MESHES );

		TINT iNumMeshes = TMath::Min( rAccLOD.GetMeshCount(), MAX_LANDMARK_ACCESSORY_MESHES );

		for ( TINT i = 0; i < iNumMeshes; i++ )
		{
			TMesh* pMesh = rAccLOD.GetMeshes()[ i ];
			TASSERT( pMesh == TNULL || pMesh->IsA( &TGetClass( ASkinMesh ) ) );
			TSTATICCAST( ASkinMesh, pMesh )->SetMaterial2( TNULL );
		}
	}
}

// $deBlob: FUNCTION 0057f180
void ALandmarkInstance::SetPaintTextures()
{
	if ( !( m_uiFlags & Flags_Completed ) || !( m_uiFlags & Flags_MeshesMatched ) )
		return;

	TerrainTypes::InstanceMeshes* pMeshes = m_pTerrainInstance->m_pSymbolData;

	if ( pMeshes == TNULL )
		return;

	TINT iNumMeshes = pMeshes->m_uiNumMeshes;
	TASSERT( iNumMeshes < MAX_LANDMARK_MESHES );

	for ( TINT i = 0; i < iNumMeshes; i++ )
	{
		if ( m_aiMeshMaterial[ i ] < 0 )
			continue;

		TMaterial* pMaterial = m_apMaterials[ i ];
		TUINT32    uiStage   = pMeshes->m_pMeshes[ i ].m_szLightmap != TNULL;

		TTexture* pProjection = AGameStatus::GetProjectionTexture( this );

		if ( pProjection != TNULL )
		{
			// NOTE: retail returns here
			pMaterial->SetTexture( uiStage, pProjection );
			return;
		}

		pMaterial->SetTexture( uiStage, m_apTextures[ m_aiMeshMaterial[ i ] ] );
	}
}

// $deBlob: FUNCTION 0057f2a0
AModelInstance2* ALandmarkInstance::CreateModel( ModelRef* a_pRef, const TCHAR* a_szName, const TMatrix44& a_rMatrix, TBOOL a_bCollision, TPhysicsResponder* a_pListener )
{
	if ( a_szName == TNULL || a_szName[ 0 ] == '\0' )
	{
		a_pRef->m_pInstance = TNULL;
		a_pRef->m_pOwner    = TNULL;
		return a_pRef->m_pInstance;
	}

	TCHAR szName[ 128 ];
	TStringManager::String8Copy( szName, a_szName, 128 );
	TINT iLen = TStringManager::String8Length( szName );
	TStringManager::String8Copy( szName + iLen, ".trb", 128 - iLen );

	AModelInstance2* pInstance = AModelFactory::GetSingleton()->CreateInstance( szName, TTRUE, TTRUE );
	pInstance->SetAmbientScale( ALightingData::GetSingleton()->GetInstanceAmbientScale( m_pTerrainInstance->m_iAmbientScaleIndex ) );

	a_pRef->m_pInstance = pInstance;
	a_pRef->m_pOwner    = this;

	TModelInstance* pModelInstance = pInstance->GetModelInstance();

	if ( a_bCollision )
	{
		ACollisionInstance2* pCollision = new ACollisionInstance2;
		pCollision->SetModelInstance( pModelInstance, TNULL );

		ACollisionInstance2::OwnerInfo oOwner;
		oOwner.m_eType  = 3;
		oOwner.m_pOwner = m_pTerrainInstance;
		pCollision->Create( pModelInstance, &oOwner, TFALSE, 0, TNULL );

		TPhysicsManager* pPhysics = TPhysicsManager::GetSingleton();

		for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
		{
			TPhysicsObjectContainer* pBody = GetBody( pCollision, i );

			if ( pBody && a_pListener )
				pPhysics->SetListener( pBody, a_pListener );

			if ( !( GetBodyLayer( pBody ) & 8 ) )
				pPhysics->SetCollisionLayer( pBody, 0x90 );
		}

		pInstance->SetCollisionInstance( pCollision );
	}

	pInstance->SetTransform( a_rMatrix );
	pInstance->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );

	return a_pRef->m_pInstance;
}

// $deBlob: FUNCTION 0057f4f0
void ALandmarkInstance::StartTransform()
{
	LandmarkData* pData = TSTATICCAST( LandmarkData, m_pTerrainInstance->m_pTypeData );

	m_uiFlags |= Flags_Completed;
	m_fTransformTimer = pData->m_fTransformTime;
	m_fFadeTimer      = 0.0f;

	if ( !IsABillBoard() )
		SetCollisionLayers( TTRUE );

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	pGameStatus->GetLevelData().AddScore( 0, m_iNumTriggers * TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x44 / 4 ] * m_iPaintPerTrigger, -1 );

	if ( pData->m_uiNumTriggers != 0 )
	{
		AGameStatus::GetSingleton()->GetLevelData().AddLandmarksCompleted( 0, 1 );
	}

	// The landmark counts as painted in the colour of its first trigger for the paint events
	m_uiCurrentColour = m_aTriggers ? TUINT8( m_aTriggers[ 0 ].m_iColour ) : 2;
	pGameStatus->OnBuildingPaintChanged( this );
	AChallengeManager::GetSingleton()->OnPaintEvent( this );
	m_uiCurrentColour = ABlobColourComponent::COLOUR_WHITE;

	AMusicManager::GetSingleton()->PlayMusicSFX( "landmark_paint", 1, &TVector4::VEC_ZERO, TNULL );

	m_uiFlags &= ~Flags_TriggersHidden;

	for ( TINT i = 0; i < m_iNumTriggers; i++ )
	{
		if ( !m_aTriggers[ i ].m_bPainted )
			m_aTriggers[ i ].Show();
	}

	for ( TINT i = 0; i < m_iNumTriggers; i++ )
		m_aTriggers[ i ].Disable();

	if ( m_oIdleModel.m_pInstance )
	{
		m_oIdleModel.m_pInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

		if ( m_pIdleFX )
			m_pIdleFX->SetVisible( TFALSE );

		m_oIdleModel.m_pInstance->GetAnimInterface()->StopAllAnims( 0.0f );
		GetCollision( m_oIdleModel.m_pInstance )->RemoveFromWorld( 0x60 );
	}

	if ( m_oTransformModel.m_pInstance )
	{
		AModelInstance2*      pTransform = m_oTransformModel.m_pInstance;
		AToshiAnimInterface2* pAnim      = pTransform->GetAnimInterface();
		pTransform->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );

		if ( m_iTransformId == -1 )
		{
			if ( m_iTransformIdleId != -1 )
				pAnim->AddAnim( TINT16( m_iTransformIdleId ), 0.0f, 1.0f, 1.0f );
		}
		else
		{
			pAnim->AddAnim( TINT16( m_iTransformId ), 0.0f, 1.0f, 1.0f );
			TASSERT( pAnim->GetAnimationLength( TINT16( m_iTransformId ) ) >= 0.0f );
			m_fTransformTimer = pAnim->GetAnimationLength( TINT16( m_iTransformId ) ) + m_fTransformTimer;
		}

		GetCollision( pTransform )->AddToWorld( 0x60 );
	}

	// The rings and the graphic around the landmark
	TVector4 vCentre  = *m_pTerrainInstance->m_ppCentre[ 0 ];
	TVector4 vExtents = *m_pTerrainInstance->m_ppCentre[ 1 ];

	TMatrix44 oMatrix = TMatrix44::IDENTITY;
	TVector4  vTop    = vCentre;
	vTop.y            = vCentre.y + vExtents.y;
	oMatrix.SetTranslation( vTop );

	TParticleEffectInstance* pRings = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pRingsEffect );

	if ( pRings )
		pRings->GetTransform() = oMatrix;

	TParticleEffectInstance* pGraphic = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pGraphicEffect );

	TVector4 vBottom = vCentre;
	vBottom.y        = vTop.y - vExtents.y * 2.0f;
	oMatrix.SetTranslation( vBottom );

	if ( pGraphic )
		pGraphic->GetTransform() = oMatrix;

	// The landmarks linked to this one get it as their parent (their paint glow)
	Links* pLinks = TSTATICCAST( Links, m_pTerrainInstance->m_pLinks );

	if ( pLinks != TNULL )
	{
		for ( TINT i = 0; i < pLinks->m_iNumInstances; i++ )
		{
			TerrainTypes::CellInstance* pLinked = pLinks->m_ppInstances[ i ];

			if ( ( pLinked->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Landmark )
				TSTATICCAST( ALandmarkInstance, pLinked->m_pModelInstance2 )->m_pParentLandmark = this;
		}
	}

	ABuildingManager::TriggerLinks( m_pTerrainInstance, AEntityIOPacket::MESSAGE_TOGGLE );
}

// $deBlob: FUNCTION 0057f920
void ALandmarkInstance::UpdateTriggers( TFLOAT a_fDeltaTime )
{
	// The "LandmarkTarget" HUD scenes over the triggers: the visible ones near the camera, their LOD by the distance,
	// Blob's colour on the colourless ones, projected to the screen
	AGameState*  pState   = AGameStateController::GetSingleton()->GetCurrentGameState();
	TBOOL        bInGame  = pState->IsA( &TGetClass( AInGameState ) );
	ABlobEntity* pBlob    = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	TINT         iZJState = pBlob->GetZJumpState();

	// Z-jumping at a trigger of this landmark: the scenes are hidden
	if ( iZJState == 10 || iZJState == 11 )
	{
		ABlobZJumpComponent::PriorityTarget* pLock = pBlob->GetZJumpComponent()->GetCurrentPriorityLock();

		if ( pLock != TNULL && pLock->m_eType == ABlobZJumpComponent::PriorityTarget::TARGET_LMARK && pLock->m_pTarget == this )
		{
			SetTargetScenesVisible( TFALSE );
			return;
		}
	}

	for ( TINT i = 0; i < m_iNumTriggers; i++ )
	{
		if ( !bInGame )
			continue;

		TASSERT( i < m_iNumTriggers );
		Trigger& rTrigger = m_aTriggers[ i ];

		TVector4 vTarget( 0.5f, 0.0f, 0.0f, 1.0f );
		TMatrix44::TransformVector( vTarget, rTrigger.m_oMatrix, vTarget );

		TSphere oSphere( vTarget.x, vTarget.y, vTarget.z, 0.5f );

		ACamera*        pCamera    = ACameraManager::GetSingleton()->GetCurrentCamera();
		const TVector4& rCameraPos = pCamera->GetMatrix().GetTranslation();
		TXUIScene*      pScene     = rTrigger.m_pScene;
		TFLOAT          fDistSqXZ  = ( vTarget.x - rCameraPos.x ) * ( vTarget.x - rCameraPos.x ) + ( vTarget.z - rCameraPos.z ) * ( vTarget.z - rCameraPos.z );

		TBOOL bShow = TFALSE;

		if ( !( m_uiFlags & Flags_TriggersHidden ) && rTrigger.m_iPaintLeft > 0 && fDistSqXZ < 40000.0f &&
		     TRenderContext::CullSphereToFrustumSimple( oSphere, pCamera->GetViewPlanes(), 6 ) )
		{
			// Not hidden behind the world
			TPhysicsRay oRay;
			oRay.m_vFrom            = rCameraPos;
			oRay.m_vTo              = vTarget;
			oRay.m_uiCollisionLayer = 16;

			TPhysicsShapeContact oContact;
			bShow = !TPhysicsManager::GetSingleton()->CollideRay( oRay, oContact, TNULL );
		}

		if ( bShow )
		{
			if ( !rTrigger.m_bPainted )
			{
				static TFLOAT s_fLowLODTime  = pScene->GetVisual()->GetTimelineControl()->GetNamedFrameTime( L"LowLOD" );
				static TFLOAT s_fHighLODTime = pScene->GetVisual()->GetTimelineControl()->GetNamedFrameTime( L"HighLOD" );

				TXUITimelineControl* pTimeline = pScene->GetVisual()->GetTimelineControl();

				if ( fDistSqXZ <= 2500.0f )
				{
					pTimeline->SetTimeLineTime( s_fHighLODTime, TFALSE );
				}
				else if ( fDistSqXZ < 22500.0f )
				{
					TFLOAT fLOD = ( fDistSqXZ - 2500.0f ) / 20000.0f;
					TMath::Clip( fLOD, 0.0f, 1.0f );
					fLOD = 1.0f - fLOD;
					pTimeline->SetTimeLineTime( ( s_fHighLODTime - s_fLowLODTime ) * fLOD + s_fLowLODTime, TFALSE );
				}
				else
				{
					pTimeline->SetTimeLineTime( s_fLowLODTime, TFALSE );
				}

				pScene->SetVisible( TTRUE );

				if ( rTrigger.m_iColour == 0 )
				{
					// A trigger of any colour shows Blob's colour (the light colours get the dark text)
					TUINT32 uiColour = pBlob->GetColourComponent()->GetCurrentColour();

					if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
						uiColour = ABlobColourComponent::COLOUR_BLACK;

					// NOTE: retail splits the colour into its bytes and joins them again (no change)
					TUINT32 uiFill = ABlobColourComponent::ms_auiColours[ uiColour ];
					SetFigureColour( rTrigger.m_pDropletLow, uiFill );
					SetFigureColour( rTrigger.m_pDropletHigh, uiFill );

					TBOOL bLight = ( uiColour == 1 || uiColour == 6 || uiColour == 5 );
					rTrigger.m_pTextPresenter1->SetVisible( bLight );
					rTrigger.m_pTextPresenter2->SetVisible( !bLight );
				}
			}
		}
		else
		{
			pScene->SetVisible( TFALSE );
		}

		if ( pScene->IsVisible() )
		{
			TVector4 vCanvas = vTarget;
			ARenderer::GetSingleton()->ProjectToCanvas( vCanvas, TNULL );

			TFLOAT fX = vCanvas.x - pScene->GetWidth() * 0.5f;
			TFLOAT fY = vCanvas.y - pScene->GetHeight() * 0.5f;
			pScene->SetPosition( fX, fY );
		}
	}
}

// $deBlob: FUNCTION 0057fec0
void ALandmarkInstance::ShowLetterbox()
{
	m_uiFlags &= ~Flags_WaitForLetterbox;

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

	TXUI* pXUI = TXUI::GetSingleton();
	pXUI->TransitionTo( TNULL, pXUI->CreateScene( pXUI->FindResource( "letterbox.xur" ), 0 ) );

	m_fBounceTimer = s_cfBounceTime;
	AChallengeManager::GetSingleton()->OnLandmarkEvent2( this );

	pBlob->HideLandmarkPrompt();

	if ( APlayerManager::GetSingleton()->GetBlobDataOf( pBlob )->IsKeyFrameCameraActive() )
	{
		m_fBounceTimer = TSTATICCAST( LandmarkData, m_pTerrainInstance->m_pTypeData )->m_fBlobDelay + m_fBounceTimer;
	}
	else
	{
		StartTransform();
		MatchMeshes();
		AddPaintPass();
	}
}

// $deBlob: FUNCTION 0057ff80
TFLOAT __stdcall ALandmarkInstance::GetDistanceSqToViewports( const TVector4& a_rPosition )
{
	AXYZViewportManager* pViewports = ARenderer::GetSingleton()->GetViewportManager();
	TFLOAT               fMin       = TMath::MAXFLOAT;

	for ( TINT i = 0; i < AXYZViewportManager::VIEWPORT_Count; i++ )
	{
		AXYZViewportManager::VIEWPORT eViewport = AXYZViewportManager::VIEWPORT( i );

		if ( pViewports->IsViewportUsed( eViewport ) )
		{
			ACamera* pCamera = pViewports->GetCamera( eViewport );

			if ( pCamera != TNULL )
			{
				TFLOAT fDistanceSq = TVector4::DistanceSq( pCamera->GetMatrix().GetTranslation(), a_rPosition );

				if ( fMin > fDistanceSq )
					fMin = fDistanceSq;
			}
		}
	}

	return fMin;
}
