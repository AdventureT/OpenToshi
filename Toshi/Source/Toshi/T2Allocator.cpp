#include "ToshiPCH.h"
#include "T2Allocator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

T2GlobalAllocator ms_oGlobalAllocator;

// $deBlob: FUNCTION 006f2100
void* T2GlobalAllocator::Malloc( TSIZE size )
{
	return TMalloc( size );
}

// $deBlob: FUNCTION 006f2110
void* T2GlobalAllocator::Malloc( TSIZE size, TSIZE alignment )
{
	return TMemalign( alignment, size );
}

// $deBlob: FUNCTION 006f20f0
void T2GlobalAllocator::Free( void* ptr )
{
	TFree( ptr );
}

// $deBlob: FUNCTION 006f20d0
TBOOL T2GlobalAllocator::CanAllocate( TSIZE size ) const
{
	return TTRUE;
}

// $deBlob: FUNCTION 006f2140
void* T2GlobalAllocator::TryMalloc( TSIZE size )
{
	return TMalloc( size );
}

// $deBlob: FUNCTION 006f2150
void* T2GlobalAllocator::TryMalloc( TSIZE size, TSIZE alignment )
{
	return TMemalign( alignment, size );
}

// $deBlob: FUNCTION 006f2130
TSIZE T2GlobalAllocator::GetUsedSize() const
{
	return 0;
}

// $deBlob: FUNCTION 006f20e0
TSIZE T2GlobalAllocator::GetCapacity() const
{
	return 0;
}

TOSHI_NAMESPACE_END
