#include "pch.h"
#include "ATRBResourceFactory.h"
#include "ATRBResource.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0078cf30
ATRBResourceFactory::ATRBResourceFactory()
{
}

// $deBlob: FUNCTION 0078cf80
ATRBResourceFactory::~ATRBResourceFactory()
{
	for ( TINT i = 0; i < m_oResources.Size(); i++ )
	{
		if ( m_oResources[ i ] != TNULL )
			delete m_oResources[ i ];
	}

	m_oResources.Reset();
}

// $deBlob: FUNCTION 0078d1a0
ATRBResource* ATRBResourceFactory::Imp_GetResource( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < m_oResources.Size(); i++ )
	{
		if ( m_oResources[ i ]->GetName().CompareNoCase( a_szName ) == 0 )
		{
			if ( !m_oResources[ i ]->IsLoaded() )
				m_oResources[ i ]->Reload();

			return m_oResources[ i ];
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 0078d060
void ATRBResourceFactory::DestroyResource( ATRBResource* a_pResource )
{
	if ( a_pResource->DecRefCount() == 0 )
		a_pResource->Unload();
}

// $deBlob: FUNCTION 0078d0c0
void ATRBResourceFactory::ReloadResources()
{
	for ( TINT i = 0; i < m_oResources.Size(); i++ )
	{
		if ( m_oResources[ i ]->IsLoaded() && m_oResources[ i ]->ShouldReload() )
			m_oResources[ i ]->Reload();
	}
}
