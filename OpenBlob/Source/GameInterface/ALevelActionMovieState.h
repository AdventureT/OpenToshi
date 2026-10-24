#pragma once
#include "AMovieState.h"

//-----------------------------------------------------------------------------
// Purpose: a movie of a level action: an AMovieState with the localised audio that disables the screen fade while it
// plays and restores it when it's destroyed.
// NOTE: no own TClass (its vtable keeps AMovieState's GetClass); no code in retail's.text creates one.
//-----------------------------------------------------------------------------
class ALevelActionMovieState
    : public AMovieState
{
public:
	ALevelActionMovieState( const TCHAR* a_szMovie );
	~ALevelActionMovieState();

	virtual void PrePlayMovie() override; // +0x44

private:
	TBOOL m_bFadeWasEnabled; // 0xE4 AGameStateController's fade before the movie
};
