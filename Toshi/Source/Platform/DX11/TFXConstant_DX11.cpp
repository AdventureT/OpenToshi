#include "ToshiPCH.h"
#include "TFXConstant_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TUINT32 TFXConstant_DX11::s_aTypes[ NUM_TYPES ];

// $deBlob: FUNCTION 006ba490
void TFXConstant_DX11::SetValue( TINT a_iIndex, const void* a_pValue )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006ba4c0
void TFXConstant_DX11::GetValue( TINT a_iIndex, void* a_pOutValue )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006ba4f0
void TFXConstant_DX11::InitTypes()
{
	TUtil::MemClear( s_aTypes, sizeof( s_aTypes ) );

	s_aTypes[ 2 ]  = 3;
	s_aTypes[ 4 ]  = 3;
	s_aTypes[ 6 ]  = 3;
	s_aTypes[ 10 ] = 4;
	s_aTypes[ 13 ] = 4;
	s_aTypes[ 16 ] = 4;
	s_aTypes[ 19 ] = 4;
	s_aTypes[ 23 ] = 5;
	s_aTypes[ 27 ] = 5;
	s_aTypes[ 31 ] = 5;
	s_aTypes[ 8 ]  = 6;
	s_aTypes[ 36 ] = 1;
	s_aTypes[ 38 ] = 2;
	s_aTypes[ 42 ] = 7;
}

// $deBlob: FUNCTION 006ba630
void TFXConstant_DX11::Apply( void* a_pShader )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006ba660
void TFXConstant_DX11::Init( void* a_pShader )
{
	TASSERT( TFALSE );
}

TOSHI_NAMESPACE_END
