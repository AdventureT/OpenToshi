#include "pch.h"
#include "AConsoleAllocator.h"

#include <cstdlib>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AConsoleAllocator AConsoleAllocator::s_Instance;

// $deBlob: FUNCTION 005b09e0
void* AConsoleAllocator::Malloc( size_t a_uiSize )
{
	return malloc( a_uiSize );
}

// $deBlob: FUNCTION 005b0a00
void* AConsoleAllocator::Malloc( size_t a_uiSize, size_t a_uiAlignment )
{
	return TNULL;
}

// $deBlob: FUNCTION 005b0870
void AConsoleAllocator::Free( void* a_pMem )
{
	free( a_pMem );
}

// $deBlob: FUNCTION 005b03e0
TBOOL AConsoleAllocator::CanAllocate( size_t a_uiSize ) const
{
	return TTRUE;
}

// $deBlob: FUNCTION 005b0d90
void* AConsoleAllocator::TryMalloc( size_t a_uiSize )
{
	return malloc( a_uiSize );
}

// $deBlob: FUNCTION 005b0db0
void* AConsoleAllocator::TryMalloc( size_t a_uiSize, size_t a_uiAlignment )
{
	return TNULL;
}

// $deBlob: FUNCTION 005b0d70
size_t AConsoleAllocator::GetUsedSize() const
{
	return size_t( -1 );
}

// $deBlob: FUNCTION 005b03f0
size_t AConsoleAllocator::GetCapacity() const
{
	return size_t( -1 );
}
