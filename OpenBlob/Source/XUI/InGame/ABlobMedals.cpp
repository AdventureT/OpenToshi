#include "pch.h"
#include "ABlobMedals.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIInput.h>
#include <XUI/TXUIVisual.h>
#include <T2Locale/T2Locale.h>
#include <Input/TInputInterface.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>
#include <Platform/Windows/Input/TInputDeviceMouse_Win.h>
#include "GUI/AGUIManager.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerProfileManager.h"
#include "MemoryCard/EnSaveData.h"
#include "Sound/ASoundInterface.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The names of the worlds (the same table as ABlobLevelSelection's)
static const TINT s_aiWorldNames[ 10 ] = { 124, 125, 126, 127, 128, 129, 130, 131, 132, 133 };

// $deBlob: FUNCTION 00636a30
// $deBlob: FUNCTION 00636a40
// $deBlob: FUNCTION 00636b90
// $deBlob: FUNCTION 00636ba0
// $deBlob: FUNCTION 0053d690
// $deBlob: FUNCTION 00858920
TDEFINE_CLASS( ABlobMedals );

// $deBlob: FUNCTION 00636af0
// $deBlob: FUNCTION 00636bb0
ABlobMedals::ABlobMedals()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopExtrasB )
{
	m_pButton4    = TNULL;
	m_pButton3    = TNULL;
	m_pTitle      = TNULL;
	m_iTitleIndex = 0;
}

// $deBlob: FUNCTION 006375c0
// $deBlob: FUNCTION 006375a0
// $deBlob: FUNCTION 006375b2
ABlobMedals::~ABlobMedals()
{
}

// $deBlob: FUNCTION 00637060
TBOOL ABlobMedals::SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam )
{
	if ( a_eMessage == XUIMessage_NamedFrame && TStringManager::String16Compare( a_wszParam, L"Repopulate", -1 ) == 0 )
	{
		Repopulate();
		return TTRUE;
	}

	return Toshi::TXUIControl::SendMessage( a_eMessage, a_wszParam );
}

// $deBlob: FUNCTION 00636c00
void ABlobMedals::OnCreated()
{
	ABlobMenu::OnCreated();

	m_pButton4 = FindChildXUIControl( 4 );
	m_pButton3 = FindChildXUIControl( 3 );
	m_pTitle   = FindChildXUIControl( 9 );
	m_pTitle->SetText( T2Locale::GetSingleton()->GetString( s_aiWorldNames[ m_iTitleIndex ] ), -1 );

	EnSaveData::SlotData* pData = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();

	// The awards won out of all of them (10 worlds, the paint medals count once)
	m_pCount = FindChildXUIControl( 7 );
	TStringManager::String16Format( m_wszCount, 0x20, L"%d/%d", pData->m_uiNumAwards, ( AGameStatus::s_iNumAwards - 3 ) * 10 );
	m_pCount->SetText( m_wszCount, -1 );

	TWCHAR* wszBuffer = TStringManager::GetTempString16();

	for ( TINT i = 0; i < MAXACHIEVEMENTS; i++ )
	{
		TStringManager::String16Format( wszBuffer, L"Achievement%d", i + 1 );
		m_apAchievements[ i ] = FindChildXUIControl( wszBuffer );
		m_apAchievements[ i ]->GetVisual()->FindChildXUIControl( L"100percent" )->SetVisible( TFALSE );
		m_apAchievements[ i ]->GetVisual()->FindChildXUIElement( L"bgshape" )->SetHeight( 51.0f );
	}

	Repopulate();

	if ( ( pData->m_iUnlockedWorlds & 0x200 ) == 0 )
		m_pButton3->SetVisible( TFALSE );

	if ( ( pData->m_iUnlockedWorlds & 0x2 ) == 0 )
		m_pButton4->SetVisible( TFALSE );
}

// $deBlob: FUNCTION 00636d80
TBOOL ABlobMedals::OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController )
{
	if ( a_pSource == m_pButton3 )
	{
		PrevTitle();
		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	if ( a_pSource == m_pButton4 )
	{
		NextTitle();
	}

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}

// $deBlob: FUNCTION 00636dd0
TBOOL ABlobMedals::OnNotifySetFocus( Toshi::TXUIElement* a_pSource )
{
	EnSaveData::SlotData* pData = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();

	TINT iAchievement;

	for ( iAchievement = 0; m_apAchievements[ iAchievement ] != a_pSource; iAchievement++ )
	{
		if ( iAchievement + 1 >= MAXACHIEVEMENTS )
			return ABlobMenu::OnNotifySetFocus( a_pSource );
	}

	// The sound tells whether the focused award is won (the first one: any paint medal)
	TBOOL bWon = TFALSE;

	if ( iAchievement != 0 )
	{
		bWon = pData->HasMedalFlags( AGameStatus::s_aAwards[ iAchievement + 3 ].m_uiFlag, m_iTitleIndex );
	}
	else
	{
		for ( TINT i = 0; i < 4; i++ )
		{
			if ( pData->HasMedalFlags( AGameStatus::s_aAwards[ i ].m_uiFlag, m_iTitleIndex ) )
				bWon = TTRUE;
		}
	}

	if ( bWon )
		ASoundInterface::PlayEvent( ASoundMap::GROUPID( 4 ), ASoundMap::EVENTID( 77 ), FMOD_EVENT_MODE( 0 ), TTRUE );
	else
		ASoundInterface::PlayEvent( ASoundMap::GROUPID( 4 ), ASoundMap::EVENTID( 79 ), FMOD_EVENT_MODE( 0 ), TTRUE );

	return TTRUE;
}

// $deBlob: FUNCTION 00636eb0
TBOOL ABlobMedals::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_eButton == XUIButton_Left )
	{
		// Left on the left column or an arrow: the previous world
		if ( !m_pButton3->IsVisible() )
			return TXUIScene::HandleButton( a_eButton, a_iController );

		if ( !m_apAchievements[ 0 ]->IsFocused() && !m_apAchievements[ 5 ]->IsFocused() && !m_pButton4->IsFocused() && !m_pButton3->IsFocused() )
			return TXUIScene::HandleButton( XUIButton_Left, a_iController );

		PrevTitle();
		return TTRUE;
	}

	if ( a_eButton != XUIButton_Right )
		return TXUIScene::HandleButton( a_eButton, a_iController );

	// Right on the right column or an arrow: the next world
	if ( !m_pButton4->IsVisible() )
		return TXUIScene::HandleButton( a_eButton, a_iController );

	if ( !m_apAchievements[ 4 ]->IsFocused() && !m_apAchievements[ 9 ]->IsFocused() && !m_pButton4->IsFocused() && !m_pButton3->IsFocused() )
		return TXUIScene::HandleButton( XUIButton_Right, a_iController );

	NextTitle();
	return TTRUE;
}

// $deBlob: FUNCTION 006370a0
void ABlobMedals::NextTitle()
{
	if ( m_oTimelineControl.IsPlaying() )
		return;

	if ( ++m_iTitleIndex >= 10 )
		m_iTitleIndex = 0;

	PlayAnimation( L"Next", L"EndNext" );

	// NOTE: retail disables the input four times (one per controller, all the same input)
	for ( TINT i = 0; i < 4; i++ )
		TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );

	m_pButton3->SetVisible( TTRUE );
}

// $deBlob: FUNCTION 00637120
void ABlobMedals::PrevTitle()
{
	if ( m_oTimelineControl.IsPlaying() )
		return;

	if ( --m_iTitleIndex < 0 )
		m_iTitleIndex = 9;

	PlayAnimation( L"Prev", L"EndPrev" );

	// NOTE: retail disables the input four times (one per controller, all the same input)
	for ( TINT i = 0; i < 4; i++ )
		TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );

	m_pButton4->SetVisible( TTRUE );
}

// The last button XUI got came from the keyboard or a controller
static TBOOL IsLastXUIDeviceKeyboardOrController()
{
	TInputDevice* pDevice = TInputDevice::GetRegisteredDevices()[ TInputDevice::GetLastXUIDeviceIndex() ];

	// NOTE: retail tests the mouse too and doesn't use the result
	pDevice->IsA( &TGetClass( TInputDXDeviceMouse ) );
	TBOOL bKeyboard   = pDevice->IsA( &TGetClass( TInputDeviceKeyboard ) );
	TBOOL bController = pDevice->IsA( &TGetClass( TInputDeviceController ) );

	return bKeyboard || bController;
}

// $deBlob: FUNCTION 006371a0
void ABlobMedals::Repopulate()
{
	AGUIManager::GetSingleton()->SetCursorActive( TTRUE );

	m_pTitle->SetText( T2Locale::GetSingleton()->GetString( s_aiWorldNames[ m_iTitleIndex ] ), -1 );

	EnSaveData::SlotData* pData = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();

	// The arrows only lead to the unlocked worlds
	TINT iNext = m_iTitleIndex + 1;

	if ( iNext > 9 )
		iNext = 0;

	if ( ( pData->m_iUnlockedWorlds & ( 1 << iNext ) ) == 0 )
		m_pButton4->SetVisible( TFALSE );

	TINT iPrev = m_iTitleIndex - 1;

	if ( iPrev < 0 )
		iPrev = 9;

	if ( ( pData->m_iUnlockedWorlds & ( 1 << iPrev ) ) == 0 )
		m_pButton3->SetVisible( TFALSE );

	// The first achievement is the best paint medal of the world (the first 4 awards)
	TWCHAR* wszBuffer         = TStringManager::GetTempString16();
	TBOOL   bPaint            = TFALSE;
	TINT    iPaintAchievement = -1;

	for ( TINT i = 0; i < 4; i++ )
	{
		if ( pData->HasMedalFlags( AGameStatus::s_aAwards[ i ].m_uiFlag, m_iTitleIndex ) )
		{
			TStringManager::String16Format( wszBuffer, L"Medal%lsOn", AGameStatus::s_aAwards[ i ].m_wszName );
			bPaint            = TTRUE;
			iPaintAchievement = i;
			break;
		}
	}

	if ( !bPaint )
		TStringManager::String16Format( wszBuffer, L"%ls", L"MedalPaintOff" );

	m_apAchievements[ 0 ]->GetVisual()->FindChildXUIElement( L"gp" )->PlayAnimation( wszBuffer, wszBuffer );
	TXUIControl* pTitle = m_apAchievements[ 0 ]->GetVisual()->FindChildXUIControl( L"Title" );

	if ( bPaint )
	{
		TASSERT( iPaintAchievement != -1 );
		pTitle->SetText( T2Locale::GetSingleton()->GetString( AGameStatus::s_aAwards[ iPaintAchievement ].m_iString ), -1 );
	}
	else
	{
		pTitle->SetText( T2Locale::GetSingleton()->GetString( 420 ), -1 );
	}

	// The other awards, one per achievement control
	for ( TINT i = 4; i < AGameStatus::s_iNumAwards; i++ )
	{
		TXUIControl* pAchievement = m_apAchievements[ i - 3 ];

		if ( TStringManager::String16Length( AGameStatus::s_aAwards[ i ].m_wszName ) > 0 )
		{
			const TWCHAR* wszFormat = pData->HasMedalFlags( AGameStatus::s_aAwards[ i ].m_uiFlag, m_iTitleIndex ) ? L"Medal%lsOn" : L"Medal%lsOff";
			TStringManager::String16Format( wszBuffer, wszFormat, AGameStatus::s_aAwards[ i ].m_wszName );

			TASSERT( i - 3 < MAXACHIEVEMENTS );
			pAchievement->GetVisual()->FindChildXUIElement( L"gp" )->PlayAnimation( wszBuffer, wszBuffer );
		}

		pAchievement->GetVisual()->FindChildXUIControl( L"Title" )->SetText( T2Locale::GetSingleton()->GetString( AGameStatus::s_aAwards[ i ].m_iString ), -1 );
	}

	// With the mouse, the focus goes back to the first achievement unless an arrow that's still shown has it
	if ( !IsLastXUIDeviceKeyboardOrController() )
	{
		if ( !m_pButton4->IsFocused() || m_pButton4->IsVisible() )
		{
			if ( !m_pButton3->IsFocused() || m_pButton3->IsVisible() )
				return;
		}
	}

	for ( TINT i = 0; i < MAXACHIEVEMENTS; i++ )
		m_apAchievements[ i ]->SetFocus( TFALSE, TFALSE, TTRUE );

	m_pButton4->SetFocus( TFALSE, TFALSE, TTRUE );
	m_pButton3->SetFocus( TFALSE, TFALSE, TTRUE );
	m_apAchievements[ 0 ]->SetFocus( TTRUE, TFALSE, TTRUE );
	m_apFocus[ 0 ] = m_apAchievements[ 0 ];
}

// $deBlob: FUNCTION 00636ff0
void ABlobMedals::TransTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransExtrasAExtrasB, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );

	TXUIScene::TransTo();
}
