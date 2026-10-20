#include "pch.h"
#include "AFontManager.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

T2GUIFont* AFontManager::sm_pMsgFont     = TNULL;
T2GUIFont* AFontManager::sm_pDebugFont   = TNULL;
T2GUIFont* AFontManager::sm_pHeadingFont = TNULL;
TBOOL      AFontManager::sm_bReady       = TFALSE;

// $deBlob: FUNCTION 005d5ff0
void AFontManager::Create()
{
	TASSERT( TNULL == sm_pMsgFont );
	TASSERT( TNULL == sm_pDebugFont );
	TASSERT( TNULL == sm_pHeadingFont );

	sm_pMsgFont     = T2GUIFontManager::GetFont( "Arial_Unicode_MS_14" );
	sm_pDebugFont   = T2GUIFontManager::GetFont( "Arial_Unicode_MS_14" );
	sm_pHeadingFont = T2GUIFontManager::GetFont( "Arial_Unicode_MS_14" );
	sm_bReady       = TTRUE;
}

// $deBlob: FUNCTION 005d60d0
void AFontManager::Destroy()
{
	T2GUIFontManager::ReleaseFont( sm_pMsgFont );
	sm_pMsgFont = TNULL;
	T2GUIFontManager::ReleaseFont( sm_pDebugFont );
	sm_pDebugFont = TNULL;
	T2GUIFontManager::ReleaseFont( sm_pHeadingFont );
	sm_pHeadingFont = TNULL;
	sm_bReady       = TFALSE;
}

// $deBlob: FUNCTION 005d6120
T2GUIFont* AFontManager::GetFont( FONTS a_eFont )
{
	switch ( a_eFont )
	{
		case FONTS_Msg:
			return sm_pMsgFont;
		case FONTS_Debug:
			return sm_pDebugFont;
		case FONTS_Heading:
			return sm_pHeadingFont;
	}

	TASSERT( TFALSE, "AFontManager::GetFont: Trying to retrieve invalid font\n" );
	return TNULL;
}
