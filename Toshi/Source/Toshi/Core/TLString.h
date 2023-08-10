#pragma once

// Should be TMap!
#include "Toshi2/T2Map.h"
#include <Toshi/Strings/TPCString.h>

namespace Toshi
{

	class TLocaleInfo
	{

	};

	class TLocalisedString
	{

	};

	class TLocale
	{
		T2Map<TPCString, TLocalisedString*> m_oNameMap;             // 0x10
		T2Map<TLocalisedString, Toshi::TComparator<int>> m_oNumMap; // 0x20
		T2Map<TPCString, TLocaleInfo*> m_oAvailLocaleInfoMap;       // 0x30
		TLocaleInfo* m_pCurLocaleInfo;                              // 0x48
	};
}


