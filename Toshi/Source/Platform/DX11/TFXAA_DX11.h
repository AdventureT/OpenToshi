#pragma once
#include <d3d11.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// NVIDIA FXAA 3.11 (preset 12, green as luma) applied by ARenderer::Update when "antialias" is on.
//-----------------------------------------------------------------------------
class TFXAA
{
public:
	TFXAA();
	~TFXAA();

	void Render( ID3D11ShaderResourceView* a_pShaderResourceView );

private:
	void Initialise();

private:
	ID3D11Texture2D*          m_pTexture;      // 0x00
	ID3D11ShaderResourceView* m_pTextureView;  // 0x04
	ID3D11RenderTargetView*   m_pRenderTarget; // 0x08
	ID3D11PixelShader*        m_pShader;       // 0x0C
	UINT                      m_uiWidth;       // 0x10
	UINT                      m_uiHeight;      // 0x14
};

TOSHI_NAMESPACE_END
