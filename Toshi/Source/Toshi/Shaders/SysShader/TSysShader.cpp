#include "ToshiPCH.h"
#include "TSysShader.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0070a7d0
const TCHAR* TSysShader::GetShaderPrefix()
{
	return "";
}

// $deBlob: FUNCTION 0070a800
TBOOL TSysShader::IsLightScattering() const
{
	return TFALSE;
}

// $deBlob: FUNCTION 0070a820
TBOOL TSysShader::SetLightScattering(TBOOL scattering) const
{
	return TFALSE;
}

TOSHI_NAMESPACE_END
