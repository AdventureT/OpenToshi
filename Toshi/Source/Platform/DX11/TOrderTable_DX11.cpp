#include "ToshiPCH.h"
#include "TRender_DX11.h"

TOSHI_NAMESPACE_START

TBOOL TOrderTable::Create(TShader* pShader, TINT priority, TUINT8 index)
{
	m_pShader     = pShader;
	m_pLastRegMat = TNULL;
	SetPriority(index);
	TRenderDX11::Interface()->GetOrderTables().Insert(this);
	m_iIndex = index;

	return TTRUE;
}

TOSHI_NAMESPACE_END
