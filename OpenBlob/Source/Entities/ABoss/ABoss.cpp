#include "pch.h"
#include "ABoss.h"
#include "Entities/APickupManager.h"
#include "Entities/APickup.h"
#include "Entities/ABossTrigger/ABossTrigger.h"
#include "Entities/AEntityParams.h"
#include "Entities/AEntityLoader.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/APropWater.h"
#include "Entities/APropGrassBomb.h"
#include "Entities/APropHazard/AContainerHazard.h"
#include "Entities/Helpers/ASpawnPointPool.h"
#include "Entities/Projectiles/AProjectile.h"
#include "Entities/Projectiles/AProjectileManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ABlobEntity/ABlobColourMaps.h"
#include "Entities/ABlobEntity/ABlobZJumpComponent.h"
#include "Animation/AModel2.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Collision/ACollisionCommon.h"
#include "Collision/ACollisionInstance2.h"
#include "Collision/ACollisionObject.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/APlayerManager.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Render/ARenderer.h"
#include "Sound/AMusicManager.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <Render/TAnimation.h>
#include <Render/TMesh.h>
#include <Render/TModel.h>
#include <Render/TRenderContext.h>
#include <Render/TSkeleton.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIScene.h>
#include <XUI/TXUIFigure.h>
#include <XUI/TXUIVisual.h>
#include <XUI/TXUITimelineControl.h>

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ABoss* ABoss::sm_pBoss;
TBOOL  ABoss::ms_bMaterialSetup;

TSTATICASSERT( sizeof( ABoss ) == 0x174 );
TSTATICASSERT( offsetof( ABoss::BossData, m_aRounds ) == 0x64 );
TSTATICASSERT( offsetof( ABoss::BossData, m_fTurnSpeed ) == 0xB0 );

// The animation a_iSequence of the boss, TNULL if it isn't playing (pI->IsAnimationPlaying in the asserts)
static TAnimation* IsAnimationPlaying( AToshiAnimInterface2* a_pAnimInterface, TINT16 a_iSequence )
{
	TSkeletonInstance* pSkeletonInstance = a_pAnimInterface->GetSkeletonInstance();
	return ( pSkeletonInstance != TNULL ) ? pSkeletonInstance->IsAnimating( a_iSequence ) : TNULL;
}

// $deBlob: FUNCTION 00772f60
TBOOL IsBlobUnavailable( ABlobEntity* a_pBlob )
{
	TINT eState = a_pBlob->GetBlobState();
	return eState == ABlobEntity::STATE_DEAD || eState == 8 || ( TUINT8( a_pBlob->GetBlobFlags() >> 19 ) & 1 ) || a_pBlob->GetUntargetableTime() > 0.0f;
}

static AEntityModelComponent* GetModelComp( AVisibleEntity* a_pEntity )
{
	return TSTATICCAST( AEntityModelComponent, a_pEntity->GetModelComponent() );
}

static ABossTrigger* GetBossTrigger( TUINT16 a_uiID )
{
	TASSERT( ABaseEntity::FindIDEntity( a_uiID )->IsA( &TGetClass( ABossTrigger ) ) );
	return TSTATICCAST( ABossTrigger, ABaseEntity::FindIDEntity( a_uiID ) );
}

// $deBlob: FUNCTION 0076ed30
// $deBlob: FUNCTION 0076ed40
// $deBlob: FUNCTION 0076edb0
// $deBlob: FUNCTION 0076ee10
// $deBlob: FUNCTION 0076ee20
// $deBlob: FUNCTION 00540920
// $deBlob: FUNCTION 00859cc0
TDEFINE_CLASS( ABoss );

// $deBlob: FUNCTION 0076ee30
ABoss::ABoss()
{
	m_iHatBone              = -1;
	m_iEyeBone              = -1;
	m_iLHandBone            = -1;
	m_iRHandBone            = -1;
	m_iLastShotBone         = -1;
	m_iHEyeAnimID           = -1;
	m_iVEyeAnimID           = -1;
	m_iHLeftHandAnimID      = -1;
	m_iVLeftHandAnimID      = -1;
	m_iShootLeftHandAnimID  = -1;
	m_iHRightHandAnimID     = -1;
	m_iVRightHandAnimID     = -1;
	m_iShootRightHandAnimID = -1;
	m_iBodyTurnAnimID       = -1;
	m_iLife                 = 50;

	m_pCollision            = TNULL;
	m_pVEyeAnim             = TNULL;
	m_pHEyeAnim             = TNULL;
	m_pHLeftHandAnim        = TNULL;
	m_pVLeftHandAnim        = TNULL;
	m_pHRightHandAnim       = TNULL;
	m_pVRightHandAnim       = TNULL;
	m_pBodyTurnAnim         = TNULL;
	m_fHEyeAnimLength       = 0.0f;
	m_fVEyeAnimLength       = 0.0f;
	m_fHLeftHandAnimLength  = 0.0f;
	m_fVLeftHandAnimLength  = 0.0f;
	m_fHRightHandAnimLength = 0.0f;
	m_fVRightHandAnimLength = 0.0f;
	m_fBodyTurnAnimLength   = 0.0f;
	m_fStunTimer            = -1.0f;
	ResetAimTargets();
	m_fChargeTime       = -1.0f;
	m_fShotTimer        = -1.0f;
	m_fFlashTime        = 0.0f;
	m_fBlobBehindTime   = 0.0f;
	m_fTurnAngle        = 0.0f;
	m_pWater            = TNULL;
	m_pContainerHazard  = TNULL;
	m_aiBossTriggers    = TNULL;
	m_pScene            = TNULL;
	m_iShotCount        = 0;
	m_uiNumBossTriggers = 0;
	m_uiRound           = 0;
	ms_bMaterialSetup   = TFALSE;

	m_Flags = ( m_Flags & ~FLAG_Dead ) | 0x2000000;
	TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType )
	[1] = 0x23;
	SetHash( TFALSE );

	TASSERT( TNULL == sm_pBoss );
	sm_pBoss = this;

	m_wszLife[ 0 ]    = 0;
	m_pDropletHigh    = TNULL;
	m_pDropletLow     = TNULL;
	m_pTextPresenter2 = TNULL;
	m_pTextPresenter1 = TNULL;
}

// $deBlob: FUNCTION 0076f100
// $deBlob: FUNCTION 00772e20
ABoss::~ABoss()
{
	delete[] m_aiBossTriggers;
	sm_pBoss = TNULL;
}

// The aim positions of the animations (ctor, Reset, OnLeaveState( STATE_Attack))
void ABoss::ResetAimTargets()
{
	m_fHEye             = 0.5f;
	m_fVEye             = 0.5f;
	m_fHLeftHand        = 0.36f;
	m_fHLeftHandTarget  = 0.36f;
	m_fVLeftHand        = 0.56f;
	m_fVLeftHandTarget  = 0.56f;
	m_fHRightHand       = 0.6f;
	m_fHRightHandTarget = 0.6f;
	m_fVRightHand       = 0.56f;
	m_fVRightHandTarget = 0.56f;
	m_fBodyTurn         = 0.5f;
}

// $deBlob: FUNCTION 0076f130
void ABoss::ProcessParams( const AEntityParams& a_rParams )
{
	AVisibleEntity::ProcessParams( a_rParams );

	TINT aShapeTypes[ 8 ] = { 4, 4, 4, 4, 4, 4, 4, 4 };

	AModelInstance2* pModelInstance2 = GetModelComp( this )->GetModelInstance();
	TModelInstance*  pModelInstance  = pModelInstance2->GetModelInstance();
	T2ModelPtr       pModel          = pModelInstance->GetModel();

	ACollisionInstance2* pCollision = new ACollisionInstance2();
	pCollision->SetModelInstance( pModelInstance, TNULL );

	ACollisionInstance2::OwnerInfo oOwner;
	oOwner.m_eType  = 2;
	oOwner.m_pOwner = this;
	pCollision->Create( pModelInstance, &oOwner, TTRUE, 8, aShapeTypes );
	pModelInstance2->SetCollisionInstance( pCollision );

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
	{
		TPhysicsObjectContainer* pObject = pCollision->GetPhysicsInfo( i ).m_pBody;
		TASSERT( ( TPhysicsManager::GetCollisionLayer( pObject ) & 8 ) == 0 );

		pPhysicsManager->SetCollisionLayer( pObject, ( TPhysicsManager::GetCollisionLayer( pObject ) & 0xFFFFFF6E ) | 2 );
		pPhysicsManager->SetCollisionGroup( pObject, 4 );

		TFLOAT fFriction    = 0.5f;
		TFLOAT fRestitution = 0.2f;
		ACollisionCommon::GetSurfaceParams( ACollisionCommon::SURFACE_Glass, fFriction, fRestitution );

		if ( pObject != TNULL )
		{
			pObject->m_uiSurface    = ACollisionCommon::SURFACE_Glass;
			pObject->m_fFriction    = fFriction;
			pObject->m_fRestitution = fRestitution;
		}
	}

	// The boss triggers, the water and the container hazard (sorted out by PostCreate)
	const AEntityLoader::EntityData& rEntityData = *TSTATICCAST( const AEntityLoader::EntityData, &a_rParams );
	m_uiNumBossTriggers                          = TUINT8( rEntityData.GetTargetCount() );

	TASSERT( TNULL == m_aiBossTriggers );
	m_aiBossTriggers = new TUINT16[ m_uiNumBossTriggers ];

	for ( TINT i = 0; i < m_uiNumBossTriggers; i++ )
		m_aiBossTriggers[ i ] = TUINT16( rEntityData.GetTarget( i ) );
}

// $deBlob: FUNCTION 0076f3f0
void ABoss::Create()
{
	AVisibleEntity::Create();
	CreateCollision();
}

// $deBlob: FUNCTION 007725f0
void ABoss::CreateCollision()
{
	if ( GetCollisionObject() == TNULL )
	{
		m_pCollision = new ACollisionObject( this, 1, 0xFF );
		GetCollisionObject()->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );
		m_pCollision->SetFlags( ACollisionObject::FLAG_Created );
	}

	// NOTE: retail writes the low byte only
	*TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType ) = 0x20;
}

// $deBlob: FUNCTION 0076f400
void ABoss::Destroy()
{
	if ( m_pCollision != TNULL )
		delete m_pCollision;

	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 0076f420
void ABoss::PostCreate()
{
	SetState( STATE_Inactive );

	// Keeps the boss triggers in m_aiBossTriggers, the other targets are the water and the container hazard
	for ( TINT i = 0; i < m_uiNumBossTriggers; i++ )
	{
		ABaseEntity* pTarget = ABaseEntity::FindIDEntity( m_aiBossTriggers[ i ] );

		if ( !pTarget->GetClass()->IsA( &TGetClass( ABossTrigger ) ) )
		{
			m_uiNumBossTriggers--;
			m_aiBossTriggers[ i ]                   = m_aiBossTriggers[ m_uiNumBossTriggers ];
			m_aiBossTriggers[ m_uiNumBossTriggers ] = 0xFFFF;
			i--;

			if ( pTarget->GetClass()->IsA( &TGetClass( APropWater ) ) )
			{
				TASSERT( TNULL == m_pWater );
				m_pWater = TSTATICCAST( APropWater, pTarget );
			}
			else if ( pTarget->GetClass()->IsA( &TGetClass( AContainerHazard ) ) )
			{
				TASSERT( TNULL == m_pContainerHazard );
				m_pContainerHazard = TSTATICCAST( AContainerHazard, pTarget );
			}
		}
	}

	TASSERT( TNULL != m_pWater, "The Boss must be linked to the ink/water!" );
	TASSERT( TNULL != m_pContainerHazard, "The Boss must be linked to a ContainerHazard for spawn points!" );

	AModelInstance2* pModelInstance = GetModelComp( this )->GetModelInstance();

	m_iHatBone = TINT16( pModelInstance->FindBone( "bossmech_m_head_tip" ) );
	TASSERT( -1 != m_iHatBone );
	m_iEyeBone = TINT16( pModelInstance->FindBone( "bossmech_cb_m_eye01" ) );
	TASSERT( -1 != m_iEyeBone );
	m_iLHandBone = TINT16( pModelInstance->FindBone( "bossmech_l_gun_tip" ) );
	TASSERT( -1 != m_iLHandBone );
	m_iRHandBone = TINT16( pModelInstance->FindBone( "bossmech_r_gun_tip" ) );
	TASSERT( -1 != m_iRHandBone );

	AToshiAnimInterface2* pAnimInterface = pModelInstance->GetAnimInterface();

	m_iHEyeAnimID = TINT16( pAnimInterface->GetAnimID( "H_Rotate_Eye" ) );
	TASSERT( m_iHEyeAnimID >= 0 );
	m_iVEyeAnimID = TINT16( pAnimInterface->GetAnimID( "P_Rotate_Eye" ) );
	TASSERT( m_iVEyeAnimID >= 0 );
	m_iHLeftHandAnimID = TINT16( pAnimInterface->GetAnimID( "H_Rotate_Larm" ) );
	TASSERT( m_iHLeftHandAnimID >= 0 );
	m_iVLeftHandAnimID = TINT16( pAnimInterface->GetAnimID( "P_Rotate_Larm" ) );
	TASSERT( m_iVLeftHandAnimID >= 0 );
	m_iShootLeftHandAnimID = TINT16( pAnimInterface->GetAnimID( "Shoot_Larm" ) );
	TASSERT( m_iShootLeftHandAnimID >= 0 );
	m_iHRightHandAnimID = TINT16( pAnimInterface->GetAnimID( "H_Rotate_Rarm" ) );
	TASSERT( m_iHRightHandAnimID >= 0 );
	m_iVRightHandAnimID = TINT16( pAnimInterface->GetAnimID( "P_Rotate_Rarm" ) );
	TASSERT( m_iVRightHandAnimID >= 0 );
	m_iShootRightHandAnimID = TINT16( pAnimInterface->GetAnimID( "Shoot_Rarm" ) );
	TASSERT( m_iShootRightHandAnimID >= 0 );
	m_iBodyTurnAnimID = TINT16( pAnimInterface->GetAnimID( "H_Rotate_Turn" ) );
	TASSERT( m_iBodyTurnAnimID >= 0 );

	m_fHEyeAnimLength       = pAnimInterface->GetAnimationLength( m_iHEyeAnimID );
	m_fVEyeAnimLength       = pAnimInterface->GetAnimationLength( m_iVEyeAnimID );
	m_fHLeftHandAnimLength  = pAnimInterface->GetAnimationLength( m_iHLeftHandAnimID );
	m_fVLeftHandAnimLength  = pAnimInterface->GetAnimationLength( m_iVLeftHandAnimID );
	m_fHRightHandAnimLength = pAnimInterface->GetAnimationLength( m_iHRightHandAnimID );
	m_fVRightHandAnimLength = pAnimInterface->GetAnimationLength( m_iVRightHandAnimID );
	m_fBodyTurnAnimLength   = pAnimInterface->GetAnimationLength( m_iBodyTurnAnimID );

	AProjectileManager::GetSingleton()->InitPool( AProjectileTypes::PROJECTILETYPES( 1 ), 6 );
	SetupRound();

	if ( !ms_bMaterialSetup )
	{
		// The colour changing material gets a second, white pass (tev mode 0x16: the hit flash, Tick STATE_StunStart)
		T2ModelPtr pModel = GetModelComp( this )->GetModelInstance()->GetModel()->GetModelPtr();
		TModelLOD& rLOD   = pModel->GetLOD( 0 );

		TASSERT( 1 == GetModelComp( this )->GetModelInstance()->GetModel()->GetModelPtr()->GetLODCount() );

		ASkinMaterialHAL* pSkinMat = TNULL;

		for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		{
			if ( TStringManager::String8CompareNoCase( rLOD.GetMeshes()[ i ]->GetMaterial()->GetName(), "ss_mat_colourchangerbit", -1 ) == 0 )
			{
				pSkinMat = TSTATICCAST( ASkinMaterialHAL, rLOD.GetMeshes()[ i ]->GetMaterial() );
				break;
			}
		}

		TTexture* pWhiteTexture = TTextureManager::GetSingleton()->GetWhiteTexture();
		pWhiteTexture->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

		TASSERT( pSkinMat->GetNumTextures() == 1 );
		pSkinMat->SetTextureNum( 2 );
		pSkinMat->SetTevMode( 0, 0 );
		TASSERT( pSkinMat->GetPassMode( 0 ) == ASkinMaterialHAL::PASSMODE_DEFAULT );
		pSkinMat->SetTevMode( 1, 0x16 );
		TASSERT( pSkinMat->GetPassMode( 1 ) == ASkinMaterialHAL::PASSMODE_DEFAULT );
		pSkinMat->SetTexture( 1, pWhiteTexture );
		pSkinMat->SetBlendMode( 13, 0 );

		ms_bMaterialSetup = TTRUE;
	}

	TXUIResource* pHud = TXUI::GetSingleton()->FindResource( "hud.xur" );

	if ( pHud != TNULL )
	{
		m_pScene = TXUI::GetSingleton()->CreateScene( pHud, L"LandmarkTarget", TFALSE );
		m_pScene->SetVisible( TFALSE );
		m_pScene->SetPresenterText( m_wszLife, 0, -1 );

		TXUIVisual* pVisual = m_pScene->GetVisual();
		pVisual->GetTimelineControl()->StopAnimation();

		TXUIElement* pShape = pVisual->FindChildXUIElement( L"DropletLow" );
		TASSERT( pShape->IsA( &TGetClass( TXUIFigure ) ) );
		m_pDropletLow = TSTATICCAST( TXUIFigure, pShape );

		pShape = pVisual->FindChildXUIElement( L"DropletHigh" );
		TASSERT( pShape->IsA( &TGetClass( TXUIFigure ) ) );
		m_pDropletHigh = TSTATICCAST( TXUIFigure, pShape );

		m_pTextPresenter1 = pVisual->FindChildXUIElement( L"XuiTextPresenter1" );
		m_pTextPresenter2 = pVisual->FindChildXUIElement( L"XuiTextPresenter2" );
	}
}

// $deBlob: FUNCTION 0076fe30
void ABoss::Tick( TFLOAT a_fDeltaTime )
{
	// The hit flash: Blob stays attached until it's over
	if ( 0.0f < m_fFlashTime )
	{
		m_fFlashTime -= a_fDeltaTime;

		if ( m_fFlashTime <= 0.0f )
		{
			m_fFlashTime = 0.0f;
			TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() )->SetLevelStartAttached( TFALSE );
			AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;
		}
	}

	switch ( m_iState )
	{
		case STATE_Intro:
			if ( HasStateFinished() )
			{
				AGameStatus::GetSingleton()->ResetFreeProjections();
				AMusicManager::GetSingleton()->SetBossLevel( m_uiRound );
				m_uiBossFlags &= ~BOSSFLAG_MusicStarted;
				SetState( STATE_RoundStart );
			}
			break;
		case STATE_RoundStart:
			if ( HasStateFinished() )
			{
				SetState( STATE_RoundStarted );
				RaiseBossTriggers();
			}
			break;
		case STATE_RoundStarted:
		case STATE_ChargeEnd:
		case STATE_TurnEnd:
			if ( HasStateFinished() )
				SetState( STATE_Attack );
			break;
		case STATE_Attack:
		{
			if ( 0.0f < m_fShotTimer )
				m_fShotTimer -= a_fDeltaTime;

			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() );

			if ( IsBlobUnavailable( pBlob ) )
			{
				m_uiBossFlags &= ~BOSSFLAG_ShotFired;
				m_fChargeTime   = 10.0f;
				m_fShotTimer    = -1.0f;
				m_iShotCount    = 0;
				m_iLastShotBone = -1;
				SetState( STATE_ChargeStart );
				break;
			}

			UpdateBodyTurn( a_fDeltaTime, pBlob );

			if ( GetBossData()->m_bTurnToBlob )
			{
				if ( m_uiBossFlags & BOSSFLAG_BlobBehind )
				{
					m_fBlobBehindTime += a_fDeltaTime;

					if ( m_fBlobBehindTime > GetBossData()->m_fTurnDelay )
					{
						SetState( STATE_TurnStart );
						break;
					}
				}
				else
				{
					m_fBlobBehindTime = 0.0f;
				}
			}

			UpdateHand( a_fDeltaTime, &m_iLHandBone, pBlob, m_pVLeftHandAnim, &m_fVLeftHandTarget, &m_fVLeftHand, &m_fVLeftHandAnimLength, m_pHLeftHandAnim, &m_fHLeftHandTarget, &m_fHLeftHand, &m_fHLeftHandAnimLength, &m_iShootLeftHandAnimID, TTRUE );

			if ( m_iState == STATE_Attack )
			{
				UpdateHand( a_fDeltaTime, &m_iRHandBone, pBlob, m_pVRightHandAnim, &m_fVRightHandTarget, &m_fVRightHand, &m_fVRightHandAnimLength, m_pHRightHandAnim, &m_fHRightHandTarget, &m_fHRightHand, &m_fHRightHandAnimLength, &m_iShootRightHandAnimID, TFALSE );

				if ( m_iState == STATE_Attack )
					UpdateEye( a_fDeltaTime, pBlob );
			}

			break;
		}
		case STATE_ChargeStart:
			if ( HasStateFinished() )
				SetState( STATE_Charge );
			break;
		case STATE_Charge:
			TASSERT( m_fChargeTime > 0.f );
			m_fChargeTime -= a_fDeltaTime;

			if ( m_fChargeTime < 0.0f )
				SetState( STATE_ChargeEnd );
			break;
		case STATE_TurnStart:
		{
			if ( HasStateFinished() )
			{
				m_fTurnAngle = GetBossData()->m_fTurnAngle * 0.017453292f;
				SetState( STATE_Turn );
			}

			TVector4 vCurFacing = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
			vCurFacing.w        = 1.0f;
			TASSERT( vCurFacing.y == 0.f && vCurFacing.IsNormalised() );

			ABaseEntity* pBlob   = APlayerManager::GetSingleton()->GetCurrentPlayerBlob();
			TVector4     vToBlob = pBlob->GetMatrix().GetTranslation() - m_oMatrix.GetTranslation();
			vToBlob.w            = pBlob->GetMatrix().m_f44;
			vToBlob.NormaliseXZ();

			TFLOAT fDot = vCurFacing.x * vToBlob.x + vToBlob.y * vCurFacing.y + vCurFacing.z * vToBlob.z;

			if ( 0.0f < fDot )
			{
				if ( 0.707f <= fDot )
				{
					SetState( STATE_TurnEnd );
				}
				else
				{
					TFLOAT fAngle = TMath::ACos( vCurFacing.x * vToBlob.x + vToBlob.y * vCurFacing.y + vCurFacing.z * vToBlob.z );
					m_fTurnAngle  = fAngle;

					// Blob is on the left: the other way round
					if ( m_oMatrix.m_f12 * vToBlob.y + vToBlob.x * m_oMatrix.m_f11 + m_oMatrix.m_f13 * vToBlob.z < 0.0f )
						m_fTurnAngle = 6.2831855f - fAngle;

					SetState( STATE_Turn );
				}
			}

			break;
		}
		case STATE_Turn:
		{
			TFLOAT fStep = GetBossData()->m_fTurnSpeed * 0.017453292f * a_fDeltaTime;

			if ( fStep > m_fTurnAngle )
				fStep = m_fTurnAngle;

			TVector4 vCurFacing = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
			vCurFacing.w        = 1.0f;
			TASSERT( vCurFacing.y == 0.f && vCurFacing.IsNormalised() );
			vCurFacing.RotateY( fStep );
			TASSERT( vCurFacing.y == 0.f && vCurFacing.IsNormalised() );
			SetFacing( vCurFacing );

			m_fTurnAngle -= fStep;

			if ( m_fTurnAngle < 0.001f )
				SetState( STATE_TurnStart );

			break;
		}
		case STATE_StunStart:
		{
			// The hit flash in Blob's colour
			TUINT32 uiColour = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() )->GetPaintColour();

			if ( TUINT8( uiColour >> 4 ) & 1 )
				uiColour = 0x10;

			TVector4 vColour;
			UnpackColour( ABlobColourMaps::ms_pLandmarkGlowColourMap[ uiColour ], vColour );
			ASkinShaderHAL::s_vTevMode16Const  = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
			ASkinShaderHAL::s_vTevMode16Colour = vColour;

			m_fFlashTime = 1.5f;
			TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() )->SetLevelStartAttached( TTRUE );
			AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TTRUE;

			if ( HasStateFinished() )
				SetState( STATE_Stunned );
			break;
		}
		case STATE_Stunned:
		{
			UpdateHUD();

			m_fStunTimer -= a_fDeltaTime;

			if ( m_fStunTimer < 0.0f )
				SetState( STATE_Recover );

			// The material flashes in Blob's colour (fast for the last 2 seconds, slowly from 5 to 2 seconds)
			TFLOAT fTime     = m_fStunTimer;
			TBOOL  bFlashOff = TFALSE;

			if ( fTime < 2.0f && fmod( fTime, 0.2f ) < 0.1 )
				bFlashOff = TTRUE;
			else if ( fTime > 2.0f && fTime < 5.0f && fmod( fTime, 0.5 ) < 0.1 )
				bFlashOff = TTRUE;

			if ( !bFlashOff )
			{
				TUINT32 uiColour = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() )->GetPaintColour();

				if ( TUINT8( uiColour >> 4 ) & 1 )
					uiColour = 0x10;

				TVector4 vColour;
				UnpackColour( ABlobColourMaps::ms_pLandmarkGlowColourMap[ uiColour ], vColour );
				ASkinShaderHAL::s_vTevMode16Const  = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
				ASkinShaderHAL::s_vTevMode16Colour = vColour;
			}
			else
			{
				ASkinShaderHAL::s_vTevMode16Const = TVector4( 0.0f, 0.0f, 0.0f, 0.0f );
			}

			break;
		}
		case STATE_Recover:
			if ( HasStateFinished() )
			{
				for ( TINT i = 0; i < m_uiNumBossTriggers; i++ )
				{
					ABossTrigger* pTrigger = GetBossTrigger( m_aiBossTriggers[ i ] );

					if ( pTrigger->GetBossStage() == m_uiRound )
						pTrigger->RaiseAgain();

					if ( pTrigger->GetBossStage() <= m_uiRound )
						pTrigger->SetHazardsActive( TTRUE );

					pTrigger->DespawnTargets( TFALSE );
				}

				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
				oPacket.m_iData    = 0;
				m_pWater->HandleEntityIO( this, oPacket );
				m_pWater->Tick( a_fDeltaTime );

				SetState( STATE_Attack );
			}
			break;
		case STATE_Hit:
			if ( HasStateFinished() )
				SetState( STATE_HitHold );
			break;
		case STATE_HitHold:
			TASSERT( ABlobEntity::STATE_BOSS == TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() )->GetBlobState() );
			break;
		case STATE_HitRecover:
			if ( HasStateFinished() )
				SetState( STATE_Stunned );
			break;
		case STATE_RoundEnd:
			if ( HasStateFinished() )
			{
				m_uiRound++;
				AMusicManager::GetSingleton()->SetBossLevel( m_uiRound );

				if ( m_uiRound < BOSSFIGHT_NUM_ROUNDS )
				{
					m_iLife = TINT16( GetMaxLife() );
					SetState( STATE_NextRound );
					SetupRound();

					// A pickup comes out of a spawn point of the container hazard
					ASpawnPointPool::SpawnPoint* pSpawnPoint = m_pContainerHazard->GetSpawnPoints()->GetSpawnPoint( m_oMatrix.GetTranslation(), m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD ) );

					if ( pSpawnPoint != TNULL )
					{
						TVector4 vEyePosition;
						GetEyePosition( vEyePosition );

						APickup* pPickup = APickupManager::CreatePickup( 2, vEyePosition, -1.0f, 30.0f );
						TASSERT( pPickup == TNULL || pPickup->IsA( &TGetClass( APickup ) ) );

						if ( pPickup != TNULL )
						{
							// It flies to the spawn point, which is released once the pickup is gone
							pPickup->Fly( vEyePosition, pSpawnPoint->m_vPosition, -1.0f, 0 );
							pPickup->SetCallback( ASpawnPointPool::ReleaseSpawnPointCallback, pSpawnPoint );
						}
						else
						{
							m_pContainerHazard->GetSpawnPoints()->ReleaseSpawnPoint( pSpawnPoint );
						}
					}
				}
				else
				{
					SetState( STATE_Defeated );
				}
			}
			break;
		case STATE_NextRound:
			if ( HasStateFinished() )
			{
				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
				oPacket.m_iData    = 0;
				m_pWater->HandleEntityIO( this, oPacket );
				m_pWater->Tick( a_fDeltaTime );

				SetState( STATE_Attack );
				RaiseBossTriggers();
			}
			break;
		case STATE_Defeated:
			if ( HasStateFinished() )
			{
				if ( !( m_uiBossFlags & BOSSFLAG_MusicStarted ) )
				{
					m_uiBossFlags |= BOSSFLAG_MusicStarted;

					APropGrassBomb* pGrassBomb = AGameStatus::GetSingleton()->GetBossGrassBomb();

					if ( pGrassBomb != TNULL )
						pGrassBomb->TryActivate();
				}

				SetState( STATE_DefeatedEnd );
			}
			break;
		default:
			break;
	}

	AVisibleEntity::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00770950
void ABoss::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TRIGGER )
	{
		if ( m_iState == STATE_Inactive )
			SetState( STATE_Intro );
	}
	else if ( ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TOUCH || a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_SLAM ) && m_iState == STATE_Stunned )
	{
		// Blob's Z-jump into the stunned boss
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCaller );

		if ( ( pBlob->GetFlags() & FLAG_Player ) && pBlob->GetZJumpComponent()->GetState() == ABlobZJumpComponent::STATE_ATTACKFLIGHT )
		{
			TVector4 vFacing = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
			vFacing.x        = -vFacing.x;
			vFacing.y        = -vFacing.y;
			vFacing.z        = -vFacing.z;
			vFacing.w        = 1.0f;
			pBlob->SetFacing( vFacing );

			TPhysicsManager::GetSingleton()->StopBody( pBlob->GetBody() );
			AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TTRUE;
			pBlob->EnterBoss( this );
			SetState( STATE_Hit );
		}
	}

	AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 00770a30
TBOOL ABoss::IsTargetable()
{
	if ( m_iState != STATE_Stunned )
		return TFALSE;

	TASSERT( GetLife() > 0 );

	ABlobEntity*         pBlob  = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() );
	ABlobZJumpComponent* pZJump = pBlob->GetZJumpComponent();

	if ( pZJump->GetState() == ABlobZJumpComponent::STATE_ATTACKFLIGHT )
	{
		ABlobZJumpComponent::PriorityTarget* pLock = pZJump->GetCurrentPriorityLock();

		if ( pLock->m_eType == 0 && this == pLock->m_pTarget )
			return TTRUE;
	}

	// Blob is in front of the point 22 units ahead of the boss
	TVector4 vAhead = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vAhead.Multiply( 22.0f );

	const TVector4& rFacing   = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	const TVector4& rPosition = m_oMatrix.GetTranslation();
	const TVector4& rBlob     = pBlob->GetMatrix().GetTranslation();

	if ( rFacing.y * ( ( rPosition.y + vAhead.y ) - rBlob.y ) + ( ( rPosition.x + vAhead.x ) - rBlob.x ) * rFacing.x + rFacing.z * ( ( rPosition.z + vAhead.z ) - rBlob.z ) < 0.0f )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 00770b20
void ABoss::Reset( ABaseEntity* a_pEntity )
{
	ResetAimTargets();
	m_fChargeTime = -1.0f;
	m_fShotTimer  = -1.0f;
	m_uiRound     = 0;
	SetupRound();
	SetState( STATE_Inactive );

	TINT iLife = GetMaxLife();
	m_Flags &= ~FLAG_Dead;
	m_iLife = TINT16( iLife );

	AGameStatus::GetSingleton()->ClearProjections();
	ASkinShaderHAL::s_vTevMode16Const = TVector4( 0.0f, 0.0f, 0.0f, 0.0f );

	AVisibleEntity::Reset( a_pEntity );
}

// $deBlob: FUNCTION 00770c10
TVector4 ABoss::GetTargetOffset()
{
	TMatrix44 mBone;
	GetModelComp( this )->GetModelInstance()->GetBoneTransform( m_iEyeBone, mBone );

	TVector4 vBase = GetBasePosition();
	TVector4 vResult;
	vResult.x = mBone.m_f41 - vBase.x;
	vResult.y = mBone.m_f42 - vBase.y;
	vResult.z = mBone.m_f43 - vBase.z;
	vResult.w = mBone.m_f44;

	return vResult;
}

// $deBlob: FUNCTION 00770c90
void ABoss::GetEyeFrontPosition( TVector4& a_rOutPosition, TFLOAT a_fDistance )
{
	TMatrix44 mBone;
	GetModelComp( this )->GetModelInstance()->GetBoneTransform( m_iEyeBone, mBone );

	TFLOAT fDistance = a_fDistance + 5.0f;
	a_rOutPosition.x = fDistance * m_oMatrix.m_f31 + mBone.m_f41;
	a_rOutPosition.y = m_oMatrix.m_f32 * fDistance + mBone.m_f42;
	a_rOutPosition.z = m_oMatrix.m_f33 * fDistance + mBone.m_f43;
	a_rOutPosition.w = mBone.m_f44;
}

// $deBlob: FUNCTION 00770d10
void ABoss::GetEyePosition( TVector4& a_rOutPosition )
{
	TMatrix44 mBone;
	GetModelComp( this )->GetModelInstance()->GetBoneTransform( m_iEyeBone, mBone );

	a_rOutPosition = mBone.GetTranslation();
}

// $deBlob: FUNCTION 00770d50
void ABoss::OnBossTriggerCompleted()
{
	// Every boss trigger of the round must be complete
	for ( TINT i = 0; i < m_uiNumBossTriggers; i++ )
	{
		ABossTrigger* pTrigger = GetBossTrigger( m_aiBossTriggers[ i ] );

		if ( pTrigger->GetBossStage() == m_uiRound && !pTrigger->IsCompleted() )
			return;
	}

	m_fStunTimer = GetBossData()->m_fStunTime;
	SetState( STATE_StunStart );
	AProjectileManager::GetSingleton()->KillAllActive( AProjectileTypes::PROJECTILETYPES( 1 ) );

	for ( TINT i = 0; i < m_uiNumBossTriggers; i++ )
	{
		ABossTrigger* pTrigger = GetBossTrigger( m_aiBossTriggers[ i ] );

		if ( pTrigger->GetBossStage() == m_uiRound )
			pTrigger->ActivateSpawners();

		pTrigger->SetHazardsActive( TFALSE );
	}

	AEntityIOPacket oPacket;
	oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
	oPacket.m_iData    = 0;
	m_pWater->HandleEntityIO( this, oPacket );
}

// $deBlob: FUNCTION 00770ff0
void ABoss::OnHit( TBOOL a_bKilled )
{
	if ( a_bKilled )
	{
		SetState( STATE_RoundEnd );

		for ( TINT i = 0; i < m_uiNumBossTriggers; i++ )
			GetBossTrigger( m_aiBossTriggers[ i ] )->DespawnTargets( m_uiRound == BOSSFIGHT_NUM_ROUNDS - 1 );
	}
	else
	{
		TASSERT( GetLife() > 0 );
		SetState( STATE_HitRecover );
	}
}

// $deBlob: FUNCTION 00771170
TBOOL ABoss::GetStunCamera( TVector4* a_pOutPosition, TQuaternion* a_pOutOrientation, TFLOAT* a_pOutFOV, TFLOAT* a_pOutFOVSpeed, TBOOL* a_pOutSnap )
{
	if ( m_iState != STATE_StunStart && ( m_iState != STATE_Stunned || m_fFlashTime <= 0.0f ) )
		return TFALSE;

	*a_pOutSnap     = TFALSE;
	*a_pOutFOV      = ( TMath::HALF_PI * 60.0f ) / 90.0f;
	*a_pOutFOVSpeed = TMath::HALF_PI;

	// 100 units in front of the boss, 30 below it, looking at the point 40 below it
	*a_pOutPosition = m_oMatrix.GetTranslation();

	TVector4 vFacing = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vFacing.NormaliseXZ();
	vFacing.Multiply( 100.0f );

	a_pOutPosition->x = a_pOutPosition->x + vFacing.x;
	a_pOutPosition->z = a_pOutPosition->z + vFacing.z;
	a_pOutPosition->y = ( a_pOutPosition->y + vFacing.y ) - 30.0f;

	TVector4 vTarget = m_oMatrix.GetTranslation();
	vTarget.y        = m_oMatrix.m_f42 - 40.0f;

	TMatrix44 mLookAt        = TMatrix44::IDENTITY;
	mLookAt.GetTranslation() = *a_pOutPosition;
	mLookAt.LookAtTarget( vTarget, mLookAt.GetTranslation() );

	TMatrix44::MatToQuat( *a_pOutOrientation, mLookAt );
	*a_pOutOrientation /= TMath::Sqrt( a_pOutOrientation->x * a_pOutOrientation->x + a_pOutOrientation->y * a_pOutOrientation->y + a_pOutOrientation->z * a_pOutOrientation->z + a_pOutOrientation->w * a_pOutOrientation->w );

	return TTRUE;
}

// $deBlob: FUNCTION 00771330
const TCHAR* ABoss::GetModelName()
{
	return GetTypeData()->m_pTRBData->m_szModelName;
}

// $deBlob: FUNCTION 00771340
void ABoss::OnLeaveState( TINT a_iState )
{
	if ( m_iState != STATE_Inactive )
	{
		if ( m_iState == STATE_Attack )
		{
			AToshiAnimInterface2* pI = GetModelComp( this )->GetModelInstance()->GetAnimInterface();

			TASSERT( m_pHLeftHandAnim == IsAnimationPlaying( pI, m_iHLeftHandAnimID ) );
			pI->StopAnim( m_iHLeftHandAnimID, 0.3f );
			m_pHLeftHandAnim = TNULL;

			TASSERT( m_pVLeftHandAnim == IsAnimationPlaying( pI, m_iVLeftHandAnimID ) );
			pI->StopAnim( m_iVLeftHandAnimID, 0.3f );
			m_pVLeftHandAnim = TNULL;

			TASSERT( m_pHRightHandAnim == IsAnimationPlaying( pI, m_iHRightHandAnimID ) );
			pI->StopAnim( m_iHRightHandAnimID, 0.3f );
			m_pHRightHandAnim = TNULL;

			TASSERT( m_pVRightHandAnim == IsAnimationPlaying( pI, m_iVRightHandAnimID ) );
			pI->StopAnim( m_iVRightHandAnimID, 0.3f );
			m_pVRightHandAnim = TNULL;

			TASSERT( m_pVEyeAnim == IsAnimationPlaying( pI, m_iVEyeAnimID ) );
			pI->StopAnim( m_iVEyeAnimID, 0.3f );
			m_pVEyeAnim = TNULL;

			TASSERT( m_pHEyeAnim == IsAnimationPlaying( pI, m_iHEyeAnimID ) );
			pI->StopAnim( m_iHEyeAnimID, 0.3f );
			m_pHEyeAnim = TNULL;

			TASSERT( m_pBodyTurnAnim == IsAnimationPlaying( pI, m_iBodyTurnAnimID ) );
			pI->StopAnim( m_iBodyTurnAnimID, 0.3f );
			m_pBodyTurnAnim = TNULL;

			ResetAimTargets();
			m_fShotTimer = -1.0f;
		}
		else if ( m_iState == STATE_Stunned )
		{
			m_pScene->SetVisible( TFALSE );
		}
	}
	else
	{
		SetVisible( TTRUE );
		m_Flags &= ~FLAG_DontTick;
	}
}

// $deBlob: FUNCTION 00771720
void ABoss::OnEnterState( TINT a_iState )
{
	switch ( m_iState )
	{
		case STATE_Inactive:
			SetVisible( TFALSE );
			m_Flags |= FLAG_DontTick;
			break;
		case STATE_Intro:
		{
			TMatrix44 mTransform;
			GetModelComp( this )->GetModelInstance()->GetTransform( mTransform );
			APlayerManager::GetSingleton()->StartBossCamera( 0, mTransform, 0 );
			break;
		}
		case STATE_Attack:
		{
			AToshiAnimInterface2* pI = GetModelComp( this )->GetModelInstance()->GetAnimInterface();

			TASSERT( TNULL == IsAnimationPlaying( pI, m_iHLeftHandAnimID ) );
			m_pHLeftHandAnim = pI->AddAnim( m_iHLeftHandAnimID, 0.1f, 1.0f, 0.0f );
			m_pHLeftHandAnim->SetTime( m_fHLeftHand * m_fHLeftHandAnimLength );

			TASSERT( TNULL == IsAnimationPlaying( pI, m_iVLeftHandAnimID ) );
			m_pVLeftHandAnim = pI->AddAnim( m_iVLeftHandAnimID, 0.1f, 1.0f, 0.0f );
			m_pVLeftHandAnim->SetTime( m_fVLeftHand * m_fVLeftHandAnimLength );

			TASSERT( TNULL == IsAnimationPlaying( pI, m_iHRightHandAnimID ) );
			m_pHRightHandAnim = pI->AddAnim( m_iHRightHandAnimID, 0.1f, 1.0f, 0.0f );
			m_pHRightHandAnim->SetTime( ( 1.0f - m_fHRightHand ) * m_fHRightHandAnimLength );

			TASSERT( TNULL == IsAnimationPlaying( pI, m_iVRightHandAnimID ) );
			m_pVRightHandAnim = pI->AddAnim( m_iVRightHandAnimID, 0.1f, 1.0f, 0.0f );
			m_pVRightHandAnim->SetTime( m_fVRightHand * m_fVRightHandAnimLength );

			TASSERT( TNULL == IsAnimationPlaying( pI, m_iVEyeAnimID ) );
			m_pVEyeAnim = pI->AddAnim( m_iVEyeAnimID, 0.1f, 1.0f, 0.0f );
			m_pVEyeAnim->SetTime( m_fVEye * m_fVEyeAnimLength );

			TASSERT( TNULL == IsAnimationPlaying( pI, m_iHEyeAnimID ) );
			m_pHEyeAnim = pI->AddAnim( m_iHEyeAnimID, 0.1f, 1.0f, 0.0f );
			m_pHEyeAnim->SetTime( m_fHEye * m_fHEyeAnimLength );

			TASSERT( TNULL == IsAnimationPlaying( pI, m_iBodyTurnAnimID ) );
			m_pBodyTurnAnim = pI->AddAnim( m_iBodyTurnAnimID, 0.1f, 1.0f, 0.0f );
			m_pBodyTurnAnim->SetTime( m_fBodyTurn * m_fBodyTurnAnimLength );

			m_uiBossFlags &= ~BOSSFLAG_ShotFired;
			m_fChargeTime     = -1.0f;
			m_iLastShotBone   = -1;
			m_fShotTimer      = -1.0f;
			m_fBlobBehindTime = 0.0f;
			break;
		}
		case STATE_Recover:
		case STATE_NextRound:
			ASkinShaderHAL::s_vTevMode16Const = TVector4( 0.0f, 0.0f, 0.0f, 0.0f );
			break;
		case STATE_Defeated:
		{
			TMatrix44 mTransform;
			GetModelComp( this )->GetModelInstance()->GetTransform( mTransform );
			APlayerManager::GetSingleton()->StartBossCamera( 0, mTransform, 1 );
			ASkinShaderHAL::s_vTevMode16Const = TVector4( 0.0f, 0.0f, 0.0f, 0.0f );
			break;
		}
		default:
			break;
	}
}

// $deBlob: FUNCTION 00771c40
void ABoss::UpdateBodyTurn( TFLOAT a_fDeltaTime, ABlobEntity* a_pBlob )
{
	TVector4 vCurFacing = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vCurFacing.w        = 1.0f;
	TASSERT( vCurFacing.y == 0.f && vCurFacing.IsNormalised() );

	TVector4 vToBlob = a_pBlob->GetMatrix().GetTranslation() - m_oMatrix.GetTranslation();
	vToBlob.w        = a_pBlob->GetMatrix().m_f44;
	vToBlob.NormaliseXZ();

	TFLOAT fDot = vCurFacing.x * vToBlob.x + vToBlob.y * vCurFacing.y + vCurFacing.z * vToBlob.z;

	// Facing Blob: the body animation goes back to the middle
	if ( fDot >= 0.998f )
	{
		TFLOAT fStep = 1.5f * a_fDeltaTime;

		if ( m_fBodyTurn >= 0.5f )
			TMath::DecrMinf( m_fBodyTurn, fStep, 0.5f );
		else
			TMath::IncrMaxf( m_fBodyTurn, fStep, 0.5f );

		m_pBodyTurnAnim->SetTime( m_fBodyTurnAnimLength * m_fBodyTurn );
		return;
	}

	if ( fDot <= -0.9999f )
		fDot = -0.9999f;

	if ( fDot >= 0.0f )
		m_uiBossFlags &= ~BOSSFLAG_BlobBehind;
	else
		m_uiBossFlags |= BOSSFLAG_BlobBehind;

	TFLOAT fAngle = TMath::ACos( fDot );
	TFLOAT fStep  = 0.9f * a_fDeltaTime;

	if ( fabs( fAngle ) <= 0.2f )
	{
		if ( m_fBodyTurn < 0.5f )
			TMath::IncrMaxf( m_fBodyTurn, fStep, 0.5f );
		else
			TMath::DecrMinf( m_fBodyTurn, fStep, 0.5f );
	}
	else if ( !( m_uiBossFlags & BOSSFLAG_TurnRight ) )
	{
		TMath::DecrMinf( m_fBodyTurn, fStep, 0.0f );
	}
	else
	{
		TMath::IncrMaxf( m_fBodyTurn, fStep, 1.0f );
	}

	m_pBodyTurnAnim->SetTime( m_fBodyTurnAnimLength * m_fBodyTurn );

	TFLOAT fTurn = fAngle;

	if ( fAngle > m_fTurnSpeed * a_fDeltaTime )
		fTurn = m_fTurnSpeed * a_fDeltaTime;

	TUINT8 uiFlags = m_uiBossFlags;

	if ( uiFlags & BOSSFLAG_BlobBehind )
	{
		// Keeps turning the same way
		if ( uiFlags & BOSSFLAG_TurnRight )
			fTurn = -fTurn;
	}
	else
	{
		if ( m_oMatrix.m_f12 * vToBlob.y + m_oMatrix.m_f11 * vToBlob.x + m_oMatrix.m_f13 * vToBlob.z >= 0.0f )
		{
			uiFlags &= ~BOSSFLAG_TurnRight;
		}
		else
		{
			fTurn = -fTurn;
			uiFlags |= BOSSFLAG_TurnRight;
		}

		m_uiBossFlags = uiFlags;
	}

	vCurFacing.RotateY( fTurn );
	SetFacing( vCurFacing );
}

// $deBlob: FUNCTION 00771f20
void ABoss::UpdateEye( TFLOAT a_fDeltaTime, ABlobEntity* a_pBlob )
{
	AToshiAnimInterface2* pI = GetModelComp( this )->GetModelInstance()->GetAnimInterface();

	TMatrix44 mBone;
	GetModelComp( this )->GetModelInstance()->GetBoneTransform( m_iEyeBone, mBone );

	TVector4 vDir = mBone.GetTranslation() - a_pBlob->GetMatrix().GetTranslation();
	vDir.w        = mBone.m_f44;
	vDir.Normalise();

	TASSERT( m_pVEyeAnim == IsAnimationPlaying( pI, m_iVEyeAnimID ) );
	TASSERT( m_pHEyeAnim == IsAnimationPlaying( pI, m_iHEyeAnimID ) );

	// Vertical
	TFLOAT fTarget = TMath::ACos( vDir.y ) / TMath::PI;

	if ( fTarget <= m_fVEye )
		TMath::DecrMinf( m_fVEye, a_fDeltaTime * 0.15f, fTarget );
	else
		TMath::IncrMaxf( m_fVEye, a_fDeltaTime * 0.15f, fTarget );

	m_pVEyeAnim->SetTime( m_fVEyeAnimLength * m_fVEye );

	// Horizontal
	vDir.NormaliseXZ();
	TFLOAT fCos = -( m_oMatrix.m_f12 * vDir.y + m_oMatrix.m_f11 * vDir.x + m_oMatrix.m_f13 * vDir.z );
	TMath::Clip( fCos, -1.0f, 1.0f );
	fTarget = TMath::ACos( fCos ) / TMath::PI;

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail compares the horizontal target with the vertical position (m_fVEye), so the horizontal eye position
	// snaps to its target instead of turning when they're on different sides
	TFLOAT fCurrent = m_fHEye;
#else
	TFLOAT fCurrent = m_fVEye;
#endif

	if ( fTarget <= fCurrent )
		TMath::DecrMinf( m_fHEye, a_fDeltaTime * 0.2f, fTarget );
	else
		TMath::IncrMaxf( m_fHEye, a_fDeltaTime * 0.2f, fTarget );

	m_pHEyeAnim->SetTime( m_fHEyeAnimLength * m_fHEye );
}

// $deBlob: FUNCTION 00772170
void ABoss::UpdateHand( TFLOAT a_fDeltaTime, TINT16* a_pBone, ABlobEntity* a_pBlob, TAnimation* a_pVAnim, TFLOAT* a_pfVTarget, TFLOAT* a_pfV, TFLOAT* a_pfVLength, TAnimation* a_pHAnim, TFLOAT* a_pfHTarget, TFLOAT* a_pfH, TFLOAT* a_pfHLength, TINT16* a_pShootAnimID, TBOOL a_bLeft )
{
	AToshiAnimInterface2* pI = GetModelComp( this )->GetModelInstance()->GetAnimInterface();

	TMatrix44 mBone;
	GetModelComp( this )->GetModelInstance()->GetBoneTransform( *a_pBone, mBone );

	TVector4 vDir = mBone.GetTranslation() - a_pBlob->GetMatrix().GetTranslation();
	vDir.w        = mBone.m_f44;
	vDir.Normalise();

	TVector4 vShotDir  = vDir;
	TBOOL    bOnTarget = TTRUE;

	// Vertical (the target is smoothed once it's close)
	TFLOAT fTargetV = TMath::ACos( vDir.y ) / TMath::PI;
	TFLOAT fOldV    = *a_pfVTarget;

	if ( fabs( fOldV - fTargetV ) <= 0.05f )
		fTargetV = fTargetV * 0.2f + fOldV * 0.8f;

	*a_pfVTarget = fTargetV;

	TFLOAT fStep = GetBossData()->m_fAimSpeedV * a_fDeltaTime;
	TFLOAT fV    = *a_pfV;

	if ( fTargetV <= fV )
	{
		if ( fV - fTargetV > fStep )
		{
			bOnTarget = TFALSE;
			*a_pfV    = fV - fStep;
		}
		else
		{
			*a_pfV = fTargetV;
		}
	}
	else if ( fTargetV - fV > fStep )
	{
		bOnTarget = TFALSE;
		*a_pfV    = fV + fStep;
	}
	else
	{
		*a_pfV = fTargetV;
	}

	a_pVAnim->SetTime( *a_pfV * *a_pfVLength );

	// Horizontal
	vDir.NormaliseXZ();

	TFLOAT fTargetH;

	if ( m_uiBossFlags & BOSSFLAG_BlobBehind )
	{
		fTargetH  = ( m_uiBossFlags & BOSSFLAG_TurnRight ) ? 0.0f : 1.0f;
		bOnTarget = TFALSE;
	}
	else
	{
		TFLOAT fCos = m_oMatrix.m_f12 * vDir.y + m_oMatrix.m_f11 * vDir.x + m_oMatrix.m_f13 * vDir.z;
		TMath::Clip( fCos, -1.0f, 1.0f );
		fTargetH = TMath::ACos( fCos ) / TMath::PI;
	}

	TFLOAT fTimeH = fTargetH;
	TASSERT( fTimeH >= 0.f );

	if ( fabs( *a_pfHTarget - fTargetH ) <= 0.1f )
		fTargetH = fTargetH * 0.2f + *a_pfHTarget * 0.8f;

	*a_pfHTarget = fTargetH;

	fStep     = GetBossData()->m_fAimSpeedH * a_fDeltaTime;
	TFLOAT fH = *a_pfH;

	if ( fTargetH <= fH )
	{
		if ( fH - fTargetH > fStep )
		{
			bOnTarget = TFALSE;
			*a_pfH    = fH - fStep;
		}
		else
		{
			*a_pfH = fTargetH;
		}
	}
	else if ( fTargetH - fH > fStep )
	{
		bOnTarget = TFALSE;
		*a_pfH    = fH + fStep;
	}
	else
	{
		*a_pfH = fTargetH;
	}

	// The hands can't cross the middle (the right hand's animation is mirrored)
	TFLOAT fTimeHAnim;

	if ( a_bLeft )
	{
		TFLOAT fLimit = GetBossData()->m_fAimLimitH;

		if ( *a_pfH > fLimit )
		{
			*a_pfH    = fLimit;
			bOnTarget = TFALSE;
		}

		fTimeHAnim = *a_pfH;
	}
	else
	{
		TFLOAT fLimit = 1.0f - GetBossData()->m_fAimLimitH;

		if ( fLimit > *a_pfH )
		{
			*a_pfH    = fLimit;
			bOnTarget = TFALSE;
		}

		fTimeHAnim = 1.0f - *a_pfH;
	}

	a_pHAnim->SetTime( fTimeHAnim * *a_pfHLength );

	// Fires when on target (or right after this hand's last shot)
	if ( bOnTarget || ( ( m_uiBossFlags & BOSSFLAG_ShotFired ) && *a_pBone == m_iLastShotBone ) )
	{
		TFLOAT fTimer = ( m_iShotCount <= 0 ) ? m_fChargeTime : m_fShotTimer;

		if ( fTimer < 0.0f )
		{
			if ( m_iShotCount < m_iNumShots )
			{
				AProjectile* pProjectile = AProjectileManager::GetSingleton()->CreateProjectile( AProjectileTypes::PROJECTILETYPES( 1 ) );

				if ( pProjectile != TNULL )
				{
					vShotDir.x = -vShotDir.x;
					vShotDir.y = -vShotDir.y;
					vShotDir.z = -vShotDir.z;
					pProjectile->Activate( this, mBone.GetTranslation(), vShotDir, 0x2F );
					pProjectile->m_uiAttackType = 0xC;

					pI->AddAnim( *a_pShootAnimID, 0.15f, 1.0f, 1.0f )->ChangeToManaged( 0.0f );

					m_iShotCount++;
					m_uiBossFlags |= BOSSFLAG_ShotFired;
					m_fShotTimer    = m_fShotDelay;
					m_iLastShotBone = *a_pBone;
				}
			}
			else
			{
				// The series is over: the charge
				m_uiBossFlags &= ~BOSSFLAG_ShotFired;
				m_fChargeTime   = m_fRoundChargeTime;
				m_fShotTimer    = -1.0f;
				m_iShotCount    = 0;
				m_iLastShotBone = -1;
				SetState( STATE_ChargeStart );
			}
		}
	}
}

// $deBlob: FUNCTION 007726c0
void ABoss::RaiseBossTriggers()
{
	for ( TINT i = 0; i < m_uiNumBossTriggers; i++ )
	{
		ABossTrigger* pTrigger = GetBossTrigger( m_aiBossTriggers[ i ] );

		if ( pTrigger->GetBossStage() == m_uiRound )
			pTrigger->Raise();

		if ( pTrigger->GetBossStage() <= m_uiRound )
			pTrigger->SetHazardsActive( TTRUE );
	}
}

// $deBlob: FUNCTION 00772800
void ABoss::SetupRound()
{
	TASSERT( m_uiRound < BOSSFIGHT_NUM_ROUNDS );

	if ( m_uiRound < BOSSFIGHT_NUM_ROUNDS )
	{
		const BossData::Round& rRound = GetBossData()->m_aRounds[ m_uiRound ];
		m_iNumShots                   = rRound.m_iNumShots;
		m_fTurnSpeed                  = rRound.m_fTurnSpeed;
		m_fShotDelay                  = rRound.m_fShotDelay;
		m_fRoundChargeTime            = rRound.m_fChargeTime;
	}
}

// $deBlob: FUNCTION 00772960
void ABoss::UpdateHUD()
{
	AGameState* pCurrentState = AGameStateController::GetSingleton()->GetCurrentGameState();

	if ( !pCurrentState->GetClass()->IsA( &TGetClass( AInGameState ) ) )
	{
		m_pScene->SetVisible( TFALSE );
		return;
	}

	// Hidden while Blob is Z-jumping into the boss
	ABlobEntity*         pBlob   = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() );
	ABlobZJumpComponent* pZJump  = pBlob->GetZJumpComponent();
	TINT                 eZState = pZJump->GetState();

	if ( eZState == 8 || eZState == 9 )
	{
		ABlobZJumpComponent::PriorityTarget* pLock = pZJump->GetCurrentPriorityLock();

		if ( pLock != TNULL && pLock->m_eType == 0 && this == pLock->m_pTarget )
		{
			m_pScene->SetVisible( TFALSE );
			return;
		}
	}

	TMatrix44 mHat;
	GetModelComp( this )->GetModelInstance()->GetBoneTransform( m_iHatBone, mHat );

	TSphere oSphere( mHat.GetTranslation().AsVector3(), 0.5f );

	ACamera*   pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
	TXUIScene* pScene  = m_pScene;
	TVector4   vCamera = pCamera->GetMatrix().GetTranslation();
	TFLOAT     fDistSq = ( mHat.m_f41 - vCamera.x ) * ( mHat.m_f41 - vCamera.x ) + ( mHat.m_f43 - vCamera.z ) * ( mHat.m_f43 - vCamera.z );

	if ( fDistSq >= 40000.0f )
	{
		pScene->SetVisible( TFALSE );
		return;
	}

	if ( !TRenderContext::CullSphereToFrustumSimple( oSphere, pCamera->GetViewPlanes(), 6 ) )
	{
		pScene->SetVisible( TFALSE );
		return;
	}

	// Hidden behind the scenery
	TPhysicsRay oRay;
	oRay.m_vFrom            = vCamera;
	oRay.m_vTo.x            = m_oMatrix.m_f31 * 3.0f + mHat.m_f41;
	oRay.m_vTo.y            = m_oMatrix.m_f32 * 3.0f + mHat.m_f42;
	oRay.m_vTo.z            = m_oMatrix.m_f33 * 3.0f + mHat.m_f43;
	oRay.m_vTo.w            = mHat.m_f44;
	oRay.m_uiCollisionLayer = 18;

	TPhysicsShapeContact oContact;

	if ( TPhysicsManager::GetSingleton()->CollideRay( oRay, oContact, TNULL ) )
	{
		pScene->SetVisible( TFALSE );
		return;
	}

	// The LOD of the HUD by the distance
	TXUITimelineControl* pTimeline = pScene->GetVisual()->GetTimelineControl();

	static TFLOAT s_fLowLODTime  = pTimeline->GetNamedFrameTime( L"LowLOD" );
	static TFLOAT s_fHighLODTime = pTimeline->GetNamedFrameTime( L"HighLOD" );

	if ( fDistSq > 22500.0f )
	{
		if ( fDistSq < 40000.0f )
		{
			TFLOAT fLerp = ( fDistSq - 22500.0f ) / 17500.0f;
			TMath::Clip( fLerp, 0.0f, 1.0f );
			fLerp = 1.0f - fLerp;
			pTimeline->SetTimeLineTime( ( s_fHighLODTime - s_fLowLODTime ) * fLerp + s_fLowLODTime, TFALSE );
		}
		else
		{
			pTimeline->SetTimeLineTime( s_fLowLODTime, TFALSE );
		}
	}
	else
	{
		pTimeline->SetTimeLineTime( s_fHighLODTime, TFALSE );
	}

	TASSERT( 0 < GetLife() && GetLife() < 1000 );
	TStringManager::String16Format( m_wszLife, 4, L"%d", m_iLife );
	pScene->SetVisible( TTRUE );

	// The droplets in Blob's colour
	TUINT32 uiColour = pBlob->GetPaintColour();

	if ( uiColour & 0x10 )
		uiColour = 0x10;

	// NOTE: retail splits the colour into its bytes and joins them back (no change)
	TUINT32 uiFill = ABlobColourComponent::ms_auiColours[ uiColour ];
	m_pDropletLow->SetFillColour( uiFill );
	m_pDropletHigh->SetFillColour( uiFill );

	// The light colours get the dark text
	TBOOL bLight = ( uiColour == 1 || uiColour == 6 || uiColour == 5 );
	m_pTextPresenter1->SetVisible( bLight );
	m_pTextPresenter2->SetVisible( !bLight );

	TVector4 vPosition = mHat.GetTranslation();
	ARenderer::GetSingleton()->ProjectToCanvas( vPosition, TNULL );

	TFLOAT fX = vPosition.x - pScene->GetWidth() * 0.5f;
	TFLOAT fY = vPosition.y - pScene->GetHeight() * 0.5f;
	pScene->SetPosition( fX, fY );
}
