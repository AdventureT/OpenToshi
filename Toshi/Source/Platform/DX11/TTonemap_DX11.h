#pragma once
#include <d3d11.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// NOTE: the DX11 tone map is a stub: Render only asserts and nothing calls it, so it doesn't change the image.
//-----------------------------------------------------------------------------
class TToneMap
{
public:
	TToneMap();
	~TToneMap();

	// NOTE: nothing calls it; the parameter type is a guess (one dword, ret 4)
	void Render( ID3D11ShaderResourceView* a_pSRV );

	void SetBlend( TFLOAT a_fBlend );

private:
	ID3D11ShaderResourceView* m_pTextureView;  // 0x00 128x128
	ID3D11RenderTargetView*   m_pRenderTarget; // 0x04
	TUINT32                   m_uiUnk08;       // 0x08
	TUINT32                   m_uiUnk0C;       // 0x0C
	TFLOAT                    m_fBlend;        // 0x10
	IUnknown*                 m_pUnk14;        // 0x14 released by the destructor, never created on DX11
	IUnknown*                 m_pUnk18;        // 0x18 released by the destructor, never created on DX11
};

TOSHI_NAMESPACE_END
