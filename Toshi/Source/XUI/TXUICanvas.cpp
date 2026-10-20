#include "ToshiPCH.h"
#include "TXUICanvas.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiCanvas" type info
const TXUITypeInfo XURXUICanvasData::sm_sTypeInfo = { L"XuiCanvas", &XURXUIElementData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUICanvasData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006dd5a0
XURXUICanvasData::XURXUICanvasData()
{
	m_pClass = TClass::Find( "TXUICanvas", &TGetClass( TXUIElement ) );
}

// $deBlob: FUNCTION 006dd5d0
TBOOL XURXUICanvasData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	a_rData += 1;
	return TTRUE;
}

// $deBlob: FUNCTION 006dd5f0
TUINT32 XURXUICanvasData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	a_rData += 1;
	return uiSize;
}

// NOTE: XuiCanvas has no properties, it only skips its property count
// $deBlob: FUNCTION 006dd610
TBOOL XURXUICanvasData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	a_rData += 1;
	return TTRUE;
}

// $deBlob: FUNCTION 006dd590
const TXUITypeInfo* XURXUICanvasData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUICanvas
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006dd620
// $deBlob: FUNCTION 006dd630
// $deBlob: FUNCTION 006dd680
// $deBlob: FUNCTION 006dd6c0
// $deBlob: FUNCTION 006dd6d0
// $deBlob: FUNCTION 0053f250
// $deBlob: FUNCTION 008591d0
TXUI_DEFINE_CLASS( TXUICanvas );

// $deBlob: FUNCTION 006dd6e0
TXUICanvas::TXUICanvas()
    : m_oTimelineControl( this )
{
}

// $deBlob: FUNCTION 006dd710
// $deBlob: FUNCTION 006dd800
// $deBlob: FUNCTION 006dd7eb
TXUICanvas::~TXUICanvas()
{
}

// $deBlob: FUNCTION 006dd730
TBOOL TXUICanvas::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, a_bCreateChildren );
	m_oTimelineControl.Create( a_rResource, a_pData );

	return TTRUE;
}

// $deBlob: FUNCTION 006dd760
void TXUICanvas::PlayAnimation( TINT a_iStartFrame, TINT a_iEndFrame )
{
	TXUIElement::PlayAnimation( a_iStartFrame, a_iEndFrame );
}

// $deBlob: FUNCTION 006dd770
void TXUICanvas::StopAnimation()
{
	TXUIElement::StopAnimation();
}

// $deBlob: FUNCTION 006dd780
void TXUICanvas::AddScene( TXUIElement* a_pElement )
{
	AddChildTail( a_pElement );
	TASSERT( a_pElement->GetFlag( FLAGS_XUIELEMENT ) );

	TXUITimeline* pTimeline = m_oTimelineControl.FindTimeline( a_pElement->GetID() );

	if ( pTimeline != TNULL )
		pTimeline->m_pElement = a_pElement;
}

TOSHI_NAMESPACE_END
