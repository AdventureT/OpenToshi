#include "pch.h"
#include "A2GUIMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( A2GUIMaterial ) == 0x5C );

// $deBlob: FUNCTION 0053b380
// $deBlob: FUNCTION 0054f3a0
// $deBlob: FUNCTION 0054f3b0
// $deBlob: FUNCTION 0054f440
// $deBlob: FUNCTION 0054f4c0
// $deBlob: FUNCTION 0054f4d0
// $deBlob: FUNCTION 00857e90
TDEFINE_CLASS( A2GUIMaterial );

// $deBlob: FUNCTION 0054f4e0
A2GUIMaterial::A2GUIMaterial()
{
	m_pTextureResource = TNULL;
	m_iNumTex          = 1;
	m_eBlendState      = 1;
}

// $deBlob: FUNCTION 0054f510
// $deBlob: FUNCTION 0054f540
A2GUIMaterial::~A2GUIMaterial()
{
}
