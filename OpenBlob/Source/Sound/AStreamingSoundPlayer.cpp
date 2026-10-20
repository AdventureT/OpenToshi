#include "pch.h"
#include "AStreamingSoundPlayer.h"
#include "ASoundInterface.h"
#include "ASoundMap.h"
#include "ACurrentLevelInformation.h"
#include "ARandom.h"
#include "Options/AOptions.h"
#include "Entities/ABaseEntity.h"
#include "Entities/AEntityModelComponent.h"
#include "Animation/AModelInstance2.h"

#include <Sound/TSound.h>
#include <File/TTRB.h>
#include <Render/TAnimation.h>
#include <Render/TSkeleton.h>
#include <Render/TModel.h>
#include <Toshi/T2String8.h>
#include <Math/TMath.h>
#include <Math/TMathInline.h>

#include <fmod.hpp>
#include <fmod_event.hpp>

//-----------------------------------------------------------------------------
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AStreamingSoundPlayer::Voice ) == 0x60 );
TSTATICASSERT( sizeof( AStreamingSoundPlayer::Stream ) == 0x70 );
TSTATICASSERT( sizeof( AStreamingSoundPlayer::Variation ) == 0x10 );
TSTATICASSERT( sizeof( AStreamingSoundPlayer ) == 0x3C );
TSTATICASSERT( ACurrentLevelInformation::DataBlock_VOMapping == 21 );
TSTATICASSERT( FMOD_ERR_FILE_DISKEJECTED == 21 );
TSTATICASSERT( FMOD_ERR_MEMORY == 43 );
TSTATICASSERT( FMOD_EVENT_INFOONLY == 4 );

// The fade out time of a stream in seconds (dword_97E228)
static TCONSTEXPR_VAR TFLOAT FADE_OUT_TIME = 0.25f;

// FMOD_NONBLOCKING | FMOD_CREATESTREAM | FMOD_HARDWARE | FMOD_2D | FMOD_LOOP_OFF / FMOD_LOOP_NORMAL (0x100a9, 0x100aa)
static TCONSTEXPR_VAR FMOD_MODE STREAM_MODE      = 0x100a9;
static TCONSTEXPR_VAR FMOD_MODE STREAM_MODE_LOOP = 0x100aa;

// The sound files of the track list's sub sounds (off_97E1F4, TrackSelection.fsb)
static const TCHAR* const s_aszTrackNames[ 13 ] = {
	"M01_Funk1.wav",
	"M02_Funk2.wav",
	"M03_Funk3.wav",
	"M04_Funk4.wav",
	"M05_lounge.wav",
	"M06_Orleans.wav",
	"M07_DubSka.wav",
	"M08_Cuban.wav",
	"M09_Samba.wav",
	"M10_HipHop.wav",
	"M11_Disco.wav",
	"M12_Boss_Remix.wav",
	"Track_Locked_static.wav",
};

//-----------------------------------------------------------------------------
// AStreamingSoundPlayer::Variation
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006143c0
AStreamingSoundPlayer::Variation::Variation()
{
	m_iEvent      = -1;
	m_iUnk0A      = -1;
	m_iVoiceIndex = -1;
	m_uiFlags     = 0;
}

// $deBlob: FUNCTION 006143e0
void AStreamingSoundPlayer::Variation::Reset()
{
	m_iEvent      = -1;
	m_iUnk0A      = -1;
	m_iVoiceIndex = -1;
	m_uiFlags     = 0;
}

//-----------------------------------------------------------------------------
// AStreamingSoundPlayer::Stream
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00614400
AStreamingSoundPlayer::Stream::Stream()
{
	// NOTE: m_uiFlags is left uninitialised, like retail (set by Open)
	m_pSound        = TNULL;
	m_iNumSubSounds = -1;
	T2String8::Format( m_szName, "" );
	m_iEventGroup   = -1;
	m_fFadeTime     = -10.0f;
	m_iUnk64        = 0;
	m_pUnk68        = TNULL;
	m_pPendingVoice = TNULL;
}

// $deBlob: FUNCTION 006144a0
// $deBlob: FUNCTION 00617b70
AStreamingSoundPlayer::Stream::~Stream()
{
}

// $deBlob: FUNCTION 00614540
void AStreamingSoundPlayer::Stream::DebugCheckVoices()
{
	// NOTE: retail walks the voices doing nothing (the compiler kept the iterator asserts)
	for ( auto it = m_Voices.Begin(); it != m_Voices.End(); it++ )
	{
	}
}

// $deBlob: FUNCTION 006145f0
void AStreamingSoundPlayer::Stream::Reset()
{
	m_pSound        = TNULL;
	m_iNumSubSounds = -1;
	T2String8::Format( m_szName, "" );
	m_iEventGroup   = -1;
	m_fFadeTime     = -10.0f;
	m_iUnk64        = 0;
	m_pUnk68        = TNULL;
	m_pPendingVoice = TNULL;
}

// $deBlob: FUNCTION 00614630
TBOOL AStreamingSoundPlayer::Stream::ReleaseSound()
{
	if ( m_pSound != TNULL )
		m_pSound->release();

	m_pSound = TNULL;
	return TTRUE;
}

// Stops the voices but a_pExcept
// $deBlob: FUNCTION 00614650
void AStreamingSoundPlayer::Stream::StopVoices( Voice* a_pExcept )
{
	for ( auto it = m_Voices.Begin(); it != m_Voices.End(); it++ )
	{
		if ( a_pExcept != it )
			it->Stop();
	}
}

// Reads the info of an event (FMOD_EVENT_INFOONLY) and drops it, like AMusicManager::GetMusicSFXLength
// $deBlob: FUNCTION 00614740
FMOD_RESULT GetEventInfo( ASoundMap::GROUPID a_eGroupID, ASoundMap::EVENTID a_eEventID, TINT a_iUnused )
{
	FMOD::Event*    pEvent = ASoundInterface::GetEvent( ASoundMap::GROUPID( TINT16( a_eGroupID ) ), a_eEventID, FMOD_EVENT_INFOONLY );
	FMOD_EVENT_INFO oInfo  = {};
	return pEvent->getInfo( TNULL, TNULL, &oInfo );
}

// $deBlob: FUNCTION 006147c0
void AStreamingSoundPlayer::Stream::StopVoicesWithout( TUINT16 a_uiKeepMask )
{
	for ( auto it = m_Voices.Begin(); it != m_Voices.End(); it++ )
	{
		if ( ( it->m_uiFlags & a_uiKeepMask ) == 0 )
			it->Stop();
	}
}

// $deBlob: FUNCTION 00614870
TINT AStreamingSoundPlayer::Stream::CountVoices( TUINT16 a_uiMask )
{
	TINT iCount = 0;

	for ( auto it = m_Voices.Begin(); it != m_Voices.End(); it++ )
	{
		if ( it->m_uiFlags & a_uiMask )
			iCount++;
	}

	return iCount;
}

// $deBlob: FUNCTION 006148c0
TBOOL AStreamingSoundPlayer::Stream::Open( const TCHAR* a_szPath, TUINT16 a_uiFlags )
{
	if ( a_szPath == TNULL )
		return TFALSE;

	FMOD_MODE eMode = ( a_uiFlags & STREAM_LOOP ) ? STREAM_MODE_LOOP : STREAM_MODE;
	TSound::GetSingleton()->GetSystem()->createStream( a_szPath, eMode, TNULL, &m_pSound );

	m_uiFlags = a_uiFlags | STREAM_OPEN_SUBSOUNDS;
	return TTRUE;
}

// Adds the events of the event group (the first one names the stream)
// NOTE: dead code in retail
// $deBlob: FUNCTION 00614920
TBOOL AStreamingSoundPlayer::Stream::InitEventGroup( TINT16 a_iGroup, T2DList<Variation>* a_pFreeVariations )
{
	FMOD::EventGroup* pEventGroup = ASoundInterface::GetEventGroup( ASoundMap::GROUPID( a_iGroup ), 0 );

	FMOD::Event* pEvent     = TNULL;
	TINT         iNumEvents = -1;

	if ( pEventGroup->getNumEvents( &iNumEvents ) != FMOD_OK )
		return TFALSE;

	m_iEventGroup = a_iGroup;

	for ( TINT i = 0; i < iNumEvents; i++ )
	{
		TINT iEventIndex = 0;
		pEvent           = TNULL;
		pEventGroup->getEventByIndex( i, FMOD_EVENT_INFOONLY, &pEvent );

		FMOD_EVENT_INFO oInfo;
		pEvent->getInfo( &iEventIndex, TNULL, &oInfo );

		if ( i == 0 )
		{
			// NOTE: retail reads the name from FMOD_EVENT_INFO::wavebankinfo, that FMOD never fills (it passes name
			// as null): the file is "Data/Sound/EN/(null)"
			const TCHAR* szName = TREINTERPRETCAST( const TCHAR*, oInfo.wavebankinfo );

			TCHAR szBuffer[ 64 ];
			T2String8::Format( szBuffer, sizeof( szBuffer ), "Data/Sound/EN/%s", szName );
			T2String8::Format( m_szName, "%s.fsb", szName );

			if ( szBuffer != TNULL )
			{
				TSound::GetSingleton()->GetSystem()->createStream( szBuffer, STREAM_MODE, TNULL, &m_pSound );
				m_uiFlags = STREAM_OPEN_SUBSOUNDS;
			}
		}

		TINT16 iEvent = TMath::Int32ToInt16( iEventIndex );

		FMOD::Event* pInfoEvent = ASoundInterface::GetEvent( ASoundMap::GROUPID( a_iGroup ), ASoundMap::EVENTID( iEvent ), FMOD_EVENT_INFOONLY );

		FMOD_EVENT_INFO oInfo2;
		pInfoEvent->getInfo( TNULL, TNULL, &oInfo2 );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00614b50
TBOOL AStreamingSoundPlayer::Stream::IsFile( const TCHAR* a_szFile )
{
	TCHAR szBuffer[ 64 ];
	T2String8::Format( szBuffer, sizeof( szBuffer ), "%s%s", "Data/Sound/EN/", a_szFile );

	return T2String8::Compare( m_szName, szBuffer, -1 ) == 0;
}

// Opens the sound, gets the number of sub sounds and creates the voices of the sub sounds
// Returns TFALSE until the stream is ready
// $deBlob: FUNCTION 00614bd0
TBOOL AStreamingSoundPlayer::Stream::Update( T2DList<Voice>* a_pFreeVoices, VOMapping* a_pVOMapping )
{
	if ( m_pSound == TNULL )
		return TFALSE;

	FMOD_RESULT    eResult = FMOD_OK;
	FMOD_OPENSTATE eState  = FMOD_OPENSTATE_READY;

	if ( ( m_uiFlags & ( STREAM_OPEN_SUBSOUNDS | 2 | STREAM_OPEN_VOICES ) ) == 0 )
		return TTRUE;

	while ( TTRUE )
	{
		if ( eResult != FMOD_OK || ( eState != FMOD_OPENSTATE_READY && eState != FMOD_OPENSTATE_PLAYING ) )
			return TTRUE;

		unsigned int uiPercentBuffered;
		bool         bStarving;
		bool         bDiskBusy = false;
		eResult                = m_pSound->getOpenState( &eState, &uiPercentBuffered, &bStarving, &bDiskBusy );

		if ( eResult == FMOD_ERR_MEMORY )
		{
			TSound::ErrorCheck( FMOD_ERR_MEMORY );
			m_uiFlags = ( m_uiFlags & 0xFEFE ) | STREAM_ERROR;
			return TFALSE;
		}

		if ( eResult != FMOD_OK )
		{
			if ( eResult == FMOD_ERR_FILE_DISKEJECTED )
			{
				// The disc was ejected: open the stream again
				ReleaseSound();

				TCHAR szBuffer[ 64 ];
				T2String8::Format( szBuffer, sizeof( szBuffer ), "Data/Sound/EN/%s", m_szName );
				Open( szBuffer, m_uiFlags );
			}

			return TFALSE;
		}

		if ( eState != FMOD_OPENSTATE_READY && eState != FMOD_OPENSTATE_PLAYING )
			return TFALSE;

		if ( m_uiFlags & STREAM_OPEN_SUBSOUNDS )
		{
			TINT iNumSubSounds = 0;
			eResult            = m_pSound->getNumSubSounds( &iNumSubSounds );

			if ( eResult == FMOD_OK )
			{
				m_iNumSubSounds = TMath::Int32ToInt16( iNumSubSounds );
				m_uiFlags &= ~STREAM_OPEN_SUBSOUNDS;
			}
		}
		else if ( m_uiFlags & STREAM_OPEN_VOICES )
		{
			TINT iNumVoices = m_Voices.Size();

			if ( m_pPendingVoice != TNULL )
			{
				if ( FetchSubSound( m_pPendingVoice, m_Voices.Size() - 1 ) )
					m_pPendingVoice = TNULL;
			}
			else if ( iNumVoices == m_iNumSubSounds )
			{
				m_uiFlags &= ~STREAM_OPEN_VOICES;
			}
			else
			{
				if ( a_pFreeVoices->IsEmpty() )
					return TFALSE;

				Voice* pVoice   = a_pFreeVoices->PopFront();
				m_pPendingVoice = pVoice;

				pVoice->Reset();
				pVoice->m_uiFlags = VOICE_GET_SUBSOUND | VOICE_GET_NAME;
				m_Voices.PushBack( pVoice );
			}
		}

		if ( ( m_uiFlags & ( STREAM_OPEN_SUBSOUNDS | 2 | STREAM_OPEN_VOICES ) ) == 0 )
			return TTRUE;
	}
}

// Fetches the sub sound (VOICE_GET_SUBSOUND) or the name (VOICE_GET_NAME) of a voice
// $deBlob: FUNCTION 00614e10
TBOOL AStreamingSoundPlayer::Stream::FetchSubSound( Voice* a_pVoice, TINT a_iIndex )
{
	TUINT16 uiFlags = a_pVoice->m_uiFlags;

	if ( uiFlags & VOICE_GET_SUBSOUND )
	{
		if ( a_iIndex != -1 )
		{
			if ( m_pSound->getSubSound( a_iIndex, &a_pVoice->m_pSound ) == FMOD_OK )
			{
				a_pVoice->m_iSubSound = a_iIndex;
				a_pVoice->m_uiFlags &= ~VOICE_GET_SUBSOUND;
			}

			return TFALSE;
		}

		a_pVoice->m_pSound  = TNULL;
		a_pVoice->m_uiFlags = uiFlags & ~VOICE_GET_SUBSOUND;
		return TTRUE;
	}

	if ( !( uiFlags & VOICE_GET_NAME ) )
		return TTRUE;

	if ( a_iIndex == -1 )
	{
		a_pVoice->m_uiFlags = uiFlags & ~VOICE_GET_NAME;
		return TTRUE;
	}

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail passes 64 for the 36 bytes of m_szName: a longer name overwrites the position and the pointers
	if ( a_pVoice->m_pSound->getName( a_pVoice->m_szName, sizeof( a_pVoice->m_szName ) ) != FMOD_OK )
#else
	if ( a_pVoice->m_pSound->getName( a_pVoice->m_szName, 64 ) != FMOD_OK )
#endif
		return TFALSE;

	a_pVoice->m_uiFlags &= ~VOICE_GET_NAME;
	return TTRUE;
}

//-----------------------------------------------------------------------------
// AStreamingSoundPlayer::Voice
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00614eb0
AStreamingSoundPlayer::Voice::Voice()
{
	Reset();
}

// $deBlob: FUNCTION 00614f00
void AStreamingSoundPlayer::Voice::Reset()
{
	// NOTE: m_pStream isn't reset
	T2String8::Format( m_szName, "" );
	m_vPosition.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	m_pSound            = TNULL;
	m_uiFlags           = 0;
	m_pChannel          = TNULL;
	m_iSubSound         = -1;
	m_iVOID             = -1;
	m_pSample           = TNULL;
	m_pAnimation        = TNULL;
	m_pSkeletonInstance = TNULL;
	m_iAnimSequence     = -1;
}

// $deBlob: FUNCTION 00614f60
void AStreamingSoundPlayer::Voice::AcquireSubSound( Stream* a_pStream )
{
	m_uiFlags |= VOICE_HAVE_SOUND;
	a_pStream->m_pSound->getSubSound( m_iSubSound, &m_pSound );
}

// Creates the (paused) channel of the sub sound
// $deBlob: FUNCTION 00614f90
TBOOL AStreamingSoundPlayer::Voice::Start()
{
	if ( m_pSound == TNULL )
		return TFALSE;

	if ( m_uiFlags & VOICE_READY )
		return TTRUE;

	if ( TSound::GetSingleton()->GetSystem()->playSound( FMOD_CHANNEL_FREE, m_pSound, true, &m_pChannel ) != FMOD_OK )
		return TFALSE;

	FMOD_RESULT ePriorityResult = m_pChannel->setPriority( 10 );
	TASSERT( ePriorityResult == FMOD_OK );

	if ( m_uiFlags & VOICE_3D )
	{
		FMOD_VECTOR vPosition = ASoundInterface::GetFMODVector( m_vPosition );
		FMOD_VECTOR vVelocity = ASoundInterface::GetFMODVector( TVector4( 0.0f, 0.0f, 0.0f, 1.0f ) );

		m_pChannel->set3DAttributes( &vPosition, &vVelocity );
		m_pChannel->set3DMinMaxDistance( 200.0f, 400.0f );
	}

	m_uiFlags = ( m_uiFlags & ~( VOICE_SUBSOUND | VOICE_HAVE_SOUND ) ) | VOICE_SUBSOUND;
	return TTRUE;
}

// Unpauses the channel once it's ready
// $deBlob: FUNCTION 006150e0
TBOOL AStreamingSoundPlayer::Voice::BeginPlay()
{
	if ( m_uiFlags & VOICE_READY )
	{
		bool bPlaying = false;
		m_pChannel->isPlaying( &bPlaying );

		if ( !bPlaying )
		{
			m_uiFlags = ( m_uiFlags & ~VOICE_SUBSOUND ) | VOICE_LOAD;
			return TFALSE;
		}

		m_uiFlags |= VOICE_STARTED;

		TFLOAT fVolume = AOptions::GetSingleton()->GetSFXVolume();

		if ( m_pStream != TNULL && ( m_pStream->m_uiFlags & STREAM_MUSIC ) )
			fVolume = AOptions::GetSingleton()->GetMusicVolume();

		FMOD_RESULT eVolumeResult = m_pChannel->setVolume( fVolume );
		TASSERT( eVolumeResult == FMOD_OK );

		FMOD_RESULT ePausedResult = m_pChannel->setPaused( false );
		TASSERT( ePausedResult == FMOD_OK );
	}
	else
	{
		m_uiFlags |= VOICE_LOAD;
	}

	return TTRUE;
}

// Stops the channel and clears the voice
// $deBlob: FUNCTION 00615200
TBOOL AStreamingSoundPlayer::Voice::Stop()
{
	if ( m_uiFlags & VOICE_LIPSYNC )
	{
		if ( m_pSkeletonInstance != TNULL )
		{
			if ( m_pAnimation != TNULL )
			{
				if ( m_pAnimation->IsActive() )
					m_pSkeletonInstance->RemoveAnimation( m_pAnimation, 0.0f );

				m_pAnimation = TNULL;
			}

			m_pSkeletonInstance = TNULL;
		}

		m_pSkeletonInstance = TNULL;
	}

	m_uiFlags = 0;

	if ( m_pChannel != TNULL )
	{
		m_pChannel->stop();
		m_pChannel = TNULL;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00615270
TBOOL AStreamingSoundPlayer::Voice::Free()
{
	Stop();

	if ( m_pSound != TNULL )
		m_pSound->release();

	m_pSound = TNULL;
	return TTRUE;
}

// Sets the weight of the lip sync animation from the amplitude of the sound at the current position
// $deBlob: FUNCTION 00615290
void AStreamingSoundPlayer::Voice::UpdateLipSync()
{
	if ( m_pChannel == TNULL )
		return;

	const VOSample* pSample = m_pSample;

	if ( pSample == TNULL )
		return;

	unsigned int uiPosition;
	FMOD_RESULT  eResult = m_pChannel->getPosition( &uiPosition, FMOD_TIMEUNIT_MS );

	if ( eResult != FMOD_OK || pSample->iNumValues < 2 )
		return;

	TFLOAT fPosition = TFLOAT( uiPosition );

	// NOTE: retail rounds (fistp) instead of truncating
	TINT iIndex = std::lrintf( ( fPosition / TFLOAT( pSample->iLength ) ) * TFLOAT( pSample->iNumValues ) );

	if ( iIndex >= pSample->iNumValues - 2 )
		iIndex = pSample->iNumValues - 1;

	// NOTE: with iIndex == iNumValues - 1 retail reads the value after the last one
	TINT16 iValue     = pSample->pValues[ iIndex ];
	TINT16 iNextValue = pSample->pValues[ iIndex + 1 ];

	TFLOAT fBlend = 0.0f;
	TFLOAT fLeft  = TFLOAT( iIndex + 1 ) * pSample->fInterval - fPosition;

	if ( fLeft > 0.0f )
		fBlend = fLeft / pSample->fInterval;

	TMath::Clip( fBlend, 0.0f, 1.0f );

	TFLOAT fAmplitude = ( TFLOAT( iNextValue ) / 32767.0f ) * ( 1.0f - fBlend ) + ( TFLOAT( iValue ) / 32767.0f ) * fBlend;
	TFLOAT fWeight    = 0.0f;

	if ( fAmplitude > 0.0f )
	{
		fWeight = TMath::Log10( fAmplitude ) * 0.5f + 1.0f;
		TMath::Clip( fWeight, 0.0f, 1.0f );
	}

	if ( m_iAnimSequence != -1 )
	{
		if ( m_pAnimation != TNULL )
			m_pAnimation->SetWeight( fWeight );
		else
			m_pAnimation = m_pSkeletonInstance->AddAnimation( TINT16( m_iAnimSequence ), fWeight, 0.16f );
	}
}

//-----------------------------------------------------------------------------
// AStreamingSoundPlayer
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00615470
AStreamingSoundPlayer::AStreamingSoundPlayer()
{
	m_uiFlags                 = 0;
	m_uiUnk04                 = 0;
	m_pTRBStreamingVOMappings = TNULL;
	m_aVoices                 = TNULL;
	m_aStreams                = TNULL;

	m_aVoices     = new Voice[ MAX_VOICES ];
	m_aStreams    = new Stream[ MAX_STREAMS ];
	m_aVariations = new Variation[ MAX_VARIATIONS ];

	m_pVOMapping = TNULL;

	for ( TINT i = 0; i < MAX_VOICES; i++ )
		m_FreeVoices.PushBack( &m_aVoices[ i ] );

	for ( TINT i = 0; i < MAX_STREAMS; i++ )
		m_FreeStreams.PushBack( &m_aStreams[ i ] );

	for ( TINT i = 0; i < MAX_VARIATIONS; i++ )
		m_FreeVariations.PushBack( &m_aVariations[ i ] );
}

// $deBlob: FUNCTION 00615670
// $deBlob: FUNCTION 0056c280
AStreamingSoundPlayer::~AStreamingSoundPlayer()
{
	ResetAll();

	{
		auto it = m_FreeVoices.Begin();

		while ( it != m_FreeVoices.End() )
		{
			Voice* pVoice = it++;
			pVoice->Remove();
		}
	}

	{
		auto it = m_FreeVariations.Begin();

		while ( it != m_FreeVariations.End() )
		{
			Variation* pVariation = it++;
			pVariation->Remove();
		}
	}

	{
		auto it = m_FreeStreams.Begin();

		while ( it != m_FreeStreams.End() )
		{
			Stream* pStream = it++;
			pStream->Remove();
		}
	}

	delete[] m_aVariations;
	delete[] m_aVoices;
	delete[] m_aStreams;
}

// TTRUE when no stream is still being opened; releases the streams that were held back (STREAM_HOLD)
// $deBlob: FUNCTION 006158e0
TBOOL AStreamingSoundPlayer::AreStreamsReady()
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TTRUE;

	for ( auto it = m_Streams.Begin(); it != m_Streams.End(); it++ )
	{
		if ( it->m_uiFlags & ( STREAM_OPEN_SUBSOUNDS | 2 | STREAM_OPEN_VOICES ) )
			return TFALSE;

		it->m_uiFlags &= ~STREAM_HOLD;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00615940
void AStreamingSoundPlayer::Tick( TFLOAT a_fDeltaTime )
{
	if ( ( m_uiFlags & 3 ) != 0 )
		return;

	for ( auto itStream = m_Streams.Begin(); itStream != m_Streams.End(); itStream++ )
	{
		Stream* pStream = itStream;

		if ( !pStream->Update( &m_FreeVoices, m_pVOMapping ) )
			continue;

		if ( pStream->m_uiFlags & ( STREAM_FADE_OUT | STREAM_FADING ) )
		{
			if ( pStream->m_uiFlags & STREAM_FADE_OUT )
			{
				pStream->m_fFadeTime = FADE_OUT_TIME;
				pStream->m_uiFlags   = ( pStream->m_uiFlags & ~( STREAM_FADE_OUT | STREAM_FADING ) ) | STREAM_FADING;
			}

			pStream->m_fFadeTime -= a_fDeltaTime;

			TFLOAT fVolume = 0.0f;
			TBOOL  bDone   = TFALSE;

			if ( pStream->m_fFadeTime >= 0.0f )
			{
				TFLOAT fMaxVolume = AOptions::GetSingleton()->GetSFXVolume();

				if ( pStream->m_uiFlags & STREAM_MUSIC )
					fMaxVolume = AOptions::GetSingleton()->GetMusicVolume();

				fVolume = ( pStream->m_fFadeTime / FADE_OUT_TIME ) * fMaxVolume;
			}
			else
			{
				pStream->m_uiFlags &= ~STREAM_FADING;
				bDone = TTRUE;
			}

			TINT iNumFading = 0;

			for ( auto itVoice = pStream->m_Voices.Begin(); itVoice != pStream->m_Voices.End(); itVoice++ )
			{
				if ( itVoice->m_pChannel != TNULL && ( itVoice->m_uiFlags & ( VOICE_STARTED | VOICE_PLAYING ) ) )
				{
					iNumFading++;

					if ( bDone )
						itVoice->Stop();
					else
						itVoice->m_pChannel->setVolume( fVolume );
				}
			}

			if ( iNumFading == 0 )
				pStream->m_uiFlags &= ~STREAM_FADING;
		}
		else if ( !( pStream->m_uiFlags & STREAM_HOLD ) )
		{
			for ( auto itVoice = pStream->m_Voices.Begin(); itVoice != pStream->m_Voices.End(); itVoice++ )
			{
				if ( pStream->m_pSound == TNULL )
					continue;

				FMOD_OPENSTATE eState;
				unsigned int   uiPercentBuffered = 0;
				bool           bStarving;
				bool           bDiskBusy = false;

				if ( pStream->m_pSound->getOpenState( &eState, &uiPercentBuffered, &bStarving, &bDiskBusy ) == FMOD_OK &&
				     ( eState == FMOD_OPENSTATE_READY || eState == FMOD_OPENSTATE_PLAYING ) )
				{
					Voice*  pVoice  = itVoice;
					TUINT16 uiFlags = pVoice->m_uiFlags;

					if ( uiFlags & VOICE_PLAYING )
					{
						bool        bPlaying   = false;
						FMOD_RESULT eIsPlaying = pVoice->m_pChannel->isPlaying( &bPlaying );

						if ( !bPlaying || eIsPlaying != FMOD_OK )
						{
							pVoice->Stop();
						}
						else if ( pVoice->m_uiFlags & VOICE_LIPSYNC )
						{
							pVoice->UpdateLipSync();
						}
					}
					else if ( uiFlags & VOICE_LOAD )
					{
						if ( pVoice->m_uiFlags & VOICE_HAVE_SOUND )
						{
							pVoice->Start();
						}
						else if ( pVoice->m_uiFlags & VOICE_SUBSOUND )
						{
							pVoice->m_uiFlags = ( uiFlags & ~( VOICE_LOAD | VOICE_READY | VOICE_SUBSOUND ) ) | VOICE_READY;
						}
						else
						{
							pVoice->AcquireSubSound( pStream );
						}
					}
					else if ( uiFlags & VOICE_PLAY )
					{
						if ( uiFlags & VOICE_STARTED )
							pVoice->m_uiFlags = ( uiFlags & ~( VOICE_PLAY | VOICE_STARTED | VOICE_READY | VOICE_PLAYING ) ) | VOICE_PLAYING;
						else
							pVoice->BeginPlay();
					}
				}
			}
		}
	}
}

// TFALSE if a stream failed to open (STREAM_ERROR)
// $deBlob: FUNCTION 00615ca0
TBOOL AStreamingSoundPlayer::AreStreamsOK()
{
	for ( auto it = m_Streams.Begin(); it != m_Streams.End(); it++ )
	{
		if ( it->m_uiFlags & STREAM_ERROR )
			return TFALSE;
	}

	return TTRUE;
}

// Returns the voice with the index a_iIndex of the stream a_szStream
// $deBlob: FUNCTION 00615cf0
AStreamingSoundPlayer::Voice* AStreamingSoundPlayer::GetVoice( const TCHAR* a_szStream, TINT16 a_iIndex )
{
	Stream* pStream = TNULL;

	for ( auto it = m_Streams.Begin(); it != m_Streams.End(); it++ )
	{
		if ( T2String8::CompareNoCase( a_szStream, it->m_szName, -1 ) == 0 )
		{
			pStream = it;
			break;
		}
	}

	if ( pStream == TNULL )
		return TNULL;

	auto itVoice = pStream->m_Voices.Begin();

	if ( itVoice == pStream->m_Voices.End() )
		return TNULL;

	for ( TINT i = 0; i != a_iIndex; i++ )
	{
		itVoice++;

		if ( itVoice == pStream->m_Voices.End() )
			return TNULL;
	}

	return itVoice;
}

// Opens the sounds of the track list: a voice for each of the 13 tracks
// $deBlob: FUNCTION 00615e00
AStreamingSoundPlayer::Stream* AStreamingSoundPlayer::LoadTrackSelection()
{
	Stream* pStream = OpenStream( "TrackSelection.fsb", STREAM_LOOP );

	pStream->m_iNumSubSounds = 13;
	pStream->m_uiFlags |= STREAM_MUSIC | STREAM_HOLD;

	for ( TINT i = 0; i < pStream->m_iNumSubSounds; i++ )
	{
		if ( !m_FreeVoices.IsEmpty() )
		{
			Voice* pVoice = m_FreeVoices.PopFront();

			pVoice->Reset();
			pVoice->m_pStream = pStream;
			pVoice->m_uiFlags = VOICE_GET_SUBSOUND;
			T2String8::Copy( pVoice->m_szName, s_aszTrackNames[ i ], -1 );
			pVoice->m_iSubSound = TMath::Int32ToInt16( i );

			pStream->m_Voices.PushBack( pVoice );
		}
	}

	return pStream;
}

// Opens the voice overs of the level's challenge: a voice for each line of the level's VOMapping.trb
// $deBlob: FUNCTION 00615f60
TBOOL AStreamingSoundPlayer::LoadChallengeVO( TINT a_iUnused )
{
	ResetAll();

	m_pTRBStreamingVOMappings = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_VOMapping );
	TASSERT( m_pTRBStreamingVOMappings != TNULL );

	m_pVOMapping = TSTATICCAST( VOMapping, m_pTRBStreamingVOMappings->GetSymbolAddress( "VOMapping" ) );

	Stream* pStream = OpenStream( "Challenge_VO_Streamed.fsb", 1 );
	pStream->m_uiFlags |= STREAM_HOLD;

	for ( TINT iEntry = 0; iEntry < m_pVOMapping->iNumEntries; iEntry++ )
	{
		VOEntry* pEntry = &m_pVOMapping->pEntries[ iEntry ];

		for ( TINT iSample = 0; iSample < pEntry->iNumSamples; iSample++ )
		{
			if ( m_FreeVoices.IsEmpty() )
				continue;

			VOSample* pSample = &pEntry->pSamples[ iSample ];

			if ( pSample->iSubSound == -1 )
				continue;

			Voice* pVoice = m_FreeVoices.PopFront();

			pVoice->Reset();
			pVoice->m_pStream = pStream;
			pVoice->m_uiFlags = VOICE_GET_SUBSOUND;
			pVoice->m_pSample = pSample;

			pVoice->m_iSubSound = TMath::Int32ToInt16( pSample->iSubSound );
			pVoice->m_iVOID     = TMath::Int32ToInt16( pEntry->iID );
			pVoice->m_iVariant  = TINT16( iSample + 1 );

			pStream->m_Voices.PushBack( pVoice );
		}
	}

	return TTRUE;
}

// Opens the stream a_szFile (Data/Sound/EN/) or returns it if it's open
// $deBlob: FUNCTION 006161c0
AStreamingSoundPlayer::Stream* AStreamingSoundPlayer::OpenStream( const TCHAR* a_szFile, TINT16 a_uiFlags )
{
	for ( auto it = m_Streams.Begin(); it != m_Streams.End(); it++ )
	{
		if ( T2String8::Compare( it->m_szName, a_szFile, 0x40 ) == 0 )
			return it;
	}

	TCHAR szPath[ 64 ];
	T2String8::Format( szPath, sizeof( szPath ), "Data/Sound/EN/%s", a_szFile );

	if ( !m_FreeStreams.IsEmpty() )
	{
		Stream* pStream = m_FreeStreams.PopFront();

		pStream->Reset();
		T2String8::Copy( pStream->m_szName, a_szFile, 0x40 );

		if ( pStream->Open( szPath, a_uiFlags ) )
		{
			m_Streams.PushFront( pStream );
			return pStream;
		}

		m_FreeStreams.PushBack( pStream );
	}

	return TNULL;
}

// NOTE: dead code in retail
// $deBlob: FUNCTION 006163b0
TBOOL AStreamingSoundPlayer::OpenEventGroupStream( TINT16 a_iGroup )
{
	for ( auto it = m_Streams.Begin(); it != m_Streams.End(); it++ )
	{
		if ( it->m_iEventGroup == a_iGroup )
			return TFALSE;
	}

	if ( m_FreeStreams.IsEmpty() )
		return TFALSE;

	Stream* pStream = m_FreeStreams.PopFront();

	pStream->Reset();
	pStream->InitEventGroup( a_iGroup, &m_FreeVariations );

	m_Streams.PushBack( pStream );
	return TTRUE;
}

// The name of the voice over line a_iID (VOMapping::ppszNames)
// $deBlob: FUNCTION 00616500
const TCHAR* AStreamingSoundPlayer::GetVOName( TINT a_iID )
{
	if ( m_pVOMapping == TNULL )
		return TNULL;

	for ( TINT i = 0; i < m_pVOMapping->iNumEntries; i++ )
	{
		VOEntry* pEntry = &m_pVOMapping->pEntries[ i ];

		if ( pEntry->iID == a_iID && pEntry->iNameIndex < m_pVOMapping->iNumNames )
			return m_pVOMapping->ppszNames[ pEntry->iNameIndex ];
	}

	return TNULL;
}

// Stops the voices of every stream
// $deBlob: FUNCTION 00616560
void AStreamingSoundPlayer::StopAll()
{
	for ( auto it = m_Streams.Begin(); it != m_Streams.End(); it++ )
		it->StopVoices( TNULL );
}

// $deBlob: FUNCTION 00616610
TBOOL AStreamingSoundPlayer::LoadVO( TINT a_iID, TINT a_iVariant )
{
	Stream* pStream = TNULL;
	Voice*  pVoice  = FindVoice( a_iID, a_iVariant, &pStream );

	if ( pVoice == TNULL || pVoice->m_iSubSound == -1 || pStream == TNULL )
		return TFALSE;

	pStream->StopVoices( pVoice );
	pVoice->m_uiFlags |= VOICE_LOAD;
	return TTRUE;
}

// $deBlob: FUNCTION 00616660
TBOOL AStreamingSoundPlayer::PlayVO( TINT a_iID, TINT a_iVariant )
{
	Stream* pStream = TNULL;
	Voice*  pVoice  = FindVoice( a_iID, a_iVariant, &pStream );

	if ( pVoice == TNULL || pVoice->m_iSubSound == -1 || pStream == TNULL )
		return TTRUE;

	pStream->StopVoices( pVoice );
	return RequestPlay( pVoice );
}

// $deBlob: FUNCTION 006166b0
TBOOL AStreamingSoundPlayer::PlayVO( TINT a_iID, TINT a_iVariant, ABaseEntity* a_pEntity, const TCHAR* a_szAnimation )
{
	AEntityModelComponent* pModelComponent = TSTATICCAST( AEntityModelComponent, a_pEntity->GetModelComponent() );

	return PlayVO( a_iID, a_iVariant, pModelComponent->GetModelInstance()->GetModelInstance(), a_szAnimation );
}

// Plays the voice over line and lip syncs the animation a_szAnimation of the model
// $deBlob: FUNCTION 006166e0
TBOOL AStreamingSoundPlayer::PlayVO( TINT a_iID, TINT a_iVariant, TModelInstance* a_pModelInstance, const TCHAR* a_szAnimation )
{
	TBOOL   bResult = TFALSE;
	Stream* pStream = TNULL;
	Voice*  pVoice  = FindVoice( a_iID, a_iVariant, &pStream );

	if ( pVoice != TNULL && pVoice->m_iSubSound != -1 && pStream != TNULL )
	{
		pStream->StopVoices( pVoice );
		bResult = RequestPlay( pVoice );

		if ( bResult && a_szAnimation != TNULL )
		{
			TSkeletonInstance* pSkeletonInstance = a_pModelInstance->GetSkeletonInstance();
			TINT               iSequence         = pSkeletonInstance->GetSkeleton()->GetSequenceID( a_szAnimation, -1 );

			if ( iSequence != -1 )
			{
				pVoice->m_pSkeletonInstance = pSkeletonInstance;
				pVoice->m_uiFlags |= VOICE_LIPSYNC;
				pVoice->m_iAnimSequence = iSequence;
				return bResult;
			}

			pVoice->m_pSkeletonInstance = TNULL;
			pVoice->m_uiFlags &= ~VOICE_LIPSYNC;
			pVoice->m_iAnimSequence = -1;
		}
	}

	return bResult;
}

// Fades out the streams that play something
// $deBlob: FUNCTION 00616790
void AStreamingSoundPlayer::FadeOut()
{
	for ( auto it = m_Streams.Begin(); it != m_Streams.End(); it++ )
	{
		Stream* pStream = it;

		if ( pStream->CountVoices( VOICE_STARTED | VOICE_PLAYING ) > 0 )
		{
			pStream->StopVoicesWithout( VOICE_STARTED | VOICE_PLAYING );
			pStream->m_uiFlags = ( pStream->m_uiFlags & ~( STREAM_FADE_OUT | STREAM_FADING ) ) | STREAM_FADE_OUT;
		}
	}
}

// TTRUE if the voice a_szName is playing
// $deBlob: FUNCTION 00616860
TBOOL AStreamingSoundPlayer::IsPlaying( const TCHAR* a_szName )
{
	Voice* pVoice = FindVoice( a_szName, TNULL );

	return pVoice != TNULL && ( pVoice->m_uiFlags & VOICE_PLAYING );
}

// $deBlob: FUNCTION 00616890
TBOOL AStreamingSoundPlayer::Load( const TCHAR* a_szName, TINT a_iUnused )
{
	Stream* pStream = TNULL;
	Voice*  pVoice  = FindVoice( a_szName, &pStream );

	if ( pVoice == TNULL || pStream == TNULL )
		return TFALSE;

	pStream->StopVoices( pVoice );
	pVoice->m_uiFlags |= VOICE_LOAD;
	return TTRUE;
}

// Plays the voice a_szName; with 0x20 in a_uiFlags the voices that play fade out
// $deBlob: FUNCTION 006168e0
TBOOL AStreamingSoundPlayer::Play( const TCHAR* a_szName, TUINT8 a_uiFlags )
{
	Stream* pStream = TNULL;
	Voice*  pVoice  = FindVoice( a_szName, &pStream );

	if ( pVoice == TNULL || pStream == TNULL )
		return TFALSE;

	if ( a_uiFlags & 0x20 )
	{
		if ( pStream->CountVoices( VOICE_STARTED | VOICE_PLAYING ) > 0 )
		{
			pStream->StopVoicesWithout( VOICE_STARTED | VOICE_PLAYING );
			pStream->m_uiFlags = ( pStream->m_uiFlags & ~( STREAM_FADE_OUT | STREAM_FADING ) ) | STREAM_FADE_OUT;
		}
		else
		{
			pStream->StopVoicesWithout( 0 );
		}
	}
	else
	{
		pStream->StopVoices( pVoice );
	}

	RequestPlay( pVoice );
	return TTRUE;
}

// $deBlob: FUNCTION 00616980
TBOOL AStreamingSoundPlayer::LoadEvent( TINT16 a_iGroup, TINT a_iEvent )
{
	Stream* pStream = TNULL;
	Voice*  pVoice  = FindVariation( a_iGroup, TINT16( ASoundMap::scm_iEventIDs[ TINT16( a_iEvent ) ] ), &pStream );

	if ( pVoice != TNULL && pStream != TNULL )
	{
		pStream->StopVoices( pVoice );
		pVoice->m_uiFlags |= VOICE_LOAD;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006169d0
TBOOL AStreamingSoundPlayer::PlayEventByName( TINT a_iGroup, const TCHAR* a_szEvent )
{
	TINT16 iGroup = TINT16( a_iGroup );

	FMOD::EventGroup* pEventGroup = ASoundInterface::GetEventGroup( ASoundMap::GROUPID( iGroup ), 0 );

	if ( pEventGroup == TNULL )
		return TFALSE;

	FMOD::Event* pEvent = TNULL;

	if ( pEventGroup->getEvent( a_szEvent, FMOD_EVENT_INFOONLY, &pEvent ) != FMOD_OK )
		return TFALSE;

	TINT iEvent = 0;
	pEvent->getInfo( &iEvent, TNULL, TNULL );

	Stream* pStream = TNULL;
	Voice*  pVoice  = FindVariation( iGroup, TINT16( iEvent ), &pStream );

	if ( pVoice == TNULL || pStream == TNULL )
		return TFALSE;

	pStream->StopVoices( pVoice );

	RequestPlay( pVoice );
	return TTRUE;
}

// $deBlob: FUNCTION 00616a80
TBOOL AStreamingSoundPlayer::PlayEvent( TINT16 a_iGroup, TINT a_iEvent )
{
	Stream* pStream = TNULL;
	Voice*  pVoice  = FindVariation( a_iGroup, TINT16( ASoundMap::scm_iEventIDs[ TINT16( a_iEvent ) ] ), &pStream );

	if ( pVoice == TNULL || pStream == TNULL )
		return TFALSE;

	pStream->StopVoices( pVoice );

	RequestPlay( pVoice );
	return TTRUE;
}

// NOTE: identical to PlayEventByName
// $deBlob: FUNCTION 00616ae0
TBOOL AStreamingSoundPlayer::PlayEventByName2( TINT a_iGroup, const TCHAR* a_szEvent )
{
	TINT16 iGroup = TINT16( a_iGroup );

	FMOD::EventGroup* pEventGroup = ASoundInterface::GetEventGroup( ASoundMap::GROUPID( iGroup ), 0 );

	if ( pEventGroup == TNULL )
		return TFALSE;

	FMOD::Event* pEvent = TNULL;

	if ( pEventGroup->getEvent( a_szEvent, FMOD_EVENT_INFOONLY, &pEvent ) != FMOD_OK )
		return TFALSE;

	TINT iEvent = 0;
	pEvent->getInfo( &iEvent, TNULL, TNULL );

	Stream* pStream = TNULL;
	Voice*  pVoice  = FindVariation( iGroup, TINT16( iEvent ), &pStream );

	if ( pVoice == TNULL || pStream == TNULL )
		return TFALSE;

	pStream->StopVoices( pVoice );

	RequestPlay( pVoice );
	return TTRUE;
}

// $deBlob: FUNCTION 00616b90
TBOOL AStreamingSoundPlayer::StopEvent( TINT16 a_iGroup, TINT16 a_iEvent )
{
	Voice* pVoice = FindVariation( a_iGroup, a_iEvent, TNULL );

	if ( pVoice != TNULL )
	{
		pVoice->Stop();
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00616bc0
TBOOL AStreamingSoundPlayer::StopEventByName( TINT a_iGroup, const TCHAR* a_szEvent )
{
	TINT16 iGroup = TINT16( a_iGroup );

	FMOD::EventGroup* pEventGroup = ASoundInterface::GetEventGroup( ASoundMap::GROUPID( iGroup ), 0 );

	if ( pEventGroup == TNULL )
		return TFALSE;

	FMOD::Event* pEvent = TNULL;

	if ( pEventGroup->getEvent( a_szEvent, FMOD_EVENT_INFOONLY, &pEvent ) != FMOD_OK )
		return TFALSE;

	TINT iEvent = 0;
	pEvent->getInfo( &iEvent, TNULL, TNULL );

	Voice* pVoice = FindVariation( iGroup, TINT16( iEvent ), TNULL );

	if ( pVoice == TNULL )
		return TFALSE;

	pVoice->Stop();
	return TTRUE;
}

// Gives the voices, the variations and the stream back
// $deBlob: FUNCTION 00616c40
TBOOL AStreamingSoundPlayer::ResetStream( Stream* a_pStream )
{
	{
		auto it = a_pStream->m_Voices.Begin();

		while ( it != a_pStream->m_Voices.End() )
		{
			Voice* pVoice = it++;
			pVoice->Remove();

			pVoice->Reset();
			m_FreeVoices.PushBack( pVoice );
		}
	}

	{
		auto it = a_pStream->m_Variations.Begin();

		while ( it != a_pStream->m_Variations.End() )
		{
			Variation* pVariation = it++;
			pVariation->Remove();

			pVariation->Reset();
			m_FreeVariations.PushBack( pVariation );
		}
	}

	a_pStream->Remove();

	if ( a_pStream->m_pSound != TNULL )
		a_pStream->m_pSound->release();

	a_pStream->m_pSound = TNULL;
	m_FreeStreams.PushBack( a_pStream );
	return TTRUE;
}

// Pauses or unpauses the channels; the pause has two owners (a_bSecondary) that are paused independently
// $deBlob: FUNCTION 00616df0
TBOOL AStreamingSoundPlayer::SetPaused( TBOOL a_bPaused, TBOOL a_bSecondary )
{
	for ( auto itStream = m_Streams.Begin(); itStream != m_Streams.End(); itStream++ )
	{
		for ( auto itVoice = itStream->m_Voices.Begin(); itVoice != itStream->m_Voices.End(); itVoice++ )
		{
			Voice* pVoice = itVoice;

			if ( pVoice->m_pChannel == TNULL )
				continue;

			if ( a_bSecondary )
			{
				if ( a_bPaused )
					pVoice->m_uiFlags |= VOICE_PAUSED_B;
				else
					pVoice->m_uiFlags &= ~VOICE_PAUSED_B;
			}
			else
			{
				if ( a_bPaused )
					pVoice->m_uiFlags |= VOICE_PAUSED_A;
				else
					pVoice->m_uiFlags &= ~VOICE_PAUSED_A;
			}

			// Unpause only when nobody else keeps the voice paused
			if ( a_bPaused == TTRUE || ( pVoice->m_uiFlags & ( VOICE_PAUSED_A | VOICE_PAUSED_B ) ) == 0 )
				pVoice->m_pChannel->setPaused( a_bPaused != TFALSE );
		}
	}

	if ( a_bPaused )
	{
		if ( a_bSecondary )
			m_uiFlags |= 2;
		else
			m_uiFlags |= 1;
	}
	else
	{
		if ( a_bSecondary )
			m_uiFlags &= ~2;
		else
			m_uiFlags &= ~1;
	}

	return TTRUE;
}

// Stops everything and gives the streams back
// $deBlob: FUNCTION 00616fc0
TBOOL AStreamingSoundPlayer::ResetAll()
{
	auto it = m_Streams.Begin();

	while ( it != m_Streams.End() )
	{
		Stream* pStream = it++;

		pStream->StopVoices( TNULL );
		ResetStream( pStream );
	}

	return TTRUE;
}

// Finds the voice over line a_iID, variant a_iVariant
// $deBlob: FUNCTION 00617050
AStreamingSoundPlayer::Voice* AStreamingSoundPlayer::FindVoice( TINT a_iID, TINT a_iVariant, Stream** a_ppStream )
{
	for ( auto itStream = m_Streams.Begin(); itStream != m_Streams.End(); itStream++ )
	{
		for ( auto itVoice = itStream->m_Voices.Begin(); itVoice != itStream->m_Voices.End(); itVoice++ )
		{
			Voice* pVoice = itVoice;

			if ( pVoice != TNULL && pVoice->m_iVariant == TMath::Int32ToInt16( a_iVariant ) && pVoice->m_iVOID == TMath::Int32ToInt16( a_iID ) )
			{
				if ( a_ppStream != TNULL )
					*a_ppStream = itStream;

				return pVoice;
			}
		}
	}

	return TNULL;
}

// Finds the voice a_szName
// $deBlob: FUNCTION 006171e0
AStreamingSoundPlayer::Voice* AStreamingSoundPlayer::FindVoice( const TCHAR* a_szName, Stream** a_ppStream )
{
	for ( auto itStream = m_Streams.Begin(); itStream != m_Streams.End(); itStream++ )
	{
		for ( auto itVoice = itStream->m_Voices.Begin(); itVoice != itStream->m_Voices.End(); itVoice++ )
		{
			if ( T2String8::CompareNoCase( itVoice->m_szName, a_szName, -1 ) == 0 )
			{
				if ( a_ppStream != TNULL )
					*a_ppStream = itStream;

				return itVoice;
			}
		}
	}

	return TNULL;
}

// Picks one of the variations of the event of the event group (not the one that played last)
// $deBlob: FUNCTION 00617360
AStreamingSoundPlayer::Voice* AStreamingSoundPlayer::FindVariation( TINT16 a_iGroup, TINT16 a_iEvent, Stream** a_ppStream )
{
	for ( auto itStream = m_Streams.Begin(); itStream != m_Streams.End(); itStream++ )
	{
		Stream* pStream = itStream;

		if ( a_iGroup != pStream->m_iEventGroup )
			continue;

		Variation* apCandidates[ MAX_VARIATIONS ];
		TINT16     iNumCandidates = 0;
		Variation* pLastPlayed    = TNULL;

		for ( auto itVariation = pStream->m_Variations.Begin(); itVariation != pStream->m_Variations.End(); itVariation++ )
		{
			if ( itVariation->m_iEvent == a_iEvent )
			{
				if ( itVariation->m_uiFlags & 1 )
					pLastPlayed = itVariation;
				else
					apCandidates[ iNumCandidates++ ] = itVariation;
			}
		}

		if ( iNumCandidates <= 0 )
			continue;

		if ( pLastPlayed != TNULL )
			pLastPlayed->m_uiFlags &= ~1;

		TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

		TINT iUpper = iNumCandidates - 1;
		TASSERT( iUpper > 0 && "a_iUpper>a_iLower" );

		// NOTE: retail's TRandom::GetInt( iUpper ) is exclusive: the last candidate is never picked
		Variation* pChosen = apCandidates[ rRandom.GetInt( iUpper ) ];
		pChosen->m_uiFlags |= 1;

		TINT16 iVoiceIndex = pChosen->m_iVoiceIndex;

		auto itVoice = pStream->m_Voices.Begin();

		if ( itVoice == pStream->m_Voices.End() )
			continue;

		TINT i = 0;

		while ( i != iVoiceIndex )
		{
			itVoice++;
			i++;

			if ( itVoice == pStream->m_Voices.End() )
				break;
		}

		if ( itVoice == pStream->m_Voices.End() )
			continue;

		if ( a_ppStream != TNULL )
			*a_ppStream = pStream;

		return itVoice;
	}

	return TNULL;
}

// $deBlob: FUNCTION 00617560
AStreamingSoundPlayer::Voice* AStreamingSoundPlayer::FindVariationByName( TINT a_iGroup, const TCHAR* a_szEvent, Stream** a_ppStream )
{
	TINT16 iGroup = TINT16( a_iGroup );

	FMOD::EventGroup* pEventGroup = ASoundInterface::GetEventGroup( ASoundMap::GROUPID( iGroup ), 0 );

	if ( pEventGroup == TNULL )
		return TNULL;

	FMOD::Event* pEvent = TNULL;

	if ( pEventGroup->getEvent( a_szEvent, FMOD_EVENT_INFOONLY, &pEvent ) != FMOD_OK )
		return TNULL;

	TINT iEvent = 0;
	pEvent->getInfo( &iEvent, TNULL, TNULL );

	return FindVariation( iGroup, TINT16( iEvent ), a_ppStream );
}

// $deBlob: FUNCTION 006175d0
TBOOL AStreamingSoundPlayer::RequestLoad( Voice* a_pVoice )
{
	a_pVoice->m_uiFlags |= VOICE_LOAD;
	return TTRUE;
}

// $deBlob: FUNCTION 006175f0
TBOOL AStreamingSoundPlayer::RequestPlay( Voice* a_pVoice )
{
	if ( a_pVoice->m_uiFlags & VOICE_READY )
		a_pVoice->m_uiFlags |= VOICE_PLAY;
	else
		a_pVoice->m_uiFlags |= VOICE_PLAY | VOICE_LOAD;

	return TTRUE;
}
