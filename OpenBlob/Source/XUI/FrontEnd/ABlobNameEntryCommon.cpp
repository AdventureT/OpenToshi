#include "pch.h"
#include "ABlobNameEntryCommon.h"
#include "ABlobFrontEnd.h"
#include "AApplication.h"
#include "GameInterface/APlayerProfileManager.h"
#include "MemoryCard/SaveLoad.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIBackButton.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00638d50
ABlobNameEntryCommon::ABlobNameEntryCommon()
    : ABlobMenu( -1 )
{
	m_iUnk158         = 0;
	m_bSaving         = TFALSE;
	m_bProfileCreated = TFALSE;

	m_wszName[ m_iUnk158 ] = L'\0';
	m_iBackgroundMovie     = ABlobFrontEnd::BGMOVIE_LoopEnterName;
}

// $deBlob: FUNCTION 006393d0
// $deBlob: FUNCTION 006393be
ABlobNameEntryCommon::~ABlobNameEntryCommon()
{
}

// $deBlob: FUNCTION 00639310
void ABlobNameEntryCommon::GoToMainMenu()
{
	TASSERT( TXUI::GetSingleton()->GetActiveScene()->IsA( &TGetClass( ABlobFrontEnd ) ) );

	TXUI*      pXUI      = TXUI::GetSingleton();
	TXUIScene* pMainMenu = pXUI->CreateScene( pXUI->FindResource( "mainmenu.xur" ), TUINT32( 0 ) );
	TXUIScene* pParent   = FindParentScene();

	if ( pParent != TNULL )
		pParent->AddChildTail( pMainMenu );

	pXUI->TransitionTo( this, pMainMenu );
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransEnterNameMainMenu, ABlobFrontEnd::BGMOVIE_LoopMainMenu, TFALSE, TFALSE, TFALSE );
}

// $deBlob: FUNCTION 00638ee0
void ABlobNameEntryCommon::OnProfileSaved( TINT a_iResult )
{
	TASSERT( TTRUE == m_bSaving );
	m_bSaving = TFALSE;

	if ( a_iResult == SaveLoad::ERROR_OK )
	{
		TSTATICCAST( ABlobFrontEnd, TXUI::GetSingleton()->GetActiveScene() )->SetProfileName( m_wszName );
		APlayerProfileManager::GetSingleton()->SetCurrentProfileName( m_wszName );
		m_bProfileCreated = TTRUE;
		return;
	}

	if ( m_pBackButton != TNULL )
		m_pBackButton->SetEnabled( TTRUE );

	// The save failed: the slot is restored
	EnSaveData::SaveGameSlot* pSlot = &TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData )->GetSlot( g_oTheApp.m_iCurrentSaveSlot );
	TUtil::MemCopy( pSlot, &m_oSlotBackup, sizeof( EnSaveData::SaveGameSlot ) );
}

// $deBlob: FUNCTION 00638ff0
void ABlobNameEntryCommon::CreateProfile()
{
	if ( m_bSaving )
		return;

	EnSaveData::SaveGameSlot* pSlot = &TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData )->GetSlot( g_oTheApp.m_iCurrentSaveSlot );
	TASSERT( TFALSE == pSlot->m_bUsedSlot );

	// NOTE: retail doesn't copy the name into the slot before saving (the saved profile has no name until the next
	// save; SetCurrentProfileName sets it once the save succeeded)
	TUtil::MemCopy( &m_oSlotBackup, pSlot, sizeof( EnSaveData::SaveGameSlot ) );
	pSlot->m_oData.Reset();
	pSlot->m_bUsedSlot = TTRUE;

	SaveLoad::GetInstance()->SetCaller( this );
	m_bSaving = TTRUE;

	if ( m_pBackButton != TNULL )
		m_pBackButton->SetEnabled( TFALSE );

	// The save finishes in Tick (OnProfileSaved)
	if ( SaveLoad::GetInstance()->IsEnabled() )
	{
		SaveLoad::GetInstance()->SaveProfile( 0, 1 );
		return;
	}

	TASSERT( TTRUE == m_bSaving );
	m_bSaving = TFALSE;
	TSTATICCAST( ABlobFrontEnd, TXUI::GetSingleton()->GetActiveScene() )->SetProfileName( m_wszName );
	APlayerProfileManager::GetSingleton()->SetCurrentProfileName( m_wszName );
	m_bProfileCreated = TTRUE;
}

// $deBlob: FUNCTION 00638df0
void ABlobNameEntryCommon::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bSaving )
	{
		SaveLoad::GetInstance()->Update( a_fDeltaTime );

		if ( SaveLoad::GetInstance()->GetError() != SaveLoad::ERROR_NONE )
			OnProfileSaved( SaveLoad::GetInstance()->GetError() );
	}

	if ( m_bProfileCreated && TXUI::GetSingleton()->GetTransToScene() == TNULL && TXUI::GetSingleton()->GetTransBackFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() )
		GoToMainMenu();

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00638e80
void ABlobNameEntryCommon::EndTransBackTo()
{
	ABlobMenu::EndTransBackTo();

	if ( m_bSaving )
		OnProfileSaved( SaveLoad::GetInstance()->GetError() );
}

// $deBlob: FUNCTION 00638eb0
void ABlobNameEntryCommon::ApplyName()
{
	TSTATICCAST( ABlobFrontEnd, TXUI::GetSingleton()->GetActiveScene() )->SetProfileName( m_wszName );
	APlayerProfileManager::GetSingleton()->SetCurrentProfileName( m_wszName );
}

// $deBlob: FUNCTION 00639160
void ABlobNameEntryCommon::SetName( const TWCHAR* a_wszName )
{
	TStringManager::String16Copy( m_wszName, a_wszName, 12 );
}

// $deBlob: FUNCTION 00639180
void ABlobNameEntryCommon::AddChar( TWCHAR a_wcChar )
{
	TINT iLength = TStringManager::String16Length( m_wszName );

	if ( iLength < GetMaxNameLength() )
	{
#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail formats m_wszName into itself (overlapping buffers)
		TWCHAR wszName[ 12 ];
		TStringManager::String16Copy( wszName, m_wszName, 12 );
		TStringManager::String16Format( m_wszName, L"%ls%lc", wszName, a_wcChar );
#else
		TStringManager::String16Format( m_wszName, L"%ls%lc", m_wszName, a_wcChar );
#endif
	}
}

// $deBlob: FUNCTION 006391d0
void ABlobNameEntryCommon::RemoveChar()
{
	TINT iLength = TStringManager::String16Length( m_wszName );

	if ( iLength > 1 )
	{
		m_wszName[ iLength - 1 ] = L'\0';
		return;
	}

	TStringManager::String16Format( m_wszName, L"" );
}

// $deBlob: FUNCTION 00639210
void ABlobNameEntryCommon::ClearName()
{
	TStringManager::String16Format( m_wszName, L"" );
}

// $deBlob: FUNCTION 00639230
TBOOL ABlobNameEntryCommon::HasName()
{
	return TStringManager::String16Length( m_wszName ) != 0 ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 00639250
void ABlobNameEntryCommon::TransTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransProfileSelEnterName, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 006392c0
void ABlobNameEntryCommon::TransFrom()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->SlideLeft();
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransEnterNameMainMenu, ABlobFrontEnd::BGMOVIE_LoopMainMenu, TFALSE, TFALSE, TFALSE );

	ABlobMenu::TransFrom();
}

// $deBlob: FUNCTION 006392f0
void ABlobNameEntryCommon::TransBackFrom()
{
	TSTATICCAST( ABlobFrontEnd, FindParentScene() )->ResetCursorTrails();
	ABlobMenu::TransBackFrom();
}
