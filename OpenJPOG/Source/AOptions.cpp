#include "pch.h"
#include "AOptions.h"

TOSHI_NAMESPACE_USING

AOptions::Result AOptions::LoadOptions()
{
	LoadOptions(-1, -1, "", sm_szOptionsName);
	return Result();
}

TBOOL AOptions::GetOption(const char* a_szOptionName, int& a_rVal)
{
	TPCString pcString = Toshi::TSystem::GetCStringPool()->Get(a_szOptionName);

	return TBOOL();
}

AOptions::Result AOptions::LoadOptions(int a_iUnk, int a_iUnk2, const Toshi::TCString& a_szOptionsDir, const Toshi::TCString& a_szOptionsName)
{
	PPropertyReader reader = PPropertyReader();
	TCString szFileName;
	szFileName.Format("%s%s.ini", a_szOptionsDir.GetString(), a_szOptionsName.GetString());
	if (!reader.Open(szFileName))
	{
		return RESULT_ERROR;
	}
	PProperties* props = new PProperties();
	if (reader.LoadPropertyBlock(*props))
	{
		if (m_pUnkProps)
		{
			delete m_pUnkProps;
		}
		m_pUnkProps = props;
		m_pCurProps = props;
		return RESULT_OK;
	}
	return RESULT_ERROR;
}
