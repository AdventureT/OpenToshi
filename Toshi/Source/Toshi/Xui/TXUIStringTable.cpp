#include "ToshiPCH.h"
#include "TXUIStringTable.h"

namespace Toshi
{
	TXUIStringTable::~TXUIStringTable()
	{
		for (size_t i = 0; i < m_uiCountOfStrings; i++)
		{

		}
	}

	TXUIStringTable::Str* TXUIStringTable::Lookup(const wchar_t* a_str)
	{
		size_t i;
		for (i = 0; i < m_uiCountOfStrings; i++)
		{
			if (TStringManager::String16Compare(a_str, m_aStringTable[i]->string, -1) == 0)
			{
				break;
			}
		}

		return m_aStringTable[i];
	}
}