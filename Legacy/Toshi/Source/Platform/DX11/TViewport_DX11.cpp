#include "ToshiPCH.h"
#include "Toshi/Render/TViewport.h"
#include "TRender_DX11.h"
#include "Includes.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006bb170
void TViewport::BeginSKU()
{
	auto pRender        = TRenderDX11::Interface();
	auto pDeviceContext = pRender->GetDeviceContext();

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = GetX();
	viewport.TopLeftY = GetY();
	viewport.Width    = GetWidth();
	viewport.Height   = GetHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	pDeviceContext->RSSetViewports(1, &viewport);

	if (m_bAllowBackgroundClear || m_bAllowDepthClear)
	{
		ID3D11RenderTargetView* pRenderTargetView;
		ID3D11DepthStencilView* pDepthStencilView;

		pDeviceContext->OMGetRenderTargets(1, &pRenderTargetView, &pDepthStencilView);

		if (m_bAllowBackgroundClear && pRenderTargetView != TNULL)
		{
			TUINT8 r, g, b, a;
			GetBackgroundColor(r, g, b, a);

			FLOAT clearColor[4];
			clearColor[0] = (TFLOAT)r / 255.0f;
			clearColor[1] = (TFLOAT)g / 255.0f;
			clearColor[2] = (TFLOAT)b / 255.0f;
			clearColor[3] = (TFLOAT)a / 255.0f;

			pDeviceContext->ClearRenderTargetView(pRenderTargetView, clearColor);
		}

		if (m_bAllowDepthClear && pDepthStencilView != TNULL)
		{
			pDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, viewport.MaxDepth, 0);
		}
	}
}

// $deBlob: FUNCTION 006bb2f0
void TViewport::EndSKU()
{
	TRenderDX11::Interface()->FlushShaders();
}

// $deBlob: FUNCTION 006bb300
void TViewport::ChangeSKU(ChangeEvent a_eEvent)
{}

TOSHI_NAMESPACE_END
