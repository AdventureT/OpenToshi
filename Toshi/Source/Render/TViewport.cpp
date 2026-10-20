#include "ToshiPCH.h"
#include "Render/TViewport.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006b2540
// $Barnyard: FUNCTION 006cd7e0
TViewport::TViewport()
{
	// NOTE: Barnyard's takes a flag to connect TRenderInterface's default viewport handlers; de Blob has none
	m_MemAllocatorBlock     = TNULL;
	m_pRenderCtx            = TNULL;
	m_ColorR                = 0;
	m_ColorG                = 0;
	m_ColorB                = 0;
	m_ColorA                = 0;
	m_bAllowBackgroundClear = TTRUE;
	m_bAllowDepthClear      = TTRUE;
	m_bDefaultBeginRender   = TTRUE;
	m_bEnable               = TTRUE;
	m_bInViewport           = TFALSE;
	m_pRenderCtx            = TRender::GetSingleton()->CreateRenderContext();

	SetWidth( -1.0f );
	SetHeight( -1.0f );
	CreateSKU();
}

// $deBlob: FUNCTION 006b2630
// $deBlob: FUNCTION 005f0200
// $Barnyard: FUNCTION 006cd330
// $Barnyard: FUNCTION 006cd860
TViewport::~TViewport()
{
	DestroySKU();

	if ( m_pRenderCtx )
	{
		delete m_pRenderCtx;
		m_pRenderCtx = TNULL;
	}
}

// $deBlob: FUNCTION 006b2650
// $Barnyard: FUNCTION 006cd7c0
void TViewport::Begin()
{
	TASSERT( TFALSE == m_bInViewport );
	m_bInViewport = TTRUE;
	BeginSKU();
}

// $deBlob: FUNCTION 006b26a0
// $Barnyard: FUNCTION 006cd7d0
void TViewport::End()
{
	EndSKU();
	TASSERT( TTRUE == m_bInViewport );
	m_bInViewport = TFALSE;
}

// $deBlob: FUNCTION 006b26f0
// $Barnyard: FUNCTION 006cd360
TFLOAT TViewport::SetX( TFLOAT a_fValue )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fX == a_fValue )
		return a_fValue;

	TFLOAT fOldValue = params.fX;
	params.fX        = a_fValue;
	m_pRenderCtx->SetViewportParameters( params );
	ChangeSKU( ChangeEvent_X );

	return fOldValue;
}

// $deBlob: FUNCTION 006b2780
// $Barnyard: FUNCTION 006cd3e0
TFLOAT TViewport::SetY( TFLOAT a_fValue )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fY == a_fValue )
		return a_fValue;

	TFLOAT fOldValue = params.fY;
	params.fY        = a_fValue;
	m_pRenderCtx->SetViewportParameters( params );
	ChangeSKU( ChangeEvent_Y );

	return fOldValue;
}

// $deBlob: FUNCTION 006b2810
// $Barnyard: FUNCTION 006cd460
TFLOAT TViewport::SetWidth( TFLOAT a_fValue )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fWidth == a_fValue )
		return a_fValue;

	TFLOAT fOldValue = params.fWidth;
	params.fWidth    = a_fValue;
	m_pRenderCtx->SetViewportParameters( params );
	ChangeSKU( ChangeEvent_Width );

	return fOldValue;
}

// $deBlob: FUNCTION 006b28a0
// $Barnyard: FUNCTION 006cd4e0
TFLOAT TViewport::SetHeight( TFLOAT a_fValue )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fHeight == a_fValue )
		return a_fValue;

	TFLOAT fOldValue = params.fHeight;
	params.fHeight   = a_fValue;
	m_pRenderCtx->SetViewportParameters( params );
	ChangeSKU( ChangeEvent_Height );

	return fOldValue;
}

// $deBlob: FUNCTION 006b2930
// $Barnyard: FUNCTION 006cd560
TFLOAT TViewport::SetMinZ( TFLOAT a_fValue )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fMinZ == a_fValue )
		return a_fValue;

	TFLOAT fOldValue = params.fMinZ;
	params.fMinZ     = a_fValue;
	m_pRenderCtx->SetViewportParameters( params );
	ChangeSKU( ChangeEvent_MinZ );

	return fOldValue;
}

// $deBlob: FUNCTION 006b29c0
// $Barnyard: FUNCTION 006cd5e0
TFLOAT TViewport::SetMaxZ( TFLOAT a_fValue )
{
	TRenderContext::VIEWPORTPARAMS params = m_pRenderCtx->GetViewportParameters();

	if ( params.fMaxZ == a_fValue )
		return a_fValue;

	TFLOAT fOldValue = params.fMaxZ;
	params.fMaxZ     = a_fValue;
	m_pRenderCtx->SetViewportParameters( params );
	ChangeSKU( ChangeEvent_MaxZ );

	return fOldValue;
}

// $deBlob: FUNCTION 006b2a50
// $Barnyard: FUNCTION 006cd660
void TViewport::SetBackgroundColor( TUINT8 r, TUINT8 g, TUINT8 b, TUINT8 a )
{
	if ( m_ColorR != r || m_ColorG != g || m_ColorB != b || m_ColorA != a )
	{
		m_ColorR = r;
		m_ColorG = g;
		m_ColorB = b;
		m_ColorA = a;
		ChangeSKU( ChangeEvent_BackgroundColor );
	}
}

// $deBlob: FUNCTION 006b2aa0
// $Barnyard: FUNCTION 006cd6c0
void TViewport::SetMemoryAllocatorBlock( TMemoryHeap* a_pMemBlock )
{
	m_MemAllocatorBlock = a_pMemBlock;
	ChangeSKU( ChangeEvent_AllocatorBlock );
}

// $deBlob: FUNCTION 006b2ac0
// $Barnyard: FUNCTION 006cd6e0
TFLOAT TViewport::GetX() const
{
	return m_pRenderCtx->GetViewportParameters().fX;
}

// $deBlob: FUNCTION 006b2ad0
// $Barnyard: FUNCTION 006cd6f0
TFLOAT TViewport::GetY() const
{
	return m_pRenderCtx->GetViewportParameters().fY;
}

// $deBlob: FUNCTION 006b2ae0
// $Barnyard: FUNCTION 006cd700
TFLOAT TViewport::GetWidth() const
{
	return m_pRenderCtx->GetViewportParameters().fWidth;
}

// $deBlob: FUNCTION 006b2af0
// $Barnyard: FUNCTION 006cd710
TFLOAT TViewport::GetHeight() const
{
	return m_pRenderCtx->GetViewportParameters().fHeight;
}

// $deBlob: FUNCTION 006b2b00
TFLOAT TViewport::GetMinZ() const
{
	return m_pRenderCtx->GetViewportParameters().fMinZ;
}

// $deBlob: FUNCTION 006b2b10
TFLOAT TViewport::GetMaxZ() const
{
	return m_pRenderCtx->GetViewportParameters().fMaxZ;
}

// $deBlob: FUNCTION 006b2b20
// $Barnyard: FUNCTION 006cd720
void TViewport::GetBackgroundColor( TUINT8& r, TUINT8& g, TUINT8& b, TUINT8& a )
{
	r = m_ColorR;
	g = m_ColorG;
	b = m_ColorB;
	a = m_ColorA;
}

// $deBlob: FUNCTION 006b2b50
// $Barnyard: FUNCTION 006cd750
TBOOL TViewport::AllowBackgroundClear( TBOOL a_bAllow )
{
	TBOOL bOldState         = m_bAllowBackgroundClear;
	m_bAllowBackgroundClear = a_bAllow;
	ChangeSKU( ChangeEvent_BackgroundClear );

	return bOldState;
}

// $deBlob: FUNCTION 006b2b80
// $Barnyard: FUNCTION 006cd780
TBOOL TViewport::AllowDepthClear( TBOOL a_bAllow )
{
	TBOOL bOldState    = m_bAllowDepthClear;
	m_bAllowDepthClear = a_bAllow;
	ChangeSKU( ChangeEvent_DepthClear );

	return bOldState;
}

// $deBlob: FUNCTION 006b2bb0
// $Barnyard: FUNCTION 006cd7b0
TBOOL TViewport::EnableDefaultBeginRender( TBOOL a_bEnable )
{
	TBOOL bOldState       = m_bDefaultBeginRender;
	m_bDefaultBeginRender = a_bEnable;

	return bOldState;
}

TOSHI_NAMESPACE_END
