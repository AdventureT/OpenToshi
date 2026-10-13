#include "ToshiPCH.h"
#include "T2ResourceManager.h"
#include "Toshi/Render/TAssetInit.h"
#include "Toshi/Core/TError.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006fab00
void T2Resource::CreateResource(const TCHAR* resourceName, void* pData, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* pCallbkData)
{
	if (m_iID == T2ResourcePtr::IDINVALID)
	{
		m_iID = T2ResourceManager::GetSingletonSafe()->CreateResource(resourceName, pData, a_fnCreateDestroyCallbk, pCallbkData);
		return;
	}

	TASSERT(TFALSE, "resource already created!");
}

// $deBlob: FUNCTION 006f9af0
void T2ResourceData::DeInit()
{
	TASSERT(HasFlag(FLAG_INITIALISED));
	m_iFlags &= ~FLAG_INITIALISED;

	Unload();
	TASSERT(m_pData == TNULL);

	m_fnCreateDestroyCallbk = TNULL;
	m_iFlags                = 0;

	if (m_iRefCount == 0)
	{
		// Resource is not used by anything, so clear it's name
		m_pResourceName[0] = '\0';
	}
	else
	{
		// Set the resource to be destroyed but don't clear the name since it's still used by something
		m_iFlags |= FLAG_DESTROYED;
	}
}

// $deBlob: FUNCTION 006f9cc0
void T2ResourceData::SetLoadedData(void* a_pData)
{
	Unload();
	m_pData = a_pData;
	m_iFlags |= FLAG_LOADED;
	m_iFlags &= ~FLAG_LOADING;
}

// $deBlob: FUNCTION 006f9c00
void T2ResourceData::Load(const TCHAR* filepath)
{
	if (HASFLAG(m_iFlags & (FLAG_LOADED | FLAG_LOADING))) return;

	m_pTRB2            = new TTRB;
	TTRB::ERROR iError = m_pTRB2->Load(filepath);

	if (iError == TTRB::ERROR_OK)
	{
		TASSERT(TNULL != m_fnCreateDestroyCallbk);
		m_pData = m_fnCreateDestroyCallbk(m_pCreateDestroyCallbkData, m_pTRB2, TTRUE);

		if (m_pData != TNULL)
		{
			m_iFlags |= FLAG_LOADED;
			m_iFlags &= ~FLAG_LOADING;
			return;
		}
	}

	m_pTRB2->Close();
	delete m_pTRB2;
	m_pTRB2 = TNULL;
}

// $deBlob: FUNCTION 006f9ce0
void T2ResourceData::Unload()
{
	if (HASFLAG(m_iFlags & FLAG_LOADED))
	{
		m_iFlags &= ~(FLAG_LOADED | FLAG_LOADING);

		if (m_pTRB2 != TNULL)
		{
			TASSERT(TNULL != m_fnCreateDestroyCallbk);

			m_fnCreateDestroyCallbk(m_pCreateDestroyCallbkData, m_pTRB2, TFALSE);

			m_pTRB2->Close();
			delete m_pTRB2;
			m_pTRB2 = TNULL;
		}

		m_pData = TNULL;
	}
}

// $deBlob: FUNCTION 006f9b90
void* T2ResourceData::GetData()
{
	if (!HasFlag(FLAG_LOADED))
	{
#ifdef TOSHI_DEBUG

		// Check if the resource is being loaded
		TASSERT(HasFlag(FLAG_LOADING), "Resource is not loaded and it's not in progress of being loaded");

#else // TOSHI_DEBUG

		// If the resource is not being loaded, return TNULL instead of waiting
		if (!HasFlag(FLAG_LOADING)) return TNULL;

#endif // TOSHI_DEBUG

		// Wait for the resource to be loaded
		while (!HasFlag(FLAG_LOADED)) Sleep(100);
	}

	return m_pData;
}

// $deBlob: FUNCTION 006fa950
void T2ResourceManager::IncRefCount(TINT a_iID)
{
	if (a_iID != T2ResourcePtr::IDINVALID)
	{
		TASSERT(a_iID >= 0);
		TASSERT(a_iID < m_iMaxNumResources);

		m_pData[a_iID].m_iRefCount++;
		TASSERT(m_pData[a_iID].m_iRefCount > 0);
	}
}

// $deBlob: FUNCTION 006faa20
void T2ResourceManager::DecRefCount(TINT a_iID)
{
	if (a_iID != T2ResourcePtr::IDINVALID)
	{
		TASSERT(a_iID >= 0);
		TASSERT(a_iID < m_iMaxNumResources);

		auto pData = &m_pData[a_iID];

		pData->m_iRefCount--;
		TASSERT(pData->m_iRefCount >= 0);

		if (pData->m_iRefCount == 0 && pData->HasAnyFlag(T2ResourceData::FLAG_DESTROYED))
		{
			pData->m_iFlags           = ~T2ResourceData::FLAG_DESTROYED;
			pData->m_pResourceName[0] = '\0';
			m_iNumUsedResources--;
		}
	}
}

// $deBlob: FUNCTION 006f9910
void T2ResourceData::Init(const TCHAR* a_pName, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pCallbkData)
{
	TASSERT(m_fnCreateDestroyCallbk == TNULL);
	TASSERT(m_pResourceName[0] == '\0');
	TASSERT(m_pData == TNULL);
	TASSERT(m_iFlags == 0);
	TASSERT(m_iRefCount == 0);
	TASSERT(a_fnCreateDestroyCallbk != TNULL);
	TASSERT(T2String8::IsLowerCase(a_pName));
	TASSERT(T2String8::Length(a_pName) < MAX_RESOURCE_NAME);

	T2String8::CopySafe(m_pResourceName, a_pName, MAX_RESOURCE_NAME);

	m_fnCreateDestroyCallbk    = a_fnCreateDestroyCallbk;
	m_pCreateDestroyCallbkData = a_pCallbkData;
	m_iFlags |= FLAG_INITIALISED;
}

// $deBlob: FUNCTION 006f9e40
T2ResourceManager::T2ResourceManager(TINT a_iMaxNumResources)
{
	m_iMaxNumResources  = a_iMaxNumResources;
	m_iNumUsedResources = 0;
	m_iUnk              = 0;

	m_pData = new T2ResourceData[m_iMaxNumResources];
	TUtil::MemClear(m_pData, m_iMaxNumResources * sizeof(T2ResourceData));

	// Initialize the dummy (invalid) resource
	m_pData[0].m_iFlags |= T2ResourceData::FLAG_INITIALISED;
}

// $deBlob: FUNCTION 006f9ed0
T2ResourceManager::~T2ResourceManager()
{
	delete[] m_pData;
}

// $deBlob: FUNCTION 006fa2e0
T2ResourceData* T2ResourceManager::GetResourceData(TINT a_iID)
{
	TASSERT(a_iID != T2ResourcePtr::IDINVALID, "Invalid ID");
	TASSERT(a_iID >= 0, "ID must be greater than 0");
	TASSERT(a_iID < m_iMaxNumResources);
	TASSERT(TFALSE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_DESTROYED));
	TASSERT(TTRUE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_INITIALISED));

	return &m_pData[a_iID];
}

// $deBlob: FUNCTION 006fa840
void* T2ResourceManager::GetData(TINT a_iID)
{
	TASSERT(a_iID >= 0, "ID must be greater than 0");
	TASSERT(a_iID < m_iMaxNumResources);
	TASSERT(TFALSE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_DESTROYED));
	TASSERT(TTRUE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_INITIALISED));

	return a_iID == T2ResourcePtr::IDINVALID ? TNULL : m_pData[a_iID].GetData();
}

// $deBlob: FUNCTION 006fa5c0
TINT T2ResourceManager::CreateResource(const TCHAR* resourceName, void* pData, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* pCallbkData)
{
	TINT iID = FindUnusedResource();

	if (m_iMaxNumResources <= iID)
	{
		TError::GetSingleton()->AddError("couldnt create resource \"%s\" because we have reached our max (%d)", resourceName, m_iMaxNumResources);
		return 0;
	}

	TASSERT(iID >= 0);
	TASSERT(iID < m_iMaxNumResources);
	TASSERT(TFALSE == m_pData[iID].HasFlag(T2ResourceData::FLAG_INITIALISED));

	m_iNumUsedResources++;
	m_pData[iID].Init(resourceName, a_fnCreateDestroyCallbk, pCallbkData);

	/*
		* Resources might also be loaded without passing pData argument but this feature
		* is deprecated in the Toshi build used in de Blob (at least for Windows).
		* Use T2ResourceData::Load instead of T2ResourceData::SetLoadedData to use the feature.
		* The Create callback is only called from T2ResourceData::Load
		*/

	m_pData[iID].SetLoadedData(pData);
	m_pData[iID].m_pTRB1 = TAssetInit::g_pCurrentTRB;

	return iID;
}

// $deBlob: FUNCTION 006fa6e0
void T2ResourceManager::DestroyResource(TINT a_iID)
{
	if (a_iID != T2ResourcePtr::IDINVALID)
	{
		TASSERT(a_iID >= 0);
		TASSERT(a_iID < m_iMaxNumResources);

		m_pData[a_iID].DeInit();

		if (m_pData[a_iID].m_iRefCount == 0)
		{
			m_iNumUsedResources--;
		}
	}
}

// $deBlob: FUNCTION 006fa0f0
TBOOL T2ResourceManager::ReloadResource(const TCHAR* pName, const TCHAR* pFilepath)
{
	TINT iID;
	FindResource(iID, pName);

	if (iID != T2ResourcePtr::IDINVALID)
	{
		TASSERT(iID >= 0);
		TASSERT(iID < m_iMaxNumResources);

		m_pData[iID].Unload();
		m_pData[iID].Load(pFilepath);
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006f9fc0
T2ResourcePtr T2ResourceManager::FindResource(TINT& iOutResource, const TCHAR* pName)
{
	TASSERT(T2String8::IsLowerCase(pName));

	T2ResourceData* pResData = m_pData;

	while (!pResData->HasAnyFlag(T2ResourceData::FLAG_INITIALISED) || TStringManager::String8CompareNoCase(pResData->GetName(), pName) != 0)
	{
		if (pResData >= m_pData + m_iMaxNumResources)
		{
			iOutResource = 0;
			return T2ResourcePtr(0);
		}
	}

	TINT iID = pResData - m_pData;

	TASSERT(iID >= 0);
	TASSERT(iID < m_iMaxNumResources);

	iOutResource = iID;
	return T2ResourcePtr(iID);
}

// $deBlob: FUNCTION 006fa770
TINT T2ResourceManager::FindUnusedResource()
{
	TASSERT(m_iNumUsedResources < m_iMaxNumResources);

	for (TINT i = m_iUnk; i < m_iMaxNumResources; i++)
	{
		// Skip resource if it is initialized or is destroyed but something has some references to it
		if (!m_pData[i].HasAnyFlag(T2ResourceData::FLAG_INITIALISED | T2ResourceData::FLAG_DESTROYED) && m_pData[i].m_iRefCount == 0)
		{
			m_iUnk = i + 1;
			return i;
		}
	}

	for (TINT i = 0; i < m_iUnk; i++)
	{
		// Skip resource if it is initialized or is destroyed but something has some references to it
		if (!m_pData[i].HasAnyFlag(T2ResourceData::FLAG_INITIALISED | T2ResourceData::FLAG_DESTROYED) && m_pData[i].m_iRefCount == 0)
		{
			m_iUnk = i + 1;
			return i;
		}
	}

	TASSERT(TFALSE, "Should never get here");
	return T2ResourcePtr::IDINVALID;
}

TOSHI_NAMESPACE_END
