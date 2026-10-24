#include "ToshiPCH.h"
#include "TVibrator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006805c0
TVibrator::TVibrator()
{
	m_uiFlags = 0;
	m_pDevice = TNULL;
	m_uiUnk1  = 0;
	m_uiUnk2  = 0;
	m_uiUnk3  = 0;
	m_uiUnk4  = 0;
}

// $deBlob: FUNCTION 006805f0
// $deBlob: FUNCTION 005e1430
TVibrator::~TVibrator()
{
}

// $deBlob: FUNCTION 00680600
TBOOL TVibrator::Create( TInputDevice* a_pDevice )
{
	m_uiFlags |= FLAGS_ENABLED;
	m_pDevice = a_pDevice;
	return TTRUE;
}

// $deBlob: FUNCTION 00680620
void TVibrator::Destroy()
{
	m_uiUnk1  = 0;
	m_uiUnk2  = 0;
	m_uiUnk3  = 0;
	m_pDevice = TNULL;
	m_uiFlags = 0;
}

// $deBlob: FUNCTION 00680650
void TVibrator::Reset()
{
	m_uiUnk1 = 0;
	m_uiUnk2 = 0;
	m_uiUnk3 = 0;
}

// $deBlob: FUNCTION 00680670
void TVibrator::Enable( TBOOL a_bEnable )
{
	if ( a_bEnable )
	{
		m_uiFlags |= FLAGS_ENABLED;
	}
	else
	{
		m_uiFlags &= ~FLAGS_ENABLED;
		Reset();
	}
}

// $deBlob: FUNCTION 006806a0
void TVibrator::Update( TFLOAT a_fDeltaTime )
{
}

// $deBlob: FUNCTION 006806b0
void TVibrator::Pause( TBOOL a_bPause )
{
	if ( m_pDevice != TNULL )
	{
		if ( a_bPause )
			m_uiFlags |= FLAGS_PAUSED;
		else
			m_uiFlags &= ~FLAGS_PAUSED;
	}
}

// $deBlob: FUNCTION 006806d0
TBOOL TVibrator::RunScript( TVibratorCommand* a_pCommands, TINT a_iNumCommands )
{
	return m_pDevice != TNULL ? TTRUE : TFALSE;
}

TOSHI_NAMESPACE_END
