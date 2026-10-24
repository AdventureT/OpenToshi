#include "pch.h"
#include "AMovieState.h"
#include "AGameStateController.h"
#include "AApplication.h"
#include "AAssetStreaming.h"
#include "GUI/AGUIManager.h"
#include "GUI/AGUICursor.h"
#include "Input/AInputManager2.h"
#include "Locale/ALocaleManager.h"
#include "Movie/AMoviePlayer.h"
#include "Render/ARenderer.h"
#include "Sound/AMusicManager.h"
#include "Subtitles/ASubtitleManager.h"
#include "MemoryCard/EnSaveData.h"

#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const AInputMap::INPUTBUTTON AMovieState::SKIP_BUTTON = 0x23;

// NOTE: no CreateObject (the state takes its movies)
// $deBlob: FUNCTION 005caac0
// $deBlob: FUNCTION 005caad0
// $deBlob: FUNCTION 005caae0
// $deBlob: FUNCTION 005caaf0
// $deBlob: FUNCTION 005cab00
// $deBlob: FUNCTION 0053c6b0
// $deBlob: FUNCTION 008584f0
TDEFINE_CLASS_NORUNTIME( AMovieState );

TSTATICASSERT( sizeof( AMovieState ) == 0xE4 );

// $deBlob: FUNCTION 005cab10
AMovieState::AMovieState( const TCHAR* a_szMovie, TBOOL a_bLocalisedAudio, TBOOL a_bLocalisedName, AGameState* a_pNextState )
    : m_vecMovies( GetGlobalAllocator(), 1, 0 )
{
	m_bLocalisedAudio  = a_bLocalisedAudio;
	m_bLocalisedName   = a_bLocalisedName;
	m_iCurrentMovie    = 0;
	m_pNextState       = a_pNextState;
	m_eState           = STATE_WAITFORSTREAMING;
	m_pSubtitle        = TNULL;
	m_bSkipOnAnyButton = TFALSE;
	m_bRenderWorld     = TFALSE;
	m_fTime            = 0.0f;
	m_bFadingOut       = TFALSE;

	m_vecMovies.PushBack( a_szMovie );
}

// $deBlob: FUNCTION 005cac00
AMovieState::AMovieState( const TCHAR** a_ppMovies, TINT a_iNumMovies, TBOOL a_bLocalisedAudio, TBOOL a_bLocalisedName, AGameState* a_pNextState )
    : m_vecMovies( GetGlobalAllocator(), a_iNumMovies, 0 )
{
	m_bLocalisedAudio  = a_bLocalisedAudio;
	m_bLocalisedName   = a_bLocalisedName;
	m_iCurrentMovie    = 0;
	m_pNextState       = a_pNextState;
	m_eState           = STATE_WAITFORSTREAMING;
	m_pSubtitle        = TNULL;
	m_bSkipOnAnyButton = TFALSE;
	m_bRenderWorld     = TFALSE;
	m_fTime            = 0.0f;

	// NOTE: retail leaves m_bFadingOut unset here
	m_bFadingOut = TFALSE;

	for ( TINT i = 0; i < a_iNumMovies; i++ )
		m_vecMovies.PushBack( a_ppMovies[ i ] );
}

// $deBlob: FUNCTION 005cad20
// $deBlob: FUNCTION 005cb490
AMovieState::~AMovieState()
{
	m_vecMovies.Clear();
}

// $deBlob: FUNCTION 005cadb0
void AMovieState::OnInsertion()
{
	AInputManager2::GetSingleton()->SetContext( AInputMap::INPUTCONTEXT_Movie );
	m_InputHelper.AddMapping( SKIP_BUTTON, TFALSE, -1.0f );
	m_InputHelper.Update( 0.0f );
}

// $deBlob: FUNCTION 005cadf0
void AMovieState::OnActivate()
{
	AGUIManager::GetSingleton()->SetGUIMask( T2GUIElement::STATE_SUBTITLES );

	if ( AMoviePlayer::GetSingleton()->IsMoviePaused() == TTRUE )
	{
		AMoviePlayer::GetSingleton()->PauseMovie( TFALSE );

		// NOTE: retail pauses the subtitles here too (they're never unpaused, no subtitle is ever played on PC)
		ASubtitleManager::GetSingleton()->SetPaused( TTRUE );
	}

	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( !pController->IsFading( 0 ) && pController->GetFade( 0 ) == 1.0f )
		pController->FadeFromBlack( 0.5f, 0 );
}

// $deBlob: FUNCTION 005cae80
AMovieState::UpdateResult AMovieState::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_InputHelper.Update( a_fDeltaTime );
	m_fTime += a_fDeltaTime;

	switch ( m_eState )
	{
		case STATE_WAITFORSTREAMING:
			if ( !AAssetStreaming::GetSingleton()->HasActiveJobs() )
			{
				m_bRenderWorld = g_oTheApp.ShouldRenderWorld();
				g_oTheApp.SetRenderWorld( TFALSE );
				AMusicManager::GetSingleton()->PauseMusic( TTRUE, 0.0f );
				PlayMovie( 0 );
			}

			break;
		case STATE_PLAYING:
			if ( !m_bFadingOut )
			{
				if ( m_pSubtitle != TNULL )
				{
					m_pSubtitle->Update( a_fDeltaTime );
				}

				if ( ( m_InputHelper.IsJustDown( SKIP_BUTTON ) == TTRUE && 1.0f < m_fTime ) || HasMovieFinished() == TTRUE ||
				     ( m_bSkipOnAnyButton == TTRUE && AInputManager2::GetSingleton()->IsAnyButtonPressed() == TTRUE ) )
				{
					PlayNextMovie();
				}
			}
			else
			{
				AGameStateController* pController = AGameStateController::GetSingleton();

				if ( !pController->IsFading( 0 ) && pController->GetFade( 0 ) == 1.0f )
				{
					if ( !HasMovieFinished() )
						StopMovie();

					m_eState = STATE_FINISHED;
				}
			}

			break;
		case STATE_FINISHED:
			m_eState = STATE_REMOVE;
			break;
		case STATE_REMOVE:
			return UpdateResult_Remove;
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $deBlob: FUNCTION 005cb010
void AMovieState::OnDeactivate()
{
	if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() == TTRUE )
	{
		AMoviePlayer::GetSingleton()->PauseMovie( TTRUE );

		ASubtitleManager::GetSingleton()->SetPaused( TTRUE );
	}

	AGUIManager::GetSingleton()->RemoveCurrentGUIMask();
	g_oTheApp.SetRenderWorld( m_bRenderWorld );
}

// $deBlob: FUNCTION 005cb060
void AMovieState::OnRemoval()
{
	AInputManager2::GetSingleton()->RemoveCurrentContext();
	AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.0f );

	if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
		StopMovie();

	TASSERT( HasMovieFinished() );

	if ( m_pNextState != TNULL )
		AGameStateController::GetSingleton()->PushState( m_pNextState );
}

// $deBlob: FUNCTION 005cb130
TBOOL AMovieState::CheckForControllerRemoval()
{
	return AInputManager2::GetSingleton()->GetNumInGameControllers() != 0;
}

// $deBlob: FUNCTION 005cb140
void AMovieState::PlayMovie( TUINT32 a_uiFlags )
{
	ALocaleManager* pLocale = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );

	// NOTE: the result isn't used
	pLocale->GetCurSoundChannel();

	TString8 strMovie( m_vecMovies[ m_iCurrentMovie ] );

	// NOTE: PlaySubtitle always returns TNULL on PC
	m_pSubtitle = ASubtitleManager::GetSingleton()->PlaySubtitle(
	    strMovie, EnSaveData::SaveExtra::ConvertSubtitleOptionsToFlag( 0, 1 ), -1.0f, TFALSE
	);

	if ( m_bLocalisedName == TTRUE )
	{
		strMovie.Concat( "_", -1 );
		strMovie.Concat( pLocale->GetCurLocaleCode(), -1 );
	}

	TINT iSoundChannel = 0;

	if ( m_bLocalisedAudio == TTRUE )
		iSoundChannel = pLocale->GetCurSoundChannel();

	PrePlayMovie();
	AGUIManager::GetSingleton()->GetCursor()->SetVisible( TFALSE, TTRUE );

	TBOOL bPlaying = AMoviePlayer::GetSingleton()->PlayMovie( strMovie, iSoundChannel, a_uiFlags );
	m_fTime        = 0.0f;
	m_eState       = bPlaying ? STATE_PLAYING : STATE_REMOVE;
}

// $deBlob: FUNCTION 005cb2b0
void AMovieState::PlayNextMovie()
{
	m_iCurrentMovie++;

	if ( m_iCurrentMovie >= m_vecMovies.Size() )
	{
		if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
		{
			AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
			m_bFadingOut = TTRUE;
		}
		else
		{
			m_eState = STATE_REMOVE;
			AGameStateController::GetSingleton()->SetFade( 1.0f, 0 );
		}
	}
	else
	{
		if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() )
			StopMovie();

		PlayMovie( 0 );
	}
}

// $deBlob: FUNCTION 005cb330
TBOOL AMovieState::HasMovieFinished()
{
	return AMoviePlayer::GetSingleton()->IsMoviePlaying() == TFALSE;
}

// $deBlob: FUNCTION 005cb350
void AMovieState::StopMovie()
{
	AGUIManager::GetSingleton()->GetCursor()->SetVisible( TTRUE, TTRUE );
	ARenderer::GetSingleton()->SetSkipFrame( TTRUE );
	AMoviePlayer::GetSingleton()->StopMovie();

	if ( m_pSubtitle != TNULL )
	{
		ASubtitleManager::GetSingleton()->StopSubtitle( m_pSubtitle );
		m_pSubtitle = TNULL;
	}
}
