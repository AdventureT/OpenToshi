#include "pch.h"
#include "ABlobDebugLevelSelect.h"
#include "ALevelInformation.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AXUIState.h"
#include "XUI/FrontEnd/Multiplayer/ABlobMPLevelSelection.h"

#include <XUI/TXUIListItem.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TINT ABlobDebugLevelSelect::ms_eMode;

// The levels of the worlds and their names
static const TCHAR* s_aszWorldLevels[ 10 ] = {
	"LastResort", "Bridge", "Slums", "Dam", "Guggentraz", "Inkydocks", "Abyss", "Ink_Harvest", "Military_02", "Spaceship"
};
static const TINT s_aiWorldNames[ 10 ] = { 124, 125, 126, 127, 128, 129, 130, 131, 132, 133 };

// The sprint challenges by slot
static const TCHAR* s_aszSlotLevels[ 20 ] = {
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

// MODE_Sprints: the levels and their names
static const TCHAR* s_aszSprints[ 12 ] = {
	"Lost_Highway_Sprint",
	"Cannons_Above_Sprint",
	"Hill_Climb_Sprint",
	"Billboard_Orama_Sprint",
	"Ship2Ship_Sprint",
	"Trainspotting_Sprint",
	"Miles_O_Silos_Sprint",
	"Top_Of_The_World_Sprint",
	"Forest_Sprint",
	"St_Peters_Square_Sprint",
	"Blob_In_Venice_Sprint",
	"Deep_Down_Sprint",
};
static const TINT s_aiSprintNames[ 12 ] = { 139, 136, 150, 146, 137, 143, 144, 135, 145, 152, 134, 151 };

// MODE_SprintsA: the levels and their names
static const TCHAR* s_aszSprintsA[ 4 ]     = { "Inky_Academy_Sprint", "Full_Metal_Inky_Sprint", "Inkyball_Sprint", "Mile_High_Fight_Club_Sprint" };
static const TINT   s_aiSprintsANames[ 4 ] = { 140, 147, 149, 141 };

// MODE_SprintsB: the levels and their names
static const TCHAR* s_aszSprintsB[ 4 ]     = { "HotPlates_R_US_Sprint", "Stamps_Sprint", "Tower_Of_Babel_Sprint", "Hells_Angels_Attack_Sprint" };
static const TINT   s_aiSprintsBNames[ 4 ] = { 138, 148, 153, 142 };

// $deBlob: FUNCTION 0065c0f0
// $deBlob: FUNCTION 0065c100
// $deBlob: FUNCTION 0065c170
// $deBlob: FUNCTION 0053e680
// $deBlob: FUNCTION 0065c1e0
// $deBlob: FUNCTION 0065c1f0
// $deBlob: FUNCTION 00858d10
TDEFINE_CLASS( ABlobDebugLevelSelect );

// $deBlob: FUNCTION 0065c200
// $deBlob: FUNCTION 0065c9c0
ABlobDebugLevelSelect::ABlobDebugLevelSelect()
{
	ALevelInformation* levelInfo = ALevelInformation::GetSingleton();
	TASSERT( levelInfo->GetLevelHeader()->m_levelCount > 0 );

	switch ( ms_eMode )
	{
		case MODE_Worlds:
			m_uiNumItems = 10;
			break;
		case MODE_Sprints:
			m_uiNumItems = 12;
			break;
		case MODE_SprintsA:
		case MODE_SprintsB:
			m_uiNumItems = 4;
			break;
		case MODE_Multiplayer:
			m_uiNumItems = 9;
			break;
		case MODE_AllLevels:
		case MODE_AllLevelsIndex:
		case MODE_AllLevels2:
			for ( TINT i = 0; i < levelInfo->GetLevelHeader()->m_levelCount; i++ )
			{
				if ( levelInfo->GetLevelHeader()->m_pLevelProps[ i ].m_uiUnk28 != 0 )
					m_uiNumItems++;
			}
			break;
	}

	m_pItemTexts = new TString16[ m_uiNumItems ];
}

// $deBlob: FUNCTION 0065c3a0
// $deBlob: FUNCTION 0065c954
// $deBlob: FUNCTION 0065c960
// $deBlob: FUNCTION 0065c9e0
ABlobDebugLevelSelect::~ABlobDebugLevelSelect()
{
	if ( m_pItemTexts != TNULL )
		delete[] m_pItemTexts;
}

// $deBlob: FUNCTION 0065c430
const TWCHAR* ABlobDebugLevelSelect::GetListItemText( TINT a_iItemIndex )
{
	const TCHAR* szText;

	switch ( ms_eMode )
	{
		case MODE_Worlds:
			szText = s_aszWorldLevels[ a_iItemIndex ];
			break;
		case MODE_Sprints:
			szText = s_aszSprints[ a_iItemIndex ];
			break;
		case MODE_SprintsA:
			szText = s_aszSprintsA[ a_iItemIndex ];
			break;
		case MODE_Multiplayer:
			szText = ABlobMPLevelSelection::sm_aszLevels[ a_iItemIndex ];
			break;
		case MODE_SprintsB:
			szText = s_aszSprintsB[ a_iItemIndex ];
			break;
		case MODE_AllLevels:
		case MODE_AllLevelsIndex:
		case MODE_AllLevels2:
		{
			// The a_iItemIndex-th level with m_uiUnk28 set
			ALevelInformation::LevelHeader* pHeader = ALevelInformation::GetSingleton()->GetLevelHeader();

			if ( a_iItemIndex < 0 || a_iItemIndex >= pHeader->m_levelCount )
				return TNULL;

			TINT iItem = 0;
			TINT i     = 0;

			for ( ; i < pHeader->m_levelCount; i++ )
			{
				if ( pHeader->m_pLevelProps[ i ].m_uiUnk28 != 0 )
				{
					if ( iItem == a_iItemIndex )
						break;

					iItem++;
				}
			}

			if ( i >= pHeader->m_levelCount )
				return TNULL;

			szText = pHeader->m_pLevelProps[ i ].m_szName;
			break;
		}
		default:
			return TNULL;
	}

	m_pItemTexts[ a_iItemIndex ].Copy( szText, -1 );
	return m_pItemTexts[ a_iItemIndex ].GetString( 0 );
}

// $deBlob: FUNCTION 0065c540
void ABlobDebugLevelSelect::OnItemPressed( TXUIListItem* a_pItem )
{
	TINT iIndex = -1;

	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		if ( &m_pItems[ i ] == a_pItem )
			iIndex = i;
	}

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AXUIState ) ) );

	TINT         iName;
	const TCHAR* szLevel;

	switch ( ms_eMode )
	{
		case MODE_Worlds:
			AGameStatus::GetSingleton()->SetSPLevel( iIndex );
			iName   = s_aiWorldNames[ iIndex ];
			szLevel = s_aszWorldLevels[ iIndex ];
			break;
		case MODE_Sprints:
		{
			TINT iSprintIndex = -1;

			switch ( iIndex )
			{
				case 0: iSprintIndex = 5; break;
				case 1: iSprintIndex = 2; break;
				case 2: iSprintIndex = 16; break;
				case 3: iSprintIndex = 12; break;
				case 4: iSprintIndex = 3; break;
				case 5: iSprintIndex = 9; break;
				case 6: iSprintIndex = 10; break;
				case 7: iSprintIndex = 1; break;
				case 8: iSprintIndex = 11; break;
				case 9: iSprintIndex = 18; break;
				case 10: iSprintIndex = 0; break;
				case 11: iSprintIndex = 17; break;
			}

			TASSERT( iSprintIndex != -1 );
			AGameStatus::GetSingleton()->SetSelectedSlot( iSprintIndex );
			iName   = s_aiSprintNames[ iIndex ];
			szLevel = s_aszSprints[ iIndex ];
			break;
		}
		case MODE_SprintsA:
		{
			TINT iSprintIndex = -1;

			switch ( iIndex )
			{
				case 0: iSprintIndex = 6; break;
				case 1: iSprintIndex = 13; break;
				case 2: iSprintIndex = 15; break;
				case 3: iSprintIndex = 7; break;
			}

			TASSERT( iSprintIndex != -1 );
			AGameStatus::GetSingleton()->SetSelectedSlot( iSprintIndex );
			iName   = s_aiSprintsANames[ iIndex ];
			szLevel = s_aszSprintsA[ iIndex ];
			break;
		}
		case MODE_Multiplayer:
			iName   = ABlobMPLevelSelection::sm_aiLevelNames[ iIndex ];
			szLevel = ABlobMPLevelSelection::sm_aszLevels[ iIndex ];
			break;
		case MODE_SprintsB:
		{
			TINT iSprintIndex = -1;

			switch ( iIndex )
			{
				case 0: iSprintIndex = 4; break;
				case 1: iSprintIndex = 14; break;
				case 2: iSprintIndex = 19; break;
				case 3: iSprintIndex = 8; break;
			}

			TASSERT( iSprintIndex != -1 );
			AGameStatus::GetSingleton()->SetSelectedSlot( iSprintIndex );
			iName   = s_aiSprintsBNames[ iIndex ];
			szLevel = s_aszSprintsB[ iIndex ];
			break;
		}
		case MODE_AllLevels:
		case MODE_AllLevelsIndex:
		case MODE_AllLevels2:
		{
			if ( iIndex == -1 || iIndex < 0 )
				return;

			// The iIndex-th level with m_uiUnk28 set
			ALevelInformation::LevelHeader* pHeader = ALevelInformation::GetSingleton()->GetLevelHeader();

			if ( iIndex >= pHeader->m_levelCount )
				return;

			TINT iItem  = 0;
			TINT iLevel = 0;

			for ( ;; iLevel++ )
			{
				if ( iLevel >= pHeader->m_levelCount )
					return;

				if ( pHeader->m_pLevelProps[ iLevel ].m_uiUnk28 != 0 )
				{
					if ( iItem == iIndex )
						break;

					iItem++;
				}
			}

			ALevelInformation::LevelProperties* pProps = &pHeader->m_pLevelProps[ iLevel ];

			// The world or the sprint challenge of the level
			TBOOL bFound = TFALSE;

			for ( TINT i = 0; i < 10; i++ )
			{
				if ( TStringManager::String8Compare( pProps->m_szUnk08, s_aszWorldLevels[ i ], TStringManager::String8Length( s_aszWorldLevels[ i ] ) ) == 0 )
				{
					AGameStatus::GetSingleton()->SetSPLevel( i );
					bFound = TTRUE;
					break;
				}
			}

			if ( !bFound )
			{
				for ( TINT i = 0; i < 20; i++ )
				{
					if ( TStringManager::String8Compare( pProps->m_szUnk08, s_aszSlotLevels[ i ], TStringManager::String8Length( s_aszSlotLevels[ i ] ) ) == 0 )
					{
						AGameStatus::GetSingleton()->SetSelectedSlot( i );
						break;
					}
				}
			}

			TSTATICCAST( AXUIState, pState )->SetNextLevel( pProps->m_szName, L"", ( ms_eMode != MODE_AllLevelsIndex ) ? -1 : iLevel );
			return;
		}
		default:
			return;
	}

	const TWCHAR* wszName = ( iName == -1 ) ? L"" : T2Locale::GetSingleton()->GetString( iName );
	TSTATICCAST( AXUIState, pState )->SetNextLevel( szLevel, wszName, iIndex );
}
