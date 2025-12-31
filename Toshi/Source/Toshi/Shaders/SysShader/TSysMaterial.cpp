#include "ToshiPCH.h"
#include "TSysMaterialHAL.h"
#include "TSysMaterial.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0070a740
TBOOL TSysMaterial::Create(BlendMode eBlendMode)
{
	return TMaterial::Create();
}

// $deBlob: FUNCTION 0070aa10
void TSysMaterial::SetBlendMode(BlendMode eBlendMode)
{
	m_eBlendMode = eBlendMode;
}

TOSHI_NAMESPACE_END
