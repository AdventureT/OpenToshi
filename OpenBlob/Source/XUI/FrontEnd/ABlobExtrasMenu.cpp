#include "pch.h"
#include "ABlobExtrasMenu.h"
#include "ABlobFrontEnd.h"
#include "GameInterface/APlayerProfileManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobExtrasMenu ) == 0x15C );

// $deBlob: FUNCTION 00630420
// $deBlob: FUNCTION 00630430
// $deBlob: FUNCTION 006304a0
// $deBlob: FUNCTION 00630510
// $deBlob: FUNCTION 00630520
// $deBlob: FUNCTION 0053d410
// $deBlob: FUNCTION 00858880
TDEFINE_CLASS( ABlobExtrasMenu );

// $deBlob: FUNCTION 00630530
ABlobExtrasMenu::ABlobExtrasMenu()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopExtrasA )
{
	m_pCutscenes        = TNULL;
	m_pMovies           = TNULL;
	m_pGallery          = TNULL;
	m_bReturnToMainMenu = TFALSE;
	m_pCutscenesNew     = TNULL;
	m_pMoviesNew        = TNULL;
	m_pGalleryNew       = TNULL;
}

// $deBlob: FUNCTION 00630870
// $deBlob: FUNCTION 00630882
// $deBlob: FUNCTION 00630890
ABlobExtrasMenu::~ABlobExtrasMenu()
{
}

// $deBlob: FUNCTION 00630590
void ABlobExtrasMenu::OnCreated()
{
	ABlobMenu::OnCreated();

	m_pCutscenes = FindChildXUIControl( 7 );
	m_pMovies    = FindChildXUIControl( 6 );
	m_pGallery   = FindChildXUIControl( 5 );

	// NOTE: retail gets the current profile and doesn't use it
	APlayerProfileManager::GetSingleton()->GetCurrentProfile();

	m_pCutscenesNew = FindChildXUIControl( 9 );
	m_pMoviesNew    = FindChildXUIControl( 10 );
	m_pGalleryNew   = FindChildXUIControl( 11 );

	UpdateNewFlagLabel( m_pCutscenesNew, EnSaveData::NEWFLAG_Cutscenes );
	UpdateNewFlagLabel( m_pMoviesNew, EnSaveData::NEWFLAG_Movies );
	UpdateNewFlagLabel( m_pGalleryNew, EnSaveData::NEWFLAG_Gallery );
}

// $deBlob: FUNCTION 00630630
void ABlobExtrasMenu::TransTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );

	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransMainMenuExtrasA, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
	pFrontEnd->SlideUp();

	if ( m_bReturnToMainMenu )
	{
		TXUIScene* pTrackSelection = FindFrontEndScene( L"MusicSelectMenu" );

		if ( pTrackSelection != TNULL )
		{
			TXUIScene* pMainMenu = FindFrontEndScene( L"MainMenu" );
			delete pTrackSelection;
			m_pPrevScene = pMainMenu;
		}

		m_bReturnToMainMenu = TFALSE;
	}

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 006306e0
void ABlobExtrasMenu::TransFrom()
{
	TSTATICCAST( ABlobFrontEnd, FindParentScene() )->SlideUp();
	ABlobMenu::TransFrom();
}

// $deBlob: FUNCTION 00630700
void ABlobExtrasMenu::TransBackTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TINT           iMovie    = pFrontEnd->GetRandomBackgroundMovie( ABlobFrontEnd::BGMOVIE_BackGen01 );

	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( iMovie, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
	pFrontEnd->ResetCursorTrails();

	UpdateNewFlagLabel( m_pCutscenesNew, EnSaveData::NEWFLAG_Cutscenes );
	UpdateNewFlagLabel( m_pMoviesNew, EnSaveData::NEWFLAG_Movies );
	UpdateNewFlagLabel( m_pGalleryNew, EnSaveData::NEWFLAG_Gallery );

	TXUIScene::TransBackTo();
}

// $deBlob: FUNCTION 006307a0
void ABlobExtrasMenu::TransBackFrom()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->SlideDown();
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_BackExtrasMainMenu, ABlobFrontEnd::BGMOVIE_LoopMainMenu, TFALSE, TFALSE, TFALSE );

	ABlobMenu::TransBackFrom();
}

// $deBlob: FUNCTION 006307d0
void ABlobExtrasMenu::EndTransTo()
{
	RevealNewFlagLabel( m_pCutscenesNew, EnSaveData::NEWFLAG_Cutscenes );
	RevealNewFlagLabel( m_pMoviesNew, EnSaveData::NEWFLAG_Movies );
	RevealNewFlagLabel( m_pGalleryNew, EnSaveData::NEWFLAG_Gallery );

	ABlobMenu::EndTransTo();
}

// $deBlob: FUNCTION 00630810
TBOOL ABlobExtrasMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();

	if ( a_pSource == m_pCutscenes )
		pProfile->GetData()->ClearNewFlag( EnSaveData::NEWFLAG_Cutscenes );

	if ( a_pSource == m_pMovies )
		pProfile->GetData()->ClearNewFlag( EnSaveData::NEWFLAG_Movies );

	if ( a_pSource == m_pGallery )
		pProfile->GetData()->ClearNewFlag( EnSaveData::NEWFLAG_Gallery );

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}
