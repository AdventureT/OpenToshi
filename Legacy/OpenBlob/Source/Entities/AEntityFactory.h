#pragma once
#include "AStack.h"

class AEntityFactory
{

	struct AEntityTypeData
	{
		TINT   unk;
		TCHAR* m_szEntityName;
	};

	void CreateEntity(const TCHAR* a_szEntityName, TINT unk);

	AEntityTypeData* GetEntityTypeData(const TCHAR* a_szEntityName);

	AEntityTypeData* AddPrecachedTypeData(const TCHAR* a_szEntityName);

	AStack<AEntityTypeData*, 128> m_sEntityTypeDatas;
};
