#pragma once
#include <d3d11.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Screen space quad and the 5x5 gaussian blur shaders TGlow uses.
//-----------------------------------------------------------------------------
class TPostProcess
{
public:
	static TCONSTEXPR_VAR TINT MAX_SAMPLES = 16;

public:
	TPostProcess();
	~TPostProcess();

	void ApplyGaussBlur( ID3D11ShaderResourceView* a_pSRV, TFLOAT a_fIntensity );
	void ApplyGaussBlurWithAlpha( ID3D11ShaderResourceView* a_pSRV, TFLOAT a_fIntensity, ID3D11ShaderResourceView* a_pStencilSRV );

private:
	static void   GaussianBlur5x5( TINT a_iTexWidth, TINT a_iTexHeight, TVector4* a_pOffsets, TVector4* a_pWeights, TFLOAT a_fMultiplier );
	static TFLOAT GaussianDistribution( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fRho );

	// The sample offsets and weights of the DirectX SDK's post process filters
	static void GetSampleOffsetsDirectionalBlur( TUINT a_uiTexWidth, TUINT a_uiTexHeight, TFLOAT a_fAngle, TVector4* a_pOffsets, TVector4* a_pWeights, TFLOAT a_fDeviation, TFLOAT a_fMultiplier );
	static void GetSampleOffsetsBloom( TUINT a_uiTexSize, TVector4* a_pOffsets, TVector4* a_pWeights, TFLOAT a_fDeviation );
	static void GetSampleOffsetsDownScale4x4( TUINT a_uiTexWidth, TUINT a_uiTexHeight, TVector4* a_pOffsets );
	static void GetSampleOffsetsDownScale3x3( TUINT a_uiTexWidth, TUINT a_uiTexHeight, TVector4* a_pOffsets );
	static void GetSampleOffsetsDownScale2x2( TUINT a_uiTexWidth, TUINT a_uiTexHeight, TVector4* a_pOffsets );

	// The DX9 leftovers: on DX11 they only assert TFALSE (the arguments aren't read) and nothing calls them
	void      Unk136( void* a_pUnk1, void* a_pUnk2 );
	IUnknown* Unk157( void* a_pUnk );
	void      Unk452( ID3D11ShaderResourceView* pTexture, void* a_pUnk );
	void      Unk477( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, void* a_pUnk4 );
	void      Unk551( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, void* a_pUnk4 );
	void      Unk604( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3 );
	void      Unk673( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, void* a_pUnk4 );
	void      Unk720( void* a_pUnk1, void* a_pUnk2 );
	void      Unk830( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3 );
	void      Unk867( void* a_pUnk1, void* a_pUnk2 );
	void      Unk912( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3 );
	void      Unk954( void* a_pUnk );
	void      Unk1010( void* a_pUnk1, void* a_pUnk2 );
	void      Unk1064( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, void* a_pUnk4, void* a_pUnk5 );
	void      Unk1118( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, void* a_pUnk4, void* a_pUnk5, void* a_pUnk6, void* a_pUnk7, void* a_pUnk8, void* a_pUnk9 );
	void      Unk1179( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3 );
	void      Unk1226( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3 );

	void RenderImpl( ID3D11PixelShader* a_pPS, ID3D11ShaderResourceView* a_pSRV, TFLOAT a_fIntensity, ID3D11ShaderResourceView* a_pStencilSRV );
	void DrawScreenOverlay();

private:
	ID3D11Buffer*       m_pQuadVB;                  // 0x00
	ID3D11InputLayout*  m_pQuadVtxDecl;             // 0x04
	ID3D11VertexShader* m_pScreenSpaceVS;           // 0x08
	TUINT32             m_aUnk0C[ 9 ];              // 0x0C never written on DX11
	ID3D11PixelShader*  m_pPSGaussBlur5x5;          // 0x30
	ID3D11PixelShader*  m_pPSGaussBlurAlpha;        // 0x34
	ID3D11PixelShader*  m_pPSGaussBlurAlphaStencil; // 0x38
	TUINT32             m_aUnk3C[ 10 ];             // 0x3C never written on DX11
	TUINT32*            m_pUnk64;                   // 0x64 new[] of 8 dwords
	TINT                m_iUnk68;                   // 0x68 8
	TINT                m_iUnk6C;                   // 0x6C 0
	TUINT64*            m_pUnk70;                   // 0x70 new[] of m_iUnk68 8 byte elements
	TINT                m_iUnk74;                   // 0x74 16
	TINT                m_iUnk78;                   // 0x78 0
};

TOSHI_NAMESPACE_END
