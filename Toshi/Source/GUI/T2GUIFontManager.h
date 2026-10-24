#pragma once
#include "T2GUIFont.h"
#include "File/TTRB.h"

#ifdef CreateFont
#  undef CreateFont
#endif

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// de Blob's T2GUIFontManager: the loaded fonts by name. A font comes from a font TRB loaded by LoadFontTRB (every symbol
// is a T2GUIFontDef) or from a "tfnt" symbol (T2GUIFontDef2, created by TAssetInit through CreateFont).
//-----------------------------------------------------------------------------
class T2GUIFontManager
{
public:
	// A loaded font TRB (0x1C)
	struct TRBRef
	{
		TRBRef();

		// $deBlob: FUNCTION 006f5b10
		~TRBRef() = default;

		TCHAR* m_szName;    // 0x00
		TTRB   m_oTRB;      // 0x04
		TINT   m_iRefCount; // 0x18
	};

	// A font (0x10)
	struct FontRef
	{
		// $deBlob: FUNCTION 006f5ad0
		FontRef()
		    : m_szName( TNULL ), m_pFont( TNULL ), m_pTRBRef( TNULL ), m_iRefCount( 0 )
		{}

		// $deBlob: FUNCTION 006f5b50
		T2GUIFont* GetFont() const { return m_pFont; }

		TCHAR*     m_szName;    // 0x00
		T2GUIFont* m_pFont;     // 0x04
		TRBRef*    m_pTRBRef;   // 0x08 the TRB of the font, TNULL for a T2GUIFontDef2
		TINT       m_iRefCount; // 0x0C
	};

	static TCONSTEXPR_VAR TINT MAX_TRBREFS  = 1;
	static TCONSTEXPR_VAR TINT MAX_FONTREFS = 16;

public:
	static TBOOL Open();
	static void  Close();

	static TBOOL LoadFontTRB( const TCHAR* a_szFileName );
	static void  UnloadFontTRB( const TCHAR* a_szFileName );

	// Returns the font of that name and adds a reference to it
	static T2GUIFont* GetFont( const TCHAR* a_szFontName );
	static void       ReleaseFont( T2GUIFont* a_pFont );

	static TBOOL CreateFont( T2GUIFontDef2* a_pFontDef );
	static TBOOL DestroyFont( T2GUIFontDef2* a_pFontDef );

private:
	static TRBRef*  FindTRBRef( const TCHAR* a_szFileName );
	static FontRef* FindFontRef( T2GUIFont* a_pFont );
	static FontRef* FindFontRef( const TCHAR* a_szFontName );
	static FontRef* FindFreeFontRef();
	static TRBRef*  FindFreeTRBRef();
	static void     LoadTRBFonts( TRBRef* a_pTRBRef );
	static TBOOL    UnloadTRBFonts( TRBRef* a_pTRBRef );

private:
	static TRBRef  s_trbrefs[ MAX_TRBREFS ];
	static FontRef s_fontrefs[ MAX_FONTREFS ];
};

TOSHI_NAMESPACE_END
