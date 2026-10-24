#pragma once
#include <GUI/T2GUIFont.h>

//-----------------------------------------------------------------------------
// Purpose: the fonts of the game's own GUI: the message, debug and heading fonts, all
// "Arial_Unicode_MS_14" on PC. A static class.
//-----------------------------------------------------------------------------
class AFontManager
{
public:
	enum FONTS
	{
		FONTS_Msg     = 0,
		FONTS_Debug   = 1,
		FONTS_Heading = 2,
	};

public:
	static void              Create();
	static void              Destroy();
	static Toshi::T2GUIFont* GetFont( FONTS a_eFont );

public:
	static Toshi::T2GUIFont* sm_pMsgFont;
	static Toshi::T2GUIFont* sm_pDebugFont;
	static Toshi::T2GUIFont* sm_pHeadingFont;
	static TBOOL             sm_bReady;
};
