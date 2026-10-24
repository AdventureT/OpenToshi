#include "ToshiPCH.h"
#include "T2Locale.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

T2Locale::t_FixStringIdPlatform        T2Locale::ms_fnFixStringIdPlatform        = TNULL;
T2Locale::t_GetOverwrittenLocaleString T2Locale::ms_fnGetOverwrittenLocaleString = TNULL;

// $deBlob: FUNCTION 00662d40
// $Barnyard: FUNCTION 006be7f0
T2Locale::T2Locale( TINT langCount, TSIZE bufferSize, void* buffer )
{
	m_pBuffer     = buffer;
	m_BufferPos   = buffer;
	m_stringtable = TNULL;
	m_LangId      = -1;
	m_bOwnsBuffer = TFALSE;

	// NOTE: retail checks its own s_singleton here (TSingleton does the same check)

	if ( m_pBuffer == TNULL )
	{
		TSIZE symbolTableSize = TTRB::GetSymbolTableSize( 1 );
		TSIZE headerSize      = TTRB::GetHeaderSize( 1 );
		m_pBuffer             = TMalloc( headerSize + bufferSize + symbolTableSize );
		m_BufferPos           = m_pBuffer;
		m_bOwnsBuffer         = TTRUE;
	}

	m_Locale.SetMemoryFunctions( TRBAllocator, TRBDeallocator, this );
}

// $deBlob: FUNCTION 00662df0
// $deBlob: FUNCTION 00663000
// $Barnyard: FUNCTION 006be710
// $Barnyard: FUNCTION 006be890
T2Locale::~T2Locale()
{
	m_Locale.Close();

	if ( m_bOwnsBuffer )
	{
		TFree( m_pBuffer );
	}
}

// $deBlob: FUNCTION 00662fd0
// $Barnyard: FUNCTION 006be7d0
void* __stdcall T2Locale::TRBAllocator( TTRB::AllocType alloctype, TSIZE size, short alignment, TSIZE unk2, void* userData )
{
	return TSTATICCAST( T2Locale, userData )->TRBAlloc( size );
}

// $deBlob: FUNCTION 00662ff0
// $Barnyard: FUNCTION 00539d40
void __stdcall T2Locale::TRBDeallocator( TTRB::AllocType alloctype, void* ptr, short alignment, TSIZE unk2, void* userData )
{
	// T2Locale doesn't have deallocator
}

// $deBlob: FUNCTION 00662e30
// $Barnyard: FUNCTION 006be750
void T2Locale::SetLanguage( Lang langid )
{
	if ( langid != m_LangId && ( m_Locale.Close(), langid >= 0 ) )
	{
		const TCHAR* szFileName = GetLanguageFilename( langid );

		// NOTE: de Blob rewinds the buffer before loading (Barnyard after)
		m_BufferPos = m_pBuffer;
		m_Locale.Load( szFileName );
		m_stringtable = m_Locale.CastSymbol<LocaleStrings>( "LocaleStrings" );
		m_LangId      = langid;
	}
}

Toshi::T2Locale::Lang T2Locale::GetLanguage() const
{
	return m_LangId;
}

// $deBlob: FUNCTION 00662fa0
TINT T2Locale::GetNumStrings() const
{
	return m_stringtable->m_numstrings;
}

// $deBlob: FUNCTION 00662e80
// $Barnyard: FUNCTION 006be7b0
T2LocalisedString T2Locale::GetString( TINT a_iNumString )
{
	TINT stringid = a_iNumString;
	TASSERT( stringid >= 0 && stringid < m_stringtable->m_numstrings );

	stringid = FixStringId( stringid );

	T2LocalisedString pString = GetOverwrittenString( stringid );
	if ( pString == TNULL ) pString = m_stringtable->Strings[ stringid ];

	return pString;
}

// $deBlob: FUNCTION 00662ef0
TWCHAR* T2Locale::GetString( TWCHAR* a_pDst, TINT a_iDstSize, TINT a_iNumString )
{
	TINT stringid = a_iNumString;
	TASSERT( stringid >= 0 && stringid < m_stringtable->m_numstrings );

	stringid = FixStringId( stringid );

	T2LocalisedString pString = GetOverwrittenString( stringid );
	if ( pString == TNULL ) pString = m_stringtable->Strings[ stringid ];

	if ( pString == TNULL )
	{
		*a_pDst = L'\0';
		return TNULL;
	}

	T2String16::CopySafe( a_pDst, pString, a_iDstSize );
	return a_pDst;
}

TOSHI_NAMESPACE_END
