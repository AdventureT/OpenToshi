#include "pch.h"
#include "AEntityFactory.h"

// $deBlob: FUNCTION 00759690
void AEntityFactory::CreateEntity(const TCHAR* a_szEntityName, TINT unk)
{
	Toshi::TString8 str = "data/entities/";
	str += a_szEntityName;
	GetEntityTypeData(a_szEntityName);
	/*Toshi::TClass* t;
	t->CreateObject();*/
}

// $deBlob: FUNCTION 0075aba0
AEntityFactory::AEntityTypeData* AEntityFactory::GetEntityTypeData(const TCHAR* a_szEntityName)
{
	for (TINT i = 0; i < m_sEntityTypeDatas.Size(); i++)
	{
		if (Toshi::T2String8::CompareNoCase(m_sEntityTypeDatas[i]->m_szEntityName, a_szEntityName)) return m_sEntityTypeDatas[i];
	}
	return AddPrecachedTypeData(a_szEntityName);
}

AEntityFactory::AEntityTypeData* AEntityFactory::AddPrecachedTypeData(const TCHAR* a_szEntityName)
{
	return TNULL;
}
