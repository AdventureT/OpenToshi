#include "ToshiPCH.h"
#include "TMemoryListener.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 007316c0
void TMemoryListener::cannotAllocate( hkMemory* a_pMem, hk_size_t a_uiSize )
{
	TDebug_FinalPrintf( "TMemoryListener: memory allocation failure\n" );
}

// $deBlob: FUNCTION 00731610
void TMemoryListener::allocationFailure( hkMemory* a_pMem, hk_size_t a_uiSize )
{
	TDebug_FinalPrintf( "TMemoryListener: memory allocation failure\n" );
}

TOSHI_NAMESPACE_END
