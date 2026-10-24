#include "pch.h"
#include "ABlobDeleteProfileMenu.h"
#include "ABlobChooseProfileMenu.h"
#include "XUI/ABlobConfirmationMenu.h"
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

// $deBlob: FUNCTION 0062fdb0
// $deBlob: FUNCTION 0062fdc0
// $deBlob: FUNCTION 0062fe30
// $deBlob: FUNCTION 0062fea0
// $deBlob: FUNCTION 0062feb0
// $deBlob: FUNCTION 0053d3d0
// $deBlob: FUNCTION 00858870
TDEFINE_CLASS( ABlobDeleteProfileMenu );

// $deBlob: FUNCTION 0062fec0
ABlobDeleteProfileMenu::ABlobDeleteProfileMenu()
    : ABlobMenu( -1 )
    , m_iSelectedProfile( -1 )
{
	m_bSaving = TFALSE;

	for ( TINT i = 0; i < NUM_PROFILES; i++ )
		m_apProfileButtons[ i ] = TNULL;
}

// $deBlob: FUNCTION 006303b0
// $deBlob: FUNCTION 006303d0
// $deBlob: FUNCTION 006303c2
ABlobDeleteProfileMenu::~ABlobDeleteProfileMenu()
{
}

// $deBlob: FUNCTION 0062ff70
void ABlobDeleteProfileMenu::TransTo()
{
	UpdateProfileButtons();
	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 0062ff80
void ABlobDeleteProfileMenu::TransBackTo()
{
	UpdateProfileButtons();
	TXUIScene::TransBackTo();
}

// $deBlob: FUNCTION 0062ff90
TBOOL ABlobDeleteProfileMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( m_iSelectedProfile != -1 || m_bSaving )
		return TTRUE;

	TINT iProfile = 0;

	while ( m_apProfileButtons[ iProfile ] != a_pSource || !APlayerProfileManager::GetSingleton()->IsProfileUsed( iProfile ) )
	{
		if ( ++iProfile >= NUM_PROFILES )
			return TFALSE;
	}

	m_iSelectedProfile = iProfile;

	// Delete the profile?
	TXUI*                  pXUI          = TXUI::GetSingleton();
	ABlobConfirmationMenu* pConfirmation = TSTATICCAST( ABlobConfirmationMenu, pXUI->CreateScene( pXUI->FindResource( "confirmation.xur" ), L"Confirmation", TFALSE ) );

	TXUIScene* pParent = FindParentScene();

	if ( pParent != TNULL )
	{
		pConfirmation->RemoveSelf();
		pParent->AddChildTail( pConfirmation );
	}

	pConfirmation->SetParentMenu( this );
	pConfirmation->SetTitle( T2Locale::GetSingleton()->GetString( 870 ) );
	pConfirmation->SetText( T2Locale::GetSingleton()->GetString( 301 ) );
	pConfirmation->HideElement1();

	TXUI::GetSingleton()->TransitionTo( this, pConfirmation );
	return TTRUE;
}

// $deBlob: FUNCTION 006300b0
void ABlobDeleteProfileMenu::OnConfirmationClosed( TBOOL a_bYes )
{
	if ( a_bYes && m_iSelectedProfile != -1 )
	{
		APlayerProfileManager::GetSingleton()->SetProfileUnused( m_iSelectedProfile );

		EnSaveData::SaveGameSlot* pSlot = &TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData )->GetSlot( m_iSelectedProfile );
		pSlot->m_bUsedSlot              = TFALSE;

		if ( SaveLoad::GetInstance()->IsEnabled() )
		{
			// NOTE: the backup is taken after the slot was marked unused (see OnProfileSaved)
			TUtil::MemCopy( &m_oSlotBackup, pSlot, sizeof( EnSaveData::SaveGameSlot ) );
			SaveLoad::GetInstance()->SetCaller( this );
			SaveLoad::GetInstance()->SaveProfile( 0, 0 );
			m_bSaving = TTRUE;

			// The save finishes in Tick (OnProfileSaved)
			if ( m_pBackButton != TNULL )
				m_pBackButton->SetEnabled( TFALSE );

			return;
		}

		UpdateProfileButtons();
	}

	m_iSelectedProfile = -1;
}

// $deBlob: FUNCTION 00630170
void ABlobDeleteProfileMenu::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bSaving )
	{
		SaveLoad::GetInstance()->Update( a_fDeltaTime );


		if ( SaveLoad::GetInstance()->GetError() != SaveLoad::ERROR_NONE && TXUI::GetSingleton()->GetTransToScene() == TNULL && TXUI::GetSingleton()->GetTransBackFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() )
			OnProfileSaved( SaveLoad::GetInstance()->GetError() );
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006301f0
void ABlobDeleteProfileMenu::UpdateProfileButtons()
{
	for ( TINT i = 0; i < NUM_PROFILES; i++ )
	{
		TWCHAR* wszName = TStringManager::GetTempString16();
		TStringManager::String16Format( wszName, L"Profile%d", i + 1 );
		m_apProfileButtons[ i ] = FindChildXUIControl( wszName );

		if ( APlayerProfileManager::GetSingleton()->IsProfileUsed( i ) )
			m_apProfileButtons[ i ]->SetText( ABlobChooseProfileMenu::sm_awszGameNames[ i ], -1 );
		else
			m_apProfileButtons[ i ]->SetText( T2Locale::GetSingleton()->GetString( 475 ), -1 );
	}
}

// $deBlob: FUNCTION 006302a0
void ABlobDeleteProfileMenu::OnProfileSaved( TINT a_iResult )
{
	TASSERT( TTRUE == m_bSaving );
	m_bSaving = TFALSE;

	if ( m_pBackButton != TNULL )
		m_pBackButton->SetEnabled( TTRUE );

	if ( a_iResult != SaveLoad::ERROR_OK )
	{
		// NOTE: retail restores the backup taken after the slot was marked unused, so the slot stays unused in the save
		// data while its profile is marked used again
		EnSaveData::SaveGameSlot* pSlot = &TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData )->GetSlot( m_iSelectedProfile );
		TUtil::MemCopy( pSlot, &m_oSlotBackup, sizeof( EnSaveData::SaveGameSlot ) );
		APlayerProfileManager::GetSingleton()->GetProfile( m_iSelectedProfile )->m_bUsed = TTRUE;
	}

	UpdateProfileButtons();
	m_iSelectedProfile = -1;
}
