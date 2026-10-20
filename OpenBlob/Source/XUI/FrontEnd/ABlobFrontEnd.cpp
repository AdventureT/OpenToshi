#include "pch.h"
#include "ABlobFrontEnd.h"
#include "XUI/ABlobCursorTrail.h"
#include "Cheats/ACheatManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/AGUIMusicController.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundLoadHelper.h"
#include "ARandom.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIVideo.h>
#include <XUI/TXUIMovie.h>
#include "Movie/AXUIBinkMovie_DX11.h"
#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 006308e0
// $deBlob: FUNCTION 006308f0
// $deBlob: FUNCTION 00630960
// $deBlob: FUNCTION 006309d0
// $deBlob: FUNCTION 006309e0
// $deBlob: FUNCTION 0053d450
// $deBlob: FUNCTION 00858890
TDEFINE_CLASS( ABlobFrontEnd );

const TWCHAR* ABlobFrontEnd::sm_awszBackgroundMovies[ NUM_BACKGROUND_MOVIES ] = {
	L"CUT_TITLE.bik",
	L"LOOP_TITLE.bik",
	L"TRANS_TITLE_PROFILEsel.bik",
	L"LOOP_PROFILEsel.bik",
	L"TRANS_PROFILEsel_ENTERNAME.bik",
	L"LOOP_ENTERNAME.bik",
	L"TRANS_ENTERNAME_MAINMENU.bik",
	L"CUT_MAINMENU.bik",
	L"LOOP_MAINMENU.bik",
	L"BACK_OPTIONS_MAINMENU.bik",
	L"BACK_MPplayer_MAINMENU.bik",
	L"BACK_MAP_MAINMENU_01.bik",
	L"BACK_MAP_MAINMENU_02.bik",
	L"BACK_MAP_MAINMENU_03.bik",
	L"BACK_MAP_MAINMENU_04.bik",
	L"BACK_MAP_MAINMENU_05.bik",
	L"BACK_MAP_MAINMENU_06.bik",
	L"BACK_MAP_MAINMENU_07.bik",
	L"BACK_MAP_MAINMENU_08.bik",
	L"BACK_MAP_MAINMENU_09.bik",
	L"BACK_MAP_MAINMENU_10.bik",
	L"BACK_EXTRAS_MAINMENU.bik",
	L"TRANS_MAINMENU_OPTIONS.bik",
	L"LOOP_OPTIONS.bik",
	L"TRANS_MAINMENU_EXTRASa.bik",
	L"LOOP_EXTRASa.bik",
	L"TRANS_EXTRASa_EXTRASb.bik",
	L"LOOP_EXTRASb.bik",
	L"TRANS_MAINMENU_MAP_01.bik",
	L"TRANS_MAINMENU_MAP_02.bik",
	L"TRANS_MAINMENU_MAP_03.bik",
	L"TRANS_MAINMENU_MAP_04.bik",
	L"TRANS_MAINMENU_MAP_05.bik",
	L"TRANS_MAINMENU_MAP_06.bik",
	L"TRANS_MAINMENU_MAP_07.bik",
	L"TRANS_MAINMENU_MAP_08.bik",
	L"TRANS_MAINMENU_MAP_09.bik",
	L"TRANS_MAINMENU_MAP_10.bik",
	L"LOOP_MAP_01.bik",
	L"LOOP_MAP_02.bik",
	L"LOOP_MAP_03.bik",
	L"LOOP_MAP_04.bik",
	L"LOOP_MAP_05.bik",
	L"LOOP_MAP_06.bik",
	L"LOOP_MAP_07.bik",
	L"LOOP_MAP_08.bik",
	L"LOOP_MAP_09.bik",
	L"LOOP_MAP_10.bik",
	L"TRANS_MAP_MUSICsel.bik",
	L"LOOP_MUSICsel.bik",
	L"BACK_MUSICsel_MAP.bik",
	L"TRANS_MAINMENU_MPplayer.bik",
	L"LOOP_MPplayer.bik",
	L"TRANS_MPplayer_MPcolour.bik",
	L"LOOP_MPcolour.bik",
	L"TRANS_MPcolour_MPgametype.bik",
	L"LOOP_MPgametype.bik",
	L"TRANS_MPgametype_MPlevelsel.bik",
	L"LOOP_MPlevelsel.bik",
	L"TRANS_MusicMixer.bik",
	L"LOOP_MusicMixer.bik",
	L"BACK_Gen01.bik",
	L"BACK_Gen02.bik",
	L"BACK_Gen03.bik",
	L"BACK_Gen04.bik",
	L"BACK_Gen05.bik",
};

// $deBlob: FUNCTION 006309f0
ABlobFrontEnd::ABlobFrontEnd()
{
	m_pProfileName     = TNULL;
	m_pBackgroundVideo = TNULL;
	m_uiUnk138         = 0;
	m_pTRB             = TNULL;
	m_uiUnk140         = 0;
	m_bUnk144          = TFALSE;
	m_uiUnk158         = 0;
	m_iBackgroundMovie = -1;
	m_fInputLockTime   = 0.0f;

	for ( TINT i = 0; i < NUM_CURSOR_TRAILS; i++ )
		m_apCursorTrails[ i ] = TNULL;
}

// $deBlob: FUNCTION 00630a90
// $deBlob: FUNCTION 00631070
// $deBlob: FUNCTION 00631060
ABlobFrontEnd::~ABlobFrontEnd()
{
	if ( AMusicManager::GetSingleton() != TNULL )
		AMusicManager::GetSingleton()->UnloadMusicData();

	m_pBackgroundVideo->GetMovie()->Close();
	TSTATICCAST( AXUIBinkMovie, m_pBackgroundVideo->GetMovie() )->OnFrontEndDestroyed();

	if ( m_pTRB != TNULL )
	{
		m_pTRB->Close();
		TFree( m_pTRB );
	}

	if ( ACheatManager::GetSingleton() != TNULL )
		ACheatManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );

	if ( TSound::GetSingleton()->IsInitialised() )
	{
		ASoundInterface::UnloadEventSystem();

		if ( AMusicManager::GetSingleton() != TNULL )
			AMusicManager::GetSingleton()->GetGUIMusicController()->SetBeatScene( TNULL );
	}
}

// $deBlob: FUNCTION 00630ca0
void ABlobFrontEnd::Tick( TFLOAT a_fDeltaTime )
{
	if ( 0.0f < m_fInputLockTime )
	{
		m_fInputLockTime -= a_fDeltaTime;

		if ( m_fInputLockTime <= 0.0f )
			TXUI::GetSingleton()->SetInputLocked( TFALSE );
	}

	ACheatManager::GetSingleton()->Update( a_fDeltaTime );

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00630b70
void ABlobFrontEnd::OnCreated()
{
	// The frontend music and sounds (loaded right away)
	TASSERT( AMusicManager::IsSingletonCreated() );
	AMusicManager::GetSingleton()->LoadMusicData( "FrontEnd", TTRUE, TFALSE );
	AMusicManager::GetSingleton()->CreateMusicStream();
	AMusicManager::GetSingleton()->SetNoPercentage( TTRUE );

	ASoundLoadHelper::GetSingleton()->LoadFrontEndSounds();
	ASoundLoadHelper::GetSingleton()->SetState( ASoundLoadHelper::LOADGROUP );

	while ( ASoundLoadHelper::GetSingleton()->GetState() != ASoundLoadHelper::LOADED )
		ASoundLoadHelper::GetSingleton()->Update();

	m_pProfileName = FindChildXUIElement( 6 );

	ACheatManager::GetSingleton()->SetEnabled( TTRUE, TFALSE );

	m_pBackgroundVideo = TSTATICCAST( TXUIVideo, FindChildXUIElement( 1 ) );

	TSTATICCAST( AXUIBinkMovie, m_pBackgroundVideo->GetMovie() )->OnFrontEndCreated();

	for ( TINT i = 0; i < NUM_CURSOR_TRAILS; i++ )
	{
		m_apCursorTrails[ i ] = TSTATICCAST( ABlobCursorTrail, FindChildXUIControl( 5 - i ) );
		m_apCursorTrails[ i ]->SetIndex( i );
		m_apCursorTrails[ i ]->CreateEffects();
	}
}

// $deBlob: FUNCTION 00630d00
void ABlobFrontEnd::PlayBackgroundMovie( TINT a_iMovie, TINT a_iAudioMovie, TBOOL a_bLoop, TBOOL a_bPaused, TBOOL a_bPauseAtEnd )
{
	TXUIMovie* pMovie = m_pBackgroundVideo->GetMovie();
	pMovie->Close();

	m_pBackgroundVideo->SetLooping( a_bLoop );

	TWCHAR* wszMovie = TStringManager::GetTempString16();
	TStringManager::String16Format( wszMovie, L"data\\movies\\%ls", sm_awszBackgroundMovies[ a_iMovie ] );

	TWCHAR  wszAudio[ 256 ];
	TWCHAR* pAudio = TNULL;

	if ( a_iAudioMovie != -1 )
	{
		TStringManager::String16Format( wszAudio, L"data\\movies\\%ls", sm_awszBackgroundMovies[ a_iAudioMovie ] );
		pAudio = wszAudio;
	}

	pMovie->Open( wszMovie, pAudio, a_bPaused, 1.0f );

	m_iBackgroundMovie = a_iMovie;
	TSTATICCAST( AXUIBinkMovie, pMovie )->SetPauseAtEnd( a_bPauseAtEnd );
}

// $deBlob: FUNCTION 00630de0
void ABlobFrontEnd::StopBackgroundMovie()
{
	m_pBackgroundVideo->GetMovie()->Close();
}

// $deBlob: FUNCTION 00630df0
TBOOL ABlobFrontEnd::IsBackgroundMoviePlaying()
{
	return m_pBackgroundVideo->GetMovie()->IsClosed() == TFALSE;
}

// $deBlob: FUNCTION 00630c80
void ABlobFrontEnd::SetProfileName( const TWCHAR* a_wszName )
{
	TSTATICCAST( TXUIControl, m_pProfileName )->SetText( a_wszName, -1 );
}

// $deBlob: FUNCTION 00630f40
void ABlobFrontEnd::ResetCursorTrails()
{
	for ( TINT i = 0; i < NUM_CURSOR_TRAILS; i++ )
		m_apCursorTrails[ i ]->Reset();

	m_oTimelineControl.StopAnimation();
	m_oTimelineControl.PlayAnimation( L"Normal", L"EndNormal" );
}

// $deBlob: FUNCTION 00631040
void ABlobFrontEnd::ClampCursorTrailFades()
{
	for ( TINT i = 0; i < NUM_CURSOR_TRAILS; i++ )
		m_apCursorTrails[ i ]->ClampFades();
}

// $deBlob: FUNCTION 00630f90
void ABlobFrontEnd::SetCursorTrailsVisible( TBOOL a_bVisible )
{
	for ( TINT i = 0; i < NUM_CURSOR_TRAILS; i++ )
		m_apCursorTrails[ i ]->SetVisible( a_bVisible );
}

// $deBlob: FUNCTION 00630e40
TINT ABlobFrontEnd::GetRandomBackgroundMovie( TINT a_iMovie )
{
	if ( a_iMovie == BGMOVIE_BackGen01 )
		return ARandom::GetSingleton()->GetRandom().GetInt( 5 ) + BGMOVIE_BackGen01;

	return a_iMovie;
}

// $deBlob: FUNCTION 00630e70
void ABlobFrontEnd::SetBackgroundMovie( TINT a_iMovie )
{
	m_iBackgroundMovie = a_iMovie;
}

// $deBlob: FUNCTION 00630e80
void ABlobFrontEnd::SlideLeft()
{
	m_oTimelineControl.StopAnimation();
	m_oTimelineControl.PlayAnimation( L"SlideLeft", L"EndSlideLeft" );
}

// $deBlob: FUNCTION 00630eb0
void ABlobFrontEnd::SlideRight()
{
	m_oTimelineControl.StopAnimation();
	m_oTimelineControl.PlayAnimation( L"SlideRight", L"EndSlideRight" );
}

// $deBlob: FUNCTION 00630ee0
void ABlobFrontEnd::SlideUp()
{
	m_oTimelineControl.StopAnimation();
	m_oTimelineControl.PlayAnimation( L"SlideUp", L"EndSlideUp" );
}

// $deBlob: FUNCTION 00630f10
void ABlobFrontEnd::SlideDown()
{
	m_oTimelineControl.StopAnimation();
	m_oTimelineControl.PlayAnimation( L"SlideDown", L"EndSlideDown" );
}

// $deBlob: FUNCTION 00630fc0
void ABlobFrontEnd::HideForGallery()
{
	for ( TINT i = 0; i < NUM_CURSOR_TRAILS; i++ )
		m_apCursorTrails[ i ]->SetVisible( TFALSE );

	m_pBackgroundVideo->SetVisible( TFALSE );
	AMusicManager::GetSingleton()->PlaySnapshot( "gallery" );
}

// $deBlob: FUNCTION 00631000
void ABlobFrontEnd::ShowAfterGallery()
{
	for ( TINT i = 0; i < NUM_CURSOR_TRAILS; i++ )
		m_apCursorTrails[ i ]->SetVisible( TTRUE );

	m_pBackgroundVideo->SetVisible( TTRUE );
	AMusicManager::GetSingleton()->ReleaseSnapshot();
}
