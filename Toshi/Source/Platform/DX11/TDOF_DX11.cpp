#include "ToshiPCH.h"
#include "TDOF_DX11.h"
#include "TRender_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006bab60
TDOF::TDOF()
{
	m_pUnk00 = TNULL;
	m_pUnk0C = TNULL;
	m_fUnk10 = 3.0f;
	m_fUnk14 = 15.0f;
	m_fUnk18 = 1.0f;
	m_fUnk1C = 20.0f;
	m_fUnk20 = 0.95f;
	m_bUnk24 = TFALSE;

	TRenderDX11* pRender = TRenderDX11::Interface();
	m_uiWidth            = pRender->GetSwapChainDesc()->BufferDesc.Width;
	m_uiHeight           = pRender->GetSwapChainDesc()->BufferDesc.Height;
}

// $deBlob: FUNCTION 006babc0
// $deBlob: FUNCTION 006aa7d0
TDOF::~TDOF()
{
	if ( m_pUnk0C )
	{
		m_pUnk0C->Release();
		m_pUnk0C = TNULL;
	}

	if ( m_pUnk00 )
	{
		m_pUnk00->Release();
		m_pUnk00 = TNULL;
	}
}

// $deBlob: FUNCTION 006bac00
void TDOF::Unknown1( void* a_pUnk )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006bac30
void TDOF::Unknown2( void* a_pUnk1, void* a_pUnk2 )
{
	TASSERT( TFALSE );
}

TOSHI_NAMESPACE_END
