#pragma once
#include "T2GUIFont.h"

TOSHI_NAMESPACE_START

static TINT* FindFontRef(const TCHAR* a_font);
static TINT* FindFreeFontRef();

class T2GUIFontDef2
{
	TCHAR* m_font;

	TBOOL Init();
};

class T2GUIFontManager
{

	TBOOL Open() {}
};

TOSHI_NAMESPACE_END
