#include "pch.h"
#include "AStaticInstanceMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AStaticInstanceMaterial ) == 0x58 );

// $deBlob: FUNCTION 00603670
// $deBlob: FUNCTION 00603680
// $deBlob: FUNCTION 00603690
// $deBlob: FUNCTION 006036a0
// $deBlob: FUNCTION 006036b0
// $deBlob: FUNCTION 0053cf50
// $deBlob: FUNCTION 00858750
TDEFINE_CLASS_NORUNTIME( AStaticInstanceMaterial );

// $deBlob: FUNCTION 006036f0
AStaticInstanceMaterial::AStaticInstanceMaterial()
{
	m_pTexture = TNULL;
}

// $deBlob: FUNCTION 006036c0
// $deBlob: FUNCTION 00603ce0
AStaticInstanceMaterial::~AStaticInstanceMaterial()
{
}

// $deBlob: FUNCTION 00603d70
TBOOL AStaticInstanceMaterial::Create()
{
	return TMaterial::Create();
}

// $deBlob: FUNCTION 00603d80
void AStaticInstanceMaterial::Create( TTexture* a_pTexture )
{
	SetTexture( a_pTexture );
	TMaterial::Create();
}

// $deBlob: FUNCTION 006036d0
void AStaticInstanceMaterial::SetTexture( TTexture* a_pTexture )
{
	m_Flags &= ~FLAGS_GLOW;
	m_pTexture = a_pTexture;

	if ( a_pTexture != TNULL )
		m_Flags |= FLAGS_GLOW;
}
