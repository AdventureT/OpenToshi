#pragma once
#include "AGUIMusicController.h"

#include <Sound/TSound.h>
#include <File/TTRB.h>
#include <Math/TVector4.h>
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>

class ATRBLoaderJob;
class ABaseEntity;

//-----------------------------------------------------------------------------
// AMusicManager (0xBD0): de Blob's interactive music. Plays the level's music stream (an FSB of up to 8 tracks,
// Data/Sound/EN/) and mixes its tracks with the keyframes of the level's music data (Data/Sound/MusicData/<name>.trb):
// the regular keyframes follow the paint percentage, the snapshot keyframes ("inked", "silent", "rainbow",
// "bosslevel%d", "conversation"...) are blended over them. The sync points of the stream (beats, bars, scratches,
// melody sections, chords) trigger the queued sync sounds (FMOD events of the music SFX group), the GUI music
// controller and the controller feedback.
//-----------------------------------------------------------------------------
class AMusicManager : public Toshi::TSingleton<AMusicManager>
{
public:
	static TCONSTEXPR_VAR TINT AMUSICMANAGER_MAX_TRACKS = 8;
	static TCONSTEXPR_VAR TINT MAX_SYNC_SOUNDS          = 48;
	static TCONSTEXPR_VAR TINT MAX_PLOOPS               = 15;
	static TCONSTEXPR_VAR TINT MAX_COLOUR_SOUNDS        = 10;
	static TCONSTEXPR_VAR TINT SOUND_COLOUR_NUMOF       = 9;

	// The single player tracks (the names of scm_aszTrackNames)
	enum MUSIC_SP_TRACK : TINT
	{
		MUSIC_SP_FUNK_1     = 0,
		MUSIC_NUM_SP_TRACKS = 12,
	};

	// The values of the score marker data
	enum SCORE_MUSIC : TINT
	{
		SCORE_MUSIC_START,
		SCORE_MUSIC_END,
		SCORE_MUSIC_NumOF
	};

	// m_ui32StreamFlags
	enum STREAMFLAGS : TUINT32
	{
		SET_SNAPSHOT           = 0x00000001, // a snapshot keyframe (m_pSnapshotKeyFrame) is applied
		SET_MUSIC_PAUSED       = 0x00000002,
		SET_PLAY_STREAM        = 0x00000004, // plays the stream once it's opened
		SET_FADE_IN            = 0x00000008,
		SET_FADE_OUT           = 0x00000010,
		SET_BEAT               = 0x00000020, // waiting for the beat
		SET_SNAPSHOT_BLEND_IN  = 0x00000040,
		SET_SNAPSHOT_BLEND_OUT = 0x00000080,
		SET_SNAPSHOT_FROM_TEMP = 0x00000200, // the snapshot blends from the volumes copied to m_pTempKeyFrame
		SET_PAUSE_LOCKED       = 0x00002000, // PauseMusic does nothing
		SET_DISK_BUSY          = 0x00004000,
		SET_PAUSED_BY_STATES   = 0x00008000, // paused by PauseAll
		SET_START_SILENT       = 0x00020000, // the music data starts with the "silent" snapshot
		SET_SILENT             = 0x00040000,
		SET_NO_PERCENTAGE      = 0x00080000, // Update doesn't set the percentage from the score
		SET_SCRATCH            = 0x00200000, // waiting for the scratch
		SET_PITCH_CHANGE       = 0x00800000,
		SET_PITCH_THEN_PAUSE   = 0x01000000, // pauses the music once the pitch change is done
		SET_PITCH_CHANGE_KEEP  = 0x02000000, // the pitch change of a snapshot or SetPitch (no sync point while set)
		SET_FORCE_PAUSE        = 0x04000000, // PauseMusic always pauses at once
		SET_START_PAUSED       = 0x08000000,
		SET_BAR                = 0x20000000, // waiting for the bar
		SET_MUSIC_OVERRIDE     = 0x40000000, // UsingMusicOverride()
		SET_RAINBOW            = 0x80000000, // the "rainbow" snapshot (StartRainbow)
	};

	// SyncSound::m_uiFlags
	enum SSFLAGS : TUINT32
	{
		SS_DEFAULT          = 0x00000001,
		SS_POSITION         = 0x00000002, // sets the position of the event
		SS_PROCESS_ALL      = 0x00000004, // ProcessSyncSounds triggers every sync sound with this flag
		SS_PLOOP            = 0x00000008, // the event is a PLoop (StopPLoops stops it)
		SS_PLOOP_VOLUME     = 0x00000010, // m_fTimeOut is the value of the PLoop
		SS_GROUP_NAME       = 0x00000020, // m_szGroup and m_iEventID are the group and the event index
		SS_UNIQUE           = 0x00000040, // not queued if the same sound is queued
		SS_START            = 0x00000080, // PlayMusicSFX starts the event at once
		SS_START_POSITION   = 0x00000100, // PlayMusicSFX starts the event at once at the position
		SS_MUSIC_PROJECT    = 0x00000200,
		SS_SFX_PROJECT      = 0x00000400,
		SS_SNAPSHOT         = 0x00000800, // applies m_pSnapshotKeyFrame
		SS_RELEASE_SNAPSHOT = 0x00001000,
		SS_MUSIC_BAR        = 0x00002000, // on the bar (m_pSSMusicBarUsedList) instead of the beat
		SS_NO_EVENT         = 0x00004000,
		SS_BUILDING_COLOUR  = 0x00008000, // the event is the sound of the colour (m_iParam) in m_pBuildingColourList
		SS_PUSH_FRONT       = 0x00010000,
	};

	// m_ui8Flags of a track
	enum TRACKFLAGS : TUINT8
	{
		TRACK_2D       = 0x01,
		TRACK_3D       = 0x02,
		TRACK_POSITION = 0x04, // m_vPosition is set
		TRACK_MUTED    = 0x08,
	};

	// The blend modes of a snapshot keyframe
	enum BLENDMODE : TINT
	{
		BLENDMODE_LERP,
		BLENDMODE_MIN,
		BLENDMODE_MAX,
	};

	// A regular keyframe of the music data (0x08)
	struct VolumeKeyFrame
	{
		TFLOAT* m_pfVolumes;           // 0x00 one per track
		TFLOAT  m_fPercentageComplete; // 0x04
	};

	// A snapshot keyframe of the music data (0x1C)
	struct SnapshotKeyFrame
	{
		const TCHAR* m_szName;        // 0x00
		TFLOAT       m_fBlendInTime;  // 0x04
		TFLOAT       m_fBlendOutTime; // 0x08
		TFLOAT       m_fPitch;        // 0x0C
		BLENDMODE    m_eBlendMode;    // 0x10
		TFLOAT*      m_pfVolumes;     // 0x14 one per track
		TUINT32      m_uiUnknown;     // 0x18
	};

	// The "MusicData" symbol of the music data TRB
	struct KeyFrameList
	{
		TINT              m_iNumTracks;            // 0x00
		TUINT32           m_ui3DTracks;            // 0x04 bit i: track i is TRACK_3D
		TINT              m_bStartSilent;          // 0x08
		TUINT32           m_uiUnknown;             // 0x0C
		SnapshotKeyFrame* m_pConversationKeyFrame; // 0x10 NOTE: only m_fBlendInTime is read (PlayConversation)
		const TCHAR*      m_szStreamName;          // 0x14 the FSB in Data/Sound/EN/
		const TCHAR*      m_szProjectName;         // 0x18 the FMOD project of the music events
		TFLOAT*           m_pfPans;                // 0x1C one per track
		VolumeKeyFrame*   m_pRegularKeyFrames;     // 0x20
		SnapshotKeyFrame* m_pSnapshotKeyFrames;    // 0x24
		TINT              m_iNumRegularKeyFrames;  // 0x28
		TINT              m_iNumSnapshotKeyFrames; // 0x2C
	};

	// The "MusicSFXGroup" symbol
	struct MusicSFXInfo
	{
		const TCHAR* m_szProject; // 0x00
		const TCHAR* m_szGroup;   // 0x04
		const TCHAR* m_szPrefix;  // 0x08 of the event names
	};

	// The "MusicStringList" symbol
	struct StringList
	{
		TINT          m_iNumStrings; // 0x00
		const TCHAR** m_ppStrings;   // 0x04
	};

	// The "LoadGroupList" symbol
	struct LoadGroupList
	{
		TINT    m_iNumGroups; // 0x00
		TINT16* m_piGroups;   // 0x04 indices into m_pMusicStringList
	};

	// The "ScoreMarkerData" symbol
	struct ScoreMarkerData
	{
		TINT    m_iNumValues; // 0x00
		TFLOAT* m_pfValues;   // 0x04
	};

	// The "BuilingColourSounds" symbol
	struct ColourSound
	{
		TUINT32 m_uiSection; // 0x00 the melody section, 0: any
		TINT16  m_iEvent;    // 0x04 index into m_pMusicStringList
		TINT16  m_iGroup;    // 0x06 index into m_pMusicStringList
	};

	struct BuildingColour
	{
		TINT         m_iColour;    // 0x00
		TINT         m_iNumSounds; // 0x04
		ColourSound* m_pSounds;    // 0x08
	};

	struct BuildingColourList
	{
		TINT            m_iNumberOfColours; // 0x00
		BuildingColour* m_pColours;         // 0x04
	};

	// A sync point of the stream (0x08)
	struct SyncPoint
	{
		TUINT32 m_uiTime;  // 0x00 in ms
		TUINT32 m_uiValue; // 0x04
	};

	// The "syncpointstruct" symbol of the markers TRB
	struct SyncPoints
	{
		TINT       m_iNumBeats;     // 0x00
		TINT       m_iNumSections;  // 0x04
		TINT       m_iNumBars;      // 0x08
		TINT       m_iNumScratches; // 0x0C
		TINT       m_iNumChords;    // 0x10
		TUINT32*   m_puiBars;       // 0x14 times in ms
		TUINT32*   m_puiBeats;      // 0x18
		TUINT32*   m_puiScratches;  // 0x1C
		SyncPoint* m_pSections;     // 0x20 melody sections
		SyncPoint* m_pChords;       // 0x24 the chord flags (GetChordName)
	};

	// A track of the stream (0x34)
	struct Track
	{
		TUINT8          m_ui8Flags;             // 0x00
		TFLOAT          m_afSpeakerLevels[ 8 ]; // 0x04 NOTE: [ 7 ] is the pan of the track (SetTrackPan)
		Toshi::TVector4 m_vPosition;            // 0x24 TRACK_3D
	};

	// The settings of the stream (0x1A8)
	struct StreamSettings
	{
		TINT                m_iNumTracks;                           // 0x000
		Track               m_aoTracks[ AMUSICMANAGER_MAX_TRACKS ]; // 0x004
		FMOD::ChannelGroup* m_pChannelGroup;                        // 0x1A4
	};

	// A sound played on a sync point (0x30)
	struct SyncSound
	{
		TUINT32           m_uiFlags;           // 0x00 SSFLAGS
		TINT16            m_iGroupID;          // 0x04 ASoundMap::GROUPID or -1
		TINT16            m_iEventID;          // 0x06
		TINT              m_iParam;            // 0x08
		TFLOAT            m_fTimeOut;          // 0x0C released when it gets below 0
		SyncSound*        m_pNext;             // 0x10
		SyncSound*        m_pPrev;             // 0x14
		Toshi::TVector4   m_vPosition;         // 0x18
		const TCHAR*      m_szGroup;           // 0x28 SS_GROUP_NAME
		SnapshotKeyFrame* m_pSnapshotKeyFrame; // 0x2C SS_SNAPSHOT
	};

	// A looping event (0x10)
	struct PLoop : public Toshi::T2DList<PLoop>::Node
	{
		// $deBlob: FUNCTION 0060bdd0
		PLoop() {}

		FMOD::Event* m_pEvent; // 0x08
		TFLOAT       m_fValue; // 0x0C
	};

	// The event of a building colour (0x10)
	struct ColourSoundEvent : public Toshi::T2DList<ColourSoundEvent>::Node
	{
		// $deBlob: FUNCTION 0060bde0
		ColourSoundEvent()
		{
			m_pEvent    = TNULL;
			m_uiSection = 0;
		}

		FMOD::Event* m_pEvent;    // 0x08
		TUINT32      m_uiSection; // 0x0C
	};

public:
	AMusicManager();
	~AMusicManager();

	// The music data
	void LoadMusicData( const TCHAR* a_szName, TBOOL a_bLoadProjects, TBOOL a_bStreamed );
	void LoadMusicOverride( TBOOL a_bLoadProjects );
	void OnMusicDataLoaded( TBOOL a_bLoadProjects );
	void UnloadMusicData();
	void UnloadStreamSettings();
	void LoadSyncPoints( const TCHAR* a_szName );
	void SetSyncPoints( Toshi::TTRB* a_pTRB );
	void LoadSoundTRB( const TCHAR* a_szName );
	void SetKeyFrameList( Toshi::TTRB* a_pTRB );
	void CloseSoundTRB();

	// The stream
	void        CreateMusicStream();
	void        CreateStream( const TCHAR* a_szName, TINT a_iUnused1, TINT a_iUnused2, TBOOL a_bCreateChannelGroup );
	void        ReleaseStream();
	TBOOL       IsStreamReady();
	void        PlayStream( TBOOL a_bPaused );
	void        RestartStream();
	void        StartMusic( TBOOL a_bPaused );
	void        StartLevelMusic();
	void        StartRainbow();
	static void SetStreamBufferSize( TUINT a_uiSize );
	void        RestoreStreamBufferSize();
	void ResetSpeakerLevels( FMOD::Channel* a_pChannel );
	void TestSpeakerLevels();

	void  Update( TFLOAT a_fDeltaTime );
	void  UpdateVolumes();
	void  UpdateTrackLevels( TFLOAT a_fDeltaTime );
	void  GetDebugInfo( TINT* a_pBeat, TINT* a_pBar, TUINT* a_pNextBeat, TUINT* a_pNextBar, TUINT* a_pPosition, TBOOL* a_pWaitingForBeat, TBOOL* a_pWaitingForBar, TBOOL* a_pStarving, TUINT* a_pPercentBuffered, TINT* a_pOpenState, TINT* a_pResult );
	TUINT GetPosition();
	void  SetPosition( TUINT a_uiPosition );

	// Pausing
	static void PauseChannel( TBOOL a_bPause, FMOD::Channel* a_pChannel, TBOOL* a_pPaused );
	void        PauseMusic( TBOOL a_bPause, TFLOAT a_fFadeTime );
	TBOOL       IsMusicPaused();
	void        PauseAll();
	void        ResumeAll();
	void        ResumeAllAndRestart();
	void        PauseMusicNow();
	void        ResumeMusic();
	void        SetPauseLocked( TBOOL a_bLocked );
	void        SetForcePause( TBOOL a_bForce );
	void        SetDiskBusy( TBOOL a_bBusy );

	// The volumes (a_uiFlags: 1 the 2D tracks, 2 the 3D tracks)
	TFLOAT GetVolume( TUINT a_uiFlags );
	void   SetCurrentVolume( TFLOAT a_fVolume, TUINT a_uiFlags );
	void   SetVolume( TFLOAT a_fVolume, TUINT a_uiFlags );
	TFLOAT GetTargetVolume( TUINT a_uiFlags );

	// The tracks
	TFLOAT GetTrackVolume( TINT a_iTrackNumber );
	void   SetTrackPan( TINT a_iTrackNumber, TFLOAT a_fPan );
	void   SetTrackPosition( TINT a_iTrackNumber, const Toshi::TVector4& a_rcPosition );
	void   SetTrackFlags( TINT a_iTrackNumber, TUINT8 a_ui8Flags, TBOOL a_bSet );
	TUINT8 GetTrackFlags( TINT a_iTrackNumber );
	void   SetTrackMuted( TINT a_iTrackNumber, TBOOL a_bMuted );
	TINT   GetNumTracks();
	void   GetTrackLevels( TINT a_iTrackNumber, TFLOAT* a_pLeft, TFLOAT* a_pRight );

	// The keyframes
	void              SetPercentage( TFLOAT a_fPercentage );
	void              UpdatePercentage();
	void              AddScore( TINT a_iScore );
	void              SetMaxScore( TINT a_iMaxScore );
	void              ResetScore();
	SnapshotKeyFrame* FindSnapshotKeyFrame( const TCHAR* a_szName );
	void              ApplySnapshotKeyFrame( SnapshotKeyFrame* a_pKeyFrame );
	void              ReleaseSnapshot();
	TBOOL             PlaySnapshot( const TCHAR* a_szName );
	TBOOL             GetRegularKeyFrameVolumes( TFLOAT* a_pfVolumes, TFLOAT a_fPercentage );
	TINT              GetNumSnapshotKeyFrames();
	const TCHAR*      GetSnapshotKeyFrameName( TINT a_iIndex );
	const TCHAR*      GetCurrentSnapshotName();
	TBOOL             IsInked();
	TBOOL             IsSnapshotActive();
	TBOOL             IsSnapshot( const TCHAR* a_szName );
	TBOOL             IsConversation();
	void              PlayConversation();
	void              ClampScore();
	TBOOL             IsSilent();
	void              SetNoPercentage( TBOOL a_bNoPercentage );
	TFLOAT            GetScoreMarker( TINT a_iMarkerNumber );

	// The boss levels
	void  SetBossLevel( TINT a_iBossLevel );
	TBOOL PlayBossLevelSnapshot();

	// The pitch
	void  ResetPitch( TFLOAT a_fTime );
	void  SetPitch( TFLOAT a_fPitch, TFLOAT a_fTime, TBOOL a_bPauseAfter );
	void  AddPitch( TFLOAT a_fPitch, TFLOAT a_fTime );
	TBOOL IsPitchChangeKept();

	// The music override (the track of the jukebox)
	void  SetMusicOverride( const TCHAR* a_szName );
	void  SetBossMusicOverride();
	void  SetRandomMusicOverride();
	void  ClearMusicOverride();
	TBOOL UsingMusicOverride();

	// $deBlob: FUNCTION 00638cf0
	TINT GetScore() const { return m_iScore; }
	// $deBlob: FUNCTION 00638d00
	TINT GetMaxScore() const { return m_iMaxScore; }
	// $deBlob: FUNCTION 00638d10
	const TCHAR* GetMusicOverride() const { return m_szMusicOverride; }

	// NOTE: empty in retail
	// $deBlob: FUNCTION 00608f10
	void OnEmptyEvent1() {}
	// $deBlob: FUNCTION 00608f20
	void                 OnEmptyEvent2() {}
	static const TCHAR*  GetTrackName( TINT a_iTrackNumber );
	static const TCHAR*  GetTrackNameFromDisplayName( const TWCHAR* a_wszName );
	static const TWCHAR* GetTrackDisplayName( const TCHAR* a_szName );
	TBOOL                IsTrackUnlocked( const TCHAR* a_szName, TINT a_iLevel );

	// The sync sounds
	TBOOL AddSyncSound( TINT a_iGroupID, TINT a_iEventID, TFLOAT a_fTimeOut, TUINT32 a_uiFlags, const Toshi::TVector4* a_pPosition, const TCHAR* a_szGroup, const TCHAR* a_szSnapshot, TINT a_iParam );
	TBOOL AddSyncSound( TINT a_iEventID, TFLOAT a_fTimeOut );
	TBOOL IsSyncSoundQueued( TINT a_iGroupID, TINT a_iEventID, TUINT32 a_uiFlags, const TCHAR* a_szGroup );
	TBOOL AddPaintSyncSound( TINT a_iColour, ABaseEntity* a_pBlob, const Toshi::TVector4* a_pPosition );
	TBOOL AddColourSyncSound( TINT a_iColour, const Toshi::TVector4* a_pPosition );
	TBOOL PlayGoodCombo();
	TBOOL PlayLevelComplete();
	void  PlayGameOver();
	void  ReleaseSyncSound( SyncSound* a_pSyncSound );
	void  FreeAllSyncSounds();
	void  RemoveMusicSFXSyncSounds( const TCHAR* a_szEvent );
	TBOOL ProcessSyncSounds( SyncSound* a_pSyncSound );
	void  PlaySyncSound( SyncSound* a_pSyncSound );
	TBOOL FindColourSound( SyncSound* a_pSyncSound );

	// The music SFX (the events of the music SFX group)
	FMOD::Event* GetMusicSFXEvent( const TCHAR* a_szName, TBOOL a_bInfoOnly );
	void         PlayMusicSFX( const TCHAR* a_szName, TUINT32 a_uiFlags, const Toshi::TVector4* a_pPosition, const TCHAR* a_szSnapshot );
	TFLOAT       GetMusicSFXLength( const TCHAR* a_szName );
	static void  GetChordName( TUINT32 a_uiChord, TCHAR* a_szName, TSIZE a_uiSize );
	const TCHAR* GetMusicString( TINT16 a_iIndex );

	// The PLoops and the colour sounds
	void AddPLoop( FMOD::Event* a_pEvent, TUINT32 a_uiFlags, TFLOAT a_fValue );
	void RemovePLoop( PLoop* a_pPLoop );
	void StopPLoops();
	void StopPLoop( const TCHAR* a_szName );
	void SetPLoopParameter( const TCHAR* a_szName, TFLOAT a_fValue );
	void SetPLoopVolume( TFLOAT a_fVolume );
	void SetPLoopPaused( TBOOL a_bPaused );
	void UpdateColourSounds( FMOD::Event* a_pEvent, TUINT32 a_uiSection );
	void StopColourSounds();

	// The sync points
	void ResetSyncPoints();
	// $deBlob: FUNCTION 006092f0
	TINT GetNumBeats() { return m_pSyncPoints->m_iNumBeats; }
	// $deBlob: FUNCTION 00609300
	TINT GetNumScratches() { return m_pSyncPoints->m_iNumScratches; }
	// $deBlob: FUNCTION 00609310
	TINT GetNumBars() { return m_pSyncPoints->m_iNumBars; }
	// $deBlob: FUNCTION 00609320
	TINT GetNumSections() { return m_pSyncPoints->m_iNumSections; }
	// $deBlob: FUNCTION 00609330
	TINT  GetNumChords() { return m_pSyncPoints->m_iNumChords; }
	TUINT GetSection( TUINT a_uiPosition );
	TUINT GetChord( TUINT a_uiPosition );
	TBOOL FindNextSyncPoint( TUINT a_uiPosition, TINT* a_pIndex, TUINT* a_pNext, TUINT* a_pPrevious, const TUINT32* a_puiTimes, TINT a_iNumTimes );

	// The GUI music controller
	// $deBlob: FUNCTION 0062af10
	AGUIMusicController* GetGUIMusicController() const { return m_pGUIMusicController; }
	void                 RegisterGUIScene( Toshi::TXUIScene* a_pScene );
	void                 UnregisterGUIScene( Toshi::TXUIScene* a_pScene );

	static void  Calculate3DTrackLevels( Track* a_pTrack );
	static void  DebugCategories( FMOD::EventCategory* a_pCategory );
	static void  DebugMemory();
	static TBOOL ErrorCheck( FMOD_RESULT a_eResult );

private:
	static void OnMusicDataStreamed( ATRBLoaderJob* a_pJob, void* a_pUserData );

private:
	Toshi::T2DList<PLoop>            m_oPLoopList;                         // 0x000
	Toshi::T2DList<PLoop>            m_oFreePLoopList;                     // 0x008
	PLoop                            m_aPLoops[ MAX_PLOOPS ];              // 0x010
	Toshi::T2DList<ColourSoundEvent> m_oColourSoundList;                   // 0x100
	Toshi::T2DList<ColourSoundEvent> m_oFreeColourSoundList;               // 0x108
	ColourSoundEvent                 m_aColourSounds[ MAX_COLOUR_SOUNDS ]; // 0x110
	TUINT32                          m_ui32StreamFlags;                    // 0x1B0
	TINT                             m_iBeat;                              // 0x1B4
	TINT                             m_iBar;                               // 0x1B8
	TINT                             m_iScratch;                           // 0x1BC
	TUINT                            m_uiNextBeat;                         // 0x1C0
	TUINT                            m_uiNextBar;                          // 0x1C4
	TUINT                            m_uiNextScratch;                      // 0x1C8
	TUINT                            m_uiPrevBeat;                         // 0x1CC
	TUINT                            m_uiPrevBar;                          // 0x1D0
	TUINT                            m_uiPrevScratch;                      // 0x1D4
	TINT                             m_iSection;                           // 0x1D8 index of the last section (GetSection)
	TINT                             m_iChord;                             // 0x1DC index of the last chord (GetChord)
	TUINT                            m_uiPosition;                         // 0x1E0 in ms
	TUINT                            m_uiStreamBufferSize;                 // 0x1E4
	TINT                             m_iBossLevel;                         // 0x1E8 -1: none
	TINT                             m_iScore;                             // 0x1EC
	TINT                             m_iMaxScore;                          // 0x1F0
	Toshi::TTRB                      m_oTRBSyncPoints;                     // 0x1F4 Data/Sound/SoundMarkers/
	Toshi::TTRB                      m_oTRBSound;                          // 0x208
	Toshi::TTRB                      m_oTRBMusicData;                      // 0x21C
	SyncSound                        m_aSyncSounds[ MAX_SYNC_SOUNDS ];     // 0x230
	MusicSFXInfo*                    m_pMusicSFXInfo;                      // 0xB30
	BuildingColourList*              m_pBuildingColourList;                // 0xB34
	KeyFrameList*                    m_pKeyFrameList;                      // 0xB38
	StreamSettings*                  m_pStreamSettings;                    // 0xB3C
	SyncSound*                       m_pSSUsedList;                        // 0xB40
	SyncSound*                       m_pSSFreeList;                        // 0xB44
	SyncSound*                       m_pSSMusicBarUsedList;                // 0xB48
	SnapshotKeyFrame*                m_pSnapshotKeyFrame;                  // 0xB4C
	SnapshotKeyFrame*                m_pTempKeyFrame;                      // 0xB50 "temp"
	StringList*                      m_pMusicStringList;                   // 0xB54
	SyncPoints*                      m_pSyncPoints;                        // 0xB58
	FMOD::Channel*                   m_pChannel;                           // 0xB5C
	FMOD::Sound*                     m_pSound;                             // 0xB60
	ScoreMarkerData*                 m_pScoreMarkerData;                   // 0xB64
	TUINT32                          m_uiUnknownB68;                       // 0xB68 never used
	FMOD::EventProject*              m_pMusicSFXProject;                   // 0xB6C
	FMOD::EventProject*              m_pMusicProject;                      // 0xB70
	AGUIMusicController*             m_pGUIMusicController;                // 0xB74
	TUINT                            m_uiSection;                          // 0xB78 the current melody section
	TUINT                            m_uiChord;                            // 0xB7C the current chord flags
	TINT                             m_iUnknownB80;                        // 0xB80 -1, never used
	TBOOL                            m_bRestartStream;                     // 0xB84
	TUINT32                          m_uiUnknownB88;                       // 0xB88 never used
	TFLOAT                           m_fPercentage;                        // 0xB8C 0-100
	TFLOAT                           m_fVolume2D;                          // 0xB90
	TFLOAT                           m_fVolume3D;                          // 0xB94
	TFLOAT                           m_fTargetVolume2D;                    // 0xB98
	TFLOAT                           m_fTargetVolume3D;                    // 0xB9C
	TFLOAT                           m_fFadeTime;                          // 0xBA0
	TFLOAT                           m_fFadeTimer;                         // 0xBA4
	TFLOAT                           m_fSnapshotBlendTime;                 // 0xBA8
	TFLOAT                           m_fSnapshotBlendTimer;                // 0xBAC
	TFLOAT                           m_fFadeVolume3D;                      // 0xBB0
	TFLOAT                           m_fFadeVolume2D;                      // 0xBB4
	TFLOAT                           m_fDiskBusyTime;                      // 0xBB8
	TFLOAT                           m_fTargetPitch;                       // 0xBBC
	TFLOAT                           m_fPitch;                             // 0xBC0
	TFLOAT                           m_fPitchTimer;                        // 0xBC4
	TFLOAT                           m_fPitchTime;                         // 0xBC8
	const TCHAR*                     m_szMusicOverride;                    // 0xBCC
};
