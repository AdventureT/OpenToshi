#pragma once
#include <XUI/TXUIScene.h>

namespace Toshi
{
class TTRB;
class TXUIVideo;
} // namespace Toshi

class ABlobCursorTrail;

//-----------------------------------------------------------------------------
// Purpose: the frontend's root scene (frontend.xur): the
// background movie the menus play in it (PlayBackgroundMovie), the profile name, the four cursor trails and the
// frontend music. The menus (ABlobSplash, ABlobMainMenu...) are scenes added to it.
//-----------------------------------------------------------------------------
class ABlobFrontEnd : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobFrontEnd, Toshi::TXUIScene );

	static TCONSTEXPR_VAR TINT NUM_CURSOR_TRAILS     = 4;
	static TCONSTEXPR_VAR TINT NUM_BACKGROUND_MOVIES = 66;

	// The background movies (indices into sm_awszBackgroundMovies)
	enum BGMOVIE : TINT
	{
		BGMOVIE_None                      = -1,
		BGMOVIE_CutTitle                  = 0,
		BGMOVIE_LoopTitle                 = 1,
		BGMOVIE_TransTitleProfileSel      = 2,
		BGMOVIE_LoopProfileSel            = 3,
		BGMOVIE_TransProfileSelEnterName  = 4,
		BGMOVIE_LoopEnterName             = 5,
		BGMOVIE_TransEnterNameMainMenu    = 6,
		BGMOVIE_CutMainMenu               = 7,
		BGMOVIE_LoopMainMenu              = 8,
		BGMOVIE_BackOptionsMainMenu       = 9,
		BGMOVIE_BackMPPlayerMainMenu      = 10,
		BGMOVIE_BackMapMainMenu01         = 11,
		BGMOVIE_BackMapMainMenu02         = 12,
		BGMOVIE_BackMapMainMenu03         = 13,
		BGMOVIE_BackMapMainMenu04         = 14,
		BGMOVIE_BackMapMainMenu05         = 15,
		BGMOVIE_BackMapMainMenu06         = 16,
		BGMOVIE_BackMapMainMenu07         = 17,
		BGMOVIE_BackMapMainMenu08         = 18,
		BGMOVIE_BackMapMainMenu09         = 19,
		BGMOVIE_BackMapMainMenu10         = 20,
		BGMOVIE_BackExtrasMainMenu        = 21,
		BGMOVIE_TransMainMenuOptions      = 22,
		BGMOVIE_LoopOptions               = 23,
		BGMOVIE_TransMainMenuExtrasA      = 24,
		BGMOVIE_LoopExtrasA               = 25,
		BGMOVIE_TransExtrasAExtrasB       = 26,
		BGMOVIE_LoopExtrasB               = 27,
		BGMOVIE_TransMainMenuMap01        = 28,
		BGMOVIE_TransMainMenuMap02        = 29,
		BGMOVIE_TransMainMenuMap03        = 30,
		BGMOVIE_TransMainMenuMap04        = 31,
		BGMOVIE_TransMainMenuMap05        = 32,
		BGMOVIE_TransMainMenuMap06        = 33,
		BGMOVIE_TransMainMenuMap07        = 34,
		BGMOVIE_TransMainMenuMap08        = 35,
		BGMOVIE_TransMainMenuMap09        = 36,
		BGMOVIE_TransMainMenuMap10        = 37,
		BGMOVIE_LoopMap01                 = 38,
		BGMOVIE_LoopMap02                 = 39,
		BGMOVIE_LoopMap03                 = 40,
		BGMOVIE_LoopMap04                 = 41,
		BGMOVIE_LoopMap05                 = 42,
		BGMOVIE_LoopMap06                 = 43,
		BGMOVIE_LoopMap07                 = 44,
		BGMOVIE_LoopMap08                 = 45,
		BGMOVIE_LoopMap09                 = 46,
		BGMOVIE_LoopMap10                 = 47,
		BGMOVIE_TransMapMusicSel          = 48,
		BGMOVIE_LoopMusicSel              = 49,
		BGMOVIE_BackMusicSelMap           = 50,
		BGMOVIE_TransMainMenuMPPlayer     = 51,
		BGMOVIE_LoopMPPlayer              = 52,
		BGMOVIE_TransMPPlayerMPColour     = 53,
		BGMOVIE_LoopMPColour              = 54,
		BGMOVIE_TransMPColourMPGameType   = 55,
		BGMOVIE_LoopMPGameType            = 56,
		BGMOVIE_TransMPGameTypeMPLevelSel = 57,
		BGMOVIE_LoopMPLevelSel            = 58,
		BGMOVIE_TransMusicMixer           = 59,
		BGMOVIE_LoopMusicMixer            = 60,
		BGMOVIE_BackGen01                 = 61,
		BGMOVIE_BackGen02                 = 62,
		BGMOVIE_BackGen03                 = 63,
		BGMOVIE_BackGen04                 = 64,
		BGMOVIE_BackGen05                 = 65,
		BGMOVIE_NUMOF                     = 66,
	};

	// The background movies, "data\movies\<name>"
	static const TWCHAR* sm_awszBackgroundMovies[ NUM_BACKGROUND_MOVIES ];

public:
	ABlobFrontEnd();
	~ABlobFrontEnd();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	// Plays a background movie (and its audio track a_iAudioMovie, -1: none) in the video element
	void PlayBackgroundMovie( TINT a_iMovie, TINT a_iAudioMovie, TBOOL a_bLoop, TBOOL a_bPaused, TBOOL a_bPauseAtEnd );

	void  StopBackgroundMovie();
	TBOOL IsBackgroundMoviePlaying();
	void  SetProfileName( const TWCHAR* a_wszName );
	void  ResetCursorTrails();
	void  ClampCursorTrailFades();
	void  SetCursorTrailsVisible( TBOOL a_bVisible );

	// The menus' slide animations of the frontend scene
	void SlideLeft();
	void SlideRight();
	void SlideUp();
	void SlideDown();

	// Hides/shows the background video and the cursor trails while a gallery picture is shown (ABlobGalleries)
	void HideForGallery();
	void ShowAfterGallery();

	// BGMOVIE_BackGen01 picks one of BGMOVIE_BackGen01..BGMOVIE_BackGen05 at random
	TINT GetRandomBackgroundMovie( TINT a_iMovie );

	// $deBlob: FUNCTION 0062af00
	TINT GetBackgroundMovie() const { return m_iBackgroundMovie; }

	void SetBackgroundMovie( TINT a_iMovie );
	// $deBlob: FUNCTION 0062aff0
	void SetInputLockTime( TFLOAT a_fTime ) { m_fInputLockTime = a_fTime; }

private:
	Toshi::TXUIElement* m_pProfileName;                        // 0x130 the element with the UID 6
	Toshi::TXUIVideo*   m_pBackgroundVideo;                    // 0x134 the element with the UID 1
	TUINT32             m_uiUnk138;                            // 0x138
	Toshi::TTRB*        m_pTRB;                                // 0x13C closed and freed by the destructor
	TUINT32             m_uiUnk140;                            // 0x140
	TBOOL               m_bUnk144;                             // 0x144
	ABlobCursorTrail*   m_apCursorTrails[ NUM_CURSOR_TRAILS ]; // 0x148 the controls with the UIDs 5 to 2
	TUINT32             m_uiUnk158;                            // 0x158
	TINT                m_iBackgroundMovie;                    // 0x15C -1: none
	TFLOAT              m_fInputLockTime;                      // 0x160 TXUI's input is unlocked when it runs out
};
