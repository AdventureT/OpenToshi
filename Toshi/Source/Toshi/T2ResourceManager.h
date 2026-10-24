#pragma once
#include "File/TTRB.h"

#ifdef FindResource
#  undef FindResource
#endif

TOSHI_NAMESPACE_START

class T2ResourceData;
class T2ResourcePtr;
class T2Resource;
class T2ResourceManagerState;

//-----------------------------------------------------------------------------
// Purpose: a fixed pool of named resources referenced by id (Source\Toshi2\T2ResourceManager.cpp).
// Id 0 is the invalid (dummy) resource. Not a TSingleton: the class has its own s_pSingleton.
//-----------------------------------------------------------------------------
class T2ResourceManager
{
public:
	// Called with TTRUE when a resource is loaded from a TRB (returns its data), with TFALSE when it's unloaded
	using t_CreateDestroyCallbk = void*(__stdcall*)( void* a_pCallbkData, TTRB* a_pTRB, TBOOL a_bCreated );
	using t_IterateCallbk       = void( __stdcall* )( void* a_pUserData, void* a_pResourceData );

public:
	T2ResourceManager( TINT a_iMaxNumResources );
	~T2ResourceManager();

	TINT CreateResource( const TCHAR* a_szName, void* a_pData, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pCallbkData );
	void DestroyResource( TINT a_iID );

	void IterateResources( t_IterateCallbk a_fnCallbk, void* a_pUserData );

	T2ResourcePtr FindResource( const TCHAR* a_szName );
	TINT          FindUnusedResource();

	TBOOL ReloadResource( const TCHAR* a_szName, const TCHAR* a_szFilePath );
	void  ReloadResource( T2ResourcePtr a_Resource, T2Resource* a_pResource, void* a_pData );

	void IncRefCount( TINT a_iResourceID );
	void DecRefCount( TINT a_iResourceID );

	void*           GetData( TINT a_iID );
	T2ResourceData* GetResourceData( TINT a_iID );

	void  CaptureState( T2ResourceManagerState* a_State );
	TBOOL IsStateSame( T2ResourceManagerState* a_State );
	void  DestroyState( T2ResourceManagerState* a_State );

	TINT GetMaxNumResources() const { return m_iMaxNumResources; }
	TINT GetNumUsedResources() const { return m_iNumUsedResources; }

	static T2ResourceManager* GetSingleton() { return s_pSingleton; }

	static T2ResourceManager* GetSingletonSafe()
	{
		TASSERT( s_pSingleton != TNULL );
		return s_pSingleton;
	}

	static TBOOL IsSingletonCreated() { return s_pSingleton != TNULL; }

private:
	static T2ResourceManager* s_pSingleton;

private:
	TINT            m_iMaxNumResources;  // 0x00
	TINT            m_iNumUsedResources; // 0x04
	TINT            m_iNextFreeResource; // 0x08 FindUnusedResource starts here
	T2ResourceData* m_pData;             // 0x0C
};                                       // 0x10

class T2ResourceData
{
public:
	static TCONSTEXPR_VAR TINT MAX_RESOURCE_NAME = 48;

	friend class T2ResourceManager;
	friend class T2ResourceManagerState;

public:
	enum FLAG : TUINT8
	{
		FLAG_LOADED      = BITFLAG( 0 ),
		FLAG_LOADING     = BITFLAG( 1 ),
		FLAG_INITIALISED = BITFLAG( 2 ),
		FLAG_DESTROYED   = BITFLAG( 3 ),
	};

	void Init( const TCHAR* a_pName, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pCallbkData );
	void DeInit();

	void SetLoadedData( void* a_pData );
	void Load( const TCHAR* a_szFilePath );
	void Unload();

	void* GetData();

	const TCHAR* GetName() const { return m_pResourceName; }
	TTRB*        GetCreatedInTRB() const { return m_pCreatedInTRB; }

	TBOOL HasFlag( TUINT a_uiFlag ) const { return ( m_iFlags & a_uiFlag ) == a_uiFlag; }
	TBOOL HasAnyFlag( TUINT a_uiFlag ) const { return ( m_iFlags & a_uiFlag ) != 0; }

private:
	void*                                    m_pData;                    // 0x00
	TTRB*                                    m_pCreatedInTRB;            // 0x04 TAssetInit::g_pCurrentTRB when created
	TUINT                                    m_iFlags;                   // 0x08
	TINT                                     m_iRefCount;                // 0x0C
	TCHAR                                    m_pResourceName[ 48 ];      // 0x10
	void*                                    m_pCreateDestroyCallbkData; // 0x40
	T2ResourceManager::t_CreateDestroyCallbk m_fnCreateDestroyCallbk;    // 0x44
	TTRB*                                    m_pTRB;                     // 0x48 loaded by Load
};                                                                       // 0x4C

//-----------------------------------------------------------------------------
// Purpose: a snapshot of every resource's name, flags and ref count (leak checks)
//-----------------------------------------------------------------------------
class T2ResourceManagerState
{
public:
	class T2ResourceState
	{
	public:
		void CopyState( T2ResourceData* a_pData );

		static TBOOL IsStateSame( T2ResourceState* a_pState1, T2ResourceState* a_pState2 );

	public:
		TCHAR m_szName[ 24 ]; // 0x00
		TUINT m_iFlags;       // 0x18
		TINT  m_iRefCount;    // 0x1C
	};                        // 0x20

public:
	T2ResourceManagerState() = default;
	~T2ResourceManagerState();

public:
	TINT             m_iMaxNumResources  = 0;     // 0x00
	TINT             m_iNumUsedResources = 0;     // 0x04
	T2ResourceState* m_ResourceStates    = TNULL; // 0x08
};                                                // 0x0C

class T2ResourcePtr
{
public:
	static TCONSTEXPR_VAR TINT IDINVALID = 0;

public:
	T2ResourcePtr( TINT a_iID = IDINVALID )
	{
		m_iResourceID = a_iID;
		T2ResourceManager::GetSingleton()->IncRefCount( m_iResourceID );
	}

	T2ResourcePtr( const T2ResourcePtr& a_rOther );

	// $deBlob: FUNCTION 0054eee0
	~T2ResourcePtr() { T2ResourceManager::GetSingleton()->DecRefCount( m_iResourceID ); }

	T2ResourcePtr& operator=( const T2ResourcePtr& a_rOther )
	{
		T2ResourceManager::GetSingleton()->DecRefCount( m_iResourceID );
		m_iResourceID = a_rOther.m_iResourceID;
		T2ResourceManager::GetSingleton()->IncRefCount( m_iResourceID );
		return *this;
	}

	// $deBlob: FUNCTION 0054ef10
	void* GetData() const { return T2ResourceManager::GetSingleton()->GetData( m_iResourceID ); }

	TINT GetID() const { return m_iResourceID; }
	operator TINT() const { return m_iResourceID; }

protected:
	TINT m_iResourceID; // 0x00
};

//-----------------------------------------------------------------------------
// Purpose: the id of a resource owned by an object (created/destroyed by it)
//-----------------------------------------------------------------------------
class T2Resource
{
public:
	friend class T2ResourceManager;

public:
	void CreateResource( const TCHAR* a_szName, void* a_pData, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pCallbkData );

	void DestroyResource()
	{
		TASSERT( T2ResourcePtr::IDINVALID != m_iID );
		T2ResourceManager::GetSingleton()->DestroyResource( m_iID );
	}

	// $deBlob: FUNCTION 006b1c80
	T2ResourcePtr GetResourcePtr() const { return T2ResourcePtr( m_iID ); }

	TINT GetResourceId() const { return m_iID; }
	operator TINT() const { return m_iID; }

private:
	TINT m_iID = T2ResourcePtr::IDINVALID; // 0x00
};

TOSHI_NAMESPACE_END
