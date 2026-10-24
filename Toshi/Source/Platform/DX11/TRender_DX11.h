#pragma once
#include "Render/TRender.h"
#include "TMSWindow_DX11.h"
#include "TRenderAdapter_DX11.h"
#include "Render/TOrderTable.h"
#include "Toshi/T2Map.h"

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcommon.h>

TOSHI_NAMESPACE_START

class TVarianceShadowMaps;
class TPostProcess;
class TToneMap;
class TGlow;
class TFXAA;
class TDOF;

//-----------------------------------------------------------------------------
// The DirectX 11 renderer, created by ARenderer::CreateTRender.
//-----------------------------------------------------------------------------
class TRenderDX11 : public TRender
{
public:
	TDECLARE_CLASS( TRenderDX11, TRender );

	friend TMSWindow;

	// de Blob's main render target is 16:9 and at least this big (TRenderDX11::CreateDisplay)
	static TCONSTEXPR_VAR TUINT32 MIN_WIDTH  = 1280;
	static TCONSTEXPR_VAR TUINT32 MIN_HEIGHT = 720;

	static TCONSTEXPR_VAR TUINT32 VERTEX_CONSTANT_BUFFER_SIZE  = 0x1000;
	static TCONSTEXPR_VAR TUINT32 PIXEL_CONSTANT_BUFFER_SIZE   = 0x400;
	static TCONSTEXPR_VAR TUINT32 NUMBUFFERS                   = 16; // ring of constant buffers
	static TCONSTEXPR_VAR TUINT32 IMMEDIATE_VERTEX_BUFFER_SIZE = 0x100000;
	static TCONSTEXPR_VAR TUINT32 IMMEDIATE_INDEX_BUFFER_SIZE  = 0x10000;
	static TCONSTEXPR_VAR TUINT32 MAXIMUM_NUMBER_OF_MIPMAPS    = 15; // CreateTexture asserts numberOfMipmaps <= 15
	static TCONSTEXPR_VAR TUINT32 NUMSAMPLERSTATES             = 12;

	//-----------------------------------------------------------------------------
	// Render states. The current state is packed into small keys; UpdateRenderStates creates (and caches by key)
	// the D3D11 state objects and sets them when the key changed.
	//-----------------------------------------------------------------------------

	// D3D11_DEPTH_STENCIL_DESC in 7 bytes:
	//  byte 0: bit 0 DepthEnable, bit 1 DepthWriteMask, bits 2-5 DepthFunc, bit 6 StencilEnable
	//  byte 1: StencilReadMask, byte 2: StencilWriteMask
	//  byte 3: FrontFace StencilFailOp (low nibble), StencilDepthFailOp (high), byte 4: StencilPassOp (low), StencilFunc (high)
	//  bytes 5-6: the same for BackFace
	// NOTE: 12 bytes: uiDepth is only 4-byte aligned
#pragma pack( push, 4 )
	struct DEPTHSTATE
	{
		TUINT64 uiDepth;      // 0x00
		UINT    uiStencilRef; // 0x08

		TBOOL operator==( const DEPTHSTATE& a_rOther ) const { return uiDepth == a_rOther.uiDepth && uiStencilRef == a_rOther.uiStencilRef; }
		TBOOL operator!=( const DEPTHSTATE& a_rOther ) const { return !( *this == a_rOther ); }

		TUINT8&       Byte( TINT a_iIndex ) { return TREINTERPRETCAST( TUINT8*, &uiDepth )[ a_iIndex ]; }
		const TUINT8& Byte( TINT a_iIndex ) const { return TREINTERPRETCAST( const TUINT8*, &uiDepth )[ a_iIndex ]; }
	};
#pragma pack( pop )

	// The fields of DEPTHSTATE byte 0
	enum DEPTHFLAGS : TUINT8
	{
		DEPTHFLAGS_ENABLE       = BITFLAG( 0 ),
		DEPTHFLAGS_WRITESHIFT   = 1,
		DEPTHFLAGS_FUNCSHIFT    = 2,
		DEPTHFLAGS_STENCILSHIFT = 6,
		DEPTHFLAGS_STENCIL      = BITFLAG( DEPTHFLAGS_STENCILSHIFT ),
	};

	// D3D11_RASTERIZER_DESC: bits 0-1 FillMode, 2-3 CullMode, 4 FrontCounterClockwise, 5 DepthClipEnable,
	// 6 ScissorEnable, 7 MultisampleEnable (only the low byte of uiFlags is used)
	struct RASTERIZERSTATE
	{
		TUINT32 uiFlags;              // 0x00
		INT     DepthBias;            // 0x04
		FLOAT   SlopeScaledDepthBias; // 0x08

		TBOOL operator==( const RASTERIZERSTATE& a_rOther ) const { return TUINT8( uiFlags ) == TUINT8( a_rOther.uiFlags ) && DepthBias == a_rOther.DepthBias && SlopeScaledDepthBias == a_rOther.SlopeScaledDepthBias; }
		TBOOL operator!=( const RASTERIZERSTATE& a_rOther ) const { return !( *this == a_rOther ); }
	};

	// The fields of RASTERIZERSTATE::uiFlags
	enum RASTERIZERFLAGS : TUINT8
	{
		RASTERIZERFLAGS_FILLMASK         = 0x03,
		RASTERIZERFLAGS_CULLSHIFT        = 2,
		RASTERIZERFLAGS_CULLMASK         = 0x0C,
		RASTERIZERFLAGS_FRONTCCWSHIFT    = 4,
		RASTERIZERFLAGS_DEPTHCLIPSHIFT   = 5,
		RASTERIZERFLAGS_SCISSORSHIFT     = 6,
		RASTERIZERFLAGS_MULTISAMPLESHIFT = 7,
	};

	// The fields of BLENDSTATE::uiColour
	enum BLENDCOLOUR : TUINT16
	{
		BLENDCOLOUR_WRITEMASK_RGB    = 0x7,
		BLENDCOLOUR_WRITEMASK_ALPHA  = 0x8,
		BLENDCOLOUR_WRITEMASK        = 0xF,
		BLENDCOLOUR_BLENDENABLESHIFT = 4,
		BLENDCOLOUR_BLENDENABLE      = BITFLAG( BLENDCOLOUR_BLENDENABLESHIFT ),
		BLENDCOLOUR_SRCBLENDSHIFT    = 5,
		BLENDCOLOUR_DESTBLENDSHIFT   = 10,
	};

	// The fields of BLENDSTATE::uiAlpha
	enum BLENDALPHA : TUINT16
	{
		BLENDALPHA_BLENDOPMASK         = 0x7,
		BLENDALPHA_BLENDOPALPHASHIFT   = 3,
		BLENDALPHA_SRCBLENDALPHASHIFT  = 6,
		BLENDALPHA_SRCBLENDALPHAMASK   = 0x7C0,
		BLENDALPHA_DESTBLENDALPHASHIFT = 11,
	};

	// D3D11_RENDER_TARGET_BLEND_DESC in 32 bits:
	//  uiColour: bits 0-3 RenderTargetWriteMask, 4 BlendEnable, 5-9 SrcBlend, 10-14 DestBlend
	//  uiAlpha:  bits 0-2 BlendOp, 3-5 BlendOpAlpha, 6-10 SrcBlendAlpha, 11-15 DestBlendAlpha
	union BLENDSTATE
	{
		struct
		{
			TUINT16 uiColour; // 0x00
			TUINT16 uiAlpha;  // 0x02
		};

		TUINT32 uiRaw;
	};

	// Comparators of the state caches
	struct DepthStateComparator
	{
		// $deBlob: FUNCTION 006aa4f0
		// $deBlob: FUNCTION 006aa540
		TINT operator()( const TUINT64& a, const TUINT64& b ) const { return ( a == b ) ? 0 : ( a < b ? -1 : 1 ); }
	};

	struct RasterizerStateComparator
	{
		// $deBlob: FUNCTION 006aa450
		// $deBlob: FUNCTION 006aa4a0
		// $deBlob: FUNCTION 006aa5d0
		TINT operator()( const RASTERIZERSTATE& a, const RASTERIZERSTATE& b ) const
		{
			if ( TUINT8( a.uiFlags ) != TUINT8( b.uiFlags ) ) return TUINT8( a.uiFlags ) < TUINT8( b.uiFlags ) ? -1 : 1;
			if ( a.DepthBias != b.DepthBias ) return a.DepthBias < b.DepthBias ? -1 : 1;
			if ( b.SlopeScaledDepthBias > a.SlopeScaledDepthBias ) return -1;
			return a.SlopeScaledDepthBias > b.SlopeScaledDepthBias ? 1 : 0;
		}
	};

	struct BlendStateComparator
	{
		TINT operator()( TUINT32 a, TUINT32 b ) const { return TINT( a - b ); }
	};

	using DepthStateMap      = T2Map<TUINT64, ID3D11DepthStencilState*, DepthStateComparator>;
	using RasterizerStateMap = T2Map<RASTERIZERSTATE, ID3D11RasterizerState*, RasterizerStateComparator>;
	using BlendStateMap      = T2Map<TUINT32, ID3D11BlendState*, BlendStateComparator>;

	// Indices of m_aSamplerStates (see Initialize)
	enum SAMPLER_
	{
		SAMPLER_POINT_CLAMP,
		SAMPLER_LINEAR_CLAMP,
		SAMPLER_POINT_WRAP,
		SAMPLER_LINEAR_WRAP,
		SAMPLER_LINEAR_MIRROR,
		SAMPLER_BILINEAR_CLAMP,
		SAMPLER_BILINEAR_WRAP,
		SAMPLER_BILINEAR_WRAP_BIAS,
		SAMPLER_ANISOTROPIC_CLAMP,
		SAMPLER_POINT_WRAP_CLAMPV,
		SAMPLER_LINEAR_WRAP_CLAMPV,
		SAMPLER_BILINEAR_WRAP_CLAMPV,
		SAMPLER_NUMOF
	};

public:
	TRenderDX11();
	~TRenderDX11();

	//-----------------------------------------------------------------------------
	// TRender
	//-----------------------------------------------------------------------------
	virtual TBOOL                         CreateDisplay( const DISPLAYPARAMS& a_rParams ) OVERRIDE; // +0x0C
	virtual TBOOL                         DestroyDisplay() OVERRIDE;                                // +0x10
	virtual void                          Update( TFLOAT a_fDeltaTime ) OVERRIDE;                   // +0x14
	virtual void                          BeginScene() OVERRIDE;                                    // +0x18
	virtual void                          EndScene() OVERRIDE;                                      // +0x1C
	virtual TRenderAdapter::Mode::Device* GetCurrentDevice() OVERRIDE;                              // +0x20
	virtual DISPLAYPARAMS*                GetCurrentDisplayParams() OVERRIDE;                       // +0x24
	virtual TBOOL                         Create() OVERRIDE;                                        // +0x28
	virtual TBOOL                         Destroy() OVERRIDE;                                       // +0x2C
	virtual void                          GetScreenOffset( TVector2& a_rVec ) OVERRIDE;             // +0x34
	virtual void                          SetScreenOffset( const TVector2& a_rVec ) OVERRIDE;       // +0x38

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL RecreateDisplay( const DISPLAYPARAMS& a_rParams ); // +0x4C
	virtual void  ShowDeviceError();                                 // +0x50
	virtual void  ShowDisplayError();                                // +0x54

	// The a_uiFlags of CreateTexture
	enum TEXTUREFLAGS : TUINT8
	{
		TEXTUREFLAGS_NONE            = 0,
		TEXTUREFLAGS_MIPMAPSFROMDATA = BITFLAG( 0 ), // the mipmaps follow the first level in a_pSrcData
		TEXTUREFLAGS_GENERATEMIPMAPS = BITFLAG( 1 ),
		TEXTUREFLAGS_RENDERTARGET    = BITFLAG( 2 ),
	};

	// The a_uiType of CreateBuffer
	enum BUFFERTYPE : TUINT32
	{
		BUFFERTYPE_VERTEX   = 0,
		BUFFERTYPE_INDEX    = 1,
		BUFFERTYPE_CONSTANT = 2, // any other value
	};

	TBOOL Create( LPCSTR a_szTitle );

	// Creates a 2D texture and returns its shader resource view
	ID3D11ShaderResourceView* CreateTexture( TUINT32 a_uiWidth, TUINT32 a_uiHeight, DXGI_FORMAT a_eFormat, const void* a_pSrcData, TUINT8 a_uiFlags, D3D11_USAGE a_eUsage, TUINT32 a_uiCPUAccessFlags, TUINT32 a_uiSampleCount );

	// Copies a_uiDataSize bytes, a_uiRowSize per row, to the (dynamic) texture of a shader resource view
	void CopyDataToTexture( ID3D11ShaderResourceView* a_pShaderResourceView, TUINT32 a_uiDataSize, const void* a_pSrcData, TUINT32 a_uiRowSize );

	// Size of a row / of the whole texture of a_eFormat (blocks of 4x4 texels for BCn)
	static TUINT32 GetTextureRowPitch( DXGI_FORMAT a_eFormat, TUINT32 a_uiWidth );
	static TUINT32 GetTextureDepthPitch( DXGI_FORMAT a_eFormat, TUINT32 a_uiWidth, TUINT32 a_uiHeight );

	// Compiles HLSL source (the level 9 targets with D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY)
	static ID3DBlob* CompileShader( const TCHAR* a_szSrcData, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const D3D_SHADER_MACRO* a_pDefines );

	HRESULT CreateVertexShader( const void* a_pShaderBytecode, SIZE_T a_uiBytecodeLength, ID3D11VertexShader** a_ppVertexShader );
	// Create a shader from a file of compiled bytecode
	HRESULT CreateVertexShaderFromFile( const TCHAR* a_szFileName, ID3D11VertexShader** a_ppVertexShader );
	HRESULT CreatePixelShaderFromFile( const TCHAR* a_szFileName, ID3D11PixelShader** a_ppPixelShader );
	HRESULT CreatePixelShader( const void* a_pShaderBytecode, SIZE_T a_uiBytecodeLength, ID3D11PixelShader** a_ppPixelShader );
	void    CreateUnkShader( void* a_pShaderData, void** a_ppOutShader );
	void    CreateFXShader( void* a_pShaderData, void** a_ppOutShader );

	// Creates the shaders, input layout and quad of the overlay
	void CompileVSPS();

	// Draws a_pShaderResourceView at (a_fPosX, a_fPosY) of the viewport (pixels), a_pUVST: uv scale (xy) and offset (zw)
	static void RenderOverlay( TFLOAT a_fPosX, TFLOAT a_fPosY, TFLOAT a_fWidth, TFLOAT a_fHeight, ID3D11ShaderResourceView* a_pShaderResourceView, ID3D11PixelShader* a_pPixelShader, const TVector4* a_pUVST );

	void DrawNonIndexed( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, ID3D11Buffer* a_pVertexBuffer, UINT a_uiVertexCount, UINT a_uiStride, UINT a_uiStartVertex, UINT a_uiOffset );
	void DrawIndexed( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, ID3D11Buffer* a_pIndexBuffer, UINT a_uiIndexBufferOffset, DXGI_FORMAT a_eIndexFormat, ID3D11Buffer* a_pVertexBuffer, UINT a_uiStride, UINT a_uiOffset );

	// Draw calls with the vertices and/or indices copied to the immediate buffers
	void DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, const void* a_pVertices, UINT a_uiVertexCount, UINT a_uiStride, UINT a_uiStartVertex );
	void DrawImmediately( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, const void* a_pIndices, DXGI_FORMAT a_eIndexFormat, const void* a_pVertices, UINT a_uiStride, UINT a_uiVertexCount );
	void DrawIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, const void* a_pIndices, DXGI_FORMAT a_eIndexFormat, ID3D11Buffer* a_pVertexBuffer, UINT a_uiStride, UINT a_uiOffset );

	// Copies vertices to the immediate vertex buffer and binds it to the stream a_uiSlot
	void SetImmediateVertexStream( const void* a_pVertices, UINT a_uiStride, UINT a_uiVertexCount, UINT a_uiSlot );

	// Escape and Alt+Enter
	void CreateAccelerators();
	void DestroyAccelerators();

	// Creates a render target view of the texture of a shader resource view
	ID3D11RenderTargetView* CreateRenderTargetView( ID3D11ShaderResourceView* a_pShaderResourceView );

	// Flushes all the order tables (renders the scene)
	void FlushOrderTables();

	// Creates the sampler states, constant and immediate buffers and sets the default render states
	void Initialize();

	// Releases what Initialize created and the cached render states
	void DestroyRenderStates();

	// Creates a sampler state (anisotropic x16 instead of trilinear)
	ID3D11SamplerState* CreateSamplerStateAutoAnisotropy( D3D11_FILTER a_eFilter, D3D11_TEXTURE_ADDRESS_MODE a_eAddressU, D3D11_TEXTURE_ADDRESS_MODE a_eAddressV, D3D11_TEXTURE_ADDRESS_MODE a_eAddressW, FLOAT a_fMipLODBias, TUINT32 a_uiBorderColour, FLOAT a_fMinLOD, FLOAT a_fMaxLOD );

	ID3D11Buffer* CreateBuffer( TUINT32 a_uiType, UINT a_uiDataSize, const void* a_pData, D3D11_USAGE a_eUsage, UINT a_uiCPUAccessFlags );

	// Copies a_iCount vectors to the constant buffers (uploaded by FlushConstantBuffers)
	void SetVec4InVSBuffer( TINT a_iIndex, const void* a_pSrc, TINT a_iCount = 1 );
	void SetVec4InPSBuffer( TINT a_iIndex, const void* a_pSrc, TINT a_iCount = 1 );

	// Uploads the modified constant buffers (to the next buffer of the rings) and binds them
	void FlushConstantBuffers();

	// Creates and sets the D3D11 states of the current render states that changed
	void UpdateRenderStates();

	void SetSamplerState( UINT a_uiStartSlot, TINT a_iSamplerId, BOOL a_bPixelShader );
	void SetSamplerState( UINT a_uiStartSlot, ID3D11SamplerState* a_pSamplerState, BOOL a_bPixelShader );

	void SetColorUpdate( TBOOL a_bUpdate );
	void SetAlphaUpdate( TBOOL a_bUpdate );
	void SetDstAlpha( TFLOAT a_fAlpha );
	// The following have
	// The alpha of SetDstAlpha (the blend factor's)
	TFLOAT GetDstAlpha() const;
	// Clears the depth of the depth stencil view to 1
	void ClearDepth();
	// Deletes the adapters of the adapter database
	void DestroyAdapters();
	void SetBlendMode( TBOOL a_bBlendEnabled, D3D11_BLEND_OP a_eBlendOp, D3D11_BLEND a_eSrcBlend, D3D11_BLEND a_eDestBlend );
	void SetZMode( TBOOL a_bDepthEnable, D3D11_COMPARISON_FUNC a_eComparisonFunc, D3D11_DEPTH_WRITE_MASK a_eDepthWriteMask );
	void SetStencil( TBOOL a_bEnable, D3D11_COMPARISON_FUNC a_eFrontFunc, UINT a_uiStencilRef, TUINT8 a_uiMask, D3D11_STENCIL_OP a_eFailOp, D3D11_STENCIL_OP a_eDepthFailOp, D3D11_STENCIL_OP a_ePassOp );
	void SetDepthBias( TFLOAT a_fDepth, TFLOAT a_fBias );

	// Copies a_uiSize bytes to a_pBuffer at a_uiOffset (Map with a_eMapType)
	void UpdateBuffer( ID3D11Buffer* a_pBuffer, TUINT32 a_uiSize, const void* a_pSrc, TUINT32 a_uiOffset, D3D11_MAP a_eMapType );

	// Copies a_uiSize bytes to a_pBuffer at a_uiOffset (UpdateSubresource)
	void UpdateBufferRegion( ID3D11Buffer* a_pBuffer, TUINT32 a_uiSize, const void* a_pSrc, TUINT32 a_uiOffset );

	// Releases the shaders and the quad of the overlay (CompileVSPS)
	void DestroyOverlayShaders();

	// Waits for the GPU to finish the submitted commands
	void WaitForEndOfRender();

	// The device was removed (Present): invalidates every resource and shader, calls the device lost callback and
	// flushes the dying resources (with the system paused)
	void OnDeviceLost();
	void OnDeviceReset();

	void SetDeviceLostCallback( void ( *a_fnCallback )() ) { m_fnDeviceLostCallback = a_fnCallback; }

	// Resolves the (multisampled) main render target
	void ResolveSubresource();

	TPriList<TOrderTable>& GetOrderTables() { return m_OrderTables; }
	// $deBlob: FUNCTION 006a7de0
	TINT                   GetNumAdapters() { return GetAdapterList()->Count(); }

	static TBOOL IsColorEqual( const FLOAT a_aColour1[ 4 ], const FLOAT a_aColour2[ 4 ] );

	TMSWindow* GetMSWindow();
	// $deBlob: FUNCTION 006a6230
	HACCEL GetAccelTable() const { return m_hAccel; }
	// $deBlob: FUNCTION 006a6240
	void                      SetUnk6D4() { m_bUnk6D4 = TTRUE; }
	// $deBlob: FUNCTION 00552010
	ID3D11Device*             GetDevice() const { return m_pDevice; }
	// $deBlob: FUNCTION 00550c90
	ID3D11DeviceContext*      GetDeviceContext() const { return m_pDeviceContext; }
	// $deBlob: FUNCTION 006b1680
	IDXGISwapChain*           GetSwapChain() const { return m_pSwapChain; }
	ID3D11RenderTargetView*   GetMainRenderTargetView() const { return m_pMainRTView; }
	ID3D11Texture2D*          GetBackBuffer() const { return m_pBackBuffer; }
	ID3D11Texture2D*          GetMainTexture() const { return m_pMainTexture; }
	DXGI_SWAP_CHAIN_DESC*     GetSwapChainDesc() { return &m_SwapChainDesc; }
	TVarianceShadowMaps*      GetVarianceShadowMaps() const { return m_pVarianceShadowMaps; }
	// $deBlob: FUNCTION 006b4d20
	TPostProcess*             GetPostProcess() const { return m_pPostProcess; }
	TToneMap*                 GetToneMap() const { return m_pToneMap; }
	TGlow*                    GetGlow() const { return m_pGlow; }
	TFXAA*                    GetFXAA() const { return m_pFXAA; }
	TDOF*                     GetDOF() const { return m_pDOF; }
	ID3D11ShaderResourceView* GetMainSRView() const { return m_pMainSRView; }
	ID3D11ShaderResourceView* GetResolveSRView() const { return m_pResolveSRView; }
	// $deBlob: FUNCTION 00706810
	TUINT32          GetNumDrawnFrames() const { return m_uiNumDrawnFrames; }
	RASTERIZERSTATE& GetCurrentRasterizer() { return m_CurrentRasterizer; }
	DEPTHSTATE&      GetCurrentDepth() { return m_CurrentDepth; }

	static const TCHAR* GetFeatureLevel( D3D_FEATURE_LEVEL a_eFeatureLevel );

	static TRenderDX11* Interface() { return TSTATICCAST( TRenderDX11, TRender::GetSingleton() ); }

private:
	// Adds a TD3DAdapter (and its modes) to the adapter list for each DXGI adapter
	void BuildAdapterDatabase();

public:
	static TBOOL s_bPresentTest;  // Present( 1, DXGI_PRESENT_TEST ) while occluded
	static TINT  s_iQualityLevel; // "quality" of setup.ini: 1 = half size, >1 = 1280x720

	static ID3D11ShaderResourceView* s_pMainTextureSRV; // shader resource view of the main RT for the 16:9 overlay

private:
	DXGI_SWAP_CHAIN_DESC      m_SwapChainDesc;       // 0x61C
	ID3D11Texture2D*          m_pBackBuffer;         // 0x658 buffer 0 of the swap chain
	ID3D11RenderTargetView*   m_pBackBufferRTView;   // 0x65C
	ID3D11DepthStencilView*   m_pDepthStencilView;   // 0x660
	IDXGISwapChain*           m_pSwapChain;          // 0x664
	TUINT32                   m_uiDisplayWidth;      // 0x668 size of the back buffer (the window)
	TUINT32                   m_uiDisplayHeight;     // 0x66C
	ID3D11Texture2D*          m_pMainTexture;        // 0x670 main render target (16:9, multisampled)
	ID3D11RenderTargetView*   m_pMainRTView;         // 0x674
	ID3D11ShaderResourceView* m_pMainSRView;         // 0x678
	ID3D11Texture2D*          m_pResolveTexture;     // 0x67C resolved copy of the main render target
	ID3D11ShaderResourceView* m_pResolveSRView;      // 0x680
	ID3D11ShaderResourceView* m_pDepthStencilSRV;    // 0x684 stencil of the depth buffer
	TBOOL                     m_bIsWidescreen;       // 0x688 the main RT doesn't have the size of the back buffer
	ID3D11DeviceContext*      m_pDeviceContext;      // 0x68C
	ID3D11Device*             m_pDevice;             // 0x690
	HACCEL                    m_hAccel;              // 0x694
	DISPLAYPARAMS             m_DisplayParams;       // 0x698
	TMSWindow                 m_Window;              // 0x6B0
	TBOOL                     m_bUnk6D4;             // 0x6D4
	ID3D11VertexShader*       m_pOverlayVS;          // shaders and quad of the overlay
	ID3D11PixelShader*        m_pOverlayPS;          // 0x6DC
	ID3D11PixelShader*        m_pOverlayOpaquePS;    // 0x6E0
	ID3D11InputLayout*        m_pOverlayLayout;      // 0x6E4
	ID3D11Buffer*             m_pOverlayQuad;        // 0x6E8
	FLOAT                     m_aClearColour[ 4 ];   // 0x6EC
	TUINT32                   m_aUnk6FC[ 6 ];        // 0x6FC
	TVarianceShadowMaps*      m_pVarianceShadowMaps; // 0x714
	TPostProcess*             m_pPostProcess;        // 0x718
	TToneMap*                 m_pToneMap;            // 0x71C
	TGlow*                    m_pGlow;               // 0x720
	TFXAA*                    m_pFXAA;               // 0x724
	TDOF*                     m_pDOF;                // 0x728
	TUINT32                   m_uiNumDrawnFrames;    // 0x72C
	TBOOL                     m_bDeviceLost;         // 0x730
	void ( *m_fnDeviceLostCallback )();              // 0x734
	TUINT32 m_uiUnk738;                              // 0x738
	// 0x73C..0x8D0 sampler states, constant buffers, immediate buffers, order tables, render state trees
	ID3D11SamplerState*   m_aSamplerStates[ NUMSAMPLERSTATES ]; // 0x73C
	TUINT8*               m_pVertexConstantBuffer;              // 0x76C
	TBOOL                 m_bVertexConstantBufferSet;           // 0x770
	ID3D11Buffer*         m_aVertexBuffers[ NUMBUFFERS ];       // 0x774 constant buffers of the vertex shaders
	TUINT32               m_uiVertexBufferIndex;                // 0x7B4
	TUINT8*               m_pPixelConstantBuffer;               // 0x7B8
	TBOOL                 m_bPixelConstantBufferSet;            // 0x7BC
	ID3D11Buffer*         m_aPixelBuffers[ NUMBUFFERS ];        // 0x7C0 constant buffers of the pixel shaders
	TUINT32               m_uiPixelBufferIndex;                 // 0x800
	ID3D11Buffer*         m_pImmediateVertexBuffer;             // 0x804
	TUINT32               m_uiImmediateVertexCurrentOffset;     // 0x808
	ID3D11Buffer*         m_pImmediateIndexBuffer;              // 0x80C
	TUINT32               m_uiImmediateIndexCurrentOffset;      // 0x810
	TPriList<TOrderTable> m_OrderTables;                        // 0x814
	TUINT32               m_uiUnk81C;                           // 0x81C
	DEPTHSTATE            m_CurrentDepth;                       // 0x820
	// NOTE: in retail the previous depth state is no DEPTHSTATE: it's copied as 3 dwords and m_DepthStates follows
	// it at 0x83C (a DEPTHSTATE is 0x10)
	TUINT64               m_uiPreviousDepth;                    // 0x830
	UINT                  m_uiPreviousStencilRef;               // 0x838
	DepthStateMap         m_DepthStates;                        // 0x83C
	RASTERIZERSTATE       m_CurrentRasterizer;                  // 0x858
	RASTERIZERSTATE       m_PreviousRasterizer;                 // 0x864
	RasterizerStateMap    m_RasterizerStates;                   // 0x870
	BLENDSTATE            m_CurrentBlendState;                  // 0x88C
	FLOAT                 m_aCurrentBlendFactor[ 4 ];           // 0x890
	BLENDSTATE            m_PreviousBlendState;                 // 0x8A0
	FLOAT                 m_aPreviousBlendFactor[ 4 ];          // 0x8A4
	BlendStateMap         m_BlendStates;                        // 0x8B4
};

TOSHI_NAMESPACE_END
