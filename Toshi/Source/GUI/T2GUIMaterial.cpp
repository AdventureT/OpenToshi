#include "ToshiPCH.h"
#include "T2GUIMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TSTATICASSERT( sizeof( T2GUIMaterial ) == 0x68 );

// $deBlob: FUNCTION 006f4c60
// $deBlob: FUNCTION 006f4c70
// $deBlob: FUNCTION 006f4cc0
// $deBlob: FUNCTION 006f4d10
// $deBlob: FUNCTION 006f4d20
// $deBlob: FUNCTION 0053f950
// $deBlob: FUNCTION 008594b0
TDEFINE_CLASS( T2GUIMaterial );

// $deBlob: FUNCTION 006f4d30
T2GUIMaterial::T2GUIMaterial() TNOEXCEPT
{
	m_pTextureResource = TNULL;
	m_iNumTex          = 1;
	m_eBlendState      = BLENDSTATE_ALPHA;
	m_eTextureAddressV = TEXTUREADDRESS_CLAMP;
	m_eTextureAddress  = TEXTUREADDRESS_CLAMP;
	m_uiUnk64          = 0;
}

// $deBlob: FUNCTION 006f4d70
// $deBlob: FUNCTION 006f4d80
T2GUIMaterial::~T2GUIMaterial()
{
}

// $deBlob: FUNCTION 006f4db0
void T2GUIMaterial::SetUnk64( TUINT32 a_uiValue )
{
	m_uiUnk64 = a_uiValue;
}

TOSHI_NAMESPACE_END
