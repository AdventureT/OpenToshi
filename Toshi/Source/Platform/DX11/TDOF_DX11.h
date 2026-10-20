#pragma once
#include <d3d11.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// NOTE: depth of field is a stub on DX11: both remaining functions only assert and nothing calls them.
//-----------------------------------------------------------------------------
class TDOF
{
public:
	TDOF();
	~TDOF();

	// NOTE: nothing calls these; the parameter types are guesses (ret 4 and ret 8)
	void Unknown1( void* a_pUnk );
	void Unknown2( void* a_pUnk1, void* a_pUnk2 );

private:
	IUnknown* m_pUnk00;   // 0x00 released by the destructor, never created on DX11
	TUINT32   m_uiWidth;  // 0x04
	TUINT32   m_uiHeight; // 0x08
	IUnknown* m_pUnk0C;   // 0x0C released by the destructor, never created on DX11
	TFLOAT    m_fUnk10;   // 0x10
	TFLOAT    m_fUnk14;   // 0x14
	TFLOAT    m_fUnk18;   // 0x18
	TFLOAT    m_fUnk1C;   // 0x1C
	TFLOAT    m_fUnk20;   // 0x20
	TBOOL     m_bUnk24;   // 0x24
};

TOSHI_NAMESPACE_END
