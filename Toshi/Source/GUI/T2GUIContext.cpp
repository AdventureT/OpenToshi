#include "ToshiPCH.h"
#include "T2GUI.h"
#include "T2GUIContext.h"

#include <Render/TRender.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006f53d0
T2GUIContext::T2GUIContext()
{
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();
	m_oRootElement.SetDimensions( TFLOAT( pDisplayParams->uiWidth ), TFLOAT( pDisplayParams->uiHeight ) );
	m_oRootElement.SetTransform( 0.0f, 0.0f, 0.0f );
}

// $deBlob: FUNCTION 006f5440
void T2GUIContext::Tick( TFLOAT a_fDeltaTime )
{
	m_oRootElement.Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006f5460
void T2GUIContext::Render()
{
	m_oRootElement.PreRender();
	m_oRootElement.Render();
	m_oRootElement.PostRender();
}

TOSHI_NAMESPACE_END
