#include "ToshiPCH.h"
#include "PPropertyName.h"
#include "Toshi/Strings/TWString.h"

void PPropertyName::SetName(const Toshi::TPCString& a_pName)
{
	m_pName = a_pName;
}

void PPropertyName::SetSubName(const Toshi::TPCString& a_pSubName)
{
	m_pName = a_pSubName;
}

Toshi::TPCString PPropertyName::GetString() const
{
	Toshi::TCString str = m_pName.GetPtr() ? m_pName.GetPtr()->m_oString : Toshi::TPCString::ms_sEmpty;
	auto wStr = Toshi::TWString(str);

	return Toshi::TPCString();
}

void PPropertyName::Split(const Toshi::TPCString& a_pName)
{
	Toshi::TCString str = a_pName.GetPtr() ? a_pName.GetPtr()->m_oString : Toshi::TPCString::ms_sEmpty;


	if (str.Find('[') == -1)
	{
		m_pName = a_pName;
		return;
	}
	TIMPLEMENT();
}
