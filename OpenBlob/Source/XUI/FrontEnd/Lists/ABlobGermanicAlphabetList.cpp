#include "pch.h"
#include "ABlobGermanicAlphabetList.h"
#include "XUI/FrontEnd/ABlobNameEntryCommon.h"

#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUIListItem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobGermanicAlphabetList ) == 0x180 );

// The extra letters of the alphabets
static const TWCHAR s_awcUpperSwedish[ 4 ] = { 0x00C5, 0x00C4, 0x00D6, 0 };
static const TWCHAR s_awcLowerSwedish[ 4 ] = { 0x00E5, 0x00E4, 0x00F6, 0 };
static const TWCHAR s_awcUpperDanish[ 4 ]  = { 0x00C6, 0x00D8, 0x00C5, 0 };
static const TWCHAR s_awcLowerDanish[ 4 ]  = { 0x00E6, 0x00F8, 0x00E5, 0 };

// $deBlob: FUNCTION 0063d040
// $deBlob: FUNCTION 0063d050
// $deBlob: FUNCTION 0063d0e0
// $deBlob: FUNCTION 0063d160
// $deBlob: FUNCTION 0063d170
// $deBlob: FUNCTION 0053d990
// $deBlob: FUNCTION 008589e0
TDEFINE_CLASS( ABlobGermanicAlphabetList );

// $deBlob: FUNCTION 0063d180
ABlobGermanicAlphabetList::ABlobGermanicAlphabetList()
{
	m_pNameEntry = TNULL;
}

// $deBlob: FUNCTION 0063d5a0
// $deBlob: FUNCTION 0063d5a5
// $deBlob: FUNCTION 0063d5b0
ABlobGermanicAlphabetList::~ABlobGermanicAlphabetList()
{
}

// $deBlob: FUNCTION 0063d4a0
void ABlobGermanicAlphabetList::OnCreated()
{
	// NOTE: retail doesn't call TXUIList::OnCreated (SetAlphabet does, the item count changes)
	TXUIScene* pParent = FindParentScene();
	TASSERT( pParent->IsA( &TGetClass( ABlobNameEntryCommon ) ) );
	m_pNameEntry = TSTATICCAST( ABlobNameEntryCommon, pParent );

	TINT ilangid = T2Locale::GetSingleton()->GetLanguage();
	TASSERT( ilangid != -1 );

	switch ( ilangid )
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			SetAlphabet( ALPHABET_Upper );
			break;
		case 8:
		case 9:
			SetAlphabet( ALPHABET_UpperDanish );
			break;
		case 10:
		case 11:
			SetAlphabet( ALPHABET_UpperSwedish );
			break;
		default:
			SetAlphabet( ALPHABET_Upper );
			break;
	}
}

// $deBlob: FUNCTION 0063d1b0
const TWCHAR* ABlobGermanicAlphabetList::GetListItemText( TINT a_iItemIndex )
{
	return m_awszChars[ a_iItemIndex ];
}

// $deBlob: FUNCTION 0063d460
void ABlobGermanicAlphabetList::OnItemPressed( TXUIListItem* a_pItem )
{
	// NOTE: retail doesn't call TXUIList::OnItemPressed
	if ( a_pItem->GetText() != TNULL )
		m_pNameEntry->AddChar( a_pItem->GetText()[ 0 ] );
}

// $deBlob: FUNCTION 0063d1c0
void ABlobGermanicAlphabetList::SetAlphabet( TINT a_eAlphabet )
{
	TINT iNumChars = 26;
	TINT iFirst    = 'A';

	if ( a_eAlphabet == ALPHABET_Lower || a_eAlphabet == ALPHABET_LowerDanish || a_eAlphabet == ALPHABET_LowerSwedish )
		iFirst = 'a';

	TWCHAR awcLetters[ MAX_CHARS ];

	for ( TINT i = 0; i < 26; i++ )
	{
		TCHAR  szLetter[ 8 ];
		TWCHAR wszLetter[ 6 ];
		TStringManager::String8Format( szLetter, sizeof( szLetter ), "%c", iFirst + i );
		TStringManager::StringCharToUnicode( wszLetter, szLetter, -1 );
		awcLetters[ i ] = wszLetter[ 0 ];
	}

	const TWCHAR* pExtra;

	switch ( a_eAlphabet )
	{
		case ALPHABET_LowerDanish:
			iNumChars = 29;
			pExtra    = s_awcLowerDanish;
			break;
		case ALPHABET_UpperDanish:
			iNumChars = 29;
			pExtra    = s_awcUpperDanish;
			break;
		case ALPHABET_LowerSwedish:
			iNumChars = 29;
			pExtra    = s_awcLowerSwedish;
			break;
		case ALPHABET_UpperSwedish:
			iNumChars = 29;
			pExtra    = s_awcUpperSwedish;
			break;
		default:
			iNumChars = 26;
			pExtra    = TNULL;
			break;
	}

	for ( TINT i = 0; i < MAX_CHARS; i++ )
	{
		if ( i < iNumChars )
			TStringManager::String16Format( m_awszChars[ i ], L"%lc", ( i >= 26 ) ? pExtra[ i - 26 ] : awcLetters[ i ] );
	}

	if ( iNumChars != TINT( m_uiNumItems ) )
	{
		// The items are created again
		m_uiNumItems = iNumChars;
		delete[] m_pItems;
		TXUIList::OnCreated();
	}
	else if ( a_eAlphabet != m_eAlphabet )
	{
		for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
			GetListItem( i )->SetText( GetListItemText( i ), -1 );
	}

	m_eAlphabet = a_eAlphabet;
}

// $deBlob: FUNCTION 0063d390
void ABlobGermanicAlphabetList::SetCase( TINT a_iCase )
{
	TINT ilangid = T2Locale::GetSingleton()->GetLanguage();
	TASSERT( ilangid != -1 );

	switch ( ilangid )
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			SetAlphabet( a_iCase != 1 );
			break;
		case 8:
		case 9:
			SetAlphabet( ( a_iCase != 1 ) + ALPHABET_UpperDanish );
			break;
		case 10:
		case 11:
			SetAlphabet( ( a_iCase != 1 ) + ALPHABET_UpperSwedish );
			break;
		default:
			SetAlphabet( ALPHABET_Upper );
			break;
	}
}
