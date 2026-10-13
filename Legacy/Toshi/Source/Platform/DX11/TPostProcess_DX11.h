#pragma once
#include "Platform/DX11/Includes.h"

TOSHI_NAMESPACE_START

class TPostProcess
{
public:
	static constexpr size_t MAX_SAMPLES = 16;

public:
	TPostProcess();

	// $deBlob: FUNCTION 006b9190
	void ApplyGaussBlur(ID3D11ShaderResourceView* pSRV, TFLOAT fIntensity) { RenderImpl(m_pPSGaussBlur5x5, pSRV, fIntensity, TNULL); }

	// $deBlob: FUNCTION 006b91b0
	void ApplyGaussBlurWithAlpha(ID3D11ShaderResourceView* pSRV, TFLOAT fIntensity, ID3D11ShaderResourceView* pStencilSRV) { RenderImpl(pStencilSRV ? m_pPSGaussBlurAlphaStencil : m_pPSGaussBlurAlpha, pSRV, fIntensity, pStencilSRV); }

private:
	void GaussianBlur5x5(TINT texWidth, TINT texHeight, TVector4* offsets, TVector4* weights, TFLOAT fUnk);
	void DrawScreenOverlay();
	void RenderImpl(ID3D11PixelShader* pPS, ID3D11ShaderResourceView* pSRV, TFLOAT fIntensity, ID3D11ShaderResourceView* pStencilSRV);

private:
	ID3D11InputLayout*  m_pQuadVtxDecl;   // 0x00
	ID3D11Buffer*       m_pQuadVB;        // 0x04
	ID3D11VertexShader* m_pScreenSpaceVS; // 0x08

	ID3D11PixelShader* m_pPSGaussBlur5x5;          // 0x30
	ID3D11PixelShader* m_pPSGaussBlurAlpha;        // 0x34
	ID3D11PixelShader* m_pPSGaussBlurAlphaStencil; // 0x38
};

TOSHI_NAMESPACE_END
