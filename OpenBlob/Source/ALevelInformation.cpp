#include "pch.h"
#include "ALevelInformation.h"

Toshi::THashTable::t_ItemCompareFunc ALevelInformation::LevelNameHashCompareFunc = [](void* unk, void* unk2, TINT unk3) {
	return Toshi::TStringManager::String8Compare((TCHAR*)unk, (TCHAR*)unk2, -1) == 0;
};

// $deBlob: FUNCTION 0056dba0
void ALevelInformation::Create(const TCHAR* a_trbName)
{
	m_pData = TNULL;
	Close();
	if (Load(a_trbName) == ERROR_OK)
	{
		m_pData = reinterpret_cast<TUINT8*>(GetSymbolAddress("LevelsMain"));
	}
	MakeStringToIDHashTable();
}

// $deBlob: FUNCTION 0056dd70
void ALevelInformation::MakeStringToIDHashTable()
{
	TINT levelCount = GetLevelHeader()->m_levelCount;
	m_pHashTable.Create(levelCount, 0x80, levelCount, levelCount);
	m_pHashTable.SetItemCompareFunction(LevelNameHashCompareFunc);

	for (TINT i = 0; i < levelCount; i++)
	{
		TCHAR toAddLevelName[0x80];
		Toshi::TStringManager::String8Copy(toAddLevelName, GetLevelName(i), 0x80);
		m_pHashTable.Insert(toAddLevelName);
	}
}

const TCHAR* ALevelInformation::GetLevelName(TINT a_iLevelIndex)
{
	TASSERT(a_iLevelIndex >= 0 && a_iLevelIndex < GetLevelHeader()->m_levelCount, "Level index out of bounds");
	return GetLevelHeader()->m_pLevelProps[a_iLevelIndex].m_levelName;
}
