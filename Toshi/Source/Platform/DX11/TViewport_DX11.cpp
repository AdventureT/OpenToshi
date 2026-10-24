#include "ToshiPCH.h"
#include "Render/TViewport.h"
#include "TRender_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006bb170
void TViewport::BeginSKU()
{
	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = GetX();
	viewport.TopLeftY = GetY();
	viewport.Width    = GetWidth();
	viewport.Height   = GetHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	pDeviceContext->RSSetViewports( 1, &viewport );

	if ( m_bAllowBackgroundClear || m_bAllowDepthClear )
	{
		ID3D11RenderTargetView* pRenderTargetView = TNULL;
		ID3D11DepthStencilView* pDepthStencilView = TNULL;
		pDeviceContext->OMGetRenderTargets( 1, &pRenderTargetView, &pDepthStencilView );

		if ( m_bAllowBackgroundClear && pRenderTargetView != TNULL )
		{
			TUINT8 r, g, b, a;
			GetBackgroundColor( r, g, b, a );

			FLOAT aClearColour[ 4 ];
			aClearColour[ 0 ] = TFLOAT( r ) / 255.0f;
			aClearColour[ 1 ] = TFLOAT( g ) / 255.0f;
			aClearColour[ 2 ] = TFLOAT( b ) / 255.0f;
			aClearColour[ 3 ] = TFLOAT( a ) / 255.0f;
			pDeviceContext->ClearRenderTargetView( pRenderTargetView, aClearColour );
		}

		if ( m_bAllowDepthClear && pDepthStencilView != TNULL )
			pDeviceContext->ClearDepthStencilView( pDepthStencilView, D3D11_CLEAR_DEPTH, viewport.MaxDepth, 0 );

#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail never releases the views OMGetRenderTargets references (two references leaked per Begin)
		if ( pRenderTargetView )
			pRenderTargetView->Release();

		if ( pDepthStencilView )
			pDepthStencilView->Release();
#endif
	}
}

// $deBlob: FUNCTION 006bb2f0
void TViewport::EndSKU()
{
	TRenderDX11::Interface()->FlushShaders();
}

// $deBlob: FUNCTION 006bb300
void TViewport::ChangeSKU( ChangeEvent a_eEvent )
{
}

// $deBlob: FUNCTION 006bb310
void TViewport::CreateSKU()
{
}

// $deBlob: FUNCTION 006bb320
void TViewport::DestroySKU()
{
}

TOSHI_NAMESPACE_END
