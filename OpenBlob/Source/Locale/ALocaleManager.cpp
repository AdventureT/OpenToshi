#include "pch.h"
#include "ALocaleManager.h"

#include <Toshi/TRegion.h>
#include <Input/TInputInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TCHAR* const ALocaleManager::s_LocaleFiles[ LOCALE_LANG_NUMOF ] = {
	"Data/Locale/eng.trb",
	"Data/Locale/eng-uk.trb",
	"Data/Locale/ned.trb",
	"Data/Locale/ger.trb",
	"Data/Locale/ita.trb",
	"Data/Locale/spa.trb",
	"Data/Locale/las.trb",
	"Data/Locale/fre.trb",
	"Data/Locale/dan.trb",
	"Data/Locale/nor.trb",
	"Data/Locale/swe.trb",
	"Data/Locale/fin.trb",
	"Data/Locale/jpn.trb",
	"Data/Locale/kor.trb",
	"Data/Locale/pt-br.trb",
	"Data/Locale/zh.trb",
};
const TCHAR* const ALocaleManager::s_LocaleFormatFiles[ LOCALE_LANG_NUMOF ] = {
	"Data/Locale/format_eng.trb",
	"Data/Locale/format_eng-uk.trb",
	"Data/Locale/format_ned.trb",
	"Data/Locale/format_ger.trb",
	"Data/Locale/format_ita.trb",
	"Data/Locale/format_spa.trb",
	"Data/Locale/format_las.trb",
	"Data/Locale/format_fre.trb",
	"Data/Locale/format_dan.trb",
	"Data/Locale/format_nor.trb",
	"Data/Locale/format_swe.trb",
	"Data/Locale/format_fin.trb",
	"Data/Locale/format_jpn.trb",
	"Data/Locale/format_kor.trb",
	"Data/Locale/format_pt-br.trb",
	"Data/Locale/format_zh.trb",
};
ALocaleManager::Override ALocaleManager::s_Overrides[ NUM_OVERRIDES ];
TBOOL                    ALocaleManager::s_bAreOverridesInitialized;

// $deBlob: FUNCTION 005e1ca0
ALocaleManager::ALocaleManager()
    : T2Locale( LOCALE_LANG_NUMOF, BUFFER_SIZE, TMemoryHeap_Malloc( TMemory_GetGlobalHeap(), BUFFER_SIZE ) )
{
	m_pLocaleBuffer = GetBuffer();
	m_Format.SetMemoryFunctions( TRBAllocator, TRBDeallocator, this );
	InitialiseOverrideTexts();
}

// The override of a string in a_iLangId (TNULL if none or no language)
// $deBlob: FUNCTION 005e23c0
const TWCHAR* ALocaleManager::GetOverwrittenString( TINT a_iStringId, Lang a_iLangId )
{
	if ( a_iLangId == LOCALE_LANG_INVALID )
		return TNULL;

	return FindOverwrittenString( a_iStringId, a_iLangId );
}

// $deBlob: FUNCTION 005e23e0
// $deBlob: FUNCTION 005e1d30
ALocaleManager::~ALocaleManager()
{
	m_Format.Close();
	T2Locale::SetLanguage( -1 );
	TFree( m_pLocaleBuffer );
}

// $deBlob: FUNCTION 005e1db0
void ALocaleManager::Create()
{
	ALocaleManager* pLocaleManager = new ALocaleManager();

	Lang iLangId;

	switch ( TRegion_GetRegion() )
	{
		case 0:
		case 3:
		{
			TINT iRegion = TRegion_GetRegion();
			iLangId      = ( iRegion == 0 ) ? ALANG_ENGLISH : Lang( TUINT( iRegion - 3 ) < 2 );
			break;
		}
		case 1:
			iLangId = ALANG_JAPANESE;
			break;
		case 2:
			iLangId = ALANG_KOREAN;
			break;
		case 4:
			iLangId = ALANG_ENGLISHUK;
			break;
		default:
			return;
	}

	pLocaleManager->SetLanguage( iLangId );
}

// $deBlob: FUNCTION 005e1ea0
void ALocaleManager::SetLanguage( Lang a_iLangId )
{
	TBOOL bChangeLang = a_iLangId != m_LangId;

	if ( bChangeLang )
		m_Format.Close();

	T2Locale::SetLanguage( a_iLangId );

	if ( bChangeLang && a_iLangId >= 0 )
	{
		m_Format.Load( s_LocaleFormatFiles[ a_iLangId ] );
		m_pLocaleFormat = m_Format.CastSymbol<LocaleFormat>( "LocaleFormat" );
	}
}

// $deBlob: FUNCTION 005e1f00
const TCHAR* ALocaleManager::GetCurLocaleCode() const
{
	return GetLocaleCode( m_LangId );
}

// $deBlob: FUNCTION 005e2000
const TCHAR* ALocaleManager::GetCurVOLocaleCode() const
{
	return GetVOLocaleCode( m_LangId );
}

// $deBlob: FUNCTION 005e2010
const TCHAR* ALocaleManager::GetVOLocaleCode( Lang a_iLangId )
{
	switch ( a_iLangId )
	{
		case ALANG_DUTCH: return "NL";
		case ALANG_GERMAN: return "DE";
		case ALANG_ITALIAN: return "IT";
		case ALANG_FRENCH: return "FR";
		default: return "EN";
	}
}

// $deBlob: FUNCTION 005e1f10
const TCHAR* ALocaleManager::GetLocaleCode( Lang a_iLangId )
{
	switch ( a_iLangId )
	{
		case ALANG_ENGLISHUK: return "UK";
		case ALANG_DUTCH: return "NL";
		case ALANG_GERMAN: return "DE";
		case ALANG_ITALIAN: return "IT";
		case ALANG_SPANISH: return "ES";
		case ALANG_LATAMSPANISH: return "LAS";
		case ALANG_FRENCH: return "FR";
		case ALANG_DANISH: return "DA";
		case ALANG_NORWEGIAN: return "NO";
		case ALANG_SWEDISH: return "SW";
		case ALANG_FINNISH: return "FI";
		case ALANG_JAPANESE: return "JP";
		case ALANG_KOREAN: return "KO";
		case ALANG_BRAZILIAN: return "PT";
		case ALANG_CHINESE: return "ZH";
		default: return "US";
	}
}

// GetCurSoundChannel for a_iLangId
// $deBlob: FUNCTION 005e2070
TINT ALocaleManager::GetSoundChannel( Lang a_iLangId ) const
{
	switch ( a_iLangId )
	{
		case ALANG_DUTCH: return 3;
		case ALANG_GERMAN: return 1;
		case ALANG_FRENCH: return 2;
		default: return 0;
	}
}

// $deBlob: FUNCTION 005e20b0
TINT ALocaleManager::GetCurSoundChannel() const
{
	switch ( m_LangId )
	{
		case ALANG_DUTCH: return 3;
		case ALANG_GERMAN: return 1;
		case ALANG_FRENCH: return 2;
		default: return 0;
	}
}

// $deBlob: FUNCTION 005e2100
ALocaleManager::StringFormat* ALocaleManager::GetStringFormat( TINT a_iStringId )
{
	TINT iStringId = FixStringIdPlatform( a_iStringId );

	TASSERT( iStringId >= 0 && iStringId < m_stringtable->m_numstrings );
	return m_pLocaleFormat->m_pFormats[ iStringId ];
}

// $deBlob: FUNCTION 005e2160
const TWCHAR* ALocaleManager::GetOverwrittenFormatString( TINT a_iStringId, const TWCHAR* a_wszFallback ) const
{
	if ( m_LangId == -1 )
		return TNULL;

	const TWCHAR* wszOverride = FindOverwrittenFormatString( a_iStringId, m_LangId );

	if ( wszOverride != TNULL )
		a_wszFallback = wszOverride;

	return a_wszFallback;
}

// The override of a string in a_iLangId, TNULL if none
// $deBlob: FUNCTION 005e2530
const TWCHAR* ALocaleManager::FindOverwrittenString( TINT a_iStringId, Lang a_iLangId )
{
	for ( TINT i = 0; i < NUM_OVERRIDES; i++ )
	{
		if ( s_Overrides[ i ].m_iStringId == a_iStringId )
			return s_Overrides[ i ].m_aLocaleStrings[ a_iLangId ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 005e2570
const TWCHAR* ALocaleManager::FindOverwrittenFormatString( TINT a_iStringId, Lang a_iLangId )
{
	for ( TINT i = 0; i < NUM_OVERRIDES; i++ )
	{
		if ( s_Overrides[ i ].m_iStringId == a_iStringId && s_Overrides[ i ].m_aLocaleStrings[ a_iLangId ] != TNULL )
			return s_Overrides[ i ].m_aFormatStrings[ a_iLangId ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 005e2190
const TCHAR* ALocaleManager::GetLanguageFilename( Lang a_iLangId )
{
	TASSERT( a_iLangId < LOCALE_LANG_NUMOF );
	TASSERT( a_iLangId > LOCALE_LANG_INVALID );

	return s_LocaleFiles[ a_iLangId ];
}

// $deBlob: FUNCTION 005e1e70
void ALocaleManager::Destroy()
{
	if ( T2Locale::GetSingleton() != TNULL )
		delete T2Locale::GetSingleton();
}

// $deBlob: FUNCTION 005e2210
TINT ALocaleManager::FixStringIdPlatform( TINT a_iStringId )
{
	if ( TInputDevice::GetLastGamepadType() != TInputDevice::Gamepad_Wii )
	{
		// The PC strings of the controller texts
		switch ( a_iStringId )
		{
			case 491: return 860;
			case 493: return 861;
			case 521: return 864;
			case 525: return 862;
			case 535: return 865;
			case 565: return 878;
			case 739: return 863;
			case 905: return 983;
			case 1071: return 1171;
			case 1239: return 1267;
			default: return a_iStringId;
		}
	}

	switch ( a_iStringId )
	{
		case 860: return 491;
		case 861: return 493;
		case 862: return 525;
		case 863: return 739;
		case 864: return 521;
		case 865: return 535;
		case 878: return 565;
		case 983: return 905;
		case 1171: return 1071;
		case 1267: return 1239;
		default: return a_iStringId;
	}
}

// $deBlob: FUNCTION 005e2470
void ALocaleManager::InitialiseOverrideTexts()
{
	if ( s_bAreOverridesInitialized ) return;
	s_bAreOverridesInitialized = TTRUE;

	for ( TINT i = 0; i < NUM_OVERRIDES; i++ )
	{
		s_Overrides[ i ].m_iStringId = 0;
		TUtil::MemClear( s_Overrides[ i ].m_aLocaleStrings, sizeof( s_Overrides[ i ].m_aLocaleStrings ) );
		TUtil::MemClear( s_Overrides[ i ].m_aFormatStrings, sizeof( s_Overrides[ i ].m_aFormatStrings ) );
	}

	static TWCHAR s_wszOverride0[] = L"@\u3092\u9577\u62bc\u3057\u3057\u3066\u5f31\u70b9\u3092\u7a81\u304d\u3001\u4fb5\u5165\u3057\u3088\u3046\u3002";
	static TWCHAR s_wszOverride1[] = L"\u3053\u306e2\u30d6\u30ed\u30c3\u30af\u306b\u306f\u304b\u3064\u3066\u30df\u30e5\u30fc\u30b8\u30c3\u30af\u30d0\u30fc\u3068\u30c0\u30f3\u30b9\u30db\u30fc\u30eb\u304c\u3042\u3063\u305f\u3002*\u300c\u4eca\u3058\u3083\u805e\u3053\u3048\u308b\u306e\u306f\u30b0\u30ec\u30fc\u30c7\u30a3\u30a2\u30f3\u305f\u3061\u304c\u30de\u30b7\u30fc\u30f3\u3092\u5897\u8a2d\u3059\u308b\u30bf\u30c3\u30d7\u97f3\u3060\u3051\u300d*\u6d3b\u6c17\u3092\u53d6\u308a\u623b\u3057\u3066\u3001\u30d6\u30ed\u30d6\uff01\u5efa\u7269\u3092\u7dd1\u306b\u5857\u308d\u3046\u3002";

	s_Overrides[ 0 ].m_iStringId                        = 520;
	s_Overrides[ 0 ].m_aLocaleStrings[ ALANG_JAPANESE ] = s_wszOverride0;
	s_Overrides[ 0 ].m_aFormatStrings[ ALANG_JAPANESE ] = L"";

	// The last '@' of the texts of the first override is the button icon 0x84
	for ( TINT i = 0; i < LOCALE_LANG_NUMOF; i++ )
	{
		if ( s_Overrides[ 0 ].m_aLocaleStrings[ i ] != TNULL )
		{
			TWCHAR* pAt = wcsrchr( const_cast<TWCHAR*>( s_Overrides[ 0 ].m_aLocaleStrings[ i ] ), L'@' );
			if ( pAt != TNULL ) *pAt = TWCHAR( 0x84 );
		}
	}

	s_Overrides[ 1 ].m_iStringId                        = 985;
	s_Overrides[ 1 ].m_aLocaleStrings[ ALANG_JAPANESE ] = s_wszOverride1;
	s_Overrides[ 1 ].m_aFormatStrings[ ALANG_JAPANESE ] = L"";
}
