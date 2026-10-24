#include "ToshiPCH.h"
#include "TRender_DX11.h"
#include "TVarianceShadowMaps_DX11.h"
#include "TPostProcess_DX11.h"
#include "TTonemap_DX11.h"
#include "TGlow_DX11.h"
#include "TFXAA_DX11.h"
#include "TDOF_DX11.h"
#include "TRenderContext_DX11.h"
#include "TDevScreenshot_DX11.h"
#include "TTexture_DX11.h"
#include "TMaterial_DX11.h"
#include "TVertexDecl_DX11.h"
#include "TFXManager_DX11.h"
#include "TFXConstant_DX11.h"
#include "Render/TResource.h"
#include "Render/TShader.h"

#include <d3dcompiler.h>
#include "Toshi/TSystem.h"
#include "Toshi/TUtil.h"
#include "Toshi/TStringManager.h"
#include "Toshi/T2MemoryAllocator.h"

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "dxgi.lib" )

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TBOOL                     TRenderDX11::s_bPresentTest    = TFALSE;
TINT                      TRenderDX11::s_iQualityLevel   = -1;
ID3D11ShaderResourceView* TRenderDX11::s_pMainTextureSRV = TNULL;

// $deBlob: FUNCTION 006a6220
TMSWindow* TRenderDX11::GetMSWindow()
{
	return &m_Window;
}

// $deBlob: FUNCTION 0053ec20
// $deBlob: FUNCTION 00858ef0
TDEFINE_CLASS( TRenderDX11 );

// 0x009a87bc: the "render states" heap of the state caches and the CPU copies of the constant buffers
static TMemoryHeap* s_pMemHeap = TNULL;

// 0x00981ea4: allocator of the state caches (its heap at 0x00981ea8)
static T2MemoryAllocator s_oRenderStatesAllocator;

static T2Allocator* GetRenderStatesAllocator()
{
	if ( s_pMemHeap == TNULL )
	{
		s_pMemHeap = TMemory_CreateHeap( 0x10000, 0, "render states" );
		s_oRenderStatesAllocator.SetHeap( s_pMemHeap );
	}

	return &s_oRenderStatesAllocator;
}

// Shaders of the overlay (CompileVSPS)
static const TCHAR* s_szOverlayVS =
    " float4 ScaleTranslate : register(c0);               float4 uvST : register(c1);                    "
    "                                         struct VS_IN                                               "
    "                                 {                                                 float4 ObjPos   :"
    " POSITION;                   float2 UV\t\t  : TEXCOORD0;\t\t\t    };                                "
    "                                                          struct VS_OUT                             "
    "    {                                                 float4 ProjPos  : SV_POSITION;                "
    "   float2 UV\t\t  : TEXCOORD0;\t\t\t    };                                                          "
    "                                VS_OUT main( VS_IN In )                       {                     "
    "                            VS_OUT Out;                                   float4 scaledvert = In.Obj"
    "Pos.xyzw * ScaleTranslate.xyzw;         scaledvert = scaledvert.xyzw + ScaleTranslate.zwzw;\t\t\t\t "
    "       scaledvert.zw = float2(0.0,1.0);\t\t\t\t\t\t\t\t     Out.ProjPos = scaledvert;\t\t\t\t       "
    " Out.UV = (In.UV*uvST.xy) + uvST.zw;           return Out;                               }          "
    "                                  ";

static const TCHAR* s_szOverlayPS =
    " struct PS_IN                                  {                                            \t\tfloa"
    "t4 Position\t\t\t: SV_POSITION; \t\tfloat2 Tex0             : TEXCOORD0;    };                      "
    "                                                                    sampler2D   diffuse_texture     "
    ": register(s0) = sampler_state { MipFilter = NONE; MinFilter = LINEAR; MagFilter = LINEAR; AddressU "
    "= WRAP; AddressV = WRAP; };   float4 main( PS_IN In )  : COLOR\t\t\t    {                           "
    "                     return tex2D( diffuse_texture, In.Tex0 );        }                             "
    "               ";

static const TCHAR* s_szOverlayOpaquePS =
    " struct PS_IN                                  {                                            \t\tfloa"
    "t4 Position\t\t\t: SV_POSITION; \t\tfloat2 Tex0             : TEXCOORD0;    };                      "
    "                                                                    sampler2D   diffuse_texture     "
    ": register(s0) = sampler_state { MipFilter = NONE; MinFilter = LINEAR; MagFilter = LINEAR; AddressU "
    "= WRAP; AddressV = WRAP; };   float4 main( PS_IN In )  : COLOR\t\t\t    {                           "
    "                     return float4(tex2D( diffuse_texture, In.Tex0 ).xyz, 1.0);        }            "
    "                                ";

static const D3D_DRIVER_TYPE s_aDriverTypes[] = {
	D3D_DRIVER_TYPE_HARDWARE,
	D3D_DRIVER_TYPE_WARP,
	D3D_DRIVER_TYPE_REFERENCE,
};

static const D3D_FEATURE_LEVEL s_aFeatureLevels[] = {
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
};

// $deBlob: FUNCTION 006a46e0
TRenderDX11::TRenderDX11()
    : m_DepthStates( GetRenderStatesAllocator() )
    , m_RasterizerStates( GetRenderStatesAllocator() )
    , m_BlendStates( GetRenderStatesAllocator() )
{
	// NOTE: GetRenderStatesAllocator (TMemory_CreateHeap) is inlined before each state cache's constructor

	m_pBackBuffer       = TNULL;
	m_pBackBufferRTView = TNULL;
	m_pDepthStencilView = TNULL;
	m_pSwapChain        = TNULL;
	m_pMainTexture      = TNULL;
	m_pMainRTView       = TNULL;
	m_pMainSRView       = TNULL;
	m_pResolveTexture   = TNULL;
	m_pResolveSRView    = TNULL;
	m_pDepthStencilSRV  = TNULL;
	m_bIsWidescreen     = TFALSE;
	m_pDeviceContext    = TNULL;
	m_pDevice           = TNULL;

	m_DisplayParams.uiWidth                   = MIN_WIDTH;
	m_DisplayParams.uiHeight                  = MIN_HEIGHT;
	m_DisplayParams.uiColourDepth             = 32;
	m_DisplayParams.eDepthStencilFormat       = DEPTHSTENCILFORMAT_D24_S8;
	m_DisplayParams.bWindowed                 = TTRUE;
	m_DisplayParams.bFullscreen               = TTRUE;
	m_DisplayParams.uiMultisampleQualityLevel = 1;

	m_bUnk6D4              = TFALSE;
	m_uiNumDrawnFrames     = 0;
	m_bDeviceLost          = TFALSE;
	m_fnDeviceLostCallback = TNULL;
	m_uiUnk738             = 0;

	m_aClearColour[ 3 ] = 0.0f;
	m_aClearColour[ 2 ] = 0.0f;
	m_aClearColour[ 1 ] = 0.0f;
	m_aClearColour[ 0 ] = 0.0f;

	TUtil::MemClear( m_aSamplerStates, sizeof( m_aSamplerStates ) );

	// NOTE: not in retail's constructor (it's created by Create)
	m_hAccel = NULL;
#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: not in retail's constructor (set by CompileVSPS/CreateDisplay)
	m_pOverlayVS          = TNULL;
	m_pOverlayPS          = TNULL;
	m_pOverlayOpaquePS    = TNULL;
	m_pOverlayLayout      = TNULL;
	m_pOverlayQuad        = TNULL;
	m_pVarianceShadowMaps = TNULL;
	m_pPostProcess        = TNULL;
	m_pToneMap            = TNULL;
	m_pGlow               = TNULL;
	m_pFXAA               = TNULL;
	m_pDOF                = TNULL;
#endif

	TREINTERPRETCAST( TUINT8*, &m_CurrentRasterizer.uiFlags )
	[ 0 ]                                    = 0;
	m_CurrentRasterizer.DepthBias            = 0;
	m_CurrentRasterizer.SlopeScaledDepthBias = 0.0f;
	TREINTERPRETCAST( TUINT8*, &m_PreviousRasterizer.uiFlags )
	[ 0 ]                                     = 0;
	m_PreviousRasterizer.DepthBias            = 0;
	m_PreviousRasterizer.SlopeScaledDepthBias = 0.0f;

	m_aCurrentBlendFactor[ 0 ]  = 1.0f;
	m_aCurrentBlendFactor[ 1 ]  = 1.0f;
	m_aCurrentBlendFactor[ 2 ]  = 1.0f;
	m_aCurrentBlendFactor[ 3 ]  = 1.0f;
	m_aPreviousBlendFactor[ 0 ] = 1.0f;
	m_aPreviousBlendFactor[ 1 ] = 1.0f;
	m_aPreviousBlendFactor[ 2 ] = 1.0f;
	m_aPreviousBlendFactor[ 3 ] = 1.0f;

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail leaves these uninitialised (Initialize keeps some of their bits)
	m_CurrentRasterizer.uiFlags  = 0;
	m_PreviousRasterizer.uiFlags = 0;
	m_CurrentBlendState.uiRaw    = 0;
	m_PreviousBlendState.uiRaw   = 0;
	m_uiPreviousDepth      = 0;
	m_uiPreviousStencilRef = 0;
#endif

	new TVertexDecl_DX11();
	new TFXManager_DX11();

	new TMaterialManager();

	m_pVertexConstantBuffer = TNULL;
	TUtil::MemClear( m_aVertexBuffers, sizeof( m_aVertexBuffers ) );
	m_bVertexConstantBufferSet = TFALSE;
	m_pPixelConstantBuffer     = TNULL;
	TUtil::MemClear( m_aPixelBuffers, sizeof( m_aPixelBuffers ) );
	m_bPixelConstantBufferSet        = TFALSE;
	m_pImmediateVertexBuffer         = TNULL;
	m_uiImmediateVertexCurrentOffset = 0;
	m_pImmediateIndexBuffer          = TNULL;
	m_uiImmediateIndexCurrentOffset  = 0;

	TFXConstant_DX11::InitTypes();

	m_CurrentDepth.uiDepth      = 0;
	m_CurrentDepth.uiStencilRef = 0;
}

// $deBlob: FUNCTION 006a4a20
// $deBlob: FUNCTION 006aa920
TRenderDX11::~TRenderDX11()
{
	if ( TTextureManager::GetSingleton() != TNULL )
		delete TTextureManager::GetSingleton();

	if ( TMaterialManager::GetSingleton() != TNULL )
		delete TMaterialManager::GetSingleton();

	if ( TFXManager_DX11::GetSingleton() != TNULL )
		delete TFXManager_DX11::GetSingleton();

	if ( TVertexDecl_DX11::GetSingleton() != TNULL )
		delete TVertexDecl_DX11::GetSingleton();

	Destroy();

	// NOTE: retail's condition is inverted, so the "render states" heap is never destroyed (the state caches below
	// still free their nodes into it)
	if ( s_pMemHeap == TNULL )
	{
		TMemory_DestroyHeap( s_pMemHeap );
		s_pMemHeap = TNULL;
	}

	m_OrderTables.RemoveAll();
}

// $deBlob: FUNCTION 006a4600
TRenderContext* TRender::CreateRenderContext()
{
	return new TRenderContextDX11( this );
}

// $deBlob: FUNCTION 006a4630
void* TRender::CreateScreenCapture()
{
	return TNULL;
}

// $deBlob: FUNCTION 006a4640
void TRender::DestroyScreenCapture( void* a_pCapture )
{
}

// $deBlob: FUNCTION 006a4b00
TBOOL TRenderDX11::CreateDisplay( const DISPLAYPARAMS& a_rParams )
{
	TASSERT( TTRUE == IsCreated() );
	TASSERT( TFALSE == IsDisplayCreated() );

	if ( !TRender::CreateDisplay() )
	{
		ShowDisplayError();
		return TFALSE;
	}

	m_DisplayParams = a_rParams;

	// The desktop mode of the first output of the first adapter
	TD3DAdapter::Mode* pMode = TSTATICCAST( TD3DAdapter, GetAdapterList()->Head().Get() )->GetMode();

	if ( !m_DisplayParams.bWindowed && !m_DisplayParams.bFullscreen )
	{
		// Borderless window of the desktop's size
		m_DisplayParams.uiWidth  = pMode->GetWidth();
		m_DisplayParams.uiHeight = pMode->GetHeight();
	}

	HWND hWnd = m_Window.GetHWND();

	IDXGIFactory1* pFactory     = TNULL;
	IDXGIDevice*   pDXGIDevice  = TNULL;
	IDXGIAdapter*  pDXGIAdapter = TNULL;
	m_pDevice->QueryInterface( __uuidof( IDXGIDevice ), (void**)&pDXGIDevice );
	pDXGIDevice->GetAdapter( &pDXGIAdapter );
	pDXGIAdapter->GetParent( __uuidof( IDXGIFactory1 ), (void**)&pFactory );

	DXGI_MODE_DESC oFoundMode = {};

	if ( !GetCurrentDisplayParams()->bWindowed )
	{
		// NOTE: retail never releases the output
		IDXGIOutput* pOutput = TNULL;
		pDXGIAdapter->EnumOutputs( 0, &pOutput );

		DXGI_MODE_DESC oMatchMode = {};
		oMatchMode.Width          = GetCurrentDisplayParams()->uiWidth;
		oMatchMode.Height         = GetCurrentDisplayParams()->uiHeight;
		oMatchMode.Format         = DXGI_FORMAT_R8G8B8A8_UNORM;
		pOutput->FindClosestMatchingMode( &oMatchMode, &oFoundMode, NULL );

		m_DisplayParams.uiWidth  = oFoundMode.Width;
		m_DisplayParams.uiHeight = oFoundMode.Height;
	}

	pDXGIAdapter->Release();
	pDXGIDevice->Release();

	memset( &m_SwapChainDesc, 0, sizeof( m_SwapChainDesc ) );
	m_SwapChainDesc.BufferCount       = 1;
	m_SwapChainDesc.BufferDesc.Width  = m_DisplayParams.uiWidth;
	m_SwapChainDesc.BufferDesc.Height = m_DisplayParams.uiHeight;

	if ( !m_DisplayParams.bFullscreen )
		oFoundMode.Format = pMode->GetDescription()->Format;

	m_SwapChainDesc.BufferDesc.Format = oFoundMode.Format;

	const DXGI_MODE_DESC* pRefreshMode     = m_DisplayParams.bFullscreen ? &oFoundMode : pMode->GetDescription();
	m_SwapChainDesc.BufferDesc.RefreshRate = pRefreshMode->RefreshRate;
	m_SwapChainDesc.OutputWindow           = hWnd;
	m_SwapChainDesc.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_SwapChainDesc.SampleDesc.Count       = 1;
	m_SwapChainDesc.SampleDesc.Quality     = 0;
	m_SwapChainDesc.Windowed               = TRUE;
	m_SwapChainDesc.SwapEffect             = DXGI_SWAP_EFFECT_DISCARD;
	m_SwapChainDesc.Flags                  = m_DisplayParams.bFullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

	pFactory->CreateSwapChain( m_pDevice, &m_SwapChainDesc, &m_pSwapChain );
	pFactory->MakeWindowAssociation( hWnd, DXGI_MWA_NO_ALT_ENTER );
	pFactory->Release();

	if ( m_pDeviceContext == TNULL || m_pDevice == TNULL || m_pSwapChain == TNULL )
	{
		ShowDeviceError();
		return TFALSE;
	}

	const TUINT32 uiRefreshNumerator   = m_SwapChainDesc.BufferDesc.RefreshRate.Numerator;
	const TUINT32 uiRefreshDenominator = m_SwapChainDesc.BufferDesc.RefreshRate.Denominator;
	const TFLOAT  fRefreshRate         = TFLOAT( uiRefreshNumerator ) / TFLOAT( uiRefreshDenominator );

	TCHAR szDisplayName[ 128 ];
	TUtil::Log( "CurrentDisplay: %s ", TStringManager::StringUnicodeToChar( szDisplayName, pMode->GetDisplayName(), -1 ) );
	TUtil::Log( "CurrentDisplayMode = %d x %d (%.4f Hz) = [%d / %d]", pMode->GetDescription()->Width, pMode->GetDescription()->Height, fRefreshRate, uiRefreshNumerator, uiRefreshDenominator );

	if ( !GetCurrentDisplayParams()->bFullscreen )
	{
		TUtil::Log( "Set Windowed %d x %d ", GetCurrentDisplayParams()->uiWidth, GetCurrentDisplayParams()->uiHeight );
		m_Window.SetWindowed();
	}
	else
	{
		TUtil::Log( "Set Fullscreen %d x %d (%.4f Hz) = [%d / %d]", GetCurrentDisplayParams()->uiWidth, GetCurrentDisplayParams()->uiHeight, fRefreshRate, uiRefreshNumerator, uiRefreshDenominator );
		m_Window.SetFullscreen();
		m_pSwapChain->SetFullscreenState( TRUE, NULL );
	}

	// Centre of the desktop for windowed mode
	TINT iWindowX = 0;
	TINT iWindowY = 0;

	if ( GetCurrentDisplayParams()->bWindowed )
	{
		iWindowX = TINT( pMode->GetWidth() - m_DisplayParams.uiWidth ) / 2;
		iWindowY = TINT( pMode->GetHeight() - m_DisplayParams.uiHeight ) / 2;
	}

	if ( !m_DisplayParams.bWindowed && !m_DisplayParams.bFullscreen )
	{
		SetWindowLongA( m_Window.GetHWND(), GWL_STYLE, GetWindowLongA( m_Window.GetHWND(), GWL_STYLE ) & 0xde33ffff );
		SetWindowPos( m_Window.GetHWND(), NULL, 0, 0, m_DisplayParams.uiWidth, m_DisplayParams.uiHeight, 0 );
	}
	else
	{
		m_Window.SetPosition( iWindowX, iWindowY, m_DisplayParams.uiWidth, m_DisplayParams.uiHeight );
	}

	m_uiDisplayWidth  = m_DisplayParams.uiWidth;
	m_uiDisplayHeight = m_DisplayParams.uiHeight;

	// The main render target is 16:9
	const TUINT32 uiExpectedWidth = ( TINT( m_DisplayParams.uiHeight ) / 9 ) * 16;

	if ( m_DisplayParams.uiWidth != uiExpectedWidth )
	{
		m_bIsWidescreen = TTRUE;

		if ( TINT( uiExpectedWidth ) < TINT( m_DisplayParams.uiWidth ) )
		{
			m_DisplayParams.uiWidth          = uiExpectedWidth;
			m_SwapChainDesc.BufferDesc.Width = uiExpectedWidth;
		}
		else
		{
			m_DisplayParams.uiHeight          = ( TINT( m_DisplayParams.uiWidth ) / 16 ) * 9;
			m_SwapChainDesc.BufferDesc.Height = m_DisplayParams.uiHeight;
		}
	}

	if ( s_iQualityLevel == 1 )
	{
		// Half size, between 1280x720 and 1920x1080
		m_bIsWidescreen = TTRUE;

		TINT iHalfWidth  = TINT( m_DisplayParams.uiWidth ) / 2;
		TINT iHalfHeight = TINT( m_DisplayParams.uiHeight ) / 2;

		if ( iHalfWidth > 1920 )
		{
			iHalfWidth  = 1920;
			iHalfHeight = 1080;
		}

		if ( iHalfWidth < TINT( MIN_WIDTH ) ) iHalfWidth = MIN_WIDTH;
		m_DisplayParams.uiWidth = iHalfWidth;

		if ( iHalfHeight < TINT( MIN_HEIGHT ) ) iHalfHeight = MIN_HEIGHT;
		m_DisplayParams.uiHeight = iHalfHeight;
	}
	else if ( s_iQualityLevel > 0 )
	{
		m_bIsWidescreen          = TTRUE;
		m_DisplayParams.uiWidth  = MIN_WIDTH;
		m_DisplayParams.uiHeight = MIN_HEIGHT;
	}

	if ( TINT( m_DisplayParams.uiWidth ) < TINT( MIN_WIDTH ) ) m_DisplayParams.uiWidth = MIN_WIDTH;
	if ( TINT( m_DisplayParams.uiHeight ) < TINT( MIN_HEIGHT ) ) m_DisplayParams.uiHeight = MIN_HEIGHT;

	m_SwapChainDesc.BufferDesc.Width  = m_DisplayParams.uiWidth;
	m_SwapChainDesc.BufferDesc.Height = m_DisplayParams.uiHeight;

	// Number of samples of the main render target: 1, 2, 4 or 8, the highest one the device supports
	TUINT32 uiSampleCount = m_DisplayParams.uiMultisampleQualityLevel;

	if ( uiSampleCount < 2 )
	{
		m_DisplayParams.uiMultisampleQualityLevel = 1;
	}
	else
	{
		if ( uiSampleCount != 2 && uiSampleCount != 4 && uiSampleCount != 8 )
			m_DisplayParams.uiMultisampleQualityLevel = 4;

		for ( uiSampleCount = m_DisplayParams.uiMultisampleQualityLevel; uiSampleCount > 1; uiSampleCount /= 2 )
		{
			UINT uiNumQualityLevels = 0;
			if ( m_pDevice->CheckMultisampleQualityLevels( DXGI_FORMAT_R8G8B8A8_UNORM, uiSampleCount, &uiNumQualityLevels ) == S_OK && uiNumQualityLevels != 0 )
			{
				m_DisplayParams.uiMultisampleQualityLevel = uiSampleCount;
				break;
			}
		}

		if ( uiSampleCount <= 1 )
			m_DisplayParams.uiMultisampleQualityLevel = 1;
	}

	if ( SUCCEEDED( m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&m_pBackBuffer ) ) &&
	     SUCCEEDED( m_pDevice->CreateRenderTargetView( m_pBackBuffer, NULL, &m_pBackBufferRTView ) ) )
	{
		TUtil::Log( "Creating Main RT %d x %d", m_DisplayParams.uiWidth, m_DisplayParams.uiHeight );

		m_pMainSRView = CreateTexture( m_DisplayParams.uiWidth, m_DisplayParams.uiHeight, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, TEXTUREFLAGS_RENDERTARGET, D3D11_USAGE_DEFAULT, 0, m_DisplayParams.uiMultisampleQualityLevel );

		ID3D11Resource* pResource = TNULL;
		m_pMainSRView->GetResource( &pResource );
		pResource->QueryInterface( __uuidof( ID3D11Texture2D ), (void**)&m_pMainTexture );

		if ( pResource )
		{
			pResource->Release();
			pResource = TNULL;
		}

		m_pMainRTView = CreateRenderTargetView( m_pMainSRView );
		m_pDeviceContext->ClearRenderTargetView( m_pMainRTView, m_aClearColour );
		m_pDeviceContext->ClearRenderTargetView( m_pBackBufferRTView, m_aClearColour );

		m_pResolveSRView = CreateTexture( m_DisplayParams.uiWidth, m_DisplayParams.uiHeight, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, TEXTUREFLAGS_RENDERTARGET, D3D11_USAGE_DEFAULT, 0, 1 );
		m_pResolveSRView->GetResource( &pResource );
		pResource->QueryInterface( __uuidof( ID3D11Texture2D ), (void**)&m_pResolveTexture );

		if ( pResource )
		{
			pResource->Release();
			pResource = TNULL;
		}

		// Depth and stencil buffer (the stencil can be read by the shaders)
		D3D11_TEXTURE2D_DESC oDepthDesc = {};
		oDepthDesc.Width                = m_DisplayParams.uiWidth;
		oDepthDesc.Height               = m_DisplayParams.uiHeight;
		oDepthDesc.MipLevels            = 1;
		oDepthDesc.ArraySize            = 1;
		oDepthDesc.Format               = DXGI_FORMAT_R24G8_TYPELESS;
		oDepthDesc.SampleDesc.Count     = m_DisplayParams.uiMultisampleQualityLevel;
		oDepthDesc.SampleDesc.Quality   = 0;
		oDepthDesc.Usage                = D3D11_USAGE_DEFAULT;
		oDepthDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		oDepthDesc.CPUAccessFlags       = 0;
		oDepthDesc.MiscFlags            = 0;

		ID3D11Texture2D* pDepthTexture = TNULL;

		if ( SUCCEEDED( m_pDevice->CreateTexture2D( &oDepthDesc, NULL, &pDepthTexture ) ) )
		{
			const TBOOL bMultisampled = m_DisplayParams.uiMultisampleQualityLevel > 1;

			D3D11_DEPTH_STENCIL_VIEW_DESC oDSVDesc = {};
			oDSVDesc.Format                        = DXGI_FORMAT_D24_UNORM_S8_UINT;
			oDSVDesc.ViewDimension                 = bMultisampled ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
			oDSVDesc.Flags                         = 0;
			oDSVDesc.Texture2D.MipSlice            = 0;
			m_pDevice->CreateDepthStencilView( pDepthTexture, &oDSVDesc, &m_pDepthStencilView );

			D3D11_SHADER_RESOURCE_VIEW_DESC oSRVDesc = {};
			oSRVDesc.Format                          = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
			oSRVDesc.ViewDimension                   = bMultisampled ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
			oSRVDesc.Texture2D.MostDetailedMip       = 0;
			oSRVDesc.Texture2D.MipLevels             = 1;
			m_pDevice->CreateShaderResourceView( pDepthTexture, &oSRVDesc, &m_pDepthStencilSRV );

			pDepthTexture->Release();
		}
	}

#ifdef DEBLOB_COMMUNITY_PATCH
	// Background test runs don't move the user's mouse cursor
	if ( !TMSWindow::IsBackgroundMode() )
#endif
		SetCursorPos( iWindowX + TINT( GetCurrentDisplayParams()->uiWidth ) / 2, TINT( GetCurrentDisplayParams()->uiHeight ) / 2 + iWindowY );

	if ( !m_DisplayParams.bWindowed )
		ShowCursor( FALSE );

	Initialize();

	TRenderContext::VIEWPORTPARAMS oViewport;
	oViewport.fX      = 0.0f;
	oViewport.fY      = 0.0f;
	oViewport.fWidth  = TFLOAT( TINT( m_SwapChainDesc.BufferDesc.Width ) );
	oViewport.fHeight = TFLOAT( TINT( m_SwapChainDesc.BufferDesc.Height ) );
	oViewport.fMinZ   = 0.0f;
	oViewport.fMaxZ   = 1.0f;
	GetCurrentContext()->SetViewportParameters( oViewport );

	new TTextureManager;

	m_pVarianceShadowMaps = new TVarianceShadowMaps;
	m_pPostProcess        = new TPostProcess;
	m_pToneMap            = new TToneMap;
	m_pGlow               = new TGlow;
	m_pDOF                = new TDOF;
	m_pFXAA               = new TFXAA;

	CompileVSPS();

	m_bDisplayCreated = TTRUE;
	return TTRUE;
}

// $deBlob: FUNCTION 006a5710
TBOOL TRenderDX11::RecreateDisplay( const DISPLAYPARAMS& a_rParams )
{
	TASSERT( TTRUE == IsCreated() );
	TASSERT( TTRUE == IsDisplayCreated() );

	if ( IsCreated() && IsDisplayCreated() )
	{
		OnDeviceLost();
		DestroyDisplay();

		if ( CreateDisplay( a_rParams ) )
		{
			OnDeviceReset();
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006a57e0
TBOOL TRenderDX11::DestroyDisplay()
{
	// NOTE: retail inlines TRender::DestroyDisplay here (FlushDyingResources, IsDisplayCreated)
	return TRender::DestroyDisplay();
}

// $deBlob: FUNCTION 006a5830
void TRenderDX11::Update( TFLOAT a_fDeltaTime )
{
	TASSERT( TTRUE == IsCreated() );

	m_Window.Update();
	TRender::Update( a_fDeltaTime );
}

// $deBlob: FUNCTION 006a58a0
void TRenderDX11::BeginScene()
{
	TASSERT( TTRUE == IsDisplayCreated() );
	TASSERT( TFALSE == IsInScene() );

	TRender::BeginScene();

	m_pDeviceContext->OMSetRenderTargets( 1, &m_pMainRTView, m_pDepthStencilView );

	const FLOAT aBlack[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pDeviceContext->ClearRenderTargetView( m_pMainRTView, aBlack );

	D3D11_VIEWPORT oViewport;
	oViewport.TopLeftX = 0.0f;
	oViewport.TopLeftY = 0.0f;
	oViewport.Width    = TFLOAT( m_SwapChainDesc.BufferDesc.Width );
	oViewport.Height   = TFLOAT( m_SwapChainDesc.BufferDesc.Height );
	oViewport.MinDepth = 0.0f;
	oViewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports( 1, &oViewport );

	m_bInScene = TTRUE;
}

// $deBlob: FUNCTION 006a5a00
void TRenderDX11::EndScene()
{
	TASSERT( TTRUE == IsDisplayCreated() );
	TASSERT( IsInScene() == TTRUE );

	m_uiNumDrawnFrames += 1;

	if ( !m_bIsWidescreen )
	{
		// The main render target has the size of the back buffer
		if ( TINT( m_DisplayParams.uiMultisampleQualityLevel ) < 2 )
			m_pDeviceContext->CopyResource( m_pBackBuffer, m_pMainTexture );
		else
			m_pDeviceContext->ResolveSubresource( m_pBackBuffer, 0, m_pMainTexture, 0, DXGI_FORMAT_R8G8B8A8_UNORM );
	}
	else
	{
		// Letterboxed into the back buffer
		m_pDeviceContext->OMSetRenderTargets( 1, &m_pBackBufferRTView, TNULL );

		D3D11_VIEWPORT oViewport;
		oViewport.TopLeftX = 0.0f;
		oViewport.TopLeftY = 0.0f;
		oViewport.Width    = TFLOAT( TINT( m_uiDisplayWidth ) );
		oViewport.Height   = TFLOAT( TINT( m_uiDisplayHeight ) );
		oViewport.MinDepth = 0.0f;
		oViewport.MaxDepth = 1.0f;
		m_pDeviceContext->RSSetViewports( 1, &oViewport );
		m_pDeviceContext->ClearRenderTargetView( m_pBackBufferRTView, m_aClearColour );

		TFLOAT fOffsetX = TFLOAT( m_uiDisplayWidth - m_DisplayParams.uiWidth );
		TFLOAT fOffsetY = TFLOAT( m_uiDisplayHeight - m_DisplayParams.uiHeight );

		if ( TINT( m_DisplayParams.uiMultisampleQualityLevel ) < 2 )
		{
			if ( s_pMainTextureSRV == TNULL )
				m_pDevice->CreateShaderResourceView( m_pMainTexture, NULL, &s_pMainTextureSRV );

			const TFLOAT fInAR  = TFLOAT( TINT( m_DisplayParams.uiWidth ) ) / TFLOAT( TINT( m_DisplayParams.uiHeight ) );
			const TFLOAT fOutAR = TFLOAT( TINT( m_uiDisplayWidth ) ) / TFLOAT( TINT( m_uiDisplayHeight ) );
			TASSERT( fabsf( fInAR - 16.0f / 9.0f ) < 0.01 );

			TFLOAT fPosX;
			TFLOAT fPosY;
			TFLOAT fWidth;
			TFLOAT fHeight;

			if ( fOutAR < fInAR )
			{
				fWidth  = TFLOAT( TINT( m_uiDisplayWidth ) );
				fHeight = fWidth / fInAR;
				fPosX   = 0.0f;
				fPosY   = ( TFLOAT( TINT( m_uiDisplayHeight ) ) - fHeight ) * 0.5f;
			}
			else
			{
				fPosY   = 0.0f;
				fHeight = TFLOAT( TINT( m_uiDisplayHeight ) );
				fWidth  = fHeight * fInAR;
				fPosX   = ( TFLOAT( TINT( m_uiDisplayWidth ) ) - fWidth ) * 0.5f;
			}

			// No blending
			m_CurrentBlendState.uiColour &= ~BLENDCOLOUR_BLENDENABLE;
			RenderOverlay( fPosX, fPosY, fWidth, fHeight, s_pMainTextureSRV, TNULL, TNULL );
		}
		else
		{
			m_pDeviceContext->ResolveSubresource( m_pResolveTexture, 0, m_pMainTexture, 0, DXGI_FORMAT_R8G8B8A8_UNORM );
			m_pDeviceContext->CopySubresourceRegion( m_pBackBuffer, 0, TINT( fOffsetX ) / 2, TINT( fOffsetY ) / 2, 0, m_pResolveTexture, 0, NULL );
		}

		ID3D11ShaderResourceView* pNullSRV = TNULL;
		m_pDeviceContext->PSSetShaderResources( 0, 1, &pNullSRV );
	}

#if defined( DEBLOB_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
	// Development: "-devshot" saves the finished frame
	TDevScreenshot::Update( m_pDevice, m_pDeviceContext, m_pBackBuffer );
#endif

	const TBOOL bPresentTest = s_bPresentTest;
	s_bPresentTest           = TFALSE;

	HRESULT hRes = m_pSwapChain->Present( 1, bPresentTest ? DXGI_PRESENT_TEST : 0 );

	if ( hRes != S_OK )
	{
		TUtil::Log( "Present Failed !!!! 0x%x", hRes );

		if ( hRes == DXGI_ERROR_DEVICE_REMOVED )
		{
			OnDeviceLost();
		}
		else if ( hRes == DXGI_ERROR_DEVICE_RESET )
		{
			OnDeviceReset();
		}
		else if ( hRes == DXGI_STATUS_OCCLUDED )
		{
			s_bPresentTest = TTRUE;
			TUtil::Log( "Pausing Occluded!\n" );

#ifdef DEBLOB_COMMUNITY_PATCH
			// Background test runs keep running (their window is outside of the desktop)
			if ( !TMSWindow::IsBackgroundMode() && !TMSWindow::IsNoPauseMode() )
#endif
				TSystemManager::GetSingleton()->Pause( TTRUE );
		}
	}

	m_bInScene = TFALSE;
	TRender::EndScene();
}

// $deBlob: FUNCTION 006a5810
TRenderAdapter::Mode::Device* TRenderDX11::GetCurrentDevice()
{
	return TNULL;
}

// $deBlob: FUNCTION 006a5820
TRender::DISPLAYPARAMS* TRenderDX11::GetCurrentDisplayParams()
{
	return &m_DisplayParams;
}

// $deBlob: FUNCTION 006a5df0
void TRenderDX11::ClearDepth()
{
	m_pDeviceContext->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
}

// $deBlob: FUNCTION 006a5e20
TBOOL TRenderDX11::Create()
{
	return Create( "de Blob" );
}

// $deBlob: FUNCTION 006a5e30
TBOOL TRenderDX11::Create( LPCSTR a_szTitle )
{
	TASSERT( TFALSE == IsCreated() );

	if ( !TRender::Create() )
		return TFALSE;

	TUtil::Log( "Creating TRenderDX11" );
	TUtil::LogDown();

	D3D_FEATURE_LEVEL eFeatureLevel;

	for ( TUINT i = 0;; )
	{
		eFeatureLevel = D3D_FEATURE_LEVEL_9_3;

		if ( SUCCEEDED( D3D11CreateDevice( NULL, s_aDriverTypes[ i ], NULL, 0, s_aFeatureLevels, 4, D3D11_SDK_VERSION, &m_pDevice, &eFeatureLevel, &m_pDeviceContext ) ) )
			break;

		// Without 11_1 (the Direct3D 11.0 runtime doesn't know it)
		if ( SUCCEEDED( D3D11CreateDevice( NULL, s_aDriverTypes[ i ], NULL, 0, s_aFeatureLevels + 1, 3, D3D11_SDK_VERSION, &m_pDevice, &eFeatureLevel, &m_pDeviceContext ) ) || ++i > 2 )
			break;
	}

	TUtil::Log( "FeatureLevel : %s", GetFeatureLevel( eFeatureLevel ) );

	BuildAdapterDatabase();

	CreateAccelerators();

	if ( m_Window.Create( this, a_szTitle ) )
	{
		if ( m_bCreateSystemResources )
			CreateSystemResources();

		TUtil::LogUp();
		m_eAspectRatio = ASPECT_RATIO_16_9;
		return TTRUE;
	}

	TUtil::Log( "Failed to create window" );
	return TFALSE;
}

// $deBlob: FUNCTION 006a5fc0
TBOOL TRenderDX11::Destroy()
{
	if ( !IsCreated() )
		return TTRUE;

	DestroyRenderStates();

	if ( m_pMainTexture )
	{
		m_pMainTexture->Release();
		m_pMainTexture = TNULL;
	}

	if ( m_pMainRTView )
	{
		m_pMainRTView->Release();
		m_pMainRTView = TNULL;
	}

	if ( m_pMainSRView )
	{
		m_pMainSRView->Release();
		m_pMainSRView = TNULL;
	}

	if ( m_pResolveTexture )
	{
		m_pResolveTexture->Release();
		m_pResolveTexture = TNULL;
	}

	if ( m_pResolveSRView )
	{
		m_pResolveSRView->Release();
		m_pResolveSRView = TNULL;
	}

	if ( m_pDepthStencilSRV )
	{
		m_pDepthStencilSRV->Release();
		m_pDepthStencilSRV = TNULL;
	}

	if ( m_pBackBuffer )
	{
		m_pBackBuffer->Release();
		m_pBackBuffer = TNULL;
	}

	if ( m_pBackBufferRTView )
	{
		m_pBackBufferRTView->Release();
		m_pBackBufferRTView = TNULL;
	}

	if ( m_pDepthStencilView )
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = TNULL;
	}

	if ( m_pSwapChain && m_DisplayParams.bFullscreen )
		m_pSwapChain->SetFullscreenState( FALSE, NULL );

	if ( m_pSwapChain )
	{
		m_pSwapChain->Release();
		m_pSwapChain = TNULL;
	}

	FlushDyingResources();
	DestroySystemResources();
	DestroyDisplay();
	m_Window.Destroy();
	DestroyAccelerators();

	if ( m_pDeviceContext )
	{
		m_pDeviceContext->Release();
		m_pDeviceContext = TNULL;
	}

	DestroyOverlayShaders();

	if ( m_pVarianceShadowMaps )
		delete m_pVarianceShadowMaps;

	if ( m_pPostProcess )
		delete m_pPostProcess;

	if ( m_pToneMap )
		delete m_pToneMap;

	if ( m_pGlow )
		delete m_pGlow;

	if ( m_pFXAA )
		delete m_pFXAA;

	if ( m_pDOF )
		delete m_pDOF;

	return TRender::Destroy();
}

// $deBlob: FUNCTION 006a6b60
ID3D11ShaderResourceView* TRenderDX11::CreateTexture( TUINT32 a_uiWidth, TUINT32 a_uiHeight, DXGI_FORMAT a_eFormat, const void* a_pSrcData, TUINT8 a_uiFlags, D3D11_USAGE a_eUsage, TUINT32 a_uiCPUAccessFlags, TUINT32 a_uiSampleCount )
{
	ID3D11ShaderResourceView* pShaderResourceView = TNULL;

	const TBOOL bMipmapsFromData = ( a_uiFlags & TEXTUREFLAGS_MIPMAPSFROMDATA ) != 0;
	const TBOOL bGenerateMipmaps = !bMipmapsFromData && ( a_uiFlags & TEXTUREFLAGS_GENERATEMIPMAPS ) != 0;
	TUINT32     uiNumMipmaps;

	if ( bMipmapsFromData || bGenerateMipmaps )
	{
		// A full chain
		const TUINT32 uiMaxSize = ( a_uiWidth <= a_uiHeight ) ? a_uiHeight : a_uiWidth;
		uiNumMipmaps            = TUINT32( floor( log2( (double)uiMaxSize ) ) + 1.0 );
	}
	else
	{
		uiNumMipmaps = 1;
	}

	D3D11_TEXTURE2D_DESC oDesc = {};
	oDesc.Width                = a_uiWidth;
	oDesc.Height               = a_uiHeight;
	oDesc.MipLevels            = bGenerateMipmaps ? 0 : uiNumMipmaps;
	oDesc.ArraySize            = 1;
	oDesc.Format               = a_eFormat;
	oDesc.SampleDesc.Count     = a_uiSampleCount;
	oDesc.SampleDesc.Quality   = 0;
	oDesc.Usage                = a_eUsage;
	oDesc.BindFlags            = ( ( a_uiFlags & TEXTUREFLAGS_RENDERTARGET ) || bGenerateMipmaps ) ? ( D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET ) : D3D11_BIND_SHADER_RESOURCE;
	oDesc.CPUAccessFlags       = a_uiCPUAccessFlags;
	oDesc.MiscFlags            = bGenerateMipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	ID3D11Texture2D* pTexture = TNULL;

	if ( a_pSrcData == TNULL )
	{
		m_pDevice->CreateTexture2D( &oDesc, NULL, &pTexture );
	}
	else if ( bGenerateMipmaps )
	{
		// Mip 0 from the data, the others are generated
		m_pDevice->CreateTexture2D( &oDesc, NULL, &pTexture );
		if ( pTexture == TNULL ) return pShaderResourceView;

		m_pDeviceContext->UpdateSubresource( pTexture, 0, NULL, a_pSrcData, GetTextureRowPitch( a_eFormat, a_uiWidth ), GetTextureDepthPitch( a_eFormat, a_uiWidth, a_uiHeight ) );
	}
	else
	{
		TASSERT( uiNumMipmaps <= MAXIMUM_NUMBER_OF_MIPMAPS );

		D3D11_SUBRESOURCE_DATA aSubresources[ MAXIMUM_NUMBER_OF_MIPMAPS ];

		if ( bMipmapsFromData )
		{
			// The mipmaps follow each other in the data
			const TUINT8* pSrcData = TSTATICCAST( const TUINT8, a_pSrcData );
			TUINT32       uiWidth  = a_uiWidth;
			TUINT32       uiHeight = a_uiHeight;

			for ( TUINT32 i = 0; i < uiNumMipmaps; i++ )
			{
				aSubresources[ i ].pSysMem          = pSrcData;
				aSubresources[ i ].SysMemPitch      = GetTextureRowPitch( a_eFormat, uiWidth );
				aSubresources[ i ].SysMemSlicePitch = GetTextureDepthPitch( a_eFormat, uiWidth, uiHeight );
				pSrcData += aSubresources[ i ].SysMemSlicePitch;

				uiWidth  = TMath::Max( uiWidth >> 1, 1U );
				uiHeight = TMath::Max( uiHeight >> 1, 1U );
			}
		}
		else
		{
			aSubresources[ 0 ].pSysMem          = a_pSrcData;
			aSubresources[ 0 ].SysMemPitch      = GetTextureRowPitch( a_eFormat, a_uiWidth );
			aSubresources[ 0 ].SysMemSlicePitch = GetTextureDepthPitch( a_eFormat, a_uiWidth, a_uiHeight );
		}

		m_pDevice->CreateTexture2D( &oDesc, aSubresources, &pTexture );
	}

	if ( pTexture != TNULL )
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC oSRVDesc = {};
		oSRVDesc.Format                          = oDesc.Format;
		oSRVDesc.ViewDimension                   = ( a_uiSampleCount > 1 ) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		oSRVDesc.Texture2D.MostDetailedMip       = 0;
		oSRVDesc.Texture2D.MipLevels             = uiNumMipmaps;
		m_pDevice->CreateShaderResourceView( pTexture, &oSRVDesc, &pShaderResourceView );

		if ( bGenerateMipmaps )
			m_pDeviceContext->GenerateMips( pShaderResourceView );

		pTexture->Release();
	}

	return pShaderResourceView;
}

// $deBlob: FUNCTION 006a7190
void TRenderDX11::CopyDataToTexture( ID3D11ShaderResourceView* a_pShaderResourceView, TUINT32 a_uiDataSize, const void* a_pSrcData, TUINT32 a_uiRowSize )
{
	ID3D11Resource* pResource = TNULL;
	a_pShaderResourceView->GetResource( &pResource );

	D3D11_MAPPED_SUBRESOURCE oMapped;
	m_pDeviceContext->Map( pResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );

	// One row at a time (the rows of the texture are oMapped.RowPitch apart)
	TUINT8*       pDst     = TSTATICCAST( TUINT8, oMapped.pData );
	const TUINT8* pSrc     = TSTATICCAST( const TUINT8, a_pSrcData );
	TUINT32       uiToCopy = a_uiDataSize;

	while ( uiToCopy != 0 )
	{
		TUtil::MemCopy( pDst, pSrc, ( uiToCopy >= a_uiRowSize ) ? a_uiRowSize : uiToCopy );
		uiToCopy -= a_uiRowSize;
		pDst += oMapped.RowPitch;
		pSrc += a_uiRowSize;
	}

	m_pDeviceContext->Unmap( pResource, 0 );
	pResource->Release();
}

// $deBlob: FUNCTION 006a9560
TUINT32 TRenderDX11::GetTextureRowPitch( DXGI_FORMAT a_eFormat, TUINT32 a_uiWidth )
{
	switch ( a_eFormat )
	{
		case DXGI_FORMAT_UNKNOWN:
			return 0;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return a_uiWidth * 16;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return a_uiWidth * 8;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return a_uiWidth * 4;
		case DXGI_FORMAT_A8_UNORM:
			return a_uiWidth;
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC4_UNORM:
			return ( ( a_uiWidth + 3 ) >> 2 ) * 8;
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC5_UNORM:
			return ( ( a_uiWidth + 3 ) >> 2 ) * 16;
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return a_uiWidth * 3;
		default:
			TASSERT( TFALSE );
			return 0;
	}
}

// $deBlob: FUNCTION 006a9680
TUINT32 TRenderDX11::GetTextureDepthPitch( DXGI_FORMAT a_eFormat, TUINT32 a_uiWidth, TUINT32 a_uiHeight )
{
	switch ( a_eFormat )
	{
		case DXGI_FORMAT_UNKNOWN:
			return 0;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return a_uiWidth * a_uiHeight * 16;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return a_uiWidth * a_uiHeight * 8;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return a_uiWidth * a_uiHeight * 4;
		case DXGI_FORMAT_A8_UNORM:
			return a_uiWidth * a_uiHeight;
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC4_UNORM:
			return ( ( a_uiWidth + 3 ) >> 2 ) * ( ( a_uiHeight + 3 ) >> 2 ) * 8;
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC5_UNORM:
			return ( ( a_uiWidth + 3 ) >> 2 ) * ( ( a_uiHeight + 3 ) >> 2 ) * 16;
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return a_uiWidth * a_uiHeight * 3;
		default:
			TASSERT( TFALSE );
			return 0;
	}
}

// $deBlob: FUNCTION 006a80e0
void TRenderDX11::DestroyAdapters()
{
	GetAdapterList()->DeleteAll();
}

// $deBlob: FUNCTION 006a80f0
void TRenderDX11::CreateAccelerators()
{
	DestroyAccelerators();

	// Escape and Alt+Enter
	ACCEL aAccels[ 2 ];
	aAccels[ 0 ].fVirt = FVIRTKEY;
	aAccels[ 0 ].key   = VK_ESCAPE;
	aAccels[ 0 ].cmd   = 0;
	aAccels[ 1 ].fVirt = FALT;
	aAccels[ 1 ].key   = VK_RETURN;
	aAccels[ 1 ].cmd   = 1;
	m_hAccel           = CreateAcceleratorTableA( aAccels, 2 );
}

// $deBlob: FUNCTION 006a8150
void TRenderDX11::DestroyAccelerators()
{
	if ( m_hAccel != NULL )
	{
		DestroyAcceleratorTable( m_hAccel );
		m_hAccel = NULL;
	}
}

// $deBlob: FUNCTION 006a6280
ID3DBlob* TRenderDX11::CompileShader( const TCHAR* a_szSrcData, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const D3D_SHADER_MACRO* a_pDefines )
{
	// The level 9 targets are compiled with D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY (sampler_state, COLOR)
	UINT uiFlags = 0;

	if ( strcmp( a_szTarget, "ps_4_0_level_9_3" ) == 0 || strcmp( a_szTarget, "vs_4_0_level_9_3" ) == 0 )
		uiFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;

	ID3DBlob* pShaderBlob = TNULL;
	ID3DBlob* pErrorBlob  = TNULL;

	if ( FAILED( D3DCompile( a_szSrcData, strlen( a_szSrcData ), NULL, a_pDefines, NULL, a_szEntryPoint, a_szTarget, uiFlags, 0, &pShaderBlob, &pErrorBlob ) ) )
	{
		OutputDebugStringA( "Shader compilation failed " );
		OutputDebugStringA( pErrorBlob ? TSTATICCAST( const TCHAR, pErrorBlob->GetBufferPointer() ) : "" );
		TUtil::Log( "Shader Compilation failed:" );
		TUtil::Log( pErrorBlob ? TSTATICCAST( TCHAR, pErrorBlob->GetBufferPointer() ) : "" );

		if ( pErrorBlob )
		{
			pErrorBlob->Release();
			pErrorBlob = TNULL;
		}

		TASSERT( 0 );
	}

	return pShaderBlob;
}

// NOTE: retail goes on (and crashes) when the file can't be opened, like here
// $deBlob: FUNCTION 006a63e0
HRESULT TRenderDX11::CreateVertexShaderFromFile( const TCHAR* a_szFileName, ID3D11VertexShader** a_ppVertexShader )
{
	TFile* pFile = TFile::Create( TString8( a_szFileName ), TFILEMODE_READ );
	if ( pFile == TNULL )
		TASSERT( !"Couldnt load file" );

	TSIZE  uiSize   = pFile->GetSize();
	TBYTE* pBuffer  = new TBYTE[ uiSize ];
	pFile->Read( pBuffer, uiSize );

	HRESULT hResult = m_pDevice->CreateVertexShader( pBuffer, uiSize, NULL, a_ppVertexShader );

	if ( FAILED( hResult ) )
		TASSERT( !"Couldnt Create Vertex Shader" );

	delete[] pBuffer;
	TFile::Destroy( pFile );
	return hResult;
}

// NOTE: retail goes on (and crashes) when the file can't be opened, like here
// $deBlob: FUNCTION 006a6530
HRESULT TRenderDX11::CreatePixelShaderFromFile( const TCHAR* a_szFileName, ID3D11PixelShader** a_ppPixelShader )
{
	TFile* pFile = TFile::Create( TString8( a_szFileName ), TFILEMODE_READ );
	if ( pFile == TNULL )
		TASSERT( !"Couldnt load file" );

	TSIZE  uiSize   = pFile->GetSize();
	TBYTE* pBuffer  = new TBYTE[ uiSize ];
	pFile->Read( pBuffer, uiSize );

	HRESULT hResult = m_pDevice->CreatePixelShader( pBuffer, uiSize, NULL, a_ppPixelShader );

	if ( FAILED( hResult ) )
		TASSERT( !"Couldnt Create Pixel Shader" );

	delete[] pBuffer;
	TFile::Destroy( pFile );
	return hResult;
}

// $deBlob: FUNCTION 006a64d0
HRESULT TRenderDX11::CreateVertexShader( const void* a_pShaderBytecode, SIZE_T a_uiBytecodeLength, ID3D11VertexShader** a_ppVertexShader )
{
	HRESULT hResult = m_pDevice->CreateVertexShader( a_pShaderBytecode, a_uiBytecodeLength, NULL, a_ppVertexShader );

	if ( FAILED( hResult ) )
		TASSERT( !"Couldnt Create Vertex Shader" );

	return hResult;
}

// $deBlob: FUNCTION 006a6620
HRESULT TRenderDX11::CreatePixelShader( const void* a_pShaderBytecode, SIZE_T a_uiBytecodeLength, ID3D11PixelShader** a_ppPixelShader )
{
	HRESULT hResult = m_pDevice->CreatePixelShader( a_pShaderBytecode, a_uiBytecodeLength, NULL, a_ppPixelShader );

	if ( FAILED( hResult ) )
		TASSERT( !"Couldnt Create Pixel Shader" );

	return hResult;
}

// A shader kind DX11 doesn't support
// $deBlob: FUNCTION 006a6680
void TRenderDX11::CreateUnkShader( void* a_pShaderData, void** a_ppOutShader )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006a66c0
void TRenderDX11::CreateFXShader( void* a_pShaderData, void** a_ppOutShader )
{
	// NOTE: TFXManager_DX11's effects aren't supported on PC
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006a8170
void TRenderDX11::CompileVSPS()
{
	ID3DBlob* pVSBlob = CompileShader( s_szOverlayVS, "main", "vs_4_0_level_9_3", TNULL );
	CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pOverlayVS );

	ID3DBlob* pPSBlob = CompileShader( s_szOverlayPS, "main", "ps_4_0_level_9_3", TNULL );
	CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pOverlayPS );
	pPSBlob->Release();

	pPSBlob = CompileShader( s_szOverlayOpaquePS, "main", "ps_4_0_level_9_3", TNULL );
	CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pOverlayOpaquePS );
	pPSBlob->Release();

	D3D11_INPUT_ELEMENT_DESC aInputDesc[ 2 ];
	aInputDesc[ 0 ].SemanticName         = "POSITION";
	aInputDesc[ 0 ].SemanticIndex        = 0;
	aInputDesc[ 0 ].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
	aInputDesc[ 0 ].InputSlot            = 0;
	aInputDesc[ 0 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDesc[ 0 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDesc[ 0 ].InstanceDataStepRate = 0;
	aInputDesc[ 1 ].SemanticName         = "TEXCOORD";
	aInputDesc[ 1 ].SemanticIndex        = 0;
	aInputDesc[ 1 ].Format               = DXGI_FORMAT_R32G32_FLOAT;
	aInputDesc[ 1 ].InputSlot            = 0;
	aInputDesc[ 1 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDesc[ 1 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDesc[ 1 ].InstanceDataStepRate = 0;
	m_pDevice->CreateInputLayout( aInputDesc, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pOverlayLayout );
	pVSBlob->Release();

	// Quad of (0, 0) - (1, -1), scaled and moved by the vertex shader (position, uv)
	FLOAT aVertices[ 20 ] = {
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 1.0f
	};

	D3D11_SUBRESOURCE_DATA oVertexData;
	oVertexData.pSysMem          = aVertices;
	oVertexData.SysMemPitch      = 0;
	oVertexData.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC oBufferDesc;
	oBufferDesc.ByteWidth           = sizeof( aVertices );
	oBufferDesc.Usage               = D3D11_USAGE_IMMUTABLE;
	oBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	oBufferDesc.CPUAccessFlags      = 0;
	oBufferDesc.MiscFlags           = 0;
	oBufferDesc.StructureByteStride = 0;

	m_pOverlayQuad = TNULL;
	m_pDevice->CreateBuffer( &oBufferDesc, &oVertexData, &m_pOverlayQuad );
}

// $deBlob: FUNCTION 006a6700
void TRenderDX11::RenderOverlay( TFLOAT a_fPosX, TFLOAT a_fPosY, TFLOAT a_fWidth, TFLOAT a_fHeight, ID3D11ShaderResourceView* a_pShaderResourceView, ID3D11PixelShader* a_pPixelShader, const TVector4* a_pUVST )
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	pRender->m_pDeviceContext->VSSetShader( pRender->m_pOverlayVS, NULL, 0 );
	pRender->m_pDeviceContext->PSSetShader( a_pPixelShader ? a_pPixelShader : pRender->m_pOverlayPS, NULL, 0 );
	pRender->m_pDeviceContext->IASetInputLayout( pRender->m_pOverlayLayout );

	// No culling, no depth test or write
	pRender->m_CurrentRasterizer.uiFlags = ( pRender->m_CurrentRasterizer.uiFlags & ~RASTERIZERFLAGS_CULLMASK ) | ( D3D11_CULL_NONE << RASTERIZERFLAGS_CULLSHIFT );
	pRender->m_CurrentDepth.Byte( 0 )    = ( pRender->m_CurrentDepth.Byte( 0 ) & 0xC0 ) | ( D3D11_COMPARISON_LESS_EQUAL << DEPTHFLAGS_FUNCSHIFT );

	pRender->m_pDeviceContext->PSSetShaderResources( 0, 1, &a_pShaderResourceView );
	pRender->m_pDeviceContext->PSSetSamplers( 0, 1, &pRender->m_aSamplerStates[ SAMPLER_LINEAR_CLAMP ] );

	UINT           uiNumViewports = 1;
	D3D11_VIEWPORT oViewport;
	pRender->m_pDeviceContext->RSGetViewports( &uiNumViewports, &oViewport );

	const TFLOAT fViewportWidth  = TFLOAT( TINT( oViewport.Width ) );
	const TFLOAT fViewportHeight = TFLOAT( TINT( oViewport.Height ) );

	// ScaleTranslate (c0): to clip space
	TVector4 vScaleTranslate;
	vScaleTranslate.x                                                = ( a_fWidth / fViewportWidth ) * 2.0f;
	vScaleTranslate.y                                                = ( a_fHeight / fViewportHeight ) * 2.0f;
	vScaleTranslate.z                                                = ( a_fPosX / fViewportWidth ) * 2.0f - 1.0f;
	vScaleTranslate.w                                                = ( 2.0f - ( a_fPosY / fViewportHeight ) * 2.0f ) - 1.0f;
	*TREINTERPRETCAST( TVector4*, pRender->m_pVertexConstantBuffer ) = vScaleTranslate;
	pRender->m_bVertexConstantBufferSet                              = TTRUE;

	// uvST (c1)
	if ( a_pUVST != TNULL )
	{
		pRender->SetVec4InVSBuffer( 1, a_pUVST, 1 );
	}
	else
	{
		TVector4 vUVST( 1.0f, 1.0f, 0.0f, 0.0f );
		pRender->SetVec4InVSBuffer( 1, &vUVST, 1 );
	}

	ID3D11Buffer* pVertexBuffer = pRender->m_pOverlayQuad;
	UINT          uiStride      = 20;
	UINT          uiOffset      = 0;

	pRender->UpdateRenderStates();
	pRender->m_pDeviceContext->IASetVertexBuffers( 0, 1, &pVertexBuffer, &uiStride, &uiOffset );
	pRender->FlushConstantBuffers();
	pRender->m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	pRender->m_pDeviceContext->Draw( 4, 0 );
}

// $deBlob: FUNCTION 006a7320
void TRenderDX11::DrawNonIndexed( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, ID3D11Buffer* a_pVertexBuffer, UINT a_uiVertexCount, UINT a_uiStride, UINT a_uiStartVertex, UINT a_uiOffset )
{
	UpdateRenderStates();
	m_pDeviceContext->IASetVertexBuffers( 0, 1, &a_pVertexBuffer, &a_uiStride, &a_uiOffset );
	FlushConstantBuffers();
	m_pDeviceContext->IASetPrimitiveTopology( a_eTopology );
	m_pDeviceContext->Draw( a_uiVertexCount, a_uiStartVertex );
}

// $deBlob: FUNCTION 006a7380
void TRenderDX11::DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, const void* a_pVertices, UINT a_uiVertexCount, UINT a_uiStride, UINT a_uiStartVertex )
{
	const TUINT32 uiVertexBufferSize = a_uiStride * a_uiVertexCount;

	// NOTE: no assert here (the indexed versions have one)
	if ( m_uiImmediateVertexCurrentOffset + uiVertexBufferSize > IMMEDIATE_VERTEX_BUFFER_SIZE )
		m_uiImmediateVertexCurrentOffset = 0;

	D3D11_MAPPED_SUBRESOURCE oMapped;
	m_pDeviceContext->Map( m_pImmediateVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
	memmove( TSTATICCAST( TUINT8, oMapped.pData ) + m_uiImmediateVertexCurrentOffset, a_pVertices, uiVertexBufferSize );
	m_pDeviceContext->Unmap( m_pImmediateVertexBuffer, 0 );

	UINT          uiOffset      = m_uiImmediateVertexCurrentOffset;
	ID3D11Buffer* pVertexBuffer = m_pImmediateVertexBuffer;

	UpdateRenderStates();
	m_pDeviceContext->IASetVertexBuffers( 0, 1, &pVertexBuffer, &a_uiStride, &uiOffset );
	FlushConstantBuffers();
	m_pDeviceContext->IASetPrimitiveTopology( a_eTopology );
	m_pDeviceContext->Draw( a_uiVertexCount, a_uiStartVertex );

	m_uiImmediateVertexCurrentOffset += uiVertexBufferSize;
}

// $deBlob: FUNCTION 006a7470
void TRenderDX11::DrawIndexed( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, ID3D11Buffer* a_pIndexBuffer, UINT a_uiIndexBufferOffset, DXGI_FORMAT a_eIndexFormat, ID3D11Buffer* a_pVertexBuffer, UINT a_uiStride, UINT a_uiOffset )
{
	UpdateRenderStates();
	m_pDeviceContext->IASetVertexBuffers( 0, 1, &a_pVertexBuffer, &a_uiStride, &a_uiOffset );
	m_pDeviceContext->IASetIndexBuffer( a_pIndexBuffer, a_eIndexFormat, a_uiIndexBufferOffset );
	FlushConstantBuffers();
	m_pDeviceContext->IASetPrimitiveTopology( a_eTopology );
	m_pDeviceContext->DrawIndexed( a_uiIndexCount, 0, 0 );
}

// Copies the indices to the immediate index buffer and returns their size
static TUINT32 CopyToImmediateIndexBuffer( ID3D11DeviceContext* a_pDeviceContext, ID3D11Buffer* a_pIndexBuffer, TUINT32& a_rCurrentOffset, const void* a_pIndices, UINT a_uiIndexCount, DXGI_FORMAT a_eIndexFormat )
{
	const TUINT32 uiIndexSize       = ( a_eIndexFormat == DXGI_FORMAT_R32_UINT || a_eIndexFormat == DXGI_FORMAT_R32_SINT ) ? 4 : 2;
	const TUINT32 uiIndexBufferSize = a_uiIndexCount * uiIndexSize;

	if ( uiIndexBufferSize + a_rCurrentOffset > TRenderDX11::IMMEDIATE_INDEX_BUFFER_SIZE )
		a_rCurrentOffset = 0;

	TASSERT( ( a_rCurrentOffset + uiIndexBufferSize ) <= TRenderDX11::IMMEDIATE_INDEX_BUFFER_SIZE );

	D3D11_MAPPED_SUBRESOURCE oMapped;
	a_pDeviceContext->Map( a_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
	memmove( TSTATICCAST( TUINT8, oMapped.pData ) + a_rCurrentOffset, a_pIndices, uiIndexBufferSize );
	a_pDeviceContext->Unmap( a_pIndexBuffer, 0 );

	return uiIndexBufferSize;
}

// Copies the vertices to the immediate vertex buffer and returns their size
static TUINT32 CopyToImmediateVertexBuffer( ID3D11DeviceContext* a_pDeviceContext, ID3D11Buffer* a_pVertexBuffer, TUINT32& a_rCurrentOffset, const void* a_pVertices, UINT a_uiStride, UINT a_uiVertexCount )
{
	const TUINT32 uiVertexBufferSize = a_uiVertexCount * a_uiStride;

	if ( uiVertexBufferSize + a_rCurrentOffset > TRenderDX11::IMMEDIATE_VERTEX_BUFFER_SIZE )
		a_rCurrentOffset = 0;

	TASSERT( ( a_rCurrentOffset + uiVertexBufferSize ) <= TRenderDX11::IMMEDIATE_VERTEX_BUFFER_SIZE );

	D3D11_MAPPED_SUBRESOURCE oMapped;
	a_pDeviceContext->Map( a_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
	memmove( TSTATICCAST( TUINT8, oMapped.pData ) + a_rCurrentOffset, a_pVertices, uiVertexBufferSize );
	a_pDeviceContext->Unmap( a_pVertexBuffer, 0 );

	return uiVertexBufferSize;
}

// $deBlob: FUNCTION 006a74e0
void TRenderDX11::DrawImmediately( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, const void* a_pIndices, DXGI_FORMAT a_eIndexFormat, const void* a_pVertices, UINT a_uiStride, UINT a_uiVertexCount )
{
	const TUINT32 uiIndexBufferSize  = CopyToImmediateIndexBuffer( m_pDeviceContext, m_pImmediateIndexBuffer, m_uiImmediateIndexCurrentOffset, a_pIndices, a_uiIndexCount, a_eIndexFormat );
	const TUINT32 uiVertexBufferSize = CopyToImmediateVertexBuffer( m_pDeviceContext, m_pImmediateVertexBuffer, m_uiImmediateVertexCurrentOffset, a_pVertices, a_uiStride, a_uiVertexCount );

	ID3D11Buffer* pVertexBuffer = m_pImmediateVertexBuffer;
	UINT          uiOffset      = m_uiImmediateVertexCurrentOffset;

	UpdateRenderStates();
	m_pDeviceContext->IASetVertexBuffers( 0, 1, &pVertexBuffer, &a_uiStride, &uiOffset );
	m_pDeviceContext->IASetIndexBuffer( m_pImmediateIndexBuffer, a_eIndexFormat, m_uiImmediateIndexCurrentOffset );
	FlushConstantBuffers();
	m_pDeviceContext->IASetPrimitiveTopology( a_eTopology );
	m_pDeviceContext->DrawIndexed( a_uiIndexCount, 0, 0 );

	m_uiImmediateIndexCurrentOffset += uiIndexBufferSize;
	m_uiImmediateVertexCurrentOffset += uiVertexBufferSize;
}

// $deBlob: FUNCTION 006a76f0
void TRenderDX11::DrawIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, const void* a_pIndices, DXGI_FORMAT a_eIndexFormat, ID3D11Buffer* a_pVertexBuffer, UINT a_uiStride, UINT a_uiOffset )
{
	const TUINT32 uiIndexBufferSize = CopyToImmediateIndexBuffer( m_pDeviceContext, m_pImmediateIndexBuffer, m_uiImmediateIndexCurrentOffset, a_pIndices, a_uiIndexCount, a_eIndexFormat );

	UpdateRenderStates();
	m_pDeviceContext->IASetVertexBuffers( 0, 1, &a_pVertexBuffer, &a_uiStride, &a_uiOffset );
	m_pDeviceContext->IASetIndexBuffer( m_pImmediateIndexBuffer, a_eIndexFormat, m_uiImmediateIndexCurrentOffset );
	FlushConstantBuffers();
	m_pDeviceContext->IASetPrimitiveTopology( a_eTopology );
	m_pDeviceContext->DrawIndexed( a_uiIndexCount, 0, 0 );

	m_uiImmediateIndexCurrentOffset += uiIndexBufferSize;
}

// $deBlob: FUNCTION 006a7850
void TRenderDX11::SetImmediateVertexStream( const void* a_pVertices, UINT a_uiStride, UINT a_uiVertexCount, UINT a_uiSlot )
{
	const TUINT32 uiVertexBufferSize = CopyToImmediateVertexBuffer( m_pDeviceContext, m_pImmediateVertexBuffer, m_uiImmediateVertexCurrentOffset, a_pVertices, a_uiStride, a_uiVertexCount );

	m_pDeviceContext->IASetVertexBuffers( a_uiSlot, 1, &m_pImmediateVertexBuffer, &a_uiStride, &m_uiImmediateVertexCurrentOffset );
	m_uiImmediateVertexCurrentOffset += uiVertexBufferSize;
}

// $deBlob: FUNCTION 006a6f80
ID3D11RenderTargetView* TRenderDX11::CreateRenderTargetView( ID3D11ShaderResourceView* a_pShaderResourceView )
{
	ID3D11RenderTargetView* pRenderTargetView = TNULL;
	ID3D11Texture2D*        pTexture          = TNULL;
	a_pShaderResourceView->GetResource( TREINTERPRETCAST( ID3D11Resource**, &pTexture ) );

	D3D11_TEXTURE2D_DESC oTextureDesc;
	pTexture->GetDesc( &oTextureDesc );

	D3D11_RENDER_TARGET_VIEW_DESC oRTVDesc = {};
	oRTVDesc.Format                        = oTextureDesc.Format;
	oRTVDesc.ViewDimension                 = ( oTextureDesc.SampleDesc.Count > 1 ) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
	oRTVDesc.Texture2D.MipSlice            = 0;
	m_pDevice->CreateRenderTargetView( pTexture, &oRTVDesc, &pRenderTargetView );

	pTexture->Release();
	return pRenderTargetView;
}

// The number of mipmaps of a full chain for a texture of a_uiWidth x a_uiHeight
// $deBlob: FUNCTION 006a9500
static TUINT32 GetNumMipmaps( TUINT32 a_uiWidth, TUINT32 a_uiHeight )
{
	const TUINT32* pMax = ( a_uiWidth >= a_uiHeight ) ? &a_uiWidth : &a_uiHeight;
	return TUINT32( floor( log2( (double)*pMax ) ) + 1.0 );
}

// $deBlob: FUNCTION 006a9470
const TCHAR* TRenderDX11::GetFeatureLevel( D3D_FEATURE_LEVEL a_eFeatureLevel )
{
	switch ( a_eFeatureLevel )
	{
		case D3D_FEATURE_LEVEL_9_1: return "D3D_FEATURE_LEVEL_9_1";
		case D3D_FEATURE_LEVEL_9_2: return "D3D_FEATURE_LEVEL_9_2";
		case D3D_FEATURE_LEVEL_9_3: return "D3D_FEATURE_LEVEL_9_3";
		case D3D_FEATURE_LEVEL_10_0: return "D3D_FEATURE_LEVEL_10_0";
		case D3D_FEATURE_LEVEL_10_1: return "D3D_FEATURE_LEVEL_10_1";
		case D3D_FEATURE_LEVEL_11_0: return "D3D_FEATURE_LEVEL_11_0";
		case D3D_FEATURE_LEVEL_11_1: return "D3D_FEATURE_LEVEL_11_1";
	}

	return "unknown";
}

// $deBlob: FUNCTION 006a7940
void TRenderDX11::ShowDeviceError()
{
	TCHAR szTitle[ 1024 ];
	TCHAR szText[ 1024 ];

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail shows an uninitialised buffer as the text of the message box
	GetPrivateProfileStringA( "Setup", "IDS_D3DDEVICEERROR", "Failed to create the Direct3D 11 device.", szText, sizeof( szText ), ".\\Setup.ini" );
#endif

	GetPrivateProfileStringA( "Setup", "IDS_D3DDEVICEERRORTITLE", "Initialization failure", szTitle, sizeof( szTitle ), ".\\Setup.ini" );
	MessageBoxA( NULL, szText, szTitle, MB_OK );
}

// $deBlob: FUNCTION 006a79a0
void TRenderDX11::ShowDisplayError()
{
	TCHAR szTitle[ 1024 ];
	TCHAR szText[ 1024 ];

	GetPrivateProfileStringA( "Setup", "IDS_D3DDISPLAYERRORTITLE", "Initialization failure", szTitle, sizeof( szTitle ), ".\\Setup.ini" );
	GetPrivateProfileStringA( "Setup", "IDS_D3DDISPLAYERROR", "Failed to create the display. Please run the Barnyard setup program and reconfigure", szText, sizeof( szText ), ".\\Setup.ini" );
	MessageBoxA( NULL, szText, szTitle, MB_OK );
}

// $deBlob: FUNCTION 006a6200
void TRenderDX11::GetScreenOffset( TVector2& a_rVec )
{
	// NOTE: retail's is empty (a_rVec is left as it is)
}

// $deBlob: FUNCTION 006a6210
void TRenderDX11::SetScreenOffset( const TVector2& a_rVec )
{
	TRender::SetScreenOffset( a_rVec );
}

// $deBlob: FUNCTION 006a7b10
void TRenderDX11::FlushOrderTables()
{
	TASSERT( TTRUE == IsInScene() );

	for ( auto it = Interface()->m_OrderTables.Begin(); it != Interface()->m_OrderTables.End(); it++ )
		it->Flush();
}

// $deBlob: FUNCTION 006a7df0
void TRenderDX11::BuildAdapterDatabase()
{
	IDXGIFactory1* pFactory     = TNULL;
	IDXGIAdapter*  pDXGIAdapter = TNULL;

	if ( FAILED( CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), (void**)&pFactory ) ) )
		return;

	TUINT32 uiAdapterIndex = 0;

	while ( pFactory->EnumAdapters( uiAdapterIndex, &pDXGIAdapter ) != DXGI_ERROR_NOT_FOUND )
	{
		TD3DAdapter* pAdapter      = new TD3DAdapter();
		pAdapter->m_uiAdapterIndex = uiAdapterIndex;
		pDXGIAdapter->GetDesc( &pAdapter->m_AdapterDesc );

		TCHAR szDescription[ 128 ];
		TStringManager::String8Copy( pAdapter->m_szDescription, TStringManager::StringUnicodeToChar( szDescription, pAdapter->m_AdapterDesc.Description, -1 ) );

		pAdapter->m_AdapterLuid = pAdapter->m_AdapterDesc.AdapterLuid;
		pAdapter->m_VendorId    = pAdapter->m_AdapterDesc.VendorId;
		pAdapter->m_DeviceId    = pAdapter->m_AdapterDesc.DeviceId;
		pAdapter->m_SubSysId    = pAdapter->m_AdapterDesc.SubSysId;
		pAdapter->m_Revision    = pAdapter->m_AdapterDesc.Revision;

		TUtil::Log( "Adapter: %s", TStringManager::StringUnicodeToChar( szDescription, pAdapter->m_AdapterDesc.Description, -1 ) );
		TUtil::LogDown();
		TUtil::Log( "Vendor: %d, Device: %d Revision: %d", pAdapter->m_AdapterDesc.VendorId, pAdapter->m_AdapterDesc.DeviceId, pAdapter->m_AdapterDesc.Revision );
		TUtil::Log( "DedicatedSystemMemory: %.2f MB", TFLOAT( pAdapter->m_AdapterDesc.DedicatedSystemMemory ) * ( 1.0f / 1024.0f ) );
		TUtil::Log( "DedicatedVideoMemory : %.2f MB", TFLOAT( pAdapter->m_AdapterDesc.DedicatedVideoMemory ) * ( 1.0f / 1024.0f ) );
		TUtil::Log( "SharedSystemMemory   : %.2f MB", TFLOAT( pAdapter->m_AdapterDesc.SharedSystemMemory ) * ( 1.0f / 1024.0f ) );

		pAdapter->EnumerateOutputs( this, pDXGIAdapter );
		pAdapter->EnumerateDevices( this );
		GetAdapterList()->InsertTail( pAdapter );

		TUtil::LogUp();
		pDXGIAdapter->Release();
		uiAdapterIndex++;
	}

	if ( pFactory )
		pFactory->Release();
}

// $deBlob: FUNCTION 006a97d0
static TBOOL __stdcall InvalidateResourceCallback( TResource* a_pResource, void* a_pUserData )
{
	a_pResource->TryInvalidate();
	return TTRUE;
}

// $deBlob: FUNCTION 006a97f0
static void InvalidateShaderCallback( TShader* a_pShader )
{
	// NOTE: retail's callback is __stdcall and returns TTRUE (TShaderList::EnumerateShaders ignores it)
	a_pShader->TryInvalidate();
}

// Calls a_fnCallback for a_pNode, its children (depth first) and, if a_bSiblings, its next siblings; stops when
// the callback returns TFALSE
// $deBlob: FUNCTION 006ab710
static TBOOL RecurseResources( TBOOL( __stdcall* a_fnCallback )( TResource*, void* ), TResource* a_pNode, TBOOL a_bSiblings, void* a_pUserData )
{
	TResource* pFirst = a_pNode;
	TResource* pNode  = a_pNode;

	if ( pNode == TNULL )
		return TTRUE;

	while ( TTRUE )
	{
		TResource* pNext = pNode->Next();

		if ( pNext == pFirst || pNext == pNode || !a_bSiblings )
			pNext = TNULL;

		if ( !a_fnCallback( pNode, a_pUserData ) )
			return TFALSE;

		if ( pNode->Child() && !RecurseResources( a_fnCallback, pNode->Child(), TTRUE, a_pUserData ) )
			return TFALSE;

		pNode = pNext;

		if ( pNode == TNULL )
			return TTRUE;
	}
}

// Recurses from a_pNode (without its siblings) or else from the tree's first root child (with its siblings)
// NOTE: retail's is TNodeTree<TResource>::RecurseSimple (TNodeTree.h:533)
// $deBlob: FUNCTION 006ab780
TMAYBE_UNUSED static TBOOL RecurseResourcesSimple( TNodeTree<TResource>* a_pTree, TBOOL( __stdcall* a_fnCallback )( TResource*, void* ), TResource* a_pNode, void* a_pUserData )
{
	TASSERT( TNULL != a_fnCallback );

	if ( a_pNode != TNULL )
		return RecurseResources( a_fnCallback, a_pNode, TFALSE, a_pUserData );

	return RecurseResources( a_fnCallback, a_pTree->ChildOfRoot(), TTRUE, a_pUserData );
}

// $deBlob: FUNCTION 006a7a30
void TRenderDX11::OnDeviceLost()
{
	TSystemManager::GetSingleton()->Pause( TTRUE );

	if ( !m_bDeviceLost )
	{
		m_bDeviceLost = TTRUE;

		TASSERT( TNULL != InvalidateResourceCallback );
		RecurseResources( InvalidateResourceCallback, GetResources()->ChildOfRoot(), TTRUE, TNULL );
		TShader::sm_oShaderList.EnumerateShaders( InvalidateShaderCallback );

		if ( m_fnDeviceLostCallback )
			m_fnDeviceLostCallback();

		FlushDyingResources();

		// NOTE: retail pauses again (not a resume)
		TSystemManager::GetSingleton()->Pause( TTRUE );
	}
}

// Not supported: asserts (recreating the display, a reset device)
// $deBlob: FUNCTION 006a7ae0
void TRenderDX11::OnDeviceReset()
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006a4570
void TRenderDX11::WaitForEndOfRender()
{
	ID3D11Query*     pQuery = TNULL;
	D3D11_QUERY_DESC oQueryDesc;
	oQueryDesc.Query     = D3D11_QUERY_EVENT;
	oQueryDesc.MiscFlags = 0;
	m_pDevice->CreateQuery( &oQueryDesc, &pQuery );

	if ( pQuery != TNULL )
	{
		m_pDeviceContext->End( pQuery );
		m_pDeviceContext->Flush();

		// NOTE: retail never releases the query
		BOOL bDone = FALSE;
		do
		{
			m_pDeviceContext->GetData( pQuery, &bDone, sizeof( bDone ), 0 );
		} while ( !bDone );
	}
}

// $deBlob: FUNCTION 006a6910
void TRenderDX11::ResolveSubresource()
{
	m_pDeviceContext->ResolveSubresource( m_pResolveTexture, 0, m_pMainTexture, 0, DXGI_FORMAT_R8G8B8A8_UNORM );
}

// $deBlob: FUNCTION 006a6940
ID3D11SamplerState* TRenderDX11::CreateSamplerStateAutoAnisotropy( D3D11_FILTER a_eFilter, D3D11_TEXTURE_ADDRESS_MODE a_eAddressU, D3D11_TEXTURE_ADDRESS_MODE a_eAddressV, D3D11_TEXTURE_ADDRESS_MODE a_eAddressW, FLOAT a_fMipLODBias, TUINT32 a_uiBorderColour, FLOAT a_fMinLOD, FLOAT a_fMaxLOD )
{
	UINT uiMaxAnisotropy;

	if ( a_eFilter == D3D11_FILTER_MIN_MAG_MIP_LINEAR )
	{
		a_eFilter       = D3D11_FILTER_ANISOTROPIC;
		uiMaxAnisotropy = 16;
	}
	else
	{
		uiMaxAnisotropy = 1;
	}

	D3D11_SAMPLER_DESC oDesc;
	TUtil::MemClear( &oDesc, sizeof( oDesc ) );
	oDesc.AddressU         = a_eAddressU;
	oDesc.AddressV         = a_eAddressV;
	oDesc.MipLODBias       = a_fMipLODBias;
	oDesc.AddressW         = a_eAddressW;
	oDesc.BorderColor[ 0 ] = TFLOAT( ( a_uiBorderColour >> 24 ) & 0xFF ) / 255.0f;
	oDesc.BorderColor[ 1 ] = TFLOAT( ( a_uiBorderColour >> 16 ) & 0xFF ) / 255.0f;
	oDesc.BorderColor[ 2 ] = TFLOAT( ( a_uiBorderColour >> 8 ) & 0xFF ) / 255.0f;
	oDesc.BorderColor[ 3 ] = TFLOAT( a_uiBorderColour & 0xFF ) / 255.0f;
	oDesc.MinLOD           = a_fMinLOD;
	oDesc.MaxLOD           = a_fMaxLOD;
	oDesc.Filter           = a_eFilter;
	oDesc.MaxAnisotropy    = uiMaxAnisotropy;

	ID3D11SamplerState* pSamplerState;
	m_pDevice->CreateSamplerState( &oDesc, &pSamplerState );
	return pSamplerState;
}

// $deBlob: FUNCTION 006a6a90
void TRenderDX11::SetSamplerState( UINT a_uiStartSlot, TINT a_iSamplerId, BOOL a_bPixelShader )
{
	ID3D11SamplerState* pSamplerState = m_aSamplerStates[ a_iSamplerId ];

	if ( !a_bPixelShader )
		m_pDeviceContext->VSSetSamplers( a_uiStartSlot, 1, &pSamplerState );
	else
		m_pDeviceContext->PSSetSamplers( a_uiStartSlot, 1, &pSamplerState );
}

// $deBlob: FUNCTION 006a6ad0
void TRenderDX11::SetSamplerState( UINT a_uiStartSlot, ID3D11SamplerState* a_pSamplerState, BOOL a_bPixelShader )
{
	if ( !a_bPixelShader )
		m_pDeviceContext->VSSetSamplers( a_uiStartSlot, 1, &a_pSamplerState );
	else
		m_pDeviceContext->PSSetSamplers( a_uiStartSlot, 1, &a_pSamplerState );
}

// $deBlob: FUNCTION 006a6b00
void TRenderDX11::SetColorUpdate( TBOOL a_bUpdate )
{
	if ( a_bUpdate )
		m_CurrentBlendState.uiColour |= BLENDCOLOUR_WRITEMASK_RGB;
	else
		m_CurrentBlendState.uiColour &= ~BLENDCOLOUR_WRITEMASK_RGB;
}

// $deBlob: FUNCTION 006a6b30
void TRenderDX11::SetAlphaUpdate( TBOOL a_bUpdate )
{
	if ( a_bUpdate )
		m_CurrentBlendState.uiColour |= BLENDCOLOUR_WRITEMASK_ALPHA;
	else
		m_CurrentBlendState.uiColour &= ~BLENDCOLOUR_WRITEMASK_ALPHA;
}

// $deBlob: FUNCTION 006a7020
ID3D11Buffer* TRenderDX11::CreateBuffer( TUINT32 a_uiType, UINT a_uiDataSize, const void* a_pData, D3D11_USAGE a_eUsage, UINT a_uiCPUAccessFlags )
{
	ID3D11Buffer* pBuffer = TNULL;

	D3D11_BUFFER_DESC oDesc;
	oDesc.ByteWidth           = a_uiDataSize;
	oDesc.Usage               = a_eUsage;
	oDesc.BindFlags           = ( a_uiType == BUFFERTYPE_VERTEX ) ? D3D11_BIND_VERTEX_BUFFER : ( ( a_uiType == BUFFERTYPE_INDEX ) ? D3D11_BIND_INDEX_BUFFER : D3D11_BIND_CONSTANT_BUFFER );
	oDesc.CPUAccessFlags      = a_uiCPUAccessFlags;
	oDesc.MiscFlags           = 0;
	oDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA  oSubData;
	D3D11_SUBRESOURCE_DATA* pSubData = TNULL;

	if ( a_pData != TNULL )
	{
		oSubData.pSysMem          = a_pData;
		oSubData.SysMemPitch      = 0;
		oSubData.SysMemSlicePitch = 0;
		pSubData                  = &oSubData;
	}

	m_pDevice->CreateBuffer( &oDesc, pSubData, &pBuffer );
	return pBuffer;
}

// $deBlob: FUNCTION 006a70d0
void TRenderDX11::UpdateBuffer( ID3D11Buffer* a_pBuffer, TUINT32 a_uiSize, const void* a_pSrc, TUINT32 a_uiOffset, D3D11_MAP a_eMapType )
{
	D3D11_MAPPED_SUBRESOURCE oMapped;
	m_pDeviceContext->Map( a_pBuffer, 0, a_eMapType, 0, &oMapped );
	TUtil::MemCopy( TSTATICCAST( TUINT8, oMapped.pData ) + a_uiOffset, a_pSrc, a_uiSize );
	m_pDeviceContext->Unmap( a_pBuffer, 0 );
}

// $deBlob: FUNCTION 006a7120
void TRenderDX11::UpdateBufferRegion( ID3D11Buffer* a_pBuffer, TUINT32 a_uiSize, const void* a_pSrc, TUINT32 a_uiOffset )
{
	D3D11_BOX oBox;
	oBox.left   = a_uiOffset;
	oBox.top    = 0;
	oBox.front  = 0;
	oBox.right  = a_uiOffset + a_uiSize;
	oBox.bottom = 1;
	oBox.back   = 1;
	m_pDeviceContext->UpdateSubresource( a_pBuffer, 0, &oBox, a_pSrc, 0, 0 );
}

// $deBlob: FUNCTION 006a7240
void TRenderDX11::SetVec4InVSBuffer( TINT a_iIndex, const void* a_pSrc, TINT a_iCount )
{
	TASSERT( TUINT32( a_iIndex * 16 + a_iCount * 16 ) <= VERTEX_CONSTANT_BUFFER_SIZE && "Buffer size exceeded" );
	TUtil::MemCopy( m_pVertexConstantBuffer + a_iIndex * 16, a_pSrc, a_iCount * 16 );
	m_bVertexConstantBufferSet = TTRUE;
}

// $deBlob: FUNCTION 006a72b0
void TRenderDX11::SetVec4InPSBuffer( TINT a_iIndex, const void* a_pSrc, TINT a_iCount )
{
	TASSERT( TUINT32( a_iIndex * 16 + a_iCount * 16 ) <= PIXEL_CONSTANT_BUFFER_SIZE && "Buffer size exceeded" );
	TUtil::MemCopy( m_pPixelConstantBuffer + a_iIndex * 16, a_pSrc, a_iCount * 16 );
	m_bPixelConstantBufferSet = TTRUE;
}

// $deBlob: FUNCTION 006a7b40
void TRenderDX11::SetDstAlpha( TFLOAT a_fAlpha )
{
	if ( a_fAlpha < 0.0f )
	{
		// The alpha is blended like the colour
		const TUINT16 uiBlendOp     = m_CurrentBlendState.uiAlpha & BLENDALPHA_BLENDOPMASK;
		m_CurrentBlendState.uiAlpha = TUINT16( ( ( uiBlendOp << 2 ) | ( m_CurrentBlendState.uiColour & 0xFFE0 ) ) * 2 ) | uiBlendOp;
	}
	else
	{
		// Alpha = a_fAlpha (BlendOpAlpha ADD, SrcBlendAlpha BLEND_FACTOR, DestBlendAlpha ZERO)
		m_aCurrentBlendFactor[ 3 ]  = a_fAlpha;
		m_CurrentBlendState.uiAlpha = ( m_CurrentBlendState.uiAlpha & BLENDALPHA_BLENDOPMASK ) | ( D3D11_BLEND_ZERO << BLENDALPHA_DESTBLENDALPHASHIFT ) | ( D3D11_BLEND_BLEND_FACTOR << BLENDALPHA_SRCBLENDALPHASHIFT ) | ( D3D11_BLEND_OP_ADD << BLENDALPHA_BLENDOPALPHASHIFT );
	}
}

// $deBlob: FUNCTION 006a7bb0
TFLOAT TRenderDX11::GetDstAlpha() const
{
	return m_aCurrentBlendFactor[ 3 ];
}

// $deBlob: FUNCTION 006a7bc0
void TRenderDX11::SetZMode( TBOOL a_bDepthEnable, D3D11_COMPARISON_FUNC a_eComparisonFunc, D3D11_DEPTH_WRITE_MASK a_eDepthWriteMask )
{
	m_CurrentDepth.Byte( 0 ) = TUINT8( ( ( ( a_eComparisonFunc & 0xF ) * 2 | ( a_eDepthWriteMask & 1 ) ) * 2 ) | ( m_CurrentDepth.Byte( 0 ) & 0xC0 ) | a_bDepthEnable );
}

// $deBlob: FUNCTION 006a7bf0
void TRenderDX11::SetStencil( TBOOL a_bEnable, D3D11_COMPARISON_FUNC a_eFrontFunc, UINT a_uiStencilRef, TUINT8 a_uiMask, D3D11_STENCIL_OP a_eFailOp, D3D11_STENCIL_OP a_eDepthFailOp, D3D11_STENCIL_OP a_ePassOp )
{
	m_CurrentDepth.Byte( 0 )    = m_CurrentDepth.Byte( 0 ) ^ ( ( TUINT8( a_bEnable << 6 ) ^ m_CurrentDepth.Byte( 0 ) ) & 0x40 );
	m_CurrentDepth.uiStencilRef = a_uiStencilRef;
	m_CurrentDepth.Byte( 1 )    = a_uiMask;
	m_CurrentDepth.Byte( 2 )    = a_uiMask;
	m_CurrentDepth.Byte( 3 )    = TUINT8( ( a_eFailOp & 0xF ) | ( a_eDepthFailOp << 4 ) );
	m_CurrentDepth.Byte( 5 )    = TUINT8( ( a_eFailOp & 0xF ) | ( a_eDepthFailOp << 4 ) );
	m_CurrentDepth.Byte( 4 )    = TUINT8( ( a_eFrontFunc << 4 ) | ( a_ePassOp & 0xF ) );
	// NOTE: retail only sets the back face's pass operation (its function is left as it is)
	m_CurrentDepth.Byte( 6 ) = TUINT8( ( m_CurrentDepth.Byte( 6 ) & 0xF0 ) | ( a_ePassOp & 0xF ) );
}

// $deBlob: FUNCTION 006a7c80
void TRenderDX11::SetDepthBias( TFLOAT a_fDepth, TFLOAT a_fBias )
{
	static TFLOAT s_fDepthBiasUnit = 1.0f / TFLOAT( pow( 2.0, 24.0 ) );
	m_CurrentRasterizer.DepthBias  = TINT( ( ( 1.0f - a_fDepth ) + a_fBias ) / s_fDepthBiasUnit );
}

// $deBlob: FUNCTION 006a7d00
void TRenderDX11::SetBlendMode( TBOOL a_bBlendEnabled, D3D11_BLEND_OP a_eBlendOp, D3D11_BLEND a_eSrcBlend, D3D11_BLEND a_eDestBlend )
{
	const TUINT16 uiOldAlpha = m_CurrentBlendState.uiAlpha;
	const TUINT16 uiBlendOp  = TUINT16( a_eBlendOp & 7 );

	m_CurrentBlendState.uiAlpha  = ( uiOldAlpha & 0xFFF8 ) | uiBlendOp;
	m_CurrentBlendState.uiColour = TUINT16( ( m_CurrentBlendState.uiColour & 0x800F ) | ( ( a_bBlendEnabled | ( ( a_eSrcBlend & 0x1F ) << 1 ) ) << 4 ) | ( ( a_eDestBlend & 0x1F ) << 10 ) );

	// The alpha is blended like the colour unless SetDstAlpha set a constant alpha
	if ( ( uiOldAlpha & 0x7C0 ) != 0x380 )
		m_CurrentBlendState.uiAlpha = TUINT16( ( ( ( a_eSrcBlend & 0x1F ) << 3 | uiBlendOp ) << 3 ) | uiBlendOp | ( a_eDestBlend << 11 ) );
}

// $deBlob: FUNCTION 006a84d0
void TRenderDX11::DestroyOverlayShaders()
{
	if ( m_pOverlayVS )
	{
		m_pOverlayVS->Release();
		m_pOverlayVS = TNULL;
	}

	if ( m_pOverlayPS )
	{
		m_pOverlayPS->Release();
		m_pOverlayPS = TNULL;
	}

	if ( m_pOverlayOpaquePS )
	{
		m_pOverlayOpaquePS->Release();
		m_pOverlayOpaquePS = TNULL;
	}

	if ( m_pOverlayLayout )
	{
		m_pOverlayLayout->Release();
		m_pOverlayLayout = TNULL;
	}

	if ( m_pOverlayQuad )
	{
		m_pOverlayQuad->Release();
		m_pOverlayQuad = TNULL;
	}
}

// $deBlob: FUNCTION 006a8560
void TRenderDX11::Initialize()
{
	// NOTE: retail fills the descriptions inline (memset + fields)
	auto CreateSamplerState = [ this ]( D3D11_FILTER a_eFilter, D3D11_TEXTURE_ADDRESS_MODE a_eAddressU, D3D11_TEXTURE_ADDRESS_MODE a_eAddressV, D3D11_TEXTURE_ADDRESS_MODE a_eAddressW, FLOAT a_fMipLODBias ) {
		D3D11_SAMPLER_DESC oDesc;
		TUtil::MemClear( &oDesc, sizeof( oDesc ) );
		oDesc.Filter           = a_eFilter;
		oDesc.AddressU         = a_eAddressU;
		oDesc.AddressV         = a_eAddressV;
		oDesc.AddressW         = a_eAddressW;
		oDesc.MipLODBias       = a_fMipLODBias;
		oDesc.MinLOD           = 0.0f;
		oDesc.BorderColor[ 0 ] = 0.0f;
		oDesc.BorderColor[ 1 ] = 0.0f;
		oDesc.BorderColor[ 2 ] = 0.0f;
		oDesc.BorderColor[ 3 ] = 0.0f;
		oDesc.MaxLOD           = D3D11_FLOAT32_MAX;
		oDesc.MaxAnisotropy    = 1;

		ID3D11SamplerState* pSamplerState;
		m_pDevice->CreateSamplerState( &oDesc, &pSamplerState );
		return pSamplerState;
	};

	m_aSamplerStates[ SAMPLER_POINT_CLAMP ]          = CreateSamplerState( D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f );
	m_aSamplerStates[ SAMPLER_LINEAR_CLAMP ]         = CreateSamplerStateAutoAnisotropy( D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX );
	m_aSamplerStates[ SAMPLER_POINT_WRAP ]           = CreateSamplerState( D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f );
	m_aSamplerStates[ SAMPLER_LINEAR_WRAP ]          = CreateSamplerStateAutoAnisotropy( D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX );
	m_aSamplerStates[ SAMPLER_LINEAR_MIRROR ]        = CreateSamplerStateAutoAnisotropy( D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX );
	m_aSamplerStates[ SAMPLER_BILINEAR_CLAMP ]       = CreateSamplerState( D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f );
	m_aSamplerStates[ SAMPLER_BILINEAR_WRAP ]        = CreateSamplerState( D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f );
	m_aSamplerStates[ SAMPLER_BILINEAR_WRAP_BIAS ]   = CreateSamplerState( D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, -1.0f );
	m_aSamplerStates[ SAMPLER_POINT_WRAP_CLAMPV ]    = CreateSamplerState( D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f );
	m_aSamplerStates[ SAMPLER_LINEAR_WRAP_CLAMPV ]   = CreateSamplerStateAutoAnisotropy( D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX );
	m_aSamplerStates[ SAMPLER_BILINEAR_WRAP_CLAMPV ] = CreateSamplerState( D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f );
	m_aSamplerStates[ SAMPLER_ANISOTROPIC_CLAMP ]    = CreateSamplerState( D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f );

	D3D11_BUFFER_DESC oBufferDesc;

	for ( TUINT32 i = 0; i < NUMBUFFERS; i++ )
	{
		ID3D11Buffer* pBuffer           = TNULL;
		oBufferDesc.ByteWidth           = VERTEX_CONSTANT_BUFFER_SIZE;
		oBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
		oBufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		oBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		oBufferDesc.MiscFlags           = 0;
		oBufferDesc.StructureByteStride = 0;
		m_pDevice->CreateBuffer( &oBufferDesc, NULL, &pBuffer );
		m_aVertexBuffers[ i ] = pBuffer;
	}

	m_bVertexConstantBufferSet = TFALSE;
	m_uiVertexBufferIndex      = 0;
	m_pVertexConstantBuffer    = TSTATICCAST( TUINT8, TMemoryHeap_Malloc( s_pMemHeap, VERTEX_CONSTANT_BUFFER_SIZE ) );

	for ( TUINT32 i = 0; i < NUMBUFFERS; i++ )
	{
		ID3D11Buffer* pBuffer           = TNULL;
		oBufferDesc.ByteWidth           = PIXEL_CONSTANT_BUFFER_SIZE;
		oBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
		oBufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		oBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		oBufferDesc.MiscFlags           = 0;
		oBufferDesc.StructureByteStride = 0;
		m_pDevice->CreateBuffer( &oBufferDesc, NULL, &pBuffer );
		m_aPixelBuffers[ i ] = pBuffer;
	}

	m_pPixelConstantBuffer    = TSTATICCAST( TUINT8, TMemoryHeap_Malloc( s_pMemHeap, PIXEL_CONSTANT_BUFFER_SIZE ) );
	m_bPixelConstantBufferSet = TFALSE;
	m_uiPixelBufferIndex      = 0;

	// Immediate vertex buffer
	oBufferDesc.ByteWidth           = IMMEDIATE_VERTEX_BUFFER_SIZE;
	oBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
	oBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	oBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	oBufferDesc.MiscFlags           = 0;
	oBufferDesc.StructureByteStride = 0;
	m_pImmediateVertexBuffer        = TNULL;
	m_pDevice->CreateBuffer( &oBufferDesc, NULL, &m_pImmediateVertexBuffer );
	m_uiImmediateVertexCurrentOffset = 0;

	// Immediate index buffer
	oBufferDesc.ByteWidth           = IMMEDIATE_INDEX_BUFFER_SIZE;
	oBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
	oBufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	oBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	oBufferDesc.MiscFlags           = 0;
	oBufferDesc.StructureByteStride = 0;
	m_pImmediateIndexBuffer         = TNULL;
	m_pDevice->CreateBuffer( &oBufferDesc, NULL, &m_pImmediateIndexBuffer );
	m_uiImmediateIndexCurrentOffset = 0;

	// Depth: enabled, write all, LESS, no stencil (masks 0xFF, KEEP, ALWAYS)
	m_CurrentDepth.Byte( 0 ) = ( m_CurrentDepth.Byte( 0 ) & 0x8B ) | 0x0B;
	m_CurrentDepth.Byte( 1 ) = D3D11_DEFAULT_STENCIL_READ_MASK;
	m_CurrentDepth.Byte( 2 ) = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	m_CurrentDepth.Byte( 3 ) = D3D11_STENCIL_OP_KEEP | ( D3D11_STENCIL_OP_KEEP << 4 );
	m_CurrentDepth.Byte( 4 ) = D3D11_STENCIL_OP_KEEP | ( D3D11_COMPARISON_ALWAYS << 4 );
	m_CurrentDepth.Byte( 5 ) = D3D11_STENCIL_OP_KEEP | ( D3D11_STENCIL_OP_KEEP << 4 );
	m_CurrentDepth.Byte( 6 ) = D3D11_STENCIL_OP_KEEP | ( D3D11_COMPARISON_ALWAYS << 4 );

	// Blend: disabled, write all, ONE/ZERO/ADD for the colour and the alpha
	m_CurrentBlendState.uiAlpha  = ( D3D11_BLEND_ZERO << BLENDALPHA_DESTBLENDALPHASHIFT ) | ( D3D11_BLEND_ONE << BLENDALPHA_SRCBLENDALPHASHIFT ) | ( D3D11_BLEND_OP_ADD << BLENDALPHA_BLENDOPALPHASHIFT ) | D3D11_BLEND_OP_ADD;
	m_CurrentBlendState.uiColour = ( m_CurrentBlendState.uiColour & 0x844F ) | BLENDCOLOUR_WRITEMASK | ( D3D11_BLEND_ONE << BLENDCOLOUR_SRCBLENDSHIFT ) | ( D3D11_BLEND_ZERO << BLENDCOLOUR_DESTBLENDSHIFT );

	// Rasterizer: solid, cull back, clockwise front faces, depth clip
	m_CurrentRasterizer.DepthBias            = 0;
	m_CurrentRasterizer.SlopeScaledDepthBias = 0.0f;
	TREINTERPRETCAST( TUINT8*, &m_CurrentRasterizer.uiFlags )
	[0] = D3D11_FILL_SOLID | ( D3D11_CULL_BACK << RASTERIZERFLAGS_CULLSHIFT ) | BITFLAG( RASTERIZERFLAGS_DEPTHCLIPSHIFT );
}

// $deBlob: FUNCTION 006a8b40
void TRenderDX11::DestroyRenderStates()
{
	for ( TUINT32 i = 0; i < NUMSAMPLERSTATES; i++ )
	{
		if ( m_aSamplerStates[ i ] )
		{
			m_aSamplerStates[ i ]->Release();
			m_aSamplerStates[ i ] = TNULL;
		}
	}

	for ( TUINT32 i = 0; i < NUMBUFFERS; i++ )
	{
		if ( m_aVertexBuffers[ i ] )
		{
			m_aVertexBuffers[ i ]->Release();
			m_aVertexBuffers[ i ] = TNULL;
		}
	}

	operator delete( m_pVertexConstantBuffer );

	for ( TUINT32 i = 0; i < NUMBUFFERS; i++ )
	{
		if ( m_aPixelBuffers[ i ] )
		{
			m_aPixelBuffers[ i ]->Release();
			m_aPixelBuffers[ i ] = TNULL;
		}
	}

	operator delete( m_pPixelConstantBuffer );

	if ( m_pImmediateVertexBuffer )
	{
		m_pImmediateVertexBuffer->Release();
		m_pImmediateVertexBuffer = TNULL;
	}

	if ( m_pImmediateIndexBuffer )
	{
		m_pImmediateIndexBuffer->Release();
		m_pImmediateIndexBuffer = TNULL;
	}

	m_CurrentDepth.uiDepth      = 0;
	m_CurrentDepth.uiStencilRef = 0;

	for ( auto it = m_DepthStates.Begin(); it != m_DepthStates.End(); it++ )
		it->GetSecond()->Release();

	m_DepthStates.Clear();

	for ( auto it = m_RasterizerStates.Begin(); it != m_RasterizerStates.End(); it++ )
		it->GetSecond()->Release();

	m_RasterizerStates.Clear();

	for ( auto it = m_BlendStates.Begin(); it != m_BlendStates.End(); it++ )
		it->GetSecond()->Release();

	m_BlendStates.Clear();
}

// $deBlob: FUNCTION 006a8d30
void TRenderDX11::UpdateRenderStates()
{
	if ( m_CurrentDepth.uiDepth != m_uiPreviousDepth || m_CurrentDepth.uiStencilRef != m_uiPreviousStencilRef )
	{
		ID3D11DepthStencilState* pDepthStencilState;
		auto                     itDepth = m_DepthStates.Find( m_CurrentDepth.uiDepth );

		if ( itDepth == m_DepthStates.End() )
		{
			D3D11_DEPTH_STENCIL_DESC oDesc;
			TUtil::MemClear( &oDesc, sizeof( oDesc ) );

			const TUINT8 uiFlags               = m_CurrentDepth.Byte( 0 );
			oDesc.DepthEnable                  = uiFlags & DEPTHFLAGS_ENABLE;
			oDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK( ( uiFlags >> DEPTHFLAGS_WRITESHIFT ) & 1 );
			oDesc.DepthFunc                    = D3D11_COMPARISON_FUNC( ( uiFlags >> DEPTHFLAGS_FUNCSHIFT ) & 0xF );
			oDesc.StencilEnable                = ( uiFlags >> DEPTHFLAGS_STENCILSHIFT ) & 1;
			oDesc.StencilReadMask              = m_CurrentDepth.Byte( 1 );
			oDesc.StencilWriteMask             = m_CurrentDepth.Byte( 2 );
			oDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 3 ) & 0xF );
			oDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 3 ) >> 4 );
			oDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 4 ) & 0xF );
			oDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_FUNC( m_CurrentDepth.Byte( 4 ) >> 4 );
			oDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 5 ) & 0xF );
			oDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 5 ) >> 4 );
			oDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 6 ) & 0xF );
			oDesc.BackFace.StencilFunc         = D3D11_COMPARISON_FUNC( m_CurrentDepth.Byte( 6 ) >> 4 );

			HRESULT hr = m_pDevice->CreateDepthStencilState( &oDesc, &pDepthStencilState );
			TASSERT( SUCCEEDED( hr ) );

			m_DepthStates.Insert( m_CurrentDepth.uiDepth, pDepthStencilState );
		}
		else
		{
			pDepthStencilState = itDepth->GetSecond();
		}

		m_pDeviceContext->OMSetDepthStencilState( pDepthStencilState, m_CurrentDepth.uiStencilRef );
		m_uiPreviousDepth      = m_CurrentDepth.uiDepth;
		m_uiPreviousStencilRef = m_CurrentDepth.uiStencilRef;
	}

	if ( m_CurrentRasterizer != m_PreviousRasterizer )
	{
		ID3D11RasterizerState* pRasterizerState;
		auto                   itRasterizer = m_RasterizerStates.Find( m_CurrentRasterizer );

		if ( itRasterizer == m_RasterizerStates.End() )
		{
			const TUINT8 uiFlags = TUINT8( m_CurrentRasterizer.uiFlags );

			D3D11_RASTERIZER_DESC oDesc;
			oDesc.FillMode              = D3D11_FILL_MODE( uiFlags & RASTERIZERFLAGS_FILLMASK );
			oDesc.CullMode              = D3D11_CULL_MODE( ( uiFlags >> RASTERIZERFLAGS_CULLSHIFT ) & 3 );
			oDesc.FrontCounterClockwise = ( uiFlags >> RASTERIZERFLAGS_FRONTCCWSHIFT ) & 1;
			oDesc.DepthBias             = m_CurrentRasterizer.DepthBias;
			oDesc.DepthBiasClamp        = 0.0f;
			oDesc.SlopeScaledDepthBias  = m_CurrentRasterizer.SlopeScaledDepthBias;
			oDesc.DepthClipEnable       = ( uiFlags >> RASTERIZERFLAGS_DEPTHCLIPSHIFT ) & 1;
			oDesc.ScissorEnable         = ( uiFlags >> RASTERIZERFLAGS_SCISSORSHIFT ) & 1;
			oDesc.MultisampleEnable     = ( uiFlags >> RASTERIZERFLAGS_MULTISAMPLESHIFT ) & 1;
			oDesc.AntialiasedLineEnable = FALSE;

			HRESULT hr = m_pDevice->CreateRasterizerState( &oDesc, &pRasterizerState );
			TASSERT( SUCCEEDED( hr ) );

			m_RasterizerStates.Insert( m_CurrentRasterizer, pRasterizerState );
		}
		else
		{
			pRasterizerState = itRasterizer->GetSecond();
		}

		m_pDeviceContext->RSSetState( pRasterizerState );
		m_PreviousRasterizer = m_CurrentRasterizer;
	}

	// A constant alpha (BLEND_FACTOR, see SetDstAlpha) needs the blending: ONE/ZERO/ADD for the colour
	if ( ( m_CurrentBlendState.uiAlpha & BLENDALPHA_SRCBLENDALPHAMASK ) == ( D3D11_BLEND_BLEND_FACTOR << BLENDALPHA_SRCBLENDALPHASHIFT ) && ( m_CurrentBlendState.uiColour & BLENDCOLOUR_BLENDENABLE ) == 0 )
	{
		m_CurrentBlendState.uiAlpha  = ( m_CurrentBlendState.uiAlpha & 0xFFF9 ) | 1;
		m_CurrentBlendState.uiColour = ( m_CurrentBlendState.uiColour & 0x845F ) | 0x450;
	}

	if ( m_CurrentBlendState.uiRaw == m_PreviousBlendState.uiRaw && IsColorEqual( m_aCurrentBlendFactor, m_aPreviousBlendFactor ) )
		return;

	ID3D11BlendState* pBlendState;
	auto              itBlend = m_BlendStates.Find( m_CurrentBlendState.uiRaw );

	if ( itBlend == m_BlendStates.End() )
	{
		D3D11_BLEND_DESC oDesc;
		TUtil::MemClear( &oDesc, sizeof( oDesc ) );

		const TUINT16 uiColour                        = m_CurrentBlendState.uiColour;
		const TUINT16 uiAlpha                         = m_CurrentBlendState.uiAlpha;
		oDesc.RenderTarget[ 0 ].BlendEnable           = ( uiColour >> BLENDCOLOUR_BLENDENABLESHIFT ) & 1;
		oDesc.RenderTarget[ 0 ].SrcBlend              = D3D11_BLEND( ( uiColour >> BLENDCOLOUR_SRCBLENDSHIFT ) & 0x1F );
		oDesc.RenderTarget[ 0 ].DestBlend             = D3D11_BLEND( ( uiColour >> BLENDCOLOUR_DESTBLENDSHIFT ) & 0x1F );
		oDesc.RenderTarget[ 0 ].BlendOp               = D3D11_BLEND_OP( uiAlpha & BLENDALPHA_BLENDOPMASK );
		oDesc.RenderTarget[ 0 ].SrcBlendAlpha         = D3D11_BLEND( ( uiAlpha >> BLENDALPHA_SRCBLENDALPHASHIFT ) & 0x1F );
		oDesc.RenderTarget[ 0 ].DestBlendAlpha        = D3D11_BLEND( ( uiAlpha >> BLENDALPHA_DESTBLENDALPHASHIFT ) & 0x1F );
		oDesc.RenderTarget[ 0 ].BlendOpAlpha          = D3D11_BLEND_OP( ( uiAlpha >> BLENDALPHA_BLENDOPALPHASHIFT ) & BLENDALPHA_BLENDOPMASK );
		oDesc.RenderTarget[ 0 ].RenderTargetWriteMask = uiColour & BLENDCOLOUR_WRITEMASK;

		HRESULT hr = m_pDevice->CreateBlendState( &oDesc, &pBlendState );
		TASSERT( SUCCEEDED( hr ) );

		m_BlendStates.Insert( m_CurrentBlendState.uiRaw, pBlendState );
	}
	else
	{
		pBlendState = itBlend->GetSecond();
	}

	m_pDeviceContext->OMSetBlendState( pBlendState, m_aCurrentBlendFactor, 0xFFFFFFFF );
	m_PreviousBlendState.uiRaw = m_CurrentBlendState.uiRaw;
	TUtil::MemCopy( m_aPreviousBlendFactor, m_aCurrentBlendFactor, sizeof( m_aPreviousBlendFactor ) );
}

// $deBlob: FUNCTION 006a92f0
void TRenderDX11::FlushConstantBuffers()
{
	D3D11_MAPPED_SUBRESOURCE oMapped;

	if ( m_bVertexConstantBufferSet )
	{
		m_uiVertexBufferIndex = ( m_uiVertexBufferIndex + 1 ) % NUMBUFFERS;
		ID3D11Buffer* pBuffer = m_aVertexBuffers[ m_uiVertexBufferIndex ];

		m_pDeviceContext->Map( pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
		TUtil::MemCopy( oMapped.pData, m_pVertexConstantBuffer, VERTEX_CONSTANT_BUFFER_SIZE );
		m_pDeviceContext->Unmap( pBuffer, 0 );
		m_bVertexConstantBufferSet = TFALSE;
	}

	if ( m_bPixelConstantBufferSet )
	{
		m_uiPixelBufferIndex  = ( m_uiPixelBufferIndex + 1 ) % NUMBUFFERS;
		ID3D11Buffer* pBuffer = m_aPixelBuffers[ m_uiPixelBufferIndex ];

		m_pDeviceContext->Map( pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
		TUtil::MemCopy( oMapped.pData, m_pPixelConstantBuffer, PIXEL_CONSTANT_BUFFER_SIZE );
		m_pDeviceContext->Unmap( pBuffer, 0 );
		m_bPixelConstantBufferSet = TFALSE;
	}

	m_pDeviceContext->VSSetConstantBuffers( 0, 1, &m_aVertexBuffers[ m_uiVertexBufferIndex ] );
	m_pDeviceContext->PSSetConstantBuffers( 0, 1, &m_aPixelBuffers[ m_uiPixelBufferIndex ] );
}

// $deBlob: FUNCTION 00672dd0
TBOOL TRenderDX11::IsColorEqual( const FLOAT a_aColour1[ 4 ], const FLOAT a_aColour2[ 4 ] )
{
	return a_aColour1[ 0 ] == a_aColour2[ 0 ] && a_aColour1[ 1 ] == a_aColour2[ 1 ] && a_aColour1[ 2 ] == a_aColour2[ 2 ] && a_aColour1[ 3 ] == a_aColour2[ 3 ];
}

TOSHI_NAMESPACE_END
