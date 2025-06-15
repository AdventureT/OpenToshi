#include "ToshiPCH.h"
#include "TSysShader.h"

TOSHI_NAMESPACE_START

const TCHAR* TSysShader::GetShaderPrefix()
{
	return "";
}

TBOOL TSysShader::IsLightScattering() const
{
	return TFALSE;
}

TBOOL TSysShader::SetLightScattering(TBOOL scattering) const
{
	return TFALSE;
}

TOSHI_NAMESPACE_END
