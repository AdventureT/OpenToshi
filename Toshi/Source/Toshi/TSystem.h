#pragma once
#include "TEvent.h"
#include "TSingleton.h"
#include "THPTimer.h"
#include "TNodeList.h"
#include "TDate.h"

TOSHI_NAMESPACE_START

class TScheduler;

//-----------------------------------------------------------------------------
// Purpose: owns the scheduler and the frame timer.
// NOTE: in de Blob's branch TSystemManager is a TSingleton created by Create (Barnyard: the global g_oSystemManager
// with a TSystemTimer), and Barnyard's TKernelInterface doesn't exist anymore.
//-----------------------------------------------------------------------------
class TSystemManager : public TSingleton<TSystemManager>
{
public:
	TSystemManager();
	~TSystemManager();

	void Update();

	// $deBlob: FUNCTION 006616a0
	// $Barnyard: FUNCTION 006bbc50
	TBOOL Pause( TBOOL a_bPause )
	{
		TBOOL bOldState = m_bPaused;
		m_bPaused       = a_bPause;
		m_PauseEmitter.Throw( &a_bPause );
		return bOldState;
	}

	TEmitter<TSystemManager, TBOOL>& GetPauseEmitter() { return m_PauseEmitter; }

	TBOOL       IsPaused() const { return m_bPaused; }
	TUINT32     GetFrameCount() const { return m_uiFrameCount; }
	THPTimer&   GetTimer() { return m_oTimer; }
	TScheduler* GetScheduler() const { return m_pScheduler; }
	TFLOAT      GetAverageFps() const { return m_fAverageFps; }
	TFLOAT      GetFPS() const { return 1.0f / m_oTimer.GetDelta(); }

public:
	static TBOOL Create();
	static void  Destroy();

	// Fills a_rOutDate with the current local date and time
	static TBOOL GetLocalTime( TDate& a_rOutDate );

private:
	TEmitter<TSystemManager, TBOOL> m_PauseEmitter; // 0x00
	TScheduler*                     m_pScheduler;   // 0x0C
	TBOOL                           m_bPaused;      // 0x10
	THPTimer                        m_oTimer;       // 0x18
	TFLOAT                          m_fSecond;      // 0x40
	TFLOAT                          m_fAverageFps;  // 0x44
	TUINT32                         m_uiFrameCount; // 0x48
	TBOOL                           m_bUnk;         // 0x4C
}; // 0x50

TOSHI_NAMESPACE_END
