#include "pch.h"
#include "ABlobJPNAlphabetList.h"
#include "XUI/FrontEnd/ABlobJPNNameEntryScene.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobJPNAlphabetList ) == 0x1A8 );

// The hiragana and katakana of the list
static const TWCHAR s_awcHiragana[ ABlobJPNAlphabetList::NUM_CHARS ] = {
	0x3042, 0x304B, 0x3055, 0x305F, 0x306A, 0x306F, 0x307E, 0x3084, 0x3089, 0x308F, 0x3044, 0x304D, 0x3057, 0x3061, 0x306B, 0x3072, 0x307F,
	0x3086, 0x308A, 0x3092, 0x3046, 0x304F, 0x3059, 0x3064, 0x306C, 0x3075, 0x3080, 0x3088, 0x308B, 0x3093, 0x3048, 0x3051, 0x305B, 0x3066,
	0x306D, 0x3078, 0x3081, 0x0021, 0x308C, 0x3002, 0x304A, 0x3053, 0x305D, 0x3068, 0x306E, 0x307B, 0x3082, 0x003F, 0x308D, 0x3001
};
static const TWCHAR s_awcKatakana[ ABlobJPNAlphabetList::NUM_CHARS ] = {
	0x30A2, 0x30AB, 0x30B5, 0x30BF, 0x30CA, 0x30CF, 0x30DE, 0x30E4, 0x30E9, 0x30EF, 0x30A4, 0x30AD, 0x30B7, 0x30C1, 0x30CB, 0x30D2, 0x30DF,
	0x30E6, 0x30EA, 0x30F2, 0x30A6, 0x30AF, 0x30B9, 0x30C4, 0x30CC, 0x30D5, 0x30E0, 0x30E8, 0x30EB, 0x30F3, 0x30A8, 0x30B1, 0x30BB, 0x30C6,
	0x30CD, 0x30D8, 0x30E1, 0x0021, 0x30EC, 0x3002, 0x30AA, 0x30B3, 0x30BD, 0x30C8, 0x30CE, 0x30DB, 0x30E2, 0x003F, 0x30ED, 0x3001
};

// The symbols after the latin letters and digits
static const TWCHAR s_awcSymbols[ 14 ] = { 0x266A, 0x266D, 0x266F, 0x2606, 0x2605, 0xE2A3, 0xE29F, 0xE29E, 0xE2A0, 0xE2A1, 0xE2B4, 0xE2B1, 0xE2B2, 0xE2B3 };

// The modifier pairs (plain, modified)
static const TWCHAR s_aHiraganaHandakuten[ 5 ][ 2 ] = {
	{ 0x306F, 0x3071 },
	{ 0x3072, 0x3074 },
	{ 0x3075, 0x3077 },
	{ 0x3078, 0x307A },
	{ 0x307B, 0x307D }
};
static const TWCHAR s_aKatakanaHandakuten[ 5 ][ 2 ] = {
	{ 0x30CF, 0x30D1 },
	{ 0x30D2, 0x30D4 },
	{ 0x30D5, 0x30D7 },
	{ 0x30D8, 0x30DA },
	{ 0x30DB, 0x30DD }
};
static const TWCHAR s_aHiraganaDakuten[ 20 ][ 2 ] = {
	{ 0x304B, 0x304C },
	{ 0x304D, 0x304E },
	{ 0x304F, 0x3050 },
	{ 0x3051, 0x3052 },
	{ 0x3053, 0x3054 },
	{ 0x306F, 0x3070 },
	{ 0x3072, 0x3073 },
	{ 0x3075, 0x3076 },
	{ 0x3078, 0x3079 },
	{ 0x307B, 0x307C },
	{ 0x3055, 0x3056 },
	{ 0x3057, 0x3058 },
	{ 0x3059, 0x305A },
	{ 0x305B, 0x305C },
	{ 0x305D, 0x305E },
	{ 0x305F, 0x3060 },
	{ 0x3061, 0x3062 },
	{ 0x3064, 0x3065 },
	{ 0x3066, 0x3067 },
	{ 0x3068, 0x3069 }
};
static const TWCHAR s_aHiraganaSmall[ 10 ][ 2 ] = {
	{ 0x3042, 0x3041 },
	{ 0x3044, 0x3043 },
	{ 0x3046, 0x3045 },
	{ 0x3048, 0x3047 },
	{ 0x304A, 0x3049 },
	{ 0x3064, 0x3063 },
	{ 0x3084, 0x3083 },
	{ 0x3086, 0x3085 },
	{ 0x3088, 0x3087 },
	{ 0x308F, 0x308E }
};
static const TWCHAR s_aKatakanaDakuten[ 21 ][ 2 ] = {
	{ 0x30AB, 0x30AC },
	{ 0x30AD, 0x30AE },
	{ 0x30AF, 0x30B0 },
	{ 0x30B1, 0x30B2 },
	{ 0x30B3, 0x30B4 },
	{ 0x30B5, 0x30B6 },
	{ 0x30B7, 0x30B8 },
	{ 0x30B9, 0x30BA },
	{ 0x30BB, 0x30BC },
	{ 0x30BD, 0x30BE },
	{ 0x30CF, 0x30D0 },
	{ 0x30D2, 0x30D3 },
	{ 0x30D5, 0x30D6 },
	{ 0x30D8, 0x30D9 },
	{ 0x30DB, 0x30DC },
	{ 0x30BF, 0x30C0 },
	{ 0x30C1, 0x30C2 },
	{ 0x30C4, 0x30C5 },
	{ 0x30C6, 0x30C7 },
	{ 0x30C8, 0x30C9 },
	{ 0x30A6, 0x30F4 }
};
// NOTE: the last pair is retail's hiragana wa (0x308F, 0x308E)
static const TWCHAR s_aKatakanaSmall[ 10 ][ 2 ] = {
	{ 0x30A2, 0x30A1 },
	{ 0x30A4, 0x30A3 },
	{ 0x30A6, 0x30A5 },
	{ 0x30A8, 0x30A7 },
	{ 0x30AA, 0x30A9 },
	{ 0x30C4, 0x30C3 },
	{ 0x30E4, 0x30E3 },
	{ 0x30E6, 0x30E5 },
	{ 0x30E8, 0x30E7 },
	{ 0x308F, 0x308E }
};

// $deBlob: FUNCTION 0063d5e0
// $deBlob: FUNCTION 0063d5f0
// $deBlob: FUNCTION 0063d680
// $deBlob: FUNCTION 0063d710
// $deBlob: FUNCTION 0063d720
// $deBlob: FUNCTION 0053d9d0
// $deBlob: FUNCTION 008589f0
TDEFINE_CLASS( ABlobJPNAlphabetList );

// $deBlob: FUNCTION 0063d730
ABlobJPNAlphabetList::ABlobJPNAlphabetList()
{
	m_pNameEntry = TNULL;
	m_uiNumItems = NUM_CHARS;
}

// $deBlob: FUNCTION 0063dce0
// $deBlob: FUNCTION 0063dce5
// $deBlob: FUNCTION 0063dcf0
ABlobJPNAlphabetList::~ABlobJPNAlphabetList()
{
}

// $deBlob: FUNCTION 0063dbf0
void ABlobJPNAlphabetList::OnCreated()
{
	T2GUIElement* pParent = GetParent();

	if ( pParent != TNULL && TSTATICCAST( TXUIElement, pParent )->IsA( &TGetClass( ABlobJPNNameEntryScene ) ) )
		m_pNameEntry = TSTATICCAST( ABlobJPNNameEntryScene, pParent );

	TXUIList::OnCreated();

	for ( TINT i = 0; i < NUM_CHARS; i++ )
	{
		SetListItemVisible( i, TTRUE );
		SetListItemEnabled( i, TTRUE );
		TStringManager::String16Format( m_awszChars[ i ], L"%lc", s_awcHiragana[ i ] );
	}

	m_eMode = MODE_Hiragana;
}

// $deBlob: FUNCTION 0063dca0
TBOOL ABlobJPNAlphabetList::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	// NOTE: retail doesn't call TXUIList::OnNotifyPress
	const TWCHAR* wszText = TSTATICCAST( TXUIControl, a_pSource )->GetText();

	if ( wszText != TNULL && TStringManager::String16Length( wszText ) > 0 )
		m_pNameEntry->AddChar( wszText[ 0 ] );

	return TFALSE;
}

// $deBlob: FUNCTION 0063d760
const TWCHAR* ABlobJPNAlphabetList::GetListItemText( TINT a_iItemIndex )
{
	return m_awszChars[ a_iItemIndex ];
}

// $deBlob: FUNCTION 0063d770
void ABlobJPNAlphabetList::ChangeMode( TINT a_eModifier )
{
	TINT eMode = MODE_Hiragana;

	if ( a_eModifier != MODIFIER_Hiragana )
	{
		if ( a_eModifier == MODIFIER_Katakana )
		{
			SetMode( MODE_Katakana );
			return;
		}

		if ( a_eModifier == MODIFIER_English )
		{
			SetMode( MODE_UpperCase );
			return;
		}

		TINT eCurrent = m_eMode;

		if ( eCurrent < MODE_KatakanaSmall )
		{
			switch ( a_eModifier )
			{
				case MODIFIER_Dakuten:
					SetMode( ( eCurrent != MODE_HiraganaDakuten ) + MODE_Hiragana );
					return;
				case MODIFIER_Handakuten:
					SetMode( 2 * ( eCurrent != MODE_HiraganaHandakuten ) + MODE_Hiragana );
					return;
				case MODIFIER_Small:
					SetMode( eCurrent == MODE_HiraganaSmall );
					return;
			}
		}
		else if ( eCurrent < MODE_UpperCase )
		{
			switch ( a_eModifier )
			{
				case MODIFIER_Dakuten:
					SetMode( ( eCurrent != MODE_KatakanaDakuten ) + MODE_Katakana );
					return;
				case MODIFIER_Handakuten:
					SetMode( 2 * ( eCurrent != MODE_KatakanaHandakuten ) + MODE_Katakana );
					return;
				case MODIFIER_Small:
					SetMode( ( eCurrent == MODE_KatakanaSmall ) + MODE_KatakanaSmall );
					return;
			}
		}
		else
		{
			if ( a_eModifier != MODIFIER_Small )
				return;

			eMode = MODE_LowerCase;
		}
	}

	SetMode( eMode );
}

// $deBlob: FUNCTION 0063d890
void ABlobJPNAlphabetList::SetMode( TINT a_eMode )
{
	const TWCHAR( *pModifiers )[ 2 ] = TNULL;
	const TWCHAR* pChars;
	TWCHAR        awcLatin[ NUM_CHARS ];

	if ( TUINT( a_eMode ) <= MODE_HiraganaHandakuten )
	{
		pChars = s_awcHiragana;
	}
	else if ( a_eMode < MODE_UpperCase )
	{
		pChars = s_awcKatakana;
	}
	else
	{
		// A-Z (a-z), 0-9 and the symbols
		TINT iFirst = ( a_eMode == MODE_LowerCase ) ? 'a' : 'A';

		for ( TINT i = 0; i < 26; i++ )
		{
			TCHAR  szChar[ 8 ];
			TWCHAR wszChar[ 6 ];
			TStringManager::String8Format( szChar, sizeof( szChar ), "%c", iFirst + i );
			TStringManager::StringCharToUnicode( wszChar, szChar, -1 );
			awcLatin[ i ] = wszChar[ 0 ];
		}

		for ( TINT i = 0; i < 10; i++ )
		{
			TCHAR  szChar[ 8 ];
			TWCHAR wszChar[ 6 ];
			TStringManager::String8Format( szChar, sizeof( szChar ), "%c", '0' + i );
			TStringManager::StringCharToUnicode( wszChar, szChar, -1 );
			awcLatin[ 26 + i ] = wszChar[ 0 ];
		}

		for ( TINT i = 0; i < 14; i++ )
			awcLatin[ 36 + i ] = s_awcSymbols[ i ];

		pChars = awcLatin;
	}

	TINT iNumModifiers = 0;

	switch ( a_eMode )
	{
		case MODE_HiraganaSmall:
			pModifiers    = s_aHiraganaSmall;
			iNumModifiers = 10;
			break;
		case MODE_Hiragana:
			iNumModifiers = 0;
			break;
		case MODE_HiraganaDakuten:
			iNumModifiers = 20;
			pModifiers    = s_aHiraganaDakuten;
			break;
		case MODE_HiraganaHandakuten:
			pModifiers    = s_aHiraganaHandakuten;
			iNumModifiers = 5;
			break;
		case MODE_KatakanaSmall:
			iNumModifiers = 10;
			pModifiers    = s_aKatakanaSmall;
			break;
		case MODE_KatakanaDakuten:
			iNumModifiers = 21;
			pModifiers    = s_aKatakanaDakuten;
			break;
		case MODE_KatakanaHandakuten:
			pModifiers    = s_aKatakanaHandakuten;
			iNumModifiers = 5;
			break;
	}

	for ( TINT i = 0; i < NUM_CHARS; i++ )
	{
		TWCHAR wcChar = pChars[ i ];

		SetListItemVisible( i, TTRUE );
		SetListItemEnabled( i, TTRUE );

		if ( pModifiers != TNULL )
		{
			for ( TINT j = 0; j < iNumModifiers; j++ )
			{
				if ( pModifiers[ j ][ 0 ] == wcChar )
				{
					wcChar = pModifiers[ j ][ 1 ];
					break;
				}
			}
		}

		TStringManager::String16Format( m_awszChars[ i ], L"%lc", wcChar );
	}

	m_eMode = a_eMode;
}

// $deBlob: FUNCTION 0063db00
void ABlobJPNAlphabetList::ModifyLastChar( TINT a_eModifier )
{
	const TWCHAR( *pModifiers )[ 2 ] = TNULL;
	TINT                         iNumModifiers;

	switch ( a_eModifier )
	{
		case MODIFIER_Small:
		if ( m_eMode == MODE_Katakana )
		{
			pModifiers    = s_aKatakanaSmall;
			iNumModifiers = 10;
		}
		else if ( m_eMode == MODE_Hiragana )
		{
			pModifiers    = s_aHiraganaSmall;
			iNumModifiers = 10;
		}
		break;

		case MODIFIER_Handakuten:
		if ( m_eMode == MODE_Katakana )
		{
			pModifiers    = s_aKatakanaHandakuten;
			iNumModifiers = 5;
		}
		else if ( m_eMode == MODE_Hiragana )
		{
			pModifiers    = s_aHiraganaHandakuten;
			iNumModifiers = 5;
		}
		break;

		case MODIFIER_Dakuten:
		if ( m_eMode == MODE_Katakana )
		{
			pModifiers    = s_aKatakanaDakuten;
			iNumModifiers = 21;
		}
		else if ( m_eMode == MODE_Hiragana )
		{
			pModifiers    = s_aHiraganaDakuten;
			iNumModifiers = 20;
		}
		break;

		default:
			break;
	}

	if ( pModifiers == TNULL )
		return;

	TWCHAR* wszName = m_pNameEntry->GetName();
	TINT    iLength = TStringManager::String16Length( wszName );

	if ( iLength == 0 )
		return;

	// Toggles the modifier of the last character
	TWCHAR wcLast = wszName[ iLength - 1 ];

	for ( TINT i = 0; i < iNumModifiers; i++ )
	{
		if ( pModifiers[ i ][ 0 ] == wcLast )
		{
			wszName[ iLength - 1 ] = pModifiers[ i ][ 1 ];
			return;
		}

		if ( pModifiers[ i ][ 1 ] == wcLast )
		{
			wszName[ iLength - 1 ] = pModifiers[ i ][ 0 ];
			return;
		}
	}
}
