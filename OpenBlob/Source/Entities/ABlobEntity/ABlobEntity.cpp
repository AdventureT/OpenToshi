#include "pch.h"
#include "Buildings/ABuildingInstance.h"
#include "Buildings/ALandmarkInstance.h"
#include "Buildings/ABuildingManager.h"
#include "Buildings/ATreeInstance.h"
#include "Buildings/AToggleVisInstance.h"
#include "Buildings/ATextureSwapInstance.h"
#include "ABlobEntity.h"
#include "ABlobRibbonTrailComponent.h"
#include "ABlobNavRingComponent.h"
#include "Entities/AEntityIOPacket.h"
#include "Collision/ACollisionInstance2.h"
#include "ABlobAnimationComponent.h"
#include "ABlobZJumpComponent.h"
#include "ABlobSoundComponent.h"
#include "Animation/AModelInstance2.h"
#include "Entities/AFXBreakpointComponent.h"
#include "Collision/ACollisionObject.h"
#include "ACurrentLevelInformation.h"
#include "ALevelInformation.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/APropHazard/APropHazard.h"
#include "Entities/APropWindCannon/APropWindCannon.h"
#include "Entities/APropTriggerHelp.h"
#include "Challenges/AChallenge.h"
#include "Entities/Helpers/ASpawnPointPool.h"
#include "Entities/APropTriggerTimeBonus.h"
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include "XUI/InGame/HUD/ABlobShakeHud.h"
#include "Entities/APropTriggerHelp.h"
#include "Platform/DX11/AJellyShader_DX11.h"
#include "XUI/InGame/HUD/ABlobHelpHud.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "Sound/AMusicManager.h"
#include "XUI/InGame/HUD/ABlobLifeLostHud.h"
#include "XUI/InGame/HUD/ABlobVisionHud.h"
#include "Entities/Projectiles/AProjectileManager.h"
#include <XUI/TXUI.h>
#include <XUI/TXUIShapeCache.h>
#include "Cameras/ACameraManager.h"
#include "Cameras/ABlobFollowCamera.h"
#include "Entities/ABoss/ABoss.h"
#include <Input/TInputInterface.h>
#include "GameInterface/AGameStateController.h"
#include "Render/ARenderer.h"
#include <Render/TModelManager.h>
#include <Platform/DX11/TRender_DX11.h>
#include "Render/AXYZViewportManager.h"
#include "Cameras/ACamera.h"
#include "Challenges/AChallengeManager.h"
#include "Input/ABlobInputMap.h"
#include "Entities/Enemies/AEnemyGrappler.h"
#include "Entities/Enemies/AEnemyManager.h"
#include "Entities/APickupLife.h"
#include "Entities/APickupManager.h"
#include "Input/AVibrationManager.h"
#include "Entities/AEntityHash.h"
#include "Entities/APropTriggerLevelComplete.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Entities/APropSwitchPressure.h"
#include "Entities/ACar/ACar.h"
#include "Entities/ACiv.h"
#include "Entities/ACritter.h"
#include "Entities/APropPlatform.h"
#include "Entities/AInkJet/AInkJet.h"
#include "Entities/AAttackMessage.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "Entities/AVisibleEntity.h"
#include "Render/Decal/ADecalManager.h"
#include "ABlobSplatTrail.h"
#include <Shaders/SysShader/TSysShader.h>
#include <Platform/DX11/TTexture_DX11.h>
#include "ARandom.h"
#include "Render/PaintParticleSystem/APaintParticleSystem.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Physics/TSphereData.h>
#include <Math/TQuaternion.h>
#include <File/TTRB.h>

#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "ABlobColourMaps.h"
#include "Fx/AParticleEffectHelper.h"
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffect.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include "XUI/InGame/HUD/ABlobAmmoHud.h"
#include "XUI/InGame/HUD/ABlobSizeHud.h"
#include "XUI/InGame/HUD/ABlobLifeHud.h"

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ABlobEntity::CommonProperties* ABlobEntity::ms_pBlobCommonProperties;
Toshi::TParticleEffect*        ABlobEntity::ms_pPaintSplashEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pElectrocutionEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pHotPlateEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pInkedStinkEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pInkSplashEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pWaterSplashEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pWaterSplashSlamEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pWaterRippleEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pLandmarkEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pUnk9B59C0;
Toshi::TParticleEffect*        ABlobEntity::ms_pUnk9B59C4;
Toshi::TParticleEffect*        ABlobEntity::ms_pDeathPopSplatEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pDeathPopSprayEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pWindLauncherTrailEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pRainbowEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pZJumpFailEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pSpikeHaemorrhageClearEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pMPShieldLoopEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pMPShieldHitEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pElectricTrapSparksEffect;
Toshi::TParticleEffect*        ABlobEntity::ms_pLandmarkPaintSplatEffect;
Toshi::TParticleEffect**       ABlobEntity::ms_apInkCloudEffects;
Toshi::TParticleEffect**       ABlobEntity::ms_apInkCloudSmallEffects;
Toshi::TParticleEffect*        ABlobEntity::ms_pUnpaintableEffect;
ABlobEntity::Properties*       ABlobEntity::ms_pBlobProperties;
void*                          ABlobEntity::ms_pBlobRibbonProperties;
void*                          ABlobEntity::ms_pZTargetPriorities;
TBOOL                          ABlobEntity::ms_bControlsLocked;
TFLOAT                         ABlobEntity::ms_fShockwaveFadeStart = 0.7f;

// The wind cannon: its pull on a blob next to it, the paint ring of the shot
static TFLOAT s_fCannonPullForce      = 150.0f; // (UpdateInput)
static TINT   s_iCannonPaintParticles = 30;     // (LaunchFromCannon)
static TINT   s_iCannonPaintDecals    = 20;

// $deBlob: FUNCTION 0073b530
// $deBlob: FUNCTION 0073b540
// $deBlob: FUNCTION 0073b5b0
// $deBlob: FUNCTION 005406c0
// $deBlob: FUNCTION 00859c40
TDEFINE_CLASS( ABlobEntity );

#ifndef DEBLOB_COMMUNITY_PATCH
TSTATICASSERT( sizeof( ABlobEntity ) == 0xBA8 );
#endif

// $deBlob: FUNCTION 0073c380
void ABlobEntity::ABlobPhysicsPlugin::OnStartStep( TFLOAT a_fTimeStep, TPhysicsManager* a_pManager )
{
	m_pPreviousGroundBody = TREINTERPRETCAST( TPhysicsObjectContainer*, m_pOwner->m_oMotion.m_auiUnk280[ 0 ] );

	TerrainTypes::Cell* pCell = TSTATICCAST( TerrainTypes::Cell, m_pOwner->GetCellPair().m_pCell );
	if ( pCell && ( TUINT8( pCell->m_uiFlags >> 26 ) & 1 ) )
	{
		m_pOwner->m_oMotion.ResetContacts();
		m_pOwner->m_uiFlags &= ~4;
		return;
	}

	m_pOwner->m_oMotion.Reset();
	m_pOwner->m_uiFlags &= ~0xC;
	m_pOwner->m_pZJump->SetContactBody( TNULL );
}

// $deBlob: FUNCTION 0073c3e0
void ABlobEntity::ABlobPhysicsPlugin::OnFinishStep( TPhysicsManager* a_pManager )
{
	ABlobEntity*             pBlob   = m_pOwner;
	TPhysicsObjectContainer* pGround = TREINTERPRETCAST( TPhysicsObjectContainer*, pBlob->m_oMotion.m_auiUnk280[ 0 ] );
	if ( !pGround )
		return;

	ACollisionInstance2::OwnerInfo* pOwnerInfo              = TSTATICCAST( ACollisionInstance2::OwnerInfo, pGround->m_pUserData );
	ABaseEntity*                    pEntity                 = pOwnerInfo->m_eType == 2 ? TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner ) : TNULL;
	APropPlatform*                  pPlatform               = pEntity && pEntity->GetClass() == &TGetClass( APropPlatform ) ? TSTATICCAST( APropPlatform, pEntity ) : TNULL;
	TBOOL                           bConveyor               = pPlatform && pPlatform->IsConveyor();
	TVector4&                       rvGroundVelocity        = pBlob->m_oMotion.m_vUnk220;
	TVector4&                       rvGroundAngularVelocity = pBlob->m_oMotion.m_vUnk230;

	if ( bConveyor )
	{
		TVector4 vUnused;
		rvGroundVelocity = pEntity->GetReactionForce( &vUnused, TNULL );
		rvGroundAngularVelocity.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	}
	else
	{
		a_pManager->GetAngularVelocity( pGround, rvGroundAngularVelocity );
		TVector4 vOffset = pBlob->m_oMotion.m_vUnk240 - pGround->m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION );
		TVector4 vTangentialVelocity;
		vTangentialVelocity.CrossProduct( rvGroundAngularVelocity, vOffset );
		a_pManager->GetLinearVelocity( pGround, rvGroundVelocity );
		rvGroundVelocity.x += vTangentialVelocity.x;
		rvGroundVelocity.y += vTangentialVelocity.y;
		rvGroundVelocity.z += vTangentialVelocity.z;
	}

	TBOOL bMovingBody = pGround->m_eMotionType != 0 && pGround->m_eMotionType != 3;
	if ( pEntity && pEntity->GetClass() == &TGetClass( AInkJet ) )
		bMovingBody = TFALSE;
	TASSERT( pGround != TNULL, "POC != TNULL" );
	if ( pGround->m_uiSurface & 0x3804000 )
		bMovingBody = TFALSE;

	if ( !bConveyor && bMovingBody && pBlob->m_eState != ABlobEntity::STATE_FLOAT &&
	     ( rvGroundVelocity.MagnitudeSq() > 0.0f || rvGroundAngularVelocity.MagnitudeSq() > 0.0f ) )
	{
		TBOOL bNewPlatform = TFALSE;
		TBOOL bWasGrounded = ( pBlob->m_uiFlags & 0x20000 ) != 0;
		if ( pGround != m_pPreviousGroundBody )
		{
			pBlob->m_uiFlags &= ~0x100000;
			if ( pPlatform || ( pEntity && pEntity->GetClass() == &TGetClass( ACar ) ) )
				bNewPlatform = TTRUE;
		}

		TVector2 vMove = TVector2::VEC_ZERO;
		if ( pBlob->m_iController >= 0 )
			vMove = ABlobInputMap::GetSingleton()->GetMoveStick( pBlob->m_iController );
		TVector4 vRelativeVelocity;
		a_pManager->GetLinearVelocity( pBlob->m_pBody, vRelativeVelocity );
		vRelativeVelocity.Substract( rvGroundVelocity );
		TBOOL bAtRest = vRelativeVelocity.MagnitudeSqXZ() < 0.001f && vMove.x * vMove.x + vMove.y * vMove.y < 0.001f;

		if ( bWasGrounded || bNewPlatform || ( bAtRest && !( pBlob->m_uiFlags & 0x100000 ) ) || pBlob->m_oMotion.m_auiUnk280[ 2 ] != 0 )
		{
			TMatrix44 oInverse;
			oInverse.Invert( *pGround->m_pMatrix );
			pBlob->m_oMotion.m_oUnk1B0.Multiply( oInverse, pBlob->m_oPhysicsMatrix );
			pBlob->m_uiFlags |= 0x100000;
		}

		if ( ( pBlob->m_uiFlags & 0x100000 ) || ( pBlob->m_uiButtons & ABlobInputMap::BUTTON_BRAKE ) )
		{
			a_pManager->SetLinearVelocity( pBlob->m_pBody, rvGroundVelocity );
			if ( pBlob->m_uiFlags & 0x100000 )
			{
				pBlob->m_oPhysicsMatrix.Multiply( *pGround->m_pMatrix, pBlob->m_oMotion.m_oUnk1B0 );
				a_pManager->UpdateBodyTransform( pBlob->m_pBody, TFALSE );
				pBlob->m_vPhysicsPosition = pBlob->m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
			}
		}
	}
}

ABlobMotionData::ABlobMotionData()
{
	Reset();
}

// $deBlob: FUNCTION 00751f70
void ABlobMotionData::Reset()
{
	m_oUnk1B0.Identity();
	m_vLiquidHit     = TVector4::VEC_ZERO;
	m_vLiquidHit.w   = 1.0f;
	m_vLiquidNormal  = TVector4::VEC_NEGY;
	m_vContactNormal = TVector4::VEC_NEGY;
	m_vUnk220        = m_vLiquidHit;
	m_vUnk230        = m_vLiquidHit;
	m_vWallNormal    = TVector4::VEC_NEGY;
	// NOTE: retail also clears bits 4 and 5 of the word at 0x2AC first, then stores 0 over it
	m_vUnk270 = TVector4::VEC_NEGY;
	m_vUnk240 = m_vLiquidHit;
	m_vUnk250 = m_vLiquidHit;

	for ( TINT i = 0; i < 11; i++ )
		m_auiUnk280[ i ] = 0;

	m_uiContactFlags          = 0;
	m_uiPrevContactFlags      = 0;
	m_uiContactCountAndGround = 0xFF00;
	m_uiWallContact           = 0xFF;
}

// The constructor of the effect slots at 0xAD8 (called by the constructor)
// $deBlob: FUNCTION 007511e0
static TUINT32* InitBlobEffects( TUINT32* a_pEffects )
{
	for ( TINT i = 0; i < 17; i++ )
		a_pEffects[ i ] = 0;

	return a_pEffects;
}

// $deBlob: FUNCTION 0073cc80
ABlobEntity::ABlobEntity()
    : m_oMotion()
    , m_oCollisionEventHandler( this )
    , m_oPhysicsMover( this )
    , m_oPhysicsPlugin( this )
    , m_oJelly( TVector4::VEC_ZERO, ms_pBlobProperties[ 0 ].m_fJellyStiffness, 9.0f ) // NOTE: retail reads m_iSize, already 0 here
    , m_oJellySquash( 0.0f, 20.0f, 9.0f )
{

	static const TVector4 s_vOrigin( 0.0f, 0.0f, 0.0f, 1.0f );
	static const TVector4 s_vAxisX( 1.0f, 0.0f, 0.0f, 1.0f );
	static const TVector4 s_vAxisZ( 0.0f, 0.0f, 1.0f, 1.0f );
	static const TVector4 s_vAxisNegY( 0.0f, -1.0f, 0.0f, 1.0f );

	m_oPhysicsMatrix = TMatrix44::IDENTITY;
	m_oJellyMatrix   = TMatrix44::IDENTITY;

	m_eState                 = 0;
	m_iPaintPoints           = 0;
	m_iSize                  = 0;
	m_iOldSize               = 0;
	m_uiLandmarkTime         = 0;
	m_pColourComponent       = TNULL;
	m_pSplatTrail            = TNULL;
	m_pSoundComponent        = TNULL;
	m_pZJump                 = TNULL;
	m_pAnimationComponent    = TNULL;
	m_pFXBreakpointComponent = TNULL;
	m_pNavRing               = TNULL;
	m_pBody                  = TNULL;

	m_iUserDataType             = 0;
	m_pUserDataOwner            = TNULL;
	m_pCollisionObject          = TNULL;
	m_pLiquidRayCastHelper      = TNULL;
	m_pUnk424                   = TNULL;
	m_pJellyQueryHelper         = TNULL;
	m_pSoftCollisionQueryHelper = TNULL;
	m_pPaintParticlePhantomA    = TNULL;
	m_pPaintParticlePhantomB    = TNULL;

	m_vBallOffset      = s_vOrigin;
	m_vBallPosition    = s_vOrigin;
	m_vJellyPosition   = s_vOrigin;
	m_vFacing          = s_vAxisZ;
	m_vLookRotation    = s_vOrigin;
	m_vRespawnPosition = s_vOrigin;
	m_vRespawnRotation = s_vOrigin;

#ifdef DEBLOB_COMMUNITY_PATCH
	m_mRenderPrev    = TMatrix44::IDENTITY;
	m_mRenderLast    = TMatrix44::IDENTITY;
	m_mRenderCurrent = TMatrix44::IDENTITY;
	m_uiRenderStep   = 0;
	m_iRenderSteps   = 0;
#endif

	m_uiFlags       = 0;
	m_uiPrevFlags   = 0;
	m_uiButtons     = 0;
	m_uiPrevButtons = 0;
	for ( TINT i = 0; i < 6; i++ )
		m_auiUnk564[ i ] = 0;

	m_fPrevSquashCompensation = 1.0f;
	for ( TINT i = 0; i < 5; i++ )
		m_auiUnk580[ i ] = 0;
	m_fAwakeTimer = 0.0f;

	m_fAwakeDuration    = 5.0f;
	m_fUnk59C           = 0.0f;
	m_fLookTurnSpeed    = ms_pBlobCommonProperties->m_fLookTurnRate;
	m_fUntargetableTime = -1.0f;
	for ( TINT i = 0; i < 6; i++ )
		m_auiUnk5A8[ i ] = 0;

	m_auiUnk5C4[ 0 ]       = 0;
	m_auiUnk5C4[ 1 ]       = 0;
	m_auiUnk5C4[ 2 ]       = 0;
	m_vCameraDirection     = s_vAxisX;
	m_vJumpDirection       = s_vOrigin;
	m_vJumpStart           = s_vOrigin;
	m_vClimbStart          = s_vOrigin;
	m_iController          = -1;
	m_vUnk614              = s_vAxisNegY;
	m_fSquash              = 1.0f;
	m_fSquashCompensation  = 1.0f;
	m_vStretchDirection    = s_vAxisNegY;
	m_fStretch             = 1.0f;
	m_fStretchCompensation = 1.0f;
	m_fStretchTimeLeft     = 0.0f;

	// m_aThreads: Thread's constructor

	m_oFreeThreads.m_pHead    = TNULL;
	m_oFreeThreads.m_pTail    = TNULL;
	m_oFreeThreads.m_iCount   = 0;
	m_oActiveThreads.m_pHead  = TNULL;
	m_oActiveThreads.m_pTail  = TNULL;
	m_oActiveThreads.m_iCount = 0;

	m_fThreadDistanceSq     = 0.0f;
	m_vThreadPosition       = s_vOrigin;
	m_iLeechbotsAttached    = 0;
	m_iMaxLeechbotsAttached = 0;
	m_uiTargettingAndShakes = 0;
	m_uiShakesNeeded        = 0;

	InitBlobEffects( m_auiSounds );

	m_pSizeHud            = TNULL;
	m_pAmmoHud            = TNULL;
	m_iZPlayerAttackStage = 0;
	m_fZPlayerAttackTime  = 0.0f;

	m_iCamShakeHotPlateID     = -1;
	m_iCamShakeID             = -1;
	m_fUnkB34                 = 0.0f;
	m_pBoss                   = TNULL;
	m_iCurrentLandmarkTrigger = -1;
	m_pHelpTrigger            = TNULL;
	m_pHudMessageTrigger      = TNULL;
	m_pHotPlate               = TNULL;
	m_pWindCannon             = TNULL;

	m_bInBlobVision                = 0;
	m_bLifeLostShown               = 0;
	m_bHasRespawnRotation          = 0;
	m_bRespawnPlaySeq              = 0;
	*(TUINT8*)&m_uiRespawnTransSeq = 0;
	m_uiRespawnSeqStartRatio       = 0;
	m_fRespawnSeqEndRatio          = 1.0f;
	m_bUnkB64                      = TFALSE;
	m_uiUnkB68                     = 0;
	m_iStartPaintPoints            = -1;
	m_iStartColour                 = -1;
	m_pShockwaveModel              = TNULL;
	m_fShockwaveTime               = 0.0f;
	m_fShockwaveDuration           = 0.0f;
	m_fShockwaveAlpha              = 1.0f;
	m_vShockwavePosition           = s_vOrigin;
	m_vShockwaveNormal             = s_vOrigin;

	m_Flags |= FLAG_Visible | 0x10;
	m_pShockwaveNoise = TNULL;
	SetTicking( TTRUE );

	// NOTE: retail stores the byte at 0x63 (the high byte of m_iCollisionGroupAndType)
	m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & 0x00FF ) | ( 36 << 8 ) );
}

// $deBlob: FUNCTION 0073d340
// $deBlob: FUNCTION 00751360
ABlobEntity::~ABlobEntity()
{
	m_oList438.Clear();
}

// $deBlob: FUNCTION 0073c970
// The finished callback of the blob's effect instances (a_pUserData: this + 0xAD8): clears the slot of a_pInstance
static void OnBlobEffectFinished( TParticleEffectInstance* a_pInstance, void* a_pUserData )
{
	TUINT32* pEffects   = TSTATICCAST( TUINT32, a_pUserData );
	TUINT32  uiInstance = TREINTERPRETCAST( TUINT32, a_pInstance );

	if ( pEffects[ 14 ] == uiInstance )
		pEffects[ 14 ] = 0;
	else if ( pEffects[ 7 ] == uiInstance )
		pEffects[ 7 ] = 0;
	else if ( pEffects[ 8 ] == uiInstance )
		pEffects[ 8 ] = 0;
	else if ( pEffects[ 9 ] == uiInstance )
		pEffects[ 9 ] = 0;
	else if ( pEffects[ 10 ] == uiInstance )
		pEffects[ 10 ] = 0;
	else if ( pEffects[ 11 ] == uiInstance )
		pEffects[ 11 ] = 0;
	else
		pEffects[ 15 ] = 0;
}

// $deBlob: FUNCTION 0073e930
void ABlobEntity::PostPhysicsUpdate( TFLOAT a_fDeltaTime, TINT a_iController )
{
	TASSERT( a_iController >= 0 );

	m_iController = TINT8( a_iController );
	m_uiFlags &= ~0x40;

	ABlobInputMap* pInput  = ABlobInputMap::GetSingleton();
	TUINT32        uiFlags = m_uiFlags;

	if ( ( uiFlags & 0x80000 ) == 0 )
	{
		if ( a_iController < 0 )
		{
			m_uiButtons = 0;
		}
		else
		{
			// Any input keeps the blob awake (m_fAwakeTimer)
			TUINT32 uiButtons = pInput->IsDisabled() ? 0 : pInput->GetButtons( a_iController );

			if ( uiButtons != 0 || 1e-05f < pInput->GetMoveStick( a_iController ).MagnitudeSq() )
				m_fAwakeTimer = m_fAwakeDuration;

			// The buttons collect until the next physics step (which sets 0x4000)
			if ( uiFlags & 0x4000 )
			{
				m_uiButtons = 0;
				m_uiFlags   = uiFlags & ~0x4000;
			}

			if ( uiButtons & ABlobInputMap::BUTTON_JUMP )
				m_uiButtons &= ~ABlobInputMap::BUTTON_JUMP;

			m_uiButtons |= uiButtons;

			// The ZLock button (binding 12): pressing it starts the lock; released (or no input) cancels it
			TBOOL bCancelLock = pInput->IsDisabled();

			if ( !bCancelLock )
			{
				const ABlobInputMap::PlayerInput& rPlayerInput = pInput->GetPlayerInput( a_iController );
				TUINT32                           uiZLock      = rPlayerInput.m_uiButtons;

				if ( ( ( ( rPlayerInput.m_uiPrevButtons ^ uiZLock ) & uiZLock ) >> 12 & 1 ) == 0 )
				{
					if ( ( uiZLock >> 12 & 1 ) == 0 )
						bCancelLock = TTRUE;
				}
				else
				{
					switch ( m_eState )
					{
						case STATE_HOTPLATE:
						case STATE_LMARK:
						case STATE_BOSS:
						case STATE_CANNON:
						case STATE_GRASSBOMB:
						case STATE_LEVEL_EXIT:
						case STATE_DEAD:
						case STATE_FAIL:
						case STATE_VICTORY:
						case STATE_LOSTLASTLIFE:
							break;
						default:
							m_pZJump->StartLock();
					}
				}
			}

			if ( bCancelLock )
			{
				switch ( GetZJumpState() )
				{
					case ABlobZJumpComponent::STATE_POINT:
					case ABlobZJumpComponent::STATE_AIMNODE:
					case ABlobZJumpComponent::STATE_LOCK:
					case ABlobZJumpComponent::STATE_LOCKLMARK:
						m_pZJump->Cancel();
				}
			}

			// The action button may start something (BFLAG 0x40, single player only)
			if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE )
			{
				switch ( m_eState )
				{
					case STATE_LMARK:
					case STATE_BOSS:
					case STATE_CANNON:
					case STATE_GRASSBOMB:
					case STATE_LEVEL_EXIT:
					case STATE_DEAD:
					case STATE_FAIL:
					case STATE_VICTORY:
					case STATE_LOSTLASTLIFE:
						break;
					default:
					{
						APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetBlobDataOf( this );
						AChallengeManager*        pManager  = AChallengeManager::GetSingleton();

						if ( ( pManager->GetNearbyChallenge() == TNULL || pManager->GetCurrentChallenge() != TNULL ) &&
						     !AGameStatus::GetSingleton()->IsLevelComplete() && !pBlobData->m_bUnk60 && m_pWindCannon == TNULL &&
						     !m_bInBlobVision && pInput->IsButtonDown( a_iController, ABlobInputMap::BINDING_NAVRING ) )
						{
							m_uiFlags |= 0x40;
						}

						break;
					}
				}
			}

			// Next to a wind cannon without moving: pulled into it
			if ( m_pWindCannon != TNULL && pInput->GetMoveStick( a_iController ).MagnitudeSq() < 1e-05f && !pInput->IsButtonDown( a_iController, ABlobInputMap::BINDING_BRAKE ) )
			{
				TINT iZJumpState = GetZJumpState();

				if ( iZJumpState != ABlobZJumpComponent::STATE_NODEFLIGHT && iZJumpState != ABlobZJumpComponent::STATE_ATTACKFLIGHT && iZJumpState != ABlobZJumpComponent::STATE_ATTACKSLAM && iZJumpState != ABlobZJumpComponent::STATE_LMARKFLIGHT && iZJumpState != ABlobZJumpComponent::STATE_SWITCHSLAM )
				{
					const TVector4& rCannonPos = m_pWindCannon->GetTarget()->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );

					TVector4 vToCannon;
					vToCannon.x = rCannonPos.x - m_oPhysicsMatrix.m_f41;
					vToCannon.y = ( rCannonPos.y - ms_pBlobProperties[ m_iSize ].m_fRadius ) - m_oPhysicsMatrix.m_f42;
					vToCannon.z = rCannonPos.z - m_oPhysicsMatrix.m_f43;
					vToCannon.w = rCannonPos.w;

					TFLOAT fDistance = vToCannon.NormaliseMag();

					if ( 0.5f <= fDistance )
					{
						TVector4 vForce;
						vForce.Multiply( vToCannon, ms_pBlobProperties[ m_iSize ].m_fMass * s_fCannonPullForce * fDistance );
						TPhysicsManager::GetSingleton()->ApplyForce( m_pBody, vForce );

						TVector4 vVelocity;
						TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );
						vVelocity.Multiply( vToCannon, vVelocity.Magnitude() );
						TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );

						m_pWindCannon->OnBlobLeaving();
					}
					else
					{
						TVector4 vPosition = rCannonPos;
						vPosition.y        = m_oPhysicsMatrix.m_f42;
						SetPhysicsPosition( vPosition );

						TVector4 vVelocity;
						TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );
						vVelocity.z = 0.0f;
						vVelocity.x = 0.0f;
						TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );

						m_pWindCannon->OnBlobEntering();
					}
				}
			}
		}
	}
	else
	{
		m_iController   = -1;
		m_uiButtons     = 0;
		m_uiPrevButtons = 0;

		switch ( GetZJumpState() )
		{
			case ABlobZJumpComponent::STATE_POINT:
			case ABlobZJumpComponent::STATE_AIMNODE:
			case ABlobZJumpComponent::STATE_LOCK:
			case ABlobZJumpComponent::STATE_LOCKLMARK:
				m_pZJump->Cancel();
				break;
		}

		if ( m_pNavRing->GetState() < ABlobNavRingComponent::STATE_HIDING )
			m_pNavRing->Hide();
	}

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE )
	{
		TBOOL bZJumping = GetZJumpState() == ABlobZJumpComponent::STATE_NODEFLIGHT;

		switch ( m_pNavRing->GetState() )
		{
			case ABlobNavRingComponent::STATE_SHOWING:
			case ABlobNavRingComponent::STATE_SHOWN:
				if ( ( m_uiFlags & 0x40 ) == 0 || bZJumping )
					m_pNavRing->Hide();

				break;
			case ABlobNavRingComponent::STATE_HIDING:
			case ABlobNavRingComponent::STATE_HIDDEN:
				if ( ( m_uiFlags & 0x40 ) != 0 && !bZJumping )
				{
					m_pRibbonTrail->SetWantedSegments( 0 );
					m_pNavRing->Show();
				}

				break;
		}
	}

	TBOOL bGreyAmongPainted = m_pColourComponent->GetCurrentColour() == 0 && AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 && !APlayerManager::GetSingleton()->IsNoBlobPainted();

	TParticleEffectInstance*& rpRainbow = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 15 ] ); // 0xB14

	if ( ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) == 0 || ( m_pAnimationComponent->m_iAnimFlags & 1 ) == 0 )
	{
		if ( rpRainbow != TNULL )
			rpRainbow->Stop();
	}
	else if ( ms_pRainbowEffect != TNULL && rpRainbow == TNULL )
	{
		rpRainbow = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pRainbowEffect );

		if ( rpRainbow != TNULL )
		{
			rpRainbow->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
			rpRainbow->SetFinishedCallback( OnBlobEffectFinished, &m_auiSounds[ 0 ] );
		}
	}

	if ( rpRainbow != TNULL )
		rpRainbow->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_vBallPosition );

	TParticleEffectInstance*& rpInkedStink = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 4 ] ); // 0xAE8

	// bGreyAmongPainted: this blob is grey while another one is painted (multiplayer game type 1)
	if ( ( ( ( m_pColourComponent->GetCurrentColour() >> 4 ) & 1 ) == 0 && !bGreyAmongPainted ) || ( m_pAnimationComponent->m_iAnimFlags & 1 ) == 0 )
	{
		if ( rpInkedStink != TNULL )
		{
			rpInkedStink->Stop();
			rpInkedStink = TNULL;
		}
	}
	else
	{
		if ( ms_pInkedStinkEffect != TNULL && rpInkedStink == TNULL )
			rpInkedStink = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pInkedStinkEffect );

		if ( rpInkedStink != TNULL )
			rpInkedStink->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
	}

	if ( m_uiFlags & 0x2000000 )
	{
		TParticleEffectInstance*& rpElectrocution = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 6 ] ); // 0xAF0

		if ( ( m_pAnimationComponent->m_iAnimFlags & 1 ) == 0 )
		{
			if ( rpElectrocution != TNULL )
			{
				rpElectrocution->StopInstant();
				rpElectrocution = TNULL;
				m_pSoundComponent->StopElectrocution();
			}
		}
		else
		{
			TParticleEffect* pElectrocutionEffect = TREINTERPRETCAST( TParticleEffect*, m_auiSounds[ 0 ] ); // 0xAD8

			if ( pElectrocutionEffect != TNULL && rpElectrocution == TNULL && m_eState != STATE_LMARK && m_eState != STATE_BOSS )
			{
				rpElectrocution = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( pElectrocutionEffect );
				m_pSoundComponent->StartElectrocution();
			}

			if ( rpElectrocution != TNULL )
			{
				rpElectrocution->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );

				// Scaled by the size: the size of the largest (10) is the base effect's
				AParticleEffectHelper::SetPositionRadius( pElectrocutionEffect, ms_pBlobProperties[ m_iSize ].m_fRadius );

				TFLOAT fScale = ms_pBlobProperties[ m_iSize ].m_fRadius / ms_pBlobProperties[ 10 ].m_fRadius; // 0x30C
				TFLOAT fMinSize;
				TFLOAT fMaxSize;
				AParticleEffectHelper::GetSize( ms_pElectrocutionEffect, 0, fMinSize, fMaxSize );
				AParticleEffectHelper::SetSize( pElectrocutionEffect, fMinSize * fScale, fMaxSize * fScale, -1 );
			}
		}
	}

	TParticleEffectInstance* pEffect = TREINTERPRETCAST( TParticleEffectInstance*, m_auiSounds[ 7 ] ); // 0xAF4

	if ( pEffect != TNULL )
	{
		pEffect->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
		pEffect->GetTransform().m_f42 = GetBoundingSphereRadius() + pEffect->GetTransform().m_f42;
	}

	// 0xAF8, 0xAFC
	for ( TINT i = 8; i <= 9; i++ )
	{
		pEffect = TREINTERPRETCAST( TParticleEffectInstance*, m_auiSounds[ i ] );

		if ( pEffect != TNULL )
			pEffect->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
	}

	// The death pop: its effect and a splat
	if ( m_eState == STATE_DEAD )
	{
		TParticleEffectInstance*& rpDeathPop = TREINTERPRETCAST( TParticleEffectInstance*&, m_auiSounds[ 12 ] ); // 0xB08

		if ( rpDeathPop == TNULL && ( m_oMotion.m_uiContactFlags & 0x28 ) != 0 )
		{
			rpDeathPop = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pDeathPopSplatEffect );

			TFLOAT fRandom = ARandom::GetSingleton()->GetRandom().GetFloat();
			ADecalManager::GetSingleton()->SetRotation( fRandom * TMath::TWO_PI );

			TFLOAT fSize = ms_pBlobProperties[ m_iSize ].m_fRadius * 4.0f;
			ADecalManager::GetSingleton()->AddDecal( &m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), fSize, fSize, 0xFF2A2718, m_pMaterialTestSplat, ( fRandom <= 0.5f ) ? ADecalManager::Type_BottomRight : ADecalManager::Type_TopRight );
		}

		if ( rpDeathPop != TNULL )
		{
			TVector4 vPos = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
			vPos.y        = ms_pBlobProperties[ m_iSize ].m_fRadius * 0.95f + m_oPhysicsMatrix.m_f42;
			rpDeathPop->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, vPos );
		}
	}

	// The length of the paint trail: none while the nav ring is shown, else the combo multiplier
	TINT iSegments;

	switch ( m_pNavRing->GetState() )
	{
		case ABlobNavRingComponent::STATE_SHOWING:
		case ABlobNavRingComponent::STATE_SHOWN:
			m_pRibbonTrail->SetWantedSegments( 0 );
			break;
		case ABlobNavRingComponent::STATE_HIDING:
		case ABlobNavRingComponent::STATE_HIDDEN:
		{
			TINT8                    iPlayer = APlayerManager::GetSingleton()->GetPlayerIndex( this );
			AGameStatus::PlayerData& rPlayer = AGameStatus::GetSingleton()->GetLevelData().m_aPlayerData[ iPlayer ];

			// NOTE: retail calls it and ignores the result
			if ( rPlayer.m_iComboState == 1 )
				AGameStatus::LevelData::GetComboMultiplier( rPlayer.m_iComboCount );

			if ( ( m_pColourComponent->GetCurrentColour() >> 4 ) & 1 )
			{
				iSegments = 0;
			}
			else
			{
				switch ( GetZJumpState() )
				{
					case ABlobZJumpComponent::STATE_NODEFLIGHT:
					case ABlobZJumpComponent::STATE_ATTACKSLAM:
					case ABlobZJumpComponent::STATE_ATTACKFLIGHT:
					case ABlobZJumpComponent::STATE_LMARKFLIGHT:
					case ABlobZJumpComponent::STATE_SWITCHSLAM:
						iSegments = ABlobRibbonTrailComponent::MAX_SEGMENTS;
						break;
					default:
					{
						TVector4 vVelocity;
						TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );

						if ( ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) == 0 || vVelocity.MagnitudeSq() <= 1.0f )
						{
							TINT iCombo     = ( AGameStatus::LevelData::GetComboMultiplier( rPlayer.m_iComboCount ) << 7 ) / 8;
							TINT iMinLength = m_pRibbonTrail->GetMinSegments();
							iSegments       = ( iMinLength >= iCombo ) ? iMinLength : iCombo;
						}
						else
						{
							iSegments = ABlobRibbonTrailComponent::MAX_SEGMENTS;
						}

						break;
					}
				}
			}

			m_pRibbonTrail->SetWantedSegments( TINT16( iSegments ) );
			break;
		}
	}

	if ( m_eState != STATE_CANNON || ( m_pWindCannon->GetState() != APropWindCannon::STATE_FIRING && m_pWindCannon->GetState() != APropWindCannon::STATE_LOADED ) )
		UpdateModelTransforms( a_fDeltaTime );
}

// The contact normals of Blob's collisions (cos 70 degrees)
static TFLOAT s_fPhantom424Radius = 30.0f;
static TBOOL  s_bUnk984E88        = TTRUE; // never written
static TFLOAT s_fWallJumpAcross   = 0.5f;
static TFLOAT s_fWallJumpUp       = 0.75f;
// $deBlob: FUNCTION 00540700
static TFLOAT s_fWallContactCos = TFLOAT( cos( 1.2217304706573486 ) ); // DAT_009b5a10
// $deBlob: FUNCTION 00540720
static TFLOAT s_fGroundContactCos = TFLOAT( cos( 1.2217304706573486 ) ); // DAT_009b5a08
// $deBlob: FUNCTION 00540740
static TFLOAT s_fLiquidSurfaceCos = TFLOAT( cos( 0.2617993950843811 ) ); // DAT_009b5a14 (sub_540740)
static TFLOAT s_fLiquidDrag       = 0.2f;
static TFLOAT s_fLiquidBuoyancy   = 1.6f;
static TFLOAT s_fLiquidCarry      = 0.1f;

static const TVector4        s_vAxisY( 0.0f, 1.0f, 0.0f, 1.0f );
static TCONSTEXPR_VAR TFLOAT s_fSlipperySpeedFactor = 0.9f;
static TCONSTEXPR_VAR TFLOAT s_fSlipperyForceFactor = 0.2f;
static TCONSTEXPR_VAR TFLOAT s_fStickyForceFactor   = 0.02f;
static TCONSTEXPR_VAR TFLOAT s_fAirForceFactor      = 3.0f;
static TFLOAT                s_fJumpKeepVelocityY; // never written

// $deBlob: FUNCTION 0073c320
void ABlobEntity::ABlobPhysicsMover::Update( TFLOAT a_fTimeStep, Toshi::TPhysicsManager* a_pManager )
{
	if ( m_pOwner->m_uiFlags & 0x20 )
	{
		// The body's cell was not ready when it was detached: retry the attach
		m_pOwner->SetLevelStartAttached( TFALSE );
	}

	if ( ( m_pOwner->m_uiFlags & 0x80000 ) == 0 )
		m_pOwner->UpdatePhysics( a_fTimeStep );
}

// $deBlob: FUNCTION 0073c360
void ABlobEntity::ABlobPhysicsMover::GetLinearVelocity( Toshi::TPhysicsManager* a_pManager, Toshi::TVector4& a_rvOutVelocity )
{
	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pOwner->m_pBody, a_rvOutVelocity );
}

// $deBlob: FUNCTION 005cd990
// The viewport of the camera of the player whose blob is a_pBlob (APlayerManager)
static TINT GetBlobViewport( APlayerManager* a_pPlayerManager, ABaseEntity* a_pBlob )
{
	ACamera* pCamera = a_pPlayerManager->GetPlayerCamera( TUINT8( a_pPlayerManager->GetPlayerIndex( a_pBlob ) ) );
	return ARenderer::GetSingleton()->GetViewportManager()->GetCameraViewport( pCamera );
}

// Shows the life lost screen of a_pBlob
static void ShowLifeLostScene( ABlobEntity* a_pBlob )
{
	TXUI* pXUI = TXUI::GetSingleton();

	// The wipe's circle is created with more spline steps
	TINT iSplineSteps          = TXUIShape::ms_iSplineSteps;
	TXUIShape::ms_iSplineSteps = 0x14;
	TXUIScene* pScene          = pXUI->CreateScene( pXUI->FindResource( "lifelost.xur" ), 0 );
	TXUIShape::ms_iSplineSteps = iSplineSteps;

	TSTATICCAST( ABlobLifeLostHud, pScene )->SetBlob( a_pBlob );
	pXUI->TransitionTo( TNULL, pScene );
}

// The ball stops where it is (STATE_DEAD and the game over states 12 to 14)
static void StopBodyMotion( TPhysicsObjectContainer* a_pBody )
{
	if ( a_pBody != TNULL && a_pBody->IsInSimulation() )
	{
		a_pBody->m_body->setLinearVelocity( hkVector4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		a_pBody->m_body->setAngularVelocity( hkVector4( 0.0f, 0.0f, 0.0f, 0.0f ) );
	}
}

// $deBlob: FUNCTION 0073f5e0
void ABlobEntity::UpdatePhysics( TFLOAT a_fTimeStep )
{
	// The entity follows the body
	SetMatrix( m_oPhysicsMatrix );

	// Keep the body awake
	if ( m_pBody != TNULL && m_pBody->IsInSimulation() && !m_pBody->m_body->isActive() )
		m_pBody->m_body->activate();

	// The paint particle phantoms follow the ball (20 around it)
	TVector4 avParticleBox[ 2 ];
	avParticleBox[ 0 ] = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	avParticleBox[ 1 ] = TVector4( 20.0f, 20.0f, 20.0f, 1.0f );
	TPhysicsManager::GetSingleton()->SetAabbPhantomBox( m_pPaintParticlePhantomA, avParticleBox );
	TPhysicsManager::GetSingleton()->SetAabbPhantomBox( m_pPaintParticlePhantomB, avParticleBox );

	// Inked: a paint point every m_fInkTime / 10 s
	if ( m_eState != STATE_DEAD && m_eState != STATE_LEVEL_EXIT )
	{
		TFLOAT& rfInkTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 2 ] ); // 0x588

		if ( ( ( m_pColourComponent->GetCurrentColour() >> 4 ) & 1 ) != 0 && IsControllable() )
		{
			rfInkTime = rfInkTime - a_fTimeStep;

			if ( 0.0f <= rfInkTime )
				goto LAB_0073f785;

			RemovePaintPoints( 1, TTRUE );
		}

		rfInkTime = ms_pBlobCommonProperties->m_fInkTime / 10.0f;
	}

LAB_0073f785:

	// The paint points changed the size class (SetPaintPoints only sets m_iSize): resize the ball (the body, the
	// phantoms, the jelly, the bounding box) now, except in states 4, 5, 7, 10 and 11. Without
	// this the body kept the old radius while the contact query and the wall/ground tests used the new one.
	if ( m_iOldSize != m_iSize && m_eState != STATE_LMARK && m_eState != STATE_BOSS && m_eState != STATE_CANNON && m_eState != STATE_LEVEL_EXIT && m_eState != STATE_DEAD )
		OnSizeChanged();

	// Held in place (BFLAG 0x200000) during the animation state 0x1F
	if ( ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) != 0 && m_pAnimationComponent->m_iState == ABlobAnimationComponent::STATE_POWERUP )
	{
		SetPhysicsPosition( m_vHoldPosition );
		TPhysicsManager::StopBody( m_pBody );
	}

	if ( m_uiFlags & 0x10000 )
	{
		TFLOAT& rfTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 0 ] ); // 0x580
		rfTime         = rfTime - a_fTimeStep;

		if ( rfTime < 0.0f )
			m_uiFlags &= ~0x10000;
	}

	// The electrocution's time (0x584)
	if ( m_uiFlags & 0x2000000 )
	{
		TFLOAT& rfTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 1 ] ); // 0x584
		rfTime         = rfTime - a_fTimeStep;

		if ( rfTime < 0.0f )
			EndElectrocution();
	}

	TFLOAT fRadius = ms_pBlobProperties[ m_iSize ].m_fRadius;
	m_uiFlags &= ~8;

	// The contacts around the body: a sphere 0.7 larger than the ball (NOTE: retail's TSphereData is on the stack)
	TSphereData oSphere;
	oSphere.SetRadius( fRadius + 0.7f );

	TMatrix44 oMatrix = TMatrix44::IDENTITY;
	oMatrix.SetTranslation( m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );

	TPhysicsShapeContact aContacts[ 16 ];
	TINT                 iNumFound;

	{
		TASSERT( m_pSoftCollisionQueryHelper, "phantomContainer" );

		if ( !m_pSoftCollisionQueryHelper->IsInSimulation() )
			iNumFound = TPhysicsManager::GetSingleton()->CollideShape( &oSphere, oMatrix, 0xE3, aContacts, 16, TNULL );
		else
			iNumFound = TPhysicsManager::GetSingleton()->CollideShape( &oSphere, oMatrix, 0xE3, aContacts, 16, m_pSoftCollisionQueryHelper );
	}

	// Keep up to 4 contacts that aren't triggers (material 0x2010)
	TINT iNumContacts = 0;

	for ( TINT i = 0; i < iNumFound; i++ )
	{
		if ( ( aContacts[ i ].m_uiMaterial & 0x2010 ) == 0 )
			m_oMotion.m_aContacts[ iNumContacts++ ] = aContacts[ i ];

		if ( 3 < iNumContacts )
			break;
	}

	m_oMotion.m_uiContactFlags &= 0xFFF3;
	m_oMotion.m_uiContactCountAndGround = TUINT16( 0xFF00 | TUINT8( iNumContacts ) );
	m_oMotion.m_uiWallContact           = 0xFF;

	if ( iNumContacts != 0 )
	{
		m_uiFlags |= 8;

		const TVector4& rcPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

		for ( TINT i = 0; i < iNumContacts; i++ )
		{
			TPhysicsShapeContact& rContact = m_oMotion.m_aContacts[ i ];
			TVector4&             rNormal  = rContact.m_vSurfaceNormal;

			// Face the normal towards the ball
			if ( rNormal.x * ( rcPosition.x - rContact.m_vPosition.x ) + rNormal.y * ( rcPosition.y - rContact.m_vPosition.y ) + rNormal.z * ( rcPosition.z - rContact.m_vPosition.z ) < 0.0f )
			{
				rNormal.x = -rNormal.x;
				rNormal.y = -rNormal.y;
				rNormal.z = -rNormal.z;
			}

			if ( -s_fGroundContactCos <= rNormal.y )
			{
				// A wall (not too steep, the first one within the ball's radius)
				if ( rNormal.y < s_fWallContactCos && ( m_oMotion.m_uiContactFlags & 4 ) == 0 )
				{
					TVector4 vOffset;
					vOffset.x = rcPosition.x - rContact.m_vPosition.x;
					vOffset.y = rcPosition.y - rContact.m_vPosition.y;
					vOffset.w = rcPosition.w;
					vOffset.z = rcPosition.z - rContact.m_vPosition.z;

					TFLOAT fDot = rNormal.y * vOffset.y + rNormal.x * vOffset.x + rNormal.z * vOffset.z;

					if ( 0.0f < fDot )
					{
						TVector4 vAlong;
						vAlong.Multiply( rNormal, fDot );
						vOffset.x = vOffset.x - vAlong.x;
						vOffset.y = vOffset.y - vAlong.y;
						vOffset.z = vOffset.z - vAlong.z;

						if ( vOffset.Magnitude() < fRadius - 0.1f )
						{
							m_oMotion.m_uiContactFlags |= 4;
							m_oMotion.m_uiWallContact = TUINT8( i );
							m_oMotion.m_vWallNormal   = rNormal;
						}
					}
				}
			}
			else if ( ( m_oMotion.m_uiContactFlags & 8 ) == 0 && fRadius * 0.5f + rcPosition.y < rContact.m_vPosition.y )
			{
				// The ground (y points down): below the ball, within its radius in XZ
				if ( TVector4::DistanceXZ( rContact.m_vPosition, rcPosition ) <= fRadius - 0.1f )
				{
					m_oMotion.m_uiContactFlags |= 8;
					m_oMotion.m_uiContactCountAndGround = TUINT16( ( m_oMotion.m_uiContactCountAndGround & 0xFF ) | ( TUINT8( i ) << 8 ) );
					m_oMotion.m_vUnk270                 = rNormal;
				}
			}
		}

		m_oMotion.m_vContactNormal = m_oMotion.m_aContacts[ 0 ].m_vSurfaceNormal;

		TFLOAT fNormalY = m_oMotion.m_aContacts[ 0 ].m_vNormal.y;

		if ( -s_fGroundContactCos <= fNormalY )
		{
			if ( s_fWallContactCos <= fNormalY )
				goto LAB_0073fd5b;

			m_oMotion.m_auiUnk280[ 7 ] = m_oMotion.m_aContacts[ 0 ].m_uiMaterial; // 0x31C
		}
		else
		{
			m_oMotion.m_auiUnk280[ 6 ] = m_oMotion.m_aContacts[ 0 ].m_uiMaterial; // 0x318
		}

		m_oMotion.m_auiUnk280[ 5 ] = TUINT32( m_oMotion.m_aContacts[ 0 ].m_pBody ); // 0x314
	}

LAB_0073fd5b:
	UpdateHazards();
	UpdateStuck( a_fTimeStep );

	// The water and ink around the ball (in state 10 with the animation state 0x10, around the char model's base bone)
	if ( m_eState == STATE_LEVEL_EXIT && m_pAnimationComponent->m_iState == ABlobAnimationComponent::STATE_LEVEL_EXIT )
	{
		TASSERT( !( m_pBody != TNULL && m_pBody->IsInSimulation() ), "!pPM->IsInSimulation(m_pBody)" );

		TMatrix44 mBone;
		m_pAnimationComponent->GetCharModelInstance()->GetBoneTransform( ABlobAnimationComponent::ms_iBoneSBBase, mBone );

		TVector4 vPosition = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
		vPosition.y        = vPosition.y - ms_pBlobProperties[ m_iSize ].m_fRadius;
		UpdateLiquidCollision( vPosition );
	}
	else
	{
		UpdateLiquidCollision( m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
	}

	UpdateStateTransitions( a_fTimeStep );

	if ( m_eState == STATE_CANNON )
	{
		m_fSquashCompensation = 1.0f;
		m_fSquash             = 1.0f;
	}
	else
	{
		UpdateJelly( a_fTimeStep );
	}

	// The paint threads: on a wall or the ground with paint, every random 0.1-0.2 of squared distance (or while more than
	// 4 threads are free) a thread is left at each contact (one per normal)
	TVector4 vBall( m_vBallOffset.x + m_oPhysicsMatrix.m_f41, m_vBallOffset.y + m_oPhysicsMatrix.m_f42, m_vBallOffset.z + m_oPhysicsMatrix.m_f43, m_vBallOffset.w );
	TUINT32  uiThreadColour = m_pColourComponent->GetCurrentColour();

	if ( ( m_uiFlags & 0xC ) != 0 && ( ( uiThreadColour & ABlobColourComponent::COLOUR_BLACK ) != 0 || uiThreadColour != 0 ) )
	{
		TFLOAT fDistanceSq = TVector4::DistanceSq( m_vThreadPosition, vBall );

		if ( fDistanceSq > m_fThreadDistanceSq || m_oFreeThreads.m_iCount > 4 )
		{
			TVector4 vLastNormal = TVector4::VEC_ZERO; // X0Y0Z0W1

			for ( TINT i = 0; i < TUINT8( m_oMotion.m_uiContactCountAndGround ); i++ )
			{
				TPhysicsShapeContact& rContact = m_oMotion.m_aContacts[ i ];

				if ( rContact.m_vSurfaceNormal.y * vLastNormal.y + vLastNormal.x * rContact.m_vSurfaceNormal.x + vLastNormal.z * rContact.m_vSurfaceNormal.z < 0.99f )
				{
					AddThread( rContact.m_vPosition, rContact.m_vSurfaceNormal );
					vLastNormal = rContact.m_vSurfaceNormal;
				}
			}

			TFLOAT fRandom      = ARandom::GetSingleton()->GetRandom().GetFloat();
			m_vThreadPosition   = vBall;
			m_fThreadDistanceSq = fRandom * 0.1f + 0.1f;
		}
	}

	// The action button (2): the start prompt of the challenge whose marker Blob touches (unless Z-jumping at a
	// lock), else the read button of the help trigger, else into the wind cannon
	if ( ( ( m_uiPrevButtons >> 2 ^ m_uiButtons >> 2 ) & m_uiButtons >> 2 & 1 ) != 0 && !m_bInBlobVision )
	{
		AChallenge* pChallenge = AChallengeManager::GetSingleton()->GetNearbyChallenge();

		if ( pChallenge != TNULL && !( pChallenge->GetFlags() & AChallenge::Flags_Completed ) )
		{
			ABlobZJumpComponent::PriorityTarget* pLock = m_pZJump->GetCurrentPriorityLock();

			if ( pLock == TNULL || pLock->m_pTarget == TNULL || m_pZJump->GetState() != ABlobZJumpComponent::STATE_LOCK )
				AChallengeManager::GetSingleton()->ShowChallengeScreen();
		}
		else if ( m_pHelpTrigger != TNULL && !( m_pHelpTrigger->GetHelpFlags() & 8 ) )
		{
			m_pHelpTrigger->Activate();
		}
		else if ( m_pWindCannon != TNULL )
		{
			SetState( STATE_CANNON );
		}
	}

	// The Blob vision button (10)
	if ( ( ( m_uiButtons >> 10 ) & 1 ) != 0 && CanEnterBlobVision() )
	{
		m_bInBlobVision = TTRUE;

		TXUI* pXUI = TXUI::GetSingleton();
		pXUI->TransitionTo( TNULL, pXUI->CreateScene( pXUI->FindResource( "blobvision.xur" ), 0 ) );
	}

	// The jump cool-down
	if ( 0.0f < *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 1 ] ) )
		*TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 1 ] ) -= a_fTimeStep;

	// Shaking off the LeechBots (button 14)
	if ( m_iLeechbotsAttached != 0 && ( ( m_uiButtons >> 14 ) & 1 ) != 0 )
		AEnemyGrappler::PlayerStruggle( this );

	switch ( m_eState )
	{
		case STATE_ROLL:
		{
			ACritter::SquashCritters( this );
			ACiv::SquashTransformedCivs( this );

			if ( IsControllable() )
			{
				TFLOAT& rfHurtTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 2 ] ); // 0x56C

				if ( !HasGroundContactFlag( 0x100000 ) && !HasWallContactFlag( 0x100000 ) )
				{
					rfHurtTime = 0.0f;
					StopRollEffects();
				}
				else
				{
					// On a hurting surface (material 0x100000): a paint point every m_fHurtTime s
					rfHurtTime = rfHurtTime - a_fTimeStep;

					if ( rfHurtTime < 0.0f )
					{
						rfHurtTime = ms_pBlobCommonProperties->m_fHurtTime;
						RemovePaintPoints( 1, TTRUE );

						if ( m_iPaintPoints == 0 )
						{
							// code_r0x0074024f
							OnOutOfPoints( TFALSE );
							break;
						}
					}

					UpdateRollEffects();

					if ( m_pAnimationComponent->m_iState == ABlobAnimationComponent::STATE_PAIN )
					{
						// code_r0x00740b3b: held in place while hurt and still
						if ( m_oMotion.m_vUnk220.MagnitudeSq() <= 0.0f )
						{
							if ( ( m_uiFlags & 0x40000 ) == 0 )
							{
								m_uiFlags |= 0x40000;
								m_vHoldPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
							}

							SetPhysicsPosition( m_vHoldPosition );
							TPhysicsManager::StopBody( m_pBody );
						}

						goto code_r0x007420cc;
					}
				}
			}

			UpdateSurfaceEffect();

			if ( !UpdateBrake( a_fTimeStep ) )
			{
				if ( GetZJumpState() == ABlobZJumpComponent::STATE_POINT )
				{
					TVector4 vVelocity;
					TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );
					vVelocity.z = 0.0f;
					vVelocity.x = 0.0f;
					TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );
				}
				else
				{
					UpdateRollMovement( a_fTimeStep );
				}

				TINT iZJumpState = GetZJumpState();

				if ( iZJumpState == ABlobZJumpComponent::STATE_POINT || ( iZJumpState == ABlobZJumpComponent::STATE_LOCK && m_pZJump->HasValidPriorityLock() ) || ( iZJumpState == ABlobZJumpComponent::STATE_LOCKLMARK && m_pZJump->CanJumpToLandmark() ) )
				{
					TryZJumpFromGround( a_fTimeStep );
				}
				else
				{
					UpdateJump( a_fTimeStep );
				}

				goto code_r0x007420cc;
			}

			break;
		}
		case STATE_BALLISTIC:
		{
			// A late jump
			if ( 0.4f < *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5A8[ 0 ] ) && *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5A8[ 1 ] ) <= 0.0f )
				UpdateJump( a_fTimeStep );

			TVector4 vVelocity;
			TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );

			// Falling (y points down): remember where it started
			if ( 0.0f <= vVelocity.y )
			{
				if ( ( m_uiFlags & 0x800000 ) == 0 )
					m_vClimbStart = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

				m_uiFlags |= 0x800000;
			}

			// The effect at 0xB10 follows the ball
			if ( m_auiSounds[ 14 ] != 0 ) // 0xB10
				TREINTERPRETCAST( TParticleEffectInstance*, m_auiSounds[ 14 ] )->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );

			UpdateSurfaceEffect();
			if ( ( m_uiFlags & 0x10000 ) == 0 && m_iLeechbotsAttached == 0 )
			{
				TINT iZJumpState = GetZJumpState();

				if ( ( iZJumpState == ABlobZJumpComponent::STATE_LOCK && m_pZJump->HasValidPriorityLock() ) || ( iZJumpState == ABlobZJumpComponent::STATE_LOCKLMARK && m_pZJump->CanJumpToLandmark() ) )
				{
					TryZJumpFromAir( a_fTimeStep );
				}

				iZJumpState = GetZJumpState();

				if ( iZJumpState != ABlobZJumpComponent::STATE_ATTACKFLIGHT && iZJumpState != ABlobZJumpComponent::STATE_ATTACKSLAM && ( ( m_uiButtons >> 5 ) & 1 ) != 0 )
				{
					// The slam: straight down, at least at m_afUnk38[ 8 ] (y points down)
					TVector4 vSlamVelocity;
					TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vSlamVelocity );
					vSlamVelocity.z = 0.0f;
					vSlamVelocity.x = 0.0f;

					if ( ( m_uiFlags & 0x20000 ) == 0 )
					{
						if ( vSlamVelocity.y < ms_pBlobCommonProperties->m_afUnk38[ 8 ] )
							vSlamVelocity.y = ms_pBlobCommonProperties->m_afUnk38[ 8 ];

						TPhysicsManager::SetLinearVelocity( m_pBody, vSlamVelocity );
						m_uiFlags |= 0x20000;

						TINT iAnimState = m_pAnimationComponent->GetState();

						if ( iAnimState != ABlobAnimationComponent::STATE_SHIELD_ACTIVATING && iAnimState != ABlobAnimationComponent::STATE_SHIELD_DEACTIVATING && iAnimState != ABlobAnimationComponent::STATE_SHIELD_ACTIVE )
							m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_BRAKE );
					}
					else
					{
						TPhysicsManager::SetLinearVelocity( m_pBody, vSlamVelocity );
					}

					TPhysicsManager::SetAngularVelocity( m_pBody, TVector4::VEC_ZERO ); // X0Y0Z0W1
					m_uiFlags &= ~0x800;
					m_pZJump->EndAttack( TFALSE );
					m_uiFlags &= ~0x1000000;
					m_pZJump->EndCharge();
					m_uiFlags |= 0x2000;

					// The jelly sits on the ball
					TVector4 vJelly      = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
					vJelly.y             = ms_pBlobProperties[ m_iSize ].m_fJellyOffset * 0.5f * ms_pBlobProperties[ m_iSize ].m_fRadius + m_oPhysicsMatrix.m_f42;
					m_oJelly.m_vRest     = vJelly;
					m_oJelly.m_vPosition = vJelly;

					TParticleEffectInstance*& rpTrail = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 14 ] ); // 0xB10

					if ( rpTrail != TNULL )
					{
						rpTrail->Stop();
						rpTrail = TNULL;
					}
					break;
				}
			}

			TUINT32 uiFlags = m_uiFlags;
			m_uiFlags       = uiFlags & 0xFFF9DFFF;

			switch ( GetZJumpState() )
			{
				case ABlobZJumpComponent::STATE_NODEFLIGHT:
				case ABlobZJumpComponent::STATE_SETEND:
				case ABlobZJumpComponent::STATE_TIMEOUT:
				case ABlobZJumpComponent::STATE_ATTACKFLIGHT:
				case ABlobZJumpComponent::STATE_LMARKFLIGHT:
				case ABlobZJumpComponent::STATE_SWITCHSLAM:
					break;
				case ABlobZJumpComponent::STATE_ATTACKSLAM:
				{
					TVector4 vZJumpVelocity;
					TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vZJumpVelocity );
					vZJumpVelocity.z = 0.0f;
					vZJumpVelocity.x = 0.0f;

					if ( vZJumpVelocity.y < ms_pBlobCommonProperties->m_afUnk38[ 7 ] )
						vZJumpVelocity.y = ms_pBlobCommonProperties->m_afUnk38[ 7 ];

					TPhysicsManager::SetLinearVelocity( m_pBody, vZJumpVelocity );
					m_uiFlags |= 0x20000;
					break;
				}
				default:
					if ( ( uiFlags & 0x1000000 ) == 0 )
						UpdateAirMovement( a_fTimeStep );

					break;
			}

			// Painted: a paint drop now and then (every 0.05 to 0.1 of the radius)
			if ( m_pColourComponent->GetCurrentColour() != 0 )
			{
				TFLOAT  fRadius      = ms_pBlobProperties[ m_iSize ].m_fRadius;
				TFLOAT& rfSprayTimer = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 4 ] ); // 0x574
				rfSprayTimer         = rfSprayTimer - a_fTimeStep;

				if ( 0.0f <= rfSprayTimer )
					goto code_r0x007420cc;

				TRandom& rRandom = ARandom::GetSingleton()->GetRandom();
				rfSprayTimer     = ( rRandom.GetFloat() * 0.05f + 0.05f ) * fRadius;

				TVector4 vPosition( m_oPhysicsMatrix.m_f41, fRadius * 0.8f + m_oPhysicsMatrix.m_f42, m_oPhysicsMatrix.m_f43, m_oPhysicsMatrix.m_f44 );

				// A quarter of the body's velocity, pushed sideways at random
				TVector4 vLinear;
				TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vLinear );

				TVector4 vVelocity;
				vVelocity.Multiply( vLinear, 0.25f );

				TVector4 vSide = vVelocity;
				vSide.NormaliseXZ();
				vSide.CrossProduct( vSide, TVector4::VEC_NEGY );
				vSide.Multiply( ( rRandom.GetFloat() - 0.5f ) * 5.0f );

				vVelocity.x = vSide.x + vVelocity.x;
				vVelocity.y = vSide.y + vVelocity.y;
				vVelocity.z = vSide.z + vVelocity.z;

				TUINT32 uiIndex = m_pColourComponent->GetCurrentColour();
				if ( ( uiIndex & 0x10 ) != 0 )
					uiIndex = 0x10;

				APaintParticleSystem::s_Instance->AddParticle(
				    vPosition,
				    vVelocity,
				    m_pColourComponent->m_uiFadeColour2,
				    fRadius,
				    TINT( TREINTERPRETCAST( TUINTPTR, m_pMaterialTestSplat ) ),
				    TINT( TREINTERPRETCAST( TUINTPTR, ms_apInkCloudSmallEffects[ uiIndex ] ) ),
				    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomA ) ),
				    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomB ) )
				);
			}

			break;
		}
		case STATE_STUCKWALL:
		{
			if ( IsControllable() )
			{
				TFLOAT& rfHurtTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 2 ] ); // 0x56C

				if ( !HasWallContactFlag( 0x100000 ) )
				{
					rfHurtTime = 0.0f;
					StopRollEffects();
				}
				else
				{
					// On a hurting surface (material 0x100000): a paint point every m_fHurtTime s
					rfHurtTime = rfHurtTime - a_fTimeStep;

					if ( rfHurtTime < 0.0f )
					{
						rfHurtTime = ms_pBlobCommonProperties->m_fHurtTime;
						RemovePaintPoints( 1, TTRUE );

						if ( m_iPaintPoints == 0 )
						{
							// code_r0x0074024f
							OnOutOfPoints( TFALSE );
							break;
						}
					}

					UpdateRollEffects();

					if ( m_pAnimationComponent->m_iState == ABlobAnimationComponent::STATE_PAIN )
					{
						// code_r0x00740b3b: held in place while hurt and still
						if ( m_oMotion.m_vUnk220.MagnitudeSq() <= 0.0f )
						{
							if ( ( m_uiFlags & 0x40000 ) == 0 )
							{
								m_uiFlags |= 0x40000;
								m_vHoldPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
							}

							SetPhysicsPosition( m_vHoldPosition );
							TPhysicsManager::StopBody( m_pBody );
						}

						goto code_r0x007420cc;
					}
				}
			}

			TINT  iZJumpState = GetZJumpState();
			TBOOL bHoldOrPush = TFALSE;

			if ( iZJumpState == ABlobZJumpComponent::STATE_POINT || iZJumpState == ABlobZJumpComponent::STATE_AIMNODE || ( iZJumpState == ABlobZJumpComponent::STATE_LOCK && m_pZJump->GetCurrentPriorityLock() != TNULL ) || iZJumpState == ABlobZJumpComponent::STATE_LOCKLMARK )
			{
				if ( ( ( m_uiPrevButtons ^ m_uiButtons ) & m_uiButtons & ABlobInputMap::BUTTON_JUMP ) != 0 && LaunchZJump( a_fTimeStep, m_oMotion.m_vWallNormal ) )
					SetState( STATE_BALLISTIC );
				else
					bHoldOrPush = TTRUE;
			}
			else
			{
				TBOOL bWallJumped = TFALSE;

				if ( iZJumpState != ABlobZJumpComponent::STATE_AIM )
					bWallJumped = TryWallJump( a_fTimeStep );

				bHoldOrPush = iZJumpState == ABlobZJumpComponent::STATE_AIM || !bWallJumped;
			}

			if ( bHoldOrPush )
			{
				iZJumpState = GetZJumpState();

				if ( iZJumpState == ABlobZJumpComponent::STATE_AIM || iZJumpState == ABlobZJumpComponent::STATE_AIMNODE )
				{
					// Hold the ball where it is
					if ( ( m_uiFlags & 0x40000 ) == 0 )
					{
						m_uiFlags |= 0x40000;
						m_vHoldPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
					}
					else
					{
						SetPhysicsPosition( m_vHoldPosition );
						TPhysicsManager::StopBody( m_pBody );
					}
				}
				else
				{
					TPhysicsManager*  pPM         = TPhysicsManager::GetSingleton();
					const Properties& rProperties = ms_pBlobProperties[ m_iSize ];

					// Press the ball against the wall and (while not going up) hold some of its weight
					TVector4 vForce;
					vForce.Multiply( m_oMotion.m_vWallNormal, ms_pBlobCommonProperties->m_afUnk14[ 3 ] * -80.0f * rProperties.m_fMass );

					TVector4 vGravity = pPM->GetInitData().m_vGravity;

					TVector4 vVelocity;
					pPM->GetLinearVelocity( m_pBody, vVelocity );

					if ( 0.0f <= vVelocity.y )
					{
						TVector4 vHold;
						vHold.Multiply( vGravity, -( rProperties.m_fMass * ms_pBlobCommonProperties->m_afUnk14[ 4 ] ) );
						vForce.x = vHold.x + vForce.x;
						vForce.y = vHold.y + vForce.y;
						vForce.z = vHold.z + vForce.z;
					}

					pPM->ApplyForce( m_pBody, vForce );
					UpdateWallMovement( a_fTimeStep );
				}
			}

			break;
		}
		case STATE_HOTPLATE:
		{
			// Shaking off the hot plate: each shake button press counts, enough of them bounce the ball off
			if ( m_uiButtons & ( m_uiButtons ^ m_uiPrevButtons ) & ABlobInputMap::BUTTON_JUMP2 )
			{
				TINT16& riShakes      = *TREINTERPRETCAST( TINT16*, TREINTERPRETCAST( TUINT8*, &m_uiTargettingAndShakes ) + 2 ); // 0xAD2
				TINT16  iShakesNeeded = TINT16( m_uiShakesNeeded );
				riShakes++;

				TINT iProgress = ( iShakesNeeded == 0 ) ? 100 : TINT( ( TFLOAT( riShakes ) * 100.0f ) / TFLOAT( iShakesNeeded ) );
				TSTATICCAST( ABlobShakeHud, ABlobHud::GetSingleton()->GetShakeHud() )->SetHotplateProgress( iProgress );

				if ( riShakes >= TINT16( m_uiShakesNeeded ) )
				{
					BounceOffContact();
					m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_PAIN );
					break;
				}
			}

			// Every m_fHotPlateTime seconds on it, the hot plate takes time: a time bonus flies from the ball to one of its
			// spawn points and its seconds come off the level's time
			TFLOAT& rfHotPlateTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 2 ] ); // 0x56C
			rfHotPlateTime += a_fTimeStep;

			if ( rfHotPlateTime > ms_pBlobCommonProperties->m_fHotPlateTime && AGameStatus::GetSingleton()->GetLevelData().m_fTimeLeft > 0.0f )
			{
				ASpawnPointPool::SpawnPoint* pPoint = TSTATICCAST( ASpawnPointPool::SpawnPoint, m_pHotPlate->GetSpawnPoint() );

				if ( pPoint != TNULL )
				{
					const TVector4&        rcPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
					APickup*               pPickup    = APickupManager::CreatePickup( 1, rcPosition, -1.0f, ms_pBlobCommonProperties->m_fPickupLifeTime );
					APropTriggerTimeBonus* pBonus     = ( pPickup != TNULL ) ? pPickup->CastTo<APropTriggerTimeBonus>() : TNULL;

					if ( pBonus == TNULL )
					{
						pPoint->m_pOwner->ReleaseSpawnPoint( pPoint );
					}
					else
					{
						pBonus->Fly( rcPosition, pPoint->m_vPosition, -1.0f, 0 );
						pBonus->SetCallback( ASpawnPointPool::ReleaseSpawnPointCallback, pPoint );

						AGameStatus::GetSingleton()->GetLevelData().m_fTimeLeft -= TFLOAT( pBonus->GetSeconds() );

						static TWCHAR s_wszTimeMessage[ 0x80 ];
						TStringManager::String16Format( s_wszTimeMessage, 0x80, T2Locale::GetSingleton()->GetString( 0x1B ), -pBonus->GetSeconds() );
						AGameStatus::GetSingleton()->GetLevelData().ShowMessage( s_wszTimeMessage, 2, &rcPosition, APlayerManager::GetSingleton()->GetPlayerIndex( this ) );
						rfHotPlateTime = 0.0f;
					}
				}
			}

			// Held where it landed on the plate
			if ( !( m_uiFlags & 0x40000 ) )
			{
				m_uiFlags |= 0x40000;
				m_vHoldPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
			}
			else
			{
				SetPhysicsPosition( m_vHoldPosition );
			}

			TFALLTHROUGH;
		}
		case STATE_RESPAWN:
			// The body doesn't move or turn
			if ( m_pBody != TNULL && m_pBody->IsInSimulation() )
			{
				m_pBody->m_body->setLinearVelocity( hkVector4( 0.0f, 0.0f, 0.0f, 0.0f ) );
				m_pBody->m_body->setAngularVelocity( hkVector4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			}

			break;
		case STATE_LMARK:
		{
			// Shaking the landmark's trigger: each shake button press paints it once, the camera button lets go
			if ( m_pLandmark->m_uiFlags & ALandmarkInstance::Flags_TriggersHidden )
			{
				ExitLandmark( TFALSE );
				break;
			}

			// Painted: waits for the landmark (ALandmarkInstance::UpdateImp ends the state)
			if ( m_pLandmark->GetTriggerPaintLeft( m_iCurrentLandmarkTrigger ) <= 0 )
				break;

			// The ball's colour must match the trigger's (any colour but white and ink for the ones of any colour), the
			// rainbow paint matches every one
			TUINT32 uiTriggerColour = TUINT32( m_pLandmark->GetTriggerColour( m_iCurrentLandmarkTrigger ) );
			TBOOL   bCanPaint       = TTRUE;
			TUINT32 uiColour        = m_pColourComponent->GetCurrentColour();

			if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
				uiColour = ABlobColourComponent::COLOUR_BLACK;

			if ( uiTriggerColour == 0 )
			{
				if ( uiColour == 0 || uiColour > 0xF )
					bCanPaint = TFALSE;
			}
			else if ( uiTriggerColour != uiColour )
			{
				bCanPaint = ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) != 0;
			}

			TUINT32 uiPressed = m_uiButtons & ( m_uiButtons ^ m_uiPrevButtons );

			if ( ( uiPressed & ABlobInputMap::BUTTON_BRAKE ) || !bCanPaint )
			{
				ExitLandmark( TFALSE );
				m_uiFlags |= 0x10000;
				*TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 0 ] ) = 0.5f; // 0x580
				break;
			}

			TINT iShakes = ( uiPressed & ABlobInputMap::BUTTON_JUMP2 ) ? 1 : 0;

			if ( ( uiPressed & ABlobInputMap::BUTTON_JUMP3 ) && TInputDevice::GetLastGamepadType() == TInputDevice::Gamepad_Wii )
				iShakes++;

			TFLOAT& rfShakeTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 2 ] ); // 0x56C

			if ( iShakes == 0 )
			{
				rfShakeTime -= a_fTimeStep;

				if ( rfShakeTime < 0.0f )
				{
					rfShakeTime = 0.0f;
					m_pSoundComponent->SetShakerOff();
				}
			}
			else
			{
				m_pSoundComponent->SetShakerOn();
				rfShakeTime = 0.1f;
			}

			TBOOL bLetGo = TFALSE;

			while ( iShakes > 0 && m_pLandmark->GetTriggerPaintLeft( m_iCurrentLandmarkTrigger ) > 0 )
			{
				iShakes--;

				AParticleEffectHelper::SetColour( ms_pLandmarkPaintSplatEffect, m_pColourComponent->GetFadeColour2(), TNULL );
				TParticleEffectInstance* pSplat = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pLandmarkPaintSplatEffect );

				if ( pSplat != TNULL )
					pSplat->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );

				ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 11, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 1.0f, -1 );
				m_pLandmark->PaintTrigger( m_iCurrentLandmarkTrigger, this );
				RemovePaintPoints( 1, TFALSE );

				// Out of paint while the trigger isn't painted yet: Blob lets go
				if ( m_iPaintPoints == 0 && m_pLandmark->GetTriggerPaintLeft( m_iCurrentLandmarkTrigger ) > 0 )
				{
					bLetGo = TTRUE;
					break;
				}
			}

			if ( bLetGo )
			{
				ExitLandmark( TFALSE );
				break;
			}

			// The trigger is painted: Blob jumps off it, or stays for the completion of the last one
			if ( m_pLandmark->GetTriggerPaintLeft( m_iCurrentLandmarkTrigger ) <= 0 )
			{
				if ( m_pLandmark->GetNumTriggersLeft() <= 0 )
					m_pSoundComponent->StopShaker();
				else
					ExitLandmark( TTRUE );
			}

			break;
		}
		case STATE_BOSS:
		{
			// Shaking the boss: each shake button press costs the boss a life point, the camera button lets go
			TUINT32 uiPressed = m_uiButtons & ( m_uiButtons ^ m_uiPrevButtons );

			if ( uiPressed & ABlobInputMap::BUTTON_BRAKE )
			{
				ExitBoss( TFALSE );
				m_uiFlags |= 0x10000;
				*TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 0 ] ) = 0.5f; // 0x580
				break;
			}

			TINT iShakes = ( uiPressed & ABlobInputMap::BUTTON_JUMP2 ) ? 1 : 0;

			if ( ( uiPressed & ABlobInputMap::BUTTON_JUMP3 ) && TInputDevice::GetLastGamepadType() == TInputDevice::Gamepad_Wii )
				iShakes++;

			TFLOAT& rfShakeTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 2 ] ); // 0x56C

			if ( iShakes > 0 )
			{
				m_pSoundComponent->SetShakerOn();
				rfShakeTime = 0.1f;
			}
			else
			{
				rfShakeTime -= a_fTimeStep;

				if ( rfShakeTime < 0.0f )
				{
					rfShakeTime = 0.0f;
					m_pSoundComponent->SetShakerOff();
				}
			}

			TBOOL bLetGo = TFALSE;

			while ( iShakes > 0 && m_pBoss->GetLife() > 0 )
			{
				iShakes--;

				AParticleEffectHelper::SetColour( ms_pLandmarkPaintSplatEffect, m_pColourComponent->GetFadeColour2(), TNULL );
				TParticleEffectInstance* pSplat = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pLandmarkPaintSplatEffect );

				if ( pSplat != TNULL )
				{
					TVector4 vEye;
					m_pBoss->GetEyePosition( vEye );
					pSplat->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, vEye );
				}

				ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 11, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 1.0f, -1 );
				m_pBoss->SetLife( m_pBoss->GetLife() - 1 );
				RemovePaintPoints( 1, TFALSE );

				if ( ABlobSizeHud* pSizeHud = m_pSizeHud )
					pSizeHud->SetLandmarkProgress( m_pBoss->GetLife(), m_pBoss->GetMaxLife() );

				ABlobHud::GetSingleton()->ShowRandomSplat();

				// Out of paint while the boss still has life: Blob lets go
				if ( m_iPaintPoints == 0 && m_pBoss->GetLife() > 0 )
				{
					bLetGo = TTRUE;
					break;
				}
			}

			if ( bLetGo )
				ExitBoss( TFALSE );
			else if ( m_pBoss->GetLife() <= 0 )
				ExitBoss( TTRUE );

			break;
		}
		case STATE_FLOAT:
		{
			// In water or ink
			TASSERT( m_uiFlags & 0x180, "m_uiFlags & (BFLAG_ININK|BFLAG_INWATER)" );
			TASSERT( m_oMotion.m_uiContactFlags & 3, "m_oCollData.m_uiFlags & (CollisionCache::FLAG_HIT_INK|CollisionCache::FLAG_HIT_WATER)" );

			if ( ( m_pColourComponent->GetCurrentColour() != 0 || AGameStatus::GetSingleton()->GetMultiPlayerGameType() != 1 || APlayerManager::GetSingleton()->IsNoBlobPainted() ) &&
			     ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) == 0 )
			{
				if ( m_uiFlags & 0x80 )
				{
					// The water washes the colour off and drains (or fills) the paint points towards m_uiWaterPoints
					WashOff();

					TFLOAT& rfDrainTime     = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 2 ] ); // 0x56C
					TFLOAT& rfDrainInterval = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 3 ] ); // 0x570

					if ( ms_pBlobCommonProperties->m_uiWaterPoints < TUINT32( m_iPaintPoints ) )
					{
						rfDrainTime += a_fTimeStep;

						if ( rfDrainInterval <= rfDrainTime && rfDrainTime != rfDrainInterval )
						{
							RemovePaintPoints( 1, TFALSE );
							rfDrainTime = 0.0f;

							TFLOAT fInterval = rfDrainInterval - ms_pBlobCommonProperties->m_fDrainIntervalStep;
							rfDrainInterval  = TMath::Max( fInterval, ms_pBlobCommonProperties->m_fMinDrainInterval );
						}
					}
					else if ( TUINT32( m_iPaintPoints ) < ms_pBlobCommonProperties->m_uiWaterPoints )
					{
						rfDrainTime += a_fTimeStep;

						if ( rfDrainInterval <= rfDrainTime && rfDrainTime != rfDrainInterval )
						{
							AddPaintPoints( 1 );
							rfDrainTime     = 0.0f;
							rfDrainInterval = 0.1f;
						}
					}
				}
				else if ( m_uiFlags & 0x100 )
				{
					ChangeColour( ABlobColourComponent::COLOUR_BLACK );
				}
			}

			UpdateLiquidBuoyancy();
			UpdateLiquidMovement( a_fTimeStep );

			// Near a flat surface: the jump out of it
			const TVector4& rNormal = m_oMotion.m_vLiquidNormal;

			if ( s_fLiquidSurfaceCos < rNormal.y * TVector4::VEC_NEGY.y + rNormal.x * TVector4::VEC_NEGY.x + rNormal.z * TVector4::VEC_NEGY.z &&
			     ( m_oPhysicsMatrix.m_f42 - m_oMotion.m_vLiquidHit.y ) - 0.5f < fRadius * 2.0f )
			{
				TINT iZJumpState = GetZJumpState();

				if ( iZJumpState == ABlobZJumpComponent::STATE_POINT || ( iZJumpState == ABlobZJumpComponent::STATE_LOCK && m_pZJump->GetCurrentPriorityLock() != TNULL ) || iZJumpState == ABlobZJumpComponent::STATE_LOCKLMARK )
				{
					TryLiquidZJump( a_fTimeStep );
				}
				else
				{
					TryLiquidJump( a_fTimeStep );
				}
			}

			break;
		}
		case STATE_GRASSBOMB:
			// Landed: the body leaves the simulation
			if ( m_oMotion.m_uiContactFlags & 0x20 )
			{
				TPhysicsManager::GetSingleton()->RemoveFromSimulation( m_pBody, TFALSE );
				ResetMotion();
			}

			break;
		case STATE_LEVEL_EXIT:
			ResetMotion();
			// NOTE: retail ignores the result
			CheckLiquid();
			break;
		case STATE_DEAD:
		{
			StopBodyMotion( m_pBody );

			APlayerManager::GetSingleton()->GetBlobDataOf( this );

			TXUI*      pXUI         = TXUI::GetSingleton();
			TXUIScene* pActiveScene = pXUI->GetActiveScene();

			if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 && !m_bLifeLostShown && pXUI->GetTransToScene() == TNULL &&
			     pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() && pActiveScene != TNULL && pActiveScene->IsA( &TGetClass( ABlobHud ) ) )
			{
				m_bLifeLostShown = TTRUE;
				ShowLifeLostScene( this );
				break;
			}

			// The splitscreen: the viewport fades to black before the respawn timer (0x58C) runs out, then the blob
			// respawns once its cell is ready
			if ( APlayerManager::GetSingleton()->GetNumPlayers() > 1 )
			{
				// $deBlob: 0x009b5bd0 (computed once)
				static TFLOAT s_cfFadeTime = 0.5f;
				TASSERT( s_cfFadeTime < ms_pBlobCommonProperties->m_fBlobKillRespawnTime );

				TFLOAT& rfRespawnTimer = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 3 ] ); // 0x58C
				TFLOAT  fPrevTimer     = rfRespawnTimer;

				if ( fPrevTimer <= 0.0f )
				{
					if ( ATerrain2Interface::GetInstance()->IsCurrentCellReady() && !ATerrain2Interface::GetInstance()->IsStreaming() &&
					     pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() )
						Respawn( TFALSE, TTRUE, TNULL );
				}
				else
				{
					rfRespawnTimer = fPrevTimer - a_fTimeStep;

					if ( rfRespawnTimer > 0.0f )
					{
						if ( s_cfFadeTime >= rfRespawnTimer && fPrevTimer > s_cfFadeTime )
						{
							TINT iViewport = GetBlobViewport( APlayerManager::GetSingleton(), this );

							if ( TUINT32( iViewport ) <= 11 && !AGameStateController::GetSingleton()->IsFadingToBlack( iViewport ) )
							{
								TASSERT( rfRespawnTimer > 0.0f && rfRespawnTimer <= s_cfFadeTime );
								AGameStateController::GetSingleton()->FadeToBlack( s_cfFadeTime, iViewport );
							}
						}
					}
					else
					{
						PrepareRespawn( TNULL, TNULL, TNULL );
					}
				}
			}

			break;
		}
		case STATE_FAIL:
		case STATE_VICTORY:
		case STATE_LOSTLASTLIFE:
		{
			// The game over: the life lost screen once the follow camera is in place
			StopBodyMotion( m_pBody );
			ResetMotion();

			APlayerManager::GetSingleton()->GetBlobDataOf( this );
			ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

			if ( pCamera->GetCameraType() != ACamera::CT_FREE )
				pCamera = APlayerManager::GetSingleton()->GetBlobCamera( this );

			TASSERT( pCamera->IsA( &TGetClass( ABlobFollowCamera ) ) );

			TXUI*      pXUI         = TXUI::GetSingleton();
			TXUIScene* pActiveScene = pXUI->GetActiveScene();

			if ( m_bLifeLostShown || pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked() ||
			     pActiveScene == TNULL || !pActiveScene->IsA( &TGetClass( ABlobHud ) ) ||
			     !TSTATICCAST( ABlobFollowCamera, pCamera )->IsAtIdealPos() )
				break;

			if ( m_eState == STATE_LOSTLASTLIFE )
			{
				// The last life: waits for the animation component's current animation
				if ( m_pAnimationComponent->GetCurrentAnim() != TNULL || AGameStatus::GetSingleton()->IsMultiplayer() )
					break;

				m_bLifeLostShown = TTRUE;
				ShowLifeLostScene( this );
				break;
			}

			m_bLifeLostShown = TTRUE;

			if ( m_eState == STATE_VICTORY && AGameStatus::GetSingleton()->IsLevelComplete() && ABlobHud::GetSingleton() != TNULL )
				ABlobHud::GetSingleton()->ShowWinner( AGameStatus::GetSingleton()->GetLevelWonLeader() );

			if ( m_bLifeLostShown && !AGameStatus::GetSingleton()->IsMultiplayer() )
				ShowLifeLostScene( this );

			break;
		}
	}

code_r0x007420cc:
	// The paint spray of a coloured Blob that spins fast (the rolling update). The timer at 0x578 counts down while the
	// Blob is coloured; when it runs out and the spin is faster than the body's max angular velocity, six paint
	// particles are added around the spin axis and the timer is set again.
	// NOTE: retail reaches this through jumps to code_r0x007420cc from the state code, which isn't translated yet, so the
	// spray runs in every frame that the colour and the timer allow it.
	if ( ( m_pColourComponent->GetCurrentColour() & 0x10 ) != 0 || m_pColourComponent->GetCurrentColour() != 0 )
	{
		TFLOAT& rfSprayTimer = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 5 ] ); // 0x578
		rfSprayTimer -= a_fTimeStep;

		if ( rfSprayTimer < 0.0f )
		{
			Toshi::TPhysicsManager* pPhysics = Toshi::TPhysicsManager::GetSingleton();

			// The spin: the angular velocity, normalised, is the spin axis
			TVector4 vSpin;
			pPhysics->GetAngularVelocity( m_pBody, vSpin );
			TFLOAT fSpeed = vSpin.NormaliseMag();

			Properties* pProperties = GetProperties( m_iSize );
			TFLOAT      fRadius     = pProperties->m_fRadius; // 0x14

			if ( pProperties->m_fMaxAngularVelocity * 0.95f < fSpeed )
			{
				TVector4 vLinear;
				pPhysics->GetLinearVelocity( m_pBody, vLinear );

				// The helper axis for the perpendicular: the basis vector least along the spin axis (z by default)
				TFLOAT fAbsX = TMath::Abs( vSpin.x );
				TFLOAT fAbsY = TMath::Abs( vSpin.y );
				TFLOAT fAbsZ = TMath::Abs( vSpin.z );

				TVector4 vHelper;
				vHelper.Set( 0.0f, 0.0f, 1.0f, 1.0f );
				if ( fAbsX < fAbsY && fAbsX < fAbsZ )
					vHelper.Set( 1.0f, 0.0f, 0.0f, 1.0f );
				else if ( fAbsY < fAbsX && fAbsY < fAbsZ )
					vHelper.Set( 0.0f, 1.0f, 0.0f, 1.0f );

				TVector4 vPerpendicular;
				vPerpendicular.CrossProduct( vSpin, vHelper );

				// The particles start at m_vBallOffset plus the body's position (x, y, z; w stays the one of m_vBallOffset)
				const TVector4& rvBody = GetBodyPosition();
				TVector4        vBase;
				vBase.Set( m_vBallOffset.x + rvBody.x, m_vBallOffset.y + rvBody.y, m_vBallOffset.z + rvBody.z, m_vBallOffset.w );

				TFLOAT fPositionScale  = fRadius * 0.8f;
				TFLOAT fVelocityScale  = fSpeed * fRadius * 1.1f;
				TFLOAT fParticleRadius = fRadius * 0.75f;

				Toshi::TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

				for ( TINT i = 0; i < 6; i++ )
				{
					// A random direction around the spin axis, pushed a little along it
					TVector4 vOffset;
					TVector4::RotateArbitaryVector( vOffset, vPerpendicular, vSpin, rRandom.GetFloat() * 6.2831855f );

					TVector4 vJitter;
					vJitter.Multiply( vSpin, rRandom.GetFloat() * 0.2f - 0.1f );
					vOffset.x += vJitter.x;
					vOffset.y += vJitter.y;
					vOffset.z += vJitter.z;

					TVector4 vPosition;
					vPosition.Multiply( vOffset, fPositionScale );
					vPosition.x += vBase.x;
					vPosition.y += vBase.y;
					vPosition.z += vBase.z;

					// The velocity: the offset turned a quarter around the spin, scaled, plus 0.9 of the body's velocity
					TVector4 vTurned;
					TVector4::RotateArbitaryVector( vTurned, vOffset, vSpin, 1.5707964f );

					TVector4 vDrift;
					vDrift.Multiply( vLinear, 0.9f );

					TVector4 vVelocity;
					vVelocity.Multiply( vTurned, fVelocityScale );
					vVelocity.x += vDrift.x;
					vVelocity.y += vDrift.y;
					vVelocity.z += vDrift.z;

					// The first two particles are decals on the ground: they get the splat material and the ink cloud of the colour
					TINT iDecal   = 0;
					TINT iParam38 = 0;
					if ( i < 2 )
					{
						TUINT32 uiIndex = m_pColourComponent->GetCurrentColour();
						if ( ( uiIndex & 0x10 ) != 0 )
							uiIndex = 0x10;

						iDecal   = TINT( TREINTERPRETCAST( TUINTPTR, m_pMaterialTestSplat ) );
						iParam38 = TINT( TREINTERPRETCAST( TUINTPTR, ms_apInkCloudSmallEffects[ uiIndex ] ) );
					}

					APaintParticleSystem::s_Instance->AddParticle(
					    vPosition,
					    vVelocity,
					    m_pColourComponent->m_uiFadeColour2,
					    fParticleRadius,
					    iDecal,
					    iParam38,
					    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomA ) ),
					    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomB ) )
					);
				}

				// The next spray: 0.01 to 0.02 of the radius
				rfSprayTimer = ( rRandom.GetFloat() * 0.01f + 0.01f ) * fRadius;
			}
		}
	}

	UpdateSoftCollision();

	m_oMotion.m_uiPrevContactFlags = m_oMotion.m_uiContactFlags;
	m_uiPrevButtons                = m_uiButtons;
	m_oMotion.m_auiUnk280[ 8 ]     = m_oMotion.m_auiUnk280[ 5 ]; // 0x320 = 0x314
	m_oMotion.m_auiUnk280[ 9 ]     = m_oMotion.m_auiUnk280[ 6 ]; // 0x324 = 0x318
	m_uiFlags                      = ( m_uiFlags & 0x7FFF7FF3 ) | 0x4000;
	m_oMotion.m_auiUnk280[ 10 ]    = m_oMotion.m_auiUnk280[ 7 ]; // 0x328 = 0x31C
	m_vPhysicsPosition             = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// $deBlob: FUNCTION 0074beb0
TBOOL ABlobEntity::UpdateBrake( TFLOAT a_fTimeStep )
{
	TASSERT( STATE_ROLL == m_eState );

	if ( ( ( m_uiButtons >> 5 ) & 1 ) != 0 && ( m_uiFlags & 0x10200 ) == 0 && m_iLeechbotsAttached == 0 &&
	     ( ( m_oMotion.m_uiContactFlags & 8 ) == 0 || ( GetContactFlags( TUINT8( m_oMotion.m_uiContactCountAndGround >> 8 ) ) & 0x3004000 ) == 0 ) &&
	     !HasWallContactFlag( 0x800000 ) )
	{
		// No brake on the wall of a car (unless Blob is also on the same car)
		if ( ( m_oMotion.m_uiContactFlags & 4 ) != 0 )
		{
			TPhysicsObjectContainer*        pWall = m_oMotion.m_aContacts[ m_oMotion.m_uiWallContact ].m_pBody;
			ACollisionInstance2::OwnerInfo* pInfo = pWall ? TSTATICCAST( ACollisionInstance2::OwnerInfo, pWall->m_pUserData ) : TNULL;

#ifdef DEBLOB_COMMUNITY_PATCH
			// NOTE: retail reads the owner info without checking the body or its user data
			if ( pInfo != TNULL && pInfo->m_eType == 2 )
#else
			if ( pInfo->m_eType == 2 )
#endif
			{
				ABaseEntity* pOwner = TSTATICCAST( ABaseEntity, pInfo->m_pOwner );

				if ( pOwner->GetClass() == &TGetClass( ACar ) )
				{
					switch ( TSTATICCAST( ACar, pOwner )->GetCarType() )
					{
						case 0:
						case 4:
						case 5:
						case 7:
							if ( ( m_oMotion.m_uiContactFlags & 8 ) == 0 || m_oMotion.m_aContacts[ TUINT8( m_oMotion.m_uiContactCountAndGround >> 8 ) ].m_pBody != pWall )
							{
								m_uiFlags &= 0xFFFBDFFF;
								return TFALSE;
							}
							break;
					}
				}
			}
		}

		const TVector4& rcPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

		// Not on a moving object: hold the ball where it stopped
		if ( m_oMotion.m_vUnk220.MagnitudeSq() <= 0.0f )
		{
			if ( ( m_uiFlags & 0x40000 ) == 0 )
			{
				m_uiFlags |= 0x40000;
				m_vHoldPosition = rcPosition;
			}

			SetPhysicsPosition( m_vHoldPosition );
			TPhysicsManager::StopBody( m_pBody );
		}

		m_uiFlags |= 0x2000;

		const Properties& rProperties = ms_pBlobProperties[ m_iSize ];

		TVector4 vOffset;
		vOffset.Multiply( m_oMotion.m_vContactNormal, rProperties.m_fJellyOffset * -0.5f * rProperties.m_fRadius );

		if ( ( m_uiFlags >> 22 ) & 1 )
		{
			m_oJelly.m_vRest = rcPosition;
		}
		else
		{
			m_oJelly.m_vRest.x = rcPosition.x + vOffset.x;
			m_oJelly.m_vRest.y = rcPosition.y + vOffset.y;
			m_oJelly.m_vRest.z = rcPosition.z + vOffset.z;
			m_oJelly.m_vRest.w = vOffset.w;
		}

		return TTRUE;
	}

	m_uiFlags &= 0xFFFBDFFF;
	return TFALSE;
}

// $deBlob: FUNCTION 00751520
void ABlobMotionData::ResetContacts()
{
	m_uiContactFlags &= 0xFFCF;
	m_vUnk240        = TVector4::VEC_ZERO;
	m_auiUnk280[ 0 ] = 0;
	m_auiUnk280[ 1 ] = 0;
	m_vUnk250        = TVector4::VEC_ZERO;
	m_auiUnk280[ 2 ] = 0;
	m_auiUnk280[ 3 ] = 0;
	m_auiUnk280[ 4 ] = 0;
}

// $deBlob: FUNCTION 007509d0
void ABlobEntity::StopRollEffects()
{
	m_pSoundComponent->StopHurtLoops();

	if ( m_auiSounds[ 9 ] != 0 ) // 0xAFC
		TREINTERPRETCAST( TParticleEffectInstance*, m_auiSounds[ 9 ] )->SetPaused( TTRUE );

	if ( m_auiSounds[ 8 ] != 0 ) // 0xAF8
		TREINTERPRETCAST( TParticleEffectInstance*, m_auiSounds[ 8 ] )->SetPaused( TTRUE );
}

// $deBlob: FUNCTION 00750a10
void ABlobEntity::UpdateSurfaceEffect()
{
	TParticleEffectInstance*& rpEffect = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 11 ] ); // 0xB04

	if ( rpEffect == TNULL )
	{
		if ( m_auiSounds[ 3 ] != 0 && ( HasGroundContactFlag( 0x2000000 ) || HasWallContactFlag( 0x2000000 ) ) ) // 0xAE4
		{
			rpEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( TREINTERPRETCAST( TParticleEffect*, m_auiSounds[ 3 ] ) );

			if ( rpEffect != TNULL )
			{
				rpEffect->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
				rpEffect->GetTransform().m_f42 = GetBoundingSphereRadius() + rpEffect->GetTransform().m_f42;
				rpEffect->SetFinishedCallback( OnBlobEffectFinished, &m_auiSounds[ 0 ] );
			}
		}

		return;
	}

	if ( !HasGroundContactFlag( 0x2000000 ) && !HasWallContactFlag( 0x2000000 ) )
	{
		rpEffect->SetPaused( TTRUE );
		return;
	}

	rpEffect->SetPaused( TFALSE );
	rpEffect->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
	rpEffect->GetTransform().m_f42 = GetBoundingSphereRadius() + rpEffect->GetTransform().m_f42;
}

// $deBlob: FUNCTION 007507c0
void ABlobEntity::UpdateRollEffects()
{
	TParticleEffectInstance*& rpPaintDrip = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 8 ] ); // 0xAF8
	TParticleEffectInstance*& rpClear     = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 9 ] ); // 0xAFC

	if ( ( m_pColourComponent->GetCurrentColour() == 0 && AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 && !APlayerManager::GetSingleton()->IsNoBlobPainted() ) ||
	     ( ( m_Flags >> 2 ) & 1 ) == 0 )
	{
		StopRollEffects();
		return;
	}

	if ( m_pColourComponent->GetCurrentColour() != 0 )
	{
		if ( rpClear != TNULL )
			rpClear->SetPaused( TTRUE );

		if ( rpPaintDrip == TNULL )
		{
			rpPaintDrip = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( TREINTERPRETCAST( TParticleEffect*, m_auiSounds[ 1 ] ) ); // 0xADC

			if ( rpPaintDrip == TNULL )
				return;

			rpPaintDrip->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
			rpPaintDrip->SetFinishedCallback( OnBlobEffectFinished, &m_auiSounds[ 0 ] );
		}
		else
		{
			rpPaintDrip->SetPaused( TFALSE );
		}

		TUINT32 uiColour = m_pColourComponent->GetCurrentColour();
		if ( uiColour & 0x10 ) uiColour = 0x10;

		AParticleEffectHelper::SetColour( TREINTERPRETCAST( TParticleEffect*, m_auiSounds[ 1 ] ), ABlobColourComponent::ms_auiColours[ uiColour ], TNULL );
		return;
	}

	if ( rpPaintDrip != TNULL )
		rpPaintDrip->SetPaused( TTRUE );

	if ( rpClear == TNULL )
	{
		rpClear = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pSpikeHaemorrhageClearEffect );

		if ( rpClear != TNULL )
		{
			rpClear->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
			rpClear->SetFinishedCallback( OnBlobEffectFinished, &m_auiSounds[ 0 ] );
		}
	}
	else
	{
		rpClear->SetPaused( TFALSE );
	}
}

// The start of the electrocution by an electric hazard: the sparks, the lost paint flying to one of the hazard's
// points (or the death when it was the last life), the electrocution effect, sound and HUD, and the bounce off
// $deBlob: FUNCTION 00745b10
void ABlobEntity::StartElectrocution( APropHazard* a_pHazard )
{
	if ( m_uiFlags & 0x2000000 )
		return;

	TParticleEffectInstance* pSparks = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pElectricTrapSparksEffect );

	if ( pSparks != TNULL )
	{
		TMatrix44 mSparks = TMatrix44::IDENTITY;
		TASSERT( m_oMotion.m_aContacts[ 0 ].m_vPosition.w == 1.0f );
		mSparks.SetBasisVector4( BASISVECTOR_TRANSLATION, m_oMotion.m_aContacts[ 0 ].m_vPosition );
		pSparks->GetTransform() = mSparks;
	}

	APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetBlobDataOf( this );

	if ( TINT( pBlobData->m_uiLives ) <= 1 )
	{
		// The last one: dies
		pBlobData->LoseLife( TFALSE );
		OnOutOfPoints( TFALSE );
		return;
	}

	m_uiFlags        = ( m_uiFlags & ~0x20000 ) | 0x2000000;
	m_auiUnk580[ 1 ] = *TREINTERPRETCAST( TUINT32*, &ms_pBlobCommonProperties->m_afUnkE0[ 0 ] ); // 0x584 the time of the electrocution

	ASpawnPointPool::SpawnPoint* pPoint = TSTATICCAST( ASpawnPointPool::SpawnPoint, a_pHazard->GetSpawnPoint() );

	if ( pPoint != TNULL )
	{
		// The lost life (a life pickup) flies to the point
		const TVector4& rcPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
		APickup*        pPickup    = APickupManager::CreatePickup( 4, rcPosition, -1.0f, ms_pBlobCommonProperties->m_afUnkE0[ 0 ] );

		if ( pPickup == TNULL )
		{
			pPoint->m_pOwner->ReleaseSpawnPoint( pPoint );
		}
		else
		{
			pPickup->Fly( rcPosition, pPoint->m_vPosition, -1.0f, 0 );
			pPickup->SetCallback( ASpawnPointPool::ReleaseSpawnPointCallback, pPoint );

			APickupLife::ListNode& rLifeNode = pPickup->CastTo<APickupLife>()->GetLifeNode();
			TASSERT( !rLifeNode.IsLinked() );
			rLifeNode.InsertBefore( m_oList438.End() );

			pBlobData->LoseLife( TTRUE );
			TREINTERPRETCAST( ABlobLifeHud*, pBlobData->m_uiLifeHud )->SetLifePending( TTRUE );
		}
	}

	m_pZJump->Cancel();
	m_pSoundComponent->StartElectrocution();

	TParticleEffectInstance*& rpElectrocution = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 6 ] ); // 0xAF0

	if ( rpElectrocution == TNULL )
		rpElectrocution = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( TREINTERPRETCAST( TParticleEffect*, m_auiSounds[ 0 ] ) ); // 0xAD8

	if ( rpElectrocution != TNULL )
	{
		TASSERT( m_oPhysicsMatrix.m_f44 == 1.0f );
		rpElectrocution->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
	}

	if ( m_pSizeHud != TNULL && ( m_pColourComponent->GetCurrentColour() != 0 || AGameStatus::GetSingleton()->GetMultiPlayerGameType() != 1 || APlayerManager::GetSingleton()->IsNoBlobPainted() ) )
		m_pSizeHud->GetVisual()->PlayAnimation( L"Zapped", L"EndZapped" );

	BounceOffContact();
}

// $deBlob: FUNCTION 00745e10
void ABlobEntity::EndElectrocution()
{
	if ( ( m_uiFlags & 0x2000000 ) == 0 )
		return;

	TParticleEffectInstance*& rpElectrocution = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 6 ] ); // 0xAF0

	if ( rpElectrocution != TNULL )
	{
		rpElectrocution->StopInstant();
		rpElectrocution = TNULL;
	}

	m_pSoundComponent->StopElectrocution();
	m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_SHOCK_RECOVER );

	if ( -1 < m_iCamShakeID )
	{
		ACameraManager::GetSingleton()->GetShakeFunction().StopShake( m_iCamShakeID );
		m_iCamShakeID = -1;
	}

	if ( m_pSizeHud != TNULL && ( m_pColourComponent->GetCurrentColour() != 0 || AGameStatus::GetSingleton()->GetMultiPlayerGameType() != 1 || APlayerManager::GetSingleton()->IsNoBlobPainted() ) )
		m_pSizeHud->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() < 0 )
	{
		APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetBlobDataOf( this );

		// No life pending anymore: nothing in m_oList438
		if ( !m_oList438.IsLinked() )
		{
			ABlobLifeHud* pLifeHud = TREINTERPRETCAST( ABlobLifeHud*, pBlobData->m_uiLifeHud ); // BlobData 0x24

			if ( pLifeHud->IsLifePending() )
				pLifeHud->SetLifePending( TFALSE );
		}
	}

	m_uiFlags &= ~0x2000000;
}

// $deBlob: FUNCTION 0074e7a0
void ABlobEntity::UpdateHazards()
{
	if ( m_oMotion.m_auiUnk280[ 5 ] == 0 || ( m_uiFlags & 8 ) == 0 || m_eState == STATE_DEAD || m_eState == STATE_FAIL || m_eState == STATE_VICTORY || m_eState == STATE_LOSTLASTLIFE ||
	     IsLocked() || IsEvilBlob() || ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) != 0 )
		return;

	TPhysicsObjectContainer* pContactBody = TREINTERPRETCAST( TPhysicsObjectContainer*, m_oMotion.m_auiUnk280[ 5 ] ); // 0x314

	// A hurting surface (material 0x100000) touched below (0x318) or beside (0x31C) the ball in this step: a paint point
	if ( ( ( m_oMotion.m_auiUnk280[ 9 ] ^ m_oMotion.m_auiUnk280[ 6 ] ) & m_oMotion.m_auiUnk280[ 6 ] & 0x100000 ) != 0 ||
	     ( ( m_oMotion.m_auiUnk280[ 10 ] ^ m_oMotion.m_auiUnk280[ 7 ] ) & m_oMotion.m_auiUnk280[ 7 ] & 0x100000 ) != 0 )
	{
		if ( TUINT32( m_iPaintPoints ) < 1 && !IsLocked() )
			OnOutOfPoints( TFALSE );

		RemovePaintPoints( 1, TTRUE );
		*TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 2 ] ) = ms_pBlobCommonProperties->m_fHurtTime; // 0x56C
		UpdateRollEffects();

		TVector4 vContactVelocity;
		TVector4 vVelocity;
		TPhysicsManager::GetSingleton()->GetLinearVelocity( pContactBody, vContactVelocity );
		TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );

		TFLOAT fRelX = vVelocity.x - vContactVelocity.x;
		TFLOAT fRelY = vVelocity.y - vContactVelocity.y;
		TFLOAT fRelZ = vVelocity.z - vContactVelocity.z;

		if ( m_pAnimationComponent->m_iState != ABlobAnimationComponent::STATE_PAIN )
		{
			if ( ( m_Flags >> 2 ) & 1 )
				m_pSoundComponent->StartHurtLoops();

			// Hit hard (falling into it) or the same body as in the last step
			if ( ( m_eState == STATE_BALLISTIC && m_oMotion.m_vContactNormal.y * fRelY + m_oMotion.m_vContactNormal.x * fRelX + m_oMotion.m_vContactNormal.z * fRelZ <= -ms_pBlobCommonProperties->m_fUnk120 ) ||
			     m_oMotion.m_auiUnk280[ 8 ] == m_oMotion.m_auiUnk280[ 5 ] )
			{
				m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_PAIN );
				m_pSoundComponent->PlayHurtHard();
			}
			else
			{
				m_pSoundComponent->PlayHurt();
			}
		}
	}

	// An electric hazard
	if ( HasGroundContactFlag( 0x8000 ) || HasWallContactFlag( 0x8000 ) )
	{
		InkBounce();

		if ( ( TUINT8( m_uiFlags >> 25 ) & 1 ) == 0 )
		{
			if ( m_iLeechbotsAttached != 0 )
			{
				BounceOffContact();
				AEnemyGrappler::ReleasePlayer( this );
				m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_PAIN );
			}
			else if ( !IsLocked() )
			{
				ACollisionInstance2::OwnerInfo* pOwnerInfo = pContactBody ? TSTATICCAST( ACollisionInstance2::OwnerInfo, pContactBody->m_pUserData ) : TNULL;

				if ( pOwnerInfo->m_eType == 2 && TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner )->IsA( &TGetClass( APropHazard ) ) )
					StartElectrocution( TSTATICCAST( APropHazard, pOwnerInfo->m_pOwner ) );
			}
		}
	}

	// A hot plate
	if ( ( HasGroundContactFlag( 0x10000 ) || HasWallContactFlag( 0x10000 ) ) && m_eState != STATE_FLOAT )
	{
		if ( m_iLeechbotsAttached != 0 )
		{
			AEnemyGrappler::ReleasePlayer( this );
			BounceOffContact();
			return;
		}

		ACollisionInstance2::OwnerInfo* pOwnerInfo = pContactBody ? TSTATICCAST( ACollisionInstance2::OwnerInfo, pContactBody->m_pUserData ) : TNULL;

		if ( pOwnerInfo->m_eType == 2 && TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner )->IsA( &TGetClass( APropHazard ) ) )
		{
			APropHazard* pHazard = TDYNAMICCAST( APropHazard, TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner ) );
			TASSERT( pHazard->GetHazardType() == APropHazard::TYPE_HOTPLATE );

			// Bounced off the plate: the normal is the plate's down axis
			m_oMotion.m_vContactNormal.x = -pHazard->GetMatrix().m_f21;
			m_oMotion.m_vContactNormal.y = -pHazard->GetMatrix().m_f22;
			m_oMotion.m_vContactNormal.z = -pHazard->GetMatrix().m_f23;
			m_oMotion.m_vContactNormal.w = 1.0f;
			m_pHotPlate                  = pHazard;
			SetState( STATE_HOTPLATE );
			m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_HOTPLATE );
		}
	}
}

// A vertex of the threads' system shader meshes (0x18 bytes)
struct ThreadVertex
{
	TVector3 vPosition; // 0x00
	TUINT32  uiColour;  // 0x0C
	TVector2 vUV;       // 0x10
};

TSTATICASSERT( sizeof( ThreadVertex ) == 0x18 );

// $deBlob: FUNCTION 0073ca90
void ABlobEntity::ThreadList::Remove( Thread* a_pThread )
{
	TASSERT( IsInList( a_pThread ) );

	if ( a_pThread->m_pPrev != TNULL )
		a_pThread->m_pPrev->m_pNext = a_pThread->m_pNext;

	if ( a_pThread->m_pNext != TNULL )
		a_pThread->m_pNext->m_pPrev = a_pThread->m_pPrev;

	if ( m_pHead == a_pThread )
	{
		m_pHead = a_pThread->m_pNext;
		TASSERT( TNULL == a_pThread->m_pPrev );
	}

	// NOTE: retail doesn't move m_pTail off a_pThread when it's the tail (PopBack does); the stale tail only makes
	// PopBack take another thread than the oldest one
	m_iCount--;
	a_pThread->m_pPrev = TNULL;
	a_pThread->m_pNext = TNULL;

	TASSERT( !IsInList( a_pThread ) );
}

// $deBlob: FUNCTION 0073cb80
ABlobEntity::Thread* ABlobEntity::ThreadList::PopBack()
{
	Thread* pThread = m_pTail;

	if ( pThread == TNULL )
		return TNULL;

	if ( pThread->m_pPrev != TNULL )
		pThread->m_pPrev->m_pNext = pThread->m_pNext;

	if ( pThread->m_pNext != TNULL )
		pThread->m_pNext->m_pPrev = pThread->m_pPrev;

	if ( m_pHead == pThread )
	{
		m_pHead = pThread->m_pNext;
		TASSERT( TNULL == pThread->m_pPrev );
	}

	if ( m_pTail == pThread )
		m_pTail = pThread->m_pPrev;

	m_iCount--;
	pThread->m_pPrev = TNULL;
	pThread->m_pNext = TNULL;

	TASSERT( !IsInList( pThread ) );
	return pThread;
}

// $deBlob: FUNCTION 0073c9f0
void ABlobEntity::ThreadList::PushFront( Thread* a_pThread )
{
	TASSERT( !IsInList( a_pThread ) );

	a_pThread->m_pNext = m_pHead;

	if ( m_pHead != TNULL )
		m_pHead->m_pPrev = a_pThread;

	m_pHead = a_pThread;

	if ( m_pTail == TNULL )
		m_pTail = a_pThread;

	m_iCount++;

	TASSERT( IsInList( a_pThread ) );
}

// $deBlob: FUNCTION 007502f0
void ABlobEntity::AddThread( const TVector4& a_rPosition, const TVector4& a_rNormal )
{
	// A free thread, else the oldest active one
	Thread* pThread = m_oFreeThreads.PopBack();

	if ( pThread == TNULL )
	{
		pThread = m_oActiveThreads.PopBack();

		if ( pThread == TNULL )
			return;
	}

	// A random direction within 30 degrees of the normal
	TRandom& rRandom = ARandom::GetSingleton()->GetRandom();
	TFLOAT   fAngle  = rRandom.GetFloatMinMax( 0.0f, TMath::PI );
	TFLOAT   fSpread = rRandom.GetFloatMinMax( 0.0f, 0.52359879f ); // PI / 6

	TFLOAT fSinAngle, fCosAngle;
	TMath::SinCos( fAngle, fSinAngle, fCosAngle );
	TFLOAT fSinSpread, fCosSpread;
	TMath::SinCos( fSpread, fSinSpread, fCosSpread );

	TQuaternion oQuat;
	oQuat.SetVectorDirection( TVector3( a_rNormal.x, a_rNormal.y, a_rNormal.z ), TVector3( 0.0f, -1.0f, 0.0f ) );
	TMatrix44 mRotation;
	mRotation.SetFromQuaternion( oQuat );

	pThread->m_vDirection.Set( fSinSpread * fCosAngle, fCosSpread, fSinSpread * fSinAngle, 1.0f );
	TMatrix44::TransformVector( pThread->m_vDirection, mRotation, pThread->m_vDirection );

	// Into the body's frame
	// NOTE: retail reads the matrix of a null body too
	TMatrix44 mBody = *( ( m_pBody != TNULL ) ? m_pBody->m_pMatrix : TNULL );
	mBody.m_f41     = TVector3::VEC_ZERO.x;
	mBody.m_f42     = TVector3::VEC_ZERO.y;
	mBody.m_f43     = TVector3::VEC_ZERO.z;
	mBody.Invert( mBody );
	TMatrix44::TransformVector( pThread->m_vDirection, mBody, pThread->m_vDirection );

	// The end left on the surface: around the contact, flattened against the wall the ball sticks to
	TFLOAT fScale = 1.0f;

	if ( m_oMotion.m_uiContactFlags & 0x40 )
	{
		TFLOAT fDot = TMath::Abs( m_vUnk614.y * a_rNormal.y + m_vUnk614.x * a_rNormal.x + m_vUnk614.z * a_rNormal.z );
		fScale      = m_fSquashCompensation * fDot + ( 1.0f - fDot );
	}

	TFLOAT fRadius = ms_pBlobProperties[ m_iSize ].m_fRadius * fScale * fSinSpread;
	pThread->m_vAnchor.Set( fRadius * fCosAngle, 0.0f, fRadius * fSinAngle, 1.0f );
	TMatrix44::TransformVector( pThread->m_vAnchor, mRotation, pThread->m_vAnchor );
	pThread->m_vAnchor.x += a_rPosition.x;
	pThread->m_vAnchor.y += a_rPosition.y;
	pThread->m_vAnchor.z += a_rPosition.z;
	pThread->m_vEnd = pThread->m_vAnchor;

	// The ball's colour at its end (faded with the ball), the paint's at the surface
	TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
		uiColour = ABlobColourComponent::COLOUR_BLACK;

	TUINT32 uiAlpha           = TUINT8( TINT( m_pAnimationComponent->m_pBallBodyModelI->GetModelInstance()->GetFade() * 192.0f ) );
	pThread->m_uiEndColour    = ( ABlobColourMaps::ms_pBodyColourMap[ uiColour ] & 0x00FFFFFF ) | ( uiAlpha << 24 );
	pThread->m_uiAnchorColour = ( m_pColourComponent->GetFadeColour2() & 0x00FFFFFF ) | 0xC0000000;

	m_oActiveThreads.PushFront( pThread );
}

// The text of the shake prompt's "Message" on the PC
static TWCHAR s_wszLandmarkMessage[ 0x100 ];

// $deBlob: FUNCTION 00745f70
void ABlobEntity::EnterLandmark( ALandmarkInstance* a_pLandmark, TINT a_iTrigger )
{
	// The shake prompt: the Wii's drum or the PC's swing
	ABlobHud*    pHud    = ABlobHud::GetSingleton();
	TXUIVisual*  pPrompt = TSTATICCAST( TXUIControl, pHud->FindChildXUIElement( L"SwingRemote" ) )->GetVisual();
	TXUIElement* pDrum   = pPrompt->FindChildXUIElement( L"Drum" );
	pDrum->SetVisible( TInputDevice::GetLastGamepadType() == TInputDevice::Gamepad_Wii );
	TXUIElement* pSwingPC = pPrompt->FindChildXUIElement( L"swing_pc" );
	pSwingPC->SetVisible( TInputDevice::GetLastGamepadType() != TInputDevice::Gamepad_Wii );
	pPrompt->FindChildXUIElement( L"raydianswing" )->SetVisible( TFALSE );
	TXUIControl* pMessage = TSTATICCAST( TXUIControl, pPrompt->FindChildXUIElement( L"Message" ) );

	if ( TInputDevice::GetLastGamepadType() == TInputDevice::Gamepad_Wii )
	{
		pMessage->SetText( T2Locale::GetSingleton()->GetString( 812 ), -1 );
	}
	else
	{
		const TWCHAR* wszAction = T2Locale::GetSingleton()->GetString( 518 );
		// NOTE: retail calls the CRT's swprintf
		TStringManager::String16Format( s_wszLandmarkMessage, 0x100, L"%ls %ls", T2Locale::GetSingleton()->GetString( 876 ), wszAction );
		pMessage->SetText( s_wszLandmarkMessage, -1 );
	}

	pPrompt->PlayAnimation( L"Display", L"EndDisplay" );

	TASSERT( STATE_LMARK != m_eState );
	TASSERT( m_iCurrentLandmarkTrigger < 0 );

	m_pLandmark               = a_pLandmark;
	m_iCurrentLandmarkTrigger = a_iTrigger;
	SetState( STATE_LMARK );

	// In front of the trigger, as far from it as the ball is
	TMatrix44 oTriggerMatrix;
	a_pLandmark->GetTriggerMatrix( a_iTrigger, oTriggerMatrix );
	const TVector4& rTriggerPosition = oTriggerMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	TVector4 vPosition;
	vPosition.Multiply( oTriggerMatrix.AsBasisVector4( BASISVECTOR_RIGHT ), TVector4::Distance( rTriggerPosition, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) ) );
	vPosition.x += rTriggerPosition.x;
	vPosition.y += rTriggerPosition.y;
	vPosition.z += rTriggerPosition.z;
	vPosition.w = 1.0f;
	SetPhysicsPosition( vPosition );

	if ( GetZJumpState() == ABlobZJumpComponent::STATE_LMARKFLIGHT )
		m_pZJump->EndAttack( TTRUE );

	m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_LMARK_IN );
	m_pSoundComponent->StartShaker();
	m_auiUnk564[ 3 ] = 0; // 0x570
	ABuildingManager::GetSingleton()->SetBlobOnLandmark( TTRUE );

	// The trigger's colour, the ball's for the ones of any colour
	TUINT32 uiColour = a_pLandmark->GetTriggerColour( a_iTrigger );

	if ( uiColour == 0 )
	{
		uiColour = m_pColourComponent->GetCurrentColour();

		if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
			uiColour = ABlobColourComponent::COLOUR_BLACK;
	}

	TUINT32 uiARGB = ABlobColourComponent::ms_auiColours[ uiColour ];

	if ( m_pSizeHud != TNULL )
		m_pSizeHud->GrowLandmarkMeter( uiARGB, a_pLandmark->GetTriggerPaintLeft( a_iTrigger ), a_pLandmark->m_iPaintPerTrigger );

	// NOTE: retail passes uiARGB to ShowSplats, which doesn't use it
	pHud->ShowSplats();
}

// $deBlob: FUNCTION 007462a0
void ABlobEntity::ExitLandmark( TBOOL a_bCompleted )
{
	if ( !( m_pLandmark->m_uiFlags & ALandmarkInstance::Flags_Completed ) )
		TSTATICCAST( TXUIControl, ABlobHud::GetSingleton()->FindChildXUIElement( L"SwingRemote" ) )->GetVisual()->PlayAnimation( L"Hide", L"EndHide" );

	TMatrix44 oTriggerMatrix;
	m_pLandmark->GetTriggerMatrix( m_iCurrentLandmarkTrigger, oTriggerMatrix );
	m_pLandmark->DisableTriggerBodies();

	// Jumps from the trigger's extents (from a_vStart to a_vTarget), else is put in front of the trigger
	TBOOL    bJump = TTRUE;
	TVector4 vStart;
	TVector4 vTarget;

	if ( !m_pLandmark->GetTriggerExtents( m_iCurrentLandmarkTrigger, vStart, vTarget ) )
	{
		const TVector4& rRight = oTriggerMatrix.AsBasisVector4( BASISVECTOR_RIGHT );
		const TVector4& rTrans = oTriggerMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

		vStart.Multiply( TVector4( rRight.x, rRight.y, rRight.z, 1.0f ), ms_pBlobProperties[ m_iSize ].m_fRadius + 2.0f );
		bJump    = TFALSE;
		vStart.x = rTrans.x + vStart.x;
		vStart.y = rTrans.y + vStart.y;
		vStart.z = rTrans.z + vStart.z;
	}

	SetPhysicsPosition( vStart );
	m_pSoundComponent->StopShaker();

	if ( !( m_pLandmark->m_uiFlags & ALandmarkInstance::Flags_Completed ) && m_pSizeHud != TNULL )
		m_pSizeHud->SetExitLandmarkPending();

	ABlobHud::GetSingleton()->HideSplats();
	AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;
	ABuildingManager::GetSingleton()->SetBlobOnLandmark( TFALSE );
	m_pLandmark               = TNULL;
	m_iCurrentLandmarkTrigger = -1;

	TASSERT( STATE_LMARK == m_eState );
	SetState( STATE_BALLISTIC );

	if ( bJump )
	{
		TFLOAT fAngle = TMath::Abs( vTarget.y - vStart.y );
		fAngle        = TMath::ATan2( fAngle, TVector4::DistanceXZ( vTarget, vStart ) );
		fAngle        = TMath::LERP( fAngle, TMath::HALF_PI, 0.2f );

		TVector4 vVelocity;
		CalculateJumpVelocity( fAngle, vStart, vTarget, 80.0f, vVelocity );
		TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );
		TPhysicsManager::SetAngularVelocity( m_pBody, TVector4::VEC_ZERO ); // TVector4::X0Y0Z0W1

		*TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 0 ] ) = 0.1f; // 0x580
		m_uiFlags                                       = ( m_uiFlags & ~0x40000 ) | 0x1010000;
		m_vJumpStart                                    = vStart;
	}

	m_pAnimationComponent->SetState( a_bCompleted ? 14 : 4 );
	m_pAnimationComponent->SetVisible( TTRUE );
}

// $deBlob: FUNCTION 00746570
void ABlobEntity::HideLandmarkPrompt()
{
	TSTATICCAST( TXUIControl, ABlobHud::GetSingleton()->FindChildXUIElement( L"SwingRemote" ) )->GetVisual()->PlayAnimation( L"Hide", L"EndHide" );

	if ( m_pSizeHud != TNULL )
		m_pSizeHud->ExitLandmark();

	ABlobHud::GetSingleton()->HideSplats();
}

// $deBlob: FUNCTION 007465b0
void ABlobEntity::EnterBoss( ABoss* a_pBoss )
{
	TASSERT( STATE_BOSS != m_eState );

	m_pBoss = a_pBoss;
	SetState( STATE_BOSS );
	m_pZJump->Deactivate();
	m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_LMARK_IN );
	m_auiUnk564[ 3 ] = 0; // 0x570

	TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
		uiColour = ABlobColourComponent::COLOUR_BLACK;

	TUINT32 uiARGB = ABlobColourComponent::ms_auiColours[ uiColour ];

	if ( m_pSizeHud != TNULL )
		m_pSizeHud->GrowLandmarkMeter( uiARGB, m_pBoss->GetLife(), m_pBoss->GetMaxLife() );

	m_pSoundComponent->StartShaker();
	// NOTE: retail passes uiARGB to ShowSplats, which doesn't use it (RET 4)
	ABlobHud::GetSingleton()->ShowSplats();
}

// $deBlob: FUNCTION 00746690
void ABlobEntity::ExitBoss( TBOOL a_bKilled )
{
	TVector4 vPosition;
	m_pBoss->GetEyeFrontPosition( vPosition, ms_pBlobProperties[ m_iSize ].m_fRadius );
	SetPhysicsPosition( vPosition );

	AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;
	m_pBoss->OnHit( a_bKilled );
	m_pBoss = TNULL;

	TASSERT( STATE_BOSS == m_eState );
	SetState( STATE_BALLISTIC );

	// Pushed off along the facing
	TVector4 vForce = m_vFacing;
	vForce.Multiply( ( m_pBody != TNULL ? m_pBody->m_fMass : 1.0f ) * 2000.0f );
	TPhysicsManager::GetSingleton()->ApplyPointForce( m_pBody, GetBodyPosition(), vForce );

	m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_LMARK_OUT );
	m_pAnimationComponent->SetVisible( TTRUE );

	if ( m_pSizeHud != TNULL )
		m_pSizeHud->SetExitLandmarkPending();

	m_pSoundComponent->StopShaker();
	ABlobHud::GetSingleton()->HideSplats();
}

// $deBlob: FUNCTION 007467d0
void ABlobEntity::EnterCannon( const TMatrix44& a_rMatrix, TUINT a_uiSize )
{
	TASSERT( STATE_CANNON == m_eState );
	TASSERT( !( m_pBody != TNULL && m_pBody->IsInSimulation() ) );

	// Shown at size a_uiSize while in the cannon (the real size comes back at the launch)
	if ( a_uiSize < 11 )
	{
		m_uiFlags |= 0x8000000;
		m_iSize    = a_uiSize;
		m_iOldSize = a_uiSize;
	}

	SetPhysicsPosition( a_rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
	m_pAnimationComponent->m_pCharModelI->SetTransform( a_rMatrix );
	m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_INSIDE_CANNON );
}

// $deBlob: FUNCTION 007468b0
void ABlobEntity::SetCannonTransform( const TMatrix44& a_rMatrix )
{
	TASSERT( STATE_CANNON == m_eState );
	TASSERT( !( m_pBody != TNULL && m_pBody->IsInSimulation() ) );

	SetPhysicsPosition( a_rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
	m_oJelly.m_vRest     = a_rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	m_oJelly.m_vPosition = a_rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	m_pAnimationComponent->m_pCharModelI->SetTransform( a_rMatrix );
}

// $deBlob: FUNCTION 00746980
void ABlobEntity::LaunchFromCannon( const TVector4& a_rPosition, const TVector4& a_rVelocity )
{
	TASSERT( STATE_CANNON == m_eState );

	if ( m_uiFlags & 0x8000000 )
	{
		m_uiFlags &= ~0x8000000;
		SetPaintPoints( m_iPaintPoints );
	}

	SetPhysicsPosition( a_rPosition );
	TASSERT( STATE_DEAD != m_eState );
	TPhysicsManager::GetSingleton()->AddToSimulation( m_pBody, TTRUE );
	TPhysicsManager::SetLinearVelocity( m_pBody, a_rVelocity );

	m_uiFlags &= ~0x4000000;
	m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_EXIT_CANNON );
	*TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 0 ] ) = 0.1f; // 0x580
	m_uiFlags                                       = ( m_uiFlags & ~0x40000 ) | 0x1010000;
	ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 3, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 1.0f, -1 );

	// A ring of paint out of the barrel
	if ( m_pColourComponent->GetCurrentColour() != 0 )
	{
		TRandom&              rRandom         = ARandom::GetSingleton()->GetRandom();
		APaintParticleSystem* pPaintParticles = APaintParticleSystem::s_Instance;
		TVector4              vDirection      = TVector4( 0.0f, 0.0f, 0.0f, 1.0f ); // TVector4::X0Y0Z0W1

		for ( TINT i = 0; i < s_iCannonPaintParticles; i++ )
		{
			TMath::SinCos( rRandom.GetFloatMinMax( 0.0f, TMath::TWO_PI ), vDirection.x, vDirection.z );
			vDirection.y = 0.0f;
			vDirection.w = 1.0f;

			TVector4 vParticlePos;
			vParticlePos.Multiply( vDirection, ms_pBlobProperties[ m_iSize ].m_fRadius );
			vParticlePos.x = m_oPhysicsMatrix.m_f41 + vParticlePos.x;
			vParticlePos.y = m_oPhysicsMatrix.m_f42 + vParticlePos.y;
			vParticlePos.z = m_oPhysicsMatrix.m_f43 + vParticlePos.z;

			vDirection.y = vDirection.y - ( rRandom.GetFloatMinMax( 0.0f, 2.5f ) + 0.5f );
			vDirection.Multiply( ms_pBlobProperties[ 0 ].m_fRadius * 15.0f );

			TVector4 vVelocity = vDirection;
			vVelocity.y        = a_rVelocity.y + vVelocity.y;
			vVelocity.x        = a_rVelocity.x + vVelocity.x;
			vVelocity.z        = a_rVelocity.z + vVelocity.z;
			vDirection         = vVelocity;

			TINT iParam38 = 0;

			if ( i < s_iCannonPaintDecals )
			{
				TUINT32 uiIndex = m_pColourComponent->GetCurrentColour();

				if ( uiIndex & 0x10 )
					uiIndex = 0x10;

				iParam38 = TINT( TREINTERPRETCAST( TUINTPTR, ms_apInkCloudSmallEffects[ uiIndex ] ) );
			}

			TINT iDecal = ( i < s_iCannonPaintDecals ) ? TINT( TREINTERPRETCAST( TUINTPTR, m_pMaterialTestSplat ) ) : 0;

			TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

			if ( uiColour & 0x10 )
				uiColour = 0x10;

			pPaintParticles->AddParticle(
			    vParticlePos,
			    vVelocity,
			    ABlobColourComponent::ms_auiColours[ uiColour ],
			    ms_pBlobProperties[ 0 ].m_fRadius,
			    iDecal,
			    iParam38,
			    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomA ) ),
			    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomB ) )
			);
		}
	}

	// The stretch out of the barrel and the trail
	m_vJumpStart       = a_rPosition;
	m_fStretchTimeLeft = ms_pBlobProperties[ m_iSize ].m_fStretchTime;

	TParticleEffectInstance*& rpTrail = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 14 ] ); // 0xB10
	rpTrail                           = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pWindLauncherTrailEffect );

	if ( rpTrail != TNULL )
	{
		TASSERT( m_oPhysicsMatrix.m_f44 == 1.0f );
		rpTrail->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
		rpTrail->SetFinishedCallback( OnBlobEffectFinished, &m_auiSounds[ 0 ] );
	}

	SetState( STATE_BALLISTIC );
}

// $deBlob: FUNCTION 0074a5d0
void ABlobEntity::BounceOffContact()
{
	TVector4 vOffset;
	vOffset.Multiply( m_oMotion.m_vContactNormal, 0.2f );

	TVector4 vPosition;
	vPosition.x = m_oPhysicsMatrix.m_f41 + vOffset.x;
	vPosition.y = m_oPhysicsMatrix.m_f42 + vOffset.y;
	vPosition.z = m_oPhysicsMatrix.m_f43 + vOffset.z;
	vPosition.w = m_oPhysicsMatrix.m_f44;
	SetPhysicsPosition( vPosition );

	TVector4 vVelocity;
	vVelocity.Multiply( m_oMotion.m_vContactNormal, 10.0f );
	vVelocity.y = vVelocity.y - ms_pBlobCommonProperties->m_fJumpVelocity;
	TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );

	SetState( STATE_BALLISTIC );
}

// $deBlob: FUNCTION 0074eb70
void ABlobEntity::UpdateStuck( TFLOAT a_fTimeStep )
{
	TFLOAT& rfStuckTime  = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5A8[ 0 ] ); // 0x5A8
	TFLOAT& rfMovingTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5A8[ 1 ] ); // 0x5AC

	TVector4 vVelocity;
	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );

	// The velocity from the position of the last step
	TVector4 vMoved;
	vMoved.x = m_oPhysicsMatrix.m_f41 - m_vPhysicsPosition.x;
	vMoved.y = m_oPhysicsMatrix.m_f42 - m_vPhysicsPosition.y;
	vMoved.z = m_oPhysicsMatrix.m_f43 - m_vPhysicsPosition.z;
	vMoved.w = m_oPhysicsMatrix.m_f44;
	vMoved.Divide( a_fTimeStep );

	TFLOAT fVelocitySq = vVelocity.MagnitudeSq();
	TFLOAT fMovedSq    = vMoved.MagnitudeSq();

	if ( 0.0025f < fVelocitySq && 0.0025f < fMovedSq )
	{
		// Moving for 0.05 s: not stuck
		rfMovingTime = a_fTimeStep + rfMovingTime;

		if ( 0.05f < rfMovingTime )
		{
			rfStuckTime  = 0.0f;
			rfMovingTime = 0.0f;
		}

		return;
	}

	TBOOL bInAir = TFALSE;

	if ( GetZJumpState() != ABlobZJumpComponent::STATE_POINT && 1 < TUINT8( m_oMotion.m_uiContactCountAndGround ) && ( ( m_uiButtons >> 5 ) & 1 ) == 0 && m_pBody != TNULL && m_pBody->IsInSimulation() )
	{
		const TVector2& rcStick = ABlobInputMap::GetSingleton()->GetMoveStick( m_iController );
		TFLOAT          fStickX = rcStick.x;
		TFLOAT          fStickY = rcStick.y;
		TFLOAT          fTime   = a_fTimeStep;
		TBOOL           bCount  = TFALSE;

		if ( m_eState == STATE_ROLL && ( 0.64f < fStickX * fStickX + fStickY * fStickY || ( m_uiButtons & ABlobInputMap::BUTTON_JUMP ) != 0 ) )
		{
			bCount = TTRUE;
		}
		else if ( m_eState == STATE_BALLISTIC && ( m_uiButtons & ABlobInputMap::BUTTON_JUMP ) != 0 )
		{
			// NOTE: retail adds 0.2 per step here (not the time step), and only this case kills
			bInAir = TTRUE;
			fTime  = 0.2f;
			bCount = TTRUE;
		}

		if ( bCount )
		{
			rfStuckTime = rfStuckTime + fTime;

			if ( 3.0f < rfStuckTime && bInAir )
			{
				rfStuckTime  = 0.0f;
				rfMovingTime = 0.0f;
				OnOutOfPoints( TFALSE );
			}
		}
	}

	rfMovingTime = 0.0f;
}

// $deBlob: FUNCTION 0074ed70
void ABlobEntity::UpdateJelly( TFLOAT a_fTimeStep )
{
	m_vBallOffset = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1

	TFLOAT         fRadius     = ms_pBlobProperties[ m_iSize ].m_fRadius;
	TFLOAT         fMaxOffset  = ms_pBlobProperties[ m_iSize ].m_fJellyOffset * fRadius;
	TFLOAT         fMaxStretch = fMaxOffset;
	const TVector4 vPosition   = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	TVector4 vVelocity;
	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );

	TFLOAT fMass = ( m_pBody == TNULL ) ? 1.0f : m_pBody->m_fMass;

	// Where the jelly rests: above the ball while rolling, off the wall or the hot plate
	if ( ( m_uiFlags & 0x2000 ) == 0 )
	{
		TVector4 vRest;

		if ( m_eState == STATE_ROLL )
		{
			vRest.Multiply( TVector4( 0.0f, 1.0f, 0.0f, 1.0f ), ms_pBlobProperties[ m_iSize ].m_fJellyOffset * 0.2f * ms_pBlobProperties[ m_iSize ].m_fRadius );
			vRest.x          = vPosition.x + vRest.x;
			vRest.y          = vPosition.y + vRest.y;
			vRest.z          = vPosition.z + vRest.z;
			m_oJelly.m_vRest = vRest;
		}
		else if ( m_eState == STATE_STUCKWALL )
		{
			TINT iZJumpState = GetZJumpState();

			if ( iZJumpState != ABlobZJumpComponent::STATE_AIM && iZJumpState != ABlobZJumpComponent::STATE_AIMNODE )
			{
				vRest.Multiply( m_oMotion.m_vWallNormal, ms_pBlobProperties[ m_iSize ].m_fJellyOffset * -0.5f * ms_pBlobProperties[ m_iSize ].m_fRadius );
				vRest.x          = vRest.x + vPosition.x;
				vRest.y          = vRest.y + vPosition.y;
				vRest.z          = vRest.z + vPosition.z;
				m_oJelly.m_vRest = vRest;
			}
		}
		else if ( m_eState == STATE_HOTPLATE )
		{
			vRest.Multiply( m_oMotion.m_vContactNormal, ms_pBlobProperties[ m_iSize ].m_fJellyOffset * -0.2f * ms_pBlobProperties[ m_iSize ].m_fRadius );
			vRest.x          = vRest.x + vPosition.x;
			vRest.y          = vRest.y + vPosition.y;
			vRest.z          = vRest.z + vPosition.z;
			m_oJelly.m_vRest = vRest;
		}
		else
		{
			m_oJelly.m_vRest = vPosition;
		}
	}

	// The jelly's gravity (y points down)
	m_oJelly.AddForce( TVector4( 0.0f, fMass * 80.0f, 0.0f, 1.0f ) );
	m_oJelly.Tick( a_fTimeStep );

	if ( ( m_uiFlags & 0x80000000 ) == 0 )
		m_oJellySquash.Tick( a_fTimeStep );

	// The model's centre: the jelly, at most fMaxOffset (0.99 of it) from the ball
	m_vJellyPosition = m_oJelly.m_vPosition;

	TVector4 vOffset;
	vOffset.x = m_vJellyPosition.x - vPosition.x;
	vOffset.y = m_vJellyPosition.y - vPosition.y;
	vOffset.z = m_vJellyPosition.z - vPosition.z;
	vOffset.w = m_vJellyPosition.w;

	if ( fMaxOffset < vOffset.NormaliseMag() )
	{
		m_vJellyPosition.Multiply( vOffset, fMaxOffset * 0.99f );
		m_vJellyPosition.x = m_vJellyPosition.x + vPosition.x;
		m_vJellyPosition.y = m_vJellyPosition.y + vPosition.y;
		m_vJellyPosition.z = m_vJellyPosition.z + vPosition.z;
	}

	m_fSquashCompensation = 1.0f;
	m_fSquash             = 1.0f;

	// Squashed against what is closest to the model's centre (layers 0x83)
	TSphereData oSphere;
	oSphere.SetRadius( fRadius );

	TMatrix44 oMatrix = TMatrix44::IDENTITY;
	oMatrix.SetTranslation( m_vJellyPosition );

	m_oMotion.m_uiContactFlags &= 0xFFBF;

	TASSERT( m_pJellyQueryHelper, "phantomContainer" );

	TPhysicsShapeContact oContact;
	TBOOL                bHit = TPhysicsManager::GetSingleton()->CollideShapeClosest( &oSphere, oMatrix, 0x83, oContact, m_pJellyQueryHelper->IsInSimulation() ? m_pJellyQueryHelper : TNULL );

	if ( bHit )
	{
		TVector4 vToContact;
		vToContact.x = m_vJellyPosition.x - oContact.m_vPosition.x;
		vToContact.y = m_vJellyPosition.y - oContact.m_vPosition.y;
		vToContact.z = m_vJellyPosition.z - oContact.m_vPosition.z;
		vToContact.w = m_vJellyPosition.w;

		m_fSquash = vToContact.Magnitude() / fRadius;
		m_fSquash = TMath::Max( 0.5f, m_fSquash );
		m_vUnk614 = oContact.m_vNormal;
		m_oMotion.m_uiContactFlags |= 0x40;
		m_fSquashCompensation = TMath::OneOverSqrt( m_fSquash );

		m_vBallOffset.Multiply( m_vUnk614, m_vUnk614.y * ( m_vJellyPosition.y - m_oPhysicsMatrix.m_f42 ) + m_vUnk614.x * ( m_vJellyPosition.x - m_oPhysicsMatrix.m_f41 ) + m_vUnk614.z * ( m_vJellyPosition.z - m_oPhysicsMatrix.m_f43 ) );
	}

	// The stretch (after a jump): along the way from m_vJumpStart, fading out over Properties::m_fStretchTime
	if ( 0.0f < m_fStretchTimeLeft )
	{
		m_fStretchTimeLeft = m_fStretchTimeLeft - a_fTimeStep;
		m_fStretchTimeLeft = TMath::Max( 0.0f, m_fStretchTimeLeft );

		TVector4 vStretch;
		vStretch.x = m_oPhysicsMatrix.m_f41 - m_vJumpStart.x;
		vStretch.y = m_oPhysicsMatrix.m_f42 - m_vJumpStart.y;
		vStretch.z = m_oPhysicsMatrix.m_f43 - m_vJumpStart.z;
		vStretch.w = m_oPhysicsMatrix.m_f44;

		TFLOAT fDistance = vStretch.NormaliseMag();
		fDistance        = TMath::Min( fMaxStretch, fDistance );

		TFLOAT fStretch = ( ( fDistance - fRadius * 0.2f ) * 2.0f ) / fRadius;
		fStretch        = TMath::Max( 0.0f, fStretch );

		m_fStretch             = TMath::LerpTowards( 1.0f, fStretch + 1.0f, m_fStretchTimeLeft / ms_pBlobProperties[ m_iSize ].m_fStretchTime );
		m_vStretchDirection    = vStretch;
		m_fStretchCompensation = TMath::OneOverSqrt( m_fStretch );
	}
	else
	{
		m_fStretch             = 1.0f;
		m_fStretchCompensation = 1.0f;
		m_fStretchTimeLeft     = 0.0f;
		m_vStretchDirection    = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	}

	// Just off the surface it's squashed against
	TVector4 vSurface;
	vSurface.Multiply( m_vUnk614, 0.1f );
	vSurface.x = vSurface.x + oContact.m_vPosition.x;
	vSurface.y = vSurface.y + oContact.m_vPosition.y;
	vSurface.z = vSurface.z + oContact.m_vPosition.z;

	TFLOAT fSquashChange = m_fSquashCompensation - m_fPrevSquashCompensation;

	if ( 1.05f < m_fSquashCompensation && 0.05f < fSquashChange )
	{
		// Squashed more: a splash of paint off the surface
		TVector4 vBounce  = m_oJelly.m_vVelocity;
		TUINT32  uiColour = m_pColourComponent->GetCurrentColour();
		TFLOAT   fSpeed   = -( m_vUnk614.y * vBounce.y + m_vUnk614.x * vBounce.x + m_vUnk614.z * m_oJelly.m_vVelocity.z );

		if ( ( ( uiColour & 0x10 ) != 0 || uiColour != 0 ) && ms_pBlobProperties[ m_iSize ].m_fUnk40 < fSpeed )
		{
			TMatrix44 oRotation = TMatrix44::IDENTITY;

			if ( -1.0f < m_vUnk614.y && m_vUnk614.y < 1.0f )
			{
				TQuaternion oQuat;
				oQuat.SetVectorDirection( TVector3( m_vUnk614.x, m_vUnk614.y, m_vUnk614.z ), TVector3( 0.0f, -1.0f, 0.0f ) );
				oRotation.SetFromQuaternion( oQuat );
			}

			TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

			TINT   iNumParticles = TMath::CeilToInt( fSquashChange * 10.0f ) + 5;
			TINT   iNumDecals    = TMath::CeilToInt( TFLOAT( iNumParticles ) * 0.25f );
			TFLOAT fScale        = m_fSquashCompensation / 1.05f;

			TVector4 vReflect;
			vReflect.Multiply( m_vUnk614, fSpeed * 2.0f );
			vBounce.x = vReflect.x + vBounce.x;
			vBounce.y = vReflect.y + vBounce.y;
			vBounce.z = vReflect.z + vBounce.z;

			APaintParticleSystem* pPaintParticles = APaintParticleSystem::s_Instance;
			TVector4              vDirection      = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1

			if ( 0 < iNumParticles )
			{
				TFLOAT fRingRadius = fRadius * 0.75f;
				TFLOAT fSpraySpeed = fRadius * 5.0f * fScale;

				for ( TINT i = 0; i < iNumParticles; i++ )
				{
					TMath::SinCos( rRandom.GetFloatMinMax( 0.0f, TMath::TWO_PI ), vDirection.x, vDirection.z );
					vDirection.y = 0.0f;
					vDirection.w = 1.0f;
					TMatrix44::TransformVector( vDirection, oRotation, vDirection );

					TVector4 vParticlePos;
					vParticlePos.Multiply( vDirection, fRingRadius );
					vParticlePos.x = vParticlePos.x + vSurface.x;
					vParticlePos.y = vParticlePos.y + vSurface.y;
					vParticlePos.z = vParticlePos.z + vSurface.z;

					vDirection.y = vDirection.y - ( rRandom.GetFloat() * 0.207f + 0.5f );
					vDirection.Multiply( fSpraySpeed );

					TVector4 vAlong;
					vAlong.Multiply( vBounce, 0.2f );
					vDirection.x = vAlong.x + vDirection.x;
					vDirection.y = vAlong.y + vDirection.y;
					vDirection.z = vAlong.z + vDirection.z;

					TINT iParam38 = 0;

					if ( i < iNumDecals )
					{
						TUINT32 uiIndex = m_pColourComponent->GetCurrentColour();
						if ( ( uiIndex & 0x10 ) != 0 )
							uiIndex = 0x10;

						iParam38 = TINT( TREINTERPRETCAST( TUINTPTR, ms_apInkCloudSmallEffects[ uiIndex ] ) );
					}

					TINT iDecal = ( i < iNumDecals ) ? TINT( TREINTERPRETCAST( TUINTPTR, m_pMaterialTestSplat ) ) : 0;

					pPaintParticles->AddParticle(
					    vParticlePos,
					    vDirection,
					    m_pColourComponent->m_uiFadeColour2,
					    fRadius,
					    iDecal,
					    iParam38,
					    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomA ) ),
					    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomB ) )
					);
				}
			}

			m_fPrevSquashCompensation = m_fSquashCompensation;
		}
	}
	else if ( m_fSquashCompensation < 1.1f )
	{
		m_fPrevSquashCompensation = 1.0f;
	}

	// Just squashed hard against something: the paint splash
	TUINT8 uiContactFlags = TUINT8( m_oMotion.m_uiContactFlags );

	if ( ( ( TUINT8( m_oMotion.m_uiPrevContactFlags ) ^ uiContactFlags ) & uiContactFlags & 0x40 ) != 0 &&
	     8.0f < TMath::Abs( m_vUnk614.x * m_oJelly.m_vVelocity.x + m_vUnk614.y * m_oJelly.m_vVelocity.y + m_vUnk614.z * m_oJelly.m_vVelocity.z ) )
	{
		TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

		if ( ( uiColour & 0x10 ) != 0 || uiColour != 0 )
		{
			AParticleEffectHelper::SetColour( ms_pPaintSplashEffect, m_pColourComponent->m_uiFadeColour2, TNULL );

			TParticleEffectInstance* pSplash = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pPaintSplashEffect );

			if ( pSplash != TNULL )
			{
				TMatrix44 oSplashMatrix = TMatrix44::IDENTITY;
				oSplashMatrix.SetTranslation( vSurface );
				pSplash->GetTransform() = oSplashMatrix;
			}
		}
	}
}

// The soft collision (UpdateSoftCollision): the push of meshes (material 0x4000000) and of the other shapes
static TFLOAT s_fSoftMeshSpeedFar   = -30.0f; // the speed into it limit at the edge of the range
static TFLOAT s_fSoftMeshSpeedNear  = 5.0f;   // ... at the surface
static TFLOAT s_fSoftMeshRange      = 0.5f;   // of s_fPhantom424Radius
static TFLOAT s_fSoftMeshWobble     = 0.0f;   // never written
static TFLOAT s_fSoftOtherSpeedFar  = -30.0f;
static TFLOAT s_fSoftOtherSpeedNear = 20.0f;
static TFLOAT s_fSoftOtherWobble    = 20.0f;
static TFLOAT s_fSoftOtherRange     = 1.0f;

// $deBlob: FUNCTION 0074de70
void ABlobEntity::UpdateSoftCollision()
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
	const TVector4&  rcPosition      = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	// Moved far in one step: not through a soft surface (layer 0x100)
	if ( s_fPhantom424Radius < TVector4::Distance( m_vPhysicsPosition, rcPosition ) )
	{
		TPhysicsRay oRay;
		oRay.m_vFrom            = m_vPhysicsPosition;
		oRay.m_vTo              = rcPosition;
		oRay.m_uiCollisionLayer = 0x100;

		TPhysicsShapeContact oContact;

		if ( pPhysicsManager->CollideRay( oRay, oContact, TNULL ) &&
		     oContact.m_vSurfaceNormal.x * ( oRay.m_vTo.x - oRay.m_vFrom.x ) + oContact.m_vSurfaceNormal.y * ( oRay.m_vTo.y - oRay.m_vFrom.y ) + oContact.m_vSurfaceNormal.z * ( oRay.m_vTo.z - oRay.m_vFrom.z ) < 0.0f )
		{
			TVector4 vPosition;
			vPosition.x = oContact.m_vPosition.x + oContact.m_vSurfaceNormal.x;
			vPosition.y = oContact.m_vPosition.y + oContact.m_vSurfaceNormal.y;
			vPosition.z = oContact.m_vPosition.z + oContact.m_vSurfaceNormal.z;
			vPosition.w = oContact.m_vPosition.w;
			SetPhysicsPosition( vPosition );
		}
	}

	m_uiFlags &= ~0x200;

	TSphereData oSphere;
	oSphere.SetRadius( s_fPhantom424Radius );

	TMatrix44 oMatrix = TMatrix44::IDENTITY;
	oMatrix.SetTranslation( rcPosition );

	TASSERT( m_pUnk424, "phantomContainer" );

	TPhysicsShapeContact aContacts[ 3 ];
	TINT                 iNumContacts = pPhysicsManager->CollideShape( &oSphere, oMatrix, 0x100, aContacts, 3, m_pUnk424->IsInSimulation() ? m_pUnk424 : TNULL );

	if ( iNumContacts <= 0 )
		return;

	TVector4 vForce = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	TVector4 vVelocity;
	pPhysicsManager->GetLinearVelocity( m_pBody, vVelocity );

	TINT   iNumPushes = 0;
	TBOOL  abPushed[ 3 ];
	TFLOAT fVelX = vVelocity.x;
	TFLOAT fVelY = vVelocity.y;
	TFLOAT fVelZ = vVelocity.z;

	for ( TINT i = 0; i < iNumContacts; i++ )
	{
		TPhysicsShapeContact& rContact = aContacts[ i ];
		abPushed[ i ]                  = TFALSE;

		// One push per surface direction
		TBOOL bSame = TFALSE;

		for ( TINT j = 0; j < i; j++ )
		{
			if ( abPushed[ j ] && 0.999f < rContact.m_vSurfaceNormal.x * aContacts[ j ].m_vSurfaceNormal.x + rContact.m_vSurfaceNormal.y * aContacts[ j ].m_vSurfaceNormal.y + rContact.m_vSurfaceNormal.z * aContacts[ j ].m_vSurfaceNormal.z )
			{
				bSame = TTRUE;
				break;
			}
		}

		if ( bSame )
			continue;

		const TVector4 vNormal = rContact.m_vSurfaceNormal;

		// In front of the surface (or on it)
		TFLOAT fSide = TMath::Sign( ( rcPosition.y * vNormal.y + rcPosition.x * vNormal.x + rcPosition.z * vNormal.z ) - ( rContact.m_vPosition.y * vNormal.y + vNormal.x * rContact.m_vPosition.x + vNormal.z * rContact.m_vPosition.z ) );

		if ( fSide < 0.0f )
			continue;

		if ( ( rContact.m_uiMaterial & 0x4000000 ) != 0 && rContact.m_pBody != TNULL && rContact.m_pBody->m_eShapeType == 6 &&
		     !TMath::IsPointInTriangle( rContact.m_avTriangle[ 0 ], rContact.m_avTriangle[ 1 ], rContact.m_avTriangle[ 2 ], rcPosition ) )
			continue;

		TVector4 vAway;
		vAway.x = rcPosition.x - rContact.m_vPosition.x;
		vAway.y = rcPosition.y - rContact.m_vPosition.y;
		vAway.z = rcPosition.z - rContact.m_vPosition.z;
		vAway.w = rcPosition.w;

		TFLOAT fDistance = vAway.NormaliseXZ() * fSide;

		TFLOAT fStiffness;
		TFLOAT fSpeedNear;
		TFLOAT fSpeedFar;
		TFLOAT fRange;
		TFLOAT fWobble;

		if ( rContact.m_uiMaterial & 0x4000000 )
		{
			static TFLOAT s_fMeshStiffness = 320.0f;
			fStiffness                     = s_fMeshStiffness;
			fSpeedNear                     = s_fSoftMeshSpeedNear;
			fSpeedFar                      = s_fSoftMeshSpeedFar;
			fRange                         = s_fSoftMeshRange;
			fWobble                        = s_fSoftMeshWobble;
		}
		else
		{
			static TFLOAT s_fOtherStiffness = 560.0f;
			fStiffness                      = s_fOtherStiffness;
			fSpeedNear                      = s_fSoftOtherSpeedNear;
			fSpeedFar                       = s_fSoftOtherSpeedFar;
			fRange                          = s_fSoftOtherRange;
			fWobble                         = s_fSoftOtherWobble;
		}

		fRange = fRange * s_fPhantom424Radius;

		if ( fRange < fDistance )
			continue;

		TFLOAT fCloseness = ( fRange - fDistance ) / fRange;

		// The speed into the surface is limited (more the closer, with a wobble over the time)
		TFLOAT fMinSpeed = TFLOAT( sin( TDOUBLE( AGameStatus::GetSingleton()->GetTimeLeft() * TMath::TWO_PI ) ) ) * fWobble + ( ( fSpeedNear - fSpeedFar ) * fCloseness + fSpeedFar );

		TFLOAT fAlong = vNormal.y * fVelY + fVelX * vNormal.x + vNormal.z * fVelZ;

		TVector4 vAlong;
		vAlong.Multiply( vNormal, fAlong );
		TFLOAT fAcrossX = fVelX - vAlong.x;
		TFLOAT fAcrossY = fVelY - vAlong.y;
		TFLOAT fAcrossZ = fVelZ - vAlong.z;

		TMath::Clip( fAlong, fMinSpeed, ms_pBlobProperties[ m_iSize ].m_fMaxSpeed );

		vAlong.Multiply( vNormal, fAlong );
		fVelX       = vAlong.x + fAcrossX;
		vVelocity.x = fVelX;
		fVelY       = vAlong.y + fAcrossY;
		vVelocity.y = fVelY;
		fVelZ       = vAlong.z + fAcrossZ;
		vVelocity.z = fVelZ;

		// And pushed away from it
		TVector4 vPush;
		vPush.Multiply( vAway, fCloseness * fStiffness );
		iNumPushes++;
		vForce.x      = vPush.x + vForce.x;
		abPushed[ i ] = TTRUE;
		vForce.y      = vPush.y + vForce.y;
		vForce.z      = vPush.z + vForce.z;
	}

	TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );

	if ( 0 < iNumPushes )
	{
		vForce.Divide( TFLOAT( iNumPushes ) );

		TFLOAT fPulse = TFLOAT( sin( TDOUBLE( AGameStatus::GetSingleton()->GetTimeLeft() * 1.5f * TMath::TWO_PI ) ) ) * 0.5f + 1.0f;

		if ( m_eState == STATE_FLOAT )
			fPulse = fPulse * 2.0f;

		vForce.Multiply( ms_pBlobProperties[ m_iSize ].m_fMass * fPulse );
		pPhysicsManager->ApplyForce( m_pBody, vForce );
		m_uiFlags |= 0x200;
	}
}

// $deBlob: FUNCTION 0074c120
void ABlobEntity::SetState( TINT a_eState )
{
	if ( a_eState == m_eState )
		return;

	// Leave the current state
	switch ( m_eState )
	{
		case STATE_ROLL:
			m_uiFlags &= ~0x100000;

			if ( a_eState != STATE_STUCKWALL )
				StopRollEffects();

			if ( m_auiSounds[ 11 ] != 0 ) // 0xB04
				TREINTERPRETCAST( TParticleEffectInstance*, m_auiSounds[ 11 ] )->SetPaused( TTRUE );

			break;
		case STATE_BALLISTIC:
		{
			TParticleEffectInstance*& rpTrail = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 14 ] ); // 0xB10

			if ( rpTrail != TNULL )
			{
				rpTrail->Stop();
				rpTrail = TNULL;
			}

			if ( m_auiSounds[ 11 ] != 0 ) // 0xB04
				TREINTERPRETCAST( TParticleEffectInstance*, m_auiSounds[ 11 ] )->SetPaused( TTRUE );

			if ( GetZJumpState() == ABlobZJumpComponent::STATE_ATTACKFLIGHT || GetZJumpState() == ABlobZJumpComponent::STATE_LMARKFLIGHT )
				m_pZJump->EndAttack( TFALSE );
		}
			m_uiFlags &= ~0x800;
			break;
		case STATE_STUCKWALL:
			m_uiFlags &= ~0x40000;

			if ( a_eState != STATE_ROLL )
				StopRollEffects();

			break;
		case STATE_HOTPLATE:
			m_uiFlags &= ~0x40000;
			m_pHotPlate = TNULL;
			// NOTE: retail clears a TUINT32 at 0xAD2: the shakes done (0xAD2) and needed (0xAD4)
			m_uiTargettingAndShakes &= 0x0000FFFF;
			m_uiShakesNeeded = 0;
			m_auiUnk564[ 2 ] = 0; // 0x56C
			m_pSoundComponent->StopHotPlate();

			if ( m_auiSounds[ 7 ] != 0 ) // 0xAF4
				TREINTERPRETCAST( TParticleEffectInstance*, m_auiSounds[ 7 ] )->SetPaused( TTRUE );

			if ( m_iCamShakeHotPlateID >= 0 )
			{
				ACameraManager::GetSingleton()->GetShakeFunction().StopShake( m_iCamShakeHotPlateID );
				m_iCamShakeHotPlateID = -1;
			}

			break;
		case STATE_LMARK:
			TASSERT( m_iCurrentLandmarkTrigger < 0, "m_iCurrentLandmarkTrigger < 0" );
			m_iCurrentLandmarkTrigger = -1;
			TFALLTHROUGH;
		case STATE_BOSS:
			m_uiFlags &= ~0x40000;
			m_pBoss = TNULL;
			// NOTE: retail clears a TUINT32 at 0xAD2: the shakes done (0xAD2) and needed (0xAD4)
			m_uiTargettingAndShakes &= 0x0000FFFF;
			m_uiShakesNeeded = 0;
			m_auiUnk564[ 2 ] = 0; // 0x56C
			break;
		case STATE_CANNON:
			if ( m_uiFlags & 0x8000000 )
			{
				m_uiFlags &= ~0x8000000;
				SetPaintPoints( m_iPaintPoints );
			}

			m_uiFlags &= 0xFBFBFFFF;

			if ( m_pAnimationComponent->m_iState == ABlobAnimationComponent::STATE_ENTER_CANNON || m_pAnimationComponent->m_iState == ABlobAnimationComponent::STATE_INSIDE_CANNON )
				m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_BALL );

			if ( m_pWindCannon != TNULL ) // 0xB4C the cannon
			{
				m_pWindCannon->OnBlobLeaving();
				m_pWindCannon = TNULL;
			}

			break;
		case STATE_DEAD:
		case STATE_FAIL:
		case STATE_VICTORY:
		case STATE_LOSTLASTLIFE:
		{
			TParticleEffectInstance*& rpEffect = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 12 ] ); // 0xB08

			if ( rpEffect != TNULL )
			{
				rpEffect->Stop();
				rpEffect = TNULL;
			}

			APlayerManager::GetSingleton()->GetBlobDataOf( this );

			// The splitscreen: the viewport of the blob fades in from black
			if ( APlayerManager::GetSingleton()->GetNumPlayers() > 1 )
			{
				TINT iViewport = GetBlobViewport( APlayerManager::GetSingleton(), this );

				if ( TUINT32( iViewport ) <= 11 )
				{
					TFLOAT fFade = AGameStateController::GetSingleton()->GetFade( iViewport );

					if ( 0.0f < fFade )
					{
						AGameStateController::GetSingleton()->FadeFromBlack( 1.0f, iViewport );
						AGameStateController::GetSingleton()->SetFade( fFade, iViewport );
					}
				}
			}

			break;
		}
		case STATE_FLOAT:
			m_uiFlags &= 0xFFFFEE7F;
			m_auiUnk564[ 2 ] = 0; // 0x56C
			break;
		case STATE_GRASSBOMB:
			m_uiFlags &= ~0x4000000;
			break;
		case STATE_RESPAWN:
			TPhysicsManager::StopBody( m_pBody );
			break;
		case STATE_LEVEL_EXIT:
			m_pLevelExitTrigger = TNULL;
			break;
	}

	// Enter the new one
	TBOOL bRemoveBody = TFALSE; // LABEL_84
	switch ( a_eState )
	{
		case STATE_ROLL:
			// NOTE: retail calls an empty function of m_pSoundComponent here
			m_uiFlags &= ~0x20000;
			break;
		case STATE_BALLISTIC:
			m_uiFlags &= 0xFF79FFFF;
			m_auiUnk564[ 0 ]           = 0;
			m_auiUnk564[ 4 ]           = 0;
			m_vClimbStart              = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
			m_oMotion.m_auiUnk280[ 6 ] = 0; // 0x318
			m_oMotion.m_auiUnk280[ 7 ] = 0; // 0x31C
			m_oMotion.ResetContacts();

			if ( m_eState == STATE_ROLL )
				*TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 1 ] ) = 0.2f;

			break;
		case STATE_STUCKWALL:
			m_uiFlags &= ~0x20000;
			break;
		case STATE_HOTPLATE:
		{
			m_uiFlags &= 0xFFF9FFFF;
			// NOTE: retail sets a TUINT32 at 0xAD2 to 0x100000: no shakes done (0xAD2), 16 needed (0xAD4)
			m_uiTargettingAndShakes &= 0x0000FFFF;
			m_uiShakesNeeded = 0x10;
			m_auiUnk564[ 2 ] = *TREINTERPRETCAST( const TUINT32*, &ms_pBlobCommonProperties->m_fHotPlateTime ); // 0x56C
			m_auiUnk564[ 3 ] = *TREINTERPRETCAST( const TUINT32*, &ms_pBlobCommonProperties->m_fHotPlateTime ); // 0x570
			m_pZJump->Cancel();
			m_pZJump->OnLanded();
			m_pSoundComponent->StartHotPlate();

			TParticleEffectInstance*& rpHotPlate = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 7 ] ); // 0xAF4

			if ( rpHotPlate != TNULL )
				rpHotPlate->SetPaused( TFALSE );
			else
				rpHotPlate = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pHotPlateEffect );

			if ( rpHotPlate != TNULL )
			{
				rpHotPlate->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
				rpHotPlate->GetTransform().m_f42 = GetBoundingSphereRadius() + rpHotPlate->GetTransform().m_f42;
				rpHotPlate->SetFinishedCallback( OnBlobEffectFinished, &m_auiSounds[ 0 ] );
			}

			if ( m_iCamShakeHotPlateID < 0 )
				m_iCamShakeHotPlateID = ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 2, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 1.0f, APlayerManager::GetSingleton()->GetPlayerIndex( this ) );

			break;
		}
		case STATE_LMARK:
			TASSERT( -1 < m_iCurrentLandmarkTrigger, "-1 < m_iCurrentLandmarkTrigger" );
			TFALLTHROUGH;
		case STATE_BOSS:
		{
			m_uiFlags &= 0xFFF9FFFF;
			m_pZJump->Cancel();
			m_fUntargetableTime = TMath::Max( 1.0f, m_fUntargetableTime );

			TParticleEffectInstance*& rpElectrocution = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 6 ] ); // 0xAF0

			if ( rpElectrocution != TNULL )
			{
				rpElectrocution->StopInstant();
				rpElectrocution = TNULL;
			}

			bRemoveBody = TTRUE;
			break;
		}
		case STATE_FLOAT:
			TASSERT( m_uiFlags & 0x180, "m_uiFlags & (BFLAG_INWATER|BFLAG_ININK)" );
			m_uiFlags &= ~0x20000;
			m_auiUnk564[ 0 ]           = 0; // 0x564
			m_oMotion.m_auiUnk280[ 6 ] = 0; // 0x318
			m_oMotion.m_auiUnk280[ 7 ] = 0; // 0x31C
			m_oMotion.ResetContacts();

			if ( !IsEvilBlob() && ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) == 0 )
			{
				if ( m_uiFlags & 0x100 )
				{
					// Into the ink: black and a paint point
					ChangeColour( 0x10 );
					RemovePaintPoints( 1, TTRUE );
				}
				else
				{
					// Into the water: washed off, the points towards m_uiWaterPoints
					WashOff();

					if ( ms_pBlobCommonProperties->m_uiWaterPoints < TUINT32( m_iPaintPoints ) )
					{
						RemovePaintPoints( 1, TFALSE );
						m_auiUnk564[ 3 ] = *TREINTERPRETCAST( const TUINT32*, &ms_pBlobCommonProperties->m_fFirstDrainDelay ); // 0x570
					}
					else if ( TUINT32( m_iPaintPoints ) < ms_pBlobCommonProperties->m_uiWaterPoints )
					{
						AddPaintPoints( 1 );
						*TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 3 ] ) = 0.1f; // 0x570
					}
				}
			}

			m_auiUnk564[ 2 ] = 0; // 0x56C

			if ( m_iLeechbotsAttached != 0 )
				AEnemyGrappler::ReleasePlayer( this );

			// NOTE: retail returns here when the blob is dead: the state stays STATE_DEAD and the body isn't touched
			if ( m_eState == STATE_DEAD )
				return;

			break;
		case STATE_CANNON:
		{
			m_pZJump->Cancel();

			APropWindCannon* pCannon = m_pWindCannon;
			AEntityIOPacket  oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_ENTER;
			oPacket.m_iData    = 0;
			pCannon->HandleEntityIO( this, oPacket );

			m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_ENTER_CANNON );
			m_uiFlags             = ( m_uiFlags & 0xFFFBFFFF ) | 0x4000000;
			m_fSquashCompensation = 1.0f;
			m_fSquash             = 1.0f;
			m_vBallOffset         = TVector4( 0.0f, 0.0f, 0.0f, 1.0f ); // TVector4::X0Y0Z0W1

			// In the cannon: the ball's radius below its matrix (0xBC)
			const TMatrix44* pCannonMatrix = &m_pWindCannon->GetTarget()->GetMatrix();
			TFLOAT           fRadius       = -ms_pBlobProperties[ m_iSize ].m_fRadius;

			TVector4 vPosition;
			vPosition.x = pCannonMatrix->m_f41 + 0.0f;
			vPosition.y = pCannonMatrix->m_f42 + fRadius;
			vPosition.z = pCannonMatrix->m_f43 + 0.0f;
			vPosition.w = 1.0f;
			SetPhysicsPosition( vPosition );

			bRemoveBody = TTRUE;
			break;
		}
		case STATE_GRASSBOMB:
			m_uiFlags |= 0x4000000;
			break;
		case STATE_RESPAWN:
			m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_RESPAWN );
			break;
		case STATE_LEVEL_EXIT:
			m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_LEVEL_EXIT );
			bRemoveBody = TTRUE;
			break;
		case STATE_DEAD:
			m_uiFlags &= 0xFFFCFFFF;
			m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_DEAD );
			m_pZJump->End();
			bRemoveBody = TTRUE;
			break;
		case STATE_FAIL:
		case STATE_VICTORY:
		case STATE_LOSTLASTLIFE:
			if ( a_eState == STATE_FAIL )
				m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_DEFEAT );
			else if ( a_eState == STATE_LOSTLASTLIFE )
				m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_LOSTLASTLIFE );
			else
				m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_VICTORY );

			m_uiFlags &= 0xFFFCFFFF;
			m_pZJump->End();
			break;
	}

	if ( bRemoveBody )
	{
		// LABEL_84
		if ( m_pBody != TNULL && m_pBody->IsInSimulation() )
			TPhysicsManager::GetSingleton()->RemoveFromSimulation( m_pBody, TTRUE );
	}
	// LABEL_95
	else if ( m_pBody == TNULL || !m_pBody->IsInSimulation() )
	{
		TPhysicsManager::GetSingleton()->AddToSimulation( m_pBody, TTRUE );

		if ( m_pBody != TNULL && m_pBody->IsInSimulation() )
			m_pBody->m_body->activate();
	}

	// LABEL_100
	m_eState = a_eState;

	m_pSoundComponent->MuteLoopsForState();
}

// Starts a liquid effect at a_rPosition
static void StartLiquidEffect( TParticleEffect* a_pEffect, const TVector4& a_rPosition )
{
	TParticleEffectInstance* pInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( a_pEffect );

	if ( pInstance != TNULL )
		pInstance->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, a_rPosition );
}

// $deBlob: FUNCTION 0074d2d0
TBOOL ABlobEntity::CheckLiquid()
{
	TBOOL bEntered = TFALSE;

	// Water
	if ( ( m_oMotion.m_uiContactFlags & 1 ) != 0 )
	{
		TUINT32 uiFlags = m_uiFlags;

		if ( ( uiFlags & 0x80 ) == 0 )
		{
			m_uiFlags = uiFlags | 0x80;
			if ( s_fLiquidSurfaceCos < m_oMotion.m_vLiquidNormal.DotProduct3( TVector4::VEC_NEGY ) )
			{
				TParticleEffect* pSplash = ms_pWaterSplashSlamEffect;

				if ( ( uiFlags & 0x20000 ) == 0 && ( m_eState != STATE_LEVEL_EXIT || m_pAnimationComponent->m_iState != ABlobAnimationComponent::STATE_LEVEL_EXIT ) )
					pSplash = ms_pWaterSplashEffect;

				StartLiquidEffect( pSplash, m_oMotion.m_vLiquidHit );

				TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

				if ( uiColour != 0 && ( ( uiColour & 0x10 ) != 0 || uiColour < 0x11 ) )
				{
					if ( ( uiColour & 0x10 ) != 0 )
						uiColour = 0x10;

					StartLiquidEffect( ms_apInkCloudEffects[ uiColour ], m_oMotion.m_vLiquidHit );
				}
			}
			m_pSoundComponent->PlayLiquidEnter();
			bEntered = TTRUE;

			// The water hint, once more than 15 s of the level have passed
			AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();

			if ( rLevelData.m_fTimeLimit - rLevelData.m_fTimeLeft > 15.0f )
				APropTriggerHelp::ShowContextHelp( 1 );
		}
	}
	else
	{
		m_uiFlags &= ~0x80;
	}

	// Ink
	if ( ( m_oMotion.m_uiContactFlags & 2 ) == 0 )
	{
		m_uiFlags &= ~0x100;
		return bEntered;
	}

	if ( ( m_uiFlags & 0x100 ) != 0 )
		return bEntered;

	m_uiFlags |= 0x100;
	if ( s_fLiquidSurfaceCos < m_oMotion.m_vLiquidNormal.DotProduct3( TVector4::VEC_NEGY ) )
		StartLiquidEffect( ms_pInkSplashEffect, m_oMotion.m_vLiquidHit );

	m_pSoundComponent->PlayLiquidEnter();
	return TTRUE;
}

// $deBlob: FUNCTION 0074d590
void ABlobEntity::UpdateLiquidCollision( const TVector4& a_rcPosition )
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
	TFLOAT           fRadius         = ms_pBlobProperties[ m_iSize ].m_fRadius;
	TFLOAT           fPadding        = ( m_uiFlags & 0x180 ) ? 0.5f : 0.0f;
	TFLOAT           fBottom         = fPadding + fRadius;

	// The ray: from 40 above the ball down to its bottom (y points down)
	TPhysicsRay oRay;
	oRay.m_vFrom            = a_rcPosition;
	oRay.m_vFrom.y          = a_rcPosition.y - 40.0f;
	oRay.m_vTo              = a_rcPosition;
	oRay.m_vTo.y            = a_rcPosition.y + fBottom;
	oRay.m_uiCollisionLayer = 0x20;

	// The liquid ray cast helper's box around the ray
	TVector4 aBox[ 2 ];
	TVector4 vSum( oRay.m_vTo.x + oRay.m_vFrom.x, oRay.m_vFrom.y + oRay.m_vTo.y, oRay.m_vTo.z + oRay.m_vFrom.z, oRay.m_vFrom.w );
	aBox[ 0 ].Multiply( vSum, 0.5f );
	TVector4 vDiff( oRay.m_vFrom.x - oRay.m_vTo.x, oRay.m_vFrom.y - oRay.m_vTo.y, oRay.m_vFrom.z - oRay.m_vTo.z, oRay.m_vFrom.w );
	aBox[ 1 ].Multiply( vDiff, 0.5f );
	aBox[ 1 ].Abs();
	pPhysicsManager->SetAabbPhantomBox( m_pLiquidRayCastHelper, aBox );

	TASSERT( pPhysicsManager->GetCollisionTestFlags( m_pLiquidRayCastHelper ) == oRay.m_uiCollisionLayer, "pPM->GetCollisionTestFlags(m_pLiquidRayCastHelper) == oRayCast.m_iTestFlags" );

	m_oMotion.m_uiContactFlags &= 0xFFFC;
	TPhysicsObjectContainer* pRayBody = TNULL;

	TPhysicsShapeContact aRayContacts[ 4 ];
	TASSERT( m_pLiquidRayCastHelper, "phantomContainer" );
	TINT iNumRayContacts = pPhysicsManager->CollideRayAll( oRay, aRayContacts, 4, m_pLiquidRayCastHelper->IsInSimulation() ? m_pLiquidRayCastHelper : TNULL );

	if ( iNumRayContacts >= 1 )
	{
		// The surface hit last from above, if it's flat enough (a surface whose normal points up, not one the ray leaves
		// again below it)
		TINT                     iSurface  = -1;
		TBOOL                    bPrevUp   = TFALSE;
		TFLOAT                   fReach    = fBottom + fRadius;
		TFLOAT                   fTop      = a_rcPosition.y - s_vAxisY.y * fReach;
		TFLOAT                   fBottomY  = a_rcPosition.y + s_vAxisY.y * fReach;
		TPhysicsObjectContainer* pPrevBody = TNULL;

		for ( TINT i = 0; i < iNumRayContacts; i++ )
		{
			TPhysicsShapeContact&    rContact = aRayContacts[ i ];
			TPhysicsObjectContainer* pBody    = rContact.m_pBody;
			TBOOL                    bUp      = rContact.m_vSurfaceNormal.y * TVector4::VEC_NEGY.y + rContact.m_vSurfaceNormal.x * TVector4::VEC_NEGY.x + rContact.m_vSurfaceNormal.z * TVector4::VEC_NEGY.z > 0.0f;

			if ( bUp )
			{
				if ( !bPrevUp || pPrevBody != pBody )
				{
					if ( rContact.m_vPosition.y > fBottomY )
						break;

					if ( iSurface == -1 || rContact.m_vPosition.y - ms_pBlobProperties[ 9 ].m_fRadius * 2.0f > aRayContacts[ iSurface ].m_vPosition.y )
						iSurface = i;
				}
			}
			else if ( i == 0 )
			{
				if ( aRayContacts[ 0 ].m_vPosition.y > fBottomY )
					break;
			}
			else if ( ( bPrevUp || pPrevBody != pBody ) && fTop > rContact.m_vPosition.y )
			{
				iSurface = -1;
			}

			pPrevBody = pBody;
			bPrevUp   = bUp;
		}

		if ( iSurface != -1 )
		{
			TPhysicsShapeContact& rContact = aRayContacts[ iSurface ];

			if ( TMath::Abs( rContact.m_vSurfaceNormal.y * TVector4::VEC_NEGY.y + rContact.m_vSurfaceNormal.x * TVector4::VEC_NEGY.x + rContact.m_vSurfaceNormal.z * TVector4::VEC_NEGY.z ) > s_fLiquidSurfaceCos )
			{
				if ( rContact.m_uiMaterial & 0x10 )
					m_oMotion.m_uiContactFlags |= 1;

				if ( rContact.m_uiMaterial & 0x2000 )
					m_oMotion.m_uiContactFlags |= 2;

				m_oMotion.m_vLiquidHit    = rContact.m_vPosition;
				m_oMotion.m_vLiquidNormal = rContact.m_vSurfaceNormal;
				pRayBody                  = rContact.m_pBody;
			}
		}
	}

	// The soft collision query: a sphere 0.25 larger than the ball
	TSphereData oSphere;
	oSphere.SetRadius( fRadius + 0.25f );

	TMatrix44 oMatrix = TMatrix44::IDENTITY;
	oMatrix.SetTranslation( m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );

	TASSERT( pPhysicsManager->GetCollisionTestFlags( m_pSoftCollisionQueryHelper ) == 0xE3, "pPM->GetCollisionTestFlags(m_pSoftCollisionQueryHelper) == iTestFlags" );
	TASSERT( m_pSoftCollisionQueryHelper, "phantomContainer" );

	TPhysicsShapeContact aContacts[ 4 ];
	TINT                 iNumContacts = pPhysicsManager->CollideShape( &oSphere, oMatrix, 0xE3, aContacts, 4, m_pSoftCollisionQueryHelper->IsInSimulation() ? m_pSoftCollisionQueryHelper : TNULL );


	for ( TINT i = 0; i < iNumContacts; i++ )
	{
		TPhysicsShapeContact&    rContact = aContacts[ i ];
		TPhysicsObjectContainer* pBody    = rContact.m_pBody;
		TUINT32                  uiLayer  = pBody ? ( pBody->m_uiCollisionFilterInfo & 0xfff ) : 0;

		// Stops at the first body that isn't a liquid
		if ( !( uiLayer & 0x20 ) )
			break;

		TUINT32 uiMaterial = rContact.m_uiMaterial;

		// A liquid wall (steeper than 15 degrees)
		if ( ( uiMaterial & 0x2010 ) && s_fLiquidSurfaceCos > TMath::Abs( rContact.m_vSurfaceNormal.y * TVector4::VEC_NEGY.y + rContact.m_vSurfaceNormal.x * TVector4::VEC_NEGY.x + TVector4::VEC_NEGY.z * rContact.m_vSurfaceNormal.z ) )
		{
			if ( m_oMotion.m_uiContactFlags & 3 )
			{
				// Already in a liquid: only a closer contact of the same body
				// NOTE: retail compares with the ray's body, which is uninitialised when the ray chose no surface
				if ( pBody != pRayBody )
					break;

				TVector4 vToHit( a_rcPosition.x - m_oMotion.m_vLiquidHit.x, a_rcPosition.y - m_oMotion.m_vLiquidHit.y, a_rcPosition.z - m_oMotion.m_vLiquidHit.z, a_rcPosition.w );
				TVector4 vToContact( a_rcPosition.x - rContact.m_vPosition.x, a_rcPosition.y - rContact.m_vPosition.y, a_rcPosition.z - rContact.m_vPosition.z, a_rcPosition.w );

				if ( !( vToContact.MagnitudeSq() < vToHit.MagnitudeSq() ) )
					break;
			}

			m_oMotion.m_vLiquidHit    = rContact.m_vPosition;
			m_oMotion.m_vLiquidNormal = rContact.m_vSurfaceNormal;

			if ( uiMaterial & 0x10 )
				m_oMotion.m_uiContactFlags |= 1;
			else
				m_oMotion.m_uiContactFlags |= 2;

			break;
		}
	}
}

// $deBlob: FUNCTION 0074ca20
void ABlobEntity::UpdateStateTransitions( TFLOAT a_fTimeStep )
{
	// The landmark time (0x3C4): 10 paint points at each full second, and its effect follows the ball
	TFLOAT& rfLandmarkTime = *TREINTERPRETCAST( TFLOAT*, &m_uiLandmarkTime );

	if ( 0.0f < rfLandmarkTime )
	{
		if ( TFLOAT( fmod( TDOUBLE( rfLandmarkTime ), 1.0 ) ) - a_fTimeStep < 0.0f )
			RemovePaintPoints( 10, TFALSE );

		rfLandmarkTime = rfLandmarkTime - a_fTimeStep;

		TParticleEffectInstance*& rpLandmark = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 5 ] ); // 0xAEC

		if ( rfLandmarkTime < 0.0f )
		{
			rfLandmarkTime = 0.0f;

			if ( rpLandmark != TNULL )
			{
				rpLandmark->StopLooping();
				rpLandmark = TNULL;
			}
		}
		else
		{
			if ( rpLandmark == TNULL )
			{
				AParticleEffectHelper::SetColour( ms_pLandmarkEffect, m_pColourComponent->m_uiFadeColour2, TNULL );
				rpLandmark = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pLandmarkEffect );
			}

			if ( rpLandmark != TNULL )
				rpLandmark->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
		}
	}

	TFLOAT& rfStateTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 0 ] );
	TBOOL   bOnGround   = ( m_oMotion.m_uiContactFlags & 8 ) != 0 && ( m_oMotion.m_uiContactFlags & 0x20 ) != 0;

	switch ( m_eState )
	{
		case STATE_ROLL:
			if ( ( m_oMotion.m_uiContactFlags & 8 ) != 0 )
			{
				rfStateTime = 0.0f;
			}
			else
			{
				rfStateTime += a_fTimeStep;

				// In the air for a while
				if ( 0.1f < rfStateTime )
				{
					SetState( STATE_BALLISTIC );
					break;
				}
			}

			goto LABEL_18;
		case STATE_BALLISTIC:
		{
			rfStateTime += a_fTimeStep;

			if ( ( m_uiFlags & 4 ) != 0 )
			{
				TINT iPrevZJumpState = GetZJumpState();
				m_pZJump->OnLanded();

				if ( m_eState == STATE_LMARK || m_eState == STATE_BOSS )
					break;

				m_pAnimationComponent->OnLanded();
				TINT iZJumpState = GetZJumpState();

				if ( ( iZJumpState == ABlobZJumpComponent::STATE_ATTACKSLAM || iZJumpState == ABlobZJumpComponent::STATE_NONE ) && iPrevZJumpState == 9 )
				{
					TVector4 vPosition;
					vPosition.Multiply( m_oMotion.m_vContactNormal, 0.5f );
					vPosition.x += m_oPhysicsMatrix.m_f41;
					vPosition.y += m_oPhysicsMatrix.m_f42;
					vPosition.z += m_oPhysicsMatrix.m_f43;
					SetPhysicsPosition( vPosition );

					if ( m_eState == STATE_BALLISTIC )
						m_uiButtons |= 0x20;

					break;
				}

				if ( iZJumpState == ABlobZJumpComponent::STATE_AIM )
				{
					TPhysicsManager::StopBody( m_pBody );
					SetState( STATE_STUCKWALL );
					break;
				}

				if ( iZJumpState == ABlobZJumpComponent::STATE_SETEND )
				{
					if ( iPrevZJumpState == 2 )
					{
						TPhysicsManager::StopBody( m_pBody );
						m_uiFlags &= ~0x800000;
						m_vClimbStart = TVector4::VEC_ZERO;
					}

					break;
				}

				if ( iZJumpState == ABlobZJumpComponent::STATE_TIMEOUT )
					break;
			}

			// Landed
			if ( 0.05f <= rfStateTime && bOnGround )
			{
				SetState( STATE_ROLL );
				break;
			}

			TINT iZJumpState = GetZJumpState();

			if ( iZJumpState == ABlobZJumpComponent::STATE_ATTACKFLIGHT || iZJumpState == ABlobZJumpComponent::STATE_LMARKFLIGHT )
			{
				if ( iZJumpState == ABlobZJumpComponent::STATE_ATTACKFLIGHT )
				{
					if ( UpdateZJumpFlight( a_fTimeStep ) )
					{
						ABlobZJumpComponent::PriorityTarget* pLock = m_pZJump->m_pCurrentPriorityLock;

						if ( pLock != TNULL && pLock->m_ePriorityType == 3 )
						{
							// The Z-jump hits the target
							m_pZJump->EndAttack( TFALSE );

							ABaseEntity*          pTarget = TSTATICCAST( ABaseEntity, pLock->m_pTarget );
							static const TVector4 s_vUp( 0.0f, 1.0f, 0.0f, 1.0f );
							AAttackMessage        oAttack( AAttackMessage::ATTACK_PlayerSlamDirect, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), m_iSize, 4, s_vUp, TFALSE );
							AEntityIOPacket       oPacket;
							oPacket.m_eMessage = AEntityIOPacket::MESSAGE_SLAM;
							oPacket.m_iData    = TINT( &oAttack );
							pTarget->HandleEntityIO( this, oPacket );
						}
						else
						{
							m_pZJump->EndAttack( TTRUE );
						}
					}
				}
				else if ( UpdateZJumpFlight( a_fTimeStep ) )
				{
					m_pZJump->EndAttack( TFALSE );
				}
				goto LABEL_45;
			}

			// Stick to the wall it hit
			if ( *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 1 ] ) > 0.0f || ( m_uiFlags & 4 ) == 0 || ( m_uiFlags & 0x10000 ) != 0 ||
			     m_iLeechbotsAttached != 0 || ( m_oMotion.m_uiContactFlags & 4 ) == 0 || *TREINTERPRETCAST( const TUINT32*, &ms_pBlobCommonProperties->m_afUnk14[ 7 ] ) == 0 ||
			     HasWallContactFlag( 0x302C000 ) )
				goto LABEL_45;

			{
				TPhysicsObjectContainer* pWall = TREINTERPRETCAST( TPhysicsObjectContainer*, m_oMotion.m_auiUnk280[ 4 ] ); // 0x310

				if ( pWall == TNULL || ( TPhysicsManager::GetCollisionLayer( pWall ) & 0x40 ) != 0 )
					goto LABEL_45;

				TBOOL bStick    = TTRUE;
				TINT* pUserData = TREINTERPRETCAST( TINT*, pWall->m_pUserData );

				if ( pUserData != TNULL )
				{
					if ( pUserData[ 0 ] == 2 )
						bStick = TREINTERPRETCAST( ABaseEntity*, pUserData[ 1 ] )->CanBlobStick();
					else if ( pUserData[ 0 ] == 3 )
						bStick = ( *TREINTERPRETCAST( TUINT8*, pUserData[ 1 ] + 4 ) & 7 ) != 3;
				}

				if ( ( ( m_uiFlags & 0x800 ) != 0 && m_vJumpStart.y > m_oPhysicsMatrix.m_f42 ) || !bStick )
					goto LABEL_45;

				TVector4 vVelocity;
				TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );

				if ( vVelocity.y >= 0.0f )
					vVelocity.y = 0.0f;

				TPhysicsManager::SetAngularVelocity( m_pBody, TVector4::VEC_ZERO );
				TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );
				rfStateTime = 0.0f;
				SetState( STATE_STUCKWALL );
				break;
			}

		LABEL_45:
			if ( CheckLiquid() )
			{
				m_pAnimationComponent->OnLanded();
				SetState( STATE_FLOAT );
			}

			break;
		}
		case STATE_STUCKWALL:
		{
			TINT iZJumpState = GetZJumpState();

			if ( iZJumpState == ABlobZJumpComponent::STATE_AIM || iZJumpState == ABlobZJumpComponent::STATE_AIMNODE )
			{
				if ( ( m_uiButtons & ABlobInputMap::BUTTON_BRAKE ) != 0 )
					InkBounce();

				break;
			}

			if ( bOnGround )
			{
				SetState( STATE_ROLL );
				break;
			}

			if ( ( m_oMotion.m_uiContactFlags & 4 ) != 0 )
			{
				if ( ( m_uiButtons & ABlobInputMap::BUTTON_BRAKE ) == 0 && m_oMotion.m_vContactNormal.y <= s_fWallContactCos && !HasWallContactFlag( 0x3004000 ) )
					goto LABEL_18;

				InkBounce();
				break;
			}

			// Lost the wall: push off it a bit and fall
			rfStateTime += a_fTimeStep;

			if ( 0.05f < rfStateTime )
			{
				rfStateTime      = 0.0f;
				m_auiUnk564[ 4 ] = 0;

				TVector4 vPosition;
				vPosition.Multiply( m_oMotion.m_vWallNormal, 0.1f );
				vPosition.x += m_oPhysicsMatrix.m_f41;
				vPosition.y += m_oPhysicsMatrix.m_f42;
				vPosition.z += m_oPhysicsMatrix.m_f43;
				SetPhysicsPosition( vPosition );
				SetState( STATE_BALLISTIC );
			}

			break;
		}
		case STATE_HOTPLATE:
			if ( HasWallContactFlag( 0x10000 ) || HasGroundContactFlag( 0x10000 ) )
				break;

			if ( bOnGround )
				SetState( STATE_ROLL );
			else if ( ( m_oMotion.m_uiContactFlags & 4 ) != 0 )
				SetState( STATE_STUCKWALL );
			else
				SetState( STATE_BALLISTIC );

			break;
		case STATE_LMARK:
		case STATE_BOSS:
			if ( m_fUntargetableTime < 1.0f )
				m_fUntargetableTime = 1.0f;

			break;
		case STATE_FLOAT:
		{
			// In water or ink
			rfStateTime += a_fTimeStep;

			if ( ( m_uiFlags & 0xC ) != 0 )
				m_uiFlags &= 0xFFFFEFFF;

			// Not near a flat surface: may jump out again
			const TVector4& rNormal = m_oMotion.m_vLiquidNormal;

			if ( s_fLiquidSurfaceCos > rNormal.y * TVector4::VEC_NEGY.y + rNormal.x * TVector4::VEC_NEGY.x + rNormal.z * TVector4::VEC_NEGY.z )
				m_uiFlags &= ~0x1000;

			TUINT16 uiLiquid = m_oMotion.m_uiContactFlags;
			m_uiFlags &= 0xFFFFFE7F;

			if ( uiLiquid & 1 )
				m_uiFlags |= 0x80;

			if ( uiLiquid & 2 )
				m_uiFlags |= 0x100;

			// Out of it: rolls on (after 0.05 s on the ground) or falls
			if ( ( m_uiFlags & 0x180 ) == 0 )
			{
				// NOTE: retail calls an empty function of m_pSoundComponent first
				m_pAnimationComponent->SetIdleAnimDelay();
				SetState( ( ( uiLiquid & 8 ) != 0 && ( uiLiquid & 0x20 ) != 0 && rfStateTime >= 0.05f ) ? STATE_ROLL : STATE_BALLISTIC );
			}

			break;
		}
		default:
			break;

		LABEL_18:
			if ( CheckLiquid() )
				SetState( STATE_FLOAT );

			break;
	}

	// The paint meter: the size and the colour (the landmark's paint counter while painting one)
	if ( m_pSizeHud == TNULL )
		return;

	m_pSizeHud->SetSize( m_iPaintPoints );

	// Multiplayer game type 1 while no blob has painted yet: the evil blob keeps its ink colour
	if ( m_pColourComponent->GetCurrentColour() == ABlobColourComponent::COLOUR_WHITE &&
	     AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 && !APlayerManager::GetSingleton()->IsNoBlobPainted() )
		return;

	// 0x5BC, capped by LevelData::m_fTimeLeft outside free paint
	const TFLOAT* pPaintLeft = TREINTERPRETCAST( const TFLOAT*, &m_auiUnk5A8[ 5 ] );

	if ( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint )
	{
		const TFLOAT* pLevelTime = &AGameStatus::GetSingleton()->GetLevelData().m_fTimeLeft;

		if ( *pPaintLeft > *pLevelTime )
			pPaintLeft = pLevelTime;
	}

	TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
		uiColour = ABlobColourComponent::COLOUR_BLACK;

	m_pSizeHud->SetColour( ABlobColourComponent::ms_auiColours[ uiColour ], ( m_uiFlags & 0x200000 ) != 0, *pPaintLeft );
}

// Jumps out of the water or ink (the jump button pressed): the velocity's direction kept, its speed scaled
// $deBlob: FUNCTION 0074a3d0
TBOOL ABlobEntity::TryLiquidJump( TFLOAT a_fTimeStep )
{
	if ( ( m_uiFlags & 0x1000 ) != 0 || ( m_uiButtons & ( m_uiButtons ^ m_uiPrevButtons ) & ABlobInputMap::BUTTON_JUMP ) == 0 )
		return TFALSE;

	TVector4 vVelocity;
	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );

	// Relative to the havok ground contact's body
	if ( ( m_oMotion.m_uiContactFlags & 0x20 ) != 0 )
	{
		vVelocity.x = vVelocity.x - m_oMotion.m_vUnk220.x;
		vVelocity.y = vVelocity.y - m_oMotion.m_vUnk220.y;
		vVelocity.z = vVelocity.z - m_oMotion.m_vUnk220.z;
	}

	TFLOAT fSpeed = vVelocity.NormaliseMag() * ms_pBlobCommonProperties->m_fJumpSpeed;

	if ( ( m_pColourComponent->GetCurrentColour() >> 4 ) & 1 )
		fSpeed = ms_pBlobCommonProperties->m_fInkedPenalty * fSpeed;

	vVelocity.Multiply( fSpeed );

	TFLOAT fJump = ( m_pColourComponent->GetCurrentColour() & 0x10 ) ? ms_pBlobCommonProperties->m_fInkedJumpVelocity : ms_pBlobCommonProperties->m_fJumpVelocity;
	vVelocity.y  = -fJump;
	TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );

	m_vJumpStart     = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	m_vJumpDirection = vVelocity;
	m_vJumpDirection.Normalise();

	*TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 1 ] ) = 0.05f;
	m_uiFlags                                       = ( m_uiFlags & 0xFF7FEFFF ) | 0x1000;

	m_pSoundComponent->PlayLiquidJump();
	m_pZJump->OnBlobJump( 0 );

	m_fStretchTimeLeft = ms_pBlobProperties[ m_iSize ].m_fStretchTime;
	return TTRUE;
}

// $deBlob: FUNCTION 0074a570
TBOOL ABlobEntity::TryLiquidZJump( TFLOAT a_fTimeStep )
{

	if ( ( m_uiFlags & 0x1000 ) != 0 || ( TUINT8( m_uiButtons ) & ( TUINT8( m_uiButtons ) ^ TUINT8( m_uiPrevButtons ) ) & ABlobInputMap::BUTTON_JUMP ) == 0 )
		return TFALSE;

	if ( !LaunchZJump( a_fTimeStep, TVector4::VEC_ZERO ) ) // X0Y0Z0W1
		return TFALSE;

	m_uiFlags |= 0x1000;
	return TTRUE;
}

// Floats the ball in the water or ink: a buoyancy that grows with the depth below the surface (bobbing with the level
// time near it) against a drag on the vertical speed
// $deBlob: FUNCTION 0074ad20
void ABlobEntity::UpdateLiquidBuoyancy()
{
	const TVector4& rNormal = m_oMotion.m_vLiquidNormal;

	// Only on a flat surface
	if ( s_fLiquidSurfaceCos > rNormal.y * TVector4::VEC_NEGY.y + rNormal.x * TVector4::VEC_NEGY.x + rNormal.z * TVector4::VEC_NEGY.z )
		return;

	// y points down: the depth of the ball's centre below the surface
	TFLOAT fDepth = ( m_oPhysicsMatrix.m_f42 - m_oMotion.m_vLiquidHit.y ) - 0.5f;

	if ( ( m_uiFlags & 0x1000 ) != 0 )
		return;

	TPhysicsManager* pPM = TPhysicsManager::GetSingleton();

	TVector4 vVelocity;
	pPM->GetLinearVelocity( m_pBody, vVelocity );

	TFLOAT fRadius = ms_pBlobProperties[ m_iSize ].m_fRadius;
	TFLOAT fDrag   = -( TMath::Sign( vVelocity.y ) * vVelocity.y * vVelocity.y * s_fLiquidDrag );

	TVector4 vForce( 0.0f, 0.0f, 0.0f, 1.0f );

	if ( fDepth <= fRadius )
	{
		if ( fDepth <= -fRadius )
			return;

		// Partly submerged: bobbing with the level time
		TFLOAT fTime = AGameStatus::GetSingleton()->GetLevelData().m_fTimeLeft;
		TFLOAT fSin  = TFLOAT( sin( double( 6.2831855f * 1.2f * fTime ) ) );

		vForce.y = ( ( fSin + 1.0f ) * 0.3f + ( ( fRadius + fDepth ) * s_fLiquidBuoyancy ) / fRadius ) * -80.0f + fDrag;
	}
	else
	{
		// Submerged: the full buoyancy, only the drag against sinking
		vForce.y = s_fLiquidBuoyancy * -80.0f + ( ( fDrag <= 0.0f ) ? fDrag : 0.0f );
	}

	pPM->ApplyForceAtPoint( m_pBody, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), vForce );
}

// Steers the ball in the water or ink with the move stick (pulled along by the havok ground contact's body)
// $deBlob: FUNCTION 0074bb70
void ABlobEntity::UpdateLiquidMovement( TFLOAT a_fTimeStep )
{
	TVector2 vStick = TVector2::VEC_ZERO;

	if ( m_iController >= 0 )
		vStick = ABlobInputMap::GetSingleton()->GetMoveStick( m_iController );

	TFLOAT           fStrength = GetStickResponse( vStick.Magnitude() ) * ms_pBlobCommonProperties->m_fLiquidStickStrength;
	TPhysicsManager* pPM       = TPhysicsManager::GetSingleton();

	TVector4 vForward;
	TVector4 vRight;
	GetMoveAxes( vForward, vRight );

	TVector4 vVelocity;
	pPM->GetLinearVelocity( m_pBody, vVelocity );

	TVector4 vRightPart;
	vRightPart.Multiply( vRight, vStick.x );
	TVector4 vForwardPart;
	vForwardPart.Multiply( vForward, vStick.y );

	TVector4 vDirection;
	vDirection.x = vRightPart.x + vForwardPart.x;
	vDirection.y = vRightPart.y + vForwardPart.y;
	vDirection.z = vRightPart.z + vForwardPart.z;
	vDirection.w = vForwardPart.w;

	const Properties& rProperties = ms_pBlobProperties[ m_iSize ];

	if ( ( m_oMotion.m_uiContactFlags & 0x20 ) == 0 || m_oMotion.m_vUnk220.MagnitudeSq() <= 0.0f )
	{
		ApplyRollDamping( a_fTimeStep, ms_pBlobCommonProperties->m_fRollDamping, vDirection, rProperties.m_fUnk34 * fStrength );
	}
	else
	{
		// Carried by a moving body: accelerate towards its velocity
		TVector4 vRelative;
		vRelative.x = m_oMotion.m_vUnk220.x - vVelocity.x;
		vRelative.y = m_oMotion.m_vUnk220.y - vVelocity.y;
		vRelative.z = m_oMotion.m_vUnk220.z - vVelocity.z;
		vRelative.w = m_oMotion.m_vUnk220.w;

		TVector4 vAcceleration;
		vAcceleration.Multiply( vRelative, 1.0f / a_fTimeStep );
		TVector4 vForce;
		vForce.Multiply( vAcceleration, s_fLiquidCarry );
		pPM->ApplyForceAtPoint( m_pBody, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), vForce );

		TVector4 vForceDirection = vForce;
		vForceDirection.Normalise();
		ApplyRollDamping( a_fTimeStep, ms_pBlobCommonProperties->m_fUnk78, vForceDirection, rProperties.m_fUnk34 * fStrength );
	}

	if ( vStick.x * vStick.x + vStick.y * vStick.y > 0.001f &&
	     rProperties.m_fUnk34 * fStrength > vVelocity.y * vDirection.y + vVelocity.x * vDirection.x + vVelocity.z * vDirection.z )
	{
		TVector4 vForce;
		vForce.Multiply( vForward, vStick.y * fStrength * ms_pBlobProperties[ m_iSize ].m_fForce );
		pPM->ApplyForce( m_pBody, vForce );

		vForce.Multiply( vRight, vStick.x * fStrength * ms_pBlobProperties[ m_iSize ].m_fForce );
		pPM->ApplyForce( m_pBody, vForce );
	}
}

// $deBlob: FUNCTION 0074af20
void ABlobEntity::UpdateRollMovement( TFLOAT a_fTimeStep )
{
	TVector2 vStick = TVector2::VEC_ZERO;

	if ( m_pAnimationComponent->m_iState != ABlobAnimationComponent::STATE_ZJUMPFAIL && m_iController >= 0 )
		vStick = ABlobInputMap::GetSingleton()->GetMoveStick( m_iController );

	TFLOAT fStrength = GetStickResponse( vStick.Magnitude() );
	TFLOAT fSpeed    = fStrength;

	// Slower on slippery ground
	if ( ( m_oMotion.m_uiContactFlags & 8 ) != 0 && ( GetContactFlags( TUINT8( m_oMotion.m_uiContactCountAndGround >> 8 ) ) & 0x3004000 ) != 0 )
		fSpeed = s_fSlipperySpeedFactor * fStrength;

	TPhysicsManager* pPM = TPhysicsManager::GetSingleton();

	TVector4 vForward;
	TVector4 vRight;
	GetMoveAxes( vForward, vRight );

	TVector4 vVelocity;
	pPM->GetLinearVelocity( m_pBody, vVelocity );

	TVector4 vRightPart;
	vRightPart.Multiply( vRight, vStick.x );
	TVector4 vForwardPart;
	vForwardPart.Multiply( vForward, vStick.y );

	TVector4 vDirection;
	vDirection.x = vForwardPart.x + vRightPart.x;
	vDirection.y = vForwardPart.y + vRightPart.y;
	vDirection.z = vForwardPart.z + vRightPart.z;
	vDirection.w = vForwardPart.w;

	TFLOAT fRightFactor = 1.0f;

	if ( ( m_oMotion.m_uiContactFlags & 0x20 ) == 0 || m_oMotion.m_vUnk220.MagnitudeSq() <= 0.0f )
	{
		ApplyRollDamping( a_fTimeStep, ms_pBlobCommonProperties->m_fRollDamping, vDirection, ms_pBlobProperties[ m_iSize ].m_fMaxSpeed * fSpeed );
		m_uiFlags &= 0xFFEFFFFF;
	}
	else
	{
		// On a moving object: pull the ball along with it
		TBOOL bMoving = 0.001f < vStick.x * vStick.x + vStick.y * vStick.y;

		TVector4 vRelative;
		pPM->GetLinearVelocity( m_pBody, vRelative );
		vRelative.x -= m_oMotion.m_vUnk220.x;
		vRelative.y -= m_oMotion.m_vUnk220.y;
		vRelative.z -= m_oMotion.m_vUnk220.z;

		TFLOAT fRelativeSq = vRelative.MagnitudeSqXZ();
		TFLOAT fFollow;

		if ( ( m_uiFlags & 0x100000 ) == 0 )
		{
			fFollow = 0.1f;

			if ( !bMoving && fRelativeSq < 0.001f )
				fFollow = 1.0f;
		}
		else
		{
			fFollow = 1.0f;

			if ( bMoving )
			{
				m_uiFlags &= 0xFFEFFFFF;
				fFollow = 0.1f;
			}
		}

		TVector4 vAcceleration;
		vAcceleration.Multiply( vRelative, 1.0f / a_fTimeStep );
		vAcceleration.Multiply( fFollow );
		vAcceleration.Multiply( -1.0f );
		vAcceleration.y = 0.0f;
		pPM->ApplyForceAtPoint( m_pBody, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), vAcceleration );

		if ( ( m_uiFlags & 0x100000 ) != 0 )
		{
			TVector4 vFollowDirection = vAcceleration;
			vFollowDirection.Normalise();
			ApplyRollDamping( a_fTimeStep, ms_pBlobCommonProperties->m_fRollDamping, vFollowDirection, ms_pBlobProperties[ m_iSize ].m_fMaxSpeed * fSpeed );
		}
	}

	if ( 0.001f < vStick.x * vStick.x + vStick.y * vStick.y &&
	     vVelocity.y * vDirection.y + vVelocity.x * vDirection.x + vVelocity.z * vDirection.z < ms_pBlobProperties[ m_iSize ].m_fMaxSpeed * fSpeed )
	{
		if ( ( m_pColourComponent->GetCurrentColour() >> 4 ) & 1 )
			fSpeed = ms_pBlobCommonProperties->m_fInkedPenalty * fSpeed;

		TVector4 vAngularVelocity;
		pPM->GetAngularVelocity( m_pBody, vAngularVelocity );

		const Properties& rProperties = ms_pBlobProperties[ m_iSize ];

		// Roll the ball (torque about the axes across the direction)
		if ( vAngularVelocity.Magnitude() < rProperties.m_fMaxAngularVelocity * fSpeed )
		{
			TVector4 vTorque;
			vTorque.Multiply( vRight, -( vStick.y * fSpeed * rProperties.m_fTorque ) );
			pPM->ApplyTorque( m_pBody, vTorque );

			vTorque.Multiply( vForward, vStick.x * fSpeed * rProperties.m_fTorque );
			pPM->ApplyTorque( m_pBody, vTorque );
		}

		// And push it on the ground
		if ( ( m_oMotion.m_uiContactFlags & 8 ) != 0 )
		{
			TUINT32 uiGroundFlags = GetContactFlags( TUINT8( m_oMotion.m_uiContactCountAndGround >> 8 ) );
			TFLOAT  fGround       = 1.0f;

			if ( uiGroundFlags & 0x2004000 )
				fGround = s_fSlipperyForceFactor;

			if ( ( m_oMotion.m_uiContactFlags & 8 ) != 0 && ( uiGroundFlags & 0x1000000 ) != 0 )
				fGround = s_fStickyForceFactor;

			TFLOAT fForwardFactor = 1.0f;

			if ( 0.5f < *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5A8[ 0 ] ) && *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5A8[ 1 ] ) <= 0.0f )
			{
				fRightFactor   = s_fAirForceFactor;
				fForwardFactor = s_fAirForceFactor;
			}

			TVector4 vForce;
			vForce.Multiply( vForward, vStick.y * fSpeed * fForwardFactor * rProperties.m_fForce );
			vForce.Multiply( fGround );
			pPM->ApplyForce( m_pBody, vForce );

			vForce.Multiply( vRight, vStick.x * fSpeed * fRightFactor * rProperties.m_fForce );
			vForce.Multiply( fGround );
			pPM->ApplyForce( m_pBody, vForce );
		}
	}
}

// $deBlob: FUNCTION 0074ab50
void ABlobEntity::ApplyRollDamping( TFLOAT a_fTimeStep, TFLOAT a_fDamping, const Toshi::TVector4& a_rDirection, TFLOAT a_fMaxSpeed )
{
	if ( ( m_oMotion.m_uiContactFlags & 8 ) != 0 && ( GetContactFlags( TUINT8( m_oMotion.m_uiContactCountAndGround >> 8 ) ) & 0x3004000 ) != 0 )
		return;

	TVector4 vVelocity;
	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );

	// Relative to the moving object the blob stands on
	if ( m_oMotion.m_uiContactFlags & 0x20 )
	{
		vVelocity.x -= m_oMotion.m_vUnk220.x;
		vVelocity.y -= m_oMotion.m_vUnk220.y;
		vVelocity.z -= m_oMotion.m_vUnk220.z;
	}

	// Keep the part along the direction (up to a_fMaxSpeed), damp the rest in XZ
	TFLOAT   fAlong = a_rDirection.y * vVelocity.y + a_rDirection.x * vVelocity.x + a_rDirection.z * vVelocity.z;
	TVector4 vAlong = TVector4::VEC_ZERO;

	if ( fAlong < a_fMaxSpeed )
	{
		vAlong.Multiply( a_rDirection, fAlong );
		vVelocity.y -= vAlong.y;
		vVelocity.x -= vAlong.x;
		vVelocity.z -= vAlong.z;
	}

	TFLOAT fKeep = 1.0f - a_fTimeStep * a_fDamping;
	vVelocity.z  = fKeep * vVelocity.z;
	vVelocity.x  = fKeep * vVelocity.x;

	if ( vVelocity.MagnitudeSqXZ() < 0.010000001f )
	{
		vVelocity.z = 0.0f;
		vVelocity.x = 0.0f;
	}

	vVelocity.x = vAlong.x + vVelocity.x;
	vVelocity.z = vAlong.z + vVelocity.z;
	vVelocity.y = vAlong.y + vVelocity.y;

	if ( m_oMotion.m_uiContactFlags & 0x20 )
	{
		vVelocity.x = vVelocity.x + m_oMotion.m_vUnk220.x;
		vVelocity.y = m_oMotion.m_vUnk220.y + vVelocity.y;
		vVelocity.z = m_oMotion.m_vUnk220.z + vVelocity.z;
	}

	TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );
}

// $deBlob: FUNCTION 007493b0
TFLOAT ABlobEntity::CalculateJumpVelocityAtSpeed( const TVector4& a_rvPos, const TVector4& a_rvTarget, TFLOAT a_fSpeed, TFLOAT a_fGravity, TVector4& a_rOutVelocity )
{
	TASSERT( !a_rvTarget.IsEqual( a_rvPos ), "a_rvTarget != a_rvPos" );

	a_rOutVelocity.x = a_rvTarget.x - a_rvPos.x;
	a_rOutVelocity.y = a_rvTarget.y - a_rvPos.y;
	a_rOutVelocity.z = a_rvTarget.z - a_rvPos.z;
	a_rOutVelocity.w = a_rvTarget.w;

	TFLOAT fDistance = a_rOutVelocity.Magnitude();
	TFLOAT fTime     = fDistance / a_fSpeed;
	TFLOAT fDistXZ   = a_rOutVelocity.MagnitudeSqXZ();

	if ( fDistXZ != 0.0f )
	{
		fDistXZ          = TMath::Sqrt( fDistXZ );
		a_rOutVelocity.y = 0.0f;
		a_rOutVelocity.x /= fDistXZ;
		a_rOutVelocity.z /= fDistXZ;
	}

	a_rOutVelocity.Multiply( ( fDistXZ / fDistance ) * a_fSpeed );
	a_rOutVelocity.y = ( a_rvTarget.y - a_rvPos.y ) / fTime - a_fGravity * 0.5f * fTime;

	return fTime;
}

// $deBlob: FUNCTION 00749950
TBOOL ABlobEntity::LaunchZJump( TFLOAT a_fTimeStep, const TVector4& a_rcOffset )
{
	TVector4 vStart;
	vStart.Multiply( a_rcOffset, 0.2f );
	vStart.x = m_oPhysicsMatrix.GetTranslation().x + vStart.x;
	vStart.y = m_oPhysicsMatrix.GetTranslation().y + vStart.y;
	vStart.z = m_oPhysicsMatrix.GetTranslation().z + vStart.z;

	TVector4 vTarget;
	m_pZJump->GetTargetPosition( vTarget );
	vTarget.y = vTarget.y - m_pZJump->GetTargetHeight();

	// NOTE: retail leaves the velocity uninitialised when the Z-jump has no target state (it sets it nowhere then); the
	// community patch zeroes it
#ifdef DEBLOB_COMMUNITY_PATCH
	TVector4 vVelocity( 0.0f, 0.0f, 0.0f, 0.0f );
	TFLOAT   fFlightTime = 0.0f;
#else
	TVector4 vVelocity;
	TFLOAT   fFlightTime;
#endif

	switch ( m_pZJump->GetState() )
	{
		case ABlobZJumpComponent::STATE_POINT:
		case ABlobZJumpComponent::STATE_AIMNODE:
		case ABlobZJumpComponent::STATE_LOCKLMARK:
			if ( vStart.IsEqual( vTarget ) )
				return TTRUE;

			fFlightTime = CalculateJumpVelocityAtSpeed( vStart, vTarget, ms_pBlobCommonProperties->m_afUnk38[ 3 ], 80.0f, vVelocity );
			m_pSoundComponent->PlayZJumpLaunch( fFlightTime );
			break;
		case ABlobZJumpComponent::STATE_LOCK:
		{
			ABlobZJumpComponent::PriorityTarget* pLock = m_pZJump->GetCurrentPriorityLock();

			if ( pLock == TNULL )
				return TFALSE;

			TBOOL bArc = TFALSE;

			if ( pLock->m_eType == 0 )
			{
				ABaseEntity* pTarget = TSTATICCAST( ABaseEntity, pLock->m_pTarget );

				if ( pTarget->IsA( &TGetClass( APropSwitchPressure ) ) )
				{
					const TVector4& rcUp = pTarget->GetMatrix().AsBasisVector4( BASISVECTOR_UP );

					if ( s_vAxisY.x * rcUp.x + s_vAxisY.y * rcUp.y + s_vAxisY.z * rcUp.z <= 0.8f )
					{
						if ( vStart.IsEqual( vTarget ) )
							return TTRUE;
					}
					else
						bArc = TTRUE;
				}
			}

			if ( bArc )
			{
				TFLOAT fHeight     = vTarget.y - vStart.y;
				TFLOAT fAngle      = TMath::ATan2( TMath::Abs( fHeight ), TVector4::DistanceXZ( vTarget, vStart ) );
				TFLOAT fTrajectory = TMath::LERP( fAngle, TMath::HALF_PI, 0.2f );
				fFlightTime        = CalculateJumpVelocity( fTrajectory, vStart, vTarget, 80.0f, vVelocity );
			}
			else
				fFlightTime = CalculateJumpVelocityAtSpeed( vStart, vTarget, ms_pBlobCommonProperties->m_afUnk38[ 3 ], 80.0f, vVelocity );

			m_pSoundComponent->PlayLiquidJump();
			break;
		}
	}

	if ( !m_pZJump->OnBlobJump( fFlightTime ) )
		return TFALSE;

	TPhysicsManager* pPM = TPhysicsManager::GetSingleton();

	SetPhysicsPosition( vStart );

	TVector4 vAngularVelocity;
	pPM->GetAngularVelocity( m_pBody, vAngularVelocity );
	pPM->RemoveFromSimulation( m_pBody, TTRUE );
	TASSERT( STATE_DEAD != m_eState );
	pPM->AddToSimulation( m_pBody, TTRUE );
	TPhysicsManager::SetAngularVelocity( m_pBody, vAngularVelocity );
	TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );

	m_vJumpStart     = vStart;
	m_vJumpDirection = vVelocity;
	m_vJumpDirection.Normalise();

	m_uiFlags &= ~0x800000;
	m_fStretchTimeLeft                              = ms_pBlobProperties[ m_iSize ].m_fStretchTime;
	*TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 1 ] ) = 0.05f;

	return TTRUE;
}

// $deBlob: FUNCTION 00749cd0
TBOOL ABlobEntity::TryZJumpFromGround( TFLOAT a_fTimeStep )
{
	if ( ( m_oMotion.m_uiContactFlags & 8 ) == 0 || HasGroundContactFlag( 0x2004000 ) )
		return TFALSE;

	if ( 0.0f < *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 1 ] ) || m_iLeechbotsAttached != 0 )
		return TFALSE;

	if ( ( TUINT8( m_uiButtons ) & ( TUINT8( m_uiButtons ) ^ TUINT8( m_uiPrevButtons ) ) & ABlobInputMap::BUTTON_JUMP ) == 0 || !LaunchZJump( a_fTimeStep, m_oMotion.m_vContactNormal ) )
		return TFALSE;

	SetState( STATE_BALLISTIC );
	return TTRUE;
}

// $deBlob: FUNCTION 0074a370
TBOOL ABlobEntity::TryZJumpFromAir( TFLOAT a_fTimeStep )
{
	if ( m_iLeechbotsAttached != 0 )
		return TFALSE;

	if ( ( TUINT8( m_uiButtons ) & ( TUINT8( m_uiButtons ) ^ TUINT8( m_uiPrevButtons ) ) & ABlobInputMap::BUTTON_JUMP ) == 0 || !LaunchZJump( a_fTimeStep, TVector4::VEC_ZERO ) ) // X0Y0Z0W1
		return TFALSE;

	SetState( STATE_BALLISTIC );
	return TTRUE;
}

// $deBlob: FUNCTION 00748ef0
TBOOL ABlobEntity::UpdateZPlayerAttack( TFLOAT a_fTimeStep )
{
	ABlobZJumpComponent::PriorityTarget* pTarget = m_pZJump->m_pCurrentPriorityLock;
	TASSERT( pTarget->m_eType == ABlobZJumpComponent::PriorityTarget::TARGET_ENTITY );

	ABaseEntity* pEntTarget = TSTATICCAST( ABaseEntity, pTarget->m_pTarget );
	TASSERT( pEntTarget->GetFlags() & FLAG_Player );
	TASSERT( pEntTarget->IsExactly( &TGetClass( ABlobEntity ) ) );
	TASSERT( m_iZPlayerAttackStage >= Z_ATTACKSTAGE_NONE && m_iZPlayerAttackStage < Z_ATTACKSTAGE_COUNT );

	if ( m_iZPlayerAttackStage == Z_ATTACKSTAGE_NONE )
	{
		m_iZPlayerAttackStage = Z_ATTACKSTAGE_OVER;
		m_fZPlayerAttackTime  = 0.0f;
	}
	else
	{
		m_fZPlayerAttackTime += a_fTimeStep;
	}

	ABlobZJumpComponent* pZJump     = m_pZJump;
	TVector4             vVelocity  = TVector4::VEC_ZERO; // X0Y0Z0W1
	TVector4             vTargetPos = pEntTarget->GetTranslation();
	const TVector4&      rcPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	if ( m_iZPlayerAttackStage == Z_ATTACKSTAGE_OVER )
	{
		TFLOAT fTop = TMath::Min( pZJump->m_vUnk124.y, pZJump->m_vUnk144.y );

		TVector4 vOver( pZJump->m_vUnk144.x + pZJump->m_vUnk124.x, pZJump->m_vUnk124.y + pZJump->m_vUnk144.y, pZJump->m_vUnk144.z + pZJump->m_vUnk124.z, pZJump->m_vUnk144.w );
		vOver.Multiply( 0.5f );
		vOver.y = fTop - 8.0f;

		pZJump->m_vUnk134 = vTargetPos;

		if ( TVector4::DistanceSq( vOver, rcPosition ) >= 0.5f && m_fZPlayerAttackTime <= 0.5f )
		{
			CalculateJumpVelocityAtSpeed( rcPosition, vOver, ms_pBlobCommonProperties->m_afUnk38[ 3 ], 80.0f, vVelocity );
			TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );
			m_vJumpDirection = vVelocity;
			m_vJumpDirection.Normalise();
			return TFALSE;
		}

		m_iZPlayerAttackStage = Z_ATTACKSTAGE_TOWARDS;
		m_fZPlayerAttackTime  = 0.0f;
	}

	TBOOL bFlyAtTarget = TFALSE;

	if ( m_iZPlayerAttackStage == Z_ATTACKSTAGE_TOWARDS )
	{
		pZJump->m_vUnk134 = vTargetPos;

		if ( m_fZPlayerAttackTime <= 0.5f && TVector4::DistanceSq( pZJump->m_vUnk134, rcPosition ) >= 0.5f )
		{
			bFlyAtTarget = TTRUE;
		}
		else
		{
			m_iZPlayerAttackStage = Z_ATTACKSTAGE_ONTO;
			m_fZPlayerAttackTime  = 0.0f;
		}
	}

	if ( !bFlyAtTarget && m_iZPlayerAttackStage == Z_ATTACKSTAGE_ONTO )
	{
		if ( m_fZPlayerAttackTime <= 0.5f && TVector4::DistanceSq( pZJump->m_vUnk134, rcPosition ) >= 0.5f )
		{
			bFlyAtTarget = TTRUE;
		}
		else
		{
			m_iZPlayerAttackStage = Z_ATTACKSTAGE_THROUGH;
			m_fZPlayerAttackTime  = 0.0f;
		}
	}

	if ( bFlyAtTarget )
	{
		vVelocity.Set( pZJump->m_vUnk134.x - rcPosition.x, pZJump->m_vUnk134.y - rcPosition.y, pZJump->m_vUnk134.z - rcPosition.z, pZJump->m_vUnk134.w );
		vVelocity.Normalise();
		m_vJumpDirection = vVelocity;
		vVelocity.Multiply( ms_pBlobCommonProperties->m_afUnk38[ 3 ] );
		TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );
		return TFALSE;
	}

	if ( m_iZPlayerAttackStage == Z_ATTACKSTAGE_THROUGH )
	{
		vVelocity.Set( pZJump->m_vUnk134.x - rcPosition.x, pZJump->m_vUnk134.y - rcPosition.y, pZJump->m_vUnk134.z - rcPosition.z, pZJump->m_vUnk134.w );
		vVelocity.Normalise();
		m_vJumpDirection = vVelocity;
		vVelocity.Multiply( 30.0f );
		TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );
		m_iZPlayerAttackStage = Z_ATTACKSTAGE_NONE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0074a690
TBOOL ABlobEntity::UpdateZJumpFlight( TFLOAT a_fTimeStep )
{
	ABlobZJumpComponent::PriorityTarget* pLock = m_pZJump->m_pCurrentPriorityLock;

	if ( pLock == TNULL )
		return TTRUE;

	if ( pLock->m_eType == 0 )
	{
		ABaseEntity* pTarget = TSTATICCAST( ABaseEntity, pLock->m_pTarget );

		if ( TUINT8( pTarget->GetFlags() >> 4 ) & 1 )
		{
			// Another player's blob
			TASSERT( pTarget->IsExactly( &TGetClass( ABlobEntity ) ) );
			return UpdateZPlayerAttack( a_fTimeStep );
		}

		if ( TUINT8( pTarget->GetFlags() >> 22 ) & 1 )
			return TTRUE;
	}

	TVector4 vFrom = m_vJumpStart;

	TVector4 vTarget;
	m_pZJump->GetTargetPosition( vTarget );
	vTarget.y -= m_pZJump->GetTargetHeight();

	const TVector4& rcPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	if ( TVector4::DistanceSq( rcPosition, vTarget ) < 0.5f )
		return TTRUE;

	TFLOAT fTime       = m_pZJump->m_fUnk154;
	TFLOAT fFlightTime = m_pZJump->m_fFlightTime;

	TSphereData oSphere;
	oSphere.SetRadius( ms_pBlobProperties[ m_iSize ].m_fRadius );

	TPhysicsShapeContact oContact;

	if ( fTime >= fFlightTime || fFlightTime < 1e-05f )
	{
		// The flight is over: straight onto the target, unless something is in the way
		if ( TPhysicsManager::GetSingleton()->SweepShapeClosest( &oSphere, m_oPhysicsMatrix, vTarget, 0x83, oContact, TNULL ) )
		{
			TVector4 vDelta( vTarget.x - rcPosition.x, vTarget.y - rcPosition.y, vTarget.z - rcPosition.z, vTarget.w );
			vDelta.Multiply( oContact.m_fDistance );
			vTarget = TVector4( rcPosition.x + vDelta.x, rcPosition.y + vDelta.y, rcPosition.z + vDelta.z, rcPosition.w );
		}

		SetPhysicsPosition( vTarget );
		return TTRUE;
	}

	TFLOAT fRatio = fTime / fFlightTime;

	// The target moved (or the ball bounced): a new arc from the start
	if ( TVector4::DistanceSq( m_pZJump->m_vUnk134, vTarget ) > 0.25f || ( m_uiFlags & 4 ) )
	{
		TVector4 vVelocity;
		TFLOAT   fArcTime = CalculateJumpVelocityAtSpeed( vFrom, vTarget, ms_pBlobCommonProperties->m_afUnk38[ 3 ], 80.0f, vVelocity );

		TVector4 vPosition( vTarget.x - vFrom.x, 0.0f, vTarget.z - vFrom.z, vTarget.w );
		vPosition.Multiply( fRatio );
		vPosition.x += vFrom.x;

		TFLOAT fElapsed = fArcTime * fRatio;
		vPosition.z += vFrom.z;
		vPosition.y = fArcTime * fRatio * 40.0f * fElapsed + vVelocity.y * fElapsed + vFrom.y;

		if ( TPhysicsManager::GetSingleton()->SweepShapeClosest( &oSphere, m_oPhysicsMatrix, vPosition, 0x83, oContact, TNULL ) )
		{
			TVector4 vDelta( vPosition.x - rcPosition.x, vPosition.y - rcPosition.y, vPosition.z - rcPosition.z, vPosition.w );
			vDelta.Multiply( oContact.m_fDistance );
			vPosition = TVector4( vDelta.x + rcPosition.x, rcPosition.y + vDelta.y, rcPosition.z + vDelta.z, rcPosition.w );
			fElapsed  = oContact.m_fDistance * fElapsed;
		}

		SetPhysicsPosition( vPosition );

		m_vJumpDirection = vVelocity;
		m_vJumpDirection.Normalise();

		vVelocity.y = fElapsed * 80.0f + vVelocity.y;
		TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );
		m_pZJump->m_vUnk134 = vTarget;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007494f0
TFLOAT ABlobEntity::CalculateJumpVelocity( TFLOAT a_fTrajectory, const TVector4& a_rvPos, const TVector4& a_rvTarget, TFLOAT a_fGravity, TVector4& a_rOutVelocity )
{
	TASSERT( !( a_rvTarget == a_rvPos ), "a_rvTarget != a_rvPos" );
	TASSERT( a_fTrajectory > 0.0f && a_fTrajectory < TMath::HALF_PI );

	a_rOutVelocity.x = a_rvTarget.x - a_rvPos.x;
	a_rOutVelocity.y = a_rvTarget.y - a_rvPos.y;
	a_rOutVelocity.z = a_rvTarget.z - a_rvPos.z;
	a_rOutVelocity.w = a_rvTarget.w;

	// The world's up is -Y
	TFLOAT fHeight   = -a_rOutVelocity.y;
	TFLOAT fDistance = a_rOutVelocity.NormaliseXZ();

	TFLOAT fSin, fCos;
	TMath::SinCos( a_fTrajectory, fSin, fCos );

	TFLOAT fSpeedSq = ( fDistance * a_fGravity * fDistance * 0.5f ) / ( ( ( fSin / fCos ) * fDistance - fHeight ) * fCos * fCos );
	TASSERT( fSpeedSq > 0.0f );

	TFLOAT fSpeed = TMath::Sqrt( fSpeedSq );

	a_rOutVelocity.Multiply( fCos );
	a_rOutVelocity.y = -fSin;
	a_rOutVelocity.Multiply( fSpeed );

	// The time of the flight
	return fDistance / ( fSpeed * fCos );
}

// $deBlob: FUNCTION 007496e0
void ABlobEntity::UpdateJump( TFLOAT a_fTimeStep )
{
	TFLOAT fUnk5A8 = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5A8[ 0 ] );
	TFLOAT fUnk5AC = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5A8[ 1 ] );

	// Just left the ground (a late jump)
	TBOOL bLate = 0.4f < fUnk5A8 && fUnk5AC <= 0.0f;

	TBOOL bSlippery = ( m_oMotion.m_uiContactFlags & 8 ) != 0 && ( GetContactFlags( TUINT8( m_oMotion.m_uiContactCountAndGround >> 8 ) ) & 0x2004000 ) != 0;

	if ( !bLate || ( ( m_eState != STATE_ROLL || bSlippery ) && m_eState != STATE_BALLISTIC ) )
		bLate = TFALSE;

	if ( ( ( ( m_oMotion.m_uiContactFlags & 8 ) != 0 && !bSlippery ) || bLate ) &&
	     *TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 1 ] ) <= 0.0f && m_iLeechbotsAttached == 0 &&
	     m_pAnimationComponent->m_iState != ABlobAnimationComponent::STATE_ZJUMPFAIL &&
	     ( ( m_uiPrevButtons ^ m_uiButtons ) & m_uiButtons & ABlobInputMap::BUTTON_JUMP ) != 0 )
	{
		TPhysicsManager* pPM = TPhysicsManager::GetSingleton();

		TVector4 vVelocity;
		pPM->GetLinearVelocity( m_pBody, vVelocity );

		TFLOAT fSpeed = vVelocity.NormaliseMag() * ms_pBlobCommonProperties->m_fJumpSpeed;

		if ( fSpeed < 0.05f )
		{
			vVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
			fSpeed    = 0.0f;
		}

		if ( ( m_pColourComponent->GetCurrentColour() >> 4 ) & 1 )
			fSpeed = ms_pBlobCommonProperties->m_fInkedPenalty * fSpeed;

		// Lift the body off the ground a bit
		TVector4 vPosition = m_pBody->m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION );
		vPosition.y -= 0.2f;
		SetPhysicsPosition( vPosition );

		vVelocity.Multiply( fSpeed );

		TFLOAT fJump = ( ( m_pColourComponent->GetCurrentColour() >> 4 ) & 1 ) ? ms_pBlobCommonProperties->m_fInkedJumpVelocity : ms_pBlobCommonProperties->m_fJumpVelocity;

		vVelocity.y = s_fJumpKeepVelocityY * vVelocity.y - fJump;
		TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );

		m_vJumpStart     = vPosition;
		m_vJumpDirection = vVelocity;
		m_vJumpDirection.Normalise();
		m_uiFlags &= 0xFF7FFFFF;

		m_pSoundComponent->PlayLiquidJump();
		m_pZJump->OnBlobJump( 0 );

		if ( m_pAnimationComponent->m_iState == ABlobAnimationComponent::STATE_BRAKE )
			m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_ANIMTOBALL );

		m_fStretchTimeLeft                              = ms_pBlobProperties[ m_iSize ].m_fStretchTime;
		*TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 1 ] ) = 0.05f;
		SetState( STATE_BALLISTIC );
	}
}

// $deBlob: FUNCTION 00749d60
TBOOL ABlobEntity::TryZJumpFromWall( TFLOAT a_fTimeStep )
{
	if ( ( TUINT8( m_uiButtons ) & ( TUINT8( m_uiButtons ) ^ TUINT8( m_uiPrevButtons ) ) & ABlobInputMap::BUTTON_JUMP ) == 0 || !LaunchZJump( a_fTimeStep, m_oMotion.m_vWallNormal ) )
		return TFALSE;

	SetState( STATE_BALLISTIC );
	return TTRUE;
}

// $deBlob: FUNCTION 00749db0
TBOOL ABlobEntity::TryWallJump( TFLOAT a_fTimeStep )
{
	const TVector4& rWallNormal = m_oMotion.m_vWallNormal;

	if ( s_fWallContactCos < rWallNormal.y )
		return TFALSE;

	if ( ( ( m_uiPrevButtons ^ m_uiButtons ) & m_uiButtons & ABlobInputMap::BUTTON_JUMP ) == 0 )
		return TFALSE;

	TPhysicsManager* pPM = TPhysicsManager::GetSingleton();

	TVector2 vStick = TVector2::VEC_ZERO;

	if ( m_iController >= 0 )
		vStick = ABlobInputMap::GetSingleton()->GetMoveStick( m_iController );

	TBOOL bUnk = s_bUnk984E88 || TREINTERPRETCAST( const TINT16*, &ms_pBlobCommonProperties->m_afUnk14[ 0 ] )[ 1 ] != 0;

	// The direction along the wall to jump to (none: straight off the wall)
	TVector4 vAlong;

	if ( vStick.x * vStick.x + vStick.y * vStick.y <= 0.001f )
	{
		if ( bUnk )
			return TFALSE;

		vAlong = TVector4::VEC_ZERO;
	}
	else
	{
		TVector4 vAcross, vUp;
		TFLOAT   fAcross, fUp;
		GetWallAxes( vStick, vAcross, vUp, fAcross, fUp );

		if ( s_fWallJumpAcross < fAcross )
		{
			vAlong.x = vAcross.x;
			vAlong.y = vAcross.y;
			vAlong.z = vAcross.z;
		}
		else if ( fAcross < -s_fWallJumpAcross )
		{
			vAlong.x = -vAcross.x;
			vAlong.y = -vAcross.y;
			vAlong.z = -vAcross.z;
		}
		else
		{
			if ( !bUnk )
			{
				if ( s_fWallJumpUp < fUp )
					return TFALSE;
			}
			else if ( -s_fWallJumpUp <= fUp )
				return TFALSE;

			vAlong = TVector4::VEC_ZERO;
		}
	}

	TVector4 vNormalXZ = rWallNormal;
	vNormalXZ.NormaliseXZ();

	// Not back into the wall
	if ( vNormalXZ.z * vAlong.z + vNormalXZ.x * vAlong.x < -0.70710677f )
		return TFALSE;

	TVector4 vDirection( vAlong.x + rWallNormal.x, rWallNormal.y + vAlong.y, rWallNormal.z + vAlong.z, rWallNormal.w );
	vDirection.Normalise();

	TFLOAT fReach = ( ( m_pColourComponent->GetCurrentColour() >> 4 ) & 1 ) ? ms_pBlobCommonProperties->m_fInkedPenalty * ms_pBlobCommonProperties->m_fWallJumpReach : ms_pBlobCommonProperties->m_fWallJumpReach;

	const TMatrix44* pBodyMatrix = m_pBody ? m_pBody->m_pMatrix : TNULL;
	const TVector4&  rPosition   = pBodyMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION );

	TVector4 vTarget;
	vTarget.Multiply( vDirection, fReach );
	vTarget.x = rPosition.x + vTarget.x;
	vTarget.y = rPosition.y + vTarget.y;
	vTarget.z = rPosition.z + vTarget.z;

	// Stop short of what's in the way
	TPhysicsRay oRay;
	oRay.m_vFrom            = rPosition;
	oRay.m_vTo              = vTarget;
	oRay.m_uiCollisionLayer = 0x81;

	TPhysicsShapeContact oHit;

	if ( pPM->CollideRay( oRay, oHit, TNULL ) )
	{
		TVector4 vPast( vTarget.x - oHit.m_vPosition.x, vTarget.y - oHit.m_vPosition.y, vTarget.z - oHit.m_vPosition.z, vTarget.w );
		TFLOAT   fPast = vPast.Magnitude();

		vTarget.Multiply( vDirection, ( fReach - fPast ) - ms_pBlobProperties[ m_iSize ].m_fRadius );
		vTarget.x = rPosition.x + vTarget.x;
		vTarget.y = rPosition.y + vTarget.y;
		vTarget.z = rPosition.z + vTarget.z;
	}

	TFLOAT fTrajectory = TMath::ATan2( TMath::Abs( vTarget.y - rPosition.y ), TVector4::DistanceXZ( vTarget, rPosition ) );
	fTrajectory        = TMath::LERP( fTrajectory, TMath::HALF_PI, ms_pBlobCommonProperties->m_fWallJumpAngle / TMath::HALF_PI );

	TVector4 vVelocity;
	CalculateJumpVelocity( fTrajectory, rPosition, vTarget, 80.0f, vVelocity );

	// Restart the body with the new velocity
	TVector4 vAngularVelocity;
	pPM->GetAngularVelocity( m_pBody, vAngularVelocity );
	pPM->RemoveFromSimulation( m_pBody, TTRUE );
	TASSERT( STATE_DEAD != m_eState );
	pPM->AddToSimulation( m_pBody, TTRUE );
	TPhysicsManager::SetAngularVelocity( m_pBody, vAngularVelocity );
	TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );

	m_vJumpDirection = vVelocity;
	m_vJumpDirection.Normalise();

	// Off the wall a bit
	TVector4 vStart;
	vStart.Multiply( rWallNormal, 0.1f );
	vStart.x = rPosition.x + vStart.x;
	vStart.y = rPosition.y + vStart.y;
	vStart.z = rPosition.z + vStart.z;
	SetPhysicsPosition( vStart );

	m_uiFlags |= 0x800;
	m_vJumpStart                                    = vStart;
	*TREINTERPRETCAST( TFLOAT*, &m_auiUnk564[ 1 ] ) = 0.05f;

	SetState( STATE_BALLISTIC );

	m_uiFlags &= 0xFF7FFFFF;

	m_pSoundComponent->PlayLiquidJump();
	m_pZJump->OnBlobJump( 0 );

	m_fStretchTimeLeft = ms_pBlobProperties[ m_iSize ].m_fStretchTime;
	return TTRUE;
}

// $deBlob: FUNCTION 00743a90
void ABlobEntity::GetNormalAxes( const Toshi::TVector4& a_rNormal, Toshi::TVector4& a_rOutAcross, Toshi::TVector4& a_rOutUp )
{
	a_rOutAcross.CrossProduct( TVector4::VEC_NEGY, a_rNormal );
	a_rOutAcross.Normalise();
	a_rOutUp.CrossProduct( a_rNormal, a_rOutAcross );
}

// $deBlob: FUNCTION 00743ac0
void ABlobEntity::GetWallAxes( const Toshi::TVector2& a_rStick, Toshi::TVector4& a_rOutAcross, Toshi::TVector4& a_rOutUp, TFLOAT& a_rfOutAcross, TFLOAT& a_rfOutUp )
{
	const TVector4& rNormal = m_oMotion.m_vWallNormal;

	a_rOutAcross.CrossProduct( TVector4::VEC_NEGY, rNormal );
	a_rOutAcross.Normalise();
	a_rOutUp.CrossProduct( rNormal, a_rOutAcross );

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	if ( pCamera->GetCameraType() != ACamera::CT_FREE )
		pCamera = APlayerManager::GetSingleton()->GetBlobCamera( this );

	const TMatrix44& rMatrix = pCamera->GetMatrix();

	TVector4 vCameraRight( rMatrix.m_f11, rMatrix.m_f12, rMatrix.m_f13, 1.0f );
	vCameraRight.NormaliseXZ();

	TVector4 vCameraForward( rMatrix.m_f31, rMatrix.m_f32, rMatrix.m_f33, 1.0f );
	vCameraForward.NormaliseXZ();

	const TVector4& rNegY = TVector4::VEC_NEGY;

	TFLOAT fAcrossForward = a_rOutAcross.x * vCameraForward.x + a_rOutAcross.y * vCameraForward.y + a_rOutAcross.z * vCameraForward.z;
	TFLOAT fAcrossRight   = a_rOutAcross.x * vCameraRight.x + a_rOutAcross.y * vCameraRight.y + a_rOutAcross.z * vCameraRight.z;
	TFLOAT fUp            = a_rOutUp.x * rNegY.x + a_rOutUp.y * rNegY.y + a_rOutUp.z * rNegY.z;
	TFLOAT fSign          = TMath::Sign( rNormal.y * vCameraForward.y + rNormal.x * vCameraForward.x + rNormal.z * vCameraForward.z );

	if ( *TREINTERPRETCAST( const TINT16*, &ms_pBlobCommonProperties->m_afUnk14[ 0 ] ) != 0 )
	{
		a_rfOutAcross = fAcrossRight * a_rStick.x * -fSign * fAcrossRight + fAcrossForward * a_rStick.y * fUp;
		a_rfOutUp     = fUp * a_rStick.y * fAcrossRight - a_rStick.x * fUp * fAcrossForward;
		return;
	}

	a_rfOutAcross = fAcrossForward * a_rStick.x * fAcrossForward + fAcrossRight * a_rStick.x * fAcrossRight;
	a_rfOutUp     = fUp * a_rStick.y;
}

// $deBlob: FUNCTION 0074b9b0
void ABlobEntity::UpdateAirMovement( TFLOAT a_fTimeStep )
{
	TVector2 vStick = TVector2::VEC_ZERO;

	if ( m_iController >= 0 )
		vStick = ABlobInputMap::GetSingleton()->GetMoveStick( m_iController );

	TFLOAT fStrength = GetStickResponse( vStick.Magnitude() ) * ms_pBlobCommonProperties->m_fAirStickStrength;

	TVector4 vForward;
	TVector4 vRight;
	GetMoveAxes( vForward, vRight );

	TPhysicsManager* pPM = TPhysicsManager::GetSingleton();

	TVector4 vVelocity;
	pPM->GetLinearVelocity( m_pBody, vVelocity );

	TVector4 vRightPart;
	vRightPart.Multiply( vRight, vStick.x );
	TVector4 vForwardPart;
	vForwardPart.Multiply( vForward, vStick.y );

	TVector4 vDirection;
	vDirection.x = vForwardPart.x + vRightPart.x;
	vDirection.y = vForwardPart.y + vRightPart.y;
	vDirection.z = vForwardPart.z + vRightPart.z;
	vDirection.w = vForwardPart.w;
	vDirection.Normalise();

	const Properties& rProperties = ms_pBlobProperties[ m_iSize ];

	if ( vVelocity.y * vDirection.y + vVelocity.x * vDirection.x + vVelocity.z * vDirection.z < rProperties.m_fMaxSpeed * fStrength )
	{
		TVector4 vForce;
		vForce.Multiply( vForward, vStick.y * fStrength * rProperties.m_fForce );
		pPM->ApplyForce( m_pBody, vForce );

		vForce.Multiply( vRight, vStick.x * fStrength * rProperties.m_fForce );
		pPM->ApplyForce( m_pBody, vForce );
	}
}

// $deBlob: FUNCTION 0074b560
void ABlobEntity::UpdateWallMovement( TFLOAT a_fTimeStep )
{
	TVector2 vStick = TVector2::VEC_ZERO;

	if ( m_iController >= 0 )
		vStick = ABlobInputMap::GetSingleton()->GetMoveStick( m_iController );

	if ( vStick.x * vStick.x + vStick.y * vStick.y < 0.001f )
	{
		ApplyRollDamping( a_fTimeStep, ms_pBlobCommonProperties->m_fRollDamping, TVector4::VEC_ZERO, TMath::MAXFLOAT );
		return;
	}

	TPhysicsManager*  pPM         = TPhysicsManager::GetSingleton();
	const Properties& rProperties = ms_pBlobProperties[ m_iSize ];
	TFLOAT            fStrength   = GetStickResponse( vStick.Magnitude() );

	TVector4 vVelocity;
	pPM->GetLinearVelocity( m_pBody, vVelocity );

	if ( rProperties.m_fMaxSpeed * fStrength <= vVelocity.Magnitude() )
		return;

	TVector4 vAcross;
	TVector4 vUp;
	TFLOAT   fAcross;
	TFLOAT   fUp;
	GetWallAxes( vStick, vAcross, vUp, fAcross, fUp );
	fAcross *= fStrength;
	fUp *= fStrength;

	// The dead zones (CommonProperties 0x18, 0x1C)
	TFLOAT fDeadAcross = ms_pBlobCommonProperties->m_afUnk14[ 1 ];
	fAcross            = ( TMath::Max( TMath::Abs( fAcross ) - fDeadAcross, 0.0f ) * TMath::Sign( fAcross ) ) / ( 1.0f - fDeadAcross );

	TFLOAT fDeadUp = ms_pBlobCommonProperties->m_afUnk14[ 2 ];
	fUp            = ( TMath::Max( TMath::Abs( fUp ) - fDeadUp, 0.0f ) * TMath::Sign( fUp ) ) / ( 1.0f - fDeadUp );

	TVector4 vAngularVelocity;
	pPM->GetAngularVelocity( m_pBody, vAngularVelocity );

	if ( vAngularVelocity.Magnitude() < rProperties.m_fMaxAngularVelocity * fStrength )
	{
		TVector4 vTorque;
		vTorque.Multiply( vUp, ms_pBlobCommonProperties->m_afUnk14[ 5 ] * fAcross * rProperties.m_fTorque );
		pPM->ApplyTorque( m_pBody, vTorque );

		vTorque.Multiply( vAcross, ms_pBlobCommonProperties->m_afUnk14[ 6 ] * fUp * rProperties.m_fTorque );
		pPM->ApplyTorque( m_pBody, vTorque );
	}

	if ( ( m_uiFlags & 0xC ) == 0 )
		return;

	// Start climbing: remember where (m_vClimbStart) and stop falling
	if ( ( m_uiFlags & 0x800000 ) == 0 && ( 0.0f <= vVelocity.y || ms_pBlobCommonProperties->m_fUnk5C < m_vJumpStart.y - m_oPhysicsMatrix.m_f42 ) )
	{
		m_uiFlags |= 0x800000;
		m_vClimbStart = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

		if ( vVelocity.y < 0.0f )
			vVelocity.y = 0.0f;

		TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );
	}

	TFLOAT   fUpFactor;
	TVector4 vForce;

	if ( ( m_uiFlags & 0x800000 ) == 0 )
	{
		vForce.Multiply( vAcross, ms_pBlobCommonProperties->m_afUnk14[ 5 ] * 0.5f * fAcross * rProperties.m_fForce );
		pPM->ApplyForce( m_pBody, vForce );
		fUpFactor = 0.5f;
	}
	else
	{
		vForce.Multiply( vAcross, ms_pBlobCommonProperties->m_afUnk14[ 5 ] * fAcross * rProperties.m_fForce );
		pPM->ApplyForce( m_pBody, vForce );
		fUpFactor = ms_pBlobCommonProperties->m_afUnk14[ 6 ];
	}

	vForce.Multiply( vUp, fUpFactor * fUp * rProperties.m_fForce );
	pPM->ApplyForce( m_pBody, vForce );
}

// $deBlob: FUNCTION 00750dd0
void ABlobEntity::ResetCameraDirection()
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	if ( pCamera->GetCameraType() != ACamera::CT_FREE )
		pCamera = APlayerManager::GetSingleton()->GetBlobCamera( this );

	const TMatrix44& rMatrix = pCamera->GetMatrix();

	m_vCameraDirection.x = m_oPhysicsMatrix.m_f41 - rMatrix.m_f41;
	m_vCameraDirection.y = m_oPhysicsMatrix.m_f42 - rMatrix.m_f42;
	m_vCameraDirection.z = m_oPhysicsMatrix.m_f43 - rMatrix.m_f43;
	m_vCameraDirection.w = m_oPhysicsMatrix.m_f44;
	m_vCameraDirection.NormaliseXZ();

	if ( rMatrix.m_f22 <= 0.0f )
	{
		m_vCameraDirection.x = -m_vCameraDirection.x;
		m_vCameraDirection.y = -m_vCameraDirection.y;
		m_vCameraDirection.z = -m_vCameraDirection.z;
	}
}

// $deBlob: FUNCTION 00750bd0
void ABlobEntity::UpdateCameraDirection( TFLOAT a_fDeltaTime )
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	if ( pCamera->GetCameraType() != ACamera::CT_FREE )
		pCamera = APlayerManager::GetSingleton()->GetBlobCamera( this );

	const TMatrix44& rMatrix = pCamera->GetMatrix();

	// From the camera to the blob in XZ (the other way when the camera is upside down)
	TVector4 vDirection;
	vDirection.x = m_oPhysicsMatrix.m_f41 - rMatrix.m_f41;
	vDirection.y = m_oPhysicsMatrix.m_f42 - rMatrix.m_f42;
	vDirection.z = m_oPhysicsMatrix.m_f43 - rMatrix.m_f43;
	vDirection.w = m_oPhysicsMatrix.m_f44;
	vDirection.NormaliseXZ();

	if ( rMatrix.m_f22 <= 0.0f )
	{
		vDirection.x = -vDirection.x;
		vDirection.y = -vDirection.y;
		vDirection.z = -vDirection.z;
	}

	TVector4 vVelocity;
	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );

	TFLOAT& rfHoldTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5C4[ 2 ] ); // 0x5CC

	if ( 1.0f < vVelocity.MagnitudeSq() )
	{
		TFLOAT fDot  = m_vCameraDirection.y * vDirection.y + m_vCameraDirection.x * vDirection.x + m_vCameraDirection.z * vDirection.z;
		TFLOAT cfCos = TFLOAT( cos( 1.2217304706573486 ) );
		TASSERT( cfCos > 0.0f && cfCos <= 1.0f );

		// Turned more than 70 degrees while moving: keep the old direction for a while
		if ( fDot < cfCos )
		{
			if ( 0.0f < rfHoldTime )
				rfHoldTime -= a_fDeltaTime;

			if ( rfHoldTime <= 0.0f )
			{
				rfHoldTime         = 0.0f;
				m_vCameraDirection = vDirection;
			}

			return;
		}
	}

	m_vCameraDirection = vDirection;
	rfHoldTime         = ms_pBlobCommonProperties->m_fCameraHoldTime;
}

// $deBlob: FUNCTION 00743990
void ABlobEntity::GetMoveAxes( Toshi::TVector4& a_rOutForward, Toshi::TVector4& a_rOutRight )
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	if ( pCamera->GetCameraType() != ACamera::CT_FREE )
		pCamera = APlayerManager::GetSingleton()->GetBlobCamera( this );

	const TMatrix44& rMatrix = pCamera->GetMatrix();

	// Blend from the blob's camera direction to the camera's up when it looks down
	TFLOAT fUp = rMatrix.m_f32 * s_vAxisY.y + rMatrix.m_f31 * s_vAxisY.x + rMatrix.m_f33 * s_vAxisY.z;
	TMath::Clip( fUp, 0.0f, 1.0f );

	a_rOutForward.Multiply( m_vCameraDirection, 1.0f - fUp );
	a_rOutForward.x = a_rOutForward.x - rMatrix.m_f21;
	a_rOutForward.y = a_rOutForward.y - 0.0f;
	a_rOutForward.z = a_rOutForward.z - rMatrix.m_f23;
	a_rOutForward.NormaliseXZ();

	a_rOutRight.CrossProduct( a_rOutForward, TVector4::VEC_NEGY );
}

// $deBlob: FUNCTION 00750610
TFLOAT ABlobEntity::GetSizeRadius() const
{
	return ms_pBlobProperties[ m_iSize ].m_fRadius;
}

// $deBlob: FUNCTION 00750630
TFLOAT ABlobEntity::GetStickResponse( TFLOAT a_fValue )
{
	TFLOAT                  fSign       = TMath::Sign( a_fValue );
	const CommonProperties* pProperties = ms_pBlobCommonProperties;
	TFLOAT                  fValue      = TMath::Abs( a_fValue );
	TFLOAT                  fResult     = 1.0f;

	if ( pProperties->m_fStickDeadZone <= fValue )
	{
		if ( fValue < pProperties->m_fStickFullZone )
		{
			TFLOAT fRamp = 0.0f;

			if ( pProperties->m_eStickCurve == 1 )
			{
				fRamp = ( ( 1.0f - pProperties->m_fStickMinResponse ) / ( pProperties->m_fStickFullZone - pProperties->m_fStickDeadZone ) ) * ( fValue - pProperties->m_fStickDeadZone );
			}
			else if ( pProperties->m_eStickCurve == 2 )
			{
				fRamp = ( ( TMath::Exp( fValue - pProperties->m_fStickDeadZone ) - 1.0f ) * ( ( 1.0f - pProperties->m_fStickMinResponse ) / ( pProperties->m_fStickFullZone - pProperties->m_fStickDeadZone ) ) ) / 1.7182817f;
			}

			fResult = pProperties->m_fStickMinResponse + fRamp;
		}
	}
	else if ( pProperties->m_eStickCurveLow == 1 )
	{
		fResult = ( pProperties->m_fStickMinResponse * fValue ) / pProperties->m_fStickDeadZone;
	}
	else if ( pProperties->m_eStickCurveLow == 2 )
	{
		// NOTE: retail computes exp( 1 ), not of the value
		fResult = ( ( TMath::Exp( 1.0f ) - 1.0f ) * pProperties->m_fStickMinResponse ) / 1.7182817f;
	}
	else
	{
		fResult = 0.0f;
	}

	TMath::Clip( fResult, 0.0f, 1.0f );
	return fResult * fSign;
}

#ifdef DEBLOB_COMMUNITY_PATCH

// The longest move of one fixed step that is still interpolated (a longer one is a teleport)
static TCONSTEXPR_VAR TFLOAT s_fRenderMaxStepMove = 3.0f;

void ABlobEntity::GetRenderPhysicsMatrix( TMatrix44& a_rOutMatrix )
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
	TUINT32          uiStep          = pPhysicsManager->GetTotalSteps();

	if ( uiStep != m_uiRenderStep )
	{
		// The body was stepped since the last frame
		TUINT32 uiNumSteps = uiStep - m_uiRenderStep;
		m_mRenderPrev      = m_mRenderLast;
		m_mRenderLast      = m_oPhysicsMatrix;
		m_uiRenderStep     = uiStep;
		m_iRenderSteps     = ( uiNumSteps <= 4 ) ? TINT( uiNumSteps ) : 0;

		TFLOAT fMaxMove = s_fRenderMaxStepMove * TFLOAT( uiNumSteps );

		if ( TVector4::DistanceSq( m_mRenderPrev.GetTranslation(), m_mRenderLast.GetTranslation() ) > fMaxMove * fMaxMove )
			m_iRenderSteps = 0;
	}
	else if ( TUtil::MemCompare( &m_mRenderLast, &m_oPhysicsMatrix, sizeof( TMatrix44 ) ) != 0 )
	{
		// Moved without a step (SetPhysicsPosition...): no interpolation
		m_mRenderLast  = m_oPhysicsMatrix;
		m_iRenderSteps = 0;
	}

	if ( m_iRenderSteps <= 0 || m_pBody == TNULL || !m_pBody->IsInSimulation() )
	{
		a_rOutMatrix     = m_oPhysicsMatrix;
		m_mRenderCurrent = a_rOutMatrix;
		return;
	}

	// One step behind the simulation: the time left in the accumulator moves the drawn state from the step before
	// the last one towards the last one
	TFLOAT fTimeStep = pPhysicsManager->GetInitData().m_fTimeStep;
	TFLOAT fProgress = ( TFLOAT( m_iRenderSteps - 1 ) + pPhysicsManager->GetStepRemainder() / fTimeStep ) / TFLOAT( m_iRenderSteps );
	TMath::Clip( fProgress, 0.0f, 1.0f );

	TQuaternion qPrev;
	TQuaternion qLast;
	TQuaternion qRender;
	TMatrix44::MatToQuat( qPrev, m_mRenderPrev );
	TMatrix44::MatToQuat( qLast, m_mRenderLast );
	TQuaternion::Slerp( qRender, qPrev, qLast, fProgress );
	qRender /= TMath::Sqrt( qRender.x * qRender.x + qRender.y * qRender.y + qRender.z * qRender.z + qRender.w * qRender.w );

	TVector4 vPosition;
	vPosition.Lerp4( m_mRenderPrev.GetTranslation(), m_mRenderLast.GetTranslation(), fProgress );
	vPosition.w = 1.0f;

	a_rOutMatrix.SetFromQuaternion( qRender );
	a_rOutMatrix.SetTranslation( vPosition );
	m_mRenderCurrent = a_rOutMatrix;
}

#endif // DEBLOB_COMMUNITY_PATCH

// $deBlob: FUNCTION 007426a0
void ABlobEntity::UpdateModelTransforms( TFLOAT a_fDeltaTime )
{
	ABlobAnimationComponent* pAnimComponent = m_pAnimationComponent;
	Properties*              pProperties    = GetProperties( m_iSize );
	TINT                     iZJumpState    = GetZJumpState();

#ifdef DEBLOB_COMMUNITY_PATCH
	// The models are drawn between the last two fixed steps (one step behind the body), not at the last step: the
	// physics runs at 60 Hz, so above 60 fps they moved only in some frames and twitched against the smooth camera
	TMatrix44 mPhysics;
	GetRenderPhysicsMatrix( mPhysics );
#else
	const TMatrix44& mPhysics = m_oPhysicsMatrix;
#endif

	TVector4 vPosition;
	vPosition.x = mPhysics.m_f41 + m_vBallOffset.x;
	vPosition.y = mPhysics.m_f42 + m_vBallOffset.y;
	vPosition.z = mPhysics.m_f43 + m_vBallOffset.z;
	vPosition.w = m_vBallOffset.w;

	if ( iZJumpState == ABlobZJumpComponent::STATE_AIM || iZJumpState == ABlobZJumpComponent::STATE_AIMNODE )
	{
		TFLOAT fOffset = pProperties->m_fRadius * 0.5f;
		vPosition.x -= m_oMotion.m_vWallNormal.x * fOffset;
		vPosition.y -= m_oMotion.m_vWallNormal.y * fOffset;
		vPosition.z -= m_oMotion.m_vWallNormal.z * fOffset;
	}

	// The level start: the blob follows the level start gate's attach bone
	TBOOL bLevelStart = AGameStatus::GetSingleton()->IsLevelStart();

	if ( bLevelStart )
	{
		TVector4 vAttach;

		if ( AGameStatus::GetSingleton()->GetLevelStartBlobPosition( vAttach ) )
		{
			SetPhysicsPosition( vAttach );
			vPosition = vAttach;
		}

		m_fAwakeTimer = 5.0f;
	}
	else if ( m_eState == STATE_LEVEL_EXIT )
	{
		// The level exit: the blob is held at the door's blob bone
		TMatrix44 mDoor;
		TSTATICCAST( APropTriggerLevelComplete, m_pLevelExitTrigger )->GetDoorBlobMatrix( mDoor );
		vPosition = mDoor.AsBasisVector4( BASISVECTOR_TRANSLATION );
		SetPhysicsPosition( vPosition );
	}

	TMatrix44 mBall = TMatrix44::IDENTITY;
	mBall.SetTranslation( vPosition );
	m_oJellyMatrix.Identity();

	// The jelly: the music squash (m_oJellySquash), the stretch after a jump along m_vStretchDirection and the squash against
	// what the ball touches along m_vUnk614 (each turned so that its axis points up, scaled, turned back)
	if ( !bLevelStart && m_eState != STATE_LEVEL_EXIT )
	{
		TFLOAT fSquash = m_oJellySquash.m_fPosition + 1.0f;
		m_oJellyMatrix.Scale( fSquash, fSquash, fSquash );

		if ( m_fStretch != 1.0f )
		{
			m_oJellyMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ).Multiply( m_vStretchDirection, -( m_fStretch - 1.0f ) );

			TVector4 vAxis = m_vStretchDirection;

			if ( vAxis.y > 0.0f )
			{
				vAxis.x = -vAxis.x;
				vAxis.y = -vAxis.y;
				vAxis.z = -vAxis.z;
			}

			TQuaternion qRotation( 0.0f, 0.0f, 0.0f, 1.0f );
			qRotation.SetVectorDirection( vAxis.AsVector3(), TVector3::VEC_NEGY );

			TMatrix44 mRotation;
			mRotation.SetFromQuaternion( qRotation );
			TMatrix44 mOld = m_oJellyMatrix;
			m_oJellyMatrix.Multiply( mOld, mRotation );

			m_oJellyMatrix.Scale( m_fStretchCompensation, m_fStretch, m_fStretchCompensation );

			qRotation.SetVectorDirection( TVector3::VEC_NEGY, vAxis.AsVector3() );
			mRotation.SetFromQuaternion( qRotation );
			mOld = m_oJellyMatrix;
			m_oJellyMatrix.Multiply( mOld, mRotation );
		}

		if ( m_fSquash != 1.0f )
		{
			m_pSoundComponent->UpdateSquashBounce();

			TVector4 vAxis = m_vUnk614;

			if ( vAxis.y > 0.0f )
			{
				vAxis.x = -vAxis.x;
				vAxis.y = -vAxis.y;
				vAxis.z = -vAxis.z;
			}

			TQuaternion qRotation( 0.0f, 0.0f, 0.0f, 1.0f );
			qRotation.SetVectorDirection( vAxis.AsVector3(), TVector3::VEC_NEGY );

			TMatrix44 mRotation;
			mRotation.SetFromQuaternion( qRotation );
			TMatrix44 mOld = m_oJellyMatrix;
			m_oJellyMatrix.Multiply( mOld, mRotation );

			m_oJellyMatrix.Scale( m_fSquashCompensation, m_fSquash, m_fSquashCompensation );

			qRotation.SetVectorDirection( TVector3::VEC_NEGY, vAxis.AsVector3() );
			mRotation.SetFromQuaternion( qRotation );
			mOld = m_oJellyMatrix;
			m_oJellyMatrix.Multiply( mOld, mRotation );
		}
	}

	TMatrix44 mTranslation = mBall;
	mBall.Multiply( mTranslation, m_oJellyMatrix );

	TMatrix44 mChar = mBall;

	// NOTE: retail reads the matrix of a null body too
#ifdef DEBLOB_COMMUNITY_PATCH
	TMatrix44 mRotation = ( m_pBody != TNULL && m_pBody->m_pMatrix == &m_oPhysicsMatrix ) ? mPhysics : *( ( m_pBody != TNULL ) ? m_pBody->m_pMatrix : TNULL );
#else
	TMatrix44 mRotation = *( ( m_pBody != TNULL ) ? m_pBody->m_pMatrix : TNULL );
#endif
	mRotation.SetTranslation( TVector4( 0.0f, 0.0f, 0.0f, 1.0f ) );

	mTranslation = mBall;
	mBall.Multiply( mTranslation, mRotation );

	// The paint threads follow the ball: a thread is freed once it's longer than 3 or the ball passed its anchor
	TUINT32 uiThreadColour = m_pColourComponent->GetCurrentColour();

	if ( uiThreadColour & ABlobColourComponent::COLOUR_BLACK )
		uiThreadColour = ABlobColourComponent::COLOUR_BLACK;

	TUINT32 uiEndColour = ( ABlobColourMaps::ms_pBodyColourMap[ uiThreadColour ] & 0x00FFFFFF ) |
	    ( TUINT32( TUINT8( TINT( pAnimComponent->m_pBallBodyModelI->GetModelInstance()->GetFade() * 192.0f ) ) ) << 24 );

	for ( Thread* pThread = m_oActiveThreads.m_pHead; pThread != TNULL; )
	{
		Thread* pNext = pThread->m_pNext;

		if ( pThread->m_uiEndColour != 0 )
		{
			pThread->m_vEnd.Multiply( pThread->m_vDirection, ms_pBlobProperties[ m_iSize ].m_fRadius * 0.98f );
			TMatrix44::TransformVector( pThread->m_vEnd, mBall, pThread->m_vEnd );

			TVector4 vThread( pThread->m_vAnchor.x - pThread->m_vEnd.x, pThread->m_vAnchor.y - pThread->m_vEnd.y, pThread->m_vAnchor.z - pThread->m_vEnd.z, pThread->m_vAnchor.w );
			TFLOAT   fFromBallX = pThread->m_vEnd.x - vPosition.x;
			TFLOAT   fFromBallY = pThread->m_vEnd.y - vPosition.y;
			TFLOAT   fFromBallZ = pThread->m_vEnd.z - vPosition.z;
			TFLOAT   fLength    = vThread.Magnitude();

			if ( 3.0f < fLength || fFromBallY * vThread.y + fFromBallX * vThread.x + fFromBallZ * vThread.z < 0.0f )
			{
				pThread->m_uiEndColour = 0;
				m_oActiveThreads.Remove( pThread );
				m_oFreeThreads.PushFront( pThread );
			}
			else
			{
				pThread->m_uiEndColour = uiEndColour;
			}
		}

		pThread = pNext;
	}

	TFLOAT fBallScale = ( pProperties->m_fRadius * 1.02f ) / 3.6f;
	mBall.Scale( fBallScale, fBallScale, fBallScale );
	pAnimComponent->m_pBallBodyModelI->SetTransform( mBall );

	TVector4 vUp = TVector4::VEC_NEGY;

	// The facing and how fast the model turns to it
	ABlobZJumpComponent* pZJump  = m_pZJump;
	TINT                 iZJump  = pZJump->GetState();
	CommonProperties*    pCommon = ms_pBlobCommonProperties;

	if ( iZJump == 1 || ( iZJump == 8 && pZJump->GetCurrentPriorityLock() != TNULL ) )
	{
		TVector4 vZJumpTarget;
		pZJump->GetTargetPosition( vZJumpTarget );
		m_vFacing.x = vZJumpTarget.x - m_oPhysicsMatrix.m_f41;
		m_vFacing.y = vZJumpTarget.y - m_oPhysicsMatrix.m_f42;
		m_vFacing.z = vZJumpTarget.z - m_oPhysicsMatrix.m_f43;
		m_vFacing.w = vZJumpTarget.w;
		m_vFacing.NormaliseXZ();
		m_fLookTurnSpeed = pCommon->m_fLookTurnRate * 2.0f;
	}
	else if ( iZJump == 4 || iZJump == 3 )
	{
		// NOTE: retail computes a basis from the jump normal and doesn't use it
		TVector4 vSide;
		vSide.CrossProduct( TVector4::VEC_NEGY, m_oMotion.m_vWallNormal );
		vSide.Normalise();
		TVector4 vForward;
		vForward.CrossProduct( m_oMotion.m_vWallNormal, vSide );

		m_vFacing        = m_oMotion.m_vWallNormal;
		m_fLookTurnSpeed = pCommon->m_fLookTurnRate * 2.0f;
	}
	else if ( m_eState == STATE_LEVEL_EXIT )
	{
		m_fLookTurnSpeed = pCommon->m_fLookTurnRate * 4.0f;
	}
	else if ( m_eState == STATE_STUCKWALL && pAnimComponent->m_iState == 10 )
	{
		m_vFacing        = m_oMotion.m_vWallNormal;
		m_fLookTurnSpeed = pCommon->m_fLookTurnRate * 2.0f;
	}
	else
	{
		TINT iAnimState = pAnimComponent->m_iState;

		if ( iAnimState == ABlobAnimationComponent::STATE_POWERUP || iAnimState == ABlobAnimationComponent::STATE_GRASSBOMB || m_eState == STATE_LOSTLASTLIFE || m_eState == STATE_GRASSBOMB ||
		     ( AGameStatus::GetSingleton()->GetTimeLeft() <= 0.0f && ( m_eState == STATE_FAIL || m_eState == STATE_VICTORY ) ) || bLevelStart ||
		     ( ( AChallengeManager::GetSingleton()->IsEnding() || ( m_uiFlags & 0x10000000 ) != 0 ) && m_eState == STATE_ROLL ) )
		{
			// Face away from the camera
			APlayerManager::GetSingleton()->GetBlobCamera( this )->GetFlatForward( m_vFacing );
			m_vFacing.x      = -m_vFacing.x;
			m_vFacing.y      = -m_vFacing.y;
			m_vFacing.z      = -m_vFacing.z;
			m_fLookTurnSpeed = pCommon->m_fLookTurnRate * 4.0f;
		}
		else
		{
			TVector4 vVelocity;

			if ( ( m_uiFlags & 0x100000 ) == 0 )
				TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );
			else
				vVelocity = m_oMotion.m_vUnk220;

			TFLOAT fSpeed = vVelocity.NormaliseXZ();

			if ( fSpeed <= pProperties->m_fMaxSpeed * pCommon->m_afUnkA0[ 5 ] )
			{
				// Standing still: from time to time, Blob faces the closest civilian or enemy within 30 (at most 10 above
				// or below, closer than the radius + 20), else away from the camera; a civilian with the flag 4 at
				// 0x1F1 gets a greeting animation
				if ( m_fAwakeTimer <= 0.0f && pAnimComponent->m_pStateAnim == TNULL )
				{
					const TVector4& rcPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

					TFLOAT        fClosest     = pProperties->m_fRadius + 20.0f;
					ABaseEntity*  pClosest     = TNULL;
					TINT          iNumEntities = 0;
					ABaseEntity** ppEntities   = AEntityHash::GetSingleton()->Query( rcPosition, 30.0f, &iNumEntities, TNULL, TFALSE, 500, TFALSE, TTRUE, TFALSE );
					TVector4      vClosestDir;

					for ( TUINT i = 0; i < TUINT( iNumEntities ); i++ )
					{
						ABaseEntity* pEntity = ppEntities[ i ];

						if ( pEntity == this || TMath::Abs( pEntity->GetBasePosition().y - rcPosition.y ) >= 10.0f )
							continue;

						if ( pEntity->GetClass() != &TGetClass( ACiv ) && !pEntity->IsA( &TGetClass( AEnemy ) ) )
							continue;

						const TVector4& rcEntityPos = pEntity->GetBasePosition();
						TVector4        vDirection( rcEntityPos.x - rcPosition.x, rcEntityPos.y - rcPosition.y, rcEntityPos.z - rcPosition.z, rcEntityPos.w );
						TFLOAT          fDistance = vDirection.NormaliseXZ() - pEntity->GetBoundingSphereRadius();

						if ( fDistance < fClosest )
						{
							fClosest    = fDistance;
							pClosest    = pEntity;
							vClosestDir = vDirection;
						}
					}

					if ( pClosest != TNULL )
					{
						if ( pClosest->GetClass() == &TGetClass( ACiv ) && ( *( TREINTERPRETCAST( TUINT8*, pClosest ) + 0x1F1 ) & 4 ) != 0 )
						{
							ABlobAnimationComponent::AnimSet* pAnims = ABlobAnimationComponent::ms_pAnimationProperties->m_pCommon;
							TINT                              iAnim  = ARandom::GetSingleton()->GetRandom().GetInt( pAnims->m_iNumCivAnims );

							if ( pAnimComponent->PlayOneShot( &pAnims->m_pCivAnims[ iAnim ] ) )
							{
								m_fUnk59C        = 5.0f;
								m_fAwakeDuration = 5.0f;
							}

							m_fLookTurnSpeed = pCommon->m_fLookTurnRate;
							m_vFacing        = vClosestDir;
							m_fAwakeTimer    = m_fAwakeDuration;
						}
						else
						{
							m_fAwakeDuration = 5.0f;
							m_fAwakeTimer    = m_fAwakeDuration;
							m_fLookTurnSpeed = pCommon->m_fLookTurnRate * 0.25f;
							m_vFacing        = vClosestDir;
						}
					}
					else
					{
						// Away from the camera
						APlayerManager::GetSingleton()->GetBlobCamera( this )->GetFlatForward( m_vFacing );
						m_vFacing.x      = -m_vFacing.x;
						m_vFacing.y      = -m_vFacing.y;
						m_vFacing.z      = -m_vFacing.z;
						m_fAwakeDuration = 5.0f;
						m_fAwakeTimer    = m_fAwakeDuration;
						m_fLookTurnSpeed = pCommon->m_fLookTurnRate * 0.25f;
					}
				}
			}
			else
			{
				// Face the way it rolls
				m_vFacing        = vVelocity;
				m_fLookTurnSpeed = pCommon->m_fLookTurnRate;
			}
		}
	}

	TVector4 vDirection( -m_vFacing.x, -m_vFacing.y, -m_vFacing.z, m_vFacing.w );

	switch ( m_eState )
	{
		case STATE_ROLL:
		case STATE_BALLISTIC:
		case STATE_LMARK:
		case STATE_BOSS:
		case STATE_FLOAT:
		case STATE_CANNON:
		case STATE_GRASSBOMB:
		case STATE_RESPAWN:
		case STATE_LEVEL_EXIT:
		case STATE_DEAD:
		case STATE_FAIL:
		case STATE_VICTORY:
		case STATE_LOSTLASTLIFE:
			vUp = TVector4::VEC_NEGY;
			break;
		case STATE_STUCKWALL:
			if ( iZJumpState != ABlobZJumpComponent::STATE_AIMNODE && iZJumpState != ABlobZJumpComponent::STATE_AIM )
			{
				vUp.x = m_oMotion.m_vWallNormal.x + TVector4::VEC_NEGY.x;
				vUp.y = m_oMotion.m_vWallNormal.y + TVector4::VEC_NEGY.y;
				vUp.z = m_oMotion.m_vWallNormal.z + TVector4::VEC_NEGY.z;
				vUp.w = m_oMotion.m_vWallNormal.w;
				vUp.Normalise();
			}
			break;
		case STATE_HOTPLATE:
			vUp = m_oMotion.m_vContactNormal;
			break;
	}

	TMatrix44 mLook;
	mLook.LookAtDirection( vDirection, vUp );

	TQuaternion qLook;
	TMatrix44::MatToQuat( qLook, mLook );

	TFLOAT fProgress = m_fLookTurnSpeed * a_fDeltaTime;
	if ( fProgress > 1.0f ) fProgress = 1.0f;

	TQuaternion& rRotation = *TREINTERPRETCAST( TQuaternion*, &m_vLookRotation );
	TQuaternion::Slerp( rRotation, rRotation, qLook, fProgress );
	rRotation /= TMath::Sqrt( rRotation.x * rRotation.x + rRotation.y * rRotation.y + rRotation.z * rRotation.z + rRotation.w * rRotation.w );

	mLook.SetFromQuaternion( rRotation );
	mChar.Multiply( mTranslation, mLook );

	TFLOAT fCharScale = pProperties->m_fRadius / 3.6f;
	mChar.Scale( fCharScale, fCharScale, fCharScale );

	if ( pAnimComponent->m_pBallEarsModelI->GetFlags() & AModelInstance2::FLAG_Visible )
		pAnimComponent->m_pBallEarsModelI->SetTransform( mChar );

	pAnimComponent->m_pCharModelI->SetTransform( mChar );

	ABlobAnimationComponent::Anim* pCurrentAnim = pAnimComponent->m_pCurrentAnim;

	if ( ( pAnimComponent->m_pCharModelI->GetFlags() & AModelInstance2::FLAG_Visible ) && pCurrentAnim != TNULL && ( pCurrentAnim->m_uiFlags & 1 ) )
	{
		TMatrix44 mBone;
		pAnimComponent->m_pCharModelI->GetBoneTransform( ABlobAnimationComponent::ms_iBoneSBBody, mBone );
		m_vBallPosition = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
		return;
	}

	m_vBallPosition = mPhysics.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// $deBlob: FUNCTION 0073de70
void ABlobEntity::Tick( TFLOAT a_fDeltaTime )
{
	ABaseEntity::Tick( a_fDeltaTime );

	// The shockwave grows from 0.01 to its size (0.65 to 1.8 by Blob's size) while its time runs out
	if ( m_pShockwaveModel != TNULL && m_fShockwaveTime > 0.0f )
	{
		TFLOAT fProgress = ( m_fShockwaveDuration - m_fShockwaveTime ) / m_fShockwaveDuration;
		TFLOAT fScale    = TMath::LERP( 0.65f, 1.8f, TFLOAT( m_iSize ) / 10.0f );
		fScale           = TMath::LERP( 0.01f, fScale, fProgress );

		TTransformObject& rTransform = TSTATICCAST( TModelInstance, m_pShockwaveModel )->GetTransform();
		rTransform.SetTranslate( m_vShockwavePosition.AsVector3() );
		rTransform.SetScale( fScale, fScale, fScale );
		m_fShockwaveTime -= 1.1f * a_fDeltaTime;
	}

	UpdateCameraDirection( a_fDeltaTime );

	m_Flags |= FLAG_UpdateMatrix;
	m_pAnimationComponent->Update( a_fDeltaTime );

	m_pColourComponent->Tick( a_fDeltaTime );

	m_pSplatTrail->Update();

	m_pRibbonTrail->Tick( a_fDeltaTime );

	m_pSoundComponent->Tick( a_fDeltaTime );

	m_pZJump->Tick( a_fDeltaTime );

	m_pNavRing->Tick( a_fDeltaTime );

	// Inked by an evil blob (GetInked, BFLAG 0x20000000): the body is rebuilt for the new colour
	if ( m_uiFlags & 0x20000000 )
	{
		m_uiFlags &= ~0x20000000;
		RecreatePhysics();
	}

	// The water ripples: one every 0.5 to 0.1 s (faster with the speed, the fastest at 10) while the ball floats on a
	// flat liquid surface
	TVector4 vVelocity;
	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vVelocity );

	TFLOAT  fRippleInterval = TMath::LERP( 0.5f, 0.1f, TMath::Min( vVelocity.MagnitudeXZ() / 10.0f, 1.0f ) );
	TFLOAT& rfRippleTime    = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 4 ] ); // 0x590
	rfRippleTime            = rfRippleTime - a_fDeltaTime;
	rfRippleTime            = TMath::Max( 0.0f, rfRippleTime );
	rfRippleTime            = TMath::Min( rfRippleTime, fRippleInterval );

	if ( m_eState == STATE_FLOAT && s_fLiquidSurfaceCos < m_oMotion.m_vLiquidNormal.DotProduct3( TVector4::VEC_NEGY ) && rfRippleTime <= 0.0f )
	{
		rfRippleTime = fRippleInterval;

		if ( m_oMotion.m_uiContactFlags & 1 )
		{
			TParticleEffectInstance* pRipple = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pWaterRippleEffect );

			if ( pRipple != TNULL )
			{
				TMatrix44 oMatrix = TMatrix44::IDENTITY;
				oMatrix.SetTranslation( m_oMotion.m_vLiquidHit );
				oMatrix.Scale( 0.2f, 0.2f, 0.2f );
				pRipple->GetTransform() = oMatrix;
			}
		}
	}

	// The effect at 0xB00 follows the model's bone blob_lb_particleeffect1, scaled by the ball's radius
	TParticleEffectInstance* pBoneEffect = TREINTERPRETCAST( TParticleEffectInstance*, m_auiSounds[ 10 ] ); // 0xB00

	if ( pBoneEffect != TNULL )
	{
		TMatrix44 oBone;
		m_pAnimationComponent->GetCharModelInstance()->GetBoneTransform( ABlobAnimationComponent::ms_iBoneEffect1, oBone );

		TMatrix44 oMatrix = TMatrix44::IDENTITY;
		oMatrix.SetTranslation( oBone.AsBasisVector4( BASISVECTOR_TRANSLATION ) );

		TFLOAT fScale = TFLOAT( ms_pBlobProperties[ m_iSize ].m_fRadius / 3.6 );
		oMatrix.Scale( fScale, fScale, fScale );
		pBoneEffect->GetTransform() = oMatrix;
	}

	if ( ( m_uiFlags & 0x80000 ) == 0 )
	{
		if ( 0.0f < m_fAwakeTimer )
			m_fAwakeTimer -= a_fDeltaTime;

		if ( 0.0f < m_fUnk59C )
			m_fUnk59C -= a_fDeltaTime;
	}

	// The time left the enemies ignore the blob
	if ( 0.0f < m_fUntargetableTime )
		m_fUntargetableTime -= a_fDeltaTime;

	if ( m_pFXBreakpointComponent != TNULL )
		m_pFXBreakpointComponent->Tick( a_fDeltaTime );

	// Single player: no life pending anymore once the life nodes are gone
	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() < 0 )
	{
		APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetBlobDataOf( this );
		ABlobLifeHud*             pLifeHud  = TREINTERPRETCAST( ABlobLifeHud*, pBlobData->m_uiLifeHud ); // BlobData 0x24

		if ( m_oList438.IsEmpty() && pLifeHud->IsLifePending() )
			pLifeHud->SetLifePending( TFALSE );
	}

	// The rainbow paint (BFLAG 0x200000): its time runs (not during the rainbow animation 0x1F), the colour cycles
	// through the complementary colours every m_afUnk100[ 1 ] seconds; it ends when the time is up or Blob is inked
	if ( TUINT8( m_uiFlags >> 0x15 ) & 1 )
	{
		TFLOAT& rfRainbowTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5A8[ 5 ] ); // 0x5BC
		TFLOAT& rfColourTime  = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5A8[ 4 ] ); // 0x5B8

		if ( m_pAnimationComponent->GetState() != ABlobAnimationComponent::STATE_POWERUP && !AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C )
			rfRainbowTime -= a_fDeltaTime;

		TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

		if ( uiColour & 0x10 )
			uiColour = 0x10;

		if ( rfRainbowTime <= 0.0f || uiColour == 0x10 )
		{
			EndRainbow();
		}
		else
		{
			rfColourTime += a_fDeltaTime;

			if ( ms_pBlobCommonProperties->m_afUnk100[ 1 ] < rfColourTime )
			{
				rfColourTime = 0.0f;
				m_pColourComponent->SetColour( ABlobColourComponent::GetComplementaryColour( uiColour ) );
			}
		}
	}

	// The paint squeeze (multiplayer only: BUTTON 0x20 held for more than 0.2 s while painted): a paint point every
	// 0.15 s, sprayed out as paint particles
	{
		TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

		if ( uiColour & 0x10 )
			uiColour = 0x10;

		TFLOAT& rfSqueezeTime = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5C4[ 1 ] ); // 0x5C8
		TFLOAT& rfPointTime   = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk5C4[ 0 ] ); // 0x5C4

		if ( ( m_uiButtons & ABlobInputMap::BUTTON_BRAKE ) == 0 || uiColour == 0x10 || uiColour == 0 || AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE )
		{
			rfSqueezeTime = 0.0f;
			m_bUnkB64     = TFALSE;

			if ( m_uiFlags & 0x400000 )
				m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_SHIELD_DEACTIVATING );

			m_uiFlags &= ~0x400000;
			rfPointTime = 0.0f;

			TParticleEffectInstance*& rpSqueeze = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 13 ] ); // 0xB0C

			if ( rpSqueeze != TNULL )
			{
				rpSqueeze->Stop();
				rpSqueeze = TNULL;
			}
		}
		else
		{
			rfSqueezeTime += a_fDeltaTime;

			if ( 0.2f < rfSqueezeTime )
			{
				m_bUnkB64 = TTRUE;

				if ( ( m_uiFlags & 0x400000 ) == 0 )
				{
					m_pSoundComponent->PlaySqueeze();
					m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_SHIELD_ACTIVATING );
				}

				m_uiFlags |= 0x400000;
				rfPointTime += a_fDeltaTime;

				if ( 0.15f < rfPointTime )
				{
					rfPointTime = 0.0f;
					RemovePaintPoints( 1, TFALSE );
				}

				// A paint particle out of the bottom of the ball, in a random direction within 0.3 of straight down
				TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

				TVector4 vSideZ;
				vSideZ.Multiply( TVector4::VEC_POSZ, rRandom.GetFloatMinMax( -0.3f, 0.3f ) );
				TVector4 vSideX;
				vSideX.Multiply( TVector4::VEC_POSX, rRandom.GetFloatMinMax( -0.3f, 0.3f ) );

				TVector4 vDirection( vSideZ.x + ( vSideX.x + TVector4::VEC_NEGY.x ), vSideZ.y + ( vSideX.y + TVector4::VEC_NEGY.y ), vSideZ.z + ( vSideX.z + TVector4::VEC_NEGY.z ), TVector4::VEC_NEGY.w );

				TFLOAT fRadius = ms_pBlobProperties[ m_iSize ].m_fRadius;
				TFLOAT fSpeed  = ms_pBlobProperties[ m_iSize ].m_fMaxAngularVelocity;
				fSpeed         = rRandom.GetFloatMinMax( fSpeed * 0.9f, fSpeed * 1.2f );

				TVector4 vVelocity;
				vVelocity.Multiply( vDirection, fSpeed * fRadius * 1.0f );

				TVector4 vPosition;
				vPosition.Multiply( vDirection, fRadius * 0.8f );
				vPosition.x = m_oPhysicsMatrix.m_f41 + vPosition.x;
				vPosition.y = m_oPhysicsMatrix.m_f42 + vPosition.y;
				vPosition.z = m_oPhysicsMatrix.m_f43 + vPosition.z;

				TINT iParam38 = 0;

				if ( rRandom.GetFloat() < 0.2f )
				{
					TUINT32 uiIndex = m_pColourComponent->GetCurrentColour();

					if ( uiIndex & 0x10 )
						uiIndex = 0x10;

					iParam38 = TINT( TREINTERPRETCAST( TUINTPTR, ms_apInkCloudSmallEffects[ uiIndex ] ) );
				}

				TINT iDecal = 0;

				if ( 0.8f < rRandom.GetFloat() )
					iDecal = TINT( TREINTERPRETCAST( TUINTPTR, m_pMaterialTestSplat ) );

				APaintParticleSystem::s_Instance->AddParticle(
				    vPosition,
				    vVelocity,
				    m_pColourComponent->m_uiFadeColour2,
				    fRadius * 0.75f,
				    iDecal,
				    iParam38,
				    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomA ) ),
				    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomB ) )
				);
			}
		}
	}

	if ( 0.0f < m_fUnkB34 )
		m_fUnkB34 -= a_fDeltaTime;

	// BFLAG 0x400 only lasts a tick
	if ( ( m_uiFlags & 0x400 ) == 0 )
	{
		m_auiUnk5A8[ 2 ] = 0; // 0x5B0
		m_auiUnk5A8[ 3 ] = 0; // 0x5B4

		if ( m_uiPrevFlags & 0x400 )
			m_pAnimationComponent->SetIdleAnimDelay();
	}

	m_uiPrevFlags = m_uiFlags;
	m_uiFlags &= ~0x400;

	// The looping effect at 0xB18 stops without the contact flag 0x80, which only lasts a tick
	TParticleEffectInstance*& rpContactLoop = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 16 ] ); // 0xB18

	if ( ( m_oMotion.m_uiContactFlags & 0x80 ) == 0 && rpContactLoop != TNULL )
	{
		rpContactLoop->StopLooping();
		rpContactLoop = TNULL;
	}

	m_oMotion.m_uiContactFlags &= ~0x80;

	TASSERT( ( m_iCamShakeHotPlateID >= 0 ) == ( m_eState == STATE_HOTPLATE ), "(m_iCamShakeHotPlateID>=0)==(m_eState==STATE_HOTPLATE)" );
}

// $deBlob: FUNCTION 0073de40
void ABlobEntity::OnOutOfWorld()
{
	// Out of the world: back to the respawn point, and the camera with it
	Respawn( TTRUE, TTRUE, TNULL );
	APlayerManager::GetSingleton()->GetBlobFollowCamera( this )->Reset( TFALSE, TTRUE );
}

// $deBlob: FUNCTION 0073d3f0
void ABlobEntity::Create()
{
	ABaseEntity::Create();

	// The initial size of the level
	TINT iSize       = ALevelInformation::GetSingleton()->GetLevelHeader()->m_pLevelProps[ AGameStatus::GetSingleton()->GetCurrentLevel() ].m_auiMedalLimits[ 7 ];
	m_iSize          = iSize;
	m_iOldSize       = iSize;
	m_iUserDataType  = 1;
	m_pUserDataOwner = this;
	m_iPaintPoints   = ( iSize * 5 + 5 ) * 2;

	CreatePhysics( 0.0f );

	m_oJelly.SetDampingRatio( ms_pBlobProperties[ m_iSize ].m_fJellyDamping );

	TFLOAT fRadius = ms_pBlobProperties[ m_iSize ].m_fRadius;
	SetBBox( TVector4( fRadius, fRadius, fRadius, 1.0f ) );
	m_fBoundingSphereRadius = ms_pBlobProperties[ m_iSize ].m_fRadius;

	m_pColourComponent = new ABlobColourComponent( this );

	m_pSplatTrail = new ABlobSplatTrail( this );

	m_pSoundComponent = new ABlobSoundComponent( this );

	m_pAnimationComponent = new ABlobAnimationComponent( this );
	m_pZJump              = new ABlobZJumpComponent( this );

	m_pCollisionObject = new ACollisionObject( this, 2, 0xFF );

	TASSERT( AGameStatus::IsSingletonCreated() );

	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
		m_pFXBreakpointComponent = AFXBreakpointComponent::Create( "blob_bodya.trb", TMemory_GetGlobalHeap(), this, TNULL );
	else
		m_pFXBreakpointComponent = AFXBreakpointComponent::Create( "blob_bodya_mp.trb", TMemory_GetGlobalHeap(), this, TNULL );

	m_pNavRing = new ABlobNavRingComponent( this );

	// The default start position (moved to the level's start point by APlayerManager)
	m_vRespawnPosition             = TVector4( -40.0f, -10.0f, 33.0f, 1.0f );
	m_vRespawnRotation             = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_bHasRespawnRotation          = 0;
	*(TUINT8*)&m_uiRespawnTransSeq = 0;
	m_uiRespawnSeqStartRatio       = 0;
	m_fRespawnSeqEndRatio          = 1.0f;
	SetPhysicsPosition( m_vRespawnPosition );
	TPhysicsManager::StopBody( m_pBody );
	m_oJelly.m_vPosition = m_vRespawnPosition;
	m_oJelly.m_vRest     = m_vRespawnPosition;
	m_oJelly.m_vVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f ); // TVector4::X0Y0Z0W1
	m_vBallOffset        = TVector4( 0.0f, 0.0f, 0.0f, 1.0f ); // TVector4::X0Y0Z0W1
	m_vBallPosition      = m_vRespawnPosition;

	m_pMaterialTestSplat = TNULL;
	m_pMaterialUnk64C    = TNULL;
	m_pMaterialThread    = TNULL;

	TSysShader* pSysShader = TSysShader::GetSingleton();

	if ( pSysShader != TNULL )
	{
		// The material of the paint splat decals
		TSysMaterial* pMatTestSplat = pSysShader->CreateMaterial( TNULL );
		m_pMaterialTestSplat        = pMatTestSplat;
		pMatTestSplat->Create( 0 );
		pMatTestSplat->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
		TTexture* pTexture = TTextureManager::GetSingleton()->FindTexture( "test_splat.tga" );
		pMatTestSplat->SetTextureNum( 1 );
		pMatTestSplat->SetTexture( 0, pTexture );

		// The threads: their material and quads, all of them free
		TSysMaterial* pMatThread = pSysShader->CreateMaterial( TNULL );
		m_pMaterialThread        = pMatThread;
		pMatThread->Create( TSysMaterial::BLENDMODE_1 );
		pMatThread->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
		pMatThread->SetFlags( TMaterial::FLAGS_ALPHAUPDATE, TTRUE );
		pMatThread->SetTexture( 0, TTextureManager::GetSingleton()->FindTexture( "thread.tga" ) );
		pMatThread->SetTextureNum( 1 );

		for ( TINT i = 0; i < 16; i++ )
		{
			TSysMesh* pMesh         = pSysShader->CreateMesh( TNULL );
			m_aThreads[ i ].m_pMesh = pMesh;
			pMesh->Create( TSysMesh::FLAGS_VERTICES_DYNAMIC_APPEND | TSysMesh::FLAGS_INDICES_DYNAMIC_APPEND | TSysMesh::FLAGS_UNK6, 4, 4 );
			pMesh->SetMaterial( m_pMaterialThread );
			m_aThreads[ i ].m_pNext = ( i < 15 ) ? &m_aThreads[ i + 1 ] : TNULL;
			m_aThreads[ i ].m_pPrev = ( i > 0 ) ? &m_aThreads[ i - 1 ] : TNULL;
		}

		m_oFreeThreads.m_pHead  = &m_aThreads[ 0 ];
		m_oFreeThreads.m_pTail  = &m_aThreads[ 15 ];
		m_oFreeThreads.m_iCount = 16;
	}

	m_pRibbonTrail = new ABlobRibbonTrailComponent( this );

	// The blob's own copies of the effects it changes (0xAD8-0xAE4)
	TParticleEffectManager* pEffectManager = TParticleSystem::GetSingleton()->GetEffectManager();
	m_auiSounds[ 0 ]                       = TREINTERPRETCAST( TUINT32, ms_pElectrocutionEffect->Copy() );
	m_auiSounds[ 1 ]                       = TREINTERPRETCAST( TUINT32, pEffectManager->FindEffect( "Blob_SpikeHaemorrhage" ) );
	m_auiSounds[ 1 ]                       = TREINTERPRETCAST( TUINT32, TREINTERPRETCAST( TParticleEffect*, m_auiSounds[ 1 ] )->Copy() );
	m_auiSounds[ 2 ]                       = TREINTERPRETCAST( TUINT32, pEffectManager->FindEffect( "Blob_Stunned" ) );
	m_auiSounds[ 2 ]                       = TREINTERPRETCAST( TUINT32, TREINTERPRETCAST( TParticleEffect*, m_auiSounds[ 2 ] )->Copy() );
	m_auiSounds[ 3 ]                       = TREINTERPRETCAST( TUINT32, pEffectManager->FindEffect( "Blob_Rockslide" ) );
	m_auiSounds[ 3 ]                       = TREINTERPRETCAST( TUINT32, TREINTERPRETCAST( TParticleEffect*, m_auiSounds[ 3 ] )->Copy() );

	// The LeechBots' target
	AEnemyManager::GetSingleton()->AddTarget( this );

	// Draws the shockwave
	// NOTE: retail never removes this callback (Destroy doesn't call RemoveCallback); the factory deletes it
	AEntityFactory::GetSingleton()->AddCallback( this, ShockwaveRenderCallback );

	// The shockwave (the Z-jump slam's distortion ring)
	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
	{
		m_pShockwaveNoise = TTextureManager::GetSingleton()->FindTexture( "shockwave_noise.tga" );
		m_pShockwaveNoise->SetWrap( D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP );
	}

	m_fShockwaveTime     = 0.0f;
	m_fShockwaveDuration = 0.0f;
	m_fShockwaveAlpha    = 1.0f;

	// The shockwave model: its only mesh in blend mode 10
	T2ModelPtr pShockwave = TModelManager::GetSingleton()->FindModel( "distort_wave.txs" );
	m_pShockwaveModel     = pShockwave->CreateInstance();

	TModelLOD& rLOD = pShockwave->GetLOD( 0 );
	TASSERT( rLOD.GetMeshCount() == 1 );

	TWorldMaterialHAL* pShockwaveMaterial = TSTATICCAST( TWorldMaterialHAL, rLOD.GetMesh( 0 )->GetMaterial() );
	pShockwaveMaterial->SetBlendMode( TWorldMaterial::BLENDMODE_SHOCKWAVE );
	pShockwaveMaterial->SetAlpha( 1 );
}

static void StopBlobEffects( TUINT32* a_pEffects );

// $deBlob: FUNCTION 0073da90
void ABlobEntity::Destroy()
{
	// The shockwave model
	m_pShockwaveNoise = TNULL;

	if ( m_pShockwaveModel != TNULL )
	{
		delete TSTATICCAST( TModelInstance, m_pShockwaveModel );
		m_pShockwaveModel = TNULL;
	}

	TPhysicsManager* pPM = TPhysicsManager::GetSingleton();

	TPhysicsPhantomContainer** apPhantoms[] = { &m_pLiquidRayCastHelper, &m_pUnk424, &m_pJellyQueryHelper, &m_pSoftCollisionQueryHelper, &m_pPaintParticlePhantomA, &m_pPaintParticlePhantomB };

	for ( TPhysicsPhantomContainer** ppPhantom : apPhantoms )
	{
		if ( *ppPhantom )
		{
			pPM->DestroyPhantom( *ppPhantom );
			*ppPhantom = TNULL;
		}
	}

	pPM->RemovePlugin( &m_oPhysicsPlugin );

	pPM->RemoveCustomMover( &m_oPhysicsMover );
	pPM->RemoveFromSimulation( m_pBody, TTRUE );
	pPM->DestroyBody( m_pBody );

	// The threads' quads and the materials
	for ( TINT i = 0; i < 16; i++ )
	{
		if ( m_aThreads[ i ].m_pMesh != TNULL )
		{
			m_aThreads[ i ].m_pMesh->DestroyResource();
			m_aThreads[ i ].m_pMesh = TNULL;
		}
	}

	TMaterial** apMaterials[] = { &m_pMaterialTestSplat, &m_pMaterialUnk64C, &m_pMaterialThread };

	for ( TMaterial** ppMaterial : apMaterials )
	{
		if ( *ppMaterial != TNULL )
		{
			( *ppMaterial )->Destroy(); // OnDestroy and delete
			*ppMaterial = TNULL;
		}
	}

	ADecalManager::GetSingleton()->Reset();
	ADecalManager::GetSingleton()->SetRainbow( TFALSE );

	delete m_pFXBreakpointComponent;
	m_pFXBreakpointComponent = TNULL;

	delete m_pAnimationComponent;
	m_pAnimationComponent = TNULL;

	delete m_pColourComponent;
	m_pColourComponent = TNULL;

	// NOTE: retail frees it without a destructor call
	delete m_pSplatTrail;
	m_pSplatTrail = TNULL;

	delete m_pRibbonTrail;
	m_pRibbonTrail = TNULL;

	delete m_pSoundComponent;
	m_pSoundComponent = TNULL;

	delete m_pZJump;
	m_pZJump = TNULL;

	delete m_pNavRing;
	m_pNavRing = TNULL;

	delete m_pCollisionObject;
	m_pCollisionObject = TNULL;

	// The effect instances, then the effects the blob made (0xAD8-0xAE4)
	StopBlobEffects( m_auiSounds );

	for ( TINT i = 0; i < 4; i++ )
	{
		if ( m_auiSounds[ i ] != 0 )
		{
			delete TREINTERPRETCAST( TParticleEffect*, m_auiSounds[ i ] );
			m_auiSounds[ i ] = 0;
		}
	}

	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 0073dde0
void ABlobEntity::SetVisible( TBOOL a_bVisible )
{
	if ( a_bVisible )
		m_Flags |= FLAG_Visible;
	else
		m_Flags &= ~FLAG_Visible;

	if ( m_pAnimationComponent != TNULL )
		m_pAnimationComponent->SetVisible( a_bVisible );

	if ( m_pFXBreakpointComponent != TNULL )
		m_pFXBreakpointComponent->SetVisible( a_bVisible );

	if ( m_pRibbonTrail != TNULL )
		m_pRibbonTrail->SetVisible( a_bVisible );
}

// $deBlob: FUNCTION 00743960
ACamera* ABlobEntity::GetViewCamera()
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	if ( pCamera->GetCameraType() != ACamera::CT_FREE )
		return APlayerManager::GetSingleton()->GetBlobCamera( this );

	return pCamera;
}

// The ink particles of the death pop
static TINT s_iDeathPopParticles = 40;

// $deBlob: FUNCTION 00747590
void ABlobEntity::PopDeath()
{
	TRandom&              rRandom         = ARandom::GetSingleton()->GetRandom();
	APaintParticleSystem* pPaintParticles = APaintParticleSystem::s_Instance;
	TVector4              vDirection      = TVector4( 0.0f, 0.0f, 0.0f, 1.0f ); // TVector4::X0Y0Z0W1

	for ( TINT i = 0; i < s_iDeathPopParticles; i++ )
	{
		TMath::SinCos( rRandom.GetFloatMinMax( 0.0f, TMath::TWO_PI ), vDirection.x, vDirection.z );
		vDirection.y = 0.0f;
		vDirection.w = 1.0f;

		TVector4 vParticlePos;
		vParticlePos.Multiply( vDirection, ms_pBlobProperties[ 0 ].m_fRadius * 0.75f );
		vParticlePos.x = m_oPhysicsMatrix.m_f41 + vParticlePos.x;
		vParticlePos.y = m_oPhysicsMatrix.m_f42 + vParticlePos.y;
		vParticlePos.z = m_oPhysicsMatrix.m_f43 + vParticlePos.z;

		vDirection.y = vDirection.y - ( rRandom.GetFloatMinMax( 0.0f, 2.5f ) + 0.5f );
		vDirection.Multiply( ms_pBlobProperties[ 0 ].m_fRadius * 15.0f );

		pPaintParticles->AddParticle(
		    vParticlePos,
		    vDirection,
		    ABlobColourComponent::ms_auiColours[ ABlobColourComponent::COLOUR_BLACK ],
		    ms_pBlobProperties[ 0 ].m_fRadius,
		    TINT( TREINTERPRETCAST( TUINTPTR, m_pMaterialTestSplat ) ),
		    TINT( TREINTERPRETCAST( TUINTPTR, ms_apInkCloudSmallEffects[ ABlobColourComponent::COLOUR_BLACK ] ) ),
		    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomA ) ),
		    TINT( TREINTERPRETCAST( TUINTPTR, m_pPaintParticlePhantomB ) )
		);
	}

	TParticleEffectInstance* pSpray = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pDeathPopSprayEffect );

	if ( pSpray != TNULL )
		pSpray->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );

	m_pSoundComponent->PlayDeath();
	SetVisible( TFALSE );
}

// $deBlob: FUNCTION 00747810
TBOOL ABlobEntity::CanInkBlob( ABlobEntity* a_pOther )
{
	TINT iMpType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();
	TASSERT( iMpType >= 0 );

	// NOTE: colour 0x10 is the ink
	TUINT32 uiColour      = m_pColourComponent->GetCurrentColour();
	TUINT32 uiOtherColour = a_pOther->m_pColourComponent->GetCurrentColour();

	switch ( iMpType )
	{
		case 0:
		case 2:
			if ( uiColour != 0 )
			{
				if ( uiColour & 0x10 )
					return ( uiOtherColour & 0x10 ) == 0;

				if ( uiOtherColour != 0 )
					return TTRUE;
			}
			break;
		case 1:
			if ( uiColour & 0x10 )
				return TTRUE;

			if ( uiOtherColour != 0 && ( uiOtherColour & 0x10 ) == 0 )
				return TTRUE;
			break;
		case 3:
			if ( uiColour != 0 && a_pOther->m_pColourComponent->GetUnk20() != m_pColourComponent->GetUnk20() )
			{
				if ( uiColour & 0x10 )
					return ( uiOtherColour & 0x10 ) == 0;

				return uiOtherColour != 0;
			}
			break;
		default:
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00747920
TBOOL ABlobEntity::LosesBlobBump( ABlobEntity* a_pOther )
{
	if ( TUINT8( m_uiFlags >> 22 ) & 1 )
		return TFALSE;

	if ( m_pZJump->GetState() == ABlobZJumpComponent::STATE_ATTACKFLIGHT )
	{
		ABlobZJumpComponent::PriorityTarget* pLock = m_pZJump->GetCurrentPriorityLock();

		if ( pLock != TNULL && pLock->m_eType == 0 && TSTATICCAST( ABaseEntity, pLock->m_pTarget )->IsFlagged( FLAG_Player ) )
		{
			if ( m_iSize > a_pOther->m_iSize )
				return TFALSE;

			if ( m_iSize == a_pOther->m_iSize && m_oPhysicsMatrix.m_f42 < a_pOther->m_oPhysicsMatrix.m_f42 )
				return TFALSE;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00747990
void ABlobEntity::GetInked( ABlobEntity* a_pAttacker )
{
	TUINT32 uiFlags = m_uiFlags;

	if ( !ms_bControlsLocked && ( uiFlags & 0x4080000 ) == 0 && ( uiFlags & 0x200000 ) == 0 && ( uiFlags & 0x10000 ) == 0 && m_eState != STATE_DEAD )
	{
		m_uiFlags = uiFlags | 0x20000000;
		InkBounce();
		ChangeColour( 0x10 );

		if ( m_eState == STATE_DEAD )
		{
			m_uiFlags &= 0xdfffffff;
			return;
		}

		*TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 2 ] ) = ms_pBlobCommonProperties->m_fInkTime / 10.0f; // 0x588
		RemovePaintPoints( 1, TTRUE );
		m_uiFlags |= 0x10000;
		*TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 0 ] ) = 0.7f; // 0x580
		m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_PAIN );
	}
}

// $deBlob: FUNCTION 00747a60
void ABlobEntity::StartRainbow()
{
	TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

	if ( TUINT8( uiColour >> 4 ) & 1 )
	{
		WashOff();
		ChangeColour( 2 );
	}
	else if ( uiColour == 0 )
	{
		ChangeColour( 2 );
	}

	m_pSoundComponent->PlayRainbow();

	m_uiFlags |= 0x200000;
	*TREINTERPRETCAST( TFLOAT*, &m_auiUnk5A8[ 5 ] ) = ms_pBlobCommonProperties->m_afUnk100[ 0 ]; // 0x5BC the time left
	m_auiUnk5A8[ 4 ]                                = 0;                                         // 0x5B8
	m_uiUnk5C0                                      = m_iPaintPoints;                            // the points to restore
	m_iPaintPoints                                  = 100;

	if ( ( TUINT8( m_uiFlags >> 27 ) & 1 ) == 0 )
		m_iSize = ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) ? 10 : 9;

	ADecalManager::GetSingleton()->SetRainbow( TTRUE );
	m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_POWERUP );

	if ( m_pZJump->GetState() == ABlobZJumpComponent::STATE_ATTACKSLAM )
		m_pZJump->OnLanded();

	TParticleEffectInstance*& rpRainbow = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 15 ] ); // 0xB14

	if ( ms_pRainbowEffect != TNULL && rpRainbow == TNULL )
	{
		rpRainbow = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pRainbowEffect );

		if ( rpRainbow != TNULL )
		{
			rpRainbow->GetTransform().SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
			rpRainbow->SetFinishedCallback( OnBlobEffectFinished, &m_auiSounds[ 0 ] );
		}
	}

	// Held in place while the rainbow animation (0x1F) plays
	m_vHoldPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	TPhysicsManager::StopBody( m_pBody );
}

// $deBlob: FUNCTION 00747be0
void ABlobEntity::EndRainbow()
{
	if ( ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) == 0 )
		return;

	ABlobSoundComponent::ReleaseRainbowSnapshot();
	m_uiFlags &= ~0x200000;
	m_auiUnk5A8[ 5 ] = 0; // 0x5BC the time left
	m_auiUnk5A8[ 4 ] = 0; // 0x5B8
	SetPaintPoints( m_uiUnk5C0 );
	ADecalManager::GetSingleton()->SetRainbow( TFALSE );

	TParticleEffectInstance* pRainbow = TREINTERPRETCAST( TParticleEffectInstance*, m_auiSounds[ 15 ] ); // 0xB14

	if ( pRainbow != TNULL )
		pRainbow->Stop();
}

// $deBlob: FUNCTION 00747c50
void ABlobEntity::SetShield( TBOOL a_bShield )
{
	if ( a_bShield )
	{
		if ( !( m_uiFlags & 0x400000 ) )
		{
			m_pSoundComponent->PlaySqueeze();
			m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_SHIELD_ACTIVATING );
		}

		m_uiFlags |= 0x400000;
	}
	else
	{
		if ( m_uiFlags & 0x400000 )
			m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_SHIELD_DEACTIVATING );

		m_uiFlags &= ~0x400000;
	}
}

// $deBlob: FUNCTION 00747cc0
TBOOL ABlobEntity::IsShielded() const
{
	TBOOL bShielded = ( m_uiFlags >> 22 ) & 1;
	return bShielded;
}

// $deBlob: FUNCTION 00747cd0
void ABlobEntity::PushDown()
{
	TVector4 vVelocity( 0.0f, -80.0f, 0.0f, 1.0f );

	const TMatrix44* pBodyMatrix = m_pBody ? m_pBody->m_pMatrix : TNULL;
	TVector4         vPosition   = pBodyMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION );
	vPosition.y -= 0.1f;

	SetPhysicsPosition( vPosition );
	TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );
}

// $deBlob: FUNCTION 00747d40
TBOOL ABlobEntity::CanEnterBlobVision()
{
	TXUI*      pXUI         = TXUI::GetSingleton();
	TXUIScene* pActiveScene = pXUI->GetActiveScene();

	if ( m_eState != STATE_ROLL || m_bInBlobVision || pActiveScene == TNULL )
		return TFALSE;

	if ( !pActiveScene->IsA( &TGetClass( ABlobHud ) ) || pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked() )
		return TFALSE;

	if ( APlayerManager::GetSingleton()->GetNumPlayers() != 1 || m_pZJump->GetState() != ABlobZJumpComponent::STATE_NONE || m_pAnimationComponent->GetState() == ABlobAnimationComponent::STATE_GRASSBOMB )
		return TFALSE;

	if ( ACameraManager::GetSingleton()->GetCurrentCamera()->GetCameraType() == ACamera::CameraType( 3 ) || ( TUINT8( m_uiFlags >> 28 ) & 1 ) || ( TUINT8( m_uiFlags >> 30 ) & 1 ) )
		return TFALSE;

	AChallengeManager* pChallengeManager = AChallengeManager::GetSingleton();

	if ( pChallengeManager->IsEnding() )
		return TFALSE;

	AChallenge* pChallenge = pChallengeManager->GetCurrentChallenge();
	return pChallenge == TNULL || !( pChallenge->GetFlags() & AChallenge::Flags_Starting );
}

// $deBlob: FUNCTION 00748a50
void ABlobEntity::StartShockwave( const TVector4& a_rPosition, const TVector4& a_rNormal, TFLOAT a_fTime, TFLOAT a_fAlpha, TBOOL a_bOrient )
{
	if ( m_pShockwaveModel == TNULL || m_fShockwaveTime > 0.0f )
		return;

	m_fShockwaveTime     = a_fTime;
	m_fShockwaveDuration = a_fTime;
	m_fShockwaveAlpha    = a_fAlpha;

	// 0.6 below the position
	TVector4 vOffset;
	vOffset.Multiply( TVector4::VEC_NEGY, 0.6f );
	m_vShockwavePosition = TVector4( a_rPosition.x + vOffset.x, a_rPosition.y + vOffset.y, a_rPosition.z + vOffset.z, a_rPosition.w );
	m_vShockwaveNormal   = a_rNormal;

	TTransformObject& rTransform = TSTATICCAST( TModelInstance, m_pShockwaveModel )->GetTransform();
	rTransform.SetMatrix( TMatrix44::IDENTITY );
	rTransform.SetScale( 0.01f, 0.01f, 0.01f );
	rTransform.SetTranslate( m_vShockwavePosition.AsVector3() );

	if ( a_bOrient )
	{
		// Faces along the normal
		TMatrix44 oMatrix = TMatrix44::IDENTITY;
		oMatrix.LookAtDirection( m_vShockwaveNormal, TVector4::VEC_NEGY );
		oMatrix.RotateX( -TMath::HALF_PI );
		m_vShockwavePosition.w = 1.0f;
		oMatrix.SetTranslation( m_vShockwavePosition );
		TSTATICCAST( TModelInstance, m_pShockwaveModel )->GetTransform().SetMatrix( oMatrix );
	}
}

// $deBlob: FUNCTION 00748c30
void ABlobEntity::UpdateShockwave( const TVector4& a_rPosition, const TVector4& a_rNormal )
{
	if ( m_pShockwaveModel == TNULL || m_fShockwaveTime <= 0.0f )
		return;

	m_vShockwavePosition = a_rPosition;
	m_vShockwaveNormal   = a_rNormal;

	// Faces along the normal
	TMatrix44 oMatrix = TMatrix44::IDENTITY;
	oMatrix.LookAtDirection( m_vShockwaveNormal, TVector4::VEC_NEGY );
	oMatrix.RotateX( -TMath::HALF_PI );
	m_vShockwavePosition.w = 1.0f;
	oMatrix.SetTranslation( m_vShockwavePosition );
	TSTATICCAST( TModelInstance, m_pShockwaveModel )->GetTransform().SetMatrix( oMatrix );
}

// $deBlob: FUNCTION 00748d50
void ABlobEntity::ShockwaveRenderCallback( void* a_pBlob )
{
	TSTATICCAST( ABlobEntity, a_pBlob )->RenderShockwave();
}

// $deBlob: FUNCTION 00748d60
void ABlobEntity::RenderShockwave()
{
	if ( m_pShockwaveModel == TNULL || m_fShockwaveTime <= 0.0f )
		return;

	TWorldShaderHAL::ResetWaterFrame();
	s_bWaterFrameReset2 = TFALSE;

	TRenderContext* pContext  = TRender::GetSingleton()->GetCurrentContext();
	TFLOAT          fOldAlpha = pContext->GetAlphaBlend();

	// Fades out over the last part of its time
	TFLOAT fAlpha    = m_fShockwaveAlpha;
	TFLOAT fFadeTime = ( 1.0f - ms_fShockwaveFadeStart ) * m_fShockwaveDuration;

	if ( fFadeTime > m_fShockwaveTime )
		fAlpha = TMath::LERP( 0.0f, m_fShockwaveAlpha, m_fShockwaveTime / fFadeTime );

	pContext->SetAlphaBlend( fAlpha );

	TModelInstance* pModelInstance = TSTATICCAST( TModelInstance, m_pShockwaveModel );
	pModelInstance->GetTransform().Push();

	TWorldMesh* pMesh = TSTATICCAST( TWorldMesh, pModelInstance->GetModel()->GetLOD( 0 ).GetMesh( 0 ) );
	pMesh->RenderWithCallback( ShockwavePacketCallback, this );

	pModelInstance->GetTransform().Pop();
	pContext->SetAlphaBlend( fOldAlpha );
}

// $deBlob: FUNCTION 00750eb0
TFLOAT ABlobEntity::GetLaunchSpeed( TFLOAT a_fHeight, TFLOAT a_fGravity )
{
	TFLOAT fTime = TFLOAT( sqrt( TDOUBLE( ( a_fHeight * 2.0f ) / a_fGravity ) ) );
	return ( a_fGravity * 0.5f ) * fTime + a_fHeight / fTime;
}

// $deBlob: FUNCTION 00750f10
void __stdcall ABlobEntity::ShockwavePacketCallback( TRenderPacket* a_pPacket, void* a_pBlob, TBOOL a_bBefore )
{
	TWorldMaterial::BLENDMODE eBlendMode = TSTATICCAST( TWorldMaterial, a_pPacket->GetMaterial() )->GetBlendMode();

	if ( eBlendMode == TWorldMaterial::BLENDMODE_SHOCKWAVE )
	{
		s_aiTexCoordGen[ 0 ] = 12;
	}
	else if ( eBlendMode == TWorldMaterial::BLENDMODE_REFLECTIVEWATER )
	{
		TSTATICCAST( ABlobEntity, a_pBlob )->m_pShockwaveNoise->Bind( 1 );

		TVector4 vIndirect( 0.6f, TFLOAT( pow( 2.0, -2.0 ) ), 0.0f, 0.0f );
		TRenderDX11::Interface()->SetVec4InPSBuffer( 0x12, &vIndirect, 1 );
		s_iIndirect = 2;
	}
}

// $deBlob: FUNCTION 00748990
TBOOL ABlobEntity::IsEvilBlob()
{
	return m_pColourComponent->GetCurrentColour() == 0 && AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 && !APlayerManager::GetSingleton()->IsNoBlobPainted();
}

// $deBlob: FUNCTION 00749380
TBOOL ABlobEntity::IsZJumpingAtPlayer()
{
	if ( m_pZJump->GetState() == ABlobZJumpComponent::STATE_ATTACKFLIGHT )
	{
		ABlobZJumpComponent::PriorityTarget* pLock = m_pZJump->GetCurrentPriorityLock();

		if ( pLock != TNULL && pLock->m_eType == 0 && TSTATICCAST( ABaseEntity, pLock->m_pTarget )->IsFlagged( FLAG_Player ) )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00751be0
void ABlobEntity::RefillPaint()
{
	TUINT uiPoints = m_iPaintPoints + 10;
	SetPaintPoints( TMath::Min( uiPoints, 100U ) );
}

// Moves the body up from its position (by 0.1) with a_rcDirection (minus a_fDown in Y) * 20 as its velocity
static void BlobKnockBack( ABlobEntity* a_pBlob, Toshi::TPhysicsObjectContainer* a_pBody, const TVector4& a_rcVelocity )
{
	const TMatrix44* pBodyMatrix = a_pBody ? a_pBody->m_pMatrix : TNULL;
	TVector4         vPosition   = pBodyMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION );
	vPosition.y -= 0.1f;

	a_pBlob->SetPhysicsPosition( vPosition );
	TPhysicsManager::SetLinearVelocity( a_pBody, a_rcVelocity );
}

// $deBlob: FUNCTION 00743de0
void ABlobEntity::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
			// A wind cannon offers itself
			if ( a_pCaller->GetClass() == &TGetClass( APropWindCannon ) )
			{
				m_pWindCannon = TSTATICCAST( APropWindCannon, a_pCaller );
				return;
			}

			break;
		case AEntityIOPacket::MESSAGE_UNTOUCH:
			if ( a_pCaller == m_pWindCannon )
			{
				m_pWindCannon->OnBlobLeaving();
				m_pWindCannon = TNULL;
			}

			break;
		case AEntityIOPacket::MESSAGE_TOUCH:
			if ( ( m_uiFlags & 0x10000 ) == 0 && ( m_pColourComponent->GetCurrentColour() & 0x10 ) == 0 )
			{
				TASSERT( a_pCaller != this );

				if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() >= 0 && a_pCaller->GetClass() == &TGetClass( ABlobEntity ) )
				{
					ABlobEntity* pOther = a_pCaller->CastTo<ABlobEntity>();

					if ( ( pOther->m_pColourComponent->GetCurrentColour() & 0x10 ) && !IsEvilBlob() )
					{
						GetInked( pOther );
						return;
					}
				}
			}
			break;
		case AEntityIOPacket::MESSAGE_TOUCHEDBY:
			// An unpainted civilian shell
			if ( m_fUnk59C <= 0.0f && a_pCaller->GetClass() == &TGetClass( ACiv ) && TSTATICCAST( ACiv, a_pCaller )->IsTransformed() )
			{
				m_fAwakeDuration = 0.0f;
				m_fAwakeTimer    = 0.0f;
				return;
			}
			break;
		case AEntityIOPacket::MESSAGE_SLAM:
			if ( m_eState != STATE_CANNON )
			{
				InkBounce();

				if ( ( m_uiFlags & 0x10000 ) == 0 )
				{
					const AAttackMessage* pAttackMessage = TREINTERPRETCAST( const AAttackMessage*, a_rPacket.m_iData ); // sent by the Z-jumping blob (OnCollisionProcess)

					TASSERT( a_pCaller != this );
					TASSERT( a_pCaller->IsExactly( &TGetClass( ABlobEntity ) ) );

					ABlobEntity* pOther  = a_pCaller->CastTo<ABlobEntity>();
					TINT         iMpType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();

					if ( iMpType >= 0 && pOther->CanInkBlob( this ) )
					{
						TBOOL bShieldHit = TFALSE;

						switch ( iMpType )
						{
							case 0:
							case 2:
							case 3:
								if ( ( pOther->m_pColourComponent->GetCurrentColour() & 0x10 ) && ( m_pColourComponent->GetCurrentColour() & 0x10 ) == 0 )
								{
									GetInked( pOther );
								}
								else if ( LosesBlobBump( pOther ) )
								{
									m_uiFlags |= 0x10000;
									*TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 0 ] ) = 1.0f; // 0x580

									TINT iKilled = APlayerManager::GetSingleton()->GetPlayerIndex( this );
									TINT iKiller = APlayerManager::GetSingleton()->GetPlayerIndex( pOther );
									RemovePaintPoints( 10, TFALSE );
									AGameStatus::GetSingleton()->OnPlayerKilled( iKiller, iKilled );
									WashOff();
								}
								else if ( TUINT8( m_uiFlags >> 22 ) & 1 )
								{
									m_pSoundComponent->PlayShieldHit();
									pOther->WashOff();
									bShieldHit = TTRUE;
								}
								break;
							case 1:
								if ( TUINT8( pOther->m_pColourComponent->GetCurrentColour() >> 4 ) & 1 )
								{
									GetInked( pOther );
								}
								else if ( TUINT8( m_uiFlags >> 22 ) & 1 )
								{
									m_pSoundComponent->PlayShieldHit();
									bShieldHit = TTRUE;
								}
								else
								{
									TINT iKilled = APlayerManager::GetSingleton()->GetPlayerIndex( this );
									TINT iKiller = APlayerManager::GetSingleton()->GetPlayerIndex( pOther );
									AGameStatus::GetSingleton()->OnPlayerKilled( iKiller, iKilled );

									TUINT32 uiColour = pOther->m_pColourComponent->GetStartColour();
									AGameStatus::GetSingleton()->OnBlobInked( this, pOther );
									pOther->SetPaintPoints( m_iPaintPoints );
									pOther->ChangeColour( uiColour );
									pOther->m_pColourComponent->SetColour( uiColour );
									RemovePaintPoints( 10, TFALSE );
									WashOff();
									OnOutOfPoints( TFALSE );
									SetPaintPoints( 10 );
									SetPaintPoints( 30 );
								}
								break;
						}

						if ( bShieldHit )
						{
							// The other blob bounces off this one's shield
							pOther->PushDown();

							TParticleEffectInstance*& rpShieldHit = *TREINTERPRETCAST( TParticleEffectInstance**, &m_auiSounds[ 13 ] ); // 0xB0C

							if ( rpShieldHit != TNULL )
								rpShieldHit->StopInstant();

							rpShieldHit = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pMPShieldHitEffect );

							TMatrix44 oMatrix( TMatrix44::IDENTITY );
							oMatrix.SetBasisVector4( BASISVECTOR_TRANSLATION, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
							TFLOAT fScale = ms_pBlobProperties[ m_iSize ].m_fRadius * 0.5f;
							oMatrix.Scale( fScale, fScale, fScale );

#ifdef DEBLOB_COMMUNITY_PATCH
							// NOTE: retail writes through the instance without checking that the effect started
							if ( rpShieldHit != TNULL )
#endif
								rpShieldHit->GetTransform() = oMatrix;
						}
					}

					TVector4 vVelocity = pAttackMessage->m_vDirection;
					vVelocity.y -= 0.3f;
					vVelocity.Multiply( 20.0f );
					BlobKnockBack( this, m_pBody, vVelocity );
					return;
				}
			}
			break;
		case AEntityIOPacket::MESSAGE_RECOIL:
			if ( m_eState != STATE_DEAD )
			{
				m_pZJump->OnRecoil();

				AAttackMessage* pAttackMessage = TREINTERPRETCAST( AAttackMessage*, a_rPacket.m_iData );
				TASSERT( pAttackMessage->m_eType == AAttackMessage::ATTACK_SlamFailedRecoil );

				TVector4 vVelocity;
				vVelocity.Multiply( pAttackMessage->m_vDirection, ms_pBlobCommonProperties->m_fJumpVelocity );
				TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );

				m_uiFlags = ( m_uiFlags & 0xfff9ffff ) | 0x8000;
				SetState( STATE_BALLISTIC );
				return;
			}
			break;
		case AEntityIOPacket::MESSAGE_ENEMYATTACK:
		case AEntityIOPacket::MESSAGE_PROPATTACK:
		case AEntityIOPacket::MESSAGE_RAMATTACK:
			if ( !ms_bControlsLocked && ( m_uiFlags & 0x4080000 ) == 0 && ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) == 0 && !IsEvilBlob() && ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) == 0 && ( m_uiFlags & 0x10000 ) == 0 )
			{
				InkBounce();
				ChangeColour( 0x10 );
				*TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 2 ] ) = ms_pBlobCommonProperties->m_fInkTime / 10.0f; // 0x588
				RemovePaintPoints( 1, TTRUE );

				AAttackMessage* pAttackMessage = TREINTERPRETCAST( AAttackMessage*, a_rPacket.m_iData );
				TASSERT( a_pCaller->IsA( &TGetClass( AVisibleEntity ) ) );

				TParticleEffect* pHitEffect = TSTATICCAST( TParticleEffect, TSTATICCAST( AVisibleEntity, a_pCaller )->GetHitEffect() );

				if ( pHitEffect != TNULL )
				{
					TParticleEffectInstance* pInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( pHitEffect );

					if ( pInstance != TNULL )
					{
						TMatrix44 oMatrix( TMatrix44::IDENTITY );
						oMatrix.SetBasisVector4( BASISVECTOR_TRANSLATION, pAttackMessage->m_vPosition );
						pInstance->GetTransform() = oMatrix;
					}
				}

				if ( m_eState != STATE_DEAD )
				{
					TVector4 vVelocity = pAttackMessage->m_vDirection;

					if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_RAMATTACK )
					{
						vVelocity.Multiply( 20.0f );
						vVelocity.y -= ms_pBlobCommonProperties->m_fJumpVelocity;
					}
					else
					{
						vVelocity.y -= 0.3f;
						vVelocity.Multiply( 20.0f );
					}

					BlobKnockBack( this, m_pBody, vVelocity );
					m_uiFlags |= 0x10000;
					*TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 0 ] ) = 0.7f; // 0x580
					m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_PAIN );
					return;
				}
			}
			break;
		case AEntityIOPacket::MESSAGE_DEATHZONE:
			if ( m_eState != STATE_DEAD )
			{
				OnOutOfPoints( TFALSE );
				SetPaintPoints( 10 );
				TPhysicsManager::StopBody( m_pBody );
				return;
			}
			break;
		case AEntityIOPacket::MESSAGE_WIND:
		{
			// m_iData: the velocity (0x00), the position (0x10), the type (0x20) and whether to move there (0x24)
			TUINT8*         pData     = TREINTERPRETCAST( TUINT8*, a_rPacket.m_iData );
			const TVector4& rVelocity = *TREINTERPRETCAST( TVector4*, pData );

			if ( *TREINTERPRETCAST( TINT*, pData + 0x20 ) == 3 )
			{
				if ( *TREINTERPRETCAST( TBOOL*, pData + 0x24 ) )
				{
					SetPhysicsPosition( *TREINTERPRETCAST( TVector4*, pData + 0x10 ) );
					SetState( STATE_BALLISTIC );
					ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 3, m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 1.0f, -1 );
				}

				*TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 0 ] ) = 0.1f; // 0x580
				m_uiFlags                                       = ( m_uiFlags & 0xfff9ffff ) | 0x10000;
				TPhysicsManager::SetLinearVelocity( m_pBody, rVelocity );
			}
			else
			{
				TPhysicsManager::GetSingleton()->ApplyForce( m_pBody, rVelocity );
			}

			m_uiFlags |= 0x1000000;
			return;
		}
	}
}

// $deBlob: FUNCTION 0073c870
// Stops the blob's effect instances (0xAE8-0xB18, this + 0xAD8 + 0x10..0x40)
static void StopBlobEffects( TUINT32* a_pEffects )
{
	for ( TINT i = 4; i <= 16; i++ )
	{
		if ( a_pEffects[ i ] != 0 )
		{
			TREINTERPRETCAST( TParticleEffectInstance*, a_pEffects[ i ] )->StopInstant();
			a_pEffects[ i ] = 0;
		}
	}
}

// $deBlob: FUNCTION 007453c0
void ABlobEntity::Reset( ABaseEntity* a_pEntity )
{
	SetState( STATE_BALLISTIC );
	m_fShockwaveTime          = 0.0f;
	m_fShockwaveDuration      = 0.0f;
	m_fShockwaveAlpha         = 1.0f;
	ms_bControlsLocked        = TFALSE;
	m_pBoss                   = TNULL;
	m_iCurrentLandmarkTrigger = -1;

	if ( ATerrain2Interface::GetInstance()->IsPositionInReadyCell( m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), TFALSE ) )
	{
		if ( m_eState != STATE_DEAD )
		{
			TPhysicsManager::GetSingleton()->AddToSimulation( m_pBody, TTRUE );

			if ( m_pBody != TNULL && m_pBody->IsInSimulation() )
				m_pBody->m_body->activate();
		}

		m_uiFlags &= 0xfff7ffdf;
	}
	else
	{
		m_uiFlags |= 0x20;
	}

	m_iZPlayerAttackStage                              = 0;    // 0xB24
	m_fZPlayerAttackTime                               = 0.0f; // 0xB28
	m_bHasRespawnRotation                              = 0;
	m_vRespawnRotation                                 = TVector4::VEC_ZERO;
	m_bRespawnPlaySeq                                  = 0;
	*TREINTERPRETCAST( TUINT8*, &m_uiRespawnTransSeq ) = 0; // NOTE: retail clears 0xB57 as a TUINT16
	m_uiRespawnSeqStartRatio                           = 0;
	m_fRespawnSeqEndRatio                              = 1.0f;

	Respawn( TTRUE, TFALSE, TNULL );
	m_pColourComponent->Reset();
	m_pZJump->Reset();

	// The initial size of the level
	TINT iSize          = ALevelInformation::GetSingleton()->GetLevelHeader()->m_pLevelProps[ AGameStatus::GetSingleton()->GetCurrentLevel() ].m_auiMedalLimits[ 7 ];
	m_iSize             = iSize;
	m_iOldSize          = iSize;
	m_iPaintPoints      = ( iSize * 5 + 5 ) * 2;
	m_oMotion           = ABlobMotionData();
	m_fUntargetableTime = -1.0f;
	m_uiTargettingAndShakes &= 0xFFFF0000; // NOTE: retail clears it as a TUINT16
	m_iZPlayerAttackStage   = 0;           // 0xB24
	m_fZPlayerAttackTime    = 0.0f;        // 0xB28
	m_auiUnk5A8[ 2 ]        = 0;           // 0x5B0
	m_auiUnk5A8[ 0 ]        = 0;           // 0x5A8
	m_auiUnk5A8[ 1 ]        = 0;           // 0x5AC
	m_iLeechbotsAttached    = 0;           // 0xAC8
	m_iMaxLeechbotsAttached = 0;

	if ( m_iCamShakeID > -1 )
	{
		ACameraManager::GetSingleton()->GetShakeFunction().StopShake( m_iCamShakeID );
		m_iCamShakeID = -1;
	}

	m_fUnkB34 = 0.0f;
	SetVisible( TTRUE );
	m_pAnimationComponent->Reset();
	m_uiLandmarkTime = 0;
	StopBlobEffects( m_auiSounds );

	EndRainbow();

	if ( m_uiFlags & 0x400000 )
		m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_SHIELD_DEACTIVATING );

	m_uiFlags &= 0xffbfffff;

	if ( m_pSizeHud != TNULL )
	{
		m_pSizeHud->Reset();
		m_pSizeHud->SetSize( m_iPaintPoints );

		// Multiplayer game type 1 while no blob has painted yet: the blob starts as the evil (ink) blob
		if ( m_pColourComponent->GetCurrentColour() == ABlobColourComponent::COLOUR_WHITE &&
		     AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 && !APlayerManager::GetSingleton()->IsNoBlobPainted() )
		{
			m_pSizeHud->SetColour( ABlobColourComponent::ms_auiColours[ ABlobColourComponent::COLOUR_BLACK ], TFALSE, 0.0f );
			m_pSizeHud->GetVisual()->PlayAnimation( L"EvilBlob", L"EndEvilBlob" );
		}
		else
		{
			TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

			if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
				uiColour = ABlobColourComponent::COLOUR_BLACK;

			m_pSizeHud->SetColour( ABlobColourComponent::ms_auiColours[ uiColour ], TFALSE, 0.0f );
		}
	}

	m_uiFlags     = 0;
	m_uiPrevFlags = 0;
	m_pNavRing->Hide();
	m_pHelpTrigger       = TNULL;
	m_pHudMessageTrigger = TNULL;
	m_bInBlobVision      = 0;
	m_bLifeLostShown     = 0;
	m_pHotPlate          = TNULL;
	m_pWindCannon        = TNULL;
	m_bUnkB64            = TFALSE;
	m_auiUnk5C4[ 1 ]     = 0; // 0x5C8
	m_uiUnkB68           = 0;

	m_pSoundComponent->SetSoundsEnabled( TTRUE );

	m_pSoundComponent->StopLoops();
	m_iStartColour      = -1;
	m_iStartPaintPoints = -1;
	m_Flags             = ( m_Flags & 0xffffe1ff ) | 0x200;
}

// $deBlob: FUNCTION 00744930
void ABlobEntity::Render()
{
	// The paint threads: camera facing quads from the anchor to the end on the ball
	if ( m_pAnimationComponent->m_iAnimFlags & 1 )
	{
		const TMatrix44& rCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
		TFLOAT           fBackX  = -rCamera.m_f31;
		TFLOAT           fBackY  = -rCamera.m_f32;
		TFLOAT           fBackZ  = -rCamera.m_f33;

		for ( TINT i = 0; i < 16; i++ )
		{
			Thread& rThread = m_aThreads[ i ];

			if ( rThread.m_uiEndColour == 0 )
				continue;

			TMatrix44 mQuad;
			mQuad.Identity();
			mQuad.m_f31 = rThread.m_vAnchor.x - rThread.m_vEnd.x;
			mQuad.m_f32 = rThread.m_vAnchor.y - rThread.m_vEnd.y;
			mQuad.m_f33 = rThread.m_vAnchor.z - rThread.m_vEnd.z;

			TFLOAT fLength = TVector3( mQuad.m_f31, mQuad.m_f32, mQuad.m_f33 ).Magnitude();

			if ( fLength < 0.05f )
				continue;

			TFLOAT fInvLength = 1.0f / fLength;
			mQuad.m_f31 *= fInvLength;
			mQuad.m_f32 *= fInvLength;
			mQuad.m_f33 *= fInvLength;

			// Thinner the longer it is
			static TFLOAT s_fThreadWidth = 0.3f;
			TFLOAT        fWidth         = TMath::Min( s_fThreadWidth / fLength, 0.2f );

			mQuad.m_f12 = mQuad.m_f33 * fBackX - mQuad.m_f31 * fBackZ;
			mQuad.m_f13 = mQuad.m_f31 * fBackY - mQuad.m_f32 * fBackX;
			mQuad.m_f11 = mQuad.m_f32 * fBackZ - mQuad.m_f33 * fBackY;
			mQuad.m_f22 = mQuad.m_f31 * mQuad.m_f13 - mQuad.m_f33 * mQuad.m_f11;
			mQuad.m_f23 = mQuad.m_f32 * mQuad.m_f11 - mQuad.m_f31 * mQuad.m_f12;
			mQuad.m_f21 = mQuad.m_f33 * mQuad.m_f12 - mQuad.m_f32 * mQuad.m_f13;

			TSysMesh::TLockBuffer oLockBuffer;
			oLockBuffer.oIndices.uiOffset = 0;
			oLockBuffer.oIndices.pBuffer  = TNULL;
			rThread.m_pMesh->Lock( &oLockBuffer );

			TASSERT( oLockBuffer.oIndices.pBuffer != TNULL && "m_pLockedBuffer!=TNULL" );
			TIndexType* pIndices = oLockBuffer.oIndices.pBuffer;
			TASSERT( oLockBuffer.oVertices.uiNumStreams != 0 && "a_uiBuffer<m_uiNumStreams" );
			ThreadVertex* pVertices = TREINTERPRETCAST( ThreadVertex*, oLockBuffer.oVertices.apStreams[ 0 ] );

			pVertices[ 0 ].vPosition = TVector3( -fWidth, 0.0f, 0.0f );
			TMatrix44::TransformVector( pVertices[ 0 ].vPosition, mQuad, pVertices[ 0 ].vPosition );
			pVertices[ 0 ].vPosition.x += rThread.m_vAnchor.x;
			pVertices[ 0 ].vPosition.y += rThread.m_vAnchor.y;
			pVertices[ 0 ].vPosition.z += rThread.m_vAnchor.z;

			pVertices[ 1 ].vPosition = TVector3( -fWidth, 0.0f, 0.0f );
			TMatrix44::TransformVector( pVertices[ 1 ].vPosition, mQuad, pVertices[ 1 ].vPosition );
			pVertices[ 1 ].vPosition.x += rThread.m_vEnd.x;
			pVertices[ 1 ].vPosition.y += rThread.m_vEnd.y;
			pVertices[ 1 ].vPosition.z += rThread.m_vEnd.z;

			pVertices[ 2 ].vPosition = TVector3( fWidth, 0.0f, 0.0f );
			TMatrix44::TransformVector( pVertices[ 2 ].vPosition, mQuad, pVertices[ 2 ].vPosition );
			pVertices[ 2 ].vPosition.x += rThread.m_vAnchor.x;
			pVertices[ 2 ].vPosition.y += rThread.m_vAnchor.y;
			pVertices[ 2 ].vPosition.z += rThread.m_vAnchor.z;

			pVertices[ 3 ].vPosition = TVector3( fWidth, 0.0f, 0.0f );
			TMatrix44::TransformVector( pVertices[ 3 ].vPosition, mQuad, pVertices[ 3 ].vPosition );
			pVertices[ 3 ].vPosition.x += rThread.m_vEnd.x;
			pVertices[ 3 ].vPosition.y += rThread.m_vEnd.y;
			pVertices[ 3 ].vPosition.z += rThread.m_vEnd.z;

			pVertices[ 0 ].vUV = TVector2( 0.0f, 0.0f );
			pVertices[ 1 ].vUV = TVector2( 0.0f, 1.0f );
			pVertices[ 2 ].vUV = TVector2( 1.0f, 0.0f );
			pVertices[ 3 ].vUV = TVector2( 1.0f, 1.0f );

			// NOTE: retail fades the stored alpha of the anchor's colour with the ball every frame
			TUINT8* pAnchorAlpha = TREINTERPRETCAST( TUINT8*, &rThread.m_uiAnchorColour ) + 3;
			*pAnchorAlpha        = TUINT8( lrintf( TFLOAT( *pAnchorAlpha ) * m_pAnimationComponent->m_pBallBodyModelI->GetModelInstance()->GetFade() ) );

			pVertices[ 0 ].uiColour = rThread.m_uiAnchorColour;
			pVertices[ 1 ].uiColour = rThread.m_uiEndColour;
			pVertices[ 2 ].uiColour = rThread.m_uiAnchorColour;
			pVertices[ 3 ].uiColour = rThread.m_uiEndColour;

			pIndices[ 0 ] = 0;
			pIndices[ 1 ] = 1;
			pIndices[ 2 ] = 2;
			pIndices[ 3 ] = 3;

			rThread.m_pMesh->Unlock( 4, 4 );
			rThread.m_pMesh->Validate();
			rThread.m_pMesh->Render();
		}
	}

	m_pRibbonTrail->Render();
	m_pAnimationComponent->Render();
}

// $deBlob: FUNCTION 00746df0
void ABlobEntity::LoadEffects()
{
	TParticleEffectManager* pManager = TParticleSystem::GetSingleton()->GetEffectManager();

	ms_pPaintSplashEffect           = pManager->FindEffect( "Blob_PaintSplash" );
	ms_pElectrocutionEffect         = pManager->FindEffect( "Blob_Electrocution" );
	ms_pHotPlateEffect              = pManager->FindEffect( "Blob_HotPlate" );
	ms_pInkedStinkEffect            = pManager->FindEffect( "Blob_InkedStink" );
	ms_pInkSplashEffect             = pManager->FindEffect( "Blob_InkSplash_med" );
	ms_pWaterSplashEffect           = pManager->FindEffect( "Blob_WaterSplash_med" );
	ms_pWaterSplashSlamEffect       = pManager->FindEffect( "Blob_WaterSplash_Slam" );
	ms_pWaterRippleEffect           = pManager->FindEffect( "Blob_WaterRipple" );
	ms_pLandmarkEffect              = pManager->FindEffect( "Blob_Landmark" );
	ms_pDeathPopSplatEffect         = pManager->FindEffect( "Blob_Death_Pop_Splat" );
	ms_pDeathPopSprayEffect         = pManager->FindEffect( "Blob_Death_Pop_Spray" );
	ms_pUnpaintableEffect           = pManager->FindEffect( "Blob_Unpaintable" );
	ms_pWindLauncherTrailEffect     = pManager->FindEffect( "Blob_WindLauncher_Trail" );
	ms_pRainbowEffect               = pManager->FindEffect( "Blob_Rainbow" );
	ms_pZJumpFailEffect             = pManager->FindEffect( "Blob_ZJump_Fail" );
	ms_pSpikeHaemorrhageClearEffect = pManager->FindEffect( "Blob_SpikeHaemorrhageClear" );
	ms_pMPShieldLoopEffect          = pManager->FindEffect( "Blob_MP_Shield_Loop" );
	ms_pMPShieldHitEffect           = pManager->FindEffect( "Blob_MP_Shield_Hit" );
	ms_pElectricTrapSparksEffect    = pManager->FindEffect( "Blob_ElectricTrapSparks" );
	ms_pLandmarkPaintSplatEffect    = pManager->FindEffect( "Blob_Landmark_PaintSplat" );

	TParticleEffect* pInkCloud = pManager->FindEffect( "Blob_InkCloudUnderwater" );

	// An ink cloud per colour (none for the first colour)
	ms_apInkCloudEffects       = TSTATICCAST( TParticleEffect*, TMemalign( 16, ABlobColourComponent::COLOUR_NUMOF * sizeof( TParticleEffect* ) ) );
	ms_apInkCloudEffects[ 16 ] = pInkCloud;
	ms_apInkCloudEffects[ 0 ]  = TNULL;
	ms_apInkCloudEffects[ 2 ]  = pInkCloud->Copy();
	ms_apInkCloudEffects[ 1 ]  = pInkCloud->Copy();
	ms_apInkCloudEffects[ 4 ]  = pInkCloud->Copy();
	ms_apInkCloudEffects[ 6 ]  = pInkCloud->Copy();
	ms_apInkCloudEffects[ 5 ]  = pInkCloud->Copy();
	ms_apInkCloudEffects[ 3 ]  = pInkCloud->Copy();
	ms_apInkCloudEffects[ 7 ]  = pInkCloud->Copy();
	AParticleEffectHelper::SetColour( ms_apInkCloudEffects[ 1 ], ABlobColourComponent::ms_auiColours[ 1 ], TNULL );
	AParticleEffectHelper::SetColour( ms_apInkCloudEffects[ 2 ], ABlobColourComponent::ms_auiColours[ 2 ], TNULL );
	AParticleEffectHelper::SetColour( ms_apInkCloudEffects[ 4 ], ABlobColourComponent::ms_auiColours[ 4 ], TNULL );
	AParticleEffectHelper::SetColour( ms_apInkCloudEffects[ 6 ], ABlobColourComponent::ms_auiColours[ 6 ], TNULL );
	AParticleEffectHelper::SetColour( ms_apInkCloudEffects[ 5 ], ABlobColourComponent::ms_auiColours[ 5 ], TNULL );
	AParticleEffectHelper::SetColour( ms_apInkCloudEffects[ 3 ], ABlobColourComponent::ms_auiColours[ 3 ], TNULL );
	AParticleEffectHelper::SetColour( ms_apInkCloudEffects[ 7 ], ABlobColourComponent::ms_auiColours[ 7 ], TNULL );

	for ( TINT i = 8; i < 16; i++ )
	{
		ms_apInkCloudEffects[ i ] = pInkCloud->Copy();
		AParticleEffectHelper::SetColour( ms_apInkCloudEffects[ i ], ABlobColourComponent::ms_auiColours[ i ], TNULL );
	}

	ms_apInkCloudSmallEffects = TSTATICCAST( TParticleEffect*, TMemalign( 16, ABlobColourComponent::COLOUR_NUMOF * sizeof( TParticleEffect* ) ) );

	for ( TINT i = 0; i < ABlobColourComponent::COLOUR_NUMOF; i++ )
	{
		if ( ms_apInkCloudEffects[ i ] == TNULL )
		{
			ms_apInkCloudSmallEffects[ i ] = TNULL;
		}
		else
		{
			ms_apInkCloudSmallEffects[ i ] = ms_apInkCloudEffects[ i ]->Copy();
			AParticleEffectHelper::SetNumParticles( ms_apInkCloudSmallEffects[ i ], 2 );
		}
	}
}

// $deBlob: FUNCTION 00745180
void ABlobEntity::UnloadProperties()
{
	ms_pBlobCommonProperties = TNULL;
	ms_pBlobProperties       = TNULL;
	ms_pBlobRibbonProperties = TNULL;

	ABlobColourMaps::Unload();

	ABlobAnimationComponent::UnloadAnimationProperties();

	ms_pZTargetPriorities = TNULL;
}

// $deBlob: FUNCTION 00747110
void ABlobEntity::ClearEffects()
{
	ms_pUnk9B59C0                   = TNULL;
	ms_pUnk9B59C4                   = TNULL;
	ms_pPaintSplashEffect           = TNULL;
	ms_pElectrocutionEffect         = TNULL;
	ms_pHotPlateEffect              = TNULL;
	ms_pInkedStinkEffect            = TNULL;
	ms_pInkSplashEffect             = TNULL;
	ms_pWaterSplashEffect           = TNULL;
	ms_pWaterSplashSlamEffect       = TNULL;
	ms_pWaterRippleEffect           = TNULL;
	ms_pLandmarkEffect              = TNULL;
	ms_pDeathPopSplatEffect         = TNULL;
	ms_pDeathPopSprayEffect         = TNULL;
	ms_pUnpaintableEffect           = TNULL;
	ms_pWindLauncherTrailEffect     = TNULL;
	ms_pRainbowEffect               = TNULL;
	ms_pZJumpFailEffect             = TNULL;
	ms_pSpikeHaemorrhageClearEffect = TNULL;
	ms_pMPShieldLoopEffect          = TNULL;
	ms_pMPShieldHitEffect           = TNULL;
	ms_pElectricTrapSparksEffect    = TNULL;
	ms_pLandmarkPaintSplatEffect    = TNULL;

	if ( ms_apInkCloudEffects != TNULL )
	{
		// The copies ([ 16 ] is the original, [ 0 ] is TNULL)
		static TCONSTEXPR_VAR TINT s_aiCopies[] = { 1, 2, 4, 6, 5, 3, 7 };

		for ( TINT i = 0; i < TARRAYSIZE( s_aiCopies ); i++ )
		{
			if ( ms_apInkCloudEffects[ s_aiCopies[ i ] ] != TNULL )
				delete ms_apInkCloudEffects[ s_aiCopies[ i ] ];
		}

		for ( TINT i = 8; i < 16; i++ )
		{
			if ( ms_apInkCloudEffects[ i ] != TNULL )
				delete ms_apInkCloudEffects[ i ];
		}
	}

	::operator delete( ms_apInkCloudEffects );
	ms_apInkCloudEffects = TNULL;

	if ( ms_apInkCloudSmallEffects != TNULL )
	{
		for ( TINT i = 0; i < ABlobColourComponent::COLOUR_NUMOF; i++ )
		{
			if ( ms_apInkCloudSmallEffects[ i ] != TNULL )
				delete ms_apInkCloudSmallEffects[ i ];
		}
	}

	::operator delete( ms_apInkCloudSmallEffects );
	ms_apInkCloudSmallEffects = TNULL;
}

// $deBlob: FUNCTION 00744e20
void ABlobEntity::LoadProperties( TBOOL a_bReload )
{
	TTRB* pTRB = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_BlobProperties );

	if ( a_bReload )
	{
		pTRB->Close();
		TINT error = pTRB->Load( "data/Entities/BlobProperties.trb" );
		TASSERT( error == TTRB::ERROR_OK );
	}

	ms_pBlobCommonProperties = TSTATICCAST( CommonProperties, pTRB->GetSymbolAddress( "BlobEntityCommonProperties" ) );
	ms_pBlobProperties       = TSTATICCAST( Properties, pTRB->GetSymbolAddress( "BlobEntityProperties" ) );
	ms_pBlobRibbonProperties = pTRB->GetSymbolAddress( "BlobRibbonProperties" );

	TINT* piNumSizes = TSTATICCAST( TINT, pTRB->GetSymbolAddress( "NumBlobEntitySizes" ) );
	TASSERT( *piNumSizes >= BLOB_SIZE_ABS_MAX );

	for ( TINT i = 0; i < *piNumSizes; i++ )
	{
		Properties& rProps  = ms_pBlobProperties[ i ];
		TFLOAT      fRadius = rProps.m_fRadius;
		TFLOAT      fMass   = rProps.m_fMass;

		rProps.m_fDensity            = fMass / ( fRadius * 4.1887903f * fRadius * fRadius );
		rProps.m_fTorque             = fRadius * fMass * rProps.m_fTorque;
		rProps.m_fForce              = rProps.m_fForce * fMass;
		rProps.m_fMaxAngularVelocity = rProps.m_fMaxSpeed / fRadius;
		rProps.m_fSlamRadius         = fRadius * rProps.m_fSlamRadius;
		rProps.m_fJellyStiffness     = rProps.m_fJellyStiffness * fMass;
	}

	CommonProperties* pProps = ms_pBlobCommonProperties;

	TFLOAT fSqrt             = TFLOAT( sqrt( ( pProps->m_fUnk5C * 2.0 ) / 80.0 ) );
	pProps->m_fJumpVelocity  = pProps->m_fUnk5C / fSqrt + fSqrt * 40.0f;
	pProps->m_fUnk6C         = TFLOAT( sqrt( pProps->m_fWallJumpReach * 80.0 ) );
	pProps->m_fWallJumpAngle = TMath::Min( pProps->m_fWallJumpAngle, 80.0f ) * 0.017453292f;

	TASSERT( pProps->m_fInkedPenalty > 0.0f );
	pProps->m_fInkedPenalty = TMath::Max( pProps->m_fInkedPenalty, 0.1f );

	TFLOAT fInked                = pProps->m_fInkedPenalty * pProps->m_fUnk5C;
	fSqrt                        = TFLOAT( sqrt( ( fInked * 2.0 ) / 80.0 ) );
	pProps->m_fInkedJumpVelocity = fInked / fSqrt + fSqrt * 40.0f;
	pProps->m_fUnk98             = TFLOAT( sqrt( pProps->m_fWallJumpReach * pProps->m_fInkedPenalty * 80.0 ) );

	ABlobColourMaps::Load( pTRB );
	ABlobAnimationComponent::LoadAnimationProperties( pTRB );

	void* pPriorities    = pTRB->GetSymbolAddress( "BlobZTargetPriorities" );
	TINT  iPriorityCount = *TSTATICCAST( TINT, pTRB->GetSymbolAddress( "NumBlobZTargetPriorities" ) );
	TASSERT( iPriorityCount == ZPRIORITY_COUNT, "There should be %d priority categories in the TRB instead of %d\n" ); // NOTE: retail passes ZPRIORITY_COUNT, iPriorityCount
	ms_pZTargetPriorities = pPriorities;
}

// $deBlob: FUNCTION 0074fc20
void ABlobEntity::CreatePhysics( TFLOAT a_fLinearDamping )
{
	TPhysicsManager* pPM = TPhysicsManager::GetSingleton();
	m_pBody              = pPM->CreateBody( &m_oPhysicsMatrix, &m_iUserDataType, TPhysicsManager::MotionType_Dynamic, TPhysicsManager::SubType_Unk2 );

	// Inked (or no colour component yet: Create calls this before creating it)
	if ( m_pColourComponent == TNULL || ( ( m_pColourComponent->GetCurrentColour() >> 4 ) & 1 ) != 0 )
		m_pBody->m_fMass = ms_pBlobProperties[ m_iSize ].m_fMass / ms_pBlobCommonProperties->m_fInkedPenalty;
	else
		m_pBody->m_fMass = ms_pBlobProperties[ m_iSize ].m_fMass;

	if ( m_iLeechbotsAttached <= 0 )
	{
		TPhysicsManager::SetSphere( m_pBody, ms_pBlobProperties[ m_iSize ].m_fRadius );
	}
	else
	{
		TPhysicsManager::AddListSphere( m_pBody, TMatrix44::IDENTITY, ms_pBlobProperties[ m_iSize ].m_fRadius );

		for ( TINT i = 0; i < m_iLeechbotsAttached; i++ )
			ALeechBot2::AddAttachedShape( m_pBody, this, i );
	}

	if ( m_pBody )
		m_pBody->m_fFriction = ms_pBlobProperties[ m_iSize ].m_fFriction;

	if ( m_pBody )
		m_pBody->m_fRestitution = ms_pBlobProperties[ m_iSize ].m_fRestitution;

	if ( m_pBody )
	{
		m_pBody->m_fLinearDamping = a_fLinearDamping;
		if ( m_pBody->m_body )
			m_pBody->m_body->setLinearDamping( a_fLinearDamping );
	}

	TFLOAT fAngularDamping = ms_pBlobCommonProperties->m_fAngularDamping;
	if ( m_pBody )
	{
		m_pBody->m_fAngularDamping = fAngularDamping;
		if ( m_pBody->m_body )
			m_pBody->m_body->setAngularDamping( fAngularDamping );
	}

	TFLOAT fMaxAngularVelocity = ms_pBlobProperties[ m_iSize ].m_fMaxAngularVelocity;
	TASSERT( m_pBody, "POC" );
	m_pBody->m_fMaxAngularVelocity = fMaxAngularVelocity;
	if ( m_pBody->m_body )
		m_pBody->m_body->setMaxAngularVelocity( fMaxAngularVelocity );

	TPhysicsManager::GetSingleton()->SetListener( m_pBody, &m_oCollisionEventHandler );
	TPhysicsManager::GetSingleton()->SetCollisionLayer( m_pBody, 4 );
	TPhysicsManager::GetSingleton()->SetCollisionGroup( m_pBody, 0x2c7 );

	if ( m_pBody )
		m_pBody->m_uiSurface = 0x1000;

	if ( m_pBody )
		m_pBody->m_pUserData = &m_iUserDataType;

	pPM->AddCustomMover( &m_oPhysicsMover );
	pPM->AddPlugin( &m_oPhysicsPlugin );

	if ( m_eState != STATE_DEAD )
	{
		pPM->AddToSimulation( m_pBody, TTRUE );

		if ( m_pBody && m_pBody->IsInSimulation() )
			m_pBody->m_body->activate();
	}

	// The phantoms around the ball (contact queries, triggers)
	if ( m_pJellyQueryHelper )
		pPM->DestroyPhantom( m_pJellyQueryHelper );

	if ( m_pSoftCollisionQueryHelper )
		pPM->DestroyPhantom( m_pSoftCollisionQueryHelper );

	TSphereData oSphere;
	TVector4&   rPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	oSphere.SetRadius( ms_pBlobProperties[ m_iSize ].m_fRadius );
	m_pJellyQueryHelper = pPM->CreateShapePhantom( &oSphere );
	pPM->SetPhantomCollisionFilterInfo( m_pJellyQueryHelper, 0x83000 );
	pPM->AddPhantomToSimulation( m_pJellyQueryHelper, rPosition, TNULL );

	oSphere.SetRadius( ms_pBlobProperties[ m_iSize ].m_fRadius + 0.7f );
	m_pSoftCollisionQueryHelper = pPM->CreateShapePhantom( &oSphere );
	pPM->SetPhantomCollisionFilterInfo( m_pSoftCollisionQueryHelper, 0xE3000 );
	pPM->AddPhantomToSimulation( m_pSoftCollisionQueryHelper, rPosition, TNULL );

	if ( m_pUnk424 == TNULL )
	{
		oSphere.SetRadius( s_fPhantom424Radius );
		m_pUnk424 = pPM->CreateShapePhantom( &oSphere );
		pPM->SetPhantomCollisionFilterInfo( m_pUnk424, 0x100000 );
		pPM->AddPhantomToSimulation( m_pUnk424, rPosition, TNULL );
	}

	if ( m_pPaintParticlePhantomA == TNULL )
	{
		m_pPaintParticlePhantomA = pPM->CreateAabbPhantom();
		pPM->SetPhantomCollisionFilterInfo( m_pPaintParticlePhantomA, 0xA1000 );
		pPM->AddPhantomToSimulation( m_pPaintParticlePhantomA, rPosition, TNULL );
	}

	if ( m_pPaintParticlePhantomB == TNULL )
	{
		TINT iOldWorld = pPM->GetDefaultWorld();
		pPM->SetDefaultWorld( APhysicsWorldType_Decal );

		m_pPaintParticlePhantomB = pPM->CreateAabbPhantom();
		pPM->SetPhantomCollisionFilterInfo( m_pPaintParticlePhantomB, 0x8000 );
		pPM->AddPhantomToSimulation( m_pPaintParticlePhantomB, rPosition, TNULL );

		TASSERT( iOldWorld == APhysicsWorldType_GamePlay );
		pPM->SetDefaultWorld( iOldWorld );
	}

	if ( m_pLiquidRayCastHelper == TNULL )
	{
		m_pLiquidRayCastHelper = pPM->CreateAabbPhantom();
		pPM->SetPhantomCollisionFilterInfo( m_pLiquidRayCastHelper, 0x20000 );
		pPM->AddPhantomToSimulation( m_pLiquidRayCastHelper, m_vRespawnPosition, TNULL );
	}
}

// $deBlob: FUNCTION 00747e40
void ABlobEntity::ResetMotion()
{
	m_oJelly.m_vPosition       = m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	m_oJelly.m_vRest           = m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	m_oJellySquash.m_fPosition = 0.0f;
	m_oJellySquash.m_fVelocity = 0.0f;
	m_oJelly.m_vVelocity       = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_fSquashCompensation      = 1.0f;
	m_fSquash                  = 1.0f;
	m_vBallOffset              = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
}

// $deBlob: FUNCTION 00747eb0
void ABlobEntity::SetRespawnPoint( const TVector4& a_rcPosition, TVector4 a_vRotation, TBOOL a_bHasRotation, TBOOL a_bPlaySeq, TBOOL a_bTransSeq, TFLOAT a_fSeqStartRatio, TFLOAT a_fSeqEndRatio )
{
	m_bHasRespawnRotation = a_bHasRotation;
	m_vRespawnPosition    = a_rcPosition;
	if ( a_bHasRotation )
		m_vRespawnRotation = a_vRotation;
	else
		m_vRespawnRotation = TVector4::VEC_ZERO;

	m_bRespawnPlaySeq     = a_bPlaySeq;

	*TREINTERPRETCAST( TFLOAT*, &m_uiRespawnSeqStartRatio ) = a_fSeqStartRatio;
	*TREINTERPRETCAST( TUINT8*, &m_uiRespawnTransSeq )      = a_bTransSeq;
	m_fRespawnSeqEndRatio                                   = a_fSeqEndRatio;
}

// $deBlob: FUNCTION 00747f20
void ABlobEntity::GetRespawnData( TVector4& a_rOutPosition, TVector4& a_rOutRotation, TBOOL& a_rbOutHasRotation, TBOOL& a_rbOutPlaySeq, TBOOL& a_rbOutTransSeq, TUINT32& a_ruiOutSeqStartRatio, TFLOAT& a_rfOutSeqEndRatio )
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	TINT8        iPlayerID   = APlayerManager::GetSingleton()->GetPlayerIndex( this );
	TASSERT( iPlayerID >= 0 );

	a_rOutPosition        = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	a_rOutRotation        = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	a_rbOutHasRotation    = TFALSE;
	a_rbOutPlaySeq        = TFALSE;
	a_rbOutTransSeq       = TTRUE;
	a_ruiOutSeqStartRatio = 0;
	a_rfOutSeqEndRatio    = 1.0f;

	TINT iGameType = pGameStatus->GetMultiPlayerGameType();

	if ( iGameType >= 0 && iGameType <= 3 )
	{
		APointDataFunction::Point* pPoint;

		if ( iGameType < 3 )
		{
			// The respawn point of the player farthest from the blob
			pPoint = pGameStatus->GetPointData().GetFarthestRespawnPoint( iPlayerID + APointDataFunction::POINT_PLAYER_1_RESPAWN, a_rOutPosition );
		}
		else
		{
			pPoint = pGameStatus->GetPointData().GetPoint( iPlayerID + 1 );
		}

		if ( pPoint != TNULL )
			a_rOutPosition = pPoint->m_vPosition;

		return;
	}

	a_rOutPosition = m_vRespawnPosition;

	if ( m_bHasRespawnRotation )
		a_rOutRotation = m_vRespawnRotation;

	a_rbOutHasRotation    = m_bHasRespawnRotation;
	a_rbOutPlaySeq        = m_bRespawnPlaySeq;
	a_rbOutTransSeq       = *TREINTERPRETCAST( TUINT8*, &m_uiRespawnTransSeq );
	a_ruiOutSeqStartRatio = m_uiRespawnSeqStartRatio;
	a_rfOutSeqEndRatio    = m_fRespawnSeqEndRatio;
}

// $deBlob: FUNCTION 00748060
void ABlobEntity::GetRespawnGroundPosition( const TVector4& a_rPosition, TVector4& a_rOutPosition )
{
	a_rOutPosition = a_rPosition;

	TBOOL bWater    = TFALSE;
	TINT  iGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();

	if ( iGameType < 0 || iGameType > 3 )
		bWater = FindRespawnGround( &a_rOutPosition, &a_rOutPosition );

	TFLOAT fRadius;

	if ( !m_bRespawnPlaySeq )
		fRadius = ms_pBlobProperties[ 10 ].m_fRadius;
	else if ( !bWater )
		fRadius = ms_pBlobProperties[ m_iSize ].m_fRadius;
	else
		return;

	a_rOutPosition.y -= fRadius + 0.5f;
}

// $deBlob: FUNCTION 007480e0
TBOOL ABlobEntity::FindRespawnGround( const TVector4* a_pPosition, TVector4* a_pOutPosition )
{
	if ( a_pPosition == TNULL )
		a_pPosition = &m_vRespawnPosition;

	TPhysicsRay oRay;
	oRay.m_vFrom            = *a_pPosition;
	oRay.m_vFrom.y          = a_pPosition->y - 1.0f;
	oRay.m_vTo              = *a_pPosition;
	oRay.m_vTo.y            = a_pPosition->y + 3.0f;
	oRay.m_uiCollisionLayer = 0xE1;

	TPhysicsShapeContact oContact;

	if ( TPhysicsManager::GetSingleton()->CollideRay( oRay, oContact, TNULL ) )
	{
		if ( a_pOutPosition != TNULL )
			*a_pOutPosition = oContact.m_vPosition;

		return ( oContact.m_uiMaterial >> 4 ) & 1;
	}

	if ( a_pOutPosition != TNULL && a_pOutPosition != a_pPosition )
		*a_pOutPosition = *a_pPosition;

	return TFALSE;
}

// $deBlob: FUNCTION 007481a0
void ABlobEntity::PrepareRespawn( const TVector4* a_pPosition, const TINT* a_pPoints, const TINT* a_pColour )
{
	m_iStartPaintPoints = ( a_pPoints != TNULL ) ? *a_pPoints : -1;
	m_iStartColour      = ( a_pColour != TNULL ) ? *a_pColour : -1;

	TVector4 vPosition;

	if ( a_pPosition != TNULL )
	{
		SetPosition( *a_pPosition );
		vPosition = *a_pPosition;
	}
	else
	{
		TVector4 vRotation;
		TBOOL    bHasRotation;
		TBOOL    bPlaySeq;
		TBOOL    bTransSeq;
		TUINT32  uiSeqStartRatio;
		TFLOAT   fSeqEndRatio;
		GetRespawnData( vPosition, vRotation, bHasRotation, bPlaySeq, bTransSeq, uiSeqStartRatio, fSeqEndRatio );

		m_bHasRespawnRotation                              = bHasRotation;
		m_vRespawnPosition                                 = vPosition;
		m_vRespawnRotation                                 = bHasRotation ? vRotation : TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		m_bRespawnPlaySeq                                  = bPlaySeq;
		m_uiRespawnSeqStartRatio                           = uiSeqStartRatio;
		m_fRespawnSeqEndRatio                              = fSeqEndRatio;
		*TREINTERPRETCAST( TUINT8*, &m_uiRespawnTransSeq ) = bTransSeq;

		TBOOL bWater    = TFALSE;
		TINT  iGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();

		if ( iGameType < 0 || iGameType > 3 )
			bWater = FindRespawnGround( &vPosition, &vPosition );

		if ( !m_bRespawnPlaySeq )
			vPosition.y -= ms_pBlobProperties[ 10 ].m_fRadius + 0.5f;
		else if ( !bWater )
			vPosition.y -= ms_pBlobProperties[ m_iSize ].m_fRadius + 0.5f;

		SetPosition( vPosition );
	}

	SetPhysicsPosition( vPosition );
	TPhysicsManager::StopBody( m_pBody );

	// NOTE: retail doesn't check the camera
	APlayerManager::GetSingleton()->GetBlobFollowCamera( this )->Reset( TFALSE, TTRUE );
	ATerrain2Interface::GetInstance()->UpdateStreaming();
	m_pSoundComponent->OnRespawn();
	AJellyShader_DX11::GetSingleton()->FinishAll();
	// Multiplayer game type 1 while no blob has painted yet: the evil (ink) blob
	// NOTE: retail doesn't check m_pSizeHud here
	if ( m_pColourComponent->GetCurrentColour() == ABlobColourComponent::COLOUR_WHITE &&
	     AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 && !APlayerManager::GetSingleton()->IsNoBlobPainted() )
	{
		m_pSizeHud->GetVisual()->PlayAnimation( L"EvilBlob", L"EndEvilBlob" );
		m_pSizeHud->SetColour( ABlobColourComponent::ms_auiColours[ ABlobColourComponent::COLOUR_BLACK ], TFALSE, 0.0f );
	}
	else
	{
		m_pSizeHud->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );
	}

	if ( APropTriggerHelp::GetHelpHud() != TNULL )
		APropTriggerHelp::GetHelpHud()->HideNow();

	if ( APropTriggerHelp::GetLetterBoxHelpHud() != TNULL )
		APropTriggerHelp::GetLetterBoxHelpHud()->HideNow();

	APropTriggerHelp::ResetHelpState();
}

// $deBlob: FUNCTION 007483f0
void ABlobEntity::Respawn( TBOOL a_bUnk, TBOOL a_bFindGround, const TVector4* a_pPosition )
{
	TBOOL    bAtStartPoint = TFALSE;
	TVector4 vPosition     = ( a_pPosition == TNULL ) ? m_vRespawnPosition : *a_pPosition;

	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
	{
		APointDataFunction::Point* pStartPoint = AGameStatus::GetSingleton()->GetPointData().GetPoint( 1 );

		if ( pStartPoint != TNULL && ( pStartPoint->m_uiFlags & 1 ) )
			bAtStartPoint = TVector4::DistanceSq( pStartPoint->m_vPosition, vPosition ) < 1.0f;
	}

	TVector4 vSpawnPosition = vPosition;

	if ( a_bFindGround && !bAtStartPoint )
	{
		GetRespawnGroundPosition( vPosition, vSpawnPosition );
	}

	SetPosition( vSpawnPosition );
	SetPhysicsPosition( vSpawnPosition );
	TPhysicsManager::StopBody( m_pBody );

	m_uiFlags &= 0xfffefe7f;
	m_oJelly.m_vPosition = vSpawnPosition;
	m_oJelly.m_vRest     = vSpawnPosition;
	m_oJelly.m_vVelocity = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_vBallPosition      = vSpawnPosition;
	m_vBallOffset        = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1

	ResetMotion();

	m_pRibbonTrail->SetPosition( vSpawnPosition );
	m_pRibbonTrail->Reset();

	EndRainbow();
	m_pColourComponent->SetColour( ABlobColourComponent::COLOUR_WHITE );

	// The initial size of the level
	TINT iSize = ALevelInformation::GetSingleton()->GetLevelHeader()->m_pLevelProps[ AGameStatus::GetSingleton()->GetCurrentLevel() ].m_auiMedalLimits[ 7 ];
	m_iSize    = iSize;
	m_iOldSize = iSize;

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 )
	{
		m_iPaintPoints = 30;

		if ( ( m_uiFlags & 0x8000000 ) == 0 )
			m_iSize = ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) ? 10 : 3;
	}
	else
	{
		m_iPaintPoints = ( iSize + 1 ) * 10;
	}

	SetVisible( TTRUE );

	// Faces along the respawn rotation (none at a given position)
	static const TVector4 s_vNoRotation( 0.0f, 0.0f, 0.0f, 1.0f );
	static const TVector4 s_vForward( 0.0f, 0.0f, 1.0f, 1.0f );

	const TVector4* pRotation = m_bHasRespawnRotation ? &m_vRespawnRotation : TNULL;

	if ( a_pPosition != TNULL )
		pRotation = &s_vNoRotation;

	if ( pRotation != TNULL )
	{
		TVector4 vFacing;
		TQuaternion::RotateVector( vFacing, *TREINTERPRETCAST( const TQuaternion*, pRotation ), s_vForward );
		m_vFacing = vFacing;
	}

	if ( m_pSizeHud != TNULL )
	{
		if ( m_pColourComponent->GetCurrentColour() == ABlobColourComponent::COLOUR_WHITE &&
		     AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 && !APlayerManager::GetSingleton()->IsNoBlobPainted() )
			m_pSizeHud->GetVisual()->PlayAnimation( L"EvilBlob", L"EndEvilBlob" );
		else
			m_pSizeHud->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );

		m_pSizeHud->SetSize( m_iPaintPoints );

		if ( !( m_pColourComponent->GetCurrentColour() == ABlobColourComponent::COLOUR_WHITE &&
		        AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 && !APlayerManager::GetSingleton()->IsNoBlobPainted() ) )
		{
			// 0x5BC (shown by the landmark paint counter), capped by LevelData::m_fTimeLeft outside free paint
			const TFLOAT* pTime = TREINTERPRETCAST( const TFLOAT*, &m_auiUnk5A8[ 5 ] );

			if ( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint )
			{
				const TFLOAT* pLevelTime = &AGameStatus::GetSingleton()->GetLevelData().m_fTimeLeft;

				if ( *pTime > *pLevelTime )
					pTime = pLevelTime;
			}

			TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

			if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
				uiColour = ABlobColourComponent::COLOUR_BLACK;

			m_pSizeHud->SetColour( ABlobColourComponent::ms_auiColours[ uiColour ], ( m_uiFlags >> 0x15 ) & 1, *pTime );
		}
	}

	if ( APlayerManager::GetSingleton()->GetNumPlayers() < 2 && !a_bUnk )
	{
		if ( bAtStartPoint )
		{
			// Back at the level start point: the level start plays again (the respawn animation of the gate)
			m_uiFlags = ( m_uiFlags & ~0x20 ) | 0x80000;
			TPhysicsManager::GetSingleton()->RemoveFromSimulation( m_pBody, TTRUE );
			AGameStatus::GetSingleton()->SetLevelStart( TTRUE, TTRUE );
			m_pAnimationComponent->SetState( ABlobAnimationComponent::STATE_RESPAWN_POOL );
			SetState( STATE_BALLISTIC );
			return;
		}

		if ( m_bRespawnPlaySeq )
		{
			SetState( STATE_RESPAWN );
			TPhysicsManager::GetSingleton()->RemoveFromSimulation( m_pBody, TTRUE );
			APlayerManager::GetSingleton()->StartRespawnCamera( APlayerManager::GetSingleton()->GetPlayerIndex( this ), vSpawnPosition, pRotation, *TREINTERPRETCAST( TUINT8*, &m_uiRespawnTransSeq ), *TREINTERPRETCAST( TFLOAT*, &m_uiRespawnSeqStartRatio ), m_fRespawnSeqEndRatio );
			return;
		}
	}

	FinishRespawn();
}

// $deBlob: FUNCTION 007487e0
void ABlobEntity::FinishRespawn()
{
	if ( m_iStartPaintPoints != -1 )
	{
		SetPaintPoints( m_iStartPaintPoints );
		m_iStartPaintPoints = -1;
	}

	if ( m_iStartColour != -1 )
	{
		m_pColourComponent->SetColour( m_iStartColour );
		m_iStartColour = -1;
	}

	TPhysicsManager::StopBody( m_pBody );

	if ( m_pBody != TNULL && m_pBody->IsInSimulation() )
		m_pBody->m_body->activate();

	SetState( STATE_BALLISTIC );
	OnSizeChanged();
	ResetCameraDirection();

	m_fUntargetableTime = TMath::Max( m_fUntargetableTime, 2.0f );
}

// Detaches the blob from the physics during the level start (it follows the level start gate), or puts it back
// once its cell is ready (flag 0x20 until then)
// $deBlob: FUNCTION 00746dc0
void ABlobEntity::StartLevelExit( ABaseEntity* a_pTrigger, const TVector4& a_rFacing )
{
	m_pLevelExitTrigger = a_pTrigger;
	SetState( STATE_LEVEL_EXIT );
	m_vFacing = a_rFacing;
}

// $deBlob: FUNCTION 00747350
void ABlobEntity::SetLevelStartAttached( TBOOL a_bAttached )
{
	TPhysicsManager* pPM = TPhysicsManager::GetSingleton();

	if ( a_bAttached )
	{
		m_uiFlags = ( m_uiFlags & 0xfff7ffdf ) | 0x80000;
		pPM->RemoveFromSimulation( m_pBody, TTRUE );
	}
	else if ( ATerrain2Interface::GetInstance()->IsPositionInReadyCell( m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), TFALSE ) )
	{
		if ( m_eState != STATE_DEAD )
		{
			pPM->AddToSimulation( m_pBody, TTRUE );

			if ( m_pBody != TNULL && m_pBody->IsInSimulation() )
				m_pBody->m_body->activate();
		}

		m_uiFlags &= 0xfff7ffdf;
	}
	else
	{
		m_uiFlags |= 0x20;
	}
}

// $deBlob: FUNCTION 007488a0
void ABlobEntity::SetStartPosition( const TVector4& a_rPosition, TBOOL a_bSetPosition, TBOOL a_bFindGround )
{
	if ( a_bSetPosition )
	{
		m_bHasRespawnRotation          = 0;
		m_vRespawnPosition             = a_rPosition;
		m_vRespawnRotation             = TVector4::VEC_ZERO;
		m_bRespawnPlaySeq              = 0;
		*(TUINT8*)&m_uiRespawnTransSeq = 0;
		m_uiRespawnSeqStartRatio       = 0;
		m_fRespawnSeqEndRatio          = 1.0f;
	}

	Respawn( TTRUE, a_bFindGround, TNULL );
}

// $deBlob: FUNCTION 00748900
void ABlobEntity::SetPhysicsPosition( const TVector4& a_rPosition )
{
	TASSERT( a_rPosition.w == 1.0f );
	m_oPhysicsMatrix.SetTranslation( a_rPosition );
	TPhysicsManager::UpdateBodyTransform( m_pBody, TFALSE );

	m_vPhysicsPosition = a_rPosition;

	if ( m_pWindCannon != TNULL )
		m_pWindCannon->OnBlobMoved( this );
}


// $deBlob: FUNCTION 00745810
void ABlobEntity::IncTargetCount()
{
	if ( 0 == m_uiNumTurretsTargettingMe )
		m_pSoundComponent->PlayEvent38();
	else
		m_pSoundComponent->NoEvent38();

	m_uiNumTurretsTargettingMe++;
}

// $deBlob: FUNCTION 00745890
void ABlobEntity::ChangeColour( TUINT32 a_uiColour )
{
	// Turning inky (ink while not inky yet)
	TBOOL bToInk = TFALSE;

	if ( IsControllable() )
	{
		if ( a_uiColour == ABlobColourComponent::COLOUR_BLACK )
		{
			bToInk = ( m_pColourComponent->GetCurrentColour() & 0x10 ) == 0;

			TUINT32 uiColour = m_pColourComponent->GetCurrentColour();
			if ( uiColour & 0x10 ) uiColour = 0x10;

			if ( ( uiColour & 0x10 ) == 0 )
				AVibrationManager::VibrateOnce( APlayerManager::GetSingleton()->GetPlayerIndex( this ), 4 );
		}
	}
	else if ( a_uiColour == ABlobColourComponent::COLOUR_BLACK )
	{
		return;
	}

	// The size HUD: "Inky" when turning inky, "Inkbot" for another new colour
	if ( m_pSizeHud != TNULL && ( m_pColourComponent->GetCurrentColour() != 0 || AGameStatus::GetSingleton()->GetMultiPlayerGameType() != 1 || APlayerManager::GetSingleton()->IsNoBlobPainted() ) )
	{
		if ( bToInk )
		{
			m_pSizeHud->GetVisual()->PlayAnimation( L"Inky", L"EndInky" );
		}
		else
		{
			TUINT32 uiOldColour = m_pColourComponent->GetCurrentColour();

			if ( uiOldColour & 0x10 )
				uiOldColour = 0x10;

			if ( uiOldColour != a_uiColour )
				m_pSizeHud->GetVisual()->PlayAnimation( L"Inkbot", L"EndInkbot" );
		}
	}

	TUINT32 uiColour = m_pColourComponent->GetCurrentColour();
	if ( uiColour & 0x10 ) uiColour = 0x10;

	if ( uiColour != a_uiColour )
	{
		m_pSoundComponent->OnColourChanging( a_uiColour );

		// The paint drip effect takes the new colour
		if ( m_auiSounds[ 8 ] != 0 )                                                                                                                              // 0xAF8
			AParticleEffectHelper::SetColour( TREINTERPRETCAST( TParticleEffect*, m_auiSounds[ 1 ] ), ABlobColourComponent::ms_auiColours[ a_uiColour ], TNULL ); // 0xADC
	}

	m_pColourComponent->MixColour( a_uiColour );

	if ( bToInk )
	{
		m_pZJump->End();
		RecreatePhysics();
	}

	if ( a_uiColour == ABlobColourComponent::COLOUR_BLACK )
	{
		if ( m_iPaintPoints == 0 )
		{
			OnOutOfPoints( TFALSE );
			return;
		}

		// The "you're inked" help
		if ( AGameStatus::GetSingleton()->GetSPLevel() == AGameStatus::SPLevel_LastResort )
			APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_Hud, 0x37C, -1, TTRUE, 5.0f, 0, TNULL, TFALSE );
	}
	else
	{
		APropTriggerHelp::ShowContextHelp( 2 );

		if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 && ABlobHud::GetSingleton() )
			ABlobHud::GetSingleton()->SetPendingShowcase( 4 );
	}
}

// The names of the blob states
static const TCHAR* s_aszStateNames[] = {
	"STATE_ROLL",
	"STATE_BALLISTIC",
	"STATE_STUCKWALL",
	"STATE_HOTPLATE",
	"STATE_LMARK",
	"STATE_BOSS",
	"STATE_FLOAT",
	"STATE_CANNON",
	"STATE_GRASSBOMB",
	"STATE_RESPAWN",
	"STATE_LEVEL_EXIT",
	"STATE_DEAD",
	"STATE_FAIL",
	"STATE_VICTORY",
	"STATE_LOSTLASTLIFE",
};

static const TWCHAR* s_awszStateNames[] = {
	L"STATE_ROLL",
	L"STATE_BALLISTIC",
	L"STATE_STUCKWALL",
	L"STATE_HOTPLATE",
	L"STATE_LMARK",
	L"STATE_BOSS",
	L"STATE_FLOAT",
	L"STATE_CANNON",
	L"STATE_GRASSBOMB",
	L"STATE_RESPAWN",
	L"STATE_LEVEL_EXIT",
	L"STATE_DEAD",
	L"STATE_FAIL",
	L"STATE_VICTORY",
	L"STATE_LOSTLASTLIFE",
};

// $deBlob: FUNCTION 00744720
const TCHAR* ABlobEntity::GetStateName( TINT a_eState ) const
{
	if ( a_eState > -1 && TUINT( a_eState ) < TARRAYSIZE( s_aszStateNames ) )
		return s_aszStateNames[ a_eState ];

	return "Unknown State";
}

// $deBlob: FUNCTION 00744750
const TWCHAR* ABlobEntity::GetStateNameW( TINT a_eState ) const
{
	if ( a_eState > -1 && TUINT( a_eState ) < TARRAYSIZE( s_awszStateNames ) )
		return s_awszStateNames[ a_eState ];

	return L"Unknown State";
}

// $deBlob: FUNCTION 00744780
void ABlobEntity::SetPaintPoints( TUINT32 a_uiPoints )
{
	TASSERT( a_uiPoints <= POINTS_MAX );

	m_iPaintPoints = a_uiPoints;

	if ( ( m_uiFlags & 0x8000000 ) == 0 )
	{
		if ( TUINT8( m_uiFlags >> 21 ) & 1 )
			m_iSize = 10;
		else
			m_iSize = TMath::Min( TINT( a_uiPoints / 10 ), 9 );
	}
}

// Returns the points that couldn't be removed
// $deBlob: FUNCTION 00744820
TUINT32 ABlobEntity::RemovePaintPoints( TUINT32 a_uiPoints, TBOOL a_bOnlyControllable )
{
	if ( a_bOnlyControllable )
	{
		if ( ms_bControlsLocked )
			return 0;

		if ( ( m_uiFlags & 0x4080000 ) != 0 || ( m_uiFlags & 0x200000 ) != 0 )
			return 0;
	}

	if ( ( m_pColourComponent->GetCurrentColour() == 0 && AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 && !APlayerManager::GetSingleton()->IsNoBlobPainted() ) ||
	     ( m_uiFlags & 0x200000 ) != 0 )
		return 0;

	if ( m_eState == STATE_DEAD )
		return a_uiPoints;

	TUINT32 uiPoints = m_iPaintPoints;

	if ( a_uiPoints < uiPoints )
	{
		SetPaintPoints( uiPoints - a_uiPoints );
		return 0;
	}

	TUINT32 uiRest = a_uiPoints - uiPoints;
	SetPaintPoints( 0 );

	if ( m_pColourComponent->GetCurrentColour() & 0x10 )
	{
		// Out of points while inky
		OnOutOfPoints( TFALSE );
		return uiRest;
	}

	m_pColourComponent->SetColour( 0 );

	if ( ( m_uiFlags & 0x80 ) == 0 )
	{
		m_pSoundComponent->PlayPaintLost();
		APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_Hud, 891, -1, TTRUE, 5.0f, 0, TNULL, TFALSE );
	}

	return uiRest;
}

// $deBlob: FUNCTION 00747400
void ABlobEntity::OnOutOfPoints( TBOOL a_bFail )
{
	if ( m_eState == STATE_DEAD || m_eState == STATE_FAIL || m_eState == STATE_LOSTLASTLIFE )
		return;

	TINT iGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();

	if ( iGameType < 0 )
		APlayerManager::GetSingleton()->GetBlobDataOf( this )->LoseLife( TFALSE );

	AGameStatus::GetSingleton()->OnPlayerDied( APlayerManager::GetSingleton()->GetPlayerIndex( this ) );
	*TREINTERPRETCAST( TFLOAT*, &m_auiUnk580[ 3 ] ) = ms_pBlobCommonProperties->m_fBlobKillRespawnTime; // 0x58C the respawn timer

	// The last life is lost (single player, not in a sprint level): game over
	if ( iGameType < 0 && TINT( APlayerManager::GetSingleton()->GetBlobDataOf( this )->m_uiLives ) <= 0 &&
	     AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel == 0 )
	{
		SetState( STATE_LOSTLASTLIFE );
		m_fUntargetableTime = TMath::Max( m_fUntargetableTime, 10.0f );
	}
	else if ( !a_bFail )
	{
		SetState( STATE_DEAD );
		PopDeath();
	}
	else
	{
		SetState( STATE_FAIL );
	}

	AProjectileManager::GetSingleton()->KillAllActive();
	AChallengeManager::GetSingleton()->OnBlobDied();

	// Died in the blob vision: the vision HUD ends
	if ( m_bInBlobVision )
	{
		TXUI*      pXUI   = TXUI::GetSingleton();
		TXUIScene* pScene = pXUI->GetActiveScene();

		if ( pScene != TNULL && pScene->IsA( &TGetClass( ABlobVisionHud ) ) )
		{
			TSTATICCAST( ABlobVisionHud, pScene )->SetUnk130();
			return;
		}

		ABlobVisionHud* pVisionHud = TNULL;
		TXUIScene*      pTransTo   = pXUI->GetTransToScene();

		if ( pTransTo != TNULL && pTransTo->IsA( &TGetClass( ABlobVisionHud ) ) )
			pVisionHud = TSTATICCAST( ABlobVisionHud, pTransTo );

#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail writes to TNULL + 0x130 when neither scene is the vision HUD
		if ( pVisionHud != TNULL )
			pVisionHud->SetUnk130();
#else
        pVisionHud->SetUnk130();
#endif
	}
}

// $deBlob: FUNCTION 007489c0
void ABlobEntity::BecomeInkBlob()
{
	WashOff();
	m_iPaintPoints = 30;

	if ( !( m_uiFlags & 0x8000000 ) )
		m_iSize = ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) ? 10 : 3;

	if ( m_pSizeHud != TNULL )
	{
		m_pSizeHud->SetColour( ABlobColourComponent::ms_auiColours[ 16 ], TFALSE, 0.0f );
		m_pSizeHud->GetVisual()->PlayAnimation( L"EvilBlob", L"EndEvilBlob" );
	}
}

// $deBlob: FUNCTION 00745a80
void ABlobEntity::WashOff()
{
	TUINT32 uiColour = m_pColourComponent->GetCurrentColour();

	if ( ( uiColour & 0x10 ) != 0 || uiColour != 0 )
	{
		if ( TUINT8( uiColour >> 4 ) & 1 )
			RecreatePhysics();

		m_pColourComponent->SetColour( 0 );
		m_pSoundComponent->OnColourChanging( 0 );

		if ( m_pSizeHud != TNULL && ( m_pColourComponent->GetCurrentColour() != 0 || AGameStatus::GetSingleton()->GetMultiPlayerGameType() != 1 || APlayerManager::GetSingleton()->IsNoBlobPainted() ) )
			m_pSizeHud->GetVisual()->PlayAnimation( L"Water", L"EndWater" );
	}
}

// $deBlob: FUNCTION 007477f0
void ABlobEntity::ClearZPlayerAttack()
{
	m_iZPlayerAttackStage = 0;    // 0xB24
	m_fZPlayerAttackTime  = 0.0f; // 0xB28
}

// $deBlob: FUNCTION 0074fa40
void ABlobEntity::OnSizeChanged()
{
	m_fUnkB34 = 0.6f;

	if ( m_pSizeHud != TNULL && m_pSizeHud->GetVisual() != TNULL &&
	     ( m_pColourComponent->GetCurrentColour() != 0 || AGameStatus::GetSingleton()->GetMultiPlayerGameType() != 1 || APlayerManager::GetSingleton()->IsNoBlobPainted() ) )
		m_pSizeHud->GetVisual()->PlayAnimation( L"SizeChanging", L"EndSizeChanging" );

	m_pZJump->OnSizeChanged();

	if ( m_oMotion.m_uiContactFlags & 0x28 )
	{
		// Keeps the bottom of the ball where it was
		TFLOAT fY              = ms_pBlobProperties[ m_iOldSize ].m_fRadius + m_oPhysicsMatrix.m_f42;
		m_oPhysicsMatrix.m_f42 = fY - ms_pBlobProperties[ m_iSize ].m_fRadius;

		if ( ( ( m_uiFlags >> 0x15 ) & 1 ) != 0 && m_pAnimationComponent->m_iState == ABlobAnimationComponent::STATE_POWERUP )
			m_vHoldPosition = m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	}

	RecreatePhysics();
	m_oJelly.SetStiffness( ms_pBlobProperties[ m_iSize ].m_fJellyStiffness );
	m_oJelly.SetDampingRatio( ms_pBlobProperties[ m_iSize ].m_fJellyDamping );

	TFLOAT fRadius = ms_pBlobProperties[ m_iSize ].m_fRadius;
	SetBBox( TVector4( fRadius, fRadius, fRadius, 1.0f ) );
	m_fBoundingSphereRadius = ms_pBlobProperties[ m_iSize ].m_fRadius;

	AVibrationManager::VibrateOnce( APlayerManager::GetSingleton()->GetPlayerIndex( this ), 4 );

	if ( m_iOldSize <= m_iSize )
		m_pSoundComponent->PlaySizeUp();
	else
		m_pSoundComponent->PlaySizeDown();

	m_iOldSize = m_iSize;
}

// $deBlob: FUNCTION 00750210
void ABlobEntity::RecreatePhysics()
{
	if ( ( m_uiFlags & 0x20000000 ) != 0 )
		return;

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	TVector4 vLinearVelocity;
	TVector4 vAngularVelocity;
	pPhysicsManager->GetLinearVelocity( m_pBody, vLinearVelocity );
	pPhysicsManager->GetAngularVelocity( m_pBody, vAngularVelocity );

	TFLOAT fLinearDamping = ( m_pBody != TNULL ) ? m_pBody->m_fLinearDamping : 0.0f;

	pPhysicsManager->SetCollisionLayer( m_pBody, 0 );
	pPhysicsManager->SetCollisionGroup( m_pBody, 0 );
	pPhysicsManager->RemoveFromSimulation( m_pBody, TTRUE );
	pPhysicsManager->DestroyBody( m_pBody );

	CreatePhysics( fLinearDamping );
	SetLevelStartAttached( ( m_uiFlags & 0x80000 ) != 0 );

	TPhysicsManager::SetLinearVelocity( m_pBody, vLinearVelocity );
	TPhysicsManager::SetAngularVelocity( m_pBody, vAngularVelocity );
}

// $deBlob: FUNCTION 00751ba0
void ABlobEntity::AddPaintPoints( TUINT32 a_uiPoints )
{
	SetPaintPoints( TMath::Min( a_uiPoints + m_iPaintPoints, TUINT32( POINTS_MAX ) ) );
}

// $deBlob: FUNCTION 007451c0
void ABlobEntity::Debug_AddPaintPoints()
{
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() );

	if ( pBlob != TNULL )
		pBlob->SetPaintPoints( TMath::Min( TUINT32( pBlob->m_iPaintPoints + 10 ), POINTS_MAX ) );
}

// $deBlob: FUNCTION 00745210
void ABlobEntity::RemoveCurrentBlobPaint( TBOOL a_bOnlyControllable )
{
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() );

	if ( pBlob != TNULL )
		pBlob->RemovePaintPoints( 10, a_bOnlyControllable );
}

// $deBlob: FUNCTION 00745240
void ABlobEntity::AddLandmarkTime( TINT a_iSeconds, TBOOL a_bOnlyControllable )
{
	if ( !a_bOnlyControllable || IsControllable() )
		*TREINTERPRETCAST( TFLOAT*, &m_uiLandmarkTime ) += TFLOAT( a_iSeconds );
}

// $deBlob: FUNCTION 00745290
void ABlobEntity::InkBounce()
{
	if ( m_eState == STATE_STUCKWALL )
	{
		m_auiUnk564[ 0 ] = 0;
		m_auiUnk564[ 4 ] = 0;

		// Out of the ink along its normal
		TVector4 vPosition;
		vPosition.Multiply( m_oMotion.m_vWallNormal, 0.5f );
		vPosition.x = vPosition.x + m_oPhysicsMatrix.m_f41;
		vPosition.y = m_oPhysicsMatrix.m_f42 + vPosition.y;
		vPosition.z = m_oPhysicsMatrix.m_f43 + vPosition.z;
		SetPhysicsPosition( vPosition );

		TVector4 vVelocity;
		vVelocity.Multiply( m_oMotion.m_vWallNormal, 10.0f );

		TVector4 vCurrent;
		TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, vCurrent );
		vVelocity.x = vCurrent.x + vVelocity.x;
		vVelocity.y = vCurrent.y + vVelocity.y;
		vVelocity.z = vCurrent.z + vVelocity.z;
		TPhysicsManager::SetLinearVelocity( m_pBody, vVelocity );

		SetState( STATE_BALLISTIC );
		m_pZJump->OnInkBounce();
	}
}

// $deBlob: FUNCTION 00745740
TINT ABlobEntity::AddLeechbot()
{
	m_iLeechbotsAttached++;
	TASSERT( AEnemyGrappler::ENEMYGRAPPLER_MAX_GRAPPLERS >= m_iLeechbotsAttached );

	InkBounce();
	RecreatePhysics();

	if ( m_iLeechbotsAttached > m_iMaxLeechbotsAttached )
		m_iMaxLeechbotsAttached = m_iLeechbotsAttached;

	return m_iLeechbotsAttached;
}

// $deBlob: FUNCTION 007457b0
void ABlobEntity::RemoveLeechbot()
{
	TASSERT( 0 < m_iLeechbotsAttached );
	m_iLeechbotsAttached--;

	RecreatePhysics();

	if ( m_iLeechbotsAttached == 0 )
		m_iMaxLeechbotsAttached = 0;
}

// $deBlob: FUNCTION 00751330
// $deBlob: FUNCTION 007512c0
ABlobEntity::ABlobCollisionEventHandler::~ABlobCollisionEventHandler() = default;

// $deBlob: FUNCTION 0073b630
void ABlobEntity::ABlobCollisionEventHandler::OnCollisionConfirmed( Toshi::TPhysicsCollisionInfo* a_pInfo )
{
	// Ink surfaces
	if ( a_pInfo->m_other.m_uiMaterial & 0x80000 )
		m_pOwner->ChangeColour( ABlobColourComponent::COLOUR_BLACK );

	if ( a_pInfo->m_other.m_uiMaterial & 0x20000 )
	{
		m_pOwner->ChangeColour( ABlobColourComponent::COLOUR_BLACK );

		if ( m_pOwner->IsControllable() )
			m_pOwner->InkBounce();
	}

	// A big Blob hitting something hard shakes the camera
	ABlobEntity* pOwner = m_pOwner;

	if ( pOwner->m_iSize < 6 )
		return;

	TVector4 vDirection;
	TFLOAT   fSpeed;

	if ( pOwner->m_pZJump->GetState() == ABlobZJumpComponent::STATE_ATTACKSLAM )
	{
		vDirection = a_pInfo->m_self.m_vVelocity;
		fSpeed     = vDirection.NormaliseMag();

		if ( fSpeed <= 1.0f )
			return;
	}
	else
	{
		if ( pOwner->m_iOldSize != pOwner->m_iSize )
			return;

		if ( 0.0f < pOwner->m_fUnkB34 )
			return;

		TPhysicsObjectContainer* pOther = a_pInfo->m_other.m_pBody;

		if ( pOther && pOther->m_pUserData && *static_cast<TINT*>( pOther->m_pUserData ) == 2 )
			return;

		vDirection = a_pInfo->m_self.m_vVelocity;
		fSpeed     = vDirection.NormaliseMag();

		if ( fSpeed <= ms_pBlobCommonProperties->m_afUnk38[ 7 ] * 0.5f )
			return;
	}

	TFLOAT fDot = a_pInfo->m_vNormal.y * vDirection.y + a_pInfo->m_vNormal.x * vDirection.x + a_pInfo->m_vNormal.z * vDirection.z;

	if ( 0.5f < TMath::Abs( fDot ) )
		ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 0, pOwner->m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 1.0f, -1 );
}

// The contact data of the HandleEntityIO messages 4/5 sent by OnCollisionProcess (0x24 bytes)
struct BlobContact
{
	TVector4                      m_vPosition; // 0x00
	TVector4                      m_vNormal;   // 0x10
	Toshi::TPhysicsCollisionInfo* m_pInfo;     // 0x20
};

// Blob's body touched something: records the contacts that drive the landing and wall/ceiling handling
// $deBlob: FUNCTION 0073b7e0
void ABlobEntity::ABlobCollisionEventHandler::OnCollisionProcess( Toshi::TPhysicsCollisionInfo* a_pInfo )
{
	if ( m_pOwner == TNULL )
		return;

	TASSERT( m_pOwner->m_pBody == a_pInfo->m_self.m_pBody );

	ABlobEntity*             pOwner       = m_pOwner;
	TPhysicsObjectContainer* pOther       = a_pInfo->m_other.m_pBody;
	TUINT32                  uiOtherLayer = pOther ? ( pOther->m_uiCollisionFilterInfo & 0xfff ) : 0;
	ABaseEntity*             pOtherEntity = TNULL;

	// The contact as sent to the entities (HandleEntityIO messages 4: touched by, 5: touched)
	BlobContact oContact;
	oContact.m_vPosition = a_pInfo->m_vPosition;
	oContact.m_vNormal   = a_pInfo->m_vNormal;
	oContact.m_pInfo     = a_pInfo;

	if ( pOther && pOther->m_pUserData )
	{
		ACollisionInstance2::OwnerInfo* pOwnerInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, pOther->m_pUserData );
		TINT                            eType      = pOwnerInfo->m_eType;

		if ( eType == 2 )
		{
			// Another entity
			pOtherEntity = TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner );

			if ( pOwner->m_pZJump->GetState() == ABlobZJumpComponent::STATE_ATTACKSLAM )
			{
				TVector4 vDirection = a_pInfo->m_vPosition - pOtherEntity->GetBasePosition();
				vDirection.NormaliseMag();

				// The Z-jump hits the entity
				AAttackMessage  oAttack( AAttackMessage::ATTACK_PlayerSlamDirect, a_pInfo->m_vPosition, pOwner->m_iSize, 4, vDirection, TFALSE );
				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_SLAM;
				oPacket.m_iData    = TINT( &oAttack );
				pOtherEntity->HandleEntityIO( pOwner, oPacket );
			}

			if ( !pOtherEntity->IsDead() )
			{
				AEntityIOPacket oMessage = { AEntityIOPacket::MESSAGE_TOUCH, TREINTERPRETCAST( TINT, &oContact ) };
				pOtherEntity->HandleEntityIO( pOwner, oMessage );
			}

			AEntityIOPacket oMessage = { AEntityIOPacket::MESSAGE_TOUCHEDBY, TREINTERPRETCAST( TINT, &oContact ) };
			pOwner->HandleEntityIO( pOtherEntity, oMessage );

			if ( pOtherEntity->IsDead() )
			{
				pOwner->m_pZJump->OnEntityRemoved( pOtherEntity );

				if ( pOwner->m_pZJump->GetState() == ABlobZJumpComponent::STATE_ATTACKSLAM )
				{
					TPhysicsManager::SetLinearVelocity( pOwner->m_pBody, TVector4( 0.0f, ms_pBlobCommonProperties->m_afUnk38[ 7 ], 0.0f, 1.0f ) );
					TPhysicsManager::SetAngularVelocity( pOwner->m_pBody, TVector4::VEC_ZERO );

					if ( pOtherEntity->GetClass() == &TGetClass( ALeechBot2 ) )
						return;

					pOwner->m_pAnimationComponent->StartCelebrateTimer();
					return;
				}
			}
		}
		else if ( eType == 1 )
		{
			// The other Blob (multiplayer)
			pOtherEntity = TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner );
			TASSERT( pOtherEntity->IsA( &TGetClass( ABlobEntity ) ) );

			AEntityIOPacket oMessage4 = { AEntityIOPacket::MESSAGE_TOUCH, TREINTERPRETCAST( TINT, &oContact ) };
			AEntityIOPacket oMessage5 = { AEntityIOPacket::MESSAGE_TOUCHEDBY, TREINTERPRETCAST( TINT, &oContact ) };
			pOtherEntity->HandleEntityIO( pOwner, oMessage4 );
			pOwner->HandleEntityIO( pOtherEntity, oMessage5 );

			if ( pOwner->IsZJumpingAtPlayer() )
			{
				// The Z-jump hits the other blob
				TVector4 vDirection = a_pInfo->m_vPosition - pOtherEntity->GetBasePosition();
				vDirection.NormaliseMag();

				AAttackMessage  oAttack( AAttackMessage::ATTACK_PlayerSlamDirect, a_pInfo->m_vPosition, pOwner->m_iSize, 4, vDirection, TFALSE );
				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_SLAM;
				oPacket.m_iData    = TINT( &oAttack );
				pOtherEntity->HandleEntityIO( pOwner, oPacket );
			}
		}
		else if ( eType == 3 && ( uiOtherLayer & 0x40 ) )
		{
			TINT eTargetsState = pOwner->m_pZJump->GetState();

			if ( eTargetsState == 5 )
			{
				TPhysicsManager::SetLinearVelocity( pOwner->m_pBody, TVector4( 0.0f, ms_pBlobCommonProperties->m_afUnk38[ 7 ], 0.0f, 1.0f ) );
				TPhysicsManager::SetAngularVelocity( pOwner->m_pBody, TVector4::VEC_ZERO );
				return;
			}

			if ( eTargetsState == 9 )
				return;
		}
	}

	TVector4 vNormalOffset;
	vNormalOffset.Multiply( a_pInfo->m_vNormal, 10.0f );
	TVector4 vProbe( a_pInfo->m_vPosition.x + vNormalOffset.x, a_pInfo->m_vPosition.y + vNormalOffset.y, a_pInfo->m_vPosition.z + vNormalOffset.z, a_pInfo->m_vPosition.w );
	// NOTE: retail then calls an empty debug draw of the line ( &a_pInfo->m_vPosition, &vProbe, 0xffff0000, 1)
	(void)vProbe;

	pOwner->m_uiFlags &= ~0x1000000;
	pOwner->m_vClimbStart          = pOwner->m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	pOwner->m_fStretch             = 1.0f;
	pOwner->m_fStretchCompensation = 1.0f;
	pOwner->m_vStretchDirection    = TVector4::VEC_ZERO;
	pOwner->m_fStretchTimeLeft     = 0.0f;

	pOwner->m_oMotion.m_auiUnk280[ 4 ] = TREINTERPRETCAST( TUINT32, a_pInfo->m_other.m_pBody );
	TFLOAT fImpactSpeed                = TMath::Abs( a_pInfo->m_self.m_vVelocity.x * a_pInfo->m_vNormal.x + a_pInfo->m_self.m_vVelocity.y * a_pInfo->m_vNormal.y + a_pInfo->m_self.m_vVelocity.z * a_pInfo->m_vNormal.z );

	pOwner->m_pZJump->SetContactBody( a_pInfo->m_other.m_pBody );

	// The Z-jump slam (or the Z-jump at a locked entity with the flag 0x10) lands: every other entity within the
	// size's slam radius of the contact gets attacked (MESSAGE_SLAM, ATTACKTYPE 4), the shockwave in single player
	ABlobZJumpComponent*                 pZJump = pOwner->m_pZJump;
	ABlobZJumpComponent::PriorityTarget* pLock  = pZJump->GetCurrentPriorityLock();

	if ( pZJump->GetState() == ABlobZJumpComponent::STATE_ATTACKSLAM ||
	     ( pZJump->GetState() == ABlobZJumpComponent::STATE_ATTACKFLIGHT && pLock != TNULL && pLock->m_eType == ABlobZJumpComponent::PriorityTarget::TARGET_ENTITY &&
	       ( TSTATICCAST( ABaseEntity, pLock->m_pTarget )->GetFlags() & 0x10 ) != 0 ) )
	{
		AVibrationManager::VibrateOnce( APlayerManager::GetSingleton()->GetPlayerIndex( pOwner ), 2 );

		TINT          iSize        = pOwner->m_iSize;
		TFLOAT        fSlamRadius  = ms_pBlobProperties[ iSize ].m_fSlamRadius;
		TINT          iNumEntities = 0;
		ABaseEntity** ppEntities   = AEntityHash::GetSingleton()->Query( a_pInfo->m_vPosition, fSlamRadius, &iNumEntities, TNULL, TFALSE, 500, TFALSE, TTRUE, TFALSE );

		for ( TUINT i = 0; i < TUINT( iNumEntities ); i++ )
		{
			ABaseEntity* pEntity = ppEntities[ i ];

			if ( pEntity == pOwner || pEntity == pOtherEntity )
				continue;

			TVector4 vDirection = a_pInfo->m_vPosition - pEntity->GetBasePosition();
			TFLOAT   fDistance  = vDirection.NormaliseMag() - pEntity->GetBoundingSphereRadius();

			if ( fDistance < fSlamRadius )
			{
				AAttackMessage oAttack( AAttackMessage::ATTACK_PlayerSlamSplash, a_pInfo->m_vPosition, iSize, 4, vDirection, TFALSE );

				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_SLAM;
				oPacket.m_iData    = TINT( &oAttack );
				pEntity->HandleEntityIO( pOwner, oPacket );
			}
		}

		pZJump->OnRecoil();

		if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE )
			pOwner->StartShockwave( a_pInfo->m_vPosition, a_pInfo->m_vNormal, 0.45f, 1.0f, TFALSE );
	}

	// A hard landing: the paint trail splats
	if ( fImpactSpeed > 20.0f && ( ( pOwner->m_pColourComponent->GetCurrentColour() >> 4 ) & 1 ) == 0 )
		pOwner->m_pRibbonTrail->OnLanding( a_pInfo->m_vNormal );

	pOwner->m_uiFlags |= 4;

	if ( a_pInfo->m_vNormal.y < -s_fGroundContactCos )
	{
		// The ground (the normal points down from Blob)
		pOwner->m_oMotion.m_uiContactFlags |= 0x20;
		pOwner->m_oMotion.m_auiUnk280[ 0 ] = TREINTERPRETCAST( TUINT32, a_pInfo->m_other.m_pBody );
		pOwner->m_oMotion.m_vUnk240        = a_pInfo->m_vPosition;
		pOwner->m_oMotion.m_auiUnk280[ 1 ] = a_pInfo->m_other.m_uiMaterial;
	}
	else if ( a_pInfo->m_vNormal.y < s_fWallContactCos )
	{
		// A wall (steeper than 70 degrees)
		pOwner->m_oMotion.m_uiContactFlags |= 0x10;
		pOwner->m_oMotion.m_auiUnk280[ 2 ] = TREINTERPRETCAST( TUINT32, a_pInfo->m_other.m_pBody );
		pOwner->m_oMotion.m_vUnk250        = a_pInfo->m_vPosition;
		pOwner->m_oMotion.m_auiUnk280[ 3 ] = a_pInfo->m_other.m_uiMaterial;
	}

	// A cell instance (owner type 3): the touch callback of its type
	if ( ( uiOtherLayer & 0xc9 ) && pOther && pOther->m_pUserData &&
	     TSTATICCAST( ACollisionInstance2::OwnerInfo, pOther->m_pUserData )->m_eType == 3 )
	{
		TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, TSTATICCAST( ACollisionInstance2::OwnerInfo, pOther->m_pUserData )->m_pOwner );

		switch ( pInstance->m_uiFlags & 7 )
		{
			case TerrainTypes::InstanceType_Building:
				ABuildingInstance::OnBlobTouched( pInstance, pOwner, &a_pInfo->m_vPosition );
				break;
			case TerrainTypes::InstanceType_Landmark:
				ALandmarkInstance::OnBlobTouched( pInstance, pOwner, &a_pInfo->m_vPosition );
				break;
			case TerrainTypes::InstanceType_Tree:
				ATreeInstance::OnBlobTouched( pInstance, pOwner, &a_pInfo->m_vPosition );
				break;
			case TerrainTypes::InstanceType_TextureSwap:
				ATextureSwapInstance::OnBlobTouched( pInstance, pOwner, &a_pInfo->m_vPosition );
				break;
			case TerrainTypes::InstanceType_ToggleVis:
				AToggleVisInstance::OnBlobTouched( pInstance, pOwner, &a_pInfo->m_vPosition );
				break;
		}

		if ( ( pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Landmark )
		{
			// A landmark: the landmark rings around Blob
			// Touching a landmark that isn't a billboard and isn't transformed (not while in it): the unpaintable
			// sound and the unpaintable effect at the contact, facing along its normal (updated once a frame)
			ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, pInstance->m_pModelInstance2 );

			if ( !pLandmark->IsABillBoard() && ( pLandmark->GetBuildingFlags() & ALandmarkInstance::Flags_Completed ) == 0 && pOwner->m_eState != STATE_LMARK )
			{
				if ( pOwner->m_pSoundComponent != TNULL )
					pOwner->m_pSoundComponent->PlayLockedBuilding( pLandmark );

				static TVector4  s_vUnpaintableNormal = TVector4::VEC_ZERO;
				static TMatrix44 s_oUnpaintableMatrix;
				static TUINT32   s_uiUnpaintableFrame;

				pOwner->m_oMotion.m_uiContactFlags |= 0x80;

				TParticleEffectInstance*& rpUnpaintable = *TREINTERPRETCAST( TParticleEffectInstance**, &pOwner->m_auiSounds[ 16 ] ); // 0xB18

				if ( rpUnpaintable == TNULL )
					rpUnpaintable = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pUnpaintableEffect );

				if ( rpUnpaintable != TNULL && s_uiUnpaintableFrame != TRender::GetSingleton()->GetFrameCount() )
				{
					s_uiUnpaintableFrame = TRender::GetSingleton()->GetFrameCount();

					TVector4 vChange( a_pInfo->m_vNormal.x - s_vUnpaintableNormal.x, a_pInfo->m_vNormal.y - s_vUnpaintableNormal.y, a_pInfo->m_vNormal.z - s_vUnpaintableNormal.z, a_pInfo->m_vNormal.w );

					if ( vChange.MagnitudeSq() > 0.05f )
					{
						s_vUnpaintableNormal = a_pInfo->m_vNormal;
						s_oUnpaintableMatrix = TMatrix44::IDENTITY;
						// NOTE: retail also computes the squared length of VEC_NEGY - the normal here and drops it
						s_oUnpaintableMatrix.LookAtDirection( s_vUnpaintableNormal, TVector4::VEC_NEGY );
						s_oUnpaintableMatrix.RotateX( -TMath::HALF_PI );
					}

					rpUnpaintable->GetTransform() = s_oUnpaintableMatrix;

					// 0.05 out of the surface
					TVector4 vOffset;
					vOffset.Multiply( s_vUnpaintableNormal, 0.05f );
					TVector4 vPosition( vOffset.x + a_pInfo->m_vPosition.x, a_pInfo->m_vPosition.y + vOffset.y, a_pInfo->m_vPosition.z + vOffset.z, a_pInfo->m_vPosition.w );
					rpUnpaintable->GetTransform().SetTranslation( vPosition );

					TFLOAT fScale = ms_pBlobProperties[ pOwner->m_iSize ].m_fRadius * 0.24f;
					rpUnpaintable->GetTransform().Scale( fScale, fScale, fScale );
				}
			}
		}
	}
}

// $deBlob: FUNCTION 00745f00
void ABlobEntity::Vibrate()
{
	AVibrationManager::VibrateOnce( APlayerManager::GetSingleton()->GetPlayerIndex( this ), 4 );
}

// $deBlob: FUNCTION 00745f30
void ABlobEntity::SetAmmoHud( ABlobAmmoHud* a_pAmmoHud )
{
	m_pAmmoHud = a_pAmmoHud;

	if ( a_pAmmoHud != TNULL )
	{
		// Any ink colour shows as ink
		TUINT32 uiColour = GetPaintColour();
		if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
			uiColour = ABlobColourComponent::COLOUR_BLACK;

		a_pAmmoHud->SetColour( uiColour );
	}
}

// $deBlob: FUNCTION 00745840
TINT ABlobEntity::GetShakeProgress() const
{
	if ( TUINT32( m_eState - STATE_HOTPLATE ) > 2 )
		return 0;

	// 0xAD2: the shakes done, 0xAD4: the shakes needed
	TINT16 iShakesNeeded = TINT16( m_uiShakesNeeded );

	if ( iShakesNeeded == 0 )
		return 100;

	return TINT( ( TFLOAT( TINT16( m_uiTargettingAndShakes >> 16 ) ) * 100.0f ) / TFLOAT( iShakesNeeded ) );
}
