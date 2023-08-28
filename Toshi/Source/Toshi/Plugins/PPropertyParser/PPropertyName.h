#pragma once

#include "Toshi/Strings/TPCString.h"

class PPropertyName
{
public:
	PPropertyName()
	{

	}

	PPropertyName(const Toshi::TPCString& a_pName, const Toshi::TPCString& a_pSubName)
	{
		m_pName = a_pName;
		m_pSubName = a_pSubName;
	}

	PPropertyName(const PPropertyName& other)
	{
		m_pName = other.m_pName;
		m_pSubName = other.m_pSubName;
	}

	TBOOL operator==(const PPropertyName& other) const
	{
		return m_pName.Compare(other.m_pName) == 0 && m_pSubName.Compare(other.m_pSubName) == 0;
	}

	void SetName(const Toshi::TPCString& a_pName);
	void SetSubName(const Toshi::TPCString& a_pName);

	Toshi::TPCString GetString() const;

	const Toshi::TPCString& GetSubName() const { return m_pSubName; }
	const Toshi::TPCString& GetName() const { return m_pName; }

protected:
	void Split(const Toshi::TPCString& a_pName);


	Toshi::TPCString m_pName;
	Toshi::TPCString m_pSubName;
};
