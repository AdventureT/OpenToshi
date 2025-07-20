#pragma once
#include "Platform/DX11/Includes.h"

TOSHI_NAMESPACE_START

class TGlow
{

public:
	TGlow();

	void Render(ID3D11ShaderResourceView* srv, ID3D11ShaderResourceView* srv2);

	void SetIntensity(TFLOAT a_fIntensity)
	{
		TASSERT(a_fIntensity >= 0.0f);
		m_fIntensity = a_fIntensity;
	}

	TFLOAT GetIntensity() const { return m_fIntensity; }

	void SetDist(TFLOAT a_fDist)
	{
		TASSERT(a_fDist >= 0.0f);
		m_fDist = a_fDist;
	}

	TFLOAT GetDist() const { return m_fDist; }

private:
	static constexpr const TCHAR* s_defaultGlowPixelShader =
	    " struct PS_IN\t\t\t\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t\tfloat4 Position\t\t\t: SV_POSITION;\t\t\tfloat2 Tex0             : TEXCOORD0;\t\t };\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t sampler2D\tframebuf_texture\t: register(s0) = sampler_state{ MipFilter = NONE; MinFilter = LINEAR; MagFilter = LINEAR; AddressU = WRAP; AddressV = WRAP; };   sampler2D\tglow_texture\t\t: register(s0) = sampler_state{ MipFilter = NONE; MinFilter = LINEAR; MagFilter = LINEAR; AddressU = WRAP; AddressV = WRAP; };   float4\tglowmag\t\t\t\t: register(c28);  float4 main( PS_IN In ) : COLOR\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t\tfloat4 glowCol = tex2D(glow_texture, In.Tex0);\t\treturn  glowCol * glowmag;\t\t\t\t\t }\t\t\t\t\t\t\t\t\t\t\t\t";

	TFLOAT                    m_fIntensity;    // 0x0
	TFLOAT                    m_fDist;         // 0x4
	TINT                      m_Unk;           // 0x8
	TUINT32                   m_uiWidth;       // 0xC
	TUINT32                   m_uiHeight;      // 0x10
	ID3D11Texture2D*          m_pTexture;      // 0x14
	ID3D11ShaderResourceView* m_pGlowSrv;      // 0x18
	ID3D11Texture2D*          m_pTexture2;     // 0x1C
	ID3D11ShaderResourceView* m_pGlowSrv2;     // 0x20
	ID3D11RenderTargetView*   m_pRenderTarget; // 0x24
	ID3D11PixelShader*        m_pPixelShader;  // 0x28
};

TOSHI_NAMESPACE_END
