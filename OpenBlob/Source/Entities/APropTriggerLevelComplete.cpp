#include "pch.h"
#include "APropTriggerLevelComplete.h"
#include "AEntityParams.h"
#include "AEntityIOPacket.h"
#include "AEntityModelComponent.h"
#include "AProp.h"
#include "APropDoor.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "Cameras/ACameraManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "GameController/AGameStatus.h"
#include "Sound/AMusicManager.h"

#include <TParticleSystem/TParticleEffectInstance.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropTriggerLevelComplete ) == 0x12C );

// $deBlob: FUNCTION 0076df40
// $deBlob: FUNCTION 0076df50
// $deBlob: FUNCTION 0076dfc0
// $deBlob: FUNCTION 0076e020
// $deBlob: FUNCTION 0076e030
// $deBlob: FUNCTION 005408e0
// $deBlob: FUNCTION 00859cb0
TDEFINE_CLASS( APropTriggerLevelComplete );

// $deBlob: FUNCTION 0076e040
APropTriggerLevelComplete::APropTriggerLevelComplete()
{
	m_mGrateMatrix      = TMatrix44::IDENTITY;
	m_eState            = STATE_Waiting;
	m_pExitDoor         = TNULL;
	m_pGrate            = TNULL;
	m_pBlob             = TNULL;
	m_iGrateOpenAnimID  = -1;
	m_iGrateCloseAnimID = -1;
	m_iDoorCamAnimID    = -1;
	m_iDoorCamBoneID    = -1;
	m_iDoorBlobBoneID   = -1;
	m_fUnk11C           = 0.0f;
	m_fUnk120           = 0.0f;
	m_pExitPoolEffect   = TNULL;
	m_pExitPool         = TNULL;

	TASSERT( !AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );

	TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType )
	[1] = 3;
	SetHash( TTRUE );
}

// $deBlob: FUNCTION 0076ebf0
// $deBlob: FUNCTION 0076ebd8
// $deBlob: FUNCTION 0076e1a0
APropTriggerLevelComplete::~APropTriggerLevelComplete()
{
	if ( m_pExitPool )
	{
		m_pExitPool->StopInstant();
		m_pExitPool = TNULL;
	}
}

// Starts the exit pool effect at the trigger
static TParticleEffectInstance* StartExitPool( TParticleEffect* a_pEffect, const TMatrix44& a_rMatrix )
{
	TParticleEffectInstance* pInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( a_pEffect );

	if ( pInstance )
		pInstance->GetTransform() = a_rMatrix;

	return pInstance;
}

// The direction from the trigger to the current camera, in the XZ plane
static TVector4 GetFacingToCamera( const TMatrix44& a_rMatrix )
{
	const TMatrix44& rCamMatrix = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();

	TVector4 vFacing;
	vFacing.x = rCamMatrix.m_f41 - a_rMatrix.m_f41;
	vFacing.y = rCamMatrix.m_f42 - a_rMatrix.m_f42;
	vFacing.z = rCamMatrix.m_f43 - a_rMatrix.m_f43;
	vFacing.w = rCamMatrix.m_f44;
	vFacing.NormaliseXZ();

	return vFacing;
}

// $deBlob: FUNCTION 0076e500
void APropTriggerLevelComplete::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_eState == STATE_Waiting )
	{
		// Ready once the exit door is open
		if ( m_pExitDoor->GetDoorState() == APropDoor::DOORSTATE_Open )
		{
			m_pBlob = TNULL;

			if ( m_pExitPool == TNULL && m_pExitPoolEffect != TNULL )
				m_pExitPool = StartExitPool( m_pExitPoolEffect, m_oMatrix );

			m_eState = STATE_Ready;
		}
	}
	else if ( m_eState == STATE_Entering )
	{
		m_pBlob->SetFacing( GetFacingToCamera( m_oMatrix ) );

		// NOTE: the level exit animation: element 6 of the Anim array that starts BlobAnimationProperties' second table
		TINT16 iExitAnim = *TREINTERPRETCAST( TINT16*, TREINTERPRETCAST( TUINT8*, ABlobAnimationComponent::ms_pAnimationProperties->m_pTable->m_pAnims00 ) + 0x64 );

		AToshiAnimInterface2* pAnimInterface = TSTATICCAST( ABlobEntity, m_pBlob )->GetAnimationComponent()->GetCharModelInstance()->GetAnimInterface();

		TBOOL bStillExiting = !pAnimInterface->IsAnimationFinished( iExitAnim ) &&
		    pAnimInterface->GetSkeletonInstance() != TNULL &&
		    pAnimInterface->GetSkeletonInstance()->IsAnimating( iExitAnim );

		if ( !bStillExiting && m_eState != STATE_Completed )
		{
			AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;
			AGameStatus::GetSingleton()->OnLevelComplete();
			m_eState = STATE_Completed;
		}
	}

	APropTrigger::Tick( a_fDeltaTime );
}

// The bones' space to the world's (Y and Z swapped)
static const TMatrix44 s_mBoneToWorld = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

// Out: the world matrix of the door's camera bone ("camera_loc"); returns m_fUnk120 (used by ABlobFollowCamera
// while the blob exits the level)
// $deBlob: FUNCTION 0076e800
TFLOAT APropTriggerLevelComplete::GetDoorCameraMatrix( TMatrix44& a_rOutMatrix )
{
	TSTATICCAST( AEntityModelComponent, m_pExitDoor->GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetSkeletonInstance()->UpdateState( TFALSE );
	TSTATICCAST( AEntityModelComponent, m_pExitDoor->GetModelComponent() )->GetModelInstance()->GetBoneTransform( m_iDoorCamBoneID, a_rOutMatrix );

	TMatrix44 mBone = a_rOutMatrix;
	a_rOutMatrix.Multiply( mBone, s_mBoneToWorld );

	return m_fUnk120;
}

// Out: the world matrix of the door's blob bone ("blobattach_loc"), where Blob is held while it exits the level
// (ABlobEntity::UpdateModelTransforms)
// $deBlob: FUNCTION 0076e890
void APropTriggerLevelComplete::GetDoorBlobMatrix( TMatrix44& a_rOutMatrix )
{
	TSTATICCAST( AEntityModelComponent, m_pExitDoor->GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetSkeletonInstance()->UpdateState( TFALSE );
	TSTATICCAST( AEntityModelComponent, m_pExitDoor->GetModelComponent() )->GetModelInstance()->GetBoneTransform( m_iDoorBlobBoneID, a_rOutMatrix );

	TMatrix44 mBone = a_rOutMatrix;
	a_rOutMatrix.Multiply( mBone, s_mBoneToWorld );
}

// The time of the door's camera animation ("LevelEnd_Camera_Alt"), 0.0f if it isn't playing (ABlobAnimationComponent's
// level exit state)
// $deBlob: FUNCTION 0076e920
TFLOAT APropTriggerLevelComplete::GetDoorCameraAnimTime()
{
	TSkeletonInstance* pSkeletonInst = TSTATICCAST( AEntityModelComponent, m_pExitDoor->GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetSkeletonInstance();

	if ( pSkeletonInst != TNULL )
	{
		TAnimation* pAnim = pSkeletonInst->IsAnimating( m_iDoorCamAnimID );

		if ( pAnim != TNULL )
			return pAnim->GetTotalTime();
	}

	return 0.0f;
}

// $deBlob: FUNCTION 0076e960
void APropTriggerLevelComplete::SetState( STATE a_eState )
{
	if ( m_eState == a_eState )
		return;

	switch ( a_eState )
	{
		case STATE_Waiting:
			if ( m_pExitPool )
			{
				m_pExitPool->Stop();
				m_pExitPool = TNULL;
			}

			m_eState = a_eState;
			m_pBlob  = TNULL;
			return;
		case STATE_Ready:
			m_pBlob = TNULL;

			if ( m_pExitPool == TNULL && m_pExitPoolEffect != TNULL )
				m_pExitPool = StartExitPool( m_pExitPoolEffect, m_oMatrix );

			break;
		case STATE_Entering:
		{
			if ( m_pExitPool )
			{
				m_pExitPool->Stop();
				m_pExitPool = TNULL;
			}

			AMusicManager::GetSingleton()->PlayLevelComplete();
			AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TTRUE;

			m_pExitDoor->KillManualAllAnimations( -1.0f );
			m_pExitDoor->AddManualAnimation( m_iDoorCamAnimID, 0.0f, 0.0f, 1.0f, 0.0f );
			m_fUnk11C = 0.0f;
			m_fUnk120 = 1.0f;

			TVector4 vFacing = GetFacingToCamera( m_oMatrix );
			TSTATICCAST( ABlobEntity, m_pBlob )->StartLevelExit( this, vFacing );

			m_pGrate->KillManualAllAnimations( -1.0f );
			m_pGrate->AddManualAnimation( m_iGrateOpenAnimID, 0.0f, 0.0f, 1.0f, 1.0f );
			m_pExitDoor->KillManualAllAnimations( -1.0f );
			m_pExitDoor->AddManualAnimation( m_iDoorCamAnimID, 0.0f, 0.0f, 1.0f, 1.0f );
			m_eState = a_eState;
			return;
		}
		case STATE_Completed:
			AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;
			AGameStatus::GetSingleton()->OnLevelComplete();
			break;
	}

	m_eState = a_eState;
}

// $deBlob: FUNCTION 0076e240
// $deBlob: FUNCTION 0076ec9d
void APropTriggerLevelComplete::PostCreate()
{
	APropTrigger::PostCreate();

	for ( TINT i = 0; i < GetNumEntityTargets(); i++ )
	{
		ABaseEntity* pTarget = GetEntityTarget( i );

		if ( pTarget->GetClass() == &TGetClass( AProp ) )
			m_pGrate = TSTATICCAST( AProp, pTarget );
		else if ( pTarget->GetClass() == &TGetClass( APropDoor ) )
			m_pExitDoor = TSTATICCAST( APropDoor, pTarget );
	}

	TASSERT( m_pGrate && "Trigger Level Complete not linked to an exit grate prop" );
	TASSERT( m_pExitDoor && "Trigger Level Complete not linked to an exit door" );

	m_mGrateMatrix = m_pGrate->GetMatrix();

	AToshiAnimInterface2* pGrateAnim = TSTATICCAST( AEntityModelComponent, m_pGrate->GetModelComponent() )->GetModelInstance()->GetAnimInterface();
	m_iGrateOpenAnimID               = TINT16( pGrateAnim->GetAnimID( "LevelEnd_Alt" ) );
	TASSERT( -1 < m_iGrateOpenAnimID && "Could not find the LevelEnd animation in the exit grate model" );
	m_iGrateCloseAnimID = TINT16( pGrateAnim->GetAnimID( "Idle" ) );
	TASSERT( -1 < m_iGrateCloseAnimID && "Could not find the Idle animation in the exit grate model" );

	AToshiAnimInterface2* pDoorAnim = TSTATICCAST( AEntityModelComponent, m_pExitDoor->GetModelComponent() )->GetModelInstance()->GetAnimInterface();
	m_iDoorCamAnimID                = TINT16( pDoorAnim->GetAnimID( "LevelEnd_Camera_Alt" ) );
	TASSERT( -1 < m_iDoorCamAnimID && "Could not find the LevelEnd_Camera animation in the exit door model" );

	TSkeleton* pDoorSkeleton = pDoorAnim->GetSkeletonInstance()->GetSkeleton();
	m_iDoorCamBoneID         = TINT16( pDoorSkeleton->GetBoneID( "camera_loc", -1 ) );
	TASSERT( -1 < m_iDoorCamBoneID && "Could not find camera_loc bone in the exit door model" );
	m_iDoorBlobBoneID = TINT16( pDoorSkeleton->GetBoneID( "blobattach_loc", -1 ) );
	TASSERT( -1 < m_iDoorBlobBoneID && "Could not find blobattach_loc bone in the exit door model" );

	m_pExitPoolEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_ExitPool" );
}

// $deBlob: FUNCTION 0076e220
// $deBlob: FUNCTION 0076eca8
void APropTriggerLevelComplete::ProcessParams( const AEntityParams& a_rParams )
{
	APropTrigger::ProcessParams( a_rParams );
	m_uiTriggerFlags |= TRIGGERFLAG_UseSlamAttack;
}

// $deBlob: FUNCTION 0076e770
void APropTriggerLevelComplete::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( m_eState == STATE_Ready && a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_SLAM && a_pCaller->GetClass() == &TGetClass( ABlobEntity ) )
	{
		m_pBlob = a_pCaller;
		SetState( STATE_Entering );

		TSTATICCAST( ABlobEntity, m_pBlob )->AddBlobFlags( 0x40000000 );

		TXUI* pXUI = TXUI::GetSingleton();

		if ( pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() )
			pXUI->TransitionTo( TNULL, pXUI->CreateScene( pXUI->FindResource( "letterbox.xur" ), 0u ) );
	}
}

// $deBlob: FUNCTION 0076e680
void APropTriggerLevelComplete::Reset( ABaseEntity* a_pEntity )
{
	APropTrigger::Reset( a_pEntity );

	if ( m_eState != STATE_Waiting )
	{
		if ( m_pExitPool )
		{
			m_pExitPool->Stop();
			m_pExitPool = TNULL;
		}

		m_pBlob  = TNULL;
		m_eState = STATE_Waiting;
	}

	m_pGrate->SetMatrix( m_mGrateMatrix );
	m_pGrate->SetVisible( TTRUE );
	m_pGrate->KillManualAllAnimations( 0.0f );
	m_pGrate->AddManualAnimation( m_iGrateCloseAnimID, 0.0f, 0.15f, 1.0f, 1.0f );

	m_pBlob   = TNULL;
	m_fUnk11C = 0.0f;
	m_fUnk120 = 0.0f;

	if ( m_pExitPool )
	{
		m_pExitPool->StopInstant();
		m_pExitPool = TNULL;
	}
}
