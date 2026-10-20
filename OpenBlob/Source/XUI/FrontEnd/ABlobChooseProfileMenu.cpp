#include "pch.h"
#include "ABlobChooseProfileMenu.h"
#include "ABlobFrontEnd.h"
#include "AApplication.h"
#include "GameInterface/APlayerProfileManager.h"
#include "MemoryCard/SaveLoad.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIBackButton.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TWCHAR ABlobChooseProfileMenu::sm_awszGameNames[ NUM_PROFILES ][ 12 ];

// $deBlob: FUNCTION 0062d7c0
// $deBlob: FUNCTION 0062d7d0
// $deBlob: FUNCTION 0062d840
// $deBlob: FUNCTION 0062d8b0
// $deBlob: FUNCTION 0062d8c0
// $deBlob: FUNCTION 0053d290
// $deBlob: FUNCTION 00858820
TDEFINE_CLASS( ABlobChooseProfileMenu );

// $deBlob: FUNCTION 0062d8d0
ABlobChooseProfileMenu::ABlobChooseProfileMenu()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopProfileSel )
    , m_bSaving( TFALSE )
    , m_bProfileCreated( TFALSE )
{
	m_iSelectedProfile = -1;
	m_iUnk4D0          = -1;

	for ( TINT i = 0; i < NUM_PROFILES; i++ )
	{
		m_apProfileButtons[ i ] = TNULL;
		swprintf( sm_awszGameNames[ i ], 256, L"%ls %d", L"Game", i + 1 );
		swprintf( m_awszProfileNames[ i ], 256, L"%ls_%d", L"Profile", i + 1 );
	}

	TXUIScene* pScene = TXUI::GetSingleton()->GetActiveScene();

	if ( pScene != TNULL )
	{
		TASSERT( pScene->IsA( &TGetClass( ABlobFrontEnd ) ) );
		TSTATICCAST( ABlobFrontEnd, pScene )->SetProfileName( L"" );
	}
}

// $deBlob: FUNCTION 0062e180
// $deBlob: FUNCTION 0062e1a0
// $deBlob: FUNCTION 0062e192
ABlobChooseProfileMenu::~ABlobChooseProfileMenu()
{
}

// $deBlob: FUNCTION 0062da30
void ABlobChooseProfileMenu::TransTo()
{
	UpdateProfileButtons();

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransTitleProfileSel, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
	pFrontEnd->SlideLeft();

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 0062dab0
void ABlobChooseProfileMenu::TransFrom()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );

	if ( m_iTransitionIndex == 1 )
		pFrontEnd->SlideLeft();

	ABlobMenu::TransFrom();
}

// $deBlob: FUNCTION 0062dad0
void ABlobChooseProfileMenu::TransBackTo()
{
	TXUIScene* pScene = TXUI::GetSingleton()->GetActiveScene();
	TASSERT( pScene->IsA( &TGetClass( ABlobFrontEnd ) ) );

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, pScene );
	pFrontEnd->SetProfileName( L"" );

	if ( m_iTransitionIndex == 1 )
	{
		TINT iMovie = pFrontEnd->GetRandomBackgroundMovie( ABlobFrontEnd::BGMOVIE_BackGen01 );

		TASSERT( -1 != m_iBackgroundMovie );
		pFrontEnd->PlayBackgroundMovie( iMovie, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
	}

	UpdateProfileButtons();
	TXUIScene::TransBackTo();
}

// $deBlob: FUNCTION 0062dbb0
void ABlobChooseProfileMenu::EndTransBackTo()
{
	APlayerProfileManager::GetSingleton()->SetCurrentProfile( -1 );
	ABlobMenu::EndTransBackTo();
}

// $deBlob: FUNCTION 0062dbd0
TBOOL ABlobChooseProfileMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	for ( TINT i = 0; i < NUM_PROFILES; i++ )
	{
		if ( m_apProfileButtons[ i ] != a_pSource )
			continue;

		APlayerProfileManager::GetSingleton()->SetCurrentProfile( i );
		g_oTheApp.SetCurrentSaveSlot( i );

		if ( APlayerProfileManager::GetSingleton()->IsProfileUsed( i ) )
		{
			TXUIScene* pScene = TXUI::GetSingleton()->GetActiveScene();
			TASSERT( pScene->IsA( &TGetClass( ABlobFrontEnd ) ) );
			TSTATICCAST( ABlobFrontEnd, pScene )->SetProfileName( APlayerProfileManager::GetSingleton()->GetCurrentProfileName() );

			TXUIScene* pMainMenu = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "mainmenu.xur" ), TUINT32( 0 ) );

			ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
			pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransEnterNameMainMenu, ABlobFrontEnd::BGMOVIE_LoopMainMenu, TFALSE, TFALSE, TFALSE );
			m_bSaving          = TFALSE;
			m_bProfileCreated  = TFALSE;
			m_iSelectedProfile = -1;

			TXUIScene* pParent = FindParentScene();

			if ( pParent != TNULL )
			{
				pMainMenu->RemoveSelf();
				pParent->AddChildTail( pMainMenu );
			}

			m_iTransitionIndex = 1;
			TXUI::GetSingleton()->TransitionTo( this, pMainMenu );
			return TTRUE;
		}

		CreateProfile( i );
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0062dd40
TBOOL ABlobChooseProfileMenu::HandleButton( TINT a_eButton, TINT a_iController )
{
	TXUI* pXUI = TXUI::GetSingleton();

	if ( pXUI->GetTransToScene() != TNULL || pXUI->GetTransBackFromScene() != TNULL || pXUI->IsInputLocked() )
		return TFALSE;

	if ( a_eButton == XUIButton_Back )
	{
		pXUI->TransitionBack( this );
		return TTRUE;
	}

	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// $deBlob: FUNCTION 0062dd90
void ABlobChooseProfileMenu::OnConfirmationClosed( TBOOL a_bYes )
{
	if ( a_bYes && m_iUnk4D0 == 0 )
		g_oTheApp.Destroy();
}

// $deBlob: FUNCTION 0062ddb0
void ABlobChooseProfileMenu::UpdateProfileButtons()
{
	for ( TINT i = 0; i < NUM_PROFILES; i++ )
	{
		TWCHAR* wszName = TStringManager::GetTempString16();
		TStringManager::String16Format( wszName, L"Profile%d", i + 1 );
		m_apProfileButtons[ i ] = FindChildXUIControl( wszName );

		if ( APlayerProfileManager::GetSingleton()->IsProfileUsed( i ) )
			m_apProfileButtons[ i ]->SetText( sm_awszGameNames[ i ], -1 );
		else
			m_apProfileButtons[ i ]->SetText( T2Locale::GetSingleton()->GetString( 469 ), -1 );
	}
}

// $deBlob: FUNCTION 0062de60
void ABlobChooseProfileMenu::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bSaving )
	{
		SaveLoad::GetInstance()->Update( a_fDeltaTime );

		if ( SaveLoad::GetInstance()->GetError() != SaveLoad::ERROR_NONE )
			OnProfileSaved( SaveLoad::GetInstance()->GetError() );
	}

	if ( m_bProfileCreated && TXUI::GetSingleton()->GetTransToScene() == TNULL && TXUI::GetSingleton()->GetTransBackFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() && m_iSelectedProfile >= 0 )
		OnNotifyPress( m_apProfileButtons[ m_iSelectedProfile ], 0 );

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0062e040
void ABlobChooseProfileMenu::CreateProfile( TINT a_iProfile )
{
	if ( m_bSaving )
		return;

	m_iSelectedProfile = a_iProfile;

	EnSaveData::SaveGameSlot* pSlot = &TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData )->GetSlot( g_oTheApp.m_iCurrentSaveSlot );
	TASSERT( TFALSE == pSlot->m_bUsedSlot );

	TUtil::MemCopy( &m_oSlotBackup, pSlot, sizeof( EnSaveData::SaveGameSlot ) );
	pSlot->m_oData.Reset();
	pSlot->m_bUsedSlot = TTRUE;
	TStringManager::String16Copy( pSlot->m_oData.m_wszName, m_awszProfileNames[ m_iSelectedProfile ], 12 );

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

	OnProfileSaved( SaveLoad::ERROR_OK );
}

// $deBlob: FUNCTION 0062df10
void ABlobChooseProfileMenu::OnProfileSaved( TINT a_iResult )
{
	TASSERT( TTRUE == m_bSaving );
	m_bSaving = TFALSE;

	if ( a_iResult == SaveLoad::ERROR_OK )
	{
		TSTATICCAST( ABlobFrontEnd, TXUI::GetSingleton()->GetActiveScene() )->SetProfileName( m_awszProfileNames[ m_iSelectedProfile ] );
		APlayerProfileManager::GetSingleton()->SetCurrentProfileName( m_awszProfileNames[ m_iSelectedProfile ] );
		m_bProfileCreated = TTRUE;
		return;
	}

	if ( m_pBackButton != TNULL )
		m_pBackButton->SetEnabled( TTRUE );

	// The save failed: the slot is restored
	EnSaveData::SaveGameSlot* pSlot = &TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData )->GetSlot( g_oTheApp.m_iCurrentSaveSlot );
	TUtil::MemCopy( pSlot, &m_oSlotBackup, sizeof( EnSaveData::SaveGameSlot ) );
}
