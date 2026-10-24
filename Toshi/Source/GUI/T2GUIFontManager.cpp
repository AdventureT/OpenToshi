#include "ToshiPCH.h"
#include "T2GUIFontManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0053f9d0
// $deBlob: FUNCTION 008594c0
T2GUIFontManager::TRBRef T2GUIFontManager::s_trbrefs[ MAX_TRBREFS ];
// $deBlob: FUNCTION 0053f990
T2GUIFontManager::FontRef T2GUIFontManager::s_fontrefs[ MAX_FONTREFS ];

static TCHAR* DuplicateString( const TCHAR* a_szString )
{
	TSIZE  uiSize = TStringManager::String8Length( a_szString ) + 1;
	TCHAR* szCopy = TSTATICCAST( TCHAR, TMalloc( uiSize ) );
	TUtil::MemCopy( szCopy, a_szString, uiSize );
	return szCopy;
}

// $deBlob: FUNCTION 006f54d0
TBOOL T2GUIFontManager::Open()
{
	T2GUIFont::SetRegion();
	return TTRUE;
}

// $deBlob: FUNCTION 006f54e0
void T2GUIFontManager::Close()
{
}

// $deBlob: FUNCTION 006f54f0
TBOOL T2GUIFontManager::LoadFontTRB( const TCHAR* a_szFileName )
{
	TRBRef* pTRBRef = FindTRBRef( a_szFileName );

	if ( pTRBRef != TNULL )
	{
		pTRBRef->m_iRefCount++;
		return TTRUE;
	}

	pTRBRef = FindFreeTRBRef();

	if ( pTRBRef == TNULL )
		return TFALSE;

	if ( pTRBRef->m_oTRB.Load( a_szFileName ) != TTRB::ERROR_OK )
		return TFALSE;

	pTRBRef->m_szName = DuplicateString( a_szFileName );
	pTRBRef->m_iRefCount++;
	LoadTRBFonts( pTRBRef );

	return TTRUE;
}

// $deBlob: FUNCTION 006f55a0
void T2GUIFontManager::UnloadFontTRB( const TCHAR* a_szFileName )
{
	TRBRef* pTRBRef = FindTRBRef( a_szFileName );

	if ( pTRBRef != TNULL && --pTRBRef->m_iRefCount == 0 )
	{
		if ( !UnloadTRBFonts( pTRBRef ) )
			TASSERT( 0 );

		pTRBRef->m_oTRB.Close();
		TFree( pTRBRef->m_szName );
	}
}

// $deBlob: FUNCTION 006f5610
T2GUIFont* T2GUIFontManager::GetFont( const TCHAR* a_szFontName )
{
	FontRef* pFontRef = FindFontRef( a_szFontName );

	if ( pFontRef != TNULL )
	{
		pFontRef->m_iRefCount++;
		return pFontRef->m_pFont;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006f5630
void T2GUIFontManager::ReleaseFont( T2GUIFont* a_pFont )
{
	if ( a_pFont != TNULL )
	{
		// NOTE: retail doesn't check the ref (an unknown font writes through TNULL)
		FontRef* pFontRef = FindFontRef( a_pFont );
		pFontRef->m_iRefCount--;
		TASSERT( pFontRef->m_iRefCount >= 0 );
	}
}

// $deBlob: FUNCTION 006f5680
TBOOL T2GUIFontManager::CreateFont( T2GUIFontDef2* a_pFontDef )
{
	if ( FindFontRef( a_pFontDef->GetName() ) != TNULL )
		return TFALSE;

	FontRef* pFontRef = FindFreeFontRef();

	if ( pFontRef == TNULL )
		return TFALSE;

	pFontRef->m_szName = DuplicateString( a_pFontDef->GetName() );
	pFontRef->m_pFont  = new T2GUIFont();
	pFontRef->m_pFont->Create( a_pFontDef );
	pFontRef->m_pTRBRef = TNULL;

	TASSERT( pFontRef->m_iRefCount == 0 );
	pFontRef->m_iRefCount++;

	return TTRUE;
}

// $deBlob: FUNCTION 006f5780
TBOOL T2GUIFontManager::DestroyFont( T2GUIFontDef2* a_pFontDef )
{
	for ( TINT i = 0; i < MAX_FONTREFS; i++ )
	{
		FontRef& rFontRef = s_fontrefs[ i ];

		if ( rFontRef.m_szName != TNULL && TStringManager::String8Compare( rFontRef.m_szName, a_pFontDef->GetName() ) == 0 )
		{
			if ( rFontRef.m_iRefCount > 0 )
				rFontRef.m_iRefCount = 0;

			// NOTE: retail leaves m_szName and m_pFont dangling (the ref is free again: no TRB, no references)
			TFree( rFontRef.m_szName );

			if ( rFontRef.m_pFont != TNULL )
				delete rFontRef.m_pFont;

			return TTRUE;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006f5830
T2GUIFontManager::TRBRef* T2GUIFontManager::FindTRBRef( const TCHAR* a_szFileName )
{
	for ( TINT i = 0; i < MAX_TRBREFS; i++ )
	{
		if ( s_trbrefs[ i ].m_iRefCount > 0 && _stricmp( a_szFileName, s_trbrefs[ i ].m_szName ) == 0 )
			return &s_trbrefs[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 006f5890
T2GUIFontManager::TRBRef* T2GUIFontManager::FindFreeTRBRef()
{
	for ( TINT i = 0; i < MAX_TRBREFS; i++ )
	{
		if ( s_trbrefs[ i ].m_iRefCount == 0 )
			return &s_trbrefs[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 006f58c0
T2GUIFontManager::FontRef* T2GUIFontManager::FindFontRef( T2GUIFont* a_pFont )
{
	for ( TINT i = 0; i < MAX_FONTREFS; i++ )
	{
		if ( ( s_fontrefs[ i ].m_pTRBRef != TNULL || s_fontrefs[ i ].m_iRefCount > 0 ) && s_fontrefs[ i ].m_pFont == a_pFont )
			return &s_fontrefs[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 006f5900
T2GUIFontManager::FontRef* T2GUIFontManager::FindFontRef( const TCHAR* a_szFontName )
{
	for ( TINT i = 0; i < MAX_FONTREFS; i++ )
	{
		if ( s_fontrefs[ i ].m_pTRBRef != TNULL || s_fontrefs[ i ].m_iRefCount > 0 )
		{
			// TNULL: the first font
			if ( a_szFontName == TNULL || _stricmp( a_szFontName, s_fontrefs[ i ].m_szName ) == 0 )
				return &s_fontrefs[ i ];
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 006f5960
T2GUIFontManager::FontRef* T2GUIFontManager::FindFreeFontRef()
{
	for ( TINT i = 0; i < MAX_FONTREFS; i++ )
	{
		if ( s_fontrefs[ i ].m_pTRBRef == TNULL && s_fontrefs[ i ].m_iRefCount == 0 )
			return &s_fontrefs[ i ];
	}

	return TNULL;
}

// Creates a font for every symbol of the TRB
// $deBlob: FUNCTION 006f5990
void T2GUIFontManager::LoadTRBFonts( TRBRef* a_pTRBRef )
{
	TTRB* pTRB  = &a_pTRBRef->m_oTRB;
	TINT  iNext = 0;

	for ( TTRB::TTRBSymbol* pSymbol = pTRB->GetSymbol( iNext ); pSymbol != TNULL; pSymbol = pTRB->GetSymbol( iNext ) )
	{
		iNext++;
		const TCHAR* szSymbolName = pTRB->GetSymbolName( pSymbol );

		if ( FindFontRef( szSymbolName ) == TNULL )
		{
			FontRef* pFontRef = FindFreeFontRef();

			if ( pFontRef == TNULL )
				return;

			pFontRef->m_szName = DuplicateString( szSymbolName );
			pFontRef->m_pFont  = new T2GUIFont();
			pFontRef->m_pFont->Create( TSTATICCAST( T2GUIFontDef, pTRB->GetSymbolAddress( pFontRef->m_szName ) ) );
			pFontRef->m_pTRBRef = a_pTRBRef;
		}
	}
}

// $deBlob: FUNCTION 006f5a70
TBOOL T2GUIFontManager::UnloadTRBFonts( TRBRef* a_pTRBRef )
{
	for ( TINT i = 0; i < MAX_FONTREFS; i++ )
	{
		FontRef& rFontRef = s_fontrefs[ i ];

		if ( rFontRef.m_pTRBRef == a_pTRBRef )
		{
			if ( rFontRef.m_iRefCount > 0 )
				rFontRef.m_iRefCount = 0;

			TFree( rFontRef.m_szName );

			if ( rFontRef.m_pFont != TNULL )
				delete rFontRef.m_pFont;

			rFontRef.m_pTRBRef = TNULL;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006f5af0
T2GUIFontManager::TRBRef::TRBRef()
    : m_szName( TNULL ), m_iRefCount( 0 )
{
}

TOSHI_NAMESPACE_END
