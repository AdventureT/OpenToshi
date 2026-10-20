#include "pch.h"
#include "APropDoor.h"
#include "AEntityParams.h"
#include "AEntityModelComponent.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AModel2.h"
#include "Animation/AToshiAnimInterface2.h"

#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "GameInterface/APlayerManager.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "APropSwitchPressure.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"

#include <Render/TAnimation.h>
#include <Render/TSkeleton.h>
#include <Render/TModel.h>
#include <XUI/TXUI.h>
#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropDoor ) == 0xD8 );

// The bones' space to the world's (Y and Z swapped)
static const TMatrix44 s_mBoneToWorld = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

// $deBlob: FUNCTION 00788490
// $deBlob: FUNCTION 007884a0
// $deBlob: FUNCTION 007884e0
// $deBlob: FUNCTION 00788510
// $deBlob: FUNCTION 00788570
// $deBlob: FUNCTION 00788580
// $deBlob: FUNCTION 00540c20
// $deBlob: FUNCTION 00859d80
TDEFINE_CLASS( APropDoor );

// $deBlob: FUNCTION 00788590
APropDoor::APropDoor()
{
	m_iOpenAnim              = -1;
	m_iCloseAnim             = -1;
	m_iLevelStartOpenAnim    = -1;
	m_iLevelStartRespawnAnim = -1;
	m_iCloseScriptedSequence = -1;
	m_bStartOpen             = TFALSE;
	m_eDoorType              = DOORTYPE_Door;
	m_fBBoxScale             = 1.0f;
	m_eState                 = DOORSTATE_Closed;
	m_iSwitch                = 0;
	m_fCloseDelay            = 0.0f;
	m_fCloseTimer            = 0.0f;
}

// $deBlob: FUNCTION 007896f0
// $deBlob: FUNCTION 007896de
// $deBlob: FUNCTION 00788610
APropDoor::~APropDoor()
{
	KillManualAllAnimations( -1.0f );
}

// $deBlob: FUNCTION 00788680
void APropDoor::Tick( TFLOAT a_fDeltaTime )
{
	AProp::Tick( a_fDeltaTime );

	switch ( m_eState )
	{
		case DOORSTATE_Opening:
			if ( IsManualAnimationFinished( m_iOpenAnim ) == TTRUE )
			{
				m_eState                 = DOORSTATE_Open;
				AModelInstance2*   pInst = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
				TSkeletonInstance* pSkel = pInst->GetAnimInterface()->GetSkeletonInstance();
				pSkel->UpdateTime( 0.0f );
				pSkel->UpdateState( TFALSE );
				SetTicking( TFALSE );

				if ( m_eDoorType == DOORTYPE_Sprint )
				{
					TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetMeshFlags( 0, 2, TFALSE );
					return;
				}
			}
			break;
		case DOORSTATE_Closing:
			if ( IsManualAnimationFinished( m_iCloseAnim ) == TTRUE )
			{
				AModelInstance2*   pInst = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
				TSkeletonInstance* pSkel = pInst->GetAnimInterface()->GetSkeletonInstance();
				pSkel->UpdateTime( 0.0f );
				pSkel->UpdateState( TFALSE );
				m_eState = DOORSTATE_Closed;
				SetTicking( TFALSE );
				return;
			}
			break;
		case DOORSTATE_WaitForClose:
		{
			// Closes once the HUD is idle, or (playing) after the close delay
			TBOOL bClose = TFALSE;

			if ( !APlayerManager::GetSingleton()->GetBlobData( 0 )->IsKeyFrameCameraActive() )
			{
				TXUI* pXUI = TXUI::GetSingleton();

				if ( pXUI->GetActiveScene() != TNULL && pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() &&
				     pXUI->GetActiveScene()->IsA( &TGetClass( ABlobHud ) ) )
				{
					bClose = TTRUE;
				}
			}
			else if ( APlayerManager::GetSingleton()->GetBlobData( 0 )->m_iCameraState == 5 )
			{
				m_fCloseTimer -= a_fDeltaTime;
				bClose = m_fCloseTimer < 0.0f;
			}

			if ( bClose )
			{
				m_fCloseTimer = 0.0f;
				Close();
				return;
			}
		}
		break;
		case DOORSTATE_LevelStart:
		{
			AModelInstance2*   pInst = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
			TSkeletonInstance* pSkel = pInst->GetAnimInterface()->GetSkeletonInstance();
			TAnimation*        pAnim = GetLevelStartAnimation();

			// Held at its start while the screen fades in
			AGameStateController* pController = AGameStateController::GetSingleton();

			if ( pController->IsFadeEnabled( 0 ) && pAnim != TNULL && pController->GetFade( 0 ) > 0.8f )
				pAnim->SetTime( 0.0f );

			if ( pSkel->GetOverlayAnimationCount() + pSkel->GetBaseAnimationCount() != 0 )
				pSkel->UpdateState( TFALSE );

			if ( pAnim == TNULL ||
			     ( pAnim->GetSequence() == m_iLevelStartOpenAnim && IsManualAnimationFinished( m_iLevelStartOpenAnim ) ) ||
			     ( pAnim->GetSequence() == m_iLevelStartRespawnAnim && IsManualAnimationFinished( m_iLevelStartRespawnAnim ) ) )
			{
				pSkel->UpdateTime( 0.0f );
				pSkel->UpdateState( TFALSE );
				m_eState = DOORSTATE_Open;
				pInst->SetMeshFlags( 0, 2, TFALSE );

				// The opened level start gate gets a bigger bounding box
				if ( pAnim != TNULL && pAnim->GetSequence() == m_iLevelStartOpenAnim )
				{
					TVector4 vBBox = GetBBox();
					vBBox.Multiply( 2.5f / m_fBBoxScale );
					SetBBox( vBBox );
					m_fBBoxScale = 2.5f;
				}

				SetTicking( TFALSE );
			}

			break;
		}
		break;
		default:
			break;
	}
}

// $deBlob: FUNCTION 00788b60
// $deBlob: FUNCTION 0078976e
void APropDoor::ProcessParams( const AEntityParams& a_rParams )
{
	AProp::ProcessParams( a_rParams );
	a_rParams.GetValue( m_bStartOpen, "StartOpen" );
	a_rParams.GetValue( TREINTERPRETCAST( TUINT8&, m_eDoorType ), "DoorType" );

	m_iOpenAnim = CreateManualAnimation( "anim_open" );

	if ( m_iOpenAnim == -1 )
		m_iOpenAnim = CreateManualAnimation( "gate_open" );

	m_iCloseAnim = CreateManualAnimation( "anim_close" );

	if ( m_eDoorType == DOORTYPE_LevelStart )
	{
		m_iLevelStartOpenAnim    = CreateManualAnimation( "LevelStart_Open" );
		m_iLevelStartRespawnAnim = CreateManualAnimation( "LevelStart_Respawn" );
		AGameStatus::GetSingleton()->SetLevelStartGate( this );
	}
	else if ( m_eDoorType == DOORTYPE_Sprint )
	{
		if ( AGameStatus::GetSingleton()->GetSprintGate() == TNULL )
			AGameStatus::GetSingleton()->SetSprintGate( this );

		m_iOpenAnim = CreateManualAnimation( "gate_open_sprint" );
	}

	if ( !m_bStartOpen )
	{
		AddManualAnimation( m_iCloseAnim, -1.0f, 0.15f, 1.0f, 1.0f );
		m_eState = DOORSTATE_Closed;
	}
	else
	{
		AddManualAnimation( m_iOpenAnim, -1.0f, 0.15f, 1.0f, 1.0f );
		m_eState = DOORSTATE_Open;
	}

	CreateModelCollision( TTRUE );
	a_rParams.GetValue( m_iCloseScriptedSequence, "Close Scripted Sequence" );

	if ( m_iCloseScriptedSequence != -1 )
		a_rParams.GetValue( m_fCloseDelay, "Close delay" );

	if ( m_eDoorType == DOORTYPE_LevelStart || m_eDoorType == DOORTYPE_Sprint )
	{
		AModelInstance2* pInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
		T2ModelPtr       pModel    = pInstance->GetModel()->GetModelPtr();
		TModelLOD&       rLOD      = pModel->GetLOD( 0 );
		pModel.Release();
		TASSERT( rLOD.GetMeshCount() == 1, "The number of meshes in the door model has changed. Code needs to be updated." );

		pInstance->SetFlag( AModelInstance2::FLAG( 0x1000 ), TTRUE );
		pInstance->SetMeshFlags( 0, 3, TTRUE );
	}

	if ( m_eDoorType == DOORTYPE_Door )
	{
		AModelInstance2* pInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
		pInstance->SetFlag( AModelInstance2::FLAG( 0x2000 ), TFALSE );
		pInstance->SetFlag( AModelInstance2::FLAG( 0x200 ), TTRUE );

		// The meshes get their texture as the second stage over an empty first one (blend 0x13)
		T2ModelPtr pModel = pInstance->GetModel()->GetModelPtr();
		TModelLOD& rLOD   = pModel->GetLOD( 0 );
		pModel.Release();

		for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		{
			TMesh* pMesh = rLOD.GetMeshes()[ i ];
			TASSERT( pMesh == TNULL || pMesh->IsA( &TGetClass( ASkinMeshHAL ) ) );

			ASkinMaterialHAL* pMaterial = TSTATICCAST( ASkinMaterialHAL, pMesh->GetMaterial() );

			if ( pMaterial->GetNumTextures() != 2 )
			{
				TASSERT( pMaterial->GetNumTextures() == 1 );
				pMaterial->SetTextureNum( 2 );
				pMaterial->SetTexture( 1, pMaterial->GetTexture( 0 ) );
				pMaterial->SetTexture( 0, TNULL );
				TASSERT( pMaterial->GetTevMode( 0 ) == 0 );
				pMaterial->SetTevMode( 0, 0x13 );
				pMaterial->SetTevMode( 1, 1 );
			}
		}
	}
}

// $deBlob: FUNCTION 007889c0
void APropDoor::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	TINT iMessage = a_rPacket.m_eMessage;

	if ( iMessage == AEntityIOPacket::MESSAGE_TRIGGER || iMessage == AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE || iMessage == AEntityIOPacket::MESSAGE_TOGGLE )
	{
		if ( m_eState == DOORSTATE_Closed )
		{
			Open();
		}
		else if ( m_eState == DOORSTATE_Open )
		{
			if ( m_iCloseScriptedSequence != -1 )
			{
				m_eState = DOORSTATE_WaitForClose;

				if ( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint )
					APlayerManager::GetSingleton()->PlaySSCamera( 0, m_iCloseScriptedSequence );

				m_fCloseTimer = m_fCloseDelay;
				AProp::HandleEntityIO( a_pCaller, a_rPacket );
				return;
			}

			Close();
		}
	}
	else if ( iMessage == AEntityIOPacket::MESSAGE_OPEN && m_eState == DOORSTATE_Closed )
	{
		Open();
	}
	else if ( iMessage == AEntityIOPacket::MESSAGE_CLOSE && m_eState == DOORSTATE_Open )
	{
		Close();
	}
	else if ( iMessage == AEntityIOPacket::MESSAGE_TOUCH && m_eState == DOORSTATE_Closing && m_iSwitch != 0 )
	{
		// Opens again from the current point of the closing animation
		TREINTERPRETCAST( APropSwitchPressure*, m_iSwitch )->ReleaseByDoor();
		AToshiAnimInterface2* pAnimInterface = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface();
		TFLOAT                fCloseTime     = pAnimInterface->GetAnimationSeqTime( m_iCloseAnim );
		KillManualAllAnimations( 0.0f );
		TFLOAT fOpenLength = pAnimInterface->GetAnimationLength( m_iOpenAnim );
		AddManualAnimation( m_iOpenAnim, fOpenLength - fCloseTime, 0.0f, 1.0f, 1.0f );
		m_eState = DOORSTATE_Opening;
		SetTicking( TTRUE );
	}

	AProp::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 00788fb0
void APropDoor::Reset( ABaseEntity* a_pEntity )
{
	KillManualAllAnimations( -1.0f );

	if ( !m_bStartOpen )
	{
		AddManualAnimation( m_iCloseAnim, -1.0f, 0.15f, 1.0f, 1.0f );
		m_eState = DOORSTATE_Closed;
	}
	else
	{
		AddManualAnimation( m_iOpenAnim, -1.0f, 0.15f, 1.0f, 1.0f );
		m_eState = DOORSTATE_Open;
	}

	TSkeletonInstance* pSkel = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetSkeletonInstance();
	pSkel->UpdateTime( 0.0f );
	pSkel->UpdateState( TFALSE );

	AProp::Reset( a_pEntity );

	if ( m_fBBoxScale != 1.0f )
	{
		TVector4 vBBox = m_vBBox;
		vBBox.Divide( m_fBBoxScale );
		SetBBox( vBBox );
		m_fBBoxScale = 1.0f;
	}

	m_fCloseTimer = 0.0f;
	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetMeshFlags( 0, 2, TTRUE );
}

// $deBlob: FUNCTION 00789540
void APropDoor::Close()
{
	KillManualAllAnimations( -1.0f );
	AddManualAnimation( m_iCloseAnim, 0.0f, 0.0f, 1.0f, 1.0f );
	m_eState = DOORSTATE_Closing;
	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 007895a0
void APropDoor::Open()
{
	KillManualAllAnimations( -1.0f );
	AddManualAnimation( m_iOpenAnim, 0.0f, 0.0f, 1.0f, 1.0f );
	m_eState = DOORSTATE_Opening;
	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 00789600
void APropDoor::ApplyBlobColour( ABaseEntity* a_pCaller, void* a_pModelInstance )
{
	if ( m_eDoorType != DOORTYPE_Door )
	{
		AProp::ApplyBlobColour( a_pCaller, a_pModelInstance );
		return;
	}

	// A door takes the colour of the blob that touches it (one paint point), an inky blob resets it
	ABlobEntity*     pBlob     = TSTATICCAST( ABlobEntity, a_pCaller );
	AModelInstance2* pInstance = TSTATICCAST( AModelInstance2, a_pModelInstance );
	TUINT32          uiColour  = pBlob->GetColourComponent()->GetCurrentColour();

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
		uiColour = ABlobColourComponent::COLOUR_BLACK;
	else if ( uiColour == 0 )
		return;

	if ( !( uiColour & ABlobColourComponent::COLOUR_BLACK ) )
	{
		if ( pInstance->GetLocalAmbientColour() != ABlobColourComponent::ms_auiColours[ uiColour ] )
		{
			pInstance->SetLocalAmbientColour( ABlobColourComponent::ms_auiColours[ uiColour ] );
			pInstance->UseLocalAmbientColour( TTRUE );
			pBlob->RemovePaintPoints( 1, TFALSE );
		}

		return;
	}


	ResetAmbientColour( TSTATICCAST( AModelInstance2, a_pModelInstance ) );
}

// Plays the level start animation (the opening, or the respawn) the blob is attached to
// $deBlob: FUNCTION 007894b0
void APropDoor::PlayLevelStart( TBOOL a_bRespawn )
{
	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetMeshFlags( 0, 2, !a_bRespawn );
	KillManualAllAnimations( -1.0f );
	AddManualAnimation( a_bRespawn ? m_iLevelStartRespawnAnim : m_iLevelStartOpenAnim, 0.0f, 0.0f, 1.0f, 1.0f );
	m_eState = DOORSTATE_LevelStart;
	SetTicking( TTRUE );
}

// The camera of the level start: the "camera_Loc" bone, turned towards the level start camera point (or 15 degrees
// off the gate's facing), and how much of it to use (1 until 60% of the animation, then down to 0 at 80%); returns
// whether the level start goes on.
// $deBlob: FUNCTION 007890c0
TBOOL APropDoor::GetLevelStartCamera( TMatrix44& a_rOutMatrix, TFLOAT& a_rfWeight )
{
	if ( m_eState != DOORSTATE_LevelStart )
		return TFALSE;

	TAnimation* pAnim = GetLevelStartAnimation();

	if ( pAnim == TNULL )
		return TFALSE;

	APointDataFunction::Point* pPoint    = AGameStatus::GetSingleton()->GetPointData().GetPoint( 14 );
	TFLOAT                     fTime     = pAnim->GetTotalTime();
	TFLOAT                     fDuration = pAnim->GetSequencePtr()->GetDuration();
	TFLOAT                     fStart;
	TFLOAT                     fEnd;

	const TFLOAT* pRange = ( pPoint != TNULL ) ? TREINTERPRETCAST( const TFLOAT*, pPoint->m_aUnk20 ) : TNULL;

	if ( pRange == TNULL || pRange[ 0 ] <= TMath::TFLOAT_EPSILON || pRange[ 1 ] <= TMath::TFLOAT_EPSILON || pRange[ 1 ] <= pRange[ 0 ] )
	{
		fStart = fDuration * 0.6f;
		fEnd   = fDuration * 0.8f;
	}
	else
	{
		fStart = pRange[ 0 ] * fDuration;
		fEnd   = pRange[ 1 ] * fDuration;
	}

	a_rfWeight  = 1.0f;
	TFLOAT fYaw = 0.0f;

	if ( fStart < fTime )
	{
		a_rfWeight = fEnd - fTime;

		if ( a_rfWeight < 0.0f )
			return TFALSE;

		a_rfWeight = a_rfWeight / ( fEnd - fStart );
	}

	if ( pPoint != TNULL )
	{
		TMatrix44 mPoint;
		mPoint.SetFromQuaternion( *TREINTERPRETCAST( const TQuaternion*, pPoint->m_aUnk00 ) );

		TVector4 vRight( mPoint.m_f11, 0.0f, mPoint.m_f13, mPoint.m_f14 );
		vRight.NormaliseXZ();
		fYaw = -TMath::ATan2( vRight.z, vRight.x ) + 0.2617994f;
	}

	AModelInstance2* pInst = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	TINT             iBone = pInst->FindBone( "camera_Loc" );
	pInst->GetTransform( a_rOutMatrix );

	TVector4 vForward( a_rOutMatrix.m_f31, 0.0f, a_rOutMatrix.m_f33, a_rOutMatrix.m_f34 );
	vForward.NormaliseXZ();
	TFLOAT fGateYaw = TMath::ATan2( vForward.x, vForward.z );

	TQuaternion qRotation;
	qRotation.SetFromEulerRollPitchYaw( 0.0f, fYaw - fGateYaw, 0.0f );

	TMatrix44 mRotation;
	mRotation.SetFromQuaternion( qRotation );

	TMatrix44 mBone;
	pInst->GetBoneTransformLocal( TINT16( iBone ), mBone );

	TMatrix44 mTemp = mRotation;
	mRotation.Multiply( mTemp, mBone );

	mTemp = a_rOutMatrix;
	a_rOutMatrix.Multiply( mTemp, mRotation );

	mTemp = a_rOutMatrix;
	a_rOutMatrix.Multiply( mTemp, s_mBoneToWorld );

	return TTRUE;
}

// The position of the "blobAttach_Loc" bone the blob follows during the level start
// $deBlob: FUNCTION 007893c0
TBOOL APropDoor::GetBlobAttachPosition( TVector4& a_rOutPosition )
{
	if ( m_eState != DOORSTATE_LevelStart )
		return TFALSE;

	AModelInstance2* pInst = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	TINT             iBone = pInst->FindBone( "blobAttach_Loc" );

	TMatrix44 mBone;
	pInst->GetBoneTransform( TINT16( iBone ), mBone );

	TMatrix44 mWorld;
	mWorld.Multiply( mBone, s_mBoneToWorld );
	a_rOutPosition = mWorld.AsBasisVector4( BASISVECTOR_TRANSLATION );

	return TTRUE;
}

// The time of the level start animation
// $deBlob: FUNCTION 00789450
TFLOAT APropDoor::GetLevelStartAnimTime()
{
	if ( m_eState == DOORSTATE_LevelStart )
	{
		TAnimation* pAnim = GetLevelStartAnimation();

		if ( pAnim != TNULL )
			return pAnim->GetTotalTime();
	}

	return 0.0f;
}

// The level start animation playing, if any
TAnimation* APropDoor::GetLevelStartAnimation()
{
	TSkeletonInstance* pSkel = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetSkeletonInstance();
	TAnimation*        pAnim = ( pSkel != TNULL ) ? pSkel->IsAnimating( m_iLevelStartOpenAnim ) : TNULL;

	if ( pAnim == TNULL && pSkel != TNULL )
		pAnim = pSkel->IsAnimating( m_iLevelStartRespawnAnim );

	return pAnim;
}
