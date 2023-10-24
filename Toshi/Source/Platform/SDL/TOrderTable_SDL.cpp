#include "ToshiPCH.h"
#include "TRender_SDL.h"

namespace Toshi {

	TBOOL TOrderTable::Create(TShader* pShader, int priority, uint8_t index)
	{
		m_pShader = pShader;
		m_pLastRegMat = TNULL;
		SetPriority(index);
		TRenderSDL::Interface()->GetOrderTables().Insert(this);
		m_iIndex = index;

		return TTRUE;
	}

}
