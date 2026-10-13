#include "ToshiPCH.h"
#include "TXUICanvas.h"

TOSHI_NAMESPACE_START

TBOOL XURXUICanvasData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);
	a_pData++;
	return TTRUE;
}

void TXUICanvas::SetDimensions(TFLOAT a_X, TFLOAT a_Y)
{}

TOSHI_NAMESPACE_END
