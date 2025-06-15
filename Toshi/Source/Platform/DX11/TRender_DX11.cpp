#include "ToshiPCH.h"
#include "TRender_DX11.h"
#include "T2Material_DX11.h"
#include "TRenderContext_DX11.h"
#include "Toshi/Core/TSystem.h"

#include <d3dcompiler.h>

TOSHI_NAMESPACE_START

UINT                      TRenderDX11::s_QualityLevel        = 1;
TBOOL                     TRenderDX11::s_bPresentTest        = TFALSE;
TMemoryHeap*              TRenderDX11::s_pMemHeap            = TNULL;
ID3D11ShaderResourceView* TRenderDX11::s_pShaderResourceView = TNULL;

TRenderContext* TRender::CreateRenderContext()
{
	return new TRenderContextDX11(this);
}

TRenderAdapter* TD3DAdapter::Mode::GetAdapter() const
{
	return m_Adapter;
}

size_t TD3DAdapter::Mode::GetModeIndex() const
{
	return m_ModeIndex;
}

TUINT32 TD3DAdapter::Mode::GetWidth() const
{
	return m_Description.Width;
}

TUINT32 TD3DAdapter::Mode::GetHeight() const
{
	return m_Description.Height;
}

TBOOL TD3DAdapter::Mode::SomeCheck1() const
{
	DXGI_FORMAT format = m_Description.Format;

	return (format == DXGI_FORMAT_B8G8R8X8_UNORM || format == DXGI_FORMAT_R8G8B8A8_UNORM);
}

TBOOL TD3DAdapter::Mode::SomeCheck2() const
{
	DXGI_FORMAT format = m_Description.Format;

	return (format == DXGI_FORMAT_B5G6R5_UNORM || format == DXGI_FORMAT_B5G5R5A1_UNORM || format == DXGI_FORMAT_B4G4R4A4_UNORM);
}

TFLOAT TD3DAdapter::Mode::GetRefreshRate() const
{
	return (TFLOAT)m_Description.RefreshRate.Numerator / m_Description.RefreshRate.Denominator;
}

TRenderAdapter::Mode::Device* TD3DAdapter::Mode::GetDevice(TINT device)
{
	TASSERT(device >= 0 && device < NUMSUPPORTEDDEVICES);
	return &m_Devices[device];
}

void TD3DAdapter::Mode::GetDisplayMode(IDXGIOutput* dxgiOutput, DXGI_MODE_DESC* modeDesc)
{
	DXGI_OUTPUT_DESC outputDesc;
	dxgiOutput->GetDesc(&outputDesc);

	MONITORINFOEXA monitorInfo;
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfoA(outputDesc.Monitor, &monitorInfo);

	DEVMODEA displaySettings = {};
	displaySettings.dmSize   = sizeof(displaySettings);
	EnumDisplaySettingsA(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &displaySettings);

	TBOOL defaultRefreshRate = TFALSE;

	if (displaySettings.dmDisplayFrequency == 1 || displaySettings.dmDisplayFrequency == 0)
	{
		defaultRefreshRate                 = TTRUE;
		displaySettings.dmDisplayFrequency = 0;
	}

	DXGI_MODE_DESC matchMode;
	matchMode.Width                   = displaySettings.dmPelsWidth;
	matchMode.Height                  = displaySettings.dmPelsHeight;
	matchMode.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	matchMode.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	matchMode.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	matchMode.RefreshRate.Denominator = (defaultRefreshRate == TTRUE) ? 0 : 1;
	matchMode.RefreshRate.Numerator   = displaySettings.dmDisplayFrequency;

	dxgiOutput->FindClosestMatchingMode(&matchMode, modeDesc, NULL);
}

TRenderDX11::TRenderDX11()
    : m_DisplayParams{ 1280, 720, 32, 3, TTRUE, TFALSE, 1 }, m_Window(), m_CurrentDepth({ 0 }, 0)
{
	m_ClearColor[0]  = 0.0f;
	m_ClearColor[1]  = 0.0f;
	m_ClearColor[2]  = 0.0f;
	m_ClearColor[3]  = 1.0f;
	m_NumDrawnFrames = 0;

	m_CurrentRasterizerId.Flags.Raw            = 0;
	m_CurrentRasterizerId.DepthBias            = 0;
	m_CurrentRasterizerId.SlopeScaledDepthBias = 0.0f;

	m_PreviousRasterizerId.Flags.Raw            = 0;
	m_PreviousRasterizerId.DepthBias            = 0;
	m_PreviousRasterizerId.SlopeScaledDepthBias = 0.0f;

	TUtil::MemClear(m_SamplerStates, sizeof(m_SamplerStates));

	m_pVertexConstantBuffer     = TNULL;
	m_IsVertexConstantBufferSet = TFALSE;
	TUtil::MemClear(m_PixelBuffers, sizeof(m_PixelBuffers));

	m_pPixelConstantBuffer     = TNULL;
	m_IsPixelConstantBufferSet = TFALSE;
	TUtil::MemClear(m_VertexBuffers, sizeof(m_VertexBuffers));

	m_MainVertexBuffer              = TNULL;
	m_iImmediateVertexCurrentOffset = 0;
	m_MainIndexBuffer               = TNULL;
	m_iImmediateIndexCurrentOffset  = 0;

	s_pMemHeap = TMemory::CreateHeap(HEAPSIZE, 0, "render states");

	TMaterialManager::CreateSingleton();

	m_CurrentBlendFactor[0] = 1.0f;
	m_CurrentBlendFactor[1] = 1.0f;
	m_CurrentBlendFactor[2] = 1.0f;
	m_CurrentBlendFactor[3] = 1.0f;

	m_PreviousBlendFactor[0] = 1.0f;
	m_PreviousBlendFactor[1] = 1.0f;
	m_PreviousBlendFactor[2] = 1.0f;
	m_PreviousBlendFactor[3] = 1.0f;

	TTODO("Some other initializations");
}

TBOOL TRenderDX11::CreateDisplay(DisplayParams* pDisplayParams)
{
	TASSERT(IsCreated() == TTRUE);
	TASSERT(IsDisplayCreated() == TFALSE);

	if (TRender::CreateDisplay() == TFALSE)
	{
		ShowDisplayError();
		return TFALSE;
	}

	m_DisplayParams = *pDisplayParams;

	auto adapterList = GetAdapterList();
	auto adapter     = adapterList->Head()->As<TD3DAdapter>();
	auto mode        = adapter->GetMode();

	if (!m_DisplayParams.Unk5 && !m_DisplayParams.IsFullscreen)
	{
		m_DisplayParams.Width  = mode->GetWidth();
		m_DisplayParams.Height = mode->GetHeight();
	}

	IDXGIDevice*   pDevice;
	IDXGIAdapter*  pAdapter;
	IDXGIFactory1* pFactory1;
	m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDevice);
	pDevice->GetAdapter(&pAdapter);
	pAdapter->GetParent(__uuidof(IDXGIFactory1), (void**)&pFactory1);

	DisplayParams* pCurrDisplayParams = GetCurrentDisplayParams();

	DXGI_MODE_DESC foundMode;
	if (pCurrDisplayParams->Unk5 == TFALSE)
	{
		IDXGIOutput* pOutput;
		pAdapter->EnumOutputs(0, &pOutput);

		TD3DAdapter::Mode::GetDisplayMode(pOutput, &foundMode);
		m_DisplayParams.Width  = foundMode.Width;
		m_DisplayParams.Height = foundMode.Height;
	}

	pAdapter->Release();
	pDevice->Release();

	Toshi::TUtil::MemClear(&m_SwapChainDesc, sizeof(m_SwapChainDesc));

	DXGI_MODE_DESC* pSelectedMode;

	if (m_DisplayParams.IsFullscreen == TFALSE)
	{
		pSelectedMode    = mode->GetDescription();
		foundMode.Format = pSelectedMode->Format;
	}
	else
	{
		pSelectedMode = &foundMode;
	}

	m_SwapChainDesc.BufferCount            = 1;
	m_SwapChainDesc.BufferDesc.Width       = m_DisplayParams.Width;
	m_SwapChainDesc.BufferDesc.Height      = m_DisplayParams.Height;
	m_SwapChainDesc.BufferDesc.Format      = pSelectedMode->Format;
	m_SwapChainDesc.BufferDesc.RefreshRate = pSelectedMode->RefreshRate;
	m_SwapChainDesc.OutputWindow           = m_Window.GetHWND();
	m_SwapChainDesc.BufferUsage            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_SwapChainDesc.SampleDesc.Count       = 1;
	m_SwapChainDesc.SampleDesc.Quality     = 0;
	m_SwapChainDesc.Windowed               = TRUE;
	m_SwapChainDesc.SwapEffect             = DXGI_SWAP_EFFECT_DISCARD;
	m_SwapChainDesc.Flags                  = m_DisplayParams.IsFullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

	pFactory1->CreateSwapChain(m_pDevice, &m_SwapChainDesc, &m_SwapChain);
	pFactory1->MakeWindowAssociation(m_Window.GetHWND(), DXGI_MWA_NO_ALT_ENTER);
	pFactory1->Release();

	if (m_pDeviceContext == TNULL || m_pDevice == TNULL || m_SwapChain == TNULL)
	{
		ShowDeviceError();
	}

	UINT   refreshRateNum   = m_SwapChainDesc.BufferDesc.RefreshRate.Numerator;
	UINT   refreshRateDenom = m_SwapChainDesc.BufferDesc.RefreshRate.Denominator;
	double refreshRate      = (double)refreshRateNum / refreshRateDenom;
	TCHAR  displayName[TD3DAdapter::Mode::DISPLAYNAMESIZE + 1];
	TStringManager::StringUnicodeToChar(displayName, mode->GetDisplayName(), -1);
	TUtil::Log("CurrentDisplay: %s", displayName);
	pSelectedMode = mode->GetDescription();
	TUtil::Log("CurrentDisplayMode = %d x %d (%.4f Hz) = [%d / %d]", pSelectedMode->Width, pSelectedMode->Height, refreshRate, refreshRateNum, refreshRateDenom);

	if (m_DisplayParams.IsFullscreen)
	{
		TUtil::Log("Set Fullscreen %d x %d (:.4f Hz) = [%d / %d]", m_DisplayParams.Width, m_DisplayParams.Height, refreshRate, refreshRateNum, refreshRateDenom);
		m_Window.SetFullscreen();
	}
	else
	{
		TUtil::Log("Set Windowed %d x %d", m_DisplayParams.Width, m_DisplayParams.Height);
		m_Window.SetWindowed();
	}

	UINT centerX = 0;
	UINT centerY = 0;

	if (!m_DisplayParams.IsFullscreen && !m_DisplayParams.Unk5)
	{
		LONG style = GetWindowLongA(m_Window.GetHWND(), GWL_STYLE);
		SetWindowLongA(m_Window.GetHWND(), GWL_STYLE, style & (WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_DISABLED | WS_GROUP | WS_HSCROLL | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_POPUP | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL));
		SetWindowPos(m_Window.GetHWND(), HWND_TOP, 0, 0, m_DisplayParams.Width, m_DisplayParams.Height, 0);
	}
	else
	{
		centerX = (mode->GetWidth() - m_DisplayParams.Width) / 2;
		centerY = (mode->GetHeight() - m_DisplayParams.Height) / 2;
		m_Window.SetPosition(centerX, centerY, m_DisplayParams.Width, m_DisplayParams.Height);
	}

	m_DisplayHeight = m_DisplayParams.Height;
	m_DisplayWidth  = m_DisplayParams.Width;
	m_IsWidescreen  = TTRUE;

	// expectedWidth is the width expected for 16:9 screen
	UINT expectedWidth = (m_DisplayHeight / 9) * 16;

	if (expectedWidth != m_DisplayWidth)
	{
		// The aspect ratio is not 16:9
		if (expectedWidth < m_DisplayWidth)
		{
			m_DisplayParams.Width            = expectedWidth;
			m_SwapChainDesc.BufferDesc.Width = expectedWidth;
		}
		else
		{
			UINT expectedHeight               = (m_DisplayWidth / 16) * 9;
			m_DisplayParams.Height            = expectedHeight;
			m_SwapChainDesc.BufferDesc.Height = expectedHeight;
		}
	}

	if (s_QualityLevel == 1)
	{
		UINT width  = m_DisplayParams.Width;
		UINT height = m_DisplayParams.Height;

		if (width > 1920)
		{
			width  = 1920;
			height = 1080;
		}

		if (width < 1280)
		{
			width = 1280;
		}

		if (height < 720)
		{
			height = 720;
		}

		m_DisplayParams.Height = height;
		m_DisplayParams.Width  = width;
	}
	else if (s_QualityLevel > 0)
	{
		m_DisplayParams.Width  = 1280;
		m_DisplayParams.Height = 720;
	}

	m_DisplayParams.Width  = TMath::Max<TUINT32>(m_DisplayParams.Width, 1280);
	m_DisplayParams.Height = TMath::Max<TUINT32>(m_DisplayParams.Height, 720);

	m_SwapChainDesc.BufferDesc.Width  = m_DisplayParams.Width;
	m_SwapChainDesc.BufferDesc.Height = m_DisplayParams.Height;
	TUINT32& qualityLevel             = m_DisplayParams.MultisampleQualityLevel;

	if (qualityLevel < 2)
	{
		qualityLevel = 1;
	}
	else
	{
		if (qualityLevel != 2 && qualityLevel != 4 && qualityLevel != 8)
		{
			qualityLevel = 4;
		}

		TBOOL isMultisampleSet = TFALSE;
		for (UINT level = qualityLevel; 1 < qualityLevel; qualityLevel /= 2)
		{
			UINT    numQualityLevels;
			HRESULT hRes = m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, level, &numQualityLevels);

			if (hRes != S_OK && numQualityLevels != 0)
			{
				m_DisplayParams.MultisampleQualityLevel = level;
				isMultisampleSet                        = TTRUE;
				break;
			}
		}

		if (!isMultisampleSet)
		{
			// Default value
			m_DisplayParams.MultisampleQualityLevel = 1;
		}
	}

	HRESULT hRes1 = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_Texture2D1);

	if (SUCCEEDED(hRes1))
	{
		HRESULT hRes2 = m_pDevice->CreateRenderTargetView(m_Texture2D1, NULL, &m_RTView1);

		if (SUCCEEDED(hRes2))
		{
			TUtil::Log("Creating Main RT %d x %d", m_DisplayParams.Width, m_DisplayParams.Height);

			{
				// Texture 1
				ID3D11Resource* pResource;
				m_SRView1 = CreateTexture(m_DisplayParams.Width, m_DisplayParams.Height, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, 4, D3D11_USAGE_DEFAULT, 0, m_DisplayParams.MultisampleQualityLevel);
				m_SRView1->GetResource(&pResource);

				if (pResource != TNULL)
				{
					pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&m_SRView1Texture);
					pResource->Release();
				}
			}

			{
				// Texture 2
				ID3D11Resource* pResource;
				m_SRView2 = CreateTexture(m_DisplayParams.Width, m_DisplayParams.Height, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, 4, D3D11_USAGE_DEFAULT, 0, 1);
				m_SRView2->GetResource(&pResource);

				if (pResource != TNULL)
				{
					pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&m_SRView2Texture);
					pResource->Release();
				}
			}

			m_RTView2 = CreateRenderTargetView(m_SRView1);
			m_pDeviceContext->ClearRenderTargetView(m_RTView1, m_ClearColor);
			m_pDeviceContext->ClearRenderTargetView(m_RTView2, m_ClearColor);

			// Create depth stencil view
			D3D11_TEXTURE2D_DESC texDesc = {};
			texDesc.Width                = m_DisplayParams.Width;
			texDesc.Height               = m_DisplayParams.Height;
			texDesc.SampleDesc.Count     = m_DisplayParams.MultisampleQualityLevel;
			texDesc.SampleDesc.Quality   = 0;
			texDesc.MipLevels            = 1;
			texDesc.ArraySize            = 1;
			texDesc.Format               = DXGI_FORMAT_R24G8_TYPELESS;
			texDesc.Usage                = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags       = 0;
			texDesc.MiscFlags            = 0;

			ID3D11Texture2D* pDepthStencilTex;
			HRESULT          hRes = m_pDevice->CreateTexture2D(&texDesc, 0, &pDepthStencilTex);

			if (SUCCEEDED(hRes))
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
				depthStencilViewDesc.Format                        = DXGI_FORMAT_D24_UNORM_S8_UINT;
				depthStencilViewDesc.Flags                         = 0;
				depthStencilViewDesc.Texture2D.MipSlice            = 0;
				depthStencilViewDesc.ViewDimension                 = m_DisplayParams.MultisampleQualityLevel > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

				m_pDevice->CreateDepthStencilView(pDepthStencilTex, &depthStencilViewDesc, &m_StencilView);

				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
				shaderResourceViewDesc.Format                          = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
				shaderResourceViewDesc.ViewDimension                   = m_DisplayParams.MultisampleQualityLevel > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResourceViewDesc.Texture2D.MostDetailedMip       = 0;
				shaderResourceViewDesc.Texture2D.MipLevels             = 1;

				m_pDevice->CreateShaderResourceView(pDepthStencilTex, &shaderResourceViewDesc, &m_StencilTexSR);
				pDepthStencilTex->Release();
			}
		}
	}

	SetCursorPos(centerX + m_DisplayParams.Width / 2, centerY + m_DisplayParams.Height / 2);

	if (m_DisplayParams.Unk5 == TFALSE)
	{
		ShowCursor(TFALSE);
	}

	Initialize();

	TRenderContext* pContext = GetCurrentRenderContext();
	pContext->SetParams({
	    0.0f,                                      // fX
	    0.0f,                                      // fY
	    (TFLOAT)m_SwapChainDesc.BufferDesc.Width,  // fWidth
	    (TFLOAT)m_SwapChainDesc.BufferDesc.Height, // fHeight
	    0.0f,                                      // fMinZ
	    1.0f                                       // fMaxZ
	});

	TTextureManager::CreateSingleton();

	m_pPostProcess = new TPostProcess();
	m_pToneMap     = new TToneMap();
	m_pFXAA        = new TFXAA();
	m_pGlow        = new TGlow();

	CreateVSPS();

	return TTRUE;
}

void TRenderDX11::Update(TFLOAT deltaTime)
{
	TASSERT(TTRUE == IsCreated());
	m_Window.Update();
	TRender::Update(deltaTime);
}

void TRenderDX11::BeginScene()
{
	TASSERT(TTRUE == IsDisplayCreated());
	TASSERT(TFALSE == IsInScene());

	TRender::BeginScene();
	m_pDeviceContext->OMSetRenderTargets(1, &m_RTView2, m_StencilView);
	m_pDeviceContext->ClearRenderTargetView(m_RTView2, m_ClearColor);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.Width    = (TFLOAT)m_SwapChainDesc.BufferDesc.Width;
	viewport.Height   = (TFLOAT)m_SwapChainDesc.BufferDesc.Height;

	m_pDeviceContext->RSSetViewports(1, &viewport);
	m_bInScene = TTRUE;
}

void TRenderDX11::EndScene()
{
	TASSERT(TTRUE == IsDisplayCreated());
	TASSERT(TTRUE == IsInScene());

	m_NumDrawnFrames += 1;

	if (m_IsWidescreen == TFALSE)
	{
		if (m_DisplayParams.MultisampleQualityLevel < 2)
		{
			m_pDeviceContext->CopyResource(m_Texture2D1, m_SRView1Texture);
		}
		else
		{
			m_pDeviceContext->ResolveSubresource(m_Texture2D1, 0, m_SRView1Texture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		}
	}
	else
	{
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.Width    = (TFLOAT)m_DisplayWidth;
		viewport.Height   = (TFLOAT)m_DisplayHeight;

		m_pDeviceContext->OMSetRenderTargets(1, &m_RTView1, NULL);
		m_pDeviceContext->RSSetViewports(1, &viewport);
		m_pDeviceContext->ClearRenderTargetView(m_RTView1, m_ClearColor);

		UINT diff1 = m_DisplayWidth - m_DisplayParams.Width;
		UINT diff2 = m_DisplayHeight - m_DisplayParams.Height;

		if (m_DisplayParams.MultisampleQualityLevel < 2)
		{
			if (s_pShaderResourceView == TNULL)
			{
				m_pDevice->CreateShaderResourceView(m_SRView1Texture, 0, &s_pShaderResourceView);
			}

			TFLOAT inAR  = (TFLOAT)m_DisplayParams.Width / m_DisplayParams.Height;
			TFLOAT outAR = (TFLOAT)m_DisplayWidth / m_DisplayHeight;
			TASSERT(fabsf(inAR - 16.0f / 9.0f) < 0.01);

			TFLOAT posX, posY, width, height;

			if (outAR < inAR)
			{
				width  = (TFLOAT)m_DisplayWidth;
				height = width / inAR;
				posX   = 0.0F;
				posY   = (m_DisplayHeight - height) * 0.5F;
			}
			else
			{
				height = (TFLOAT)m_DisplayHeight;
				width  = height * inAR;
				posX   = (m_DisplayWidth - width) * 0.5F;
				posY   = 0.0F;
			}

			m_CurrentBlendState.Parts.bBlendEnabled = FALSE;
			RenderOverlay(posX, posY, width, height, s_pShaderResourceView, NULL, NULL);
		}
		else
		{
			m_pDeviceContext->ResolveSubresource(m_SRView2Texture, 0, m_SRView1Texture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
			m_pDeviceContext->CopySubresourceRegion(m_Texture2D1, NULL, diff1 / 2, diff2 / 2, 0, m_SRView2Texture, 0, NULL);
		}

		ID3D11ShaderResourceView* pRV = TNULL;
		m_pDeviceContext->PSGetShaderResources(0, 1, &pRV);
	}

	HRESULT hRes = m_SwapChain->Present(1, s_bPresentTest ? DXGI_PRESENT_TEST : 0);

	if (!SUCCEEDED(hRes))
	{
		TOSHI_CORE_ERROR("Present Failed !!!! {0:x}", hRes);

		if (hRes == DXGI_ERROR_DEVICE_REMOVED)
		{
			TTODO("FUN_006a7a30");
		}
		else if (hRes == DXGI_ERROR_DEVICE_RESET)
		{
			TASSERT(TFALSE);
		}
		else if (hRes == DXGI_STATUS_OCCLUDED)
		{
			s_bPresentTest = TTRUE;
			TOSHI_CORE_ERROR("Pausing Occluded!");
			TSystemManager::GetSingleton()->Pause(TTRUE);
		}
	}

	m_bInScene = TFALSE;
	TRender::EndScene();
}

TBOOL TRenderDX11::RecreateDisplay(DisplayParams* pDisplayParams)
{
	TASSERT(TTRUE == IsCreated());
	TASSERT(TTRUE == IsDisplayCreated());

	TTODO("FUN_006a7a30");

	DestroyDisplay();
	TBOOL bRes = CreateDisplay(pDisplayParams);
	TASSERT(bRes);

	return TTRUE == bRes;
}

void TRenderDX11::ShowDeviceError()
{
	CHAR caption[1024] = { 0 };
	CHAR text[1024]    = { 0 };

	GetPrivateProfileStringA("Setup", "IDS_D3DDEVICEERRORTITLE", "Initialization failure", caption, sizeof(caption), ".\\Setup.ini");
	GetPrivateProfileStringA("Setup", "IDS_D3DDEVICEERROR", "Failed to create D3D11 device", caption, sizeof(caption), ".\\Setup.ini");

	MessageBoxA(NULL, text, caption, MB_OK);
}

void TRenderDX11::ShowDisplayError()
{
	CHAR caption[1024] = { 0 };
	CHAR text[1024]    = { 0 };

	GetPrivateProfileStringA("Setup", "IDS_D3DDISPLAYERRORTITLE", "Initialization failure", caption, sizeof(caption), ".\\Setup.ini");
	GetPrivateProfileStringA("Setup", "IDS_D3DDISPLAYERROR", "Failed to create the display. Please run the Barnyard setup program and reconfigure", text, sizeof(text), ".\\Setup.ini");

	MessageBoxA(NULL, text, caption, MB_OK);
}

void TRenderDX11::CreateVSPS()
{
	ID3DBlob* shaderVS = CompileShader(s_defaultVertexShader, "main", "vs_4_0_level_9_3", NULL);
	CreateVertexShader(shaderVS->GetBufferPointer(), shaderVS->GetBufferSize(), &m_pVertexShader);

	ID3DBlob* shader = CompileShader(s_defaultPixelShader, "main", "ps_4_0_level_9_3", NULL);
	CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), &m_pPixelShader1);
	shader->Release();

	shader = CompileShader(s_defaultPixelShader2, "main", "ps_4_0_level_9_3", NULL);
	CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), &m_pPixelShader2);
	shader->Release();

	D3D11_INPUT_ELEMENT_DESC inputDesc[2];

	inputDesc[0].SemanticName         = "POSITION";
	inputDesc[0].SemanticIndex        = 0;
	inputDesc[0].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
	inputDesc[0].InputSlot            = 0;
	inputDesc[0].AlignedByteOffset    = -1;
	inputDesc[0].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	inputDesc[0].InstanceDataStepRate = 0;

	inputDesc[1].SemanticName         = "TEXCOORD";
	inputDesc[1].SemanticIndex        = 0;
	inputDesc[1].Format               = DXGI_FORMAT_R32G32_FLOAT;
	inputDesc[1].InputSlot            = 0;
	inputDesc[1].AlignedByteOffset    = -1;
	inputDesc[1].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	inputDesc[1].InstanceDataStepRate = 0;

	m_pDevice->CreateInputLayout(inputDesc, 2, shaderVS->GetBufferPointer(), shaderVS->GetBufferSize(), &m_pInputLayout);
	shaderVS->Release();

	D3D11_SUBRESOURCE_DATA vertexData;

	FLOAT vertices[20] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f };

	vertexData.pSysMem          = vertices;
	vertexData.SysMemPitch      = 0;
	vertexData.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth           = sizeof(vertices);
	bufferDesc.Usage               = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags      = 0;
	bufferDesc.MiscFlags           = 0;
	bufferDesc.StructureByteStride = 0;

	m_pDevice->CreateBuffer(&bufferDesc, &vertexData, &m_pQuarterScreenQuadBuffer);
}

TBOOL TRenderDX11::Create(LPCSTR a_name)
{
	// 006a5e30
	TASSERT(TFALSE == IsCreated());
	TBOOL bResult = TRender::Create();

	if (bResult)
	{
		TUtil::Log("Creating TRenderDX11");
		TUtil::LogUp();

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_3;

		for (size_t i = 0; i < sizeof(m_scpDriverTypes) / sizeof(*m_scpDriverTypes); i++)
		{
			if (D3D11CreateDevice(NULL, m_scpDriverTypes[i], NULL, 0, m_scpFeatureLevels, 4, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext) == S_OK) break;
			if (D3D11CreateDevice(NULL, m_scpDriverTypes[i], NULL, 0, &m_scpFeatureLevels[1], 3, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext) == S_OK) break;
		}

		TUtil::Log("FeatureLevel : %s", GetFeatureLevel(featureLevel));

		BuildAdapterDatabase();

		if (m_hAccel != NULL)
		{
			DestroyAcceleratorTable(m_hAccel);
			m_hAccel = NULL;
		}

		ACCEL accels[2];
		accels[0].fVirt = FVIRTKEY;
		accels[0].key   = VK_ESCAPE;

		accels[1].fVirt = FALT;
		accels[1].key   = VK_RETURN;
		accels[1].cmd   = 1;

		m_hAccel = CreateAcceleratorTableA(accels, sizeof(accels) / sizeof(*accels));

		if (m_Window.Create(this, a_name))
		{
			m_eAspectRatio = ASPECT_RATIO_16_9;
			return TTRUE;
		}

		TUtil::Log("Failed to create Window");
	}

	return TFALSE;
}

void TRenderDX11::Initialize()
{
	m_SamplerStates[0]  = CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX, 1);
	m_SamplerStates[1]  = CreateSamplerStateAutoAnisotropy(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX);
	m_SamplerStates[2]  = CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX, 1);
	m_SamplerStates[3]  = CreateSamplerStateAutoAnisotropy(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX);
	m_SamplerStates[4]  = CreateSamplerStateAutoAnisotropy(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_MIRROR, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX);
	m_SamplerStates[5]  = CreateSamplerState(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX, 1);
	m_SamplerStates[6]  = CreateSamplerState(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX, 1);
	m_SamplerStates[7]  = CreateSamplerState(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, -1.0f, 0, 0.0f, D3D11_FLOAT32_MAX, 1);
	m_SamplerStates[9]  = CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX, 1);
	m_SamplerStates[10] = CreateSamplerStateAutoAnisotropy(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX);
	m_SamplerStates[11] = CreateSamplerState(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX, 1);
	m_SamplerStates[8]  = CreateSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f, 0, 0.0f, D3D11_FLOAT32_MAX, 1);

	for (size_t i = 0; i < NUMBUFFERS; i++)
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth           = VERTEX_CONSTANT_BUFFER_SIZE;
		bufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags           = 0;
		bufferDesc.StructureByteStride = 0;

		m_pDevice->CreateBuffer(&bufferDesc, NULL, &m_VertexBuffers[i]);
	}

	m_pVertexConstantBuffer     = s_pMemHeap->Malloc(VERTEX_CONSTANT_BUFFER_SIZE);
	m_IsVertexConstantBufferSet = TFALSE;
	m_VertexBufferIndex         = 0;

	for (size_t i = 0; i < NUMBUFFERS; i++)
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth           = PIXEL_CONSTANT_BUFFER_SIZE;
		bufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags           = 0;
		bufferDesc.StructureByteStride = 0;

		m_pDevice->CreateBuffer(&bufferDesc, NULL, &m_PixelBuffers[i]);
	}

	m_pPixelConstantBuffer     = s_pMemHeap->Malloc(PIXEL_CONSTANT_BUFFER_SIZE);
	m_IsPixelConstantBufferSet = TFALSE;
	m_PixelBufferIndex         = 0;

	// Main vertex buffer
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth           = IMMEDIATE_VERTEX_BUFFER_SIZE;
		bufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags           = 0;
		bufferDesc.StructureByteStride = 0;

		m_pDevice->CreateBuffer(&bufferDesc, NULL, &m_MainVertexBuffer);
		m_iImmediateVertexCurrentOffset = 0;
	}

	// Main index buffer
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth           = IMMEDIATE_INDEX_BUFFER_SIZE;
		bufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags           = 0;
		bufferDesc.StructureByteStride = 0;

		m_pDevice->CreateBuffer(&bufferDesc, NULL, &m_MainIndexBuffer);
		m_iImmediateIndexCurrentOffset = 0;
	}

	m_CurrentDepth.first.Parts.bDepthEnable                = TRUE;
	m_CurrentDepth.first.Parts.DepthWriteMask              = D3D11_DEPTH_WRITE_MASK_ALL;
	m_CurrentDepth.first.Parts.DepthFunc                   = D3D11_COMPARISON_LESS;
	m_CurrentDepth.first.Parts.bStencilEnable              = FALSE;
	m_CurrentDepth.first.Parts.StencilReadMask             = 0b11111111;
	m_CurrentDepth.first.Parts.StencilWriteMask            = 0b11111111;
	m_CurrentDepth.first.Parts.FrontFaceStencilFailOp      = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.first.Parts.FrontFaceStencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.first.Parts.FrontStencilPassOp          = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.first.Parts.FrontStencilFunc            = D3D11_COMPARISON_ALWAYS;
	m_CurrentDepth.first.Parts.BackFaceStencilFailOp       = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.first.Parts.BackFaceStencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.first.Parts.BackStencilPassOp           = D3D11_STENCIL_OP_KEEP;
	m_CurrentDepth.first.Parts.BackStencilFunc             = D3D11_COMPARISON_ALWAYS;

	m_CurrentBlendState.Parts.BlendOp               = D3D11_BLEND_OP_ADD;
	m_CurrentBlendState.Parts.BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	m_CurrentBlendState.Parts.SrcBlendAlpha         = D3D11_BLEND_ONE;
	m_CurrentBlendState.Parts.DestBlendAlpha        = D3D11_BLEND_ZERO;
	m_CurrentBlendState.Parts.RenderTargetWriteMask = 0b1111;
	m_CurrentBlendState.Parts.SrcBlend              = D3D11_BLEND_ONE;
	m_CurrentBlendState.Parts.DestBlend             = D3D11_BLEND_ZERO;
	m_CurrentBlendState.Parts.Unknown2              = 1;

	m_CurrentRasterizerId.Flags.Parts.FillMode               = D3D11_FILL_SOLID;
	m_CurrentRasterizerId.Flags.Parts.CullMode               = D3D11_CULL_BACK;
	m_CurrentRasterizerId.Flags.Parts.bFrontCounterClockwise = FALSE;
	m_CurrentRasterizerId.Flags.Parts.bDepthClipEnable       = TRUE;
	m_CurrentRasterizerId.Flags.Parts.bScissorEnable         = FALSE;
	m_CurrentRasterizerId.Flags.Parts.bMultisampleEnable     = FALSE;
	m_CurrentRasterizerId.DepthBias                          = 0;
	m_CurrentRasterizerId.SlopeScaledDepthBias               = 0.0f;
}

TBOOL TRenderDX11::IsColorEqual(const FLOAT a_Vec41[4], const FLOAT a_Vec42[4])
{
	return (a_Vec41[0] == a_Vec42[0]) && (a_Vec41[1] == a_Vec42[1]) && (a_Vec41[2] == a_Vec42[2]) && (a_Vec41[3] == a_Vec42[3]);
}

TINT TRenderDX11::GetTextureRowPitch(DXGI_FORMAT format, TINT width)
{
	switch (format)
	{
		case DXGI_FORMAT_UNKNOWN: return 0;
		case DXGI_FORMAT_R32G32B32A32_FLOAT: return width << 4;
		case DXGI_FORMAT_R16G16B16A16_FLOAT: return width << 3;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_B8G8R8A8_UNORM: return width << 2;
		case DXGI_FORMAT_A8_UNORM: return width;
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC4_UNORM: return ((width + 3U) >> 2) << 3;
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC5_UNORM: return ((width + 3U) >> 2) << 4;
		case DXGI_FORMAT_B8G8R8X8_UNORM: return width * 3;
	}

	TASSERT(TFALSE);
	return 0;
}

TINT TRenderDX11::GetTextureDepthPitch(DXGI_FORMAT format, TINT width, TINT height)
{
	switch (format)
	{
		case DXGI_FORMAT_UNKNOWN: return 0;
		case DXGI_FORMAT_R32G32B32A32_FLOAT: return width * height * 16;
		case DXGI_FORMAT_R16G16B16A16_FLOAT: return width * height * 8;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_B8G8R8A8_UNORM: return width * height * 4;
		case DXGI_FORMAT_A8_UNORM: return width * height;
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC4_UNORM: return ((width + 3U) >> 2) * ((height + 3U) >> 2) * 8;
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC5_UNORM: return ((width + 3U) >> 2) * ((height + 3U) >> 2) * 16;
		case DXGI_FORMAT_B8G8R8X8_UNORM: return width * height * 3;
	}

	TASSERT(TFALSE);
	return 0;
}

const TCHAR* TRenderDX11::GetFeatureLevel(D3D_FEATURE_LEVEL a_featureLevel)
{
	// 006a9470
	switch (a_featureLevel)
	{
		case D3D_FEATURE_LEVEL_9_1: return "D3D_FEATURE_LEVEL_9_1";
		case D3D_FEATURE_LEVEL_9_2: return "D3D_FEATURE_LEVEL_9_2";
		case D3D_FEATURE_LEVEL_9_3: return "D3D_FEATURE_LEVEL_9_3";
		case D3D_FEATURE_LEVEL_10_0: return "D3D_FEATURE_LEVEL_10_0";
		case D3D_FEATURE_LEVEL_10_1: return "D3D_FEATURE_LEVEL_10_1";
		case D3D_FEATURE_LEVEL_11_0: return "D3D_FEATURE_LEVEL_11_0";
		case D3D_FEATURE_LEVEL_11_1: return "D3D_FEATURE_LEVEL_11_1";
		default: return "unknown";
	}
}

ID3DBlob* TRenderDX11::CompileShader(const TCHAR* srcData, LPCSTR pEntrypoint, LPCSTR pTarget, const D3D_SHADER_MACRO* pDefines)
{
	size_t srcLength = T2String8::Length(srcData);

	ID3DBlob* pShaderBlob = TNULL;
	ID3DBlob* pErrorBlob  = TNULL;

	HRESULT hRes = D3DCompile(srcData, srcLength, NULL, pDefines, NULL, pEntrypoint, pTarget, 0x1000, 0, &pShaderBlob, &pErrorBlob);

	if (!SUCCEEDED(hRes))
	{
		TOSHI_CORE_CRITICAL("Shader compilation failed");

		if (pErrorBlob != TNULL)
		{
			TOSHI_CORE_CRITICAL((const TCHAR*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}

		TASSERT(TFALSE);
	}

	return pShaderBlob;
}

ID3DBlob* TRenderDX11::CompileShaderFromFile(const TCHAR* filepath, LPCSTR pEntrypoint, LPCSTR pTarget, const D3D_SHADER_MACRO* pDefines)
{
	TFile* pFile    = TFile::Create(filepath);
	DWORD  fileSize = pFile->GetSize();
	TCHAR* srcData  = new TCHAR[fileSize + 1];
	pFile->Read(srcData, fileSize);
	srcData[fileSize] = '\0';
	pFile->Destroy();

	ID3DBlob* shader = CompileShader(srcData, pEntrypoint, pTarget, pDefines);
	delete[] srcData;

	return shader;
}

void TRenderDX11::SetVec4InVSBuffer(BufferOffset index, const void* src, TINT count)
{
	TUINT offset = index * 16;
	TUINT size   = count * 16;

	TASSERT(offset + size <= VERTEX_CONSTANT_BUFFER_SIZE, "Buffer size exceeded");
	TUtil::MemCopy((TCHAR*)m_pVertexConstantBuffer + offset, src, size);
	m_IsVertexConstantBufferSet = TTRUE;
}

void TRenderDX11::SetVec4InPSBuffer(BufferOffset index, const void* src, TINT count)
{
	TUINT offset = index * 16;
	TUINT size   = count * 16;

	TASSERT(offset + size <= PIXEL_CONSTANT_BUFFER_SIZE, "Buffer size exceeded");
	TUtil::MemCopy((TCHAR*)m_pPixelConstantBuffer + offset, src, size);
	m_IsPixelConstantBufferSet = TTRUE;
}

HRESULT TRenderDX11::CreatePixelShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11PixelShader** ppPixelShader)
{
	HRESULT hRes = m_pDevice->CreatePixelShader(pShaderBytecode, BytecodeLength, NULL, ppPixelShader);
	TASSERT(SUCCEEDED(hRes), "Couldnt Create Pixel Shader");

	return hRes;
}

HRESULT TRenderDX11::CreateVertexShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11VertexShader** ppVertexShader)
{
	HRESULT hRes = m_pDevice->CreateVertexShader(pShaderBytecode, BytecodeLength, NULL, ppVertexShader);
	TASSERT(SUCCEEDED(hRes), "Couldnt Create Pixel Shader");

	return hRes;
}

ID3D11ShaderResourceView* TRenderDX11::CreateTexture(UINT width, UINT height, DXGI_FORMAT format, const void* srcData, TUINT8 flags, D3D11_USAGE usage, TUINT32 cpuAccessFlags, TUINT32 sampleDescCount)
{
	D3D11_SUBRESOURCE_DATA subResourceData = {};
	D3D11_TEXTURE2D_DESC   textureDesc     = {};

	UINT  numberOfMipmaps = 0;
	TBOOL noMipLevels;
	TBOOL doScaryThings = TTRUE;

	if (flags & 1 || (flags & 2) == 0)
	{
		noMipLevels = TFALSE;

		if ((flags & 1) == 0)
		{
			numberOfMipmaps = 1;
			doScaryThings   = TFALSE;
		}
	}
	else
	{
		noMipLevels = TTRUE;
	}

	if (doScaryThings)
	{
		TASSERT(TFALSE, "Not implemented");
	}

	numberOfMipmaps = noMipLevels ? 0 : numberOfMipmaps;

	textureDesc.SampleDesc.Count   = sampleDescCount;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.ArraySize          = 1;
	textureDesc.Usage              = usage;
	textureDesc.Width              = width;
	textureDesc.Height             = height;
	textureDesc.Format             = format;
	textureDesc.CPUAccessFlags     = cpuAccessFlags;
	textureDesc.MipLevels          = numberOfMipmaps;
	textureDesc.MiscFlags          = noMipLevels ? D3D11_RESOURCE_MISC_GENERATE_MIPS : NULL;
	textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

	if (flags & 4 || noMipLevels)
	{
		textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}

	ID3D11Texture2D* pTexture = TNULL;

	if (srcData == TNULL)
	{
		m_pDevice->CreateTexture2D(&textureDesc, TNULL, &pTexture);
	}
	else
	{
		if (noMipLevels == TFALSE)
		{
			TASSERT(numberOfMipmaps <= MAXIMUM_NUMBER_OF_MIPMAPS);

			if (!HASFLAG(flags & 1))
			{
				D3D11_SUBRESOURCE_DATA subresourceData;
				subresourceData.pSysMem          = srcData;
				subresourceData.SysMemPitch      = GetTextureRowPitch(format, width);
				subresourceData.SysMemSlicePitch = GetTextureDepthPitch(format, width, height);

				m_pDevice->CreateTexture2D(&textureDesc, &subresourceData, &pTexture);
			}
			else
			{
				TASSERT(TFALSE, "MipMap generator is not implemented");
			}
		}
		else
		{
			m_pDevice->CreateTexture2D(&textureDesc, 0, &pTexture);
			if (pTexture == TNULL) return TNULL;

			m_pDeviceContext->UpdateSubresource(pTexture, NULL, NULL, srcData, GetTextureRowPitch(format, width), GetTextureDepthPitch(format, width, height));
		}
	}

	if (pTexture)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		shaderResourceViewDesc.Format                    = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension             = sampleDescCount > 1 ? D3D_SRV_DIMENSION_TEXTURE2DMS : D3D_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels       = numberOfMipmaps;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

		ID3D11ShaderResourceView* pShaderResourceView = TNULL;
		m_pDevice->CreateShaderResourceView(pTexture, &shaderResourceViewDesc, &pShaderResourceView);

		if (pShaderResourceView != TNULL && noMipLevels == TFALSE)
		{
			m_pDeviceContext->GenerateMips(pShaderResourceView);
		}

		pTexture->Release();
		return pShaderResourceView;
	}

	return TNULL;
}

ID3D11RenderTargetView* TRenderDX11::CreateRenderTargetView(ID3D11ShaderResourceView* pShaderResourceView)
{
	ID3D11Texture2D* pTexture;
	pShaderResourceView->GetResource((ID3D11Resource**)&pTexture);

	D3D11_TEXTURE2D_DESC texDesc;
	pTexture->GetDesc(&texDesc);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format             = texDesc.Format;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	renderTargetViewDesc.ViewDimension      = texDesc.SampleDesc.Count > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;

	ID3D11RenderTargetView* pRenderTargetView = TNULL;
	m_pDevice->CreateRenderTargetView(pTexture, &renderTargetViewDesc, &pRenderTargetView);

	return pRenderTargetView;
}

ID3D11SamplerState* TRenderDX11::CreateSamplerStateAutoAnisotropy(
    D3D11_FILTER               filter,
    D3D11_TEXTURE_ADDRESS_MODE addressU,
    D3D11_TEXTURE_ADDRESS_MODE addressV,
    D3D11_TEXTURE_ADDRESS_MODE addressW,
    FLOAT                      mipLODBias,
    TUINT32                    borderColor,
    FLOAT                      minLOD,
    FLOAT                      maxLOD
)
{
	D3D11_SAMPLER_DESC samplerDesc = {};

	if (filter == D3D11_FILTER_MIN_MAG_MIP_LINEAR)
	{
		filter                    = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.MaxAnisotropy = 0x10;
	}
	else
	{
		samplerDesc.MaxAnisotropy = 1;
	}

	samplerDesc.AddressU       = addressU;
	samplerDesc.AddressV       = addressV;
	samplerDesc.MipLODBias     = mipLODBias;
	samplerDesc.AddressW       = addressW;
	samplerDesc.BorderColor[0] = (TFLOAT)((borderColor >> 24) & 0xFF) / 255.0f;
	samplerDesc.BorderColor[1] = (TFLOAT)((borderColor >> 16) & 0xFF) / 255.0f;
	samplerDesc.BorderColor[2] = (TFLOAT)((borderColor >> 8) & 0xFF) / 255.0f;
	samplerDesc.BorderColor[3] = (TFLOAT)((borderColor >> 0) & 0xFF) / 255.0f;
	samplerDesc.MinLOD         = minLOD;
	samplerDesc.MaxLOD         = maxLOD;
	samplerDesc.Filter         = filter;

	ID3D11SamplerState* pSamplerState;
	m_pDevice->CreateSamplerState(&samplerDesc, &pSamplerState);

	return pSamplerState;
}

ID3D11SamplerState* TRenderDX11::
    CreateSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU, D3D11_TEXTURE_ADDRESS_MODE addressV, D3D11_TEXTURE_ADDRESS_MODE addressW, FLOAT mipLODBias, TUINT32 borderColor, FLOAT minLOD, FLOAT maxLOD, UINT maxAnisotropy)
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU           = addressU;
	samplerDesc.AddressV           = addressV;
	samplerDesc.MipLODBias         = mipLODBias;
	samplerDesc.AddressW           = addressW;
	samplerDesc.BorderColor[0]     = (TFLOAT)((borderColor >> 24) & 0xFF) / 255.0f;
	samplerDesc.BorderColor[1]     = (TFLOAT)((borderColor >> 16) & 0xFF) / 255.0f;
	samplerDesc.BorderColor[2]     = (TFLOAT)((borderColor >> 8) & 0xFF) / 255.0f;
	samplerDesc.BorderColor[3]     = (TFLOAT)((borderColor >> 0) & 0xFF) / 255.0f;
	samplerDesc.MinLOD             = minLOD;
	samplerDesc.MaxLOD             = maxLOD;
	samplerDesc.Filter             = filter;
	samplerDesc.MaxAnisotropy      = maxAnisotropy;

	ID3D11SamplerState* pSamplerState;
	m_pDevice->CreateSamplerState(&samplerDesc, &pSamplerState);

	return pSamplerState;
}

ID3D11Buffer* TRenderDX11::CreateBuffer(UINT flags, UINT dataSize, void* data, D3D11_USAGE usage, UINT cpuAccessFlags)
{
	D3D11_BUFFER_DESC bufferDesc;

	bufferDesc.ByteWidth           = dataSize;
	bufferDesc.Usage               = usage;
	bufferDesc.CPUAccessFlags      = cpuAccessFlags;
	bufferDesc.MiscFlags           = 0;
	bufferDesc.StructureByteStride = 0;

	if (flags == 0)
	{
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	}
	else
	{
		bufferDesc.BindFlags = (flags == 1) ? D3D11_BIND_INDEX_BUFFER : D3D11_BIND_CONSTANT_BUFFER;
	}

	ID3D11Buffer* pBuffer;

	if (data != TNULL)
	{
		D3D11_SUBRESOURCE_DATA subData;
		subData.pSysMem          = data;
		subData.SysMemPitch      = 0;
		subData.SysMemSlicePitch = 0;

		m_pDevice->CreateBuffer(&bufferDesc, &subData, &pBuffer);
	}
	else
	{
		m_pDevice->CreateBuffer(&bufferDesc, TNULL, &pBuffer);
	}


	return pBuffer;
}

void TRenderDX11::SetDstAlpha(TFLOAT alpha)
{
	if (alpha >= 0)
	{
		m_CurrentBlendFactor[3]                  = alpha;
		m_CurrentBlendState.Parts.BlendOpAlpha   = D3D11_BLEND_OP_ADD;
		m_CurrentBlendState.Parts.SrcBlendAlpha  = D3D11_BLEND_BLEND_FACTOR;
		m_CurrentBlendState.Parts.DestBlendAlpha = D3D11_BLEND_ZERO;
	}
	else
	{
		m_CurrentBlendState.Parts.BlendOpAlpha   = m_CurrentBlendState.Parts.BlendOp;
		m_CurrentBlendState.Parts.SrcBlendAlpha  = m_CurrentBlendState.Parts.SrcBlend;
		m_CurrentBlendState.Parts.DestBlendAlpha = m_CurrentBlendState.Parts.DestBlend;
	}
}

void TRenderDX11::SetBlendMode(TBOOL blendEnabled, D3D11_BLEND_OP blendOp, D3D11_BLEND srcBlendAlpha, D3D11_BLEND destBlendAlpha)
{
	m_CurrentBlendState.Parts.BlendOp       = blendOp;
	m_CurrentBlendState.Parts.bBlendEnabled = blendEnabled;
	m_CurrentBlendState.Parts.SrcBlend      = srcBlendAlpha;
	m_CurrentBlendState.Parts.DestBlend     = destBlendAlpha;

	if (m_CurrentBlendState.Parts.SrcBlendAlpha != D3D11_BLEND_BLEND_FACTOR)
	{
		m_CurrentBlendState.Parts.BlendOpAlpha   = blendOp;
		m_CurrentBlendState.Parts.SrcBlendAlpha  = srcBlendAlpha;
		m_CurrentBlendState.Parts.DestBlendAlpha = destBlendAlpha;
	}
}

void TRenderDX11::SetAlphaUpdate(TBOOL update)
{
	if (update)
	{
		m_CurrentBlendState.Parts.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
	}
	else
	{
		m_CurrentBlendState.Parts.RenderTargetWriteMask &= ~D3D11_COLOR_WRITE_ENABLE_ALPHA;
	}
}

void TRenderDX11::SetColorUpdate(TBOOL update)
{
	if (update)
	{
		m_CurrentBlendState.Parts.RenderTargetWriteMask = 0b111;
	}
	else
	{
		m_CurrentBlendState.Parts.RenderTargetWriteMask = 0b000;
	}
}

void TRenderDX11::SetZMode(TBOOL depthEnable, D3D11_COMPARISON_FUNC comparisonFunc, D3D11_DEPTH_WRITE_MASK depthWriteMask)
{
	m_CurrentDepth.first.Parts.bDepthEnable   = depthEnable;
	m_CurrentDepth.first.Parts.DepthWriteMask = depthWriteMask;
	m_CurrentDepth.first.Parts.DepthFunc      = comparisonFunc;
}

void TRenderDX11::DrawImmediately(D3D11_PRIMITIVE_TOPOLOGY ePrimitiveType, UINT iIndexCount, void* pIndexData, DXGI_FORMAT eFormat, void* pVertexData, UINT iStrideSize, UINT iStrides)
{
	TINT iIndexSize = (eFormat == DXGI_FORMAT_R32_UINT) ? 4 : (eFormat == DXGI_FORMAT_R32_SINT) ? 2 :
	                                                                                              0;

	TASSERT(iIndexSize != 0);

	// Index buffer
	UINT iIndexBufferSize = iIndexSize * iIndexCount;

	if ((m_iImmediateIndexCurrentOffset + iIndexBufferSize) <= IMMEDIATE_INDEX_BUFFER_SIZE)
	{
		m_iImmediateIndexCurrentOffset = 0;
	}

	TASSERT((m_iImmediateIndexCurrentOffset + iIndexBufferSize) <= IMMEDIATE_INDEX_BUFFER_SIZE);

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	m_pDeviceContext->Map(m_MainIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	Toshi::TUtil::MemCopy((void*)((uintptr_t)mappedSubresource.pData + m_iImmediateIndexCurrentOffset), pIndexData, iIndexBufferSize);
	m_pDeviceContext->Unmap(m_MainIndexBuffer, 0);

	// Vertex buffer
	UINT iVertexBufferSize = iStrideSize * iStrides;

	if ((m_iImmediateVertexCurrentOffset + iVertexBufferSize) <= IMMEDIATE_VERTEX_BUFFER_SIZE)
	{
		m_iImmediateVertexCurrentOffset = 0;
	}

	TASSERT((m_iImmediateVertexCurrentOffset + iVertexBufferSize) <= IMMEDIATE_VERTEX_BUFFER_SIZE);

	m_pDeviceContext->Map(m_MainVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	Toshi::TUtil::MemCopy((void*)((uintptr_t)mappedSubresource.pData + m_iImmediateVertexCurrentOffset), pVertexData, iVertexBufferSize);
	m_pDeviceContext->Unmap(m_MainVertexBuffer, 0);

	// Drawing
	UpdateRenderStates();
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_MainVertexBuffer, &iStrideSize, &m_iImmediateVertexCurrentOffset);
	m_pDeviceContext->IASetIndexBuffer(m_MainIndexBuffer, eFormat, m_iImmediateIndexCurrentOffset);
	FlushConstantBuffers();
	m_pDeviceContext->IASetPrimitiveTopology(ePrimitiveType);
	m_pDeviceContext->DrawIndexed(iIndexCount, 0, 0);
	m_iImmediateIndexCurrentOffset += iIndexBufferSize;
	m_iImmediateVertexCurrentOffset += iVertexBufferSize;
}

void TRenderDX11::DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY ePrimitiveType, UINT indexCount, ID3D11Buffer* pIndexBuffer, UINT indexBufferOffset, DXGI_FORMAT indexBufferFormat, ID3D11Buffer* pVertexBuffer, UINT pStrides, UINT pOffsets)
{
	UpdateRenderStates();
	m_pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &pStrides, &pOffsets);
	m_pDeviceContext->IASetIndexBuffer(pIndexBuffer, indexBufferFormat, indexBufferOffset);
	FlushConstantBuffers();

	m_pDeviceContext->IASetPrimitiveTopology(ePrimitiveType);
	m_pDeviceContext->DrawIndexed(indexCount, 0, 0);
}

void TRenderDX11::DrawNonIndexed(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology, ID3D11Buffer* pVertexBuffer, UINT vertexCount, UINT strides, UINT startVertex, UINT offsets)
{
	UpdateRenderStates();
	m_pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &strides, &offsets);
	FlushConstantBuffers();
	m_pDeviceContext->IASetPrimitiveTopology(primitiveTopology);
	m_pDeviceContext->Draw(vertexCount, startVertex);
}

void TRenderDX11::CopyDataToTexture(ID3D11ShaderResourceView* pSRTex, UINT dataSize, void* src, UINT rowSize)
{
	UINT                     uVar1;
	UINT*                    copySize;
	TCHAR*                   _Src;
	UINT                     leftSize;
	D3D11_MAPPED_SUBRESOURCE dstPos;
	ID3D11Texture2D*         pTexture;

	pTexture = (ID3D11Texture2D*)0x0;
	pSRTex->GetResource((ID3D11Resource**)&pTexture);
	m_pDeviceContext->Map(pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &dstPos);
	uVar1    = rowSize;
	_Src     = (TCHAR*)src;
	leftSize = dataSize;
	while (dataSize = leftSize, leftSize != 0)
	{
		copySize = &dataSize;
		if (uVar1 <= leftSize)
		{
			copySize = &rowSize;
		}
		TUtil::MemCopy(dstPos.pData, _Src, *copySize);
		leftSize     = leftSize - uVar1;
		dstPos.pData = (void*)((TINT)dstPos.pData + dstPos.RowPitch);
		_Src         = _Src + uVar1;
		dataSize     = leftSize;
	}
	m_pDeviceContext->Unmap(pTexture, 0);
	pTexture->Release();
	return;

	//ID3D11Texture2D* pTexture;
	//pSRTex->GetResource((ID3D11Resource**)&pTexture);

	//D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	//m_pDeviceContext->Map(pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);

	//TUINT8* srcPos = (TUINT8*)src;
	//TUINT8* dstPos = (TUINT8*)mappedSubresource.pData;

	///*while (dataSize != 0)
	//{
	//	UINT copySize = TMath::Min(dataSize, rowSize);

	//	TUtil::MemCopy(dstPos, srcPos, copySize);

	//	dataSize -= rowSize;
	//	srcPos += rowSize;
	//	dstPos += mappedSubresource.RowPitch;
	//}*/

	//UINT leftSize = dataSize;
	//while (dataSize = leftSize, leftSize != 0) {
	//	UINT* copySize = &dataSize;
	//	if (rowSize <= leftSize) {
	//		copySize = &rowSize;
	//	}
	//	TUtil::MemCopy(mappedSubresource.pData, srcPos, *copySize);
	//	leftSize = leftSize - rowSize;
	//	mappedSubresource.pData = (void*)((TINT)mappedSubresource.pData + mappedSubresource.RowPitch);
	//	srcPos = srcPos + rowSize;
	//	dataSize = leftSize;
	//}

	//m_pDeviceContext->Unmap(pTexture, 0);
	//pTexture->Release();
}

void TRenderDX11::SetSamplerState(UINT startSlot, TINT samplerId, BOOL SetForPS)
{
	auto pSampler = m_SamplerStates[samplerId];

	if (SetForPS == TRUE)
	{
		m_pDeviceContext->PSSetSamplers(startSlot, 1, &pSampler);
	}
	else
	{
		m_pDeviceContext->VSSetSamplers(startSlot, 1, &pSampler);
	}
}

void TRenderDX11::WaitForEndOfRender()
{
	D3D11_QUERY_DESC queryDesc = { D3D11_QUERY_EVENT, 0 };
	ID3D11Query*     pQuery    = NULL;

	m_pDevice->CreateQuery(&queryDesc, &pQuery);

	if (pQuery != NULL)
	{
		m_pDeviceContext->End(pQuery);
		m_pDeviceContext->Flush();

		TINT data = 0;

		while (data == 0)
		{
			m_pDeviceContext->GetData(pQuery, &data, 4, 0);
		}
	}
}

void TRenderDX11::FlushShaders()
{
	TASSERT(TTRUE == IsInScene());

	for (auto it = m_OrderTables.Begin(); it != m_OrderTables.End(); it++)
	{
		it->Flush();
	}
}

void TRenderDX11::ResolveSubresource()
{
	m_pDeviceContext->ResolveSubresource(m_SRView2Texture, 0, m_SRView1Texture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void TRenderDX11::UpdateRenderStates()
{
	// Update depth state if needed
	if (m_CurrentDepth.GetFirst().Raw != m_PreviousDepth.GetFirst().Raw || m_CurrentDepth.GetSecond() != m_PreviousDepth.GetSecond())
	{
		auto pFoundNode = m_DepthStatesTree.Find(m_CurrentDepth.GetFirst());

		ID3D11DepthStencilState* pDepthStencilState;

		if (pFoundNode == m_DepthStatesTree.End())
		{
			// We don't have a depth stencil state with these flags yet
			auto currentState = m_CurrentDepth.GetFirst();

			D3D11_DEPTH_STENCIL_DESC depthStencilDesk;
			depthStencilDesk.DepthEnable                  = currentState.Parts.bDepthEnable;
			depthStencilDesk.DepthWriteMask               = (D3D11_DEPTH_WRITE_MASK)currentState.Parts.DepthWriteMask;
			depthStencilDesk.DepthFunc                    = (D3D11_COMPARISON_FUNC)currentState.Parts.DepthFunc;
			depthStencilDesk.StencilEnable                = currentState.Parts.bStencilEnable;
			depthStencilDesk.StencilReadMask              = currentState.Parts.StencilReadMask;
			depthStencilDesk.StencilWriteMask             = currentState.Parts.StencilWriteMask;
			depthStencilDesk.FrontFace.StencilFailOp      = (D3D11_STENCIL_OP)currentState.Parts.FrontFaceStencilFailOp;
			depthStencilDesk.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)currentState.Parts.FrontFaceStencilDepthFailOp;
			depthStencilDesk.FrontFace.StencilPassOp      = (D3D11_STENCIL_OP)currentState.Parts.FrontStencilPassOp;
			depthStencilDesk.FrontFace.StencilFunc        = (D3D11_COMPARISON_FUNC)currentState.Parts.FrontStencilFunc;
			depthStencilDesk.BackFace.StencilFailOp       = (D3D11_STENCIL_OP)currentState.Parts.BackFaceStencilFailOp;
			depthStencilDesk.BackFace.StencilDepthFailOp  = (D3D11_STENCIL_OP)currentState.Parts.BackFaceStencilDepthFailOp;
			depthStencilDesk.BackFace.StencilPassOp       = (D3D11_STENCIL_OP)currentState.Parts.BackStencilPassOp;
			depthStencilDesk.BackFace.StencilFunc         = (D3D11_COMPARISON_FUNC)currentState.Parts.BackStencilFunc;

			HRESULT hRes = m_pDevice->CreateDepthStencilState(&depthStencilDesk, &pDepthStencilState);
			TASSERT(SUCCEEDED(hRes));

			m_DepthStatesTree.Insert({ m_CurrentDepth.GetFirst(), pDepthStencilState });
		}
		else
		{
			// We already have a depth stencil state with these flags
			pDepthStencilState = pFoundNode->second;
		}

		m_pDeviceContext->OMSetDepthStencilState(pDepthStencilState, m_CurrentDepth.GetSecond());
		m_PreviousDepth = m_CurrentDepth;
	}

	// Update rasterizer state if needed
	if (m_CurrentRasterizerId != m_PreviousRasterizerId)
	{
		auto pFoundNode = m_RasterizersTree.Find(m_CurrentRasterizerId);

		ID3D11RasterizerState* pRasterizerState;

		if (pFoundNode == m_RasterizersTree.End())
		{
			// We don't have a rasterizer state with these flags yet
			D3D11_RASTERIZER_DESC rasterizerDesc;
			rasterizerDesc.DepthBiasClamp        = 0.0f;
			rasterizerDesc.AntialiasedLineEnable = FALSE;
			rasterizerDesc.MultisampleEnable     = m_CurrentRasterizerId.Flags.Parts.bMultisampleEnable;
			rasterizerDesc.DepthBias             = m_CurrentRasterizerId.DepthBias;
			rasterizerDesc.DepthClipEnable       = m_CurrentRasterizerId.Flags.Parts.bDepthClipEnable;
			rasterizerDesc.ScissorEnable         = m_CurrentRasterizerId.Flags.Parts.bScissorEnable;
			rasterizerDesc.FrontCounterClockwise = m_CurrentRasterizerId.Flags.Parts.bFrontCounterClockwise;
			rasterizerDesc.FillMode              = (D3D11_FILL_MODE)m_CurrentRasterizerId.Flags.Parts.FillMode;
			rasterizerDesc.CullMode              = (D3D11_CULL_MODE)m_CurrentRasterizerId.Flags.Parts.CullMode;
			rasterizerDesc.SlopeScaledDepthBias  = m_CurrentRasterizerId.SlopeScaledDepthBias;

			HRESULT hRes = m_pDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerState);
			TASSERT(SUCCEEDED(hRes));

			m_RasterizersTree.Insert({ m_CurrentRasterizerId, pRasterizerState });
		}
		else
		{
			// We already have a rasterizer state with these flags
			pRasterizerState = pFoundNode->second;
		}

		m_pDeviceContext->RSSetState(pRasterizerState);
		m_PreviousRasterizerId = m_CurrentRasterizerId;
	}

	// Turn on blend if needed
	if (m_CurrentBlendState.Parts.SrcBlendAlpha == D3D11_BLEND_BLEND_FACTOR && m_CurrentBlendState.Parts.bBlendEnabled == FALSE)
	{
		m_CurrentBlendState.Parts.bBlendEnabled = TRUE;
		m_CurrentBlendState.Parts.BlendOp       = D3D11_BLEND_OP_ADD;
		m_CurrentBlendState.Parts.SrcBlend      = D3D11_BLEND_ONE;
		m_CurrentBlendState.Parts.DestBlend     = D3D11_BLEND_ZERO;
	}

	// Update blend state if needed
	if (m_CurrentBlendState.Raw != m_PreviousBlendState.Raw || !IsColorEqual(m_CurrentBlendFactor, m_PreviousBlendFactor))
	{
		auto pFoundNode = m_BlendStatesTree.Find(m_CurrentBlendState);

		ID3D11BlendState* pBlendState;

		if (pFoundNode == m_BlendStatesTree.End())
		{
			// We don't have a blend state with these flags yet
			D3D11_BLEND_DESC blendDesc;

			blendDesc.AlphaToCoverageEnable  = FALSE;
			blendDesc.IndependentBlendEnable = FALSE;

			blendDesc.RenderTarget[0].BlendOp               = m_CurrentBlendState.Parts.BlendOp;
			blendDesc.RenderTarget[0].BlendOpAlpha          = m_CurrentBlendState.Parts.BlendOpAlpha;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = m_CurrentBlendState.Parts.RenderTargetWriteMask;
			blendDesc.RenderTarget[0].SrcBlend              = m_CurrentBlendState.Parts.SrcBlend;
			blendDesc.RenderTarget[0].SrcBlendAlpha         = m_CurrentBlendState.Parts.SrcBlendAlpha;
			blendDesc.RenderTarget[0].DestBlendAlpha        = m_CurrentBlendState.Parts.DestBlendAlpha;
			blendDesc.RenderTarget[0].BlendEnable           = m_CurrentBlendState.Parts.bBlendEnabled;
			blendDesc.RenderTarget[0].DestBlend             = m_CurrentBlendState.Parts.DestBlend;

			blendDesc.RenderTarget[1].BlendOp               = blendDesc.RenderTarget[0].BlendOp;
			blendDesc.RenderTarget[1].BlendOpAlpha          = blendDesc.RenderTarget[0].BlendOpAlpha;
			blendDesc.RenderTarget[1].RenderTargetWriteMask = blendDesc.RenderTarget[0].RenderTargetWriteMask;
			blendDesc.RenderTarget[1].SrcBlend              = blendDesc.RenderTarget[0].SrcBlend;
			blendDesc.RenderTarget[1].SrcBlendAlpha         = blendDesc.RenderTarget[0].SrcBlendAlpha;
			blendDesc.RenderTarget[1].DestBlendAlpha        = blendDesc.RenderTarget[0].DestBlendAlpha;
			blendDesc.RenderTarget[1].BlendEnable           = blendDesc.RenderTarget[0].BlendEnable;
			blendDesc.RenderTarget[1].DestBlend             = blendDesc.RenderTarget[0].DestBlend;

			HRESULT hRes = m_pDevice->CreateBlendState(&blendDesc, &pBlendState);
			TASSERT(SUCCEEDED(hRes));

			m_BlendStatesTree.Insert({ m_CurrentBlendState, pBlendState });
		}
		else
		{
			// We already have a blend state with these flags
			pBlendState = pFoundNode->second;
		}

		m_pDeviceContext->OMSetBlendState(pBlendState, m_CurrentBlendFactor, -1);
		m_PreviousBlendState     = m_CurrentBlendState;
		m_PreviousBlendFactor[0] = m_CurrentBlendFactor[0];
		m_PreviousBlendFactor[1] = m_CurrentBlendFactor[1];
		m_PreviousBlendFactor[2] = m_CurrentBlendFactor[2];
		m_PreviousBlendFactor[3] = m_CurrentBlendFactor[3];
	}
}

void TRenderDX11::RenderOverlay(TFLOAT posX, TFLOAT posY, TFLOAT width, TFLOAT height, ID3D11ShaderResourceView* pShaderResourceView, ID3D11PixelShader* pPixelShader, const TVector4* uvVec)
{
	auto pRender        = TRenderDX11::Interface();
	auto pDeviceContext = pRender->GetDeviceContext();
	pDeviceContext->VSSetShader(pRender->m_pVertexShader, NULL, NULL);

	if (pPixelShader == NULL)
	{
		pPixelShader = pRender->m_pPixelShader1;
	}

	pDeviceContext->PSSetShader(pPixelShader, NULL, NULL);
	pDeviceContext->IASetInputLayout(pRender->m_pInputLayout);

	pRender->m_CurrentRasterizerId.Flags.Parts.CullMode = D3D11_CULL_NONE;

	auto& depthFlags                = pRender->m_CurrentDepth.first;
	depthFlags.Parts.bDepthEnable   = FALSE;
	depthFlags.Parts.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthFlags.Parts.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;

	pDeviceContext->PSSetShaderResources(0, 1, &pShaderResourceView);
	pDeviceContext->PSSetSamplers(0, 1, &pRender->m_SamplerStates[1]);

	UINT numViewports = 1;

	D3D11_VIEWPORT viewPort;
	pDeviceContext->RSGetViewports(&numViewports, &viewPort);

	TVector4 scaleTranslate;
	scaleTranslate.x = (width / viewPort.Width) * 2.0f;
	scaleTranslate.y = (height / viewPort.Height) * 2.0f;
	scaleTranslate.z = (posX / viewPort.Width) * 2.0f - 1.0f;
	scaleTranslate.w = (2.0f - (posY / viewPort.Height) * 2.0f) - 1.0f;
	pRender->SetVec4InVSBuffer(VSBufferOffset_V4ScaleTranslate, &scaleTranslate, 1);

	if (uvVec == TNULL)
	{
		uvVec            = &scaleTranslate;
		scaleTranslate.x = 1.0F;
		scaleTranslate.y = 1.0F;
		scaleTranslate.z = 0.0F;
		scaleTranslate.w = 0.0F;
	}

	pRender->m_IsVertexConstantBufferSet = TTRUE;
	pRender->SetVec4InVSBuffer(VSBufferOffset_V4UVST, uvVec, 1);

	UINT stride  = 20;
	UINT offsets = 0;

	pRender->UpdateRenderStates();
	pDeviceContext->IASetVertexBuffers(0, 1, &pRender->m_pQuarterScreenQuadBuffer, &stride, &offsets);
	pRender->FlushConstantBuffers();

	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pDeviceContext->Draw(4, 0);
}

void TRenderDX11::FlushConstantBuffers()
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresources;

	if (m_IsVertexConstantBufferSet)
	{
		m_VertexBufferIndex = (m_VertexBufferIndex + 1) % NUMBUFFERS;
		m_pDeviceContext->Map(m_VertexBuffers[m_VertexBufferIndex], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresources);
		memcpy(mappedSubresources.pData, m_pVertexConstantBuffer, VERTEX_CONSTANT_BUFFER_SIZE);
		m_pDeviceContext->Unmap(m_VertexBuffers[m_VertexBufferIndex], 0);
		m_IsVertexConstantBufferSet = TFALSE;
	}

	if (m_IsPixelConstantBufferSet)
	{
		m_PixelBufferIndex = (m_PixelBufferIndex + 1) % NUMBUFFERS;
		m_pDeviceContext->Map(m_PixelBuffers[m_PixelBufferIndex], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresources);
		memcpy(mappedSubresources.pData, m_pPixelConstantBuffer, PIXEL_CONSTANT_BUFFER_SIZE);
		m_pDeviceContext->Unmap(m_PixelBuffers[m_PixelBufferIndex], 0);
		m_IsPixelConstantBufferSet = TFALSE;
	}

	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_VertexBuffers[m_VertexBufferIndex]);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_PixelBuffers[m_PixelBufferIndex]);
}

void TRenderDX11::BuildAdapterDatabase()
{
	// 006a7df0
	IDXGIFactory1* pFactory;

	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));

	if (SUCCEEDED(hr))
	{
		IDXGIAdapter* dxgiAdapter;

		TINT    adapterIndex = 0;
		HRESULT enumResult   = pFactory->EnumAdapters(adapterIndex, &dxgiAdapter);

		while (enumResult != DXGI_ERROR_NOT_FOUND)
		{
			TD3DAdapter* d3dAdapter  = new TD3DAdapter;
			auto         adapterDesc = d3dAdapter->GetAdapterDesc();
			dxgiAdapter->GetDesc(adapterDesc);

			d3dAdapter->SetAdapterIndex(adapterIndex);
			d3dAdapter->UpdateAdapterInfo();

			TCHAR adapter[128];
			TStringManager::StringUnicodeToChar(adapter, adapterDesc->Description, -1);
			d3dAdapter->SetDescription(adapter);

			TUtil::Log("Adapter: %s", adapter);
			TUtil::LogUp();
			TUtil::Log("Vendor: %d, Device: %d Revision: %d", adapterDesc->VendorId, adapterDesc->DeviceId, adapterDesc->Revision);
			TUtil::Log("DedicatedSystemMemory: %.2f MB", (double)adapterDesc->DedicatedSystemMemory / 1024 / 1024);
			TUtil::Log("DedicatedVideoMemory : %.2f MB", (double)adapterDesc->DedicatedVideoMemory / 1024 / 1024);
			TUtil::Log("SharedSystemMemory   : %.2f MB", (double)adapterDesc->SharedSystemMemory / 1024 / 1024);

			d3dAdapter->EnumerateOutputs(this, dxgiAdapter);

			GetAdapterList()->InsertTail(*d3dAdapter);
			TUtil::LogDown();
			dxgiAdapter->Release();
			enumResult = pFactory->EnumAdapters(++adapterIndex, &dxgiAdapter);
		}

		if (pFactory) pFactory->Release();
	}
}

void TD3DAdapter::EnumerateOutputs(TRenderDX11* render, IDXGIAdapter* dxgiAdapter)
{
	IDXGIOutput* dxgiOutput;
	TBOOL        shouldSaveMonitorData = TTRUE;
	GetModeList()->DeleteAll();

	TINT    displayIndex = 0;
	HRESULT enumResult   = dxgiAdapter->EnumOutputs(displayIndex, &dxgiOutput);

	DXGI_MODE_DESC   modeDesc;
	DXGI_OUTPUT_DESC outputDesc;
	while (enumResult != DXGI_ERROR_NOT_FOUND)
	{
		dxgiOutput->GetDesc(&outputDesc);
		LONG deviceHeight = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
		LONG deviceWidth  = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;

		TCHAR deviceName[128];
		TStringManager::StringUnicodeToChar(deviceName, outputDesc.DeviceName, -1);

		TUtil::Log("Display[%d]: %s (%dx%d)", displayIndex, deviceName, deviceWidth, deviceHeight);
		TUtil::LogConsole("Display[%d]: %s (%dx%d)", displayIndex, deviceName, deviceWidth, deviceHeight);

		UINT numModes = 0;
		dxgiOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);

		if (numModes > 0)
		{
			TD3DAdapter::Mode::GetDisplayMode(dxgiOutput, &modeDesc);

			if (shouldSaveMonitorData == TTRUE)
			{
				m_Mode.SetAdapter(this);
				m_Mode.SetDescription(modeDesc);
				m_Mode.SetName(outputDesc.DeviceName);
				shouldSaveMonitorData = TFALSE;
			}

			DXGI_MODE_DESC* descriptions = new DXGI_MODE_DESC[numModes];

			dxgiOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, descriptions);

			for (size_t i = 0; i < numModes; i++)
			{
				Mode* mode = new Mode;
				mode->SetModeIndex(i);
				mode->SetDisplayIndex(displayIndex);
				mode->SetAdapter(this);
				mode->SetDescription(descriptions[i]);
				mode->SetName(outputDesc.DeviceName);

				GetModeList()->InsertTail(*mode);
			}

			delete[] descriptions;
		}

		displayIndex += 1;
		enumResult = dxgiAdapter->EnumOutputs(displayIndex, &dxgiOutput);
	}
}

TOSHI_NAMESPACE_END
