#pragma once
#include "AGameState.h"
#include "GUI/A2GUIBackground.h"
#include "Input/AInputHelper.h"

//-----------------------------------------------------------------------------
// Purpose: the frontend's legal screen and movies: the legal background for 5 seconds, the THQ Nordic logo, the logo
// movie and the intro movie (each can be skipped), then Finish creates the frontend (frontend.xur and the splash
// screen).
//-----------------------------------------------------------------------------
class AFrontEndMovieState : public AGameState
{
public:
	TDECLARE_CLASS( AFrontEndMovieState, AGameState );

	enum STATE
	{
		STATE_LEGAL,         // the background bkg_Legal_Blob_ENG
		STATE_THQLOGO,       // the movie THQNORDIC_LOGO_Color_white
		STATE_LOGOMOVIE,     // the movie logoMovie
		STATE_INTRO,         // the movie 00_ChromaCity
		STATE_FADETOBLACK,   // fading to black before the background m_eNextState
		STATE_FADEFROMBLACK, // fading from black to the background m_eNextState
		STATE_RESTART,       // fading to black before the frontend restarts
	};

	// The background/movie of each state
	static const TCHAR* scm_szAssetName[ 4 ];

public:
	AFrontEndMovieState();
	~AFrontEndMovieState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override; // +0x18
	virtual void         OnInsertion() override;                   // +0x1C
	virtual void         OnRemoval() override;                     // +0x20
	virtual void         OnActivate() override;                    // +0x2C
	virtual void         OnDeactivate() override;                  // +0x30

	// Pushes the first frontend state: the language menu or ASaveLoadStartAppState (by region)
	static void StartBlobFrontendMenu();

	// $deBlob: FUNCTION 005d5800
	void SetState( STATE a_eState ) { m_eState = a_eState; }

private:
	TBOOL HasBackgroundStoppedShowing();
	TBOOL HasMovieStopped();
	void  StopMovieIfPlaying();

	void GoToNextMovie( STATE a_eState );
	void GoToNextBackground( STATE a_eState, TFLOAT a_fFadeOutTime, TFLOAT a_fFadeInTime );
	// Stops the movie and fades to black before the frontend restarts
	void         Restart();
	void         InitState( STATE a_eState );
	void         CreateBackGround( STATE a_eState );
	UpdateResult Finish();

private:
	A2GUIBackground m_Background;          // 0x02C
	TFLOAT          m_fBackgroundLeftTime; // 0x104
	STATE           m_eState;              // 0x108
	TBOOL           m_bSkipMovie;          // 0x10C
	TBOOL           m_bSkipAllMovies;      // 0x10D
	AInputHelper    m_InputHelper;         // 0x110 the skip buttons
	TBOOL           m_bEntityDataLoaded;   // 0x134
	STATE           m_eNextState;          // 0x138 the background after the fade
	TFLOAT          m_fFadeInTime;         // 0x13C
};
