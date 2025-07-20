#include "ToshiPCH.h"
#include "TSysMaterialHAL.h"
#include "TSysMaterial.h"

TOSHI_NAMESPACE_START

TBOOL TSysMaterial::Create(BlendMode eBlendMode)
{
	return TMaterial::Create();
}

void TSysMaterial::SetBlendMode(BlendMode eBlendMode)
{
	m_eBlendMode = eBlendMode;
}

TOSHI_NAMESPACE_END
