#include "ToshiPCH.h"
#include "TRender_DX11.h"
#include "TRenderContext_DX11.h"
#include "TDevScreenshot_DX11.h"
#include "Toshi/TSystem.h"
#include "Toshi/TUtil.h"
#include "Toshi/TStringManager.h"

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "dxgi.lib" )

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TRenderDX11 );

// 0x008c9d74
static const D3D_DRIVER_TYPE s_aDriverTypes[] = {
	D3D_DRIVER_TYPE_HARDWARE,
	D3D_DRIVER_TYPE_WARP,
	D3D_DRIVER_TYPE_REFERENCE,
};

// 0x008c9d80 (the second try starts at 0x008c9d84, without 11_1)
static const D3D_FEATURE_LEVEL s_aFeatureLevels[] = {
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
};

// $deBlob: FUNCTION 006a46e0
TRenderDX11::TRenderDX11()
{
	TIMPLEMENT();

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

	m_bUnk6D4          = TFALSE;
	m_uiNumDrawnFrames = 0;
	m_bUnk730          = TFALSE;
	m_uiUnk734         = 0;
	m_uiUnk738         = 0;

	m_aClearColour[ 3 ] = 0.0f;
	m_aClearColour[ 2 ] = 0.0f;
	m_aClearColour[ 1 ] = 0.0f;
	m_aClearColour[ 0 ] = 0.0f;

	TUtil::MemClear( m_aSamplerStates, sizeof( m_aSamplerStates ) );

	// NOTE: not in retail's constructor (it's created by Create)
	m_hAccel = NULL;
#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: not in retail's constructor (set by CompileVSPS/CreateDisplay)
	m_pOverlayVS       = TNULL;
	m_pOverlayPS       = TNULL;
	m_pOverlayOpaquePS = TNULL;
	m_pOverlayLayout   = TNULL;
	m_pOverlayQuad     = TNULL;
	m_pUnk714          = TNULL;
	m_pPostProcess     = TNULL;
	m_pToneMap         = TNULL;
	m_pGlow            = TNULL;
	m_pFXAA            = TNULL;
	m_pDOF             = TNULL;
#endif

	// NOTE: the state caches use the global allocator, retail's use a T2Allocator (0x00981ea4) of its own sub heap
	// TMemory::CreateSubHeap( 0x10000, 0, "render states" ) (s_pMemHeap 0x009a87bc, created by the first one)
	TREINTERPRETCAST( TUINT8*, &m_CurrentRasterizer.uiFlags )[ 0 ]  = 0;
	m_CurrentRasterizer.DepthBias                                   = 0;
	m_CurrentRasterizer.SlopeScaledDepthBias                        = 0.0f;
	TREINTERPRETCAST( TUINT8*, &m_PreviousRasterizer.uiFlags )[ 0 ] = 0;
	m_PreviousRasterizer.DepthBias                                  = 0;
	m_PreviousRasterizer.SlopeScaledDepthBias                       = 0.0f;

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
	m_PreviousDepth.uiDepth      = 0;
	m_PreviousDepth.uiStencilRef = 0;
#endif

#if 0 // GHIDRA 006a46e0: TRenderDX11 * __fastcall Toshi::TRenderDX11::TRenderDX11(TRenderDX11 *this)
	pTVar2 = operator_new(4);
	if (pTVar2 != (TVertexDecl_DX11 *)0x0) {
		TVertexDecl_DX11::TVertexDecl_DX11(pTVar2);
	}
	pTVar3 = operator_new(4);
	if (pTVar3 != (TFXManager_DX11 *)0x0) {
		TFXManager_DX11::TFXManager_DX11(pTVar3);
	}
	pTVar4 = operator_new(4);
	if (pTVar4 != (TMaterialManager *)0x0) {
		TMaterialManager::TMaterialManager(pTVar4);
	}
#endif

	m_pVertexConstantBuffer    = TNULL;
	TUtil::MemClear( m_aVertexBuffers, sizeof( m_aVertexBuffers ) );
	m_bVertexConstantBufferSet = TFALSE;
	m_pPixelConstantBuffer     = TNULL;
	TUtil::MemClear( m_aPixelBuffers, sizeof( m_aPixelBuffers ) );
	m_bPixelConstantBufferSet        = TFALSE;
	m_pImmediateVertexBuffer         = TNULL;
	m_uiImmediateVertexCurrentOffset = 0;
	m_pImmediateIndexBuffer          = TNULL;
	m_uiImmediateIndexCurrentOffset  = 0;

#if 0 // GHIDRA 006a46e0: (continued)
	FUN_006ba4f0();
#endif

	m_CurrentDepth.uiDepth      = 0;
	m_CurrentDepth.uiStencilRef = 0;
}

// $deBlob: FUNCTION 006a4a20
TRenderDX11::~TRenderDX11()
{
	TIMPLEMENT();

#if 0 // GHIDRA 006a4a20: void __fastcall Toshi::TRenderDX11::~TRenderDX11(TRenderDX11 *this)
	pTVar1 = TTextureManager::s_Instance;
	if (TTextureManager::s_Instance != (TTextureManager *)0x0) {
		TTextureManager::~TTextureManager(TTextureManager::s_Instance);
		TFree(pTVar1);
	}
	piVar2 = TMaterialManager::s_Instance;
	if (TMaterialManager::s_Instance != (int *)0x0) {
		TMaterialManager::~TMaterialManager();
		TFree(piVar2);
	}
	piVar2 = TFXManager_DX11::s_Instance;
	if (TFXManager_DX11::s_Instance != (int *)0x0) {
		FUN_006ba860();
		TFree(piVar2);
	}
	piVar2 = TVertexDecl_DX11::s_Instance;
	if (TVertexDecl_DX11::s_Instance != (int *)0x0) {
		TVertexDecl_DX11::FUN_006bad30();
		TFree(piVar2);
	}
#endif

	Destroy();

#if 0 // GHIDRA 006a4a20: (continued)
	if (s_pMemHeap == 0) {
		TMemory::DestroyHeap((TMemoryHeap *)0x0);
		s_pMemHeap = 0;
	}
#endif

	m_OrderTables.RemoveAll();
}

// $deBlob: FUNCTION 006a4600
TRenderContext* TRender::CreateRenderContext()
{
	return new TRenderContextDX11( this );
}

// $deBlob: FUNCTION 006a4b00
TBOOL TRenderDX11::CreateDisplay( const DISPLAYPARAMS& a_rParams )
{
	TIMPLEMENT();

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

	const DXGI_MODE_DESC* pRefreshMode           = m_DisplayParams.bFullscreen ? &oFoundMode : pMode->GetDescription();
	m_SwapChainDesc.BufferDesc.RefreshRate       = pRefreshMode->RefreshRate;
	m_SwapChainDesc.OutputWindow                 = hWnd;
	m_SwapChainDesc.BufferUsage                  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_SwapChainDesc.SampleDesc.Count             = 1;
	m_SwapChainDesc.SampleDesc.Quality           = 0;
	m_SwapChainDesc.Windowed                     = TRUE;
	m_SwapChainDesc.SwapEffect                   = DXGI_SWAP_EFFECT_DISCARD;
	m_SwapChainDesc.Flags                        = m_DisplayParams.bFullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

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
		// NOTE: the arguments aren't visible in the decompilation
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

		m_pMainSRView = CreateTexture( m_DisplayParams.uiWidth, m_DisplayParams.uiHeight, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, 4, D3D11_USAGE_DEFAULT, 0, m_DisplayParams.uiMultisampleQualityLevel );

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

		m_pResolveSRView = CreateTexture( m_DisplayParams.uiWidth, m_DisplayParams.uiHeight, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, 4, D3D11_USAGE_DEFAULT, 0, 1 );
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

#if 0 // GHIDRA 006a4b00: void __thiscall Toshi::TRenderDX11::CreateDisplay(TRenderDX11 *this,TRender_DisplayParams *displayParams)
	pTVar16 = operator_new(0xc);
	if (pTVar16 != (TTextureManager *)0x0) {
		TTextureManager::TTextureManager(pTVar16);
	}
	sampleCount = FUN_007f3660(0x140,0x10);
	if (sampleCount == 0) {
		sampleCount = 0;
	}
	else {
		sampleCount = FUN_006b7140(sampleCount);
	}
	*(int *)&this->field_0x714 = sampleCount;
	pTVar17 = operator_new(0x7c);
	if (pTVar17 == (TPostProcess *)0x0) {
		pTVar17 = (TPostProcess *)0x0;
	}
	else {
		pTVar17 = TPostProcess::TPostProcess(pTVar17);
	}
	this->m_pPostProcess = pTVar17;
	pTVar18 = operator_new(0x1c);
	if (pTVar18 == (TToneMap *)0x0) {
		pTVar18 = (TToneMap *)0x0;
	}
	else {
		pTVar18 = TToneMap::TToneMap(pTVar18);
	}
	this->m_pToneMap = pTVar18;
	pTVar19 = operator_new(0x2c);
	if (pTVar19 == (TGlow *)0x0) {
		pTVar19 = (TGlow *)0x0;
	}
	else {
		pTVar19 = (TGlow *)TGlow::TGlow(pTVar19);
	}
	this->m_pGlow = pTVar19;
	pvVar20 = operator_new(0x28);
	if (pvVar20 == (void *)0x0) {
		uVar21 = 0;
	}
	else {
		uVar21 = FUN_006bab60();
	}
	*(undefined4 *)&this->field_0x728 = uVar21;
	pTVar22 = operator_new(0x18);
	if (pTVar22 == (TFXAA *)0x0) {
		pTVar22 = (TFXAA *)0x0;
	}
	else {
		TFXAA::TFXAA(pTVar22);
		pTVar22 = extraout_EAX;
	}
	this->m_FXAA = pTVar22;
	CompileVSPS(this);
#endif

	m_bDisplayCreated = TTRUE;
	return TTRUE;
}

// $deBlob: FUNCTION 006a5710
TBOOL TRenderDX11::RecreateDisplay( const DISPLAYPARAMS& a_rParams )
{
	TIMPLEMENT();

	TASSERT( TTRUE == IsCreated() );
	TASSERT( TTRUE == IsDisplayCreated() );

	if ( IsCreated() && IsDisplayCreated() )
	{
#if 0 // GHIDRA 006a5710: bool __thiscall Toshi::TRenderDX11::RecreateDisplay(TRenderDX11 *this,undefined4 param_2)
		FlushResources_(this);
#endif
		DestroyDisplay();

		if ( CreateDisplay( a_rParams ) )
		{
			// NOTE: retail asserts here (TRender_DX11.cpp:1933, recreating the display isn't supported)
			TASSERT( TFALSE );
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006a57e0
TBOOL TRenderDX11::DestroyDisplay()
{
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
	TIMPLEMENT();

	TASSERT( TTRUE == IsDisplayCreated() );
	TASSERT( IsInScene() == TTRUE );

	m_uiNumDrawnFrames += 1;

	if ( !m_bIsWidescreen )
	{
		// The main render target has the size of the back buffer
		if ( m_DisplayParams.uiMultisampleQualityLevel < 2 )
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

		if ( m_DisplayParams.uiMultisampleQualityLevel < 2 )
		{
			if ( s_pMainTextureSRV == TNULL )
				m_pDevice->CreateShaderResourceView( m_pMainTexture, NULL, &s_pMainTextureSRV );

			const TFLOAT fInAR  = TFLOAT( TINT( m_DisplayParams.uiWidth ) ) / TFLOAT( TINT( m_DisplayParams.uiHeight ) );
			const TFLOAT fOutAR = TFLOAT( TINT( m_uiDisplayWidth ) ) / TFLOAT( TINT( m_uiDisplayHeight ) );
			TASSERT( fabsf( fInAR - 16.0f / 9.0f ) < 0.01 );

#if 0 // GHIDRA 006a5a00: void __fastcall Toshi::TRenderDX11::EndScene(TRenderDX11 *param_1)
			if (outAR < inAR) {
				width = (float)(int)param_1->m_DisplayWidth;
				height = width / inAR;
				posX = 0.0;
				posY = ((float)(int)param_1->m_DisplayHeight - height) * 0.5;
			}
			else {
				posY = 0.0;
				height = (float)(int)param_1->m_DisplayHeight;
				width = height * inAR;
				posX = ((float)(int)param_1->m_DisplayWidth - width) * 0.5;
			}
			param_1->m_CurrentBlendState2 = param_1->m_CurrentBlendState2 & 0xffef;
			RenderOverlay(posX,posY,width,height,s_pShaderResourceView,(ID3D11PixelShader *)0x0,
										(float *)0x0);
#endif
			( void )fOutAR;
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
#if 0 // GHIDRA 006a5a00: (continued)
			FlushResources_(param_1);
#endif
		}
		else if ( hRes == DXGI_ERROR_DEVICE_RESET )
		{
			TASSERT( TFALSE );
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

// $deBlob: FUNCTION 006a5e20
TBOOL TRenderDX11::Create()
{
	return Create( "de Blob" );
}

// $deBlob: FUNCTION 006a5e30
TBOOL TRenderDX11::Create( LPCSTR a_szTitle )
{
	TIMPLEMENT();
	TASSERT( TFALSE == IsCreated() );

	if ( !TRender::Create() )
		return TFALSE;

	TUtil::Log( "Creating TRenderDX11" );
	TUtil::LogUp();

	D3D_FEATURE_LEVEL eFeatureLevel;

	for ( TUINT i = 0; ; )
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

	if ( m_hAccel != NULL )
	{
		DestroyAcceleratorTable( m_hAccel );
		m_hAccel = NULL;
	}

	// Escape and Alt+Enter
	ACCEL aAccels[ 2 ];
	aAccels[ 0 ].fVirt = FVIRTKEY;
	aAccels[ 0 ].key   = VK_ESCAPE;
	aAccels[ 0 ].cmd   = 0;
	aAccels[ 1 ].fVirt = FALT;
	aAccels[ 1 ].key   = VK_RETURN;
	aAccels[ 1 ].cmd   = 1;
	m_hAccel           = CreateAcceleratorTableA( aAccels, 2 );

	if ( m_Window.Create( this, a_szTitle ) )
	{
		if ( m_bCreateSystemResources )
			CreateSystemResources();

		TUtil::LogDown();
		m_eAspectRatio = ASPECT_RATIO_16_9;
		return TTRUE;
	}

	TUtil::Log( "Failed to create window" );
	return TFALSE;
}

// $deBlob: FUNCTION 006a5fc0
TBOOL TRenderDX11::Destroy()
{
	TIMPLEMENT();

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

	if ( m_hAccel != NULL )
	{
		DestroyAcceleratorTable( m_hAccel );
		m_hAccel = NULL;
	}

	if ( m_pDeviceContext )
	{
		m_pDeviceContext->Release();
		m_pDeviceContext = TNULL;
	}

	DestroyOverlayShaders();

#if 0 // GHIDRA 006a5fc0: bool __fastcall Toshi::TRenderDX11::Destroy(TRenderDX11 *param_1)
	iVar8 = *(int *)&param_1->field_0x714;
	if (iVar8 != 0) {
		FUN_006b7200();
		FUN_007f3648(iVar8);
	}
	pTVar9 = param_1->m_pPostProcess;
	if (pTVar9 != (TPostProcess *)0x0) {
		FUN_006b8f40();
		TFree(pTVar9);
	}
	pTVar10 = param_1->m_pToneMap;
	if (pTVar10 != (TToneMap *)0x0) {
		TToneMap::~TToneMap(pTVar10);
		TFree((int *)pTVar10);
	}
	pTVar11 = param_1->m_pGlow;
	if (pTVar11 != (TGlow *)0x0) {
		FUN_006b47c0();
		TFree((int *)pTVar11);
	}
	pTVar12 = param_1->m_FXAA;
	if (pTVar12 != (TFXAA *)0x0) {
		TFXAA::~TFXAA(pTVar12);
		TFree((int *)pTVar12);
	}
	piVar4 = *(int **)&param_1->field_0x728;
	if (piVar4 != (int *)0x0) {
		FUN_006babc0();
		TFree(piVar4);
	}
#endif

	return TRender::Destroy();
}

// $deBlob: FUNCTION 006a6b60
ID3D11ShaderResourceView* TRenderDX11::CreateTexture( TUINT32 a_uiWidth, TUINT32 a_uiHeight, DXGI_FORMAT a_eFormat, const void* a_pSrcData, TUINT8 a_uiFlags, D3D11_USAGE a_eUsage, TUINT32 a_uiCPUAccessFlags, TUINT32 a_uiSampleCount )
{
	TIMPLEMENT();

	ID3D11ShaderResourceView* pShaderResourceView = TNULL;

	const TBOOL bMipmapsFromData = ( a_uiFlags & 1 ) != 0;
	const TBOOL bGenerateMipmaps = !bMipmapsFromData && ( a_uiFlags & 2 ) != 0;
	TUINT32     uiNumMipmaps     = 1;

	if ( bMipmapsFromData || bGenerateMipmaps )
	{
#if 0 // GHIDRA 006a6b60: ID3D11ShaderResourceView * __thiscall Toshi::TRenderDX11::CreateTexture (TRenderDX11 *this,uint width,uint height,DXGI_FORMAT format,int srcData,byte flags, D3D11_USAGE usage,UINT cpuAccessFlags,uint sampleDescCount)
		local_fc = height;
		numberOfMipmaps = width;
		puVar1 = &local_fc;
		if (height <= width) {
			puVar1 = &numberOfMipmaps;
		}
		dVar4 = FUN_007f3afa((double)(int)*puVar1 + (&dZero)[-((int)*puVar1 >> 0x1f)]);
		floor(SUB84(dVar4,0),(double)CONCAT44(unaff_ESI,unaff_EDI));
		local_114 = (double)fVar3;
		auVar3._0_8_ = local_114 + 1.0;
		auVar3._8_8_ = 0;
		numberOfMipmaps = _dtoui3(auVar3);
#endif
		return TNULL;
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
	oDesc.BindFlags            = ( ( a_uiFlags & 4 ) || bGenerateMipmaps ) ? ( D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET ) : D3D11_BIND_SHADER_RESOURCE;
	oDesc.CPUAccessFlags       = a_uiCPUAccessFlags;
	oDesc.MiscFlags            = bGenerateMipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	ID3D11Texture2D* pTexture = TNULL;

	if ( a_pSrcData == TNULL )
	{
		m_pDevice->CreateTexture2D( &oDesc, NULL, &pTexture );
	}
	else
	{
#if 0 // GHIDRA 006a6b60: (continued)
		if (noMipLevels == false) {
			if ((_DAT_009a8818 == 0) && (0xf < numberOfMipmaps)) {
				TDebug_AssertHandler
									("numberOfMipmaps <= MAXIMUM_NUMBER_OF_MIPMAPS",
									"..\\..\\Source\\Render\\TRender_DX11.cpp",0x514,"TASSERT");
				_DAT_009a8818 = 1;
			}
			if (firstFlag == 0) {
				local_bc.pSysMem = pSrcData;
				local_bc.SysMemPitch = GetTextureRowPitch(format,width);
				local_bc.SysMemSlicePitch = GetTextureDepthPitch(format,width,height);
			}
			else if (numberOfMipmaps != 0) {
				(... one D3D11_SUBRESOURCE_DATA per mipmap, halving width and height down to 1 ...)
			}
			pDVar5 = &local_bc;
			goto LAB_006a6ea7;
		}
		(*(code *)this->m_pDevice->vftable->CreateTexture2D)(this->m_pDevice,&local_e8,0);
		if (pTexture == (ID3D11Resource *)0x0) goto LAB_006a6f58;
		SrcRowPitch = GetTextureRowPitch(format,width);
		SrcDepthPitch = GetTextureDepthPitch(format,width,height);
		(*(code *)local_108->m_pDeviceContext->vftable->UpdateSubresource)
							(local_108->m_pDeviceContext,pTexture,0,0,pSrcData,SrcRowPitch,SrcDepthPitch);
#endif
		return TNULL;
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
	// NOTE: retail shows an uninitialised buffer as the text of the message box (see Docs/RetailBugs.md)
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
	// NOTE: TRender::FlushShaders 00691190 asserts the same and calls this
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
		TUtil::LogUp();
		TUtil::Log( "Vendor: %d, Device: %d Revision: %d", pAdapter->m_AdapterDesc.VendorId, pAdapter->m_AdapterDesc.DeviceId, pAdapter->m_AdapterDesc.Revision );
		TUtil::Log( "DedicatedSystemMemory: %.2f MB", TFLOAT( pAdapter->m_AdapterDesc.DedicatedSystemMemory ) * ( 1.0f / 1024.0f ) );
		TUtil::Log( "DedicatedVideoMemory : %.2f MB", TFLOAT( pAdapter->m_AdapterDesc.DedicatedVideoMemory ) * ( 1.0f / 1024.0f ) );
		TUtil::Log( "SharedSystemMemory   : %.2f MB", TFLOAT( pAdapter->m_AdapterDesc.SharedSystemMemory ) * ( 1.0f / 1024.0f ) );

		pAdapter->EnumerateOutputs( this, pDXGIAdapter );
		GetAdapterList()->InsertTail( pAdapter );

		TUtil::LogDown();
		pDXGIAdapter->Release();
		uiAdapterIndex++;
	}

	if ( pFactory )
		pFactory->Release();
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
	oDesc.AddressU       = a_eAddressU;
	oDesc.AddressV       = a_eAddressV;
	oDesc.MipLODBias     = a_fMipLODBias;
	oDesc.AddressW       = a_eAddressW;
	oDesc.BorderColor[ 0 ] = TFLOAT( ( a_uiBorderColour >> 24 ) & 0xFF ) / 255.0f;
	oDesc.BorderColor[ 1 ] = TFLOAT( ( a_uiBorderColour >> 16 ) & 0xFF ) / 255.0f;
	oDesc.BorderColor[ 2 ] = TFLOAT( ( a_uiBorderColour >> 8 ) & 0xFF ) / 255.0f;
	oDesc.BorderColor[ 3 ] = TFLOAT( a_uiBorderColour & 0xFF ) / 255.0f;
	oDesc.MinLOD         = a_fMinLOD;
	oDesc.MaxLOD         = a_fMaxLOD;
	oDesc.Filter         = a_eFilter;
	oDesc.MaxAnisotropy  = uiMaxAnisotropy;

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
		m_CurrentBlendState.uiColour |= 7;
	else
		m_CurrentBlendState.uiColour &= 0xFFF8;
}

// $deBlob: FUNCTION 006a6b30
void TRenderDX11::SetAlphaUpdate( TBOOL a_bUpdate )
{
	if ( a_bUpdate )
		m_CurrentBlendState.uiColour |= 8;
	else
		m_CurrentBlendState.uiColour &= 0xFFF7;
}

// $deBlob: FUNCTION 006a7020
ID3D11Buffer* TRenderDX11::CreateBuffer( TUINT32 a_uiType, UINT a_uiDataSize, const void* a_pData, D3D11_USAGE a_eUsage, UINT a_uiCPUAccessFlags )
{
	ID3D11Buffer* pBuffer = TNULL;

	D3D11_BUFFER_DESC oDesc;
	oDesc.ByteWidth           = a_uiDataSize;
	oDesc.Usage               = a_eUsage;
	oDesc.BindFlags           = ( a_uiType == 0 ) ? D3D11_BIND_VERTEX_BUFFER : ( ( a_uiType == 1 ) ? D3D11_BIND_INDEX_BUFFER : D3D11_BIND_CONSTANT_BUFFER );
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
		const TUINT16 uiBlendOp      = m_CurrentBlendState.uiAlpha & 7;
		m_CurrentBlendState.uiAlpha  = TUINT16( ( ( uiBlendOp << 2 ) | ( m_CurrentBlendState.uiColour & 0xFFE0 ) ) * 2 ) | uiBlendOp;
	}
	else
	{
		// Alpha = a_fAlpha (BlendOpAlpha ADD, SrcBlendAlpha BLEND_FACTOR, DestBlendAlpha ZERO)
		m_aCurrentBlendFactor[ 3 ]  = a_fAlpha;
		m_CurrentBlendState.uiAlpha = ( m_CurrentBlendState.uiAlpha & 7 ) | 0xB88;
	}
}

// $deBlob: FUNCTION 006a7bc0
void TRenderDX11::SetZMode( TBOOL a_bDepthEnable, D3D11_COMPARISON_FUNC a_eComparisonFunc, D3D11_DEPTH_WRITE_MASK a_eDepthWriteMask )
{
	m_CurrentDepth.Byte( 0 ) = TUINT8( ( ( ( a_eComparisonFunc & 0xF ) * 2 | ( a_eDepthWriteMask & 1 ) ) * 2 ) | ( m_CurrentDepth.Byte( 0 ) & 0xC0 ) | a_bDepthEnable );
}

// $deBlob: FUNCTION 006a7bf0
void TRenderDX11::SetStencil( TBOOL a_bEnable, D3D11_COMPARISON_FUNC a_eFrontFunc, UINT a_uiStencilRef, TUINT8 a_uiMask, D3D11_STENCIL_OP a_eFailOp, D3D11_STENCIL_OP a_eDepthFailOp, D3D11_STENCIL_OP a_ePassOp )
{
	m_CurrentDepth.Byte( 0 )   = m_CurrentDepth.Byte( 0 ) ^ ( ( TUINT8( a_bEnable << 6 ) ^ m_CurrentDepth.Byte( 0 ) ) & 0x40 );
	m_CurrentDepth.uiStencilRef = a_uiStencilRef;
	m_CurrentDepth.Byte( 1 )   = a_uiMask;
	m_CurrentDepth.Byte( 2 )   = a_uiMask;
	m_CurrentDepth.Byte( 3 )   = TUINT8( ( a_eFailOp & 0xF ) | ( a_eDepthFailOp << 4 ) );
	m_CurrentDepth.Byte( 5 )   = TUINT8( ( a_eFailOp & 0xF ) | ( a_eDepthFailOp << 4 ) );
	m_CurrentDepth.Byte( 4 )   = TUINT8( ( a_eFrontFunc << 4 ) | ( a_ePassOp & 0xF ) );
	// NOTE: retail only sets the back face's pass operation (its function is left as it is)
	m_CurrentDepth.Byte( 6 ) = TUINT8( ( m_CurrentDepth.Byte( 6 ) & 0xF0 ) | ( a_ePassOp & 0xF ) );
}

// $deBlob: FUNCTION 006a7c80
void TRenderDX11::SetDepthBias( TFLOAT a_fDepth, TFLOAT a_fBias )
{
	static TFLOAT s_fDepthBiasUnit = 1.0f / TFLOAT( pow( 2.0, 24.0 ) ); // 0x009a887c
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

	// NOTE: retail allocates the CPU copies from its "render states" heap
	m_bVertexConstantBufferSet = TFALSE;
	m_uiVertexBufferIndex      = 0;
	m_pVertexConstantBuffer    = TSTATICCAST( TUINT8, TMalloc( VERTEX_CONSTANT_BUFFER_SIZE ) );

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

	m_pPixelConstantBuffer    = TSTATICCAST( TUINT8, TMalloc( PIXEL_CONSTANT_BUFFER_SIZE ) );
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
	m_CurrentDepth.Byte( 1 ) = 0xFF;
	m_CurrentDepth.Byte( 2 ) = 0xFF;
	m_CurrentDepth.Byte( 3 ) = 0x11;
	m_CurrentDepth.Byte( 4 ) = 0x81;
	m_CurrentDepth.Byte( 5 ) = 0x11;
	m_CurrentDepth.Byte( 6 ) = 0x81;

	// Blend: disabled, write all, ONE/ZERO/ADD for the colour and the alpha
	m_CurrentBlendState.uiAlpha  = 0x889;
	m_CurrentBlendState.uiColour = ( m_CurrentBlendState.uiColour & 0x844F ) | 0x44F;

	// Rasterizer: solid, cull back, clockwise front faces, depth clip
	m_CurrentRasterizer.DepthBias                         = 0;
	m_CurrentRasterizer.SlopeScaledDepthBias              = 0.0f;
	TREINTERPRETCAST( TUINT8*, &m_CurrentRasterizer.uiFlags )[ 0 ] = 0x2F;
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

	TFree( m_pVertexConstantBuffer );

	for ( TUINT32 i = 0; i < NUMBUFFERS; i++ )
	{
		if ( m_aPixelBuffers[ i ] )
		{
			m_aPixelBuffers[ i ]->Release();
			m_aPixelBuffers[ i ] = TNULL;
		}
	}

	TFree( m_pPixelConstantBuffer );

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
	if ( m_CurrentDepth != m_PreviousDepth )
	{
		ID3D11DepthStencilState* pDepthStencilState;
		auto                     itDepth = m_DepthStates.Find( m_CurrentDepth.uiDepth );

		if ( itDepth == m_DepthStates.End() )
		{
			D3D11_DEPTH_STENCIL_DESC oDesc;
			TUtil::MemClear( &oDesc, sizeof( oDesc ) );

			const TUINT8 uiFlags                = m_CurrentDepth.Byte( 0 );
			oDesc.DepthEnable                   = uiFlags & 1;
			oDesc.DepthWriteMask                = D3D11_DEPTH_WRITE_MASK( ( uiFlags >> 1 ) & 1 );
			oDesc.DepthFunc                     = D3D11_COMPARISON_FUNC( ( uiFlags >> 2 ) & 0xF );
			oDesc.StencilEnable                 = ( uiFlags >> 6 ) & 1;
			oDesc.StencilReadMask               = m_CurrentDepth.Byte( 1 );
			oDesc.StencilWriteMask              = m_CurrentDepth.Byte( 2 );
			oDesc.FrontFace.StencilFailOp       = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 3 ) & 0xF );
			oDesc.FrontFace.StencilDepthFailOp  = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 3 ) >> 4 );
			oDesc.FrontFace.StencilPassOp       = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 4 ) & 0xF );
			oDesc.FrontFace.StencilFunc         = D3D11_COMPARISON_FUNC( m_CurrentDepth.Byte( 4 ) >> 4 );
			oDesc.BackFace.StencilFailOp        = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 5 ) & 0xF );
			oDesc.BackFace.StencilDepthFailOp   = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 5 ) >> 4 );
			oDesc.BackFace.StencilPassOp        = D3D11_STENCIL_OP( m_CurrentDepth.Byte( 6 ) & 0xF );
			oDesc.BackFace.StencilFunc          = D3D11_COMPARISON_FUNC( m_CurrentDepth.Byte( 6 ) >> 4 );

			HRESULT hr = m_pDevice->CreateDepthStencilState( &oDesc, &pDepthStencilState );
			TASSERT( SUCCEEDED( hr ) );

			m_DepthStates.Insert( m_CurrentDepth.uiDepth, pDepthStencilState );
		}
		else
		{
			pDepthStencilState = itDepth->GetSecond();
		}

		m_pDeviceContext->OMSetDepthStencilState( pDepthStencilState, m_CurrentDepth.uiStencilRef );
		m_PreviousDepth = m_CurrentDepth;
	}

	if ( m_CurrentRasterizer != m_PreviousRasterizer )
	{
		ID3D11RasterizerState* pRasterizerState;
		auto                   itRasterizer = m_RasterizerStates.Find( m_CurrentRasterizer );

		if ( itRasterizer == m_RasterizerStates.End() )
		{
			const TUINT8 uiFlags = TUINT8( m_CurrentRasterizer.uiFlags );

			D3D11_RASTERIZER_DESC oDesc;
			oDesc.FillMode              = D3D11_FILL_MODE( uiFlags & 3 );
			oDesc.CullMode              = D3D11_CULL_MODE( ( uiFlags >> 2 ) & 3 );
			oDesc.FrontCounterClockwise = ( uiFlags >> 4 ) & 1;
			oDesc.DepthBias             = m_CurrentRasterizer.DepthBias;
			oDesc.DepthBiasClamp        = 0.0f;
			oDesc.SlopeScaledDepthBias  = m_CurrentRasterizer.SlopeScaledDepthBias;
			oDesc.DepthClipEnable       = ( uiFlags >> 5 ) & 1;
			oDesc.ScissorEnable         = ( uiFlags >> 6 ) & 1;
			oDesc.MultisampleEnable     = ( uiFlags >> 7 ) & 1;
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
	if ( ( m_CurrentBlendState.uiAlpha & 0x7C0 ) == 0x380 && ( m_CurrentBlendState.uiColour & 0x10 ) == 0 )
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

		const TUINT16 uiColour                         = m_CurrentBlendState.uiColour;
		const TUINT16 uiAlpha                          = m_CurrentBlendState.uiAlpha;
		oDesc.RenderTarget[ 0 ].BlendEnable            = ( uiColour >> 4 ) & 1;
		oDesc.RenderTarget[ 0 ].SrcBlend               = D3D11_BLEND( ( uiColour >> 5 ) & 0x1F );
		oDesc.RenderTarget[ 0 ].DestBlend              = D3D11_BLEND( ( uiColour >> 10 ) & 0x1F );
		oDesc.RenderTarget[ 0 ].BlendOp                = D3D11_BLEND_OP( uiAlpha & 7 );
		oDesc.RenderTarget[ 0 ].SrcBlendAlpha          = D3D11_BLEND( ( uiAlpha >> 6 ) & 0x1F );
		oDesc.RenderTarget[ 0 ].DestBlendAlpha         = D3D11_BLEND( ( uiAlpha >> 11 ) & 0x1F );
		oDesc.RenderTarget[ 0 ].BlendOpAlpha           = D3D11_BLEND_OP( ( uiAlpha >> 3 ) & 7 );
		oDesc.RenderTarget[ 0 ].RenderTargetWriteMask  = uiColour & 0xF;

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
