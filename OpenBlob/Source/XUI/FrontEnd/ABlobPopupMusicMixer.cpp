#include "pch.h"
#include "ABlobPopupMusicMixer.h"
#include "ABlobExtrasMenu.h"
#include "ABlobTrackSelection.h"
#include "Sound/AMusicManager.h"

#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobPopupMusicMixer ) == 0x140 );

// $deBlob: FUNCTION 0063a4f0
// $deBlob: FUNCTION 0063a500
// $deBlob: FUNCTION 0063a580
// $deBlob: FUNCTION 0063a600
// $deBlob: FUNCTION 0063a610
// $deBlob: FUNCTION 0053d790
// $deBlob: FUNCTION 00858960
TDEFINE_CLASS( ABlobPopupMusicMixer );

// $deBlob: FUNCTION 0063a7b0
ABlobPopupMusicMixer::ABlobPopupMusicMixer()
    : ABlobMenu( -1 )
{
}

// $deBlob: FUNCTION 0063a7d0
// $deBlob: FUNCTION 0063a7e2
// $deBlob: FUNCTION 0063a7f0
ABlobPopupMusicMixer::~ABlobPopupMusicMixer()
{
}

// $deBlob: FUNCTION 0063a620
TBOOL ABlobPopupMusicMixer::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( TStringManager::String16CompareNoCase( L"btnContinue", a_pSource->GetID() ) == 0 )
	{
		TXUI::GetSingleton()->TransitionBack( this );
		AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.1f );
		AMusicManager::GetSingleton()->ResetPitch( 0.5f );
	}

	if ( TStringManager::String16CompareNoCase( L"btnChangeTune", a_pSource->GetID() ) == 0 )
	{
		TXUIScene* pTrackSelection = FindFrontEndScene( L"MusicSelectMenu" );
		TASSERT( pTrackSelection->IsA( &TGetClass( TXUIScene ) ) );

		TSTATICCAST( ABlobTrackSelection, pTrackSelection )->SetDestroyMixer();
		TXUI::GetSingleton()->TransitionTo( this, pTrackSelection );
		AMusicManager::GetSingleton()->ResetPitch( 0.5f );
	}

	if ( TStringManager::String16CompareNoCase( L"btnQuit", a_pSource->GetID() ) == 0 )
	{
		TXUIScene* pExtras = FindFrontEndScene( L"ExtrasScene" );
		TASSERT( pExtras->IsA( &TGetClass( TXUIScene ) ) );

		TSTATICCAST( ABlobExtrasMenu, pExtras )->SetReturnToMainMenu();
		TXUI::GetSingleton()->TransitionTo( this, pExtras );
		AMusicManager::GetSingleton()->ResetPitch( 0.5f );
		AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.5f );
	}

	// NOTE: retail doesn't call ABlobMenu::OnNotifyPress
	return TFALSE;
}
