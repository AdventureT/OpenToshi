#pragma once
#include <T2Locale/T2Locale.h>

//-----------------------------------------------------------------------------
// Purpose: the game's locale: the string tables of the languages and their format tables.
//-----------------------------------------------------------------------------
class ALocaleManager : public Toshi::T2Locale
{
public:
	enum LANG : Lang
	{
		ALANG_INVALID = -1,
		ALANG_ENGLISH = 0,
		ALANG_ENGLISHUK,
		ALANG_DUTCH,
		ALANG_GERMAN,
		ALANG_ITALIAN,
		ALANG_SPANISH,
		ALANG_LATAMSPANISH,
		ALANG_FRENCH,
		ALANG_DANISH,
		ALANG_NORWEGIAN,
		ALANG_SWEDISH,
		ALANG_FINNISH,
		ALANG_JAPANESE,
		ALANG_KOREAN,
		ALANG_BRAZILIAN,
		ALANG_CHINESE,
		LOCALE_LANG_NUMOF
	};

	static TCONSTEXPR_VAR TINT LOCALE_LANG_INVALID = ALANG_INVALID;

	static TCONSTEXPR_VAR TSIZE BUFFER_SIZE = 0x64000;

	static const TCHAR* const s_LocaleFiles[ LOCALE_LANG_NUMOF ];

	static const TCHAR* const s_LocaleFormatFiles[ LOCALE_LANG_NUMOF ];

	// Texts replacing strings of the string tables
	struct Override
	{
		TINT          m_iStringId;                           // 0x00
		const TWCHAR* m_aLocaleStrings[ LOCALE_LANG_NUMOF ]; // 0x04
		const TWCHAR* m_aFormatStrings[ LOCALE_LANG_NUMOF ]; // 0x44
	};

	static TCONSTEXPR_VAR TINT NUM_OVERRIDES = 2;

	// The format of a string (Data/Locale/*Format.trb)
	struct StringFormat
	{
		const TWCHAR* m_wszFormat; // 0x00
	};

	struct LocaleFormat
	{
		StringFormat** m_pFormats; // 0x00 one per string
		TUINT32        m_uiUnk04;  // 0x04 passed to TXUIControl::SetText2 with the format
	};

public:
	ALocaleManager();
	~ALocaleManager();

	virtual const TCHAR* GetLanguageFilename( Lang a_iLangId ) override;

	// Loads the strings and the formats of the language
	void SetLanguage( Lang a_iLangId );

	// The format of a string
	StringFormat* GetStringFormat( TINT a_iStringId );

	// The override of the current language's format of a string (s_Overrides), a_wszFallback if none, TNULL with no
	// language
	const TWCHAR* GetOverwrittenFormatString( TINT a_iStringId, const TWCHAR* a_wszFallback ) const;

	static const TWCHAR* FindOverwrittenFormatString( TINT a_iStringId, Lang a_iLangId );

	TUINT32 GetFormatUnk04() const { return m_pLocaleFormat->m_uiUnk04; }

	// Creates the locale manager with the language of the region
	static void Create();

	static void Destroy();

	static const TCHAR* GetLocaleCode( Lang a_iLangId );

	// The code of the current language ("UK", "DE"...)
	const TCHAR* GetCurLocaleCode() const;

	// The code of the voice-over language of a_iLangId (EN, NL, DE, IT or FR)
	static const TCHAR* GetVOLocaleCode( Lang a_iLangId );
	const TCHAR*        GetCurVOLocaleCode() const;

	// The audio channel of the movies in the current language (AMovieState::PlayMovie)
	TINT GetCurSoundChannel() const;
	TINT GetSoundChannel( Lang a_iLangId ) const;

	// The string overrides of s_Overrides in a_iLangId
	static const TWCHAR* GetOverwrittenString( TINT a_iStringId, Lang a_iLangId );
	static const TWCHAR* FindOverwrittenString( TINT a_iStringId, Lang a_iLangId );

	// The ids of the strings that differ between the controllers and the keyboard
	static TINT FixStringIdPlatform( TINT a_iStringId );

private:
	static void InitialiseOverrideTexts();

private:
	static Override s_Overrides[ NUM_OVERRIDES ];
	static TBOOL    s_bAreOverridesInitialized;

private:
	void*         m_pLocaleBuffer; // 0x2C
	Toshi::TTRB   m_Format;        // 0x30
	LocaleFormat* m_pLocaleFormat; // 0x44
};
