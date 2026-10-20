/**
 * @file T2Locale.h
 * Localization system for the Toshi engine
 * 
 * This file provides localization functionality including:
 * - Language and region management
 * - String localization
 * - Culture-specific formatting
 * - Localization resource handling
 */

#pragma once
#include "Toshi/TSingleton.h"
#include "File/TTRB.h"

TOSHI_NAMESPACE_START

using T2LocalisedString = TWCHAR*;

class T2Locale : public Toshi::TSingleton<T2Locale>
{
public:
	using Lang = TINT32;

	struct LocaleStrings
	{
		TINT32             m_numstrings;
		T2LocalisedString* Strings;
	};

	// NOTE: de Blob's T2Locale::GetString calls two game functions directly, here the game installs them (TNULL: no
	// remapping/overrides)
	using t_FixStringIdPlatform        = TINT ( * )( TINT a_iStringId );
	using t_GetOverwrittenLocaleString = T2LocalisedString ( * )( TINT a_iStringId, Lang a_iLang );

public:
	T2Locale( const T2Locale& )  = delete;
	T2Locale( const T2Locale&& ) = delete;

	virtual ~T2Locale();
	virtual const TCHAR* GetLanguageFilename( Lang lang ) = 0;

	void SetLanguage( Lang langid );
	Lang GetLanguage() const;

	TINT              GetNumStrings() const;
	void*             GetBuffer() const { return m_pBuffer; }
	T2LocalisedString GetString( TINT a_iNumString );
	TWCHAR*           GetString( TWCHAR* a_pDst, TINT a_iDstSize, TINT a_iNumString );

	static void SetGameHooks( t_FixStringIdPlatform a_fnFixStringId, t_GetOverwrittenLocaleString a_fnGetOverwrittenString )
	{
		ms_fnFixStringIdPlatform        = a_fnFixStringId;
		ms_fnGetOverwrittenLocaleString = a_fnGetOverwrittenString;
	}

protected:
	T2Locale( TINT langCount, size_t bufferSize, void* buffer );

	static void* __stdcall TRBAllocator( TTRB::AllocType alloctype, size_t size, short alignment, size_t unk2, void* userData );
	static void __stdcall TRBDeallocator( TTRB::AllocType alloctype, void* ptr, short alignment, size_t unk2, void* userData );

	// $deBlob: FUNCTION 00662fb0
	void* TRBAlloc( size_t size )
	{
		void* ptr   = m_BufferPos;
		m_BufferPos = static_cast<TCHAR*>( ptr ) + size;
		return ptr;
	}

	TINT              FixStringId( TINT a_iStringId ) const { return ms_fnFixStringIdPlatform ? ms_fnFixStringIdPlatform( a_iStringId ) : a_iStringId; }
	T2LocalisedString GetOverwrittenString( TINT a_iStringId ) const { return ms_fnGetOverwrittenLocaleString ? ms_fnGetOverwrittenLocaleString( a_iStringId, m_LangId ) : TNULL; }

protected:
	static t_FixStringIdPlatform        ms_fnFixStringIdPlatform;
	static t_GetOverwrittenLocaleString ms_fnGetOverwrittenLocaleString;

	TTRB           m_Locale;      // 0x04
	void*          m_pBuffer;     // 0x18
	void*          m_BufferPos;   // 0x1C
	LocaleStrings* m_stringtable; // 0x20
	Lang           m_LangId;      // 0x24
	TBOOL          m_bOwnsBuffer; // 0x28
};

TOSHI_NAMESPACE_END
