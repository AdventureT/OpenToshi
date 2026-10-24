#pragma once
#include <d3d11.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Blurs the bright parts of the frame into a half size target and adds them back.
//-----------------------------------------------------------------------------
class TGlow
{
public:
	TGlow();
	~TGlow();

	void Render( ID3D11ShaderResourceView* a_pSRV, ID3D11ShaderResourceView* a_pStencilSRV );

	void SetDist( TFLOAT a_fDist );
	void SetIntensity( TFLOAT a_fIntensity );

	TFLOAT GetIntensity() const { return m_fIntensity; }
	TFLOAT GetDist() const { return m_fDist; }

private:
	TFLOAT                    m_fIntensity;    // 0x00
	TFLOAT                    m_fDist;         // 0x04
	TINT                      m_iUnk8;         // 0x08 never written
	TUINT32                   m_uiWidth;       // 0x0C
	TUINT32                   m_uiHeight;      // 0x10
	ID3D11Texture2D*          m_pTexture;      // 0x14
	ID3D11ShaderResourceView* m_pGlowSRV;      // 0x18
	ID3D11Texture2D*          m_pTexture2;     // 0x1C
	ID3D11ShaderResourceView* m_pGlowSRV2;     // 0x20
	ID3D11RenderTargetView*   m_pRenderTarget; // 0x24
	ID3D11PixelShader*        m_pPixelShader;  // 0x28
};

TOSHI_NAMESPACE_END
