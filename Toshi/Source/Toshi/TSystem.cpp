#include "ToshiPCH.h"
#include "TSystem.h"
#include "TScheduler.h"
#include "Math/TMathInline.h"

#include <time.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00661870
// $Barnyard: FUNCTION 00710770
TSystemManager::TSystemManager()
    : m_PauseEmitter( this )
{
	m_pScheduler   = TNULL;
	m_bPaused      = TFALSE;
	m_fSecond      = 0.0f;
	m_fAverageFps  = 0.0f;
	m_uiFrameCount = 0;
	m_bUnk         = TFALSE;
}

// $deBlob: FUNCTION 006618f0
// $Barnyard: FUNCTION 00713e20
TSystemManager::~TSystemManager()
{
}

// $deBlob: FUNCTION 00661640
// $Barnyard: FUNCTION 006bbb00
void TSystemManager::Update()
{
	m_oTimer.Update();

	// Average fps, refreshed every second
	TFLOAT fDeltaTime = m_oTimer.GetDelta();
	m_fSecond += fDeltaTime;

	if ( m_fSecond > 1.0f )
	{
		m_fSecond     = 0.0f;
		m_fAverageFps = ( 1.0f / fDeltaTime ) * 0.5f + m_fAverageFps * 0.5f;
	}

	m_uiFrameCount += 1;

	// NOTE: inlined in retail
	m_pScheduler->Update( GetSingleton()->m_oTimer.GetDelta() );
}

// $deBlob: FUNCTION 00661540
// $Barnyard: FUNCTION 006bba90
TBOOL TSystemManager::Create()
{
	TSystemManager* pSystemManager = new TSystemManager();
	pSystemManager->m_pScheduler   = new TScheduler();
	return TTRUE;
}

// $deBlob: FUNCTION 00661590
void TSystemManager::Destroy()
{
	TASSERT( TNULL != GetSingleton() );

	if ( GetSingleton()->m_pScheduler )
		delete GetSingleton()->m_pScheduler;

	if ( TSystemManager* pSystemManager = GetSingleton() )
		delete pSystemManager;
}

// NOTE: retail converts each field with TMath's checked casts (TMathInline.h:96, 97)
static TUINT8 TDate_ToUInt8( TINT a_iX )
{
	TASSERT( ( a_iX >= 0 ) && ( a_iX <= TMath::TUINT8_MAX ) );
	return TUINT8( a_iX );
}

static TUINT16 TDate_ToUInt16( TINT a_iX )
{
	TASSERT( ( a_iX >= 0 ) && ( a_iX <= TMath::TUINT16_MAX ) );
	return TUINT16( a_iX );
}

// $deBlob: FUNCTION 006616c0
// $Barnyard: FUNCTION 006bbbb0
TBOOL TSystemManager::GetLocalTime( TDate& a_rOutDate )
{
	__time64_t iTime;
	_time64( &iTime );
	tm* pTime = _localtime64( &iTime );

	a_rOutDate.uiHour   = TDate_ToUInt8( pTime->tm_hour );
	a_rOutDate.uiMinute = TDate_ToUInt8( pTime->tm_min );
	a_rOutDate.uiSecond = TDate_ToUInt8( pTime->tm_sec );
	a_rOutDate.uiDay    = TDate_ToUInt8( pTime->tm_mday );
	a_rOutDate.uiMonth  = TDate_ToUInt8( pTime->tm_mon );
	a_rOutDate.uiYear   = TDate_ToUInt16( pTime->tm_year + 1900 );

	return TTRUE;
}

TOSHI_NAMESPACE_END
