#include "ToshiPCH.h"
#include "PPropertyName.h"
#include "Toshi/Strings/TWString.h"

void PPropertyName::SetName(const Toshi::TPCString& a_pName)
{
	if (m_pName.m_pPooledString != a_pName.m_pPooledString)
	{
		if (m_pName.m_pPooledString)
		{
			m_pName.m_pPooledString->m_iCount--;
			if (m_pName.m_pPooledString->m_iCount == 0)
			{
				m_pName.m_pPooledString->Delete();
			}
		}
		m_pName.m_pPooledString = a_pName.m_pPooledString;
		if (a_pName.m_pPooledString)
		{
			a_pName.m_pPooledString->m_iCount++;
		}
	}
}

void PPropertyName::SetSubName(const Toshi::TPCString& a_pSubName)
{
	if (m_pSubName.m_pPooledString != a_pSubName.m_pPooledString)
	{
		if (m_pSubName.m_pPooledString)
		{
			m_pSubName.m_pPooledString->m_iCount--;
			if (m_pSubName.m_pPooledString->m_iCount == 0)
			{
				m_pSubName.m_pPooledString->Delete();
			}
		}
		m_pSubName.m_pPooledString = a_pSubName.m_pPooledString;
		if (a_pSubName.m_pPooledString)
		{
			a_pSubName.m_pPooledString->m_iCount++;
		}
	}
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
