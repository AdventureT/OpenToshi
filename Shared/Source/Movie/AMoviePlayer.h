#pragma once

#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Core/TEvent.h"

namespace AMovieEvent
{
enum Type
{
	Type_Finished,
	Type_Looping,
	Type_Stopped,
};
};

class AMoviePlayer : public Toshi::TSingleton<AMoviePlayer>
{
public:
	typedef uint32_t PlayFlags;
	enum PlayFlags_
	{
		PlayFlags_None  = 0,
		PlayFlags_Loop  = BITFIELD(0),
		PlayFlags_Muted = BITFIELD(1),
	};

public:
	// $deBlob: FUNCTION 005e7320
	AMoviePlayer()
	    : m_Emitter(this)
	{
		m_bInitialised  = TFALSE;
		m_bIsFullscreen = TTRUE;
		m_bIsMuted      = TFALSE;
		m_bIsHidden     = TTRUE;
	}

	void ThrowEvent(AMovieEvent::Type type) { m_Emitter.Throw(&type); }

	virtual ~AMoviePlayer()                                                                                = default;
	virtual TBOOL PlayMovie(const char* fileName, uint32_t soundChannel, PlayFlags flags = PlayFlags_None) = 0;
	virtual void  StopMovie()                                                                              = 0;
	virtual void  PauseMovie(TBOOL pause)                                                                  = 0;
	// $deBlob: FUNCTION 005520a0
	virtual void  Mute(TBOOL mute = TTRUE) { m_bIsMuted = mute; }
	virtual TBOOL IsMoviePlaying() = 0;
	virtual TBOOL IsMoviePaused()  = 0;
	// $deBlob: FUNCTION 00552060
	virtual TBOOL IsFullScreen() { return m_bIsFullscreen; }
	// $deBlob: FUNCTION 00552020
	virtual int   Unk1() { return 0; }
	// $deBlob: FUNCTION 00552030
	virtual int   Unk2() { return 0; }
	// $deBlob: FUNCTION 00552000
	virtual int   Unk3() { return 0; }
	// $deBlob: FUNCTION 005520e0
	virtual void  SetHideMovie(TBOOL hide = TTRUE) { m_bIsHidden = hide; }
	// $deBlob: FUNCTION 005520d0
	virtual void  SetFullScreen(TBOOL fullscreen = TTRUE) { m_bIsFullscreen = fullscreen; }
	virtual void  OnRender(float deltaTime) = 0;
	// $deBlob: FUNCTION 005e7460
	virtual void  OnUpdate(float deltaTime) {}
	// $deBlob: FUNCTION 005e7470
	virtual void  OnCreate()
	{
		TASSERT(TFALSE == m_bInitialised);
		m_bInitialised = TTRUE;
	}
	// $deBlob: FUNCTION 005e74b0
	virtual void OnDestroy()
	{
		TASSERT(TTRUE == m_bInitialised);
		m_bInitialised = TFALSE;
	}
	// $deBlob: FUNCTION 005e74f0
	virtual void OnActivate() {}
	// $deBlob: FUNCTION 005e7500
	virtual void OnDeactivate() {}

protected:
	Toshi::TEmitter<AMoviePlayer, AMovieEvent::Type> m_Emitter;       // 0x4
	TBOOL                                            m_bInitialised;  // 0x10
	TBOOL                                            m_bIsFullscreen; // 0x11
	TBOOL                                            m_bIsMuted;      // 0x12
	TBOOL                                            m_bIsHidden;     // 0x13
};
