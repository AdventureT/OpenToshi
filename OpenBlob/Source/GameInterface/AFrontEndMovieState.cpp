#include "pch.h"
#include "AFrontEndMovieState.h"
#include "AGameStateController.h"
#include "AFXViewState.h"
#include "AXUIState.h"
#include "GUI/AGUIManager.h"
#include "ASaveLoadStartAppState.h"
#include "AApplication.h"
#include "APlayerProfileManager.h"
#include "Memory/AMemory.h"
#include "Render/ARenderer.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include "Movie/AMoviePlayer.h"
#include "Entities/AEntityData.h"

#include <Toshi/TRegion.h>
#include <GUI/T2GUI.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005d4650
// $deBlob: FUNCTION 005d4660
// $deBlob: FUNCTION 005d46d0
// $deBlob: FUNCTION 005d4730
// $deBlob: FUNCTION 005d4740
// $deBlob: FUNCTION 0053c8d0
// $deBlob: FUNCTION 008585a0
TDEFINE_CLASS( AFrontEndMovieState );

const TCHAR* AFrontEndMovieState::scm_szAssetName[ 4 ] = {
	"bkg_Legal_Blob_ENG",
	"THQNORDIC_LOGO_Color_white",
	"logoMovie",
	"00_ChromaCity",
};

// $deBlob: FUNCTION 005d4750
AFrontEndMovieState::AFrontEndMovieState()
    : m_fBackgroundLeftTime( 5.0f )
    , m_eState( STATE_LEGAL )
    , m_bSkipMovie( TFALSE )
    , m_bSkipAllMovies( TFALSE )
{
	m_bEntityDataLoaded = TFALSE;
	m_eNextState        = STATE_FADETOBLACK;
	m_fFadeInTime       = 0.0f;
}

// $deBlob: FUNCTION 005d54c0
// $deBlob: FUNCTION 005d47f0
AFrontEndMovieState::~AFrontEndMovieState()
{
}

// $deBlob: FUNCTION 005d4900
AGameState::UpdateResult AFrontEndMovieState::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_InputHelper.Update( a_fDeltaTime );

	// The skip buttons work once the legal screen is over
	if ( m_eState != STATE_LEGAL )
	{
		if ( m_InputHelper.IsJustDown( 42 ) == TTRUE )
			m_bSkipMovie = TTRUE;

		if ( m_InputHelper.IsJustDown( 43 ) == TTRUE )
			m_bSkipAllMovies = TTRUE;
	}

	if ( m_bSkipAllMovies && m_eState != STATE_LEGAL && m_eState != STATE_FADEFROMBLACK && m_eState != STATE_FADETOBLACK && m_eState != STATE_RESTART )
	{
		StopMovieIfPlaying();
		return Finish();
	}

	AGameState::OnUpdate( a_fDeltaTime );

	if ( m_eState == STATE_LEGAL || ( TUINT32( m_eState - 1 ) <= 1 && AMoviePlayer::GetSingleton()->IsMoviePlaying() == TTRUE ) )
		m_fBackgroundLeftTime -= a_fDeltaTime;

	// The entity data is loaded while the legal screen shows
	if ( !m_bEntityDataLoaded && AEntityData::GetSingleton() == TNULL )
	{
		new AEntityData();
		AEntityData::GetSingleton()->Load();
		m_bEntityDataLoaded = TTRUE;
	}

	STATE eState = m_eState;

	if ( ( ( eState == STATE_THQLOGO || eState == STATE_LOGOMOVIE || eState == STATE_INTRO ) && m_bSkipMovie == TTRUE ) || m_fBackgroundLeftTime <= 0.0f )
	{
		if ( g_oTheApp.m_bFXView )
			m_bSkipAllMovies = TTRUE;

		if ( g_oTheApp.m_bSkipFrontend )
		{
			if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
				AMoviePlayer::GetSingleton()->StopMovie();

			return Finish();
		}
	}

	switch ( eState )
	{
		case STATE_LEGAL:
			if ( HasBackgroundStoppedShowing() == TTRUE )
				GoToNextMovie( STATE_THQLOGO );
			break;
		case STATE_THQLOGO:
			if ( HasMovieStopped() == TTRUE )
				GoToNextMovie( STATE_LOGOMOVIE );
			break;
		case STATE_LOGOMOVIE:
			if ( HasMovieStopped() == TTRUE )
				GoToNextMovie( STATE_INTRO );
			break;
		case STATE_INTRO:
			if ( HasMovieStopped() == TTRUE )
			{
				StopMovieIfPlaying();
				m_bSkipMovie = TFALSE;
				return Finish();
			}
			break;
		case STATE_FADETOBLACK:
		{
			AGameStateController* pController = AGameStateController::GetSingleton();

			if ( pController->GetFade( 0 ) < 1.0f && pController->IsFadingToBlack( 0 ) )
				break;

			if ( m_bSkipAllMovies )
			{
				pController->SetFade( 0.0f, 0 );
				StopMovieIfPlaying();
				return Finish();
			}

			if ( m_fFadeInTime <= 0.0f )
			{
				pController->SetFade( 0.0f, 0 );
				GoToNextBackground( m_eNextState, 0.0f, 0.0f );
			}
			else
			{
				InitState( STATE_FADEFROMBLACK );
				CreateBackGround( m_eNextState );
				pController->FadeFromBlack( m_fFadeInTime, 0 );
			}

			break;
		}
		case STATE_FADEFROMBLACK:
		{
			AGameStateController* pController = AGameStateController::GetSingleton();

			if ( pController->GetFade( 0 ) <= 0.0f || !pController->IsFading( 0 ) )
				GoToNextBackground( m_eNextState, 0.0f, 0.0f );

			break;
		}
		case STATE_RESTART:
		{
			AGameStateController* pController = AGameStateController::GetSingleton();

			if ( pController->GetFade( 0 ) >= 1.0f || !pController->IsFadingToBlack( 0 ) )
			{
				pController->ResetFades();
				StartBlobFrontendMenu();
			}

			break;
		}
	}

	return UpdateResult_OK;
}

// $deBlob: FUNCTION 005d4860
void AFrontEndMovieState::OnInsertion()
{
	AInputManager2::GetSingleton()->SetContext( AInputMap::INPUTCONTEXT_FrontEndMovie );
	m_InputHelper.AddMapping( 42, TFALSE, -1.0f );
	m_InputHelper.AddMapping( 43, TFALSE, -1.0f );

	m_Background.Create( scm_szAssetName[ m_eState ] );
	m_Background.SetVisible( TTRUE );

	AGUIManager::GetSingleton()->SetCursorEnabled( TFALSE );

	AGameState::OnInsertion();
}

// $deBlob: FUNCTION 005d48e0
void AFrontEndMovieState::OnRemoval()
{
	AInputManager2::GetSingleton()->RemoveCurrentContext();

	AGameState::OnRemoval();
}

// $deBlob: FUNCTION 005d4cf0
void AFrontEndMovieState::OnActivate()
{
	AGameState::OnActivate();

	T2GUIElement* pRoot = T2GUI::GetSingleton()->GetRootElement();
	pRoot->AddChildHead( &m_Background );

	TFLOAT fWidth, fHeight;
	pRoot->GetDimensions( fWidth, fHeight );
	m_Background.SetDimensions( fWidth, fHeight );

	g_oTheApp.SetRenderWorld( TFALSE );
}

// $deBlob: FUNCTION 005d4d60
void AFrontEndMovieState::OnDeactivate()
{
	AGameState::OnDeactivate();

	if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
		AMoviePlayer::GetSingleton()->StopMovie();

	m_Background.RemoveSelf();
	g_oTheApp.SetRenderWorld( TTRUE );
}

// $deBlob: FUNCTION 005d4f50
TBOOL AFrontEndMovieState::HasBackgroundStoppedShowing()
{
	switch ( m_eState )
	{
		case STATE_THQLOGO:
		case STATE_LOGOMOVIE:
		case STATE_INTRO:
			if ( m_bSkipMovie == TTRUE )
				return TTRUE;
	}

	return m_fBackgroundLeftTime <= 0.0f;
}

// $deBlob: FUNCTION 005d4f90
TBOOL AFrontEndMovieState::HasMovieStopped()
{
	switch ( m_eState )
	{
		case STATE_THQLOGO:
		case STATE_LOGOMOVIE:
		case STATE_INTRO:
			if ( m_bSkipMovie == TTRUE )
				return TTRUE;
	}

	return AMoviePlayer::GetSingleton()->IsMoviePlaying() == TFALSE;
}

// $deBlob: FUNCTION 005d4f30
void AFrontEndMovieState::StopMovieIfPlaying()
{
	if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
		AMoviePlayer::GetSingleton()->StopMovie();
}

// $deBlob: FUNCTION 005d4fe0
void AFrontEndMovieState::GoToNextMovie( STATE a_eState )
{
	if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
		AMoviePlayer::GetSingleton()->StopMovie();

	m_fBackgroundLeftTime = 5.0f;
	m_eState              = a_eState;
	m_bSkipMovie          = TFALSE;
	m_Background.SetVisible( TFALSE );

	AMoviePlayer::GetSingleton()->PlayMovie( scm_szAssetName[ a_eState ], 0, AMoviePlayer::PlayFlags_None );
}

// $deBlob: FUNCTION 005d5050
void AFrontEndMovieState::GoToNextBackground( STATE a_eState, TFLOAT a_fFadeOutTime, TFLOAT a_fFadeInTime )
{
	if ( 0.0f < a_fFadeOutTime )
	{
		m_eNextState  = a_eState;
		m_fFadeInTime = a_fFadeInTime;

		if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
			AMoviePlayer::GetSingleton()->StopMovie();

		m_fBackgroundLeftTime = 5.0f;
		m_eState              = STATE_FADETOBLACK;
		m_bSkipMovie          = TFALSE;
		AGameStateController::GetSingleton()->FadeToBlack( a_fFadeOutTime, 0 );
		return;
	}

	if ( 0.0f < a_fFadeInTime )
	{
		m_fFadeInTime = a_fFadeInTime;
		m_eNextState  = a_eState;

		if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
			AMoviePlayer::GetSingleton()->StopMovie();

		m_fBackgroundLeftTime = 5.0f;
		m_eState              = STATE_FADEFROMBLACK;
		m_bSkipMovie          = TFALSE;
		m_Background.Create( scm_szAssetName[ a_eState ] );
		m_Background.SetVisible( TTRUE );
		AGameStateController::GetSingleton()->FadeFromBlack( m_fFadeInTime, 0 );
		return;
	}

	if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
		AMoviePlayer::GetSingleton()->StopMovie();

	m_fBackgroundLeftTime = 5.0f;
	m_eState              = a_eState;
	m_bSkipMovie          = TFALSE;
	m_Background.Create( scm_szAssetName[ a_eState ] );
	m_Background.SetVisible( TTRUE );
}

// $deBlob: FUNCTION 005d5280
void AFrontEndMovieState::InitState( STATE a_eState )
{
	if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
		AMoviePlayer::GetSingleton()->StopMovie();

	m_fBackgroundLeftTime = 5.0f;
	m_eState              = a_eState;
	m_bSkipMovie          = TFALSE;
}

// $deBlob: FUNCTION 005d51c0
void AFrontEndMovieState::Restart()
{
	if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
		AMoviePlayer::GetSingleton()->StopMovie();

	m_fBackgroundLeftTime = 5.0f;
	m_eState              = STATE_RESTART;
	m_bSkipMovie          = TFALSE;
	AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
}

// $deBlob: FUNCTION 005d5220
void AFrontEndMovieState::CreateBackGround( STATE a_eState )
{
	m_Background.Create( scm_szAssetName[ a_eState ] );
	m_Background.SetVisible( TTRUE );
}

// Creates the frontend: frontend.xur (ABlobFrontEnd) and the splash screen (an AXUIState waiting for
// frontendcarousel.trb)
// $deBlob: FUNCTION 005d5300
AGameState::UpdateResult AFrontEndMovieState::Finish()
{
	if ( g_oTheApp.m_bFXView )
	{
		AGameStateController::GetSingleton()->ResetStack();

		AGameStateController::GetSingleton()->PushState( new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) AFXViewState() );
		return UpdateResult_OK;
	}

	if ( g_oTheApp.m_bSkipFrontend )
	{
		APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();
		pProfileManager->SetCurrentProfile( 0 );

		if ( pProfileManager->IsProfileUsed( 0 ) )
			return UpdateResult_OK;

		pProfileManager->SetCurrentProfileName( L"Archie" );
		return UpdateResult_OK;
	}

	AGameStateController::GetSingleton()->ResetStack();
	AXUIState::LoadFrontendCarousel();
	AGameStateController::GetSingleton()->SetFade( 1.0f, 0 );
	ARenderer::GetSingleton()->Update( 0.0f );
	ARenderer::GetSingleton()->Update( 0.0f );

	TXUI*      pXUI      = TXUI::GetSingleton();
	TXUIScene* pFrontEnd = pXUI->CreateScene( pXUI->FindResource( "frontend.xur" ), TUINT32( 0 ) );
	TASSERT( pFrontEnd->IsA( &TGetClass( ABlobFrontEnd ) ) );

	TSTATICCAST( ABlobFrontEnd, pFrontEnd )->SetCursorTrailsVisible( TFALSE );

	AXUIState::Assets oAssets;
	oAssets.m_sResourceName = "splash.xur";
	oAssets.m_wszSceneName  = TNULL;
	oAssets.m_fSoundVolume  = 0.5f;
	oAssets.m_pTargetParent = pFrontEnd;
	oAssets.m_bDelayed      = TTRUE;

	AGameStateController::GetSingleton()->PushState( new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) AXUIState( oAssets ) );

	return UpdateResult_OK;
}

// $deBlob: FUNCTION 005d4dc0
void AFrontEndMovieState::StartBlobFrontendMenu()
{
	switch ( TRegion_GetRegion() )
	{
		case 0:
		case 3:
		{
			AXUIState::Assets oAssets;
			oAssets.m_sResourceName = "language.xur";
			oAssets.m_wszSceneName  = TNULL;
			oAssets.m_fSoundVolume  = 0.5f;

			AGameStateController::GetSingleton()->PushState( new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) AXUIState( oAssets ) );

			AGUIManager::GetSingleton()->SetCursorVisible( TFALSE, TTRUE );
			return;
		}
		case 1:
		case 2:
			break;
		default:
			return;
	}

	AGameStateController::GetSingleton()->PushState( new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) ASaveLoadStartAppState() );
}
