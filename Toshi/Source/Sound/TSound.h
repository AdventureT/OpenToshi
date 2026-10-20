#pragma once
#include "Toshi/TSingleton.h"

#include <fmod.hpp>
#include <fmod_event.hpp>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// TSound (0x20): owns the FMOD Event system (FMOD Ex 4.44) and its FMOD::System. Source:
// ..\..\Source\Sound\TSound.cpp and TSound_Win.cpp.
//-----------------------------------------------------------------------------
class TSound : public TSingleton<TSound>
{
public:
	enum SpeakerType : TINT
	{
		SpeakerType_Mono,
		SpeakerType_Stereo,
		SpeakerType_Driver,
		SpeakerType_NUMOF
	};

	struct SoundInitValues
	{
		SoundInitValues()
		    : pPoolMem( TNULL ), iPoolSize( -1 ), iMaxChannels( -1 ), iFileBufferSize( -1 ), eSpeakerType( SpeakerType_NUMOF )
		{}

		void*       pPoolMem;        // 0x00
		TINT        iPoolSize;       // 0x04
		TINT        iMaxChannels;    // 0x08
		TINT        iFileBufferSize; // 0x0C
		SpeakerType eSpeakerType;    // 0x10
	};

public:
	TSound();
	~TSound();

	TBOOL Create( const SoundInitValues* a_pInitValues );
	TBOOL Create( void* a_pPoolMem, TINT a_iPoolSize, TINT a_iMaxChannels, TINT a_iFileBufferSize, SpeakerType a_eSpeakerType );
	void  ResetSystem();
	void  Destroy();
	void  Update();

	FMOD::EventSystem* GetEventSystem();
	FMOD::System*      GetSystem();

	TBOOL IsInitialised() const { return m_bInitialised; }

	void SetInitialised( TBOOL a_bInitialised );

	static TBOOL ErrorCheck( FMOD_RESULT a_eResult );

#ifdef DEBLOB_COMMUNITY_PATCH
	// Dev switches: "-mute" and "-background" create FMOD with FMOD_OUTPUTTYPE_NOSOUND (nothing reaches the speakers,
	// events, streams and timing keep working); "-devaudio" keeps the real output
	static TBOOL IsMuteMode();

	// -gamedir: FMOD reads its files (.fev, .fsb, streams) through TFileManager, so they fall back to the game folder
	FMOD_RESULT SetTFileSystem( TINT a_iBlockAlign );
#endif

private:
	// Platform specific (TSound_Win.cpp)
	static TBOOL InitialiseMemory( void* a_pPoolMem, TINT a_iPoolSize );
	static TBOOL InitialiseFileSystem();
	static TBOOL InitialiseNetwork();
	static TBOOL UpdateNetwork();
	static TBOOL DestroyNetwork();
	TBOOL        Initialise( TINT a_iMaxChannels, SpeakerType a_eSpeakerType );

private:
	FMOD::EventSystem* m_pEventSystem; // 0x00
	FMOD::System*      m_pSystem;      // 0x04
	TBOOL              m_bInitialised; // 0x08
	TBOOL              m_bUnknown;     // 0x09 cleared by the ctor, never used
	SoundInitValues    m_oInitValues;  // 0x0C
};

TOSHI_NAMESPACE_END
