#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Math/TVector4.h>

namespace FMOD
{
class Sound;
class Channel;
} // namespace FMOD

namespace Toshi
{
class TTRB;
class TAnimation;
class TSkeletonInstance;
class TModelInstance;
} // namespace Toshi

class ABaseEntity;

//-----------------------------------------------------------------------------
// AStreamingSoundPlayer (0x3C): plays the streamed sounds of the game outside of the FMOD events: the music preview of
// the track list ("TrackSelection.fsb") and the voice overs of the challenges ("Challenge_VO_Streamed.fsb",
// Data/Sound/EN/). A stream is an opened FSB (Stream, 2 of them), its sub sounds are played through voices (Voice, 256
// of them, one per sub sound) that sync an animation with the amplitude of the sound (lip sync).
//-----------------------------------------------------------------------------
class AStreamingSoundPlayer : public Toshi::TSingleton<AStreamingSoundPlayer>
{
public:
	static TCONSTEXPR_VAR TINT MAX_VOICES     = 256;
	static TCONSTEXPR_VAR TINT MAX_STREAMS    = 2;
	static TCONSTEXPR_VAR TINT MAX_VARIATIONS = 8;

	// Voice::m_uiFlags
	enum VOICE_FLAGS : TUINT16
	{
		VOICE_LOAD         = 0x0001, // wait for the sound, then get its sub sound and create the channel
		VOICE_PLAY         = 0x0002, // start the channel once it's ready
		VOICE_GET_SUBSOUND = 0x0004, // the sub sound isn't fetched yet (Stream::FetchSubSound)
		VOICE_GET_NAME     = 0x0008, // the name isn't fetched yet
		VOICE_STARTED      = 0x0010, // the channel was unpaused
		VOICE_READY        = 0x0020, // the channel is created (paused)
		VOICE_PLAYING      = 0x0040, // the channel is playing: the voice is released once it's finished
		VOICE_SUBSOUND     = 0x0200, // the channel is created
		VOICE_HAVE_SOUND   = 0x0400, // the sub sound is fetched
		VOICE_LIPSYNC      = 0x0800, // m_pSkeletonInstance/m_iAnimSequence: an animation follows the amplitude
		VOICE_PAUSED_A     = 0x1000, // paused by SetPaused( TTRUE, TFALSE )
		VOICE_PAUSED_B     = 0x2000, // paused by SetPaused( TTRUE, TTRUE )
		VOICE_3D           = 0x0100, // the channel is positioned at m_vPosition
	};

	// Stream::m_uiFlags
	enum STREAM_FLAGS : TUINT16
	{
		STREAM_OPEN_SUBSOUNDS = 0x0001, // the number of sub sounds isn't known yet
		STREAM_OPEN_VOICES    = 0x0004, // the voices of the sub sounds are being created
		STREAM_LOOP           = 0x0010, // FMOD_LOOP_NORMAL
		STREAM_FADE_OUT       = 0x0020, // fade out the voices (Stream::m_fFadeTime)
		STREAM_FADING         = 0x0040, // fading out
		STREAM_HOLD           = 0x0080, // don't start the voices until every stream is opened
		STREAM_ERROR          = 0x0100, // the stream couldn't be opened
		STREAM_MUSIC          = 0x0200, // music volume instead of the sound effects volume
	};

	// The amplitude envelope of a voice over line (0x14, the level's VOMapping.trb)
	struct VOSample
	{
		TFLOAT  fInterval;  // 0x00 time between two values
		TINT    iLength;    // 0x04 length of the line
		TINT    iNumValues; // 0x08
		TINT    iSubSound;  // 0x0C index of the sub sound in the FSB, -1: none
		TINT16* pValues;    // 0x10 the amplitudes
	};

	// 0x10
	struct VOEntry
	{
		TINT      iID;         // 0x00
		TINT      iNameIndex;  // 0x04 index into VOMapping::ppszNames
		TINT      iNumSamples; // 0x08
		VOSample* pSamples;    // 0x0C
	};

	// The "VOMapping" symbol of VOMapping.trb (0x14)
	struct VOMapping
	{
		VOEntry*      pEntries;    // 0x00
		TINT          iNumEntries; // 0x04
		TINT          iUnk08;      // 0x08
		const TCHAR** ppszNames;   // 0x0C
		TINT          iNumNames;   // 0x10
	};

	class Stream;

	// 0x60: one sub sound of a stream
	class Voice : public Toshi::T2DList<Voice>::Node
	{
	public:
		Voice();

		void  Reset();
		void  AcquireSubSound( Stream* a_pStream );
		TBOOL Start();
		TBOOL BeginPlay();
		TBOOL Stop();
		TBOOL Free();
		void  UpdateLipSync();

	public:
		TCHAR                     m_szName[ 36 ];      // 0x08
		Toshi::TVector4           m_vPosition;         // 0x2C
		Stream*                   m_pStream;           // 0x3C
		FMOD::Sound*              m_pSound;            // 0x40 the sub sound
		FMOD::Channel*            m_pChannel;          // 0x44
		const VOSample*           m_pSample;           // 0x48 the amplitudes of the line (lip sync)
		Toshi::TAnimation*        m_pAnimation;        // 0x4C
		Toshi::TSkeletonInstance* m_pSkeletonInstance; // 0x50
		TINT                      m_iAnimSequence;     // 0x54 -1: none
		TUINT16                   m_uiFlags;           // 0x58
		TINT16                    m_iSubSound;         // 0x5A -1: none
		TINT16                    m_iVOID;             // 0x5C -1: none
		TINT16                    m_iVariant;          // 0x5E
	};

	// 0x10: a variation of an event of a stream (the voice to play)
	class Variation : public Toshi::T2DList<Variation>::Node
	{
	public:
		Variation();

		void Reset();

	public:
		TINT16  m_iEvent;      // 0x08
		TINT16  m_iUnk0A;      // 0x0A
		TINT16  m_iVoiceIndex; // 0x0C index of the voice in the stream
		TUINT16 m_uiFlags;     // 0x0E 1: played last
	};

	// 0x70: an opened FSB
	class Stream : public Toshi::T2DList<Stream>::Node
	{
	public:
		Stream();
		~Stream();

		void  Reset();
		TBOOL ReleaseSound();
		void  StopVoices( Voice* a_pExcept );
		void  StopVoicesWithout( TUINT16 a_uiKeepMask );
		TINT  CountVoices( TUINT16 a_uiMask );
		void  DebugCheckVoices();
		TBOOL Open( const TCHAR* a_szPath, TUINT16 a_uiFlags );
		TBOOL InitEventGroup( TINT16 a_iGroup, Toshi::T2DList<Variation>* a_pFreeVariations );
		TBOOL IsFile( const TCHAR* a_szFile );
		TBOOL Update( Toshi::T2DList<Voice>* a_pFreeVoices, VOMapping* a_pVOMapping );
		TBOOL FetchSubSound( Voice* a_pVoice, TINT a_iIndex );

	public:
		TCHAR                     m_szName[ 64 ];  // 0x08
		Toshi::T2DList<Voice>     m_Voices;        // 0x48
		Toshi::T2DList<Variation> m_Variations;    // 0x50
		FMOD::Sound*              m_pSound;        // 0x58
		TFLOAT                    m_fFadeTime;     // 0x5C -10 when idle
		TINT16                    m_iNumSubSounds; // 0x60 -1: unknown
		TUINT16                   m_uiFlags;       // 0x62
		TINT16                    m_iUnk64;        // 0x64
		TINT16                    m_iEventGroup;   // 0x66 -1: none
		void*                     m_pUnk68;        // 0x68
		Voice*                    m_pPendingVoice; // 0x6C the voice whose sub sound is being fetched
	};

public:
	AStreamingSoundPlayer();
	~AStreamingSoundPlayer();

	TBOOL AreStreamsReady();
	void  Tick( TFLOAT a_fDeltaTime );

	// $deBlob: FUNCTION 00615c80
	void Pause() { SetPaused( TTRUE, TFALSE ); }

	// $deBlob: FUNCTION 00615c90
	void Resume() { SetPaused( TFALSE, TFALSE ); }

	TBOOL AreStreamsOK();

	Voice*  GetVoice( const TCHAR* a_szStream, TINT16 a_iIndex );
	Stream* LoadTrackSelection();
	TBOOL   LoadChallengeVO( TINT a_iUnused );

	// $deBlob: FUNCTION 006161b0
	void ClearVOMapping() { m_pTRBStreamingVOMappings = TNULL; }


	Stream* OpenStream( const TCHAR* a_szFile, TINT16 a_uiFlags );
	TBOOL   OpenEventGroupStream( TINT16 a_iGroup );

	// NOTE: unused stubs
	// $deBlob: FUNCTION 006164e0
	TBOOL Unk6164E0( TINT a_iUnused ) { return TTRUE; }

	// $deBlob: FUNCTION 006164f0
	TBOOL Unk6164F0( TINT a_iUnused ) { return TTRUE; }

	const TCHAR* GetVOName( TINT a_iID );

	void StopAll();

	// Voice overs by id and variant
	TBOOL LoadVO( TINT a_iID, TINT a_iVariant );
	TBOOL PlayVO( TINT a_iID, TINT a_iVariant );
	TBOOL PlayVO( TINT a_iID, TINT a_iVariant, ABaseEntity* a_pEntity, const TCHAR* a_szAnimation );
	TBOOL PlayVO( TINT a_iID, TINT a_iVariant, Toshi::TModelInstance* a_pModelInstance, const TCHAR* a_szAnimation );

	void  FadeOut();
	TBOOL IsPlaying( const TCHAR* a_szName );
	TBOOL Load( const TCHAR* a_szName, TINT a_iUnused );
	TBOOL Play( const TCHAR* a_szName, TUINT8 a_uiFlags );

	// Variations of an event of an event group
	TBOOL LoadEvent( TINT16 a_iGroup, TINT a_iEvent );
	TBOOL PlayEventByName( TINT a_iGroup, const TCHAR* a_szEvent );
	TBOOL PlayEvent( TINT16 a_iGroup, TINT a_iEvent );
	TBOOL PlayEventByName2( TINT a_iGroup, const TCHAR* a_szEvent );
	TBOOL StopEvent( TINT16 a_iGroup, TINT16 a_iEvent );
	TBOOL StopEventByName( TINT a_iGroup, const TCHAR* a_szEvent );

	TBOOL ResetStream( Stream* a_pStream );
	TBOOL SetPaused( TBOOL a_bPaused, TBOOL a_bSecondary );
	TBOOL ResetAll();

	Voice* FindVoice( TINT a_iID, TINT a_iVariant, Stream** a_ppStream );
	Voice* FindVoice( const TCHAR* a_szName, Stream** a_ppStream );
	Voice* FindVariation( TINT16 a_iGroup, TINT16 a_iEvent, Stream** a_ppStream );
	Voice* FindVariationByName( TINT a_iGroup, const TCHAR* a_szEvent, Stream** a_ppStream );

	static TBOOL RequestLoad( Voice* a_pVoice );
	static TBOOL RequestPlay( Voice* a_pVoice );

	// The streams that are open
	Toshi::T2DList<Stream>& GetStreams() { return m_Streams; }

private:
	TUINT32                   m_uiFlags;                 // 0x00 1 and 2: paused (SetPaused)
	TUINT32                   m_uiUnk04;                 // 0x04
	Toshi::TTRB*              m_pTRBStreamingVOMappings; // 0x08 VOMapping.trb
	VOMapping*                m_pVOMapping;              // 0x0C
	Toshi::T2DList<Voice>     m_FreeVoices;              // 0x10
	Toshi::T2DList<Variation> m_FreeVariations;          // 0x18
	Toshi::T2DList<Stream>    m_Streams;                 // 0x20 the opened streams
	Toshi::T2DList<Stream>    m_FreeStreams;             // 0x28
	Variation*                m_aVariations;             // 0x30 MAX_VARIATIONS
	Voice*                    m_aVoices;                 // 0x34 MAX_VOICES
	Stream*                   m_aStreams;                // 0x38 MAX_STREAMS
};
