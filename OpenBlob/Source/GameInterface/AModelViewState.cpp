#include "pch.h"
#include "AModelViewState.h"
#include "AAssetStreaming.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AModel2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "Cameras/ACameraManager.h"
#include "Entities/ATRBResourceFactory.h"
#include "Input/AInputManager2.h"
#include "Render/ARenderer.h"

#include <Render/TModel.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005ca710
AModelViewState::AModelViewState()
{
	m_iAnim   = -1;
	m_uiUnk2C = 0;
	m_iState  = 0;
}

// $deBlob: FUNCTION 005caa80
// $deBlob: FUNCTION 005ca740
AModelViewState::~AModelViewState()
{
}

// $deBlob: FUNCTION 005ca750
AGameState::UpdateResult AModelViewState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_iState == 0 )
	{
		// Wait for the streaming to finish before creating the model
		if ( !AAssetStreaming::GetSingleton()->HasActiveJobs() )
		{
			m_pInstance = AModelFactory::GetSingleton()->CreateInstance( "data/WorldModel.trb", TTRUE, TTRUE );
			m_iState    = 1;
		}

		return AGameState::OnUpdate( a_fDeltaTime );
	}

	if ( m_pInstance->GetModel()->IsLoaded() )
	{
		ARenderer::GetSingleton()->SetBackgroundColour( 0, 0, 0 );

		TMatrix44 mTransform = TMatrix44::IDENTITY;
		m_pInstance->SetTransform( mTransform );

		TModelInstance* pModelInstance = m_pInstance->GetModelInstance();
		pModelInstance->SetForceFade( TTRUE );
		pModelInstance->SetFade( 1.0f );

		m_pInstance->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
	}

	m_pInstance->UpdateImp( a_fDeltaTime );

	// Plays the animations one after the other
	AToshiAnimInterface2* pAnimInterface = m_pInstance->GetAnimInterface();

	if ( pAnimInterface != TNULL && pAnimInterface->GetSkeletonInstance() != TNULL && pAnimInterface->GetSkeletonInstance()->GetSkeleton()->GetSequenceCount() != 0 )
	{
		TINT16 iSequence;

		if ( m_iAnim == -1 )
		{
			m_iAnim   = 0;
			iSequence = 0;
			pAnimInterface->AddAnim( iSequence, 0.15f, 1.0f, 1.0f );
		}
		else if ( pAnimInterface->GetSkeletonInstance()->IsAnimating( m_iAnim ) == TNULL )
		{
			m_iAnim++;

			TINT16 iNumSequences = ( pAnimInterface->GetSkeletonInstance() != TNULL ) ? pAnimInterface->GetSkeletonInstance()->GetSkeleton()->GetSequenceCount() : 0;

			if ( iNumSequences <= m_iAnim )
				m_iAnim = 0;

			pAnimInterface->SetInvalidPose();
			iSequence = m_iAnim;
			pAnimInterface->AddAnim( iSequence, 0.15f, 1.0f, 1.0f );
		}
	}

	AInputManager2* pInputManager = AInputManager2::GetSingleton();

	if ( pInputManager->GetButtonDown( 1, pInputManager->GetControllerHandle( AInputManager2::INPUTDEVICE( 0 ) ) ) )
		PlayNextAnim();

	if ( pInputManager->GetButtonDown( 0, pInputManager->GetControllerHandle( AInputManager2::INPUTDEVICE( 0 ) ) ) )
		return UpdateResult_Remove;

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $deBlob: FUNCTION 005ca920
void AModelViewState::OnInsertion()
{
	new ATRBResourceFactory();

	AGameState::OnInsertion();

	ACameraManager::GetSingleton()->SetCurrentCamera( ACamera::CT_FREE, 0, TTRUE );

	TVector4 vTarget( 0.0f, 0.0f, 0.0f, 1.0f );
	TVector4 vEye( 0.0f, -4.0f, -4.0f, 1.0f );
	ACameraManager::GetSingleton()->GetCurrentCamera()->LookAtPoint( vEye, vTarget );

	AInputManager2::GetSingleton()->SetContext( AInputMap::INPUTCONTEXT_Menu );
}

// $deBlob: FUNCTION 005ca9e0
void AModelViewState::OnRemoval()
{
	AInputManager2::GetSingleton()->RemoveCurrentContext();
	AModelFactory::GetSingleton()->DestroyInstance( m_pInstance );

	if ( ATRBResourceFactory::GetSingleton() != TNULL )
		delete ATRBResourceFactory::GetSingleton();

	AGameState::OnRemoval();
}

// $deBlob: FUNCTION 005caa20
void AModelViewState::PlayNextAnim()
{
	AToshiAnimInterface2* pAnimInterface = m_pInstance->GetAnimInterface();

	if ( pAnimInterface == TNULL )
		return;

	m_iAnim++;

	TINT16 iNumSequences = ( pAnimInterface->GetSkeletonInstance() != TNULL ) ? pAnimInterface->GetSkeletonInstance()->GetSkeleton()->GetSequenceCount() : 0;

	if ( iNumSequences <= m_iAnim )
		m_iAnim = 0;

	pAnimInterface->SetInvalidPose();
	pAnimInterface->AddAnim( m_iAnim, 0.15f, 1.0f, 1.0f );
}
