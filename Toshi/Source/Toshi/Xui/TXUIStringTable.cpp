#include "ToshiPCH.h"
#include "TXUIStringTable.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006eb0f0
TXUIStringTable::~TXUIStringTable()
{
	for (size_t i = 0; i < m_uiCountOfStrings; i++)
	{
	}
}

TXUIStringTable::Str* TXUIStringTable::Lookup(const TWCHAR* a_str)
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

TOSHI_NAMESPACE_END
