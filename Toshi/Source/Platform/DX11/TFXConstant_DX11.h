#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: the shader constants of a TFXManager_DX11 effect. Unused on PC: every method asserts. The method names are
//-----------------------------------------------------------------------------
class TFXConstant_DX11
{
public:
	static TCONSTEXPR_VAR TINT NUM_TYPES = 46;

public:
	void SetValue( TINT a_iIndex, const void* a_pValue );
	void GetValue( TINT a_iIndex, void* a_pOutValue );
	void Apply( void* a_pShader );
	void Init( void* a_pShader );

	// Called by TRenderDX11's constructor
	static void InitTypes();

private:
	static TUINT32 s_aTypes[ NUM_TYPES ]; // written by InitTypes, never read

private:
	TUINT8 m_aData[ 0x178 ]; // allocated by TFXManager_DX11::FXLite::Init, never constructed
}; // 0x178

TOSHI_NAMESPACE_END
