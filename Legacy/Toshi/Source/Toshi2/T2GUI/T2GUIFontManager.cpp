#include "ToshiPCH.h"
#include "T2GUIFontManager.h"

TBOOL Toshi::T2GUIFontDef2::Init()
{
	FindFontRef(m_font);
	return TFALSE;
}

TINT* Toshi::FindFontRef(const TCHAR* a_font)
{
	return TNULL;
}

TINT* Toshi::FindFreeFontRef()
{
	return TNULL;
}
