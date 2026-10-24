#include "pch.h"
#include "AStaticInstanceShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AStaticInstanceShader ) == 0xC );

// $deBlob: FUNCTION 00603570
// $deBlob: FUNCTION 00603580
// $deBlob: FUNCTION 00603590
// $deBlob: FUNCTION 006035a0
// $deBlob: FUNCTION 006035b0
// $deBlob: FUNCTION 0053cfd0
// $deBlob: FUNCTION 00858770
TDEFINE_CLASS_NORUNTIME( AStaticInstanceShader );

// $deBlob: FUNCTION 00603620
AStaticInstanceShader::AStaticInstanceShader()
{
}

// $deBlob: FUNCTION 006035c0
// $deBlob: FUNCTION 00603d40
AStaticInstanceShader::~AStaticInstanceShader()
{
}

// $deBlob: FUNCTION 00603dc0
const TCHAR* AStaticInstanceShader::GetShaderPrefix()
{
	return "si_";
}
