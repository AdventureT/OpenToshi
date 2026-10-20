#include "pch.h"
#include "ANavMarkers.h"

#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ANavMarkers* ANavMarkers::ms_pSingleton;

TSTATICASSERT( sizeof( ANavMarkers ) == 0x4 );

// $deBlob: FUNCTION 007900c0
ANavMarkers::ANavMarkers( TTRB* a_pTRB )
{
	m_pData       = TNULL;
	m_pData       = a_pTRB->GetSymbolAddress( "NavMarkers" );
	ms_pSingleton = this;
}

// $deBlob: FUNCTION 007900f0
// $deBlob: FUNCTION 005c8780
ANavMarkers::~ANavMarkers()
{
	ms_pSingleton = TNULL;
}
