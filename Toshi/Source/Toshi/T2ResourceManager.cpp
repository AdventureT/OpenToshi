#include "ToshiPCH.h"
#include "T2ResourceManager.h"
#include "Toshi/TError.h"
#include "Render/TAssetInit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

T2ResourceManager* T2ResourceManager::s_pSingleton;

// $deBlob: FUNCTION 006f9e40
T2ResourceManager::T2ResourceManager( TINT a_iMaxNumResources )
{
	TASSERT( s_pSingleton == TNULL );
	s_pSingleton = this;

	m_iMaxNumResources  = a_iMaxNumResources;
	m_iNumUsedResources = 0;
	m_iNextFreeResource = 0;

	m_pData = new T2ResourceData[ m_iMaxNumResources ];
	TUtil::MemClear( m_pData, m_iMaxNumResources * sizeof( T2ResourceData ) );

	// The dummy (invalid) resource
	m_pData[ 0 ].m_iFlags |= T2ResourceData::FLAG_INITIALISED;
}

// $deBlob: FUNCTION 006f9d70
T2ResourcePtr::T2ResourcePtr( const T2ResourcePtr& a_rOther )
{
	m_iResourceID = a_rOther.m_iResourceID;
	T2ResourceManager::GetSingleton()->IncRefCount( m_iResourceID );
}

// $deBlob: FUNCTION 006f9ed0
// $deBlob: FUNCTION 00691710
T2ResourceManager::~T2ResourceManager()
{
	TASSERT( s_pSingleton != TNULL );
	s_pSingleton = TNULL;

	delete[] m_pData;
}

// $deBlob: FUNCTION 006fa5c0
TINT T2ResourceManager::CreateResource( const TCHAR* a_szName, void* a_pData, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pCallbkData )
{
	TINT iID = FindUnusedResource();

	if ( iID >= m_iMaxNumResources )
	{
		TError::GetSingleton()->AddError( "couldnt create resource '%s' because we have reached our max (%d)", a_szName, m_iMaxNumResources );
		return T2ResourcePtr::IDINVALID;
	}

	TASSERT( iID >= 0 );
	TASSERT( iID < m_iMaxNumResources );
	TASSERT( TFALSE == m_pData[ iID ].HasFlag( T2ResourceData::FLAG_INITIALISED ) );

	m_iNumUsedResources++;
	m_pData[ iID ].Init( a_szName, a_fnCreateDestroyCallbk, a_pCallbkData );

	// NOTE: resources are always created with their data; the create callback is only called by T2ResourceData::Load
	m_pData[ iID ].SetLoadedData( a_pData );
	m_pData[ iID ].m_pCreatedInTRB = TAssetInit::g_pCurrentTRB;

	return iID;
}

// $deBlob: FUNCTION 006fa6e0
void T2ResourceManager::DestroyResource( TINT a_iID )
{
	if ( a_iID != T2ResourcePtr::IDINVALID )
	{
		TASSERT( a_iID >= 0 );
		TASSERT( a_iID < m_iMaxNumResources );

		T2ResourceData* pData = &m_pData[ a_iID ];
		pData->DeInit();

		// Still referenced: freed by DecRefCount once the last reference goes
		if ( pData->m_iRefCount == 0 )
			m_iNumUsedResources--;
	}
}

// $deBlob: FUNCTION 006f9f20
void T2ResourceManager::IterateResources( t_IterateCallbk a_fnCallbk, void* a_pUserData )
{
	T2ResourceData* pEnd = m_pData + m_iMaxNumResources;

	for ( T2ResourceData* pResData = m_pData + 1; pResData < pEnd; pResData++ )
	{
		if ( pResData->HasFlag( T2ResourceData::FLAG_INITIALISED ) && pResData->HasFlag( T2ResourceData::FLAG_LOADED ) )
			a_fnCallbk( a_pUserData, pResData->GetData() );
	}
}

// $deBlob: FUNCTION 006f9fc0
T2ResourcePtr T2ResourceManager::FindResource( const TCHAR* a_szName )
{
	TASSERT( T2String8::IsLowerCase( a_szName ) );

	T2ResourceData* pEnd = m_pData + m_iMaxNumResources;

	for ( T2ResourceData* pResData = m_pData + 1; pResData < pEnd; pResData++ )
	{
		if ( pResData->HasFlag( T2ResourceData::FLAG_INITIALISED ) && T2String8::CompareNoCase( pResData->GetName(), a_szName ) == 0 )
		{
			TINT iID = TINT( pResData - m_pData );
			TASSERT( iID >= 0 );
			TASSERT( iID < m_iMaxNumResources );

			return T2ResourcePtr( iID );
		}
	}

	return T2ResourcePtr( T2ResourcePtr::IDINVALID );
}

// $deBlob: FUNCTION 006fa770
TINT T2ResourceManager::FindUnusedResource()
{
	TASSERT( m_iNumUsedResources < m_iMaxNumResources );

	// Free: neither initialised nor destroyed-but-referenced
	for ( TINT i = m_iNextFreeResource; i < m_iMaxNumResources; i++ )
	{
		if ( !m_pData[ i ].HasAnyFlag( T2ResourceData::FLAG_INITIALISED | T2ResourceData::FLAG_DESTROYED ) && m_pData[ i ].m_iRefCount == 0 )
		{
			m_iNextFreeResource = i + 1;
			return i;
		}
	}

	for ( TINT i = 0; i < m_iNextFreeResource; i++ )
	{
		if ( !m_pData[ i ].HasAnyFlag( T2ResourceData::FLAG_INITIALISED | T2ResourceData::FLAG_DESTROYED ) && m_pData[ i ].m_iRefCount == 0 )
		{
			m_iNextFreeResource = i + 1;
			return i;
		}
	}

	TASSERT( !"Should never get here" );
	return T2ResourcePtr::IDINVALID;
}

// $deBlob: FUNCTION 006fa0f0
TBOOL T2ResourceManager::ReloadResource( const TCHAR* a_szName, const TCHAR* a_szFilePath )
{
	T2ResourcePtr resource = FindResource( a_szName );
	TINT          iID      = resource.GetID();

	if ( iID != T2ResourcePtr::IDINVALID )
	{
		TASSERT( iID >= 0 );
		TASSERT( iID < m_iMaxNumResources );

		m_pData[ iID ].Unload();
		m_pData[ iID ].Load( a_szFilePath );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006fa1c0
void T2ResourceManager::ReloadResource( T2ResourcePtr a_Resource, T2Resource* a_pResource, void* a_pData )
{
	TINT iID = a_Resource.GetID();

	TASSERT( iID != T2ResourcePtr::IDINVALID );
	TASSERT( iID >= 0 );
	TASSERT( iID < m_iMaxNumResources );
	TASSERT( m_pData[ iID ].HasFlag( T2ResourceData::FLAG_INITIALISED ) );

	m_pData[ iID ].SetLoadedData( a_pData );
	a_pResource->m_iID = iID;
}

// $deBlob: FUNCTION 006fa950
void T2ResourceManager::IncRefCount( TINT a_iResourceID )
{
	if ( a_iResourceID != T2ResourcePtr::IDINVALID )
	{
		TASSERT( a_iResourceID >= 0 );
		TASSERT( a_iResourceID < m_iMaxNumResources );

		m_pData[ a_iResourceID ].m_iRefCount++;
		TASSERT( m_pData[ a_iResourceID ].m_iRefCount > 0 );
	}
}

// $deBlob: FUNCTION 006faa20
void T2ResourceManager::DecRefCount( TINT a_iResourceID )
{
	if ( a_iResourceID != T2ResourcePtr::IDINVALID )
	{
		TASSERT( a_iResourceID >= 0 );
		TASSERT( a_iResourceID < m_iMaxNumResources );

		T2ResourceData* pData = &m_pData[ a_iResourceID ];
		pData->m_iRefCount--;
		TASSERT( pData->m_iRefCount >= 0 );

		if ( pData->m_iRefCount == 0 && pData->HasAnyFlag( T2ResourceData::FLAG_DESTROYED ) )
		{
			// The last reference to a destroyed resource: free its slot
			pData->m_iFlags &= ~T2ResourceData::FLAG_DESTROYED;
			m_iNumUsedResources--;
			pData->m_pResourceName[ 0 ] = '\0';
		}
	}
}

// $deBlob: FUNCTION 006fa840
void* T2ResourceManager::GetData( TINT a_iID )
{
	TASSERT( a_iID >= 0 );
	TASSERT( a_iID < m_iMaxNumResources );
	TASSERT( TFALSE == m_pData[ a_iID ].HasFlag( T2ResourceData::FLAG_DESTROYED ) );
	TASSERT( TTRUE == m_pData[ a_iID ].HasFlag( T2ResourceData::FLAG_INITIALISED ) );

	if ( a_iID == T2ResourcePtr::IDINVALID )
		return TNULL;

	return m_pData[ a_iID ].GetData();
}

// $deBlob: FUNCTION 006fa2e0
T2ResourceData* T2ResourceManager::GetResourceData( TINT a_iID )
{
	TASSERT( a_iID != T2ResourcePtr::IDINVALID );
	TASSERT( a_iID >= 0 );
	TASSERT( a_iID < m_iMaxNumResources );
	TASSERT( TFALSE == m_pData[ a_iID ].HasFlag( T2ResourceData::FLAG_DESTROYED ) );
	TASSERT( TTRUE == m_pData[ a_iID ].HasFlag( T2ResourceData::FLAG_INITIALISED ) );

	return &m_pData[ a_iID ];
}

// $deBlob: FUNCTION 006f9910
void T2ResourceData::Init( const TCHAR* a_pName, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pCallbkData )
{
	TASSERT( m_fnCreateDestroyCallbk == TNULL );
	TASSERT( m_pResourceName[ 0 ] == '\0' );
	TASSERT( m_pData == TNULL );
	TASSERT( m_iFlags == 0 );
	TASSERT( m_iRefCount == 0 );

	TASSERT( a_fnCreateDestroyCallbk != TNULL );
	TASSERT( T2String8::IsLowerCase( a_pName ) );

	TASSERT( T2String8::Length( a_pName ) < MAX_RESOURCE_NAME );
	T2String8::CopySafe( m_pResourceName, a_pName, MAX_RESOURCE_NAME );

	m_fnCreateDestroyCallbk    = a_fnCreateDestroyCallbk;
	m_pCreateDestroyCallbkData = a_pCallbkData;
	m_iFlags |= FLAG_INITIALISED;
}

// $deBlob: FUNCTION 006f9af0
void T2ResourceData::DeInit()
{
	TASSERT( HasFlag( FLAG_INITIALISED ) );
	m_iFlags &= ~FLAG_INITIALISED;

	Unload();
	TASSERT( m_pData == TNULL );

	m_fnCreateDestroyCallbk = TNULL;
	m_iFlags                = 0;

	if ( m_iRefCount > 0 )
	{
		// Still referenced: keep the name until the last reference is released (T2ResourceManager::DecRefCount)
		m_iFlags |= FLAG_DESTROYED;
	}
	else
	{
		m_pResourceName[ 0 ] = '\0';
	}
}

// $deBlob: FUNCTION 006f9cc0
void T2ResourceData::SetLoadedData( void* a_pData )
{
	Unload();
	m_pData = a_pData;
	m_iFlags |= FLAG_LOADED;
	m_iFlags &= ~FLAG_LOADING;
}

// $deBlob: FUNCTION 006f9c00
void T2ResourceData::Load( const TCHAR* a_szFilePath )
{
	if ( HASANYFLAG( m_iFlags, FLAG_LOADED | FLAG_LOADING ) )
		return;

	m_pTRB = new TTRB;

	if ( m_pTRB->Load( a_szFilePath, 0 ) == TTRB::ERROR_OK )
	{
		TASSERT( TNULL != m_fnCreateDestroyCallbk );
		m_pData = m_fnCreateDestroyCallbk( m_pCreateDestroyCallbkData, m_pTRB, TTRUE );

		if ( m_pData != TNULL )
		{
			m_iFlags |= FLAG_LOADED;
			m_iFlags &= ~FLAG_LOADING;
			return;
		}
	}

	m_pTRB->Close();

	if ( m_pTRB )
		delete m_pTRB;

	m_pTRB = TNULL;
}

// $deBlob: FUNCTION 006f9ce0
void T2ResourceData::Unload()
{
	if ( HASANYFLAG( m_iFlags, FLAG_LOADED ) )
	{
		m_iFlags &= ~( FLAG_LOADED | FLAG_LOADING );

		if ( m_pTRB != TNULL )
		{
			TASSERT( TNULL != m_fnCreateDestroyCallbk );
			m_fnCreateDestroyCallbk( m_pCreateDestroyCallbkData, m_pTRB, TFALSE );

			m_pTRB->Close();

			if ( m_pTRB )
				delete m_pTRB;

			m_pTRB = TNULL;
		}

		m_pData = TNULL;
	}
}

// $deBlob: FUNCTION 006f9b90
void* T2ResourceData::GetData()
{
	if ( !HasFlag( FLAG_LOADED ) )
	{
		TASSERT( HasFlag( FLAG_LOADING ) );

		// Wait for the resource to be loaded
		// NOTE: no deadline, as in retail (nothing sets FLAG_LOADING in de Blob)
		while ( !HasFlag( FLAG_LOADED ) )
			Sleep( 100 );
	}

	return m_pData;
}

// $deBlob: FUNCTION 006fab00
void T2Resource::CreateResource( const TCHAR* a_szName, void* a_pData, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pCallbkData )
{
	if ( m_iID == T2ResourcePtr::IDINVALID )
	{
		m_iID = T2ResourceManager::GetSingleton()->CreateResource( a_szName, a_pData, a_fnCreateDestroyCallbk, a_pCallbkData );
	}
	else
	{
		TASSERT( !"resource already created!" );
	}
}

// $deBlob: FUNCTION 006fa410
void T2ResourceManager::CaptureState( T2ResourceManagerState* a_State )
{
	TASSERT( TNULL == a_State->m_ResourceStates );

	a_State->m_iMaxNumResources  = m_iMaxNumResources;
	a_State->m_iNumUsedResources = m_iNumUsedResources;
	a_State->m_ResourceStates    = new T2ResourceManagerState::T2ResourceState[ m_iMaxNumResources ];

	for ( TINT i = 0; i < m_iMaxNumResources; i++ )
		a_State->m_ResourceStates[ i ].CopyState( &m_pData[ i ] );
}

// $deBlob: FUNCTION 006fa4e0
TBOOL T2ResourceManager::IsStateSame( T2ResourceManagerState* a_State )
{
	TINT iNumDifferent = 0;

	for ( TINT i = 0; i < m_iMaxNumResources; i++ )
	{
		T2ResourceManagerState::T2ResourceState state;
		state.CopyState( &m_pData[ i ] );

		if ( !T2ResourceManagerState::T2ResourceState::IsStateSame( &state, &a_State->m_ResourceStates[ i ] ) )
			iNumDifferent++;
	}

	TBOOL bSame = a_State->m_iNumUsedResources == m_iNumUsedResources;

	if ( iNumDifferent > 0 )
		return TFALSE;

	return bSame;
}

// $deBlob: FUNCTION 006fa5a0
void T2ResourceManager::DestroyState( T2ResourceManagerState* a_State )
{
	delete[] a_State->m_ResourceStates;
	a_State->m_ResourceStates = TNULL;
}

// $deBlob: FUNCTION 006f9e20
T2ResourceManagerState::~T2ResourceManagerState()
{
	if ( m_ResourceStates )
	{
		delete[] m_ResourceStates;
		m_ResourceStates = TNULL;
	}
}

// $deBlob: FUNCTION 006f9d90
void T2ResourceManagerState::T2ResourceState::CopyState( T2ResourceData* a_pData )
{
	T2String8::CopySafe( m_szName, a_pData->m_pResourceName, sizeof( m_szName ) );
	m_iFlags    = a_pData->m_iFlags;
	m_iRefCount = a_pData->m_iRefCount;
}

// $deBlob: FUNCTION 006f9dc0
TBOOL T2ResourceManagerState::T2ResourceState::IsStateSame( T2ResourceState* a_pState1, T2ResourceState* a_pState2 )
{
	return a_pState1->m_iFlags == a_pState2->m_iFlags && a_pState1->m_iRefCount == a_pState2->m_iRefCount;
}

TOSHI_NAMESPACE_END
