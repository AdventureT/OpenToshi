#include "pch.h"
#include "ASpriteMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00602a00
// $deBlob: FUNCTION 00602a10
// $deBlob: FUNCTION 00602a20
// $deBlob: FUNCTION 00602a30
// $deBlob: FUNCTION 00602a40
// $deBlob: FUNCTION 0053ce90
// $deBlob: FUNCTION 00858720
TDEFINE_CLASS_NORUNTIME( ASpriteMaterial );

// $deBlob: FUNCTION 00602a70
ASpriteMaterial::ASpriteMaterial()
{
	m_ePrimitiveType = 0;
	m_eBlendMode     = 0;
}

// $deBlob: FUNCTION 00602a90
// $deBlob: FUNCTION 00603090
ASpriteMaterial::~ASpriteMaterial()
{
}

// $deBlob: FUNCTION 00602a50
TINT ASpriteMaterial::SetBlendMode( TINT a_eBlendMode )
{
	m_eBlendMode = a_eBlendMode;
	return a_eBlendMode;
}

// $deBlob: FUNCTION 00602a60
TINT ASpriteMaterial::SetPrimitiveType( TINT a_ePrimitiveType )
{
	m_ePrimitiveType = a_ePrimitiveType;
	return a_ePrimitiveType;
}
