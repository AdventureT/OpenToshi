#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/TEvent.h>

#include <stdio.h>

//-----------------------------------------------------------------------------
// The events AMoviePlayer throws (ThrowEvent): 0 when a movie ended, 1 when a looping movie restarts and 2 when it
// was stopped.
//-----------------------------------------------------------------------------
namespace AMovieEvent
{
enum Type
{
	Type_Finished,
	Type_Looping,
	Type_Stopped,
};
} // namespace AMovieEvent

//-----------------------------------------------------------------------------
// Purpose: the full-screen movie player: the legal screen movies, the attract/intro movie and the cutscenes. The
// platform player is ADX11MoviePlayer (AMoviePlayer_DX11.cpp); ARenderer draws it instead of the world (OnRender) and
// AApplication updates it.
//-----------------------------------------------------------------------------
class AMoviePlayer : public Toshi::TSingleton<AMoviePlayer>
{
public:
	// The flags of PlayMovie
	typedef TUINT32 PlayFlags;
	enum PlayFlags_ : PlayFlags
	{
		PlayFlags_None  = 0,
		PlayFlags_Loop  = BITFLAG( 0 ),
		PlayFlags_Muted = BITFLAG( 1 ),
	};

public:
	AMoviePlayer();
	virtual ~AMoviePlayer(); // +0x00

	virtual TBOOL PlayMovie( const TCHAR* a_szFileName, TUINT32 a_uiSoundChannel, PlayFlags a_uiFlags ) = 0; // +0x04
	virtual void  StopMovie()                                                                           = 0; // +0x08
	virtual void  PauseMovie( TBOOL a_bPause )                                                          = 0; // +0x0C
	// $deBlob: FUNCTION 005520a0
	virtual void  Mute( TBOOL a_bMute ) { m_bIsMuted = a_bMute; } // +0x10
	virtual TBOOL IsMoviePlaying() = 0;                           // +0x14
	virtual TBOOL IsMoviePaused()  = 0;                           // +0x18
	// $deBlob: FUNCTION 00552060
	virtual TBOOL IsFullScreen() { return m_bIsFullscreen; } // +0x1C


	// $deBlob: FUNCTION 00552020
	virtual TINT GetUnk20() { return 0; } // +0x20
	// $deBlob: FUNCTION 00552030
	virtual TINT GetUnk24() { return 0; } // +0x24
	// $deBlob: FUNCTION 00552000
	virtual TINT GetUnk28() { return 0; } // +0x28

	// $deBlob: FUNCTION 005520e0
	virtual void SetHideMovie( TBOOL a_bHide ) { m_bIsHidden = a_bHide; } // +0x2C
	// $deBlob: FUNCTION 005520d0
	virtual void  SetFullScreen( TBOOL a_bFullScreen ) { m_bIsFullscreen = a_bFullScreen; } // +0x30
	virtual void  OnRender( TFLOAT a_fDeltaTime ) = 0;                                      // +0x34
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime );                                          // +0x38
	virtual void  OnCreate();                                                               // +0x3C
	virtual void  OnDestroy();                                                              // +0x40
	virtual void  OnActivate();                                                             // +0x44
	virtual void  OnDeactivate();                                                           // +0x48

	void ThrowEvent( AMovieEvent::Type a_eEvent );

	Toshi::TEmitter<AMoviePlayer, AMovieEvent::Type>& GetEmitter() { return m_Emitter; }

#ifdef DEBLOB_COMMUNITY_PATCH
	// Opens a file of the movie players read-only, falling back to the -gamedir folder; NOTE: a helper of this port
	static FILE* OpenFile( const TCHAR* a_szFileName );
#endif // DEBLOB_COMMUNITY_PATCH

protected:
	Toshi::TEmitter<AMoviePlayer, AMovieEvent::Type> m_Emitter;       // 0x04
	TBOOL                                            m_bInitialised;  // 0x10
	TBOOL                                            m_bIsFullscreen; // 0x11
	TBOOL                                            m_bIsMuted;      // 0x12
	TBOOL                                            m_bIsHidden;     // 0x13
};
