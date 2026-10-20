#include "pch.h"
#include "ABlobLevelSelection.h"
#include "Sound/AStreamingSoundPlayer.h"
#include "ABlobFrontEnd.h"
#include "ABlobTrackList.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/APlayerProfileManager.h"
#include "GameInterface/AXUIState.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIInput.h>
#include <XUI/TXUIBackButton.h>
#include <XUI/TXUIVisual.h>
#include <XUI/TXUIListItem.h>
#include <T2Locale/T2Locale.h>
#include <Input/TInputInterface.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The UIDs of the world buttons
static const TINT s_aiWorldUIDs[ ABlobLevelSelection::NUM_WORLDS ] = { 5, 4, 6, 7, 8, 13, 12, 10, 9, 11 };

// The names of the worlds and of the sprint challenges, two per world
static const TINT s_aiWorldNames[ ABlobLevelSelection::NUM_WORLDS ]    = { 124, 125, 126, 127, 128, 129, 130, 131, 132, 133 };
static const TINT s_aiSlotNames[ ABlobLevelSelection::NUM_WORLDS * 2 ] = { 134, 135, 137, 136, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153 };

// The levels of the worlds and of the sprint challenges, ALevelInformation names
static const TCHAR* s_aszWorldLevels[ ABlobLevelSelection::NUM_WORLDS ] = {
	"LastResort", "Bridge", "Slums", "Dam", "Guggentraz", "Inkydocks", "Abyss", "Ink_Harvest", "Military_02", "Spaceship"
};

static const TCHAR* s_aszSlotLevels[ ABlobLevelSelection::NUM_WORLDS * 2 ] = {
	"Blob_In_Venice_Sprint",
	"Top_Of_The_World_Sprint",
	"Ship2Ship_Sprint",
	"Cannons_Above_Sprint",
	"HotPlates_R_US_Sprint",
	"Lost_Highway_Sprint",
	"Inky_Academy_Sprint",
	"Mile_High_Fight_Club_Sprint",
	"Hells_Angels_Attack_Sprint",
	"Trainspotting_Sprint",
	"Miles_O_Silos_Sprint",
	"Forest_Sprint",
	"Billboard_Orama_Sprint",
	"Full_Metal_Inky_Sprint",
	"Stamps_Sprint",
	"Inkyball_Sprint",
	"Hill_Climb_Sprint",
	"Deep_Down_Sprint",
	"St_Peters_Square_Sprint",
	"Tower_Of_Babel_Sprint",
};

// The offset of the track description from the track list
static const TFLOAT s_fTrackDescriptionX = 51.0f;
static const TFLOAT s_fTrackDescriptionY = 12.0f;

// The "New" flag of a world (EnSaveData::SlotData::m_uiNewFlags)
static TUINT32 GetWorldNewFlag( TINT a_iWorld )
{
	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint )
		return ( a_iWorld >= 0 && a_iWorld < ABlobLevelSelection::NUM_WORLDS ) ? EnSaveData::NEWFLAG_FreePaintWorld << a_iWorld : 0;

	return ( a_iWorld >= 0 && a_iWorld < ABlobLevelSelection::NUM_WORLDS ) ? EnSaveData::NEWFLAG_World << a_iWorld : 0;
}

static TINT CountUnlockedWorlds( TUINT32 a_uiWorlds )
{
	TINT iCount = 0;

	for ( TINT i = 0; i < ABlobLevelSelection::NUM_WORLDS; i++ )
	{
		if ( a_uiWorlds & ( 1 << i ) )
			iCount++;
	}

	return iCount;
}

// $deBlob: FUNCTION 006330c0
// $deBlob: FUNCTION 006330d0
// $deBlob: FUNCTION 00633140
// $deBlob: FUNCTION 006331b0
// $deBlob: FUNCTION 006331c0
// $deBlob: FUNCTION 0053d610
// $deBlob: FUNCTION 00858900
TDEFINE_CLASS( ABlobLevelSelection );

// $deBlob: FUNCTION 006331d0
ABlobLevelSelection::ABlobLevelSelection()
    : ABlobMenu( -1 )
{
	m_uiUnk168          = 0;
	m_pWorldPanel       = TNULL;
	m_pLevel            = TNULL;
	m_pTitle            = TNULL;
	m_pLevelPanel       = TNULL;
	m_pMusicPanel       = TNULL;
	m_bWorldSelected    = TFALSE;
	m_pTime             = TNULL;
	m_pScore            = TNULL;
	m_pTrackDescription = TNULL;
	m_pTrackLeft        = TNULL;
	m_pTrackRight       = TNULL;
	m_pTrackName        = TNULL;
	m_pMedals           = TNULL;
	m_bStatsShown       = TFALSE;
	m_bEnableInput      = TFALSE;
	m_apSlots[ 1 ]      = TNULL;
	m_apSlots[ 0 ]      = TNULL;

	for ( TINT i = 0; i < NUM_WORLDS; i++ )
	{
		m_apWorlds[ i ]   = TNULL;
		m_apWorldNew[ i ] = TNULL;
	}

	m_apTrackStatic[ 1 ] = TNULL;
	m_apTrackStatic[ 0 ] = TNULL;
}

// $deBlob: FUNCTION 00635d20
// $deBlob: FUNCTION 00635d00
// $deBlob: FUNCTION 00635d12
ABlobLevelSelection::~ABlobLevelSelection()
{
}

// $deBlob: FUNCTION 00633370
void ABlobLevelSelection::OnCreated()
{
	ABlobMenu::OnCreated();

	if ( TStringManager::String16Compare( GetID(), L"MultiplayerLevelSelection" ) == 0 )
		m_iBackgroundMovie = ABlobFrontEnd::BGMOVIE_LoopMPPlayer;

	// NOTE: retail calls TStringManager::GetTempString16 here and discards the result

	for ( TINT i = 0; i < NUM_WORLDS; i++ )
		m_apWorlds[ i ] = FindChildXUIControl( s_aiWorldUIDs[ i ] );

	for ( TINT i = 0; i < NUM_WORLDS; i++ )
	{
		m_apWorldNew[ i ] = FindChildXUIControl( i + 111 );
		m_apWorlds[ i ]->SetText( T2Locale::GetSingleton()->GetString( s_aiWorldNames[ i ] ), -1 );
	}

	m_pWorldPanel = TSTATICCAST( TXUIScene, FindChildXUIControl( 15 ) );
	m_pWorldPanel->SetEnabled( TFALSE );

	m_pLevel       = FindChildXUIControl( 34 );
	m_apSlots[ 0 ] = FindChildXUIControl( 33 );
	m_apSlots[ 1 ] = FindChildXUIControl( 32 );
	m_pTitle       = FindChildXUIControl( 20 );

	m_pLevelPanel = TSTATICCAST( TXUIScene, FindChildXUIControl( 35 ) );
	m_pLevelPanel->SetAcceptsInput( TFALSE );

	m_pMusicPanel = TSTATICCAST( TXUIScene, FindChildXUIControl( 56 ) );
	m_pMusicPanel->SetAcceptsInput( TFALSE );

	m_pTrackList = TSTATICCAST( ABlobTrackList, FindChildXUIControl( 58 ) );
	m_pTrackList->SetStartLevel( TTRUE );
	m_pTrackList->SetPlaying( TFALSE );

	AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.0f );

	TXUIControl* pMedals = FindChildXUIControl( 24 );
	pMedals->SetEnabled( TFALSE );

	m_pMedals = pMedals->GetVisual()->FindChildXUIElement( L"Medals" );
	m_pMedals->PlayAnimation( L"NoMedal", L"NoMedal" );

	m_pTime = FindChildXUIControl( 23 );
	m_pTime->SetText( L"", -1 );
	m_pScore = FindChildXUIControl( 25 );
	m_pScore->SetText( L"", -1 );

	m_apSlotNew[ 0 ] = FindChildXUIControl( 107 );
	m_apSlotNew[ 0 ]->SetVisible( TFALSE );
	m_apSlotNew[ 1 ] = FindChildXUIControl( 108 );
	m_apSlotNew[ 1 ]->SetVisible( TFALSE );

	m_pTrackDescription = FindChildXUIControl( 59 );
	m_pTrackList->SetDescription( m_pTrackDescription );
	m_pTrackDescription->SetPosition( s_fTrackDescriptionX + m_pTrackList->GetPositionX(), s_fTrackDescriptionY + m_pTrackList->GetPositionY() );

	m_pTrackLeft  = FindChildXUIControl( 62 );
	m_pTrackRight = FindChildXUIControl( 61 );
	m_pTrackName  = FindChildXUIControl( 60 );
	m_pTrackName->SetText( m_pTrackList->GetFocussedItemText(), -1 );

	m_apTrackStatic[ 0 ] = FindChildXUIElement( 63 );
	m_apTrackStatic[ 1 ] = FindChildXUIElement( 85 );

	if ( m_pTrackList->IsItemUnlocked( m_pTrackList->GetFocussedItemIndex() ) )
	{
		for ( TINT i = 0; i < 2; i++ )
			m_apTrackStatic[ i ]->PlayAnimation( L"Loop", L"EndLoop" );
	}
	else
	{
		for ( TINT i = 0; i < 2; i++ )
			m_apTrackStatic[ i ]->PlayAnimation( L"StaticLoop", L"StaticLoopEnd" );
	}
}

// $deBlob: FUNCTION 00633790
TBOOL ABlobLevelSelection::OnNotifySetFocus( TXUIElement* a_pSource )
{
	TBOOL bWorld = TFALSE;

	for ( TINT i = 0; i < NUM_WORLDS && !bWorld; i++ )
	{
		if ( a_pSource == m_apWorlds[ i ] )
			bWorld = TTRUE;
	}

	if ( bWorld )
		ASoundInterface::PlayEvent( ASoundMap::GROUPID_GUI, ASoundMap::EVENTID( 0x50 ), FMOD_EVENT_DEFAULT, TTRUE );
	else
		ABlobMenu::PlayFocusSound( a_pSource );

	if ( a_pSource != m_pLevel )
	{
		if ( a_pSource == m_apSlots[ 0 ] )
		{
			ShowSlotStats( AGameStatus::GetSingleton()->GetSelectedWorld() * 2 );
			return TTRUE;
		}

		if ( a_pSource == m_apSlots[ 1 ] )
			ShowSlotStats( AGameStatus::GetSingleton()->GetSelectedWorld() * 2 + 1 );

		return TTRUE;
	}

	TINT        iWorld = AGameStatus::GetSingleton()->GetSelectedWorld();
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AXUIState ) ) );

	APlayerProfile*       pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
	EnSaveData::SlotData* pData    = pProfile->GetData();

	if ( !pData->IsLevelFlagSet( 0, iWorld ) )
	{
		m_pMedals->PlayAnimation( L"NoMedal", L"NoMedal" );

		if ( m_bStatsShown )
		{
			m_pWorldPanel->PlayAnimation( L"HideStats", L"EndHideStats" );
			m_bStatsShown = TFALSE;
		}
	}
	else
	{
		SetStatsTime( pData->GetMedalTime( iWorld ), TFALSE );
		TStringManager::String16Format( m_wszScore, 128, L"%d", pData->m_auiBestPoints[ iWorld ] );
		m_pScore->SetText( m_wszScore, -1 );

		if ( pProfile->HasGoldMedal( iWorld ) )
			m_pMedals->PlayAnimation( L"Gold", L"Gold" );
		else if ( pProfile->HasSilverMedal( iWorld ) )
			m_pMedals->PlayAnimation( L"Silver", L"Silver" );
		else if ( pProfile->HasBronzeMedal( iWorld ) )
			m_pMedals->PlayAnimation( L"Bronze", L"Bronze" );

		if ( !m_bStatsShown )
		{
			m_pWorldPanel->PlayAnimation( L"ShowStats", L"EndShowStats" );
			m_bStatsShown = TTRUE;
		}
	}

	m_pTitle->SetText( T2Locale::GetSingleton()->GetString( s_aiWorldNames[ iWorld ] ), -1 );
	return TTRUE;
}

// $deBlob: FUNCTION 00633a00
TBOOL ABlobLevelSelection::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( m_bWorldSelected && m_pMusicPanel->IsVisible() && !m_pTrackList->IsScrolling() && a_pSource != m_pBackButton )
	{
		TInputDevice* pDevice     = TInputDevice::GetRegisteredDevices()[ TInputDevice::GetLastXUIDeviceIndex() ];
		TBOOL         bKeyboard   = pDevice->GetClass()->IsA( &TGetClass( TInputDeviceKeyboard ) );
		TBOOL         bController = pDevice->GetClass()->IsA( &TGetClass( TInputDeviceController ) );

		if ( ( bKeyboard || bController ) && ( a_pSource == m_pTrackLeft || a_pSource == m_pTrackRight ) )
		{
			a_pSource = m_pTrackDescription;
			m_pTrackRight->SetFocus( TFALSE, TFALSE, TTRUE );
			m_pTrackLeft->SetFocus( TFALSE, TFALSE, TTRUE );
			m_pTrackDescription->SetFocus( TTRUE, TFALSE, TTRUE );
			SetFocus( m_pTrackDescription, a_iController );
		}
	}

	for ( TINT iWorld = 0; iWorld < NUM_WORLDS; iWorld++ )
	{
		if ( a_pSource != m_apWorlds[ iWorld ] )
			continue;

		APlayerProfile*       pProfile    = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
		EnSaveData::SlotData* pData       = pProfile->GetData();
		AGameStatus*          pGameStatus = AGameStatus::GetSingleton();
		pGameStatus->SetSelectedWorld( iWorld );

		m_pPrevFocus = GetFocus( a_iController );

		TWCHAR wszLevel[ 32 ];
		TStringManager::String16Format( wszLevel, 32, L"Level%d", iWorld + 1 );
		PlayAnimation( wszLevel, wszLevel );

		if ( pGameStatus->GetLevelMode() == AGameStatus::Mode_FreePaint )
		{
			pData->m_uiNewFlags &= ~GetWorldNewFlag( iWorld );
			ShowMusicPanel( a_iController, TTRUE );
		}
		else
		{
			pData->m_uiNewFlags &= ~GetWorldNewFlag( iWorld );

			TWCHAR wszLevelTr[ 32 ];
			TStringManager::String16Format( wszLevelTr, 32, L"Level%dTr", iWorld + 1 );

			if ( !pData->IsLevelFlagSet( 0, iWorld ) )
			{
				m_pWorldPanel->PlayAnimation( L"DisplayNoStats", L"EndDisplayNoStats" );
				m_pLevelPanel->PlayAnimation( wszLevel, wszLevel );
				m_bStatsShown = TFALSE;
			}
			else
			{
				m_pWorldPanel->PlayAnimation( L"Display", L"EndDisplay" );
				m_pLevelPanel->PlayAnimation( wszLevelTr, wszLevelTr );
				m_bStatsShown = TTRUE;
			}

			m_pTitle->SetText( L"", -1 );
			m_pTime->SetText( L"", -1 );
			m_pScore->SetText( L"", -1 );

			for ( TINT i = 0; i < 2; i++ )
			{
				TXUIControl* pSlot = m_apSlots[ i ];
				pSlot->SetText( T2Locale::GetSingleton()->GetString( s_aiSlotNames[ iWorld * 2 + i ] ), -1 );

				TINT    iSlot = iWorld * 2 + i;
				TUINT32 uiBit = 1 << iSlot;

				if ( ( pData->m_iUnlockedChallenges & uiBit ) == 0 )
				{
					pSlot->SetImagePath( L"../Images/Icon_LevelSelect_Locked.tga" );

					if ( i == 0 )
					{
						m_pLevel->SetNav( TXUIControl::Nav_Right, TNULL );
						m_pLevel->SetNavId( TXUIControl::Nav_Right, TNULL );
						m_apSlots[ 1 ]->SetNav( TXUIControl::Nav_Left, TNULL );
						m_apSlots[ 1 ]->SetNavId( TXUIControl::Nav_Left, TNULL );
					}
					else if ( i == 1 )
					{
						m_pLevel->SetNav( TXUIControl::Nav_Left, TNULL );
						m_pLevel->SetNavId( TXUIControl::Nav_Left, TNULL );
						m_apSlots[ 0 ]->SetNav( TXUIControl::Nav_Right, TNULL );
						m_apSlots[ 0 ]->SetNavId( TXUIControl::Nav_Right, TNULL );
					}
				}
				else
				{
					if ( AGameStatus::IsLandmarkSlot( iSlot ) )
						pSlot->SetImagePath( L"../Images/Icon_LevelSelect_Landmark.tga" );
					else if ( AGameStatus::IsPaintSlot( iSlot ) )
						pSlot->SetImagePath( L"../Images/Icon_LevelSelect_Paint.tga" );
					else if ( AGameStatus::IsCombatSlot( iSlot ) )
						pSlot->SetImagePath( L"../Images/Icon_LevelSelect_Combat.tga" );
					else if ( AGameStatus::IsRaceSlot( iSlot ) )
						pSlot->SetImagePath( L"../Images/Icon_LevelSelect_Race.tga" );

					if ( i == 0 )
					{
						m_pLevel->SetNav( TXUIControl::Nav_Right, m_apSlots[ 0 ] );
						m_pLevel->SetNavId( TXUIControl::Nav_Right, m_apSlots[ 0 ] ? m_apSlots[ 0 ]->GetID() : TNULL );
						m_apSlots[ 1 ]->SetNav( TXUIControl::Nav_Left, m_apSlots[ 0 ] );
						m_apSlots[ 1 ]->SetNavId( TXUIControl::Nav_Left, m_apSlots[ 0 ] ? m_apSlots[ 0 ]->GetID() : TNULL );
					}
					else if ( i == 1 )
					{
						m_pLevel->SetNav( TXUIControl::Nav_Left, m_apSlots[ 1 ] );
						m_pLevel->SetNavId( TXUIControl::Nav_Left, m_apSlots[ 1 ] ? m_apSlots[ 1 ]->GetID() : TNULL );
						m_apSlots[ 0 ]->SetNav( TXUIControl::Nav_Right, m_apSlots[ 1 ] );
						m_apSlots[ 0 ]->SetNavId( TXUIControl::Nav_Right, m_apSlots[ 1 ] ? m_apSlots[ 1 ]->GetID() : TNULL );
					}
				}

				m_apSlotNew[ i ]->SetVisible( ( pData->m_auiUnk308[ 0 ] & uiBit ) != 0 );
				pSlot->SetEnabled( ( pData->m_iUnlockedChallenges & uiBit ) != 0 );
			}

			m_apSlots[ 0 ]->SetFocus( TFALSE, TFALSE, TTRUE );
			m_apSlots[ 1 ]->SetFocus( TFALSE, TFALSE, TTRUE );

			m_pWorldPanel->SetFocus( m_pLevel, a_iController );
			m_pLevel->SetFocus( TTRUE, TFALSE, TTRUE );
			SetFocus( m_pWorldPanel, a_iController );

			for ( TXUIScene* pParent = FindParentScene(); pParent != TNULL; pParent = pParent->FindParentScene() )
				pParent->SetFocus( m_pWorldPanel, a_iController );
		}

		m_bWorldSelected = TTRUE;
		m_pWorldPanel->SetAcceptsInput( TTRUE );
		m_pWorldPanel->SetEnabled( TTRUE );

		for ( TINT i = 0; i < NUM_WORLDS; i++ )
			m_apWorldNew[ i ]->SetVisible( TFALSE );

		return TTRUE;
	}

	if ( a_pSource == m_pLevel )
	{
		ShowMusicPanel( a_iController, TFALSE );
		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	if ( a_pSource == m_apSlots[ 0 ] || a_pSource == m_apSlots[ 1 ] )
	{
		AGameStatus* pGameStatus = AGameStatus::GetSingleton();
		TINT         iSlot       = pGameStatus->GetSelectedWorld() * 2 + ( a_pSource == m_apSlots[ 0 ] ? 0 : 1 );

		APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData()->m_auiUnk308[ 0 ] &= ~( 1 << iSlot );

		pGameStatus->SetSPLevel( -1 );
		pGameStatus->SetSelectedSlot( iSlot );
		pGameStatus->SetLevelMode( AGameStatus::Mode_Story );

		AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
		TASSERT( TTRUE == pState->IsA( &TGetClass( AXUIState ) ) );

		TINT          iName   = s_aiSlotNames[ iSlot ];
		const TWCHAR* wszName = ( iName == -1 ) ? L"" : T2Locale::GetSingleton()->GetString( iName );
		TSTATICCAST( AXUIState, pState )->SetNextLevel( s_aszSlotLevels[ iSlot ], wszName, -1 );
		TSTATICCAST( AXUIState, pState )->SetNextState( AXUIState::NextState_LoadLevel );

		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	if ( a_pSource == m_pTrackLeft )
	{
		OnTrackScrolledLeft();
		return TTRUE;
	}

	if ( a_pSource == m_pTrackRight )
	{
		OnTrackScrolledRight();
		return TTRUE;
	}

	if ( a_pSource == m_pTrackDescription )
	{
		m_pTrackList->OnItemPressed( m_pTrackList->GetListItem( m_pTrackList->GetFocussedItemIndex() ) );
		return TTRUE;
	}

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}

// $deBlob: FUNCTION 00634460
void ABlobLevelSelection::TransTo()
{
	EnSaveData::SlotData* pData      = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();
	TINT                  iNumWorlds = 0;
	TUINT32               uiWorldBit = 1;

	for ( TINT i = 0; i < NUM_WORLDS; i++, uiWorldBit <<= 1 )
	{
		TXUIElement* pGrey    = m_apWorlds[ i ]->GetVisual()->FindChildXUIElement( L"Grey" );
		TXUIElement* pClr     = m_apWorlds[ i ]->GetVisual()->FindChildXUIElement( L"Clr" );
		TUINT32      uiWorlds = pData->m_iUnlockedWorlds;

		if ( uiWorlds & uiWorldBit )
			iNumWorlds++;

		TUINT32 uiUnlockBit = uiWorldBit;

		if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint )
		{
			if ( i == 9 )
			{
				// Free Paint has no tenth world
				m_apWorlds[ 9 ]->SetVisible( TFALSE );
				pGrey->SetVisible( TFALSE );
				pClr->SetVisible( TFALSE );
				m_apWorldNew[ 9 ]->SetVisible( TFALSE );
				continue;
			}

			// NOTE: a world is unlocked for Free Paint once the next one is unlocked in Story
			uiUnlockBit = uiWorldBit << 1;
		}

		m_apWorlds[ i ]->SetVisible( ( uiUnlockBit & uiWorlds ) != 0 );

		TBOOL bStats = pData->IsLevelFlagSet( 0, i );
		pGrey->SetVisible( !bStats );
		pClr->SetVisible( bStats );

		ABlobMenu::UpdateNewFlagLabel( m_apWorldNew[ i ], GetWorldNewFlag( i ) );
	}

	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint && iNumWorlds < 10 && --iNumWorlds == 0 )
		iNumWorlds = 1;

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->PlayBackgroundMovie( iNumWorlds + ABlobFrontEnd::BGMOVIE_TransMainMenuMap01 - 1, iNumWorlds + ABlobFrontEnd::BGMOVIE_LoopMap01 - 1, TFALSE, TFALSE, TFALSE );
	pFrontEnd->ResetCursorTrails();

	TXUIElement* pFreePaint = FindChildXUIElement( 1 );

	if ( pFreePaint != TNULL )
		pFreePaint->SetVisible( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint );

	TXUIControl* pFocus = m_apFocus[ 0 ];

	if ( pFocus != TNULL && pFocus != m_apWorlds[ 0 ] )
	{
		pFocus->SetFocus( TFALSE, TFALSE, TTRUE );
		m_apFocus[ 0 ] = m_apWorlds[ 0 ];
		m_apWorlds[ 0 ]->SetFocus( TTRUE, TFALSE, TTRUE );
	}

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 00634750
void ABlobLevelSelection::TransBackFrom()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->ResetCursorTrails();

	TINT iNumWorlds = CountUnlockedWorlds( APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData()->m_iUnlockedWorlds );

	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint && iNumWorlds < 10 && --iNumWorlds == 0 )
		iNumWorlds = 1;

	pFrontEnd->PlayBackgroundMovie( iNumWorlds + ABlobFrontEnd::BGMOVIE_BackMapMainMenu01 - 1, ABlobFrontEnd::BGMOVIE_LoopMainMenu, TFALSE, TFALSE, TFALSE );

	AGameStatus::GetSingleton()->SetSelectedWorld( -1 );
	AGameStatus::GetSingleton()->SetSPLevel( -1 );
	AGameStatus::GetSingleton()->SetSelectedSlot( -1 );
	AGameStatus::GetSingleton()->SetLevelMode( AGameStatus::Mode_Story );

	ABlobMenu::TransBackFrom();
}

// $deBlob: FUNCTION 00634830
void ABlobLevelSelection::EndTransTo()
{
	EnSaveData::SlotData* pData      = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();
	TINT                  iNumWorlds = 0;

	for ( TINT i = 0; i < NUM_WORLDS; i++ )
	{
		if ( pData->m_iUnlockedWorlds & ( 1 << i ) )
			iNumWorlds++;

		if ( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint || i != 9 )
			ABlobMenu::RevealNewFlagLabel( m_apWorldNew[ i ], GetWorldNewFlag( i ) );
	}

	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint && iNumWorlds < 10 && --iNumWorlds == 0 )
		iNumWorlds = 1;

	m_iBackgroundMovie = TINT8( iNumWorlds + ABlobFrontEnd::BGMOVIE_LoopMap01 - 1 );

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->SetCursorTrailsVisible( TFALSE );

	ABlobMenu::EndTransTo();
}

// $deBlob: FUNCTION 006349f0
void ABlobLevelSelection::EndTransBackFrom()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->SetCursorTrailsVisible( TTRUE );
	TXUIScene::EndTransBackFrom();
}

// $deBlob: FUNCTION 00634a10
void ABlobLevelSelection::OnMouseOverChange( TXUIElement* a_pElement, TINT a_iController )
{
	if ( m_bWorldSelected && a_pElement->IsA( &TGetClass( TXUIControl ) ) )
	{
		TXUIControl* pControl = TSTATICCAST( TXUIControl, a_pElement );

		if ( m_pMusicPanel->IsVisible() && pControl->FindParentScene() != m_pMusicPanel )
			return;

		if ( pControl->FindParentScene() != m_pWorldPanel )
			return;
	}

	TXUIScene::OnMouseOverChange( a_pElement, a_iController );
}

// $deBlob: FUNCTION 00634a80
TXUIElement* ABlobLevelSelection::RayTraceDeepest( const TVector2& a_rPoint, const TVector2& a_rScale, TINT a_iController )
{
	if ( m_bWorldSelected )
	{
		if ( m_pMusicPanel->IsVisible() )
			return m_pMusicPanel->RayTraceDeepest( a_rPoint, a_rScale, a_iController );

		return m_pWorldPanel->RayTraceDeepest( a_rPoint, a_rScale, a_iController );
	}

	return TXUIScene::RayTraceDeepest( a_rPoint, a_rScale, a_iController );
}

// $deBlob: FUNCTION 00634ac0
TBOOL ABlobLevelSelection::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( m_pWorldPanel->GetTimelineControl().IsPlaying() )
		return TFALSE;

	if ( m_bWorldSelected && a_eButton == XUIButton_Back )
	{
		if ( !m_pMusicPanel->IsVisible() )
		{
			m_pWorldPanel->SetAcceptsInput( TFALSE );
			m_pWorldPanel->SetEnabled( TFALSE );

			TXUIControl*          pFocus = GetFocus( a_iController );
			EnSaveData::SlotData* pData  = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();

			if ( !m_bStatsShown )
				m_pWorldPanel->PlayAnimation( L"HideNoStats", L"EndHideNoStats" );
			else
				m_pWorldPanel->PlayAnimation( L"Hide", L"EndHide" );

			m_bStatsShown = TFALSE;
			pFocus->SetFocus( TFALSE, TFALSE, TTRUE );

			m_pWorldPanel->SetFocus( TNULL, a_iController );
			SetFocus( m_pPrevFocus, a_iController );
			m_bWorldSelected = TFALSE;

			for ( TXUIScene* pParent = FindParentScene(); pParent != TNULL; pParent = pParent->FindParentScene() )
				pParent->SetFocus( m_pPrevFocus, a_iController );

			if ( m_pPrevFocus != TNULL )
			{
				m_pPrevFocus->SetFocus( TTRUE, TFALSE, TTRUE );
				m_pPrevFocus = TNULL;
			}

			for ( TINT i = 0; i < NUM_WORLDS; i++ )
				m_apWorldNew[ i ]->SetVisible( ( pData->m_uiNewFlags & GetWorldNewFlag( i ) ) != 0 );
		}
		else
		{
			if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint )
			{
				m_pWorldPanel->PlayAnimation( L"FromFreePaintMood", L"EndFromFreePaintMood" );
				m_pWorldPanel->SetAcceptsInput( TFALSE );
				SetFocus( m_pPrevFocus, a_iController );
				m_bWorldSelected = TFALSE;

				for ( TXUIScene* pParent = FindParentScene(); pParent != TNULL; pParent = pParent->FindParentScene() )
					pParent->SetFocus( m_pPrevFocus, a_iController );

				if ( m_pPrevFocus != TNULL )
				{
					m_pPrevFocus->SetFocus( TTRUE, TFALSE, TTRUE );
					m_pPrevFocus = TNULL;
				}
			}
			else
			{
				if ( !m_bStatsShown )
					m_pWorldPanel->PlayAnimation( L"FromMusicNoStats", L"EndFromMusicNoStats" );
				else
					m_pWorldPanel->PlayAnimation( L"FromMusic", L"EndFromMusic" );

				m_pMusicPanel->SetAcceptsInput( TFALSE );
				m_pLevel->SetFocus( TFALSE, TFALSE, TTRUE );
				SetFocus( m_pWorldPanel, a_iController );
				m_pWorldPanel->SetFocus( m_pLevel, a_iController );
				m_pLevel->SetFocus( TTRUE, TFALSE, TTRUE );
			}

			// Stops the track preview
			AStreamingSoundPlayer::GetSingleton()->StopAll();

			AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.5f );
			m_pTrackList->SetPlaying( TFALSE );
		}

		TXUI::GetSingleton()->GetInput()->SetRepeatElement( TNULL );
		return TTRUE;
	}

	if ( !m_bWorldSelected || !m_pMusicPanel->IsVisible() || m_pTrackList->IsScrolling() )
	{
		if ( !m_bWorldSelected && GetFocus( a_iController ) != TNULL )
		{
			TXUIControl* pNav;

			switch ( a_eButton )
			{
				case XUIButton_Up:
					pNav = GetFocus( a_iController )->GetNav( TXUIControl::Nav_Up );
					break;
				case XUIButton_Down:
					pNav = GetFocus( a_iController )->GetNav( TXUIControl::Nav_Down );
					break;
				case XUIButton_Left:
					pNav = GetFocus( a_iController )->GetNav( TXUIControl::Nav_Left );
					break;
				case XUIButton_Right:
					pNav = GetFocus( a_iController )->GetNav( TXUIControl::Nav_Right );
					break;
				default:
					return TXUIScene::HandleButton( a_eButton, a_iController );
			}

			// The worlds that aren't unlocked are hidden: don't navigate to them
			if ( pNav != TNULL && !pNav->IsVisible() )
				return TTRUE;
		}
	}
	else
	{
		if ( a_eButton == XUIButton_Left )
		{
			if ( !m_pTrackList->IsFirstItemFocussed() )
				ABlobMenu::PlayFocusSound( m_pTrackLeft );

			m_pTrackRight->SetFocus( TFALSE, TFALSE, TTRUE );
			m_pTrackLeft->SetFocus( TTRUE, TFALSE, TTRUE );
			m_pTrackDescription->SetFocus( TFALSE, TFALSE, TTRUE );
			OnTrackScrolledLeft();
			return TTRUE;
		}

		if ( a_eButton == XUIButton_Right )
		{
			if ( !m_pTrackList->IsLastItemFocussed() )
				ABlobMenu::PlayFocusSound( m_pTrackRight );

			m_pTrackRight->SetFocus( TTRUE, TFALSE, TTRUE );
			m_pTrackLeft->SetFocus( TFALSE, TFALSE, TTRUE );
			m_pTrackDescription->SetFocus( TFALSE, TFALSE, TTRUE );
			OnTrackScrolledRight();
			return TTRUE;
		}

		if ( a_eButton == XUIButton_Up )
		{
			m_pTrackRight->SetFocus( TFALSE, TFALSE, TTRUE );
			m_pTrackLeft->SetFocus( TFALSE, TFALSE, TTRUE );
			m_pTrackDescription->SetFocus( TTRUE, TFALSE, TTRUE );
			return TXUIScene::HandleButton( XUIButton_Up, a_iController );
		}
	}

	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// $deBlob: FUNCTION 006350c0
TBOOL ABlobLevelSelection::SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam )
{
	if ( a_eMessage == XUIMessage_NamedFrame && a_wszParam != TNULL && TStringManager::String16Compare( a_wszParam, L"Reveal" ) == 0 )
	{
		EnSaveData::SlotData* pData = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();
		TINT                  iSlot = AGameStatus::GetSingleton()->GetSelectedWorld() * 2;

		for ( TINT i = 0; i < 2; i++ )
		{
			TUINT32 uiBit = 1 << ( iSlot + i );

			if ( pData->m_auiUnk308[ 1 ] & uiBit )
			{
				pData->m_auiUnk308[ 1 ] &= ~uiBit;
				m_apSlotNew[ i ]->GetVisual()->FindChildXUIElement( L"Reveal" )->PlayAnimation( L"Reveal", L"EndReveal" );
			}
		}
	}

	return TXUIControl::SendMessage( a_eMessage, a_wszParam );
}

// $deBlob: FUNCTION 006351c0
void ABlobLevelSelection::OnTrackScrolledLeft()
{
	TBOOL bScrolled = m_pTrackList->ScrollLeft();
	( bScrolled ? m_pTrackRight : m_pTrackLeft )->SetVisible( bScrolled );
	m_pTrackName->SetText( m_pTrackList->GetFocussedItemText(), -1 );

	if ( !m_pTrackList->IsItemUnlocked( m_pTrackList->GetFocussedItemIndex() ) )
	{
		m_pTrackList->SetPreviewTrack( "Track_Locked_static.wav" );

		for ( TINT i = 0; i < 2; i++ )
			m_apTrackStatic[ i ]->PlayAnimation( L"StaticLoop", L"StaticLoopEnd" );

		return;
	}

	for ( TINT i = 0; i < 2; i++ )
		m_apTrackStatic[ i ]->PlayAnimation( L"Loop", L"EndLoop" );

	if ( m_pTrackList->IsLastItemFocussed() )
	{
		m_pTrackList->CancelPreviewTrack();

		// Stops the preview
		AStreamingSoundPlayer::GetSingleton()->FadeOut();
		return;
	}

	TCHAR szTrack[ 128 ];
	T2String8::Format( szTrack, "%s.wav", m_pTrackList->GetTrack( m_pTrackList->GetFocussedItemIndex() ) );

	// Previews the track unless it's playing
	if ( !AStreamingSoundPlayer::GetSingleton()->IsPlaying( szTrack ) )
		m_pTrackList->SetPreviewTrack( szTrack );
}

// $deBlob: FUNCTION 00635350
void ABlobLevelSelection::OnTrackScrolledRight()
{
	TBOOL bScrolled = m_pTrackList->ScrollRight();
	( bScrolled ? m_pTrackLeft : m_pTrackRight )->SetVisible( bScrolled );
	m_pTrackName->SetText( m_pTrackList->GetFocussedItemText(), -1 );

	if ( !m_pTrackList->IsItemUnlocked( m_pTrackList->GetFocussedItemIndex() ) )
	{
		m_pTrackList->SetPreviewTrack( "Track_Locked_static.wav" );

		for ( TINT i = 0; i < 2; i++ )
			m_apTrackStatic[ i ]->PlayAnimation( L"StaticLoop", L"StaticLoopEnd" );

		return;
	}

	for ( TINT i = 0; i < 2; i++ )
		m_apTrackStatic[ i ]->PlayAnimation( L"Loop", L"EndLoop" );

	if ( m_pTrackList->IsLastItemFocussed() )
	{
		m_pTrackList->CancelPreviewTrack();

		// Stops the preview
		AStreamingSoundPlayer::GetSingleton()->FadeOut();
		return;
	}

	TCHAR szTrack[ 128 ];
	T2String8::Format( szTrack, "%s.wav", m_pTrackList->GetTrack( m_pTrackList->GetFocussedItemIndex() ) );

	// Previews the track unless it's playing
	if ( !AStreamingSoundPlayer::GetSingleton()->IsPlaying( szTrack ) )
		m_pTrackList->SetPreviewTrack( szTrack );
}

// $deBlob: FUNCTION 006354e0
void ABlobLevelSelection::Tick( TFLOAT a_fDeltaTime )
{

	if ( m_bEnableInput && !AGameStateController::GetSingleton()->IsFading( 0 ) && AGameStateController::GetSingleton()->GetFade( 0 ) <= 0.0f )
	{
		m_bEnableInput = TFALSE;
		TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
	}

	if ( TXUI::GetSingleton()->GetTransToScene() == TNULL && TXUI::GetSingleton()->GetTransBackFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() && m_bWorldSelected )
	{
		for ( TINT i = 0; i < NUM_WORLDS; i++ )
		{
			if ( m_apWorlds[ i ] != TNULL )
				m_apWorlds[ i ]->SetFocus( TFALSE, TFALSE, TTRUE );
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006355e0
void ABlobLevelSelection::SetStatsTime( TFLOAT a_fTime, TBOOL a_bTenths )
{
	if ( a_fTime == -1.0f )
	{
		m_pTime->SetText( L"", -1 );
		return;
	}

	TFLOAT fSeconds = TFLOAT( fmod( a_fTime, 60.0 ) );
	TINT   iSeconds = TINT( fSeconds );
	TINT   iMinutes = TINT( TFLOAT( a_fTime / 60.0 ) );

	if ( iMinutes == 0 )
		TStringManager::String16Format( m_wszTime, 128, ( iSeconds < 10 ) ? L"0:0%d" : L"0:%d", iSeconds );
	else
		TStringManager::String16Format( m_wszTime, 128, ( iSeconds < 10 ) ? L"%d:0%d" : L"%d:%d", iMinutes, iSeconds );

	if ( a_bTenths )
	{
		TWCHAR wszTenths[ 32 ];
		TStringManager::String16Format( wszTenths, 32, L"%d", TINT( TFLOAT( fmod( fSeconds, 1.0 ) ) * 10.0f ) );

#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail formats m_wszTime into itself (overlapping buffers)
		TWCHAR wszTime[ 128 ];
		TStringManager::String16Copy( wszTime, m_wszTime, 128 );
		TStringManager::String16Format( m_wszTime, 128, L"%ls.%ls", wszTime, wszTenths );
#else
		TStringManager::String16Format( m_wszTime, 128, L"%ls.%ls", m_wszTime, wszTenths );
#endif
	}

	m_pTime->SetText( m_wszTime, -1 );
}

// $deBlob: FUNCTION 00635730
void ABlobLevelSelection::ShowMusicPanel( TINT a_iController, TBOOL a_bFreePaint )
{
	if ( a_bFreePaint )
		m_pWorldPanel->PlayAnimation( L"ToFreePaintMood", L"EndToFreePaintMood" );
	else if ( m_bStatsShown )
		m_pWorldPanel->PlayAnimation( L"ToMusic", L"EndToMusic" );
	else
		m_pWorldPanel->PlayAnimation( L"ToMusicNoStats", L"EndToMusicNoStats" );

	m_pMusicPanel->SetAcceptsInput( TTRUE );
	AMusicManager::GetSingleton()->PauseMusic( TTRUE, 0.5f );
	m_pTrackList->SetPlaying( TTRUE );

	SetFocus( m_pWorldPanel, a_iController );
	m_pMusicPanel->SetFocus( m_pTrackDescription, a_iController );
	m_pWorldPanel->SetFocus( m_pMusicPanel, a_iController );

	for ( TXUIScene* pParent = FindParentScene(); pParent != TNULL; pParent = pParent->FindParentScene() )
		pParent->SetFocus( m_pWorldPanel, a_iController );

	m_pTrackDescription->SetFocus( TTRUE, TFALSE, TTRUE );

	TINT iWorld = AGameStatus::GetSingleton()->GetSelectedWorld();
	AGameStatus::GetSingleton()->SetSPLevel( iWorld );

	if ( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint )
		AGameStatus::GetSingleton()->SetLevelMode( AGameStatus::Mode_Story );

	m_pTrackList->SetLevel( iWorld );

	TINT iTrack = m_pTrackList->GetFocussedItemIndex();
	m_pTrackName->SetText( m_pTrackList->GetFocussedItemText(), -1 );

	if ( !m_pTrackList->IsItemUnlocked( iTrack ) )
	{
		// Stops the preview
		AStreamingSoundPlayer::GetSingleton()->FadeOut();

		for ( TINT i = 0; i < 2; i++ )
			m_apTrackStatic[ i ]->PlayAnimation( L"StaticLoop", L"StaticLoopEnd" );
	}
	else
	{
		for ( TINT i = 0; i < 2; i++ )
			m_apTrackStatic[ i ]->PlayAnimation( L"Loop", L"EndLoop" );

		TCHAR szTrack[ 128 ];
		T2String8::Format( szTrack, "%s.wav", m_pTrackList->GetTrack( m_pTrackList->GetFocussedItemIndex() ) );

		// Plays the track's preview unless it's playing
		if ( !AStreamingSoundPlayer::GetSingleton()->IsPlaying( szTrack ) )
		{
			AStreamingSoundPlayer::GetSingleton()->Play( szTrack, 0x20 );
			m_pTrackList->SetPreviewTrack( szTrack );
		}
	}

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AXUIState ) ) );

	TINT          iName   = s_aiWorldNames[ iWorld ];
	const TWCHAR* wszName = ( iName == -1 ) ? L"" : T2Locale::GetSingleton()->GetString( iName );
	TSTATICCAST( AXUIState, pState )->SetNextLevel( s_aszWorldLevels[ iWorld ], wszName, -1 );
}

// $deBlob: FUNCTION 00635b20
void ABlobLevelSelection::ShowSlotStats( TINT a_iSlot )
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AXUIState ) ) );

	APlayerProfile*       pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
	EnSaveData::SlotData* pData    = pProfile->GetData();

	if ( !pProfile->HasSprintBronzeMedal( a_iSlot ) )
	{
		m_pTime->SetText( L"", -1 );
		m_pMedals->PlayAnimation( L"NoMedal", L"NoMedal" );

		if ( m_bStatsShown )
		{
			m_pWorldPanel->PlayAnimation( L"HideStats", L"EndHideStats" );
			m_bStatsShown = TFALSE;
		}

		m_pScore->SetText( L"", -1 );
	}
	else
	{
		SetStatsTime( pData->GetSprintMedalTime( a_iSlot ), TTRUE );
		TStringManager::String16Format( m_wszScore, L"%d/%d", pData->m_auiSprintBestCount[ a_iSlot ], pData->m_auiSprintTotal[ a_iSlot ] );
		m_pScore->SetText( m_wszScore, -1 );

		if ( !m_bStatsShown )
		{
			m_pWorldPanel->PlayAnimation( L"ShowStats", L"EndShowStats" );
			m_bStatsShown = TTRUE;
		}
	}

	if ( pProfile->HasSprintGoldMedal( a_iSlot ) )
		m_pMedals->PlayAnimation( L"Gold", L"Gold" );
	else if ( pProfile->HasSprintSilverMedal( a_iSlot ) )
		m_pMedals->PlayAnimation( L"Silver", L"Silver" );
	else if ( pProfile->HasSprintBronzeMedal( a_iSlot ) )
		m_pMedals->PlayAnimation( L"Bronze", L"Bronze" );

	m_pTitle->SetText( T2Locale::GetSingleton()->GetString( s_aiSlotNames[ a_iSlot ] ), -1 );
}
