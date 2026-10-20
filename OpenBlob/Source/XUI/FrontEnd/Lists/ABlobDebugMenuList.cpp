#include "pch.h"
#include "ABlobDebugMenuList.h"
#include "ABlobDebugLevelSelect.h"
#include "GameInterface/APlayerProfileManager.h"
#include "MemoryCard/SaveLoad.h"

#include <XUI/TXUIListItem.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL ABlobDebugMenuList::s_bMusicTester;

// $deBlob: FUNCTION 0063c6b0
// $deBlob: FUNCTION 0063c6c0
// $deBlob: FUNCTION 0063c740
// $deBlob: FUNCTION 0063c7c0
// $deBlob: FUNCTION 0063c7d0
// $deBlob: FUNCTION 0053d8d0
// $deBlob: FUNCTION 008589b0
TDEFINE_CLASS( ABlobDebugMenuList );

// $deBlob: FUNCTION 0063c7e0
ABlobDebugMenuList::ABlobDebugMenuList()
{
	s_bMusicTester = TFALSE;
}

// $deBlob: FUNCTION 0063cac0
// $deBlob: FUNCTION 0063cad0
// $deBlob: FUNCTION 0063cac5
ABlobDebugMenuList::~ABlobDebugMenuList()
{
}

// $deBlob: FUNCTION 0063c800
void ABlobDebugMenuList::OnItemPressed( TXUIListItem* a_pItem )
{
	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		if ( a_pItem != &m_pItems[ i ] )
			continue;

		s_bMusicTester = TStringManager::String16Compare( a_pItem->GetText(), L"Music Tester", -1 ) == 0;

		APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
		TBOOL           bSave    = TTRUE;

		switch ( i )
		{
			case 0:
				ABlobDebugLevelSelect::ms_eMode = 0;
				bSave                   = TFALSE;
				break;
			case 1:
				ABlobDebugLevelSelect::ms_eMode = 1;
				bSave                   = TFALSE;
				break;
			case 2:
				ABlobDebugLevelSelect::ms_eMode = 2;
				bSave                   = TFALSE;
				break;
			case 3:
				ABlobDebugLevelSelect::ms_eMode = 4;
				bSave                   = TFALSE;
				break;
			case 4:
				ABlobDebugLevelSelect::ms_eMode = 3;
				bSave                   = TFALSE;
				break;
			case 5:
				ABlobDebugLevelSelect::ms_eMode = 5;
				bSave                   = TFALSE;
				break;
			case 6:
				ABlobDebugLevelSelect::ms_eMode = 6;
				bSave                   = TFALSE;
				break;
			case 7:
				ABlobDebugLevelSelect::ms_eMode = 7;
				bSave                   = TFALSE;
				break;
			case 8:
				// Everything
				pProfile->UnlockWorldAndModes( 0 );
				pProfile->UnlockWorldAndModes( 1 );
				pProfile->UnlockWorldAndModes( 2 );
				pProfile->UnlockWorldAndModes( 3 );
				pProfile->UnlockWorldAndModes( 4 );
				pProfile->UnlockWorldAndModes( 5 );
				pProfile->UnlockWorldAndModes( 6 );
				pProfile->UnlockWorldAndModes( 8 );
				pProfile->UnlockWorldAndModes( 7 );
				pProfile->UnlockWorldAndModes( 9 );
				pProfile->UnlockAllBlobPartyItems();
				pProfile->UnlockAllMovies();
				pProfile->UnlockAllGalleryItems();
				pProfile->UnlockAllVideos();
				pProfile->UnlockLastGalleryItem();
				break;
			case 9:
				pProfile->UnlockAllWorlds();
				break;
			case 10:
				pProfile->UnlockWorldAndModes( 0 );
				break;
			case 11:
				pProfile->UnlockWorldAndModes( 1 );
				break;
			case 12:
				pProfile->UnlockWorldAndModes( 2 );
				break;
			case 13:
				pProfile->UnlockWorldAndModes( 3 );
				break;
			case 14:
				pProfile->UnlockWorldAndModes( 4 );
				break;
			case 15:
				pProfile->UnlockWorldAndModes( 5 );
				break;
			case 16:
				pProfile->UnlockWorldAndModes( 6 );
				break;
			case 17:
				pProfile->UnlockWorldAndModes( 8 );
				break;
			case 18:
				pProfile->UnlockWorldAndModes( 7 );
				break;
			case 19:
				pProfile->UnlockWorldAndModes( 9 );
				break;
			case 20:
				pProfile->UnlockAllMovies();
				break;
			case 21:
				pProfile->UnlockAllGalleryItems();
				break;
			case 22:
				pProfile->UnlockAllVideos();
				break;
			case 23:
				pProfile->UnlockLastGalleryItem();
				break;
			case 24:
				pProfile->UnlockAllTracks();
				break;
			default:
				bSave = TFALSE;
				break;
		}

		if ( bSave )
		{
			// The profile is saved at once
			SaveLoad::GetInstance()->SetCaller( FindParentScene() );
			SaveLoad::GetInstance()->SaveProfile( 0, 0 );
			SaveLoad::GetInstance()->Update( 0.0f );
		}
	}

	TXUICommonList::OnItemPressed( a_pItem );
}
