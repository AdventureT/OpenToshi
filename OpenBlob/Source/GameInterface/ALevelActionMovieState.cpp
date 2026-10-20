#include "pch.h"
#include "ALevelActionMovieState.h"
#include "AGameStateController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005c8ee0
ALevelActionMovieState::ALevelActionMovieState( const TCHAR* a_szMovie )
    : AMovieState( a_szMovie, TTRUE, TFALSE, TNULL )
{
	m_bFadeWasEnabled = TFALSE;
}

// $deBlob: FUNCTION 005c8f10
// $deBlob: FUNCTION 005c8fb0
ALevelActionMovieState::~ALevelActionMovieState()
{
	AGameStateController::GetSingleton()->EnableFade( m_bFadeWasEnabled, 0 );
}

// $deBlob: FUNCTION 005c8f80
void ALevelActionMovieState::PrePlayMovie()
{
	m_bFadeWasEnabled = AGameStateController::GetSingleton()->IsFadeEnabled( 0 );
	AGameStateController::GetSingleton()->EnableFade( TFALSE, 0 );
}
