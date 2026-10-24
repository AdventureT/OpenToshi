#pragma once
#include "Toshi/File/TTRB.h"
#include "Toshi/Thread/TThread.h"
#include "Toshi/Utils/TUtil.h"

#include <cstdint>

#ifdef FindResource
#  undef FindResource
#endif

TOSHI_NAMESPACE_START

class T2ResourceData;
class T2ResourcePtr;

class T2ResourceManager : public TSingleton<T2ResourceManager>
{
public:
	using t_CreateDestroyCallbk = void* (*)(void* pCustomData, TTRB* trb, TBOOL bCreated);

public:
	T2ResourceManager(TINT a_iMaxNumResources);
	~T2ResourceManager();

	TINT CreateResource(const TCHAR* resourceName, void* pData, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* pCallbkData);
	void DestroyResource(TINT a_iID);

	TBOOL ReloadResource(const TCHAR* pName, const TCHAR* pFilepath);

	void IncRefCount(TINT a_iID);
	void DecRefCount(TINT a_iID);

	T2ResourcePtr FindResource(TINT& iOutResource, const TCHAR* pName);
	TINT          FindUnusedResource();

	T2ResourceData* GetResourceData(TINT a_iID);
	void*           GetData(TINT a_iID);

	TINT GetMaxNumResources() const { return m_iMaxNumResources; }
	TINT GetNumUsedResources() const { return m_iNumUsedResources; }

private:
	TINT            m_iMaxNumResources;
	TINT            m_iNumUsedResources;
	TINT            m_iUnk;
	T2ResourceData* m_pData;
};

class T2ResourceData
{
public:
	static constexpr TINT MAX_RESOURCE_NAME = 48;

	friend class T2ResourceManager;

public:
	enum FLAG : TUINT8
	{
		FLAG_LOADED      = BITFIELD(0),
		FLAG_LOADING     = BITFIELD(1),
		FLAG_INITIALISED = BITFIELD(2),
		FLAG_DESTROYED   = BITFIELD(3),
	};

	void Init(const TCHAR* a_pName, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pCallbkData);
	void DeInit();

	void Load(const TCHAR* filepath);
	void Unload();

	void  SetLoadedData(void* a_pData);
	void* GetData();

	const TCHAR* GetName() const { return m_pResourceName; }

	TBOOL HasFlag(TUINT8 flag) const { return (m_iFlags & flag) == flag; }

	TBOOL HasAnyFlag(TUINT8 flag) const { return (m_iFlags & flag) != 0; }

private:
	void*                                    m_pData;
	TTRB*                                    m_pTRB1;
	TINT                                     m_iFlags;
	TINT                                     m_iRefCount;
	TCHAR                                    m_pResourceName[48];
	void*                                    m_pCreateDestroyCallbkData;
	T2ResourceManager::t_CreateDestroyCallbk m_fnCreateDestroyCallbk;
	TTRB*                                    m_pTRB2;
};

class T2ResourcePtr
{
public:
	static constexpr TINT IDINVALID = 0;

public:
	T2ResourcePtr()
	{
		m_iResourceID = IDINVALID;
		T2ResourceManager::GetSingleton()->IncRefCount(IDINVALID);
	}

	T2ResourcePtr(TINT a_iID)
	{
		m_iResourceID = a_iID;
		T2ResourceManager::GetSingleton()->IncRefCount(m_iResourceID);
	}

	explicit T2ResourcePtr(const T2ResourcePtr& other)
	{
		m_iResourceID = other.m_iResourceID;
		T2ResourceManager::GetSingletonSafe()->IncRefCount(m_iResourceID);
	}

	~T2ResourcePtr() { T2ResourceManager::GetSingleton()->DecRefCount(m_iResourceID); }

	void operator=(const T2ResourcePtr& other)
	{
		T2ResourceManager::GetSingletonSafe()->DecRefCount(m_iResourceID);
		m_iResourceID = other.m_iResourceID;
		T2ResourceManager::GetSingletonSafe()->IncRefCount(m_iResourceID);
	}

	void* GetData() const { return T2ResourceManager::GetSingletonSafe()->GetData(m_iResourceID); }

	TINT GetID() const { return m_iResourceID; }

	operator TINT() const { return m_iResourceID; }

protected:
	TINT m_iResourceID;
};

template <class T> class T2ResPtr : protected T2ResourcePtr
{
public:
	T2ResPtr(TINT a_iID = IDINVALID)
	    : T2ResourcePtr(a_iID) {}
	T2ResPtr(const T2ResPtr& a_rOther)
	    : T2ResourcePtr(a_rOther.m_iResourceID) {}
	T2ResPtr(T2ResPtr&& a_rOther)
	    : T2ResourcePtr(a_rOther.m_iResourceID) { a_rOther.m_iResourceID = IDINVALID; }

	void operator=(const T2ResPtr& a_rOther)
	{
		T2ResourceManager::GetSingletonSafe()->DecRefCount(m_iResourceID);
		m_iResourceID = a_rOther.m_iResourceID;
		T2ResourceManager::GetSingletonSafe()->IncRefCount(m_iResourceID);
	}

	TINT GetID() const { return m_iResourceID; }
	operator TINT() const { return m_iResourceID; }

	T* GetData() const { return TSTATICCAST(T, T2ResourcePtr::GetData()); }
	T* operator->() const { return TSTATICCAST(T, T2ResourcePtr::GetData()); }
};

class T2Resource
{
public:
	void CreateResource(const TCHAR* resourceName, void* pData, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* pCallbkData);

	void DestroyResource()
	{
		TASSERT(m_iID != T2ResourcePtr::IDINVALID);
		T2ResourceManager::GetSingletonSafe()->DestroyResource(m_iID);
	}

	T2ResourcePtr GetResourcePtr() const { return T2ResourcePtr(m_iID); }

	template <class T> T2ResPtr<T> GetResPtr() const { return T2ResPtr<T>(m_iID); }

	TINT GetResourceId() { return m_iID; }

	operator TINT() const { return m_iID; }

private:
	TINT m_iID = 0;
};

TOSHI_NAMESPACE_END
