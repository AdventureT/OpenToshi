#pragma once
#include "Toshi/Utils/TRegion.h"

TOSHI_NAMESPACE_START

typedef TBOOL (*t_CanBreakBetweenChars)(TWCHAR, TWCHAR);

inline TBOOL CanBreakBetweenChars_Default(TWCHAR, TWCHAR)
{
	TIMPLEMENT();
	return TFALSE;
}

inline TBOOL CanBreakBetweenChars_Japanese(TWCHAR, TWCHAR)
{
	TIMPLEMENT();
	return TFALSE;
}

static t_CanBreakBetweenChars s_CanBreakBetweenChars = CanBreakBetweenChars_Default;

class T2GUIFont
{
	T2GUIFont() {}


	void SetRegion();
};

TOSHI_NAMESPACE_END
