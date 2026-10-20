#include "pch.h"
#include "ALightingData.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ALightingData* ALightingData::sm_singleton;

// $deBlob: FUNCTION 005f75b0
ALightingData::ALightingData()
{
	m_pCurrent = TNULL;
	TASSERT( 0 == sm_singleton );
	sm_singleton = this;
}

// $deBlob: FUNCTION 005f7600
// $deBlob: FUNCTION 005c3c30
ALightingData::~ALightingData()
{
	m_pCurrent = TNULL;
	TASSERT( this == sm_singleton );
	sm_singleton = TNULL;
}

// Points to the "LightingData" symbol of the terrain's TRB
// $deBlob: FUNCTION 005f7650
void ALightingData::Load( TTRB* a_pTRB )
{
	m_pCurrent = TSTATICCAST( Light, a_pTRB->GetSymbolAddress( "LightingData" ) );
}

// $deBlob: FUNCTION 005f7670
TFLOAT ALightingData::GetInstanceAmbientScale( TINT a_iIndex ) const
{
	// NOTE: retail takes the terrain instance and reads its index (TerrainTypes CellInstance 0x3A)
	TINT16 index = TINT16( a_iIndex );
	TASSERT( index >= 0 && index < m_pCurrent->numInstanceAmbientScale );
	return m_pCurrent->pfInstanceAmbientScale[ index ];
}
