#include "ToshiPCH.h"
#include "T2ResourceHelper.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006fac50
T2ModelPtr& T2ModelPtr::operator=( T2ModelPtr a_Other )
{
	T2ResourceManager::GetSingleton()->DecRefCount( m_iResourceID );
	m_iResourceID = a_Other.m_iResourceID;
	T2ResourceManager::GetSingleton()->IncRefCount( m_iResourceID );
	return *this;
}

TOSHI_NAMESPACE_END
