#pragma once
#include "AGameState.h"
#include "Input/AInputHelper.h"

#include <ToshiTools/T2DynamicVector.h>
#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: plays a list of movies, then pushes the next state: the level's movie before ALoadLevelState
// (AGameControllerHelper) and AIOMovieComponent's movie.
// The movie starts once AAssetStreaming has no job left; the skip button (0x23) skips it after a second, then the
// screen fades to black and the state is removed.
//-----------------------------------------------------------------------------
class AMovieState
    : public AGameState
{
public:
	TDECLARE_CLASS( AMovieState, AGameState );

	// m_eState
	enum STATE
	{
		STATE_WAITFORSTREAMING, // the movie starts once AAssetStreaming has no active job
		STATE_PLAYING,
		STATE_FINISHED, // one more frame before the removal
		STATE_REMOVE,
	};

	// The button that skips the movies (OnInsertion)
	static const AInputMap::INPUTBUTTON SKIP_BUTTON;

public:
	// a_bLocalisedAudio: the movie's audio channel of the current language (ALocaleManager::GetCurSoundChannel);
	// a_bLocalisedName: the file is "<a_szMovie>_<locale code>"; a_pNextState: pushed on removal (TNULL: none)
	AMovieState( const TCHAR* a_szMovie, TBOOL a_bLocalisedAudio, TBOOL a_bLocalisedName, AGameState* a_pNextState );
	AMovieState( const TCHAR** a_ppMovies, TINT a_iNumMovies, TBOOL a_bLocalisedAudio, TBOOL a_bLocalisedName, AGameState* a_pNextState );
	~AMovieState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 005c9030
	virtual TBOOL        WasControllerRemoved() override { return TTRUE; } // +0x10
	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override;         // +0x18
	virtual void         OnInsertion() override;                           // +0x1C
	virtual void         OnRemoval() override;                             // +0x20
	virtual void         OnActivate() override;                            // +0x2C
	virtual void         OnDeactivate() override;                          // +0x30
	virtual TBOOL        CheckForControllerRemoval() override;             // +0x38

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// Plays the current movie (m_iCurrentMovie) with the movie player's flags a_uiFlags
	virtual void PlayMovie( TUINT32 a_uiFlags ); // +0x40

	// Called before each movie starts
	// $deBlob: FUNCTION 005cb590
	virtual void PrePlayMovie() {} // +0x44

protected:
	void  PlayNextMovie();
	void  StopMovie();
	TBOOL HasMovieFinished();

protected:
	Toshi::T2DynamicVector<const TCHAR*> m_vecMovies;        // 0x2C
	TBOOL                                m_bLocalisedAudio;  // 0x40
	TBOOL                                m_bLocalisedName;   // 0x41
	TINT                                 m_iCurrentMovie;    // 0x44
	AGameState*                          m_pNextState;       // 0x48
	STATE                                m_eState;           // 0x4C
	Toshi::T2GUIRectangle                m_Rectangle;        // 0x50 not used
	class ASubtitle*                     m_pSubtitle;        // 0xB0 the subtitle of the movie (ASubtitleManager)
	AInputHelper                         m_InputHelper;      // 0xB4
	TBOOL                                m_bSkipOnAnyButton; // 0xD8
	TBOOL                                m_bRenderWorld;     // 0xD9 g_oTheApp's, restored by OnDeactivate
	TFLOAT                               m_fTime;            // 0xDC the time of the current movie
	TBOOL                                m_bFadingOut;       // 0xE0 the last movie ended, fading to black
};
