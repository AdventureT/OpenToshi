#include "pch.h"
#include "GameInterface/AXUIState.h"
#include "GameInterface/AGameStateController.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "APositionalSoundManager.h"
#include "AMusicManager.h"
#include "ALevelInformation.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "ASoundInterface.h"
#include "ASoundLoadHelper.h"
#include "ASoundMap.h"
#include "AAssetStreaming.h"
#include "ARandom.h"
#include "Options/AOptions.h"
#include "GameInterface/APlayerProfileManager.h"

#include <Toshi/T2String8.h>
#include <Toshi/T2String16.h>
#include <Toshi/TStringManager.h>
#include <Toshi/TUtil.h>
#include <T2Locale/T2Locale.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The boss track and the conversation snapshot
static const TCHAR* s_szBossTrack    = "M12_Boss";
static const TCHAR* s_szConversation = "conversation";

// The single player tracks and the locale strings of their names
static const TCHAR* s_aszTrackNames[ AMusicManager::MUSIC_NUM_SP_TRACKS ] = {
	"M05_lounge",
	"M01_Funk1",
	"M02_Funk2",
	"M10_HipHop",
	"M11_Disco",
	"M07_DubSka",
	"M04_Funk4",
	"M03_Funk3",
	"M06_Orleans",
	"M08_Cuban",
	"M12_Boss_Remix",
	"M09_Samba",
};

static const TINT s_aiTrackDisplayNames[ AMusicManager::MUSIC_NUM_SP_TRACKS ] = {
	0x23A, 0x236, 0x237, 0x23F, 0x240, 0x23C, 0x239, 0x238, 0x23B, 0x23D, 0x241, 0x23E
};

static const TWCHAR* s_wszNoMoodFound = L"No mood found";

// The distances of the 3D tracks: full volume up to s_f3DMinDistance, silent at s_f3DMaxDistance
static const TFLOAT s_f3DMinDistance = 20.0f;
static const TFLOAT s_f3DMaxDistance = 200.0f;

// Added to the position of the stream when looking for the sync points (ms)
static const TUINT s_uiSyncPointLatency = 34;

// The controller feedback on the bars and the beats (never cleared in retail)
static TBOOL s_bBarFeedback  = TTRUE;
static TBOOL s_bBeatFeedback = TTRUE;

// FLT_MIN: the epsilon of the pitch and the fade time
static TCONSTEXPR_VAR TFLOAT s_fEpsilon = 1.17549435e-38f;

// $deBlob: FUNCTION 00605760
AMusicManager::AMusicManager()
{
	m_ui32StreamFlags = 0;
	m_iBeat           = 0;
	m_iBar            = 0;
	m_iScratch        = 0;
	m_uiNextBeat      = 0;
	m_uiNextBar       = 0;
	m_uiNextScratch   = 0;
	m_uiPrevBeat      = 0;
	m_uiPrevBar       = 0;
	m_uiPrevScratch   = 0;
	m_iSection        = 0;
	m_iChord          = 0;
	m_uiPosition      = 0;
	m_iBossLevel      = -1;
	m_iScore          = 0;
	m_iMaxScore       = 10000;

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail doesn't initialise them (GetMusicSFXEvent checks m_pMusicSFXInfo before any music data is loaded)
	m_pMusicSFXInfo   = TNULL;
	m_uiUnknownB68    = 0;
	m_szMusicOverride = TNULL;
#endif

	m_pBuildingColourList = TNULL;
	m_pKeyFrameList       = TNULL;
	m_pStreamSettings     = TNULL;
	m_pSSUsedList         = TNULL;
	m_pSSFreeList         = TNULL;
	m_pSSMusicBarUsedList = TNULL;
	m_pSnapshotKeyFrame   = TNULL;
	m_pMusicStringList    = TNULL;
	m_pSyncPoints         = TNULL;
	m_pChannel            = TNULL;
	m_pSound              = TNULL;
	m_pScoreMarkerData    = TNULL;
	m_pMusicSFXProject    = TNULL;
	m_pMusicProject       = TNULL;
	m_pGUIMusicController = TNULL;
	m_uiSection           = 0;
	m_uiChord             = 0;
	m_iUnknownB80         = -1;
	m_bRestartStream      = TFALSE;
	m_uiUnknownB88        = 0;
	m_fPercentage         = 0.0f;
	m_fVolume2D           = 1.0f;
	m_fVolume3D           = 1.0f;
	m_fTargetVolume2D     = 1.0f;
	m_fTargetVolume3D     = 1.0f;
	m_fFadeTime           = 0.0f;
	m_fFadeTimer          = 0.0f;
	m_fSnapshotBlendTime  = 0.0f;
	m_fSnapshotBlendTimer = 0.0f;
	m_fFadeVolume3D       = 0.5f;
	m_fFadeVolume2D       = 0.5f;
	m_fDiskBusyTime       = 0.0f;
	m_fTargetPitch        = 1.0f;
	m_fPitch              = 1.0f;
	m_fPitchTimer         = 0.0f;
	m_fPitchTime          = 1.0f;

	// The keyframe the snapshots blend from
	m_pTempKeyFrame              = new SnapshotKeyFrame();
	m_pTempKeyFrame->m_pfVolumes = new TFLOAT[ AMUSICMANAGER_MAX_TRACKS ];
	m_pTempKeyFrame->m_szName    = "temp";

	// The free sync sounds
	// NOTE: retail links the first MAX_SYNC_SOUNDS - 1 sync sounds in the loop (and sets their flags), then the last
	SyncSound* pPrev = TNULL;

	for ( TINT i = 0; i < MAX_SYNC_SOUNDS - 1; i++ )
	{
		m_aSyncSounds[ i ].m_pPrev   = pPrev;
		m_aSyncSounds[ i ].m_pNext   = &m_aSyncSounds[ i + 1 ];
		m_aSyncSounds[ i ].m_uiFlags = SS_DEFAULT;
		pPrev                        = &m_aSyncSounds[ i ];
	}

	m_aSyncSounds[ MAX_SYNC_SOUNDS - 1 ].m_pNext = TNULL;
	m_aSyncSounds[ MAX_SYNC_SOUNDS - 1 ].m_pPrev = pPrev;
	m_pSSFreeList                                = &m_aSyncSounds[ 0 ];

	for ( TINT i = 0; i < MAX_COLOUR_SOUNDS; i++ )
		m_oFreeColourSoundList.PushBack( &m_aColourSounds[ i ] );

	for ( TINT i = 0; i < MAX_PLOOPS; i++ )
		m_oFreePLoopList.PushBack( &m_aPLoops[ i ] );

	m_pGUIMusicController = new AGUIMusicController();

	if ( TSound::GetSingleton()->IsInitialised() )
	{
		TSound::GetSingleton()->GetSystem();
		ResetSpeakerLevels( TNULL );

		FMOD_TIMEUNIT eTimeUnit;
		TSound::GetSingleton()->GetSystem()->getStreamBufferSize( &m_uiStreamBufferSize, &eTimeUnit );
	}
}

// $deBlob: FUNCTION 00605bc0
// $deBlob: FUNCTION 0056c160
AMusicManager::~AMusicManager()
{
	if ( m_pTempKeyFrame != TNULL )
		delete[] m_pTempKeyFrame->m_pfVolumes;

	m_oFreePLoopList.Clear();
	m_oFreeColourSoundList.Clear();

	delete m_pTempKeyFrame;

	if ( m_pGUIMusicController != TNULL )
		delete m_pGUIMusicController;

	m_pGUIMusicController = TNULL;

	TASSERT( m_pStreamSettings == TNULL );
}

// $deBlob: FUNCTION 00605d70
void AMusicManager::PauseChannel( TBOOL a_bPause, FMOD::Channel* a_pChannel, TBOOL* a_pPaused )
{
	if ( a_pChannel == TNULL )
		return;

	if ( a_bPause )
	{
		bool bPlaying = false;
		*a_pPaused    = TFALSE;
		a_pChannel->isPlaying( &bPlaying );

		if ( bPlaying )
		{
			bool        bChannelPaused = false;
			FMOD_RESULT eResult        = a_pChannel->getPaused( &bChannelPaused );
			TASSERT( FMOD_OK == eResult );

			*a_pPaused = !bChannelPaused;
		}

		if ( *a_pPaused )
			a_pChannel->setPaused( true );
	}
	else if ( *a_pPaused )
	{
		a_pChannel->setPaused( false );
	}
}

// $deBlob: FUNCTION 00605e20
void AMusicManager::TestSpeakerLevels()
{
	TSound::GetSingleton()->GetSystem();
	ResetSpeakerLevels( TNULL );
}

// $deBlob: FUNCTION 00605e40
void AMusicManager::LoadMusicData( const TCHAR* a_szName, TBOOL a_bLoadProjects, TBOOL a_bStreamed )
{
	TCHAR szFolder[] = "Data/Sound/MusicData/";
	TCHAR szFileName[ 256 ];
	TStringManager::String8Format( szFileName, 256, "%s%s.trb", szFolder, a_szName );

	TASSERT( m_oTRBMusicData.GetHeader() == TNULL );

	ResetSyncPoints();
	TFree( m_pStreamSettings );
	m_pStreamSettings = TNULL;
	m_oTRBMusicData.Close();

	if ( ASoundLoadHelper::GetSingleton()->GetState() == ASoundLoadHelper::LOADED )
		ASoundLoadHelper::GetSingleton()->UnloadSoundEventGroups();

	TINT iCurrentAlloced = 0;
	TINT iMaxAlloced     = 0;
	FMOD_Memory_GetStats( &iCurrentAlloced, &iMaxAlloced, 1 );

	if ( a_bLoadProjects )
	{
		ASoundLoadHelper::GetSingleton()->UnloadSoundEventGroups();
		ASoundLoadHelper::GetSingleton()->UnloadEventProjects();
	}

	m_oTRBSyncPoints.Close();

	if ( a_bStreamed )
	{
		AAssetStreaming* pAssetStreaming = AAssetStreaming::GetSingleton();
		ATRBLoaderJob*   pJob            = pAssetStreaming->GetFreeJob();
		pJob->Init( &m_oTRBMusicData, szFileName );
		pJob->m_pOnFinishedUserData = (void*)TUINTPTR( a_bLoadProjects ? 1 : 0 );
		pJob->m_fnOnFinished        = OnMusicDataStreamed;
		pAssetStreaming->AddMainThreadJob2( pJob );
		return;
	}

	TTRB::ERROR rResult = m_oTRBMusicData.Load( szFileName );
	TASSERT( TTRB::ERROR_OK == rResult );

	OnMusicDataLoaded( a_bLoadProjects );
}

// $deBlob: FUNCTION 00606020
void AMusicManager::CreateMusicStream()
{
	if ( TSound::GetSingleton()->IsInitialised() && m_pKeyFrameList != TNULL )
		CreateStream( m_pKeyFrameList->m_szStreamName, 1, 1, TTRUE );
}

// $deBlob: FUNCTION 00606050
void AMusicManager::CreateStream( const TCHAR* a_szName, TINT a_iUnused1, TINT a_iUnused2, TBOOL a_bCreateChannelGroup )
{
	if ( a_szName == TNULL )
		return;

	SetStreamBufferSize( 0x40000 );
	FMOD::System* pSystem = TSound::GetSingleton()->GetSystem();

	if ( T2String8::Length( a_szName ) == 0 || T2String8::CompareNoCase( "default", a_szName ) == 0 )
		return;

	TCHAR szFolder[] = "Data/Sound/EN/";
	ReleaseStream();

	TCHAR szFileName[ 256 ];
	TStringManager::String8Format( szFileName, 256, "%s%s", szFolder, a_szName );

	if ( TSound::GetSingleton()->IsInitialised() )
	{

		FMOD::Sound* pSound = TNULL;
		TSound::ErrorCheck( pSystem->createStream( szFileName, FMOD_CREATESTREAM | FMOD_SOFTWARE | FMOD_LOOP_NORMAL, TNULL, &pSound ) );

		TINT iNumSubSounds = 0;
		TSound::ErrorCheck( pSound->getNumSubSounds( &iNumSubSounds ) );

		if ( iNumSubSounds >= 1 )
			TSound::ErrorCheck( pSound->getSubSound( 0, &m_pSound ) );
		else
			m_pSound = pSound;
	}

	if ( a_bCreateChannelGroup && m_pStreamSettings != TNULL )
	{
		if ( m_pStreamSettings->m_pChannelGroup != TNULL )
		{
			m_pStreamSettings->m_pChannelGroup->release();
			m_pStreamSettings->m_pChannelGroup = TNULL;
		}

		TSound::ErrorCheck( pSystem->createChannelGroup( a_szName, &m_pStreamSettings->m_pChannelGroup ) );
	}
}

// $deBlob: FUNCTION 006061f0
void AMusicManager::LoadMusicOverride( TBOOL a_bLoadProjects )
{
	TASSERT( UsingMusicOverride() );
	LoadMusicData( m_szMusicOverride, a_bLoadProjects, TFALSE );
}

// $deBlob: FUNCTION 00606250
void AMusicManager::ReleaseStream()
{
	PauseMusic( TTRUE, 0.0f );

	if ( m_pSound != TNULL )
	{
		FMOD::Sound* pParent = TNULL;
		m_pSound->getSubSoundParent( &pParent );

		if ( pParent != TNULL )
			pParent->release();
		else
			m_pSound->release();

		m_pSound = TNULL;
	}
}

// $deBlob: FUNCTION 006062c0
TBOOL AMusicManager::IsStreamReady()
{
	if ( m_pSound != TNULL )
	{
		FMOD_OPENSTATE eOpenState;
		TUINT          uiPercentBuffered = 0;
		bool           bStarving         = false;
		bool           bDiskBusy         = false;
		TSound::ErrorCheck( m_pSound->getOpenState( &eOpenState, &uiPercentBuffered, &bStarving, &bDiskBusy ) );

		if ( eOpenState != FMOD_OPENSTATE_READY || bStarving )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00606320
void AMusicManager::LoadSyncPoints( const TCHAR* a_szName )
{
	TCHAR szFolder[] = "Data/Sound/SoundMarkers/";
	TCHAR szFileName[ 256 ];
	TStringManager::String8Format( szFileName, 256, "%s%s", szFolder, a_szName );

	m_pSyncPoints = TNULL;
	m_oTRBSyncPoints.Close();

	if ( m_oTRBSyncPoints.Load( szFileName ) != TTRB::ERROR_OK )
	{
		// NOTE: retail closes m_oTRBSound here, not m_oTRBSyncPoints
		m_oTRBSound.Close();
		return;
	}

	m_pSyncPoints = m_oTRBSyncPoints.CastSymbol<SyncPoints>( "syncpointstruct" );
}

// $deBlob: FUNCTION 006063f0
void AMusicManager::SetSyncPoints( TTRB* a_pTRB )
{
	m_pSyncPoints = a_pTRB->CastSymbol<SyncPoints>( "syncpointstruct" );
}

// $deBlob: FUNCTION 00606410
void AMusicManager::SetStreamBufferSize( TUINT a_uiSize )
{
	FMOD::System* pSystem = TSound::GetSingleton()->GetSystem();
	pSystem->setStreamBufferSize( a_uiSize, FMOD_TIMEUNIT_RAWBYTES );
}

// $deBlob: FUNCTION 00606430
void AMusicManager::PauseMusic( TBOOL a_bPause, TFLOAT a_fFadeTime )
{
	TUINT32 uiFlags = m_ui32StreamFlags;

	if ( uiFlags & SET_PAUSE_LOCKED )
		return;

	TBOOL  bPause;
	TFLOAT fFadeTime;

	if ( uiFlags & SET_FORCE_PAUSE )
	{
		bPause    = TTRUE;
		fFadeTime = 0.0f;
		a_bPause  = TTRUE;
	}
	else
	{
		fFadeTime = a_fFadeTime;
		bPause    = a_bPause;
	}

	if ( m_pSound == TNULL || ( !bPause && ( uiFlags & SET_SILENT ) ) )
		return;

	if ( s_fEpsilon + 0.0f < fFadeTime )
	{
		// NOTE: retail doesn't initialise it (getPaused fails without a channel)
		bool bPaused = false;
		m_pChannel->getPaused( &bPaused );

		if ( !bPause || !bPaused )
		{
			m_fFadeTime  = fFadeTime;
			m_fFadeTimer = 0.0f;

			if ( bPause == TTRUE )
			{
				m_ui32StreamFlags = ( m_ui32StreamFlags & ~( SET_MUSIC_PAUSED | SET_FADE_IN | SET_FADE_OUT ) ) | SET_MUSIC_PAUSED | SET_FADE_OUT;
				TASSERT( 0 == ( m_ui32StreamFlags & SET_FADE_IN ) );
			}
			else
			{
				m_ui32StreamFlags = ( m_ui32StreamFlags & ~( SET_MUSIC_PAUSED | SET_FADE_IN | SET_FADE_OUT ) ) | SET_FADE_IN;
				TASSERT( 0 == ( m_ui32StreamFlags & SET_FADE_OUT ) );
			}

			m_fFadeVolume2D = m_fVolume2D;
			m_fFadeVolume3D = m_fVolume3D;
		}
	}
	else
	{
		m_ui32StreamFlags = uiFlags & ~( SET_FADE_IN | SET_FADE_OUT );
		SetPLoopPaused( a_bPause );
		m_pChannel->setPaused( a_bPause );
	}
}

// $deBlob: FUNCTION 006065a0
TBOOL AMusicManager::IsMusicPaused()
{
	TBOOL bMusicPaused = ( m_ui32StreamFlags & SET_MUSIC_PAUSED ) != 0;
	bool  bPaused      = false;

	if ( m_pChannel != TNULL )
		m_pChannel->getPaused( &bPaused );

	return bMusicPaused && bPaused;
}

// $deBlob: FUNCTION 006065f0
void AMusicManager::StartMusic( TBOOL a_bPaused )
{
	m_ui32StreamFlags |= SET_PLAY_STREAM;
	m_iScore = 0;

	if ( m_ui32StreamFlags & SET_START_SILENT )
		m_ui32StreamFlags |= SET_START_PAUSED | SET_SILENT;

	if ( a_bPaused )
		m_ui32StreamFlags |= SET_MUSIC_PAUSED;
}

// $deBlob: FUNCTION 00606630
void AMusicManager::LoadSoundTRB( const TCHAR* a_szName )
{
	TCHAR szFolder[] = "Data/Sound/";
	TCHAR szFileName[ 256 ];
	TStringManager::String8Format( szFileName, 256, "%s%s", szFolder, a_szName );

	// NOTE: retail loads a_szName, not the path it made
	if ( m_oTRBSound.Load( a_szName ) != TTRB::ERROR_OK )
		m_oTRBSound.Close();
}

// $deBlob: FUNCTION 006066b0
void AMusicManager::SetKeyFrameList( TTRB* a_pTRB )
{
	// NOTE: retail ignores a_pTRB
	m_pKeyFrameList = m_oTRBSyncPoints.CastSymbol<KeyFrameList>( "MusicData" );
}

// $deBlob: FUNCTION 006066d0
TFLOAT AMusicManager::GetVolume( TUINT a_uiFlags )
{
	if ( a_uiFlags & 1 )
		return m_fVolume2D;

	return m_fVolume3D;
}

// $deBlob: FUNCTION 006066f0
void AMusicManager::SetCurrentVolume( TFLOAT a_fVolume, TUINT a_uiFlags )
{
	if ( a_uiFlags & 1 )
	{
		m_fVolume2D     = a_fVolume;
		m_fFadeVolume2D = a_fVolume;
	}
	else
	{
		m_fVolume3D     = a_fVolume;
		m_fFadeVolume3D = a_fVolume;
	}
}

// $deBlob: FUNCTION 00606730
void AMusicManager::SetVolume( TFLOAT a_fVolume, TUINT a_uiFlags )
{
	if ( a_uiFlags & 1 )
	{
		m_fTargetVolume2D = a_fVolume;
		m_fVolume2D       = a_fVolume;
		m_fFadeVolume2D   = a_fVolume;
	}

	if ( a_uiFlags & 2 )
	{
		m_fTargetVolume3D = a_fVolume;

#ifdef DEBLOB_COMMUNITY_PATCH
		m_fVolume3D     = m_fTargetVolume3D;
		m_fFadeVolume3D = m_fTargetVolume3D;
#else
		// NOTE: retail sets the 3D volumes to the target volume of the 2D tracks
		m_fVolume3D     = m_fTargetVolume2D;
		m_fFadeVolume3D = m_fTargetVolume2D;
#endif
	}
}

// $deBlob: FUNCTION 00606780
TFLOAT AMusicManager::GetTargetVolume( TUINT a_uiFlags )
{
	if ( a_uiFlags & 1 )
		return m_fTargetVolume2D;

	// NOTE: retail returns the current volume of the 3D tracks
	return m_fVolume3D;
}

// $deBlob: FUNCTION 006067a0
TFLOAT AMusicManager::GetTrackVolume( TINT a_iTrackNumber )
{
	// NOTE: retail's check lets a_iTrackNumber be m_iNumTracks
	if ( m_pStreamSettings->m_iNumTracks < a_iTrackNumber )
		return 0.0f;

	Track& rTrack = m_pStreamSettings->m_aoTracks[ a_iTrackNumber ];
	return ( rTrack.m_afSpeakerLevels[ 1 ] + rTrack.m_afSpeakerLevels[ 0 ] ) * 0.5f;
}

// $deBlob: FUNCTION 006067d0
void AMusicManager::SetTrackPan( TINT a_iTrackNumber, TFLOAT a_fPan )
{
	if ( m_pStreamSettings->m_iNumTracks >= a_iTrackNumber )
	{
		Track& rTrack = m_pStreamSettings->m_aoTracks[ a_iTrackNumber ];

		if ( rTrack.m_ui8Flags & TRACK_2D )
		{
			TMath::Clip( a_fPan, -1.0f, 1.0f );
			rTrack.m_afSpeakerLevels[ 7 ] = a_fPan;
			a_fPan                        = TMath::Abs( a_fPan );

			if ( rTrack.m_afSpeakerLevels[ 7 ] <= 0.0f )
			{
				rTrack.m_afSpeakerLevels[ 0 ] = 1.0f;
				rTrack.m_afSpeakerLevels[ 1 ] = 1.0f - a_fPan;
			}
			else
			{
				rTrack.m_afSpeakerLevels[ 1 ] = 1.0f;
				rTrack.m_afSpeakerLevels[ 0 ] = 1.0f - a_fPan;
			}
		}
	}
}

// $deBlob: FUNCTION 00606890
void AMusicManager::SetTrackPosition( TINT a_iTrackNumber, const TVector4& a_rcPosition )
{
	if ( a_iTrackNumber <= m_pStreamSettings->m_iNumTracks )
	{
		TASSERT( a_iTrackNumber < AMUSICMANAGER_MAX_TRACKS );

		m_pStreamSettings->m_aoTracks[ a_iTrackNumber ].m_ui8Flags |= TRACK_POSITION;
		m_pStreamSettings->m_aoTracks[ a_iTrackNumber ].m_vPosition = a_rcPosition;
	}
}

// $deBlob: FUNCTION 00606900
void AMusicManager::SetPercentage( TFLOAT a_fPercentage )
{
	m_fPercentage = a_fPercentage;
	TMath::Clip( m_fPercentage, 0.0f, 100.0f );

	if ( ( m_ui32StreamFlags & SET_START_SILENT ) &&
	     ( m_fPercentage > 0.0f ||
	       ( ( m_ui32StreamFlags & SET_SNAPSHOT ) && m_pSnapshotKeyFrame != TNULL && TStringManager::String8CompareNoCase( "inked", m_pSnapshotKeyFrame->m_szName ) == 0 ) ||
	       ( ( m_ui32StreamFlags & SET_SNAPSHOT ) && m_pSnapshotKeyFrame != TNULL && TStringManager::String8CompareNoCase( "rainbow", m_pSnapshotKeyFrame->m_szName ) == 0 ) ) )
	{
		if ( m_pChannel != TNULL )
		{
			m_ui32StreamFlags &= ~SET_START_SILENT;
			m_pChannel->setPaused( false );
		}
	}

	if ( ( m_ui32StreamFlags & SET_SILENT ) && m_fPercentage > 0.0f )
	{
		m_ui32StreamFlags &= ~SET_SILENT;

		if ( !( m_ui32StreamFlags & SET_SNAPSHOT ) || m_pSnapshotKeyFrame == TNULL || TStringManager::String8CompareNoCase( "inked", m_pSnapshotKeyFrame->m_szName ) != 0 )
			ReleaseSnapshot();
	}
}

// $deBlob: FUNCTION 00606a00
void AMusicManager::SetTrackFlags( TINT a_iTrackNumber, TUINT8 a_ui8Flags, TBOOL a_bSet )
{
	if ( a_bSet )
		m_pStreamSettings->m_aoTracks[ a_iTrackNumber ].m_ui8Flags |= a_ui8Flags;
	else
		m_pStreamSettings->m_aoTracks[ a_iTrackNumber ].m_ui8Flags &= ~a_ui8Flags;
}

// $deBlob: FUNCTION 00606a40
TUINT8 AMusicManager::GetTrackFlags( TINT a_iTrackNumber )
{
	if ( m_pStreamSettings != TNULL && a_iTrackNumber <= m_pStreamSettings->m_iNumTracks )
		return m_pStreamSettings->m_aoTracks[ a_iTrackNumber ].m_ui8Flags;

	return 0;
}

// $deBlob: FUNCTION 00606a70
void AMusicManager::ReleaseSnapshot()
{
	if ( m_ui32StreamFlags & SET_RAINBOW )
		return;

	TBOOL bInked = TFALSE;

	if ( ( m_ui32StreamFlags & SET_SNAPSHOT ) && m_pSnapshotKeyFrame != TNULL && TStringManager::String8CompareNoCase( "inked", m_pSnapshotKeyFrame->m_szName ) == 0 )
		bInked = TTRUE;

	if ( m_ui32StreamFlags & SET_SILENT )
	{
		if ( bInked && TSound::GetSingleton()->IsInitialised() )
		{
			SnapshotKeyFrame* pKeyFrame = FindSnapshotKeyFrame( "silent" );

			if ( pKeyFrame != TNULL )
				ApplySnapshotKeyFrame( pKeyFrame );
		}
	}
	else if ( m_iBossLevel < 0 || !PlayBossLevelSnapshot() )
	{
		if ( m_ui32StreamFlags & SET_SNAPSHOT )
		{
			if ( m_pStreamSettings->m_pChannelGroup != TNULL && ( m_ui32StreamFlags & SET_PITCH_CHANGE_KEEP ) )
				ResetPitch( m_pSnapshotKeyFrame->m_fBlendOutTime );

			m_fSnapshotBlendTime  = m_pSnapshotKeyFrame->m_fBlendOutTime;
			m_fSnapshotBlendTimer = 0.0f;
			m_ui32StreamFlags     = ( m_ui32StreamFlags & ~SET_SNAPSHOT_BLEND_IN ) | SET_SNAPSHOT_BLEND_OUT;
		}
	}
}

// $deBlob: FUNCTION 00606b70
void AMusicManager::FreeAllSyncSounds()
{
	SyncSound* apLists[ 2 ] = { m_pSSUsedList, m_pSSMusicBarUsedList };

	for ( TINT i = 0; i < 2; i++ )
	{
		SyncSound* pList = apLists[ i ];

		if ( pList == TNULL )
			continue;

		SyncSound* pLast = pList;

		while ( pLast->m_pNext != TNULL )
			pLast = pLast->m_pNext;

		if ( m_pSSFreeList != TNULL )
			m_pSSFreeList->m_pPrev = pList;

		pLast->m_pNext = m_pSSFreeList;
		m_pSSFreeList  = pList;
		pList->m_pPrev = TNULL;
	}

	m_pSSUsedList         = TNULL;
	m_pSSMusicBarUsedList = TNULL;
}

// $deBlob: FUNCTION 00606c00
void AMusicManager::StopColourSounds()
{
	while ( m_oColourSoundList.Size() > 0 )
	{
		ColourSoundEvent* pColourSound = m_oColourSoundList.Head();
		pColourSound->Remove();

		if ( pColourSound->m_pEvent != TNULL )
			pColourSound->m_pEvent->stop( true );

		m_oFreeColourSoundList.PushBack( pColourSound );
	}
}

// $deBlob: FUNCTION 00606c90
void AMusicManager::StopPLoops()
{
	while ( m_oPLoopList.Size() > 0 )
	{
		PLoop* pPLoop = m_oPLoopList.Head();
		pPLoop->Remove();

		if ( pPLoop->m_pEvent != TNULL )
		{
			pPLoop->m_pEvent->stop( true );
			pPLoop->m_pEvent = TNULL;
		}

		m_oFreePLoopList.PushBack( pPLoop );
	}
}

// $deBlob: FUNCTION 00606d10
void AMusicManager::StopPLoop( const TCHAR* a_szName )
{
	for ( auto it = m_oPLoopList.Begin(); it != m_oPLoopList.End(); )
	{
		PLoop* pPLoop = it;
		it++;

		if ( pPLoop->m_pEvent == TNULL )
			continue;

		char* szName;

		if ( pPLoop->m_pEvent->getInfo( TNULL, &szName, TNULL ) != FMOD_OK || T2String8::CompareNoCase( a_szName, szName ) == 0 )
		{
			pPLoop->m_pEvent->stop( false );
			pPLoop->Remove();
			m_oFreePLoopList.PushBack( pPLoop );
		}
	}
}

// $deBlob: FUNCTION 00606de0
void AMusicManager::SetPLoopParameter( const TCHAR* a_szName, TFLOAT a_fValue )
{
	for ( auto it = m_oPLoopList.Begin(); it != m_oPLoopList.End(); )
	{
		PLoop* pPLoop = it;
		it++;

		if ( pPLoop->m_pEvent == TNULL )
			continue;

		char* szName;

		if ( pPLoop->m_pEvent->getInfo( TNULL, &szName, TNULL ) != FMOD_OK || T2String8::CompareNoCase( a_szName, szName ) != 0 )
			continue;

		FMOD::EventParameter* pParameter     = TNULL;
		TINT                  iNumParameters = 0;
		pPLoop->m_pEvent->getNumParameters( &iNumParameters );

		// The primary parameter
		if ( pPLoop->m_pEvent->getParameterByIndex( -1, &pParameter ) == FMOD_OK )
		{
			TFLOAT fMin = 0.0f;
			TFLOAT fMax = 0.0f;
			pParameter->getRange( &fMin, &fMax );
			TMath::Clip( a_fValue, fMin, fMax );
			pParameter->setValue( a_fValue );
		}
	}
}

// $deBlob: FUNCTION 00606f00
void AMusicManager::Update( TFLOAT a_fDeltaTime )
{
	FMOD_OPENSTATE eOpenState = FMOD_OPENSTATE_READY;

	if ( m_pSound == TNULL )
		return;

	TUINT       uiPercentBuffered;
	bool        bStarving = false;
	bool        bDiskBusy = false;
	FMOD_RESULT eResult   = m_pSound->getOpenState( &eOpenState, &uiPercentBuffered, &bStarving, &bDiskBusy );

	if ( ( m_ui32StreamFlags & SET_PLAY_STREAM ) && ( eOpenState == FMOD_OPENSTATE_READY || eOpenState == FMOD_OPENSTATE_PLAYING ) )
	{
		TBOOL bPaused = TFALSE;

		if ( ( m_ui32StreamFlags & SET_MUSIC_PAUSED ) || ( m_ui32StreamFlags & SET_START_PAUSED ) )
			bPaused = TTRUE;

		PlayStream( bPaused );
		m_ui32StreamFlags &= ~SET_PLAY_STREAM;
	}

	if ( m_pChannel != TNULL )
	{
		if ( bStarving )
		{
			m_pChannel->setMute( true );
			return;
		}

		bool bMute = false;
		m_pChannel->getMute( &bMute );

		if ( bMute )
			m_pChannel->setMute( false );
	}

	if ( eOpenState == FMOD_OPENSTATE_ERROR || eResult == FMOD_ERR_FILE_DISKEJECTED )
	{
		if ( ( m_ui32StreamFlags & SET_PLAY_STREAM ) && m_bRestartStream )
			RestartStream();

		return;
	}

	if ( eOpenState == FMOD_OPENSTATE_READY || eOpenState == FMOD_OPENSTATE_PLAYING )
		m_bRestartStream = TFALSE;

	// The fades
	if ( m_pChannel != TNULL && ( m_ui32StreamFlags & ( SET_FADE_IN | SET_FADE_OUT ) ) )
	{
		if ( m_ui32StreamFlags & SET_FADE_IN )
		{
			bool bPaused;
			m_pChannel->getPaused( &bPaused );

			if ( bPaused == true )
			{
				m_pChannel->setPaused( false );
				SetPLoopPaused( TFALSE );
			}
		}

		if ( m_fFadeTime > m_fFadeTimer )
		{
			TFLOAT fProgress = m_fFadeTimer / m_fFadeTime;
			TFLOAT fVolume2D;
			TFLOAT fVolume3D;

			if ( !( m_ui32StreamFlags & SET_FADE_IN ) )
			{
				fVolume2D = TMath::LERPClamped( m_fFadeVolume2D, 0.0f, fProgress );
				fVolume3D = TMath::LERPClamped( m_fFadeVolume3D, 0.0f, fProgress );
			}
			else
			{
				fVolume2D = TMath::LERPClamped( m_fFadeVolume2D, m_fTargetVolume2D, fProgress );
				fVolume3D = TMath::LERPClamped( m_fFadeVolume3D, m_fTargetVolume3D, fProgress );
			}

			// NOTE: the fade volumes are updated too, so the fade isn't linear
			m_fVolume2D     = fVolume2D;
			m_fFadeVolume2D = fVolume2D;
			m_fVolume3D     = fVolume3D;
			m_fFadeVolume3D = fVolume3D;
			SetPLoopVolume( fVolume3D );
			m_fFadeTimer += a_fDeltaTime;
		}
		else
		{
			if ( m_ui32StreamFlags & SET_FADE_IN )
			{
				m_fVolume2D     = m_fTargetVolume2D;
				m_fFadeVolume2D = m_fTargetVolume2D;
				m_fVolume3D     = m_fTargetVolume3D;
				m_fFadeVolume3D = m_fTargetVolume3D;
				SetPLoopVolume( 1.0f );
				m_ui32StreamFlags &= ~SET_MUSIC_PAUSED;
			}
			else
			{
				m_fVolume2D     = 0.0f;
				m_fFadeVolume2D = 0.0f;
				m_fVolume3D     = 0.0f;
				m_fFadeVolume3D = 0.0f;
				SetPLoopVolume( 0.0f );
				SetPLoopPaused( TTRUE );
				m_pChannel->setPaused( true );
				TASSERT( 0 != ( m_ui32StreamFlags & SET_MUSIC_PAUSED ) );
			}

			m_ui32StreamFlags &= ~( SET_FADE_IN | SET_FADE_OUT );
		}
	}

	// The time outs of the sync sounds
	SyncSound* apLists[ 2 ] = { m_pSSUsedList, m_pSSMusicBarUsedList };

	for ( TINT i = 0; i < 2; i++ )
	{
		for ( SyncSound* pSyncSound = apLists[ i ]; pSyncSound != TNULL; )
		{
			SyncSound* pNext = pSyncSound->m_pNext;

			if ( pSyncSound->m_fTimeOut >= 0.0f )
				pSyncSound->m_fTimeOut -= a_fDeltaTime;
			else
				ReleaseSyncSound( pSyncSound );

			pSyncSound = pNext;
		}
	}

	// The pitch
	TUINT uiPosition = 0;

	if ( m_pChannel != TNULL )
	{
		m_pChannel->getPosition( &uiPosition, FMOD_TIMEUNIT_MS );
		m_uiPosition = uiPosition;
		{
			static TUINT s_uiLastLog = 0;
		}

		if ( m_pStreamSettings->m_pChannelGroup != TNULL && ( m_ui32StreamFlags & SET_PITCH_CHANGE ) )
		{
			TFLOAT fPitch;

			if ( m_fPitchTimer <= m_fPitchTime )
			{
				TFLOAT fRemaining = ( m_fPitchTime - m_fPitchTimer ) / m_fPitchTime;
				fPitch            = ( 1.0f - fRemaining ) * m_fTargetPitch + m_fPitch * fRemaining;
			}
			else
			{
				fPitch = m_fTargetPitch;
				m_ui32StreamFlags &= ~SET_PITCH_CHANGE;
				m_fPitch = fPitch;

				if ( m_ui32StreamFlags & SET_PITCH_THEN_PAUSE )
				{
					m_ui32StreamFlags &= ~SET_PITCH_THEN_PAUSE;
					PauseMusic( TTRUE, 0.0f );
				}
			}

			m_pStreamSettings->m_pChannelGroup->setPitch( fPitch );
			m_fPitchTimer += a_fDeltaTime;
		}
	}

	// The melody sections and the chords
	TUINT uiOldSection = m_uiSection;
	m_uiChord          = GetChord( uiPosition );
	m_uiSection        = GetSection( uiPosition );

	if ( uiOldSection != m_uiSection )
		m_pGUIMusicController->OnMelodyChange();

	// The sync points
	TUINT uiTime  = uiPosition + s_uiSyncPointLatency;
	bool  bPaused = true;

	if ( m_pChannel != TNULL )
	{
		m_pChannel->getPaused( &bPaused );

		if ( !bPaused )
		{
			if ( !( m_ui32StreamFlags & SET_BAR ) && !( m_ui32StreamFlags & SET_PITCH_CHANGE_KEEP ) &&
			     FindNextSyncPoint( uiTime, &m_iBar, &m_uiNextBar, &m_uiPrevBar, m_pSyncPoints->m_puiBars, m_pSyncPoints->m_iNumBars ) )
			{
				m_ui32StreamFlags |= SET_BAR;
			}

			if ( !( m_ui32StreamFlags & SET_BEAT ) && !( m_ui32StreamFlags & SET_PITCH_CHANGE_KEEP ) &&
			     FindNextSyncPoint( uiTime, &m_iBeat, &m_uiNextBeat, &m_uiPrevBeat, m_pSyncPoints->m_puiBeats, m_pSyncPoints->m_iNumBeats ) )
			{
				m_ui32StreamFlags |= SET_BEAT;
			}

			if ( !( m_ui32StreamFlags & SET_SCRATCH ) && !( m_ui32StreamFlags & SET_PITCH_CHANGE_KEEP ) &&
			     FindNextSyncPoint( uiTime, &m_iScratch, &m_uiNextScratch, &m_uiPrevScratch, m_pSyncPoints->m_puiScratches, m_pSyncPoints->m_iNumScratches ) )
			{
				m_ui32StreamFlags |= SET_SCRATCH;
			}

			TBOOL bBarSyncSound = TFALSE;

			if ( m_ui32StreamFlags & SET_BAR )
			{
				if ( uiTime < m_uiNextBar )
				{
					if ( uiTime < m_uiPrevBar )
						m_ui32StreamFlags &= ~SET_BAR;
				}
				else
				{
					m_ui32StreamFlags &= ~SET_BAR;
					bBarSyncSound = ProcessSyncSounds( m_pSSMusicBarUsedList );
					m_pGUIMusicController->OnBar();

					if ( s_bBarFeedback )
					{
						// The players' blobs squash on the bar
						TINT iNumPlayers = APlayerManager::GetSingleton()->GetNumPlayers();

						for ( TINT i = 0; i < iNumPlayers; i++ )
						{
#ifdef DEBLOB_COMMUNITY_PATCH
							// NOTE: after quitting a level the player count stays 1 without a blob
							if ( ABaseEntity* pBlob = APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( i ) ) )
								TSTATICCAST( ABlobEntity, pBlob )->OnMusicBar();
#else
							TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( i ) ) )->OnMusicBar();
#endif
						}
					}
				}
			}

			if ( ( m_ui32StreamFlags & SET_BEAT ) && !( m_ui32StreamFlags & SET_PITCH_CHANGE ) )
			{
				if ( uiTime < m_uiNextBeat )
				{
					if ( uiTime < m_uiPrevBeat )
						m_ui32StreamFlags &= ~SET_BEAT;
				}
				else
				{
					m_ui32StreamFlags &= ~SET_BEAT;
					m_pGUIMusicController->OnBeat();

					if ( s_bBeatFeedback )
					{
						// The players' blobs squash on the beat
						TINT iNumPlayers = APlayerManager::GetSingleton()->GetNumPlayers();

						for ( TINT i = 0; i < iNumPlayers; i++ )
						{
#ifdef DEBLOB_COMMUNITY_PATCH
							// NOTE: after quitting a level the player count stays 1 without a blob
							if ( ABaseEntity* pBlob = APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( i ) ) )
								TSTATICCAST( ABlobEntity, pBlob )->OnMusicBeat();
#else
							TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( i ) ) )->OnMusicBeat();
#endif
						}
					}

					if ( !bBarSyncSound )
						ProcessSyncSounds( m_pSSUsedList );
				}
			}

			if ( m_ui32StreamFlags & SET_SCRATCH )
			{
				if ( uiTime < m_uiNextScratch )
				{
					if ( uiTime < m_uiPrevScratch )
						m_ui32StreamFlags &= ~SET_SCRATCH;
				}
				else
				{
					m_pGUIMusicController->OnScratch();
					m_ui32StreamFlags &= ~SET_SCRATCH;
				}
			}
		}
	}

	// The keyframes
	if ( eOpenState == FMOD_OPENSTATE_READY || eOpenState == FMOD_OPENSTATE_PLAYING )
	{
		if ( !( m_ui32StreamFlags & SET_NO_PERCENTAGE ) &&
		     ( m_iBossLevel < 0 ||
		       ( ( !( m_ui32StreamFlags & SET_SNAPSHOT ) || m_pSnapshotKeyFrame == TNULL || TStringManager::String8CompareNoCase( "inked", m_pSnapshotKeyFrame->m_szName ) != 0 ) && !PlayBossLevelSnapshot() ) ) )
		{
			UpdatePercentage();
		}

		UpdateTrackLevels( a_fDeltaTime );

		// NOTE: retail updates the levels twice while the stream plays
		if ( eOpenState == FMOD_OPENSTATE_PLAYING )
			UpdateTrackLevels( a_fDeltaTime );
	}
}

// $deBlob: FUNCTION 00607700
void AMusicManager::UpdatePercentage()
{
	TFLOAT fScore = TFLOAT( m_iScore ) / TFLOAT( m_iMaxScore );
	TMath::Clip( fScore, 0.0f, 1.0f );
	SetPercentage( fScore * 100.0f );
}

// $deBlob: FUNCTION 00607760
void AMusicManager::SetBossLevel( TINT a_iBossLevel )
{
	m_iBossLevel = a_iBossLevel;

	if ( !( m_ui32StreamFlags & SET_SNAPSHOT ) || m_pSnapshotKeyFrame == TNULL || TStringManager::String8CompareNoCase( "inked", m_pSnapshotKeyFrame->m_szName ) != 0 )
		PlayBossLevelSnapshot();
}

// $deBlob: FUNCTION 006077a0
TBOOL AMusicManager::PlayBossLevelSnapshot()
{
	if ( m_iBossLevel < 0 )
		return TFALSE;

	TCHAR szName[ 128 ];

	if ( m_iBossLevel >= 4 )
		T2String8::Format( szName, "bosslevelfinal", 128 );
	else
		T2String8::Format( szName, "bosslevel%d", m_iBossLevel + 1 );

	SnapshotKeyFrame* pKeyFrame = FindSnapshotKeyFrame( szName );

	if ( pKeyFrame == TNULL )
		return TFALSE;

	if ( pKeyFrame != m_pSnapshotKeyFrame )
		ApplySnapshotKeyFrame( pKeyFrame );

	return TTRUE;
}

// $deBlob: FUNCTION 00607840
TBOOL AMusicManager::AddSyncSound( TINT a_iEventID, TFLOAT a_fTimeOut )
{
	return AddSyncSound( -1, a_iEventID, a_fTimeOut, SS_DEFAULT, &TVector4::VEC_ZERO, TNULL, TNULL, 0 );
}

// $deBlob: FUNCTION 00607870
TBOOL AMusicManager::AddSyncSound( TINT a_iGroupID, TINT a_iEventID, TFLOAT a_fTimeOut, TUINT32 a_uiFlags, const TVector4* a_pPosition, const TCHAR* a_szGroup, const TCHAR* a_szSnapshot, TINT a_iParam )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TTRUE;

	if ( m_pSSFreeList == TNULL )
		return TFALSE;

	SyncSound* pList  = ( a_uiFlags & SS_MUSIC_BAR ) ? m_pSSMusicBarUsedList : m_pSSUsedList;
	TUINT32    bFront = a_uiFlags & SS_PUSH_FRONT;

	if ( !bFront && pList != TNULL )
	{
		while ( pList->m_pNext != TNULL )
			pList = pList->m_pNext;
	}

	if ( ( a_uiFlags & SS_UNIQUE ) && !( a_uiFlags & SS_BUILDING_COLOUR ) )
	{
		if ( IsSyncSoundQueued( a_iGroupID, a_iEventID, a_uiFlags, a_szGroup ) )
			return TFALSE;
	}

	SyncSound* pSyncSound = m_pSSFreeList;
	pSyncSound->m_pPrev   = TNULL;
	m_pSSFreeList         = m_pSSFreeList->m_pNext;

	if ( m_pSSFreeList != TNULL )
		m_pSSFreeList->m_pPrev = TNULL;

	if ( pList != TNULL )
	{
		if ( bFront )
			pList->m_pPrev = pSyncSound;
		else
			pList->m_pNext = pSyncSound;
	}

	pSyncSound->m_iGroupID = TINT16( a_iGroupID );
	pSyncSound->m_iEventID = TINT16( a_iEventID );

	if ( bFront )
	{
		pSyncSound->m_pNext = pList;
		pSyncSound->m_pPrev = TNULL;
	}
	else
	{
		pSyncSound->m_pPrev = pList;
		pSyncSound->m_pNext = TNULL;
	}

	pSyncSound->m_uiFlags           = a_uiFlags;
	pSyncSound->m_iParam            = a_iParam;
	pSyncSound->m_pSnapshotKeyFrame = TNULL;
	pSyncSound->m_vPosition         = *a_pPosition;
	pSyncSound->m_fTimeOut          = a_fTimeOut;

	if ( a_uiFlags & SS_GROUP_NAME )
		pSyncSound->m_szGroup = a_szGroup;

	if ( a_uiFlags & SS_SNAPSHOT )
		pSyncSound->m_pSnapshotKeyFrame = FindSnapshotKeyFrame( a_szSnapshot );

	if ( bFront )
	{
		if ( a_uiFlags & SS_MUSIC_BAR )
		{
			m_pSSMusicBarUsedList = pSyncSound;
			TASSERT( m_pSSMusicBarUsedList->m_pNext != m_pSSMusicBarUsedList );
		}
		else
		{
			m_pSSUsedList = pSyncSound;
			TASSERT( m_pSSUsedList->m_pNext != m_pSSUsedList );
		}
	}
	else if ( pList == TNULL )
	{
		if ( a_uiFlags & SS_MUSIC_BAR )
		{
			m_pSSMusicBarUsedList = pSyncSound;
			TASSERT( m_pSSMusicBarUsedList->m_pNext != m_pSSMusicBarUsedList );
		}
		else
		{
			m_pSSUsedList = pSyncSound;
			TASSERT( m_pSSUsedList->m_pNext != m_pSSUsedList );
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00607b00
TBOOL AMusicManager::IsSyncSoundQueued( TINT a_iGroupID, TINT a_iEventID, TUINT32 a_uiFlags, const TCHAR* a_szGroup )
{
	TBOOL bByGroupName = TFALSE;

	if ( a_szGroup != TNULL && ( a_uiFlags & SS_GROUP_NAME ) )
		bByGroupName = TTRUE;

	SyncSound* apLists[ 2 ] = { m_pSSUsedList, m_pSSMusicBarUsedList };

	for ( TINT i = 0; i < 2; i++ )
	{
		for ( SyncSound* pSyncSound = apLists[ i ]; pSyncSound != TNULL; pSyncSound = pSyncSound->m_pNext )
		{
			if ( pSyncSound->m_iEventID != a_iEventID )
				continue;

			if ( !bByGroupName && pSyncSound->m_iGroupID == a_iGroupID )
				return TTRUE;

			if ( pSyncSound->m_szGroup != TNULL && bByGroupName && T2String8::Compare( a_szGroup, pSyncSound->m_szGroup ) == 0 )
				return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00607ba0
TBOOL AMusicManager::AddPaintSyncSound( TINT a_iColour, ABaseEntity* a_pBlob, const TVector4* a_pPosition )
{
	// Only the sound of the player with the highest paint (in the modes other than -1, 1 and 2)
	TBOOL bPlay = TTRUE;

	TINT iGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();

	if ( iGameType != -1 && iGameType != 1 && iGameType != 2 )
	{
		TINT iMaxPoints  = -1;
		TINT iNumPlayers = APlayerManager::GetSingleton()->GetNumPlayers();
		TINT iBlobPoints = -1;

		if ( iNumPlayers > 0 )
		{
			for ( TINT i = 0; i < iNumPlayers; i++ )
			{
				ABaseEntity* pBlob   = APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( i ) );
				TINT         iPoints = AGameStatus::GetSingleton()->GetLevelData().GetPoints( i );

				if ( iMaxPoints < iPoints )
					iMaxPoints = iPoints;

				if ( pBlob == a_pBlob )
					iBlobPoints = iPoints;
			}

			bPlay = iBlobPoints == iMaxPoints;
		}
	}

	if ( !bPlay )
		return TFALSE;

	if ( TUINT( a_iColour - 8 ) <= 7 )
		a_iColour = 7;

	return AddSyncSound( 0, 0, 10.0f, SS_BUILDING_COLOUR | SS_MUSIC_PROJECT | SS_GROUP_NAME | SS_POSITION, a_pPosition, TNULL, TNULL, a_iColour );
}

// $deBlob: FUNCTION 00607cb0
TBOOL AMusicManager::AddColourSyncSound( TINT a_iColour, const TVector4* a_pPosition )
{
	if ( TUINT( a_iColour - 8 ) <= 7 )
		a_iColour = 7;

	return AddSyncSound( 0, 0, 10.0f, SS_BUILDING_COLOUR | SS_MUSIC_PROJECT | SS_GROUP_NAME | SS_POSITION, a_pPosition, TNULL, TNULL, a_iColour );
}

// $deBlob: FUNCTION 00607cf0
TBOOL AMusicManager::PlayGoodCombo()
{
	if ( ( m_ui32StreamFlags & SET_SNAPSHOT ) && m_pSnapshotKeyFrame != TNULL && TStringManager::String8CompareNoCase( "inked", m_pSnapshotKeyFrame->m_szName ) == 0 )
		return TFALSE;

	TUINT32 uiFlags = SS_SNAPSHOT | SS_RELEASE_SNAPSHOT | SS_UNIQUE | SS_DEFAULT;

	if ( TUINT( m_pSyncPoints->m_iNumBars ) > 0 )
		uiFlags |= SS_MUSIC_BAR;

	PlayMusicSFX( "good_combo", uiFlags, &TVector4::VEC_ZERO, "combo" );
	return TTRUE;
}

// $deBlob: FUNCTION 00607d50
TBOOL AMusicManager::PlayLevelComplete()
{
	if ( ( m_ui32StreamFlags & SET_SNAPSHOT ) && m_pSnapshotKeyFrame != TNULL && TStringManager::String8CompareNoCase( "inked", m_pSnapshotKeyFrame->m_szName ) == 0 )
		return TFALSE;

	if ( TSound::GetSingleton()->IsInitialised() )
	{
		SnapshotKeyFrame* pKeyFrame = FindSnapshotKeyFrame( "silent" );

		if ( pKeyFrame != TNULL )
			ApplySnapshotKeyFrame( pKeyFrame );
	}

	if ( TSound::GetSingleton()->IsInitialised() && m_pMusicSFXInfo != TNULL )
	{
		FMOD::Event* pEvent = GetMusicSFXEvent( "level_complete", TFALSE );

		if ( pEvent != TNULL )
			pEvent->start();
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00607dd0
void AMusicManager::PlayConversation()
{
	if ( m_ui32StreamFlags & SET_SILENT )
		return;

	if ( ( m_ui32StreamFlags & SET_SNAPSHOT ) && m_pSnapshotKeyFrame != TNULL && TStringManager::String8CompareNoCase( "inked", m_pSnapshotKeyFrame->m_szName ) == 0 )
		return;

	if ( !TSound::GetSingleton()->IsInitialised() )
		return;

	SnapshotKeyFrame* pKeyFrame = FindSnapshotKeyFrame( s_szConversation );

	if ( pKeyFrame != TNULL )
	{
		ApplySnapshotKeyFrame( pKeyFrame );
		return;
	}

	// No conversation snapshot in the music data: the volumes of the end score marker
	TASSERT( 1 < m_pScoreMarkerData->m_iNumValues && "a_iMarkerNumber < m_pScoreMarkerData->m_iNumValues" );
	TFLOAT fMarker = m_pScoreMarkerData->m_pfValues[ SCORE_MUSIC_END ];

	if ( m_fPercentage > fMarker )
	{
		GetRegularKeyFrameVolumes( m_pTempKeyFrame->m_pfVolumes, fMarker );

		TFLOAT fBlendTime                = m_pKeyFrameList->m_pConversationKeyFrame->m_fBlendInTime;
		m_pTempKeyFrame->m_fBlendInTime  = fBlendTime;
		m_pTempKeyFrame->m_fBlendOutTime = fBlendTime;
		m_pTempKeyFrame->m_fPitch        = 1.0f;
		m_pTempKeyFrame->m_eBlendMode    = BLENDMODE_LERP;
		m_pTempKeyFrame->m_szName        = s_szConversation;
		ApplySnapshotKeyFrame( m_pTempKeyFrame );
	}
}

// $deBlob: FUNCTION 00607f00
void AMusicManager::ReleaseSyncSound( SyncSound* a_pSyncSound )
{
	TBOOL      bFound       = TFALSE;
	TBOOL      bHead        = TFALSE;
	SyncSound* apLists[ 2 ] = { m_pSSUsedList, m_pSSMusicBarUsedList };

	for ( TINT i = 0; i < 2 && !bFound; i++ )
	{
		for ( SyncSound* pSyncSound = apLists[ i ]; pSyncSound != TNULL && !bFound; )
		{
			if ( pSyncSound == a_pSyncSound )
			{
				if ( pSyncSound->m_pPrev != TNULL )
					pSyncSound->m_pPrev->m_pNext = pSyncSound->m_pNext;
				else
					bHead = TTRUE;

				if ( pSyncSound->m_pNext != TNULL )
					pSyncSound->m_pNext->m_pPrev = pSyncSound->m_pPrev;

				bFound = TTRUE;
			}
			else
			{
				pSyncSound = pSyncSound->m_pNext;
			}
		}

		if ( bHead )
		{
			SyncSound* pNext = a_pSyncSound->m_pNext;

			if ( apLists[ i ] == m_pSSUsedList )
				m_pSSUsedList = pNext;
			else
				m_pSSMusicBarUsedList = pNext;

			if ( pNext != TNULL )
				pNext->m_pPrev = TNULL;
		}
	}

	a_pSyncSound->m_pPrev = TNULL;
	a_pSyncSound->m_pNext = m_pSSFreeList;

	if ( m_pSSFreeList != TNULL )
		m_pSSFreeList->m_pPrev = a_pSyncSound;

	m_pSSFreeList = a_pSyncSound;
}

// $deBlob: FUNCTION 00607fe0
void AMusicManager::CloseSoundTRB()
{
	m_oTRBSound.Close();
}

// $deBlob: FUNCTION 00607ff0
void AMusicManager::UnloadMusicData()
{
	m_iMaxScore = 10000;
	ResetSyncPoints();
	FreeAllSyncSounds();
	StopColourSounds();
	StopPLoops();
	m_oTRBSyncPoints.Close();
	PauseMusic( TTRUE, 0.0f );

	if ( m_pSound != TNULL )
	{
		FMOD::Sound* pParent = TNULL;
		m_pSound->getSubSoundParent( &pParent );

		if ( pParent != TNULL )
			pParent->release();
		else
			m_pSound->release();

		m_pSound = TNULL;
	}

	m_ui32StreamFlags &= SET_MUSIC_OVERRIDE;
	m_fVolume2D           = m_fTargetVolume2D;
	m_iBeat               = 0;
	m_uiNextBeat          = 0;
	m_pSnapshotKeyFrame   = TNULL;
	m_uiUnknownB88        = 0;
	m_fPercentage         = 0.0f;
	m_fVolume3D           = m_fTargetVolume3D;
	m_fFadeTime           = 0.0f;
	m_fFadeTimer          = 0.0f;
	m_fSnapshotBlendTime  = 0.0f;
	m_fSnapshotBlendTimer = 0.0f;
	m_fFadeVolume3D       = 0.5f;
	m_fFadeVolume2D       = 0.5f;
	m_fDiskBusyTime       = 0.0f;

	TFree( m_pStreamSettings );
	m_pStreamSettings = TNULL;
	m_oTRBMusicData.Close();
}

// $deBlob: FUNCTION 00608140
void AMusicManager::ResetSpeakerLevels( FMOD::Channel* a_pChannel )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return;

	TBOOL         bTestSound = TFALSE;
	FMOD::Sound*  pSound     = TNULL;
	FMOD::Sound*  pSubSound  = TNULL;
	FMOD::System* pSystem    = TSound::GetSingleton()->GetSystem();

	if ( a_pChannel == TNULL )
	{
		FMOD_RESULT eResult = pSystem->createSound( "Data/Sound/EN/markertest.fsb", FMOD_HARDWARE, TNULL, &pSound );
		pSound->getSubSound( 0, &pSubSound );
		pSystem->playSound( FMOD_CHANNEL_FREE, pSubSound, true, &a_pChannel );
		TSound::ErrorCheck( eResult );
		bTestSound = TTRUE;
	}

	FMOD_SPEAKERMODE eSpeakerMode = FMOD_SPEAKERMODE_STEREO;
	TSound::ErrorCheck( pSystem->getSpeakerMode( &eSpeakerMode ) );

	// Each speaker plays its own channel
	TFLOAT aafLevels[ 6 ][ 6 ];

	for ( TINT i = 0; i < 6; i++ )
	{
		for ( TINT j = 0; j < 6; j++ )
			aafLevels[ i ][ j ] = ( j == i ) ? 1.0f : 0.0f;
	}

	TSound::ErrorCheck( a_pChannel->setSpeakerLevels( FMOD_SPEAKER_FRONT_LEFT, aafLevels[ 0 ], 6 ) );
	TSound::ErrorCheck( a_pChannel->setSpeakerLevels( FMOD_SPEAKER_FRONT_RIGHT, aafLevels[ 1 ], 6 ) );

	if ( eSpeakerMode == FMOD_SPEAKERMODE_SURROUND || eSpeakerMode == FMOD_SPEAKERMODE_5POINT1 )
	{
		TSound::ErrorCheck( a_pChannel->setSpeakerLevels( FMOD_SPEAKER_BACK_LEFT, aafLevels[ 4 ], 6 ) );
		TSound::ErrorCheck( a_pChannel->setSpeakerLevels( FMOD_SPEAKER_BACK_RIGHT, aafLevels[ 5 ], 6 ) );
		TSound::ErrorCheck( a_pChannel->setSpeakerLevels( FMOD_SPEAKER_FRONT_CENTER, aafLevels[ 2 ], 6 ) );
	}

	if ( bTestSound )
	{
		a_pChannel->stop();
		pSubSound->release();
		pSound->release();
	}
}

// $deBlob: FUNCTION 006082f0
void AMusicManager::SetDiskBusy( TBOOL a_bBusy )
{
	if ( a_bBusy )
	{
		FMOD_File_SetDiskBusy( 1 );

		if ( !( m_ui32StreamFlags & SET_DISK_BUSY ) )
			m_fDiskBusyTime = 0.0f;
	}
	else
	{
		FMOD_File_SetDiskBusy( 0 );
		m_ui32StreamFlags &= ~SET_DISK_BUSY;
		m_fDiskBusyTime = 0.0f;
	}
}

// $deBlob: FUNCTION 00608340
void AMusicManager::SetTrackMuted( TINT a_iTrackNumber, TBOOL a_bMuted )
{
	if ( m_pStreamSettings != TNULL && m_pStreamSettings->m_iNumTracks > a_iTrackNumber )
	{
		TASSERT( 0 != ( m_pStreamSettings->m_aoTracks[ a_iTrackNumber ].m_ui8Flags & TRACK_3D ) );

		if ( a_bMuted )
			m_pStreamSettings->m_aoTracks[ a_iTrackNumber ].m_ui8Flags |= TRACK_MUTED;
		else
			m_pStreamSettings->m_aoTracks[ a_iTrackNumber ].m_ui8Flags &= ~TRACK_MUTED;
	}
}

// $deBlob: FUNCTION 006083c0
// $deBlob: FUNCTION 00608790
void AMusicManager::PauseAll()
{
	bool bPaused = false;

	if ( m_pChannel != TNULL )
	{
		m_pChannel->getPaused( &bPaused );

		if ( !bPaused )
		{
			if ( !( m_ui32StreamFlags & SET_FADE_OUT ) )
				m_ui32StreamFlags |= SET_PAUSED_BY_STATES;

			PauseMusic( TTRUE, 0.0f );
		}
	}

	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( pController != TNULL )
	{
		Toshi::T2Vector<AGameState*, AGameStateController::MAX_NUM_STATES>& rStack = pController->GetStateStack();

		for ( TINT i = 0; i < rStack.Size(); i++ )
		{
			if ( rStack[ i ]->IsA( &TGetClass( AXUIState ) ) )
			{
				AXUIState* pState = TSTATICCAST( AXUIState, rStack[ i ] );
				PauseChannel( TTRUE, pState->GetChannel(), pState->GetChannelPausedFlag() );
			}
		}
	}
}

// $deBlob: FUNCTION 006085b0
void AMusicManager::ResumeAll()
{
	if ( m_ui32StreamFlags & SET_PAUSED_BY_STATES )
	{
		m_ui32StreamFlags &= ~SET_PAUSED_BY_STATES;

		if ( m_pChannel != TNULL )
			PauseMusic( TFALSE, 1.0f );
	}

	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( pController != TNULL )
	{
		Toshi::T2Vector<AGameState*, AGameStateController::MAX_NUM_STATES>& rStack = pController->GetStateStack();

		for ( TINT i = 0; i < rStack.Size(); i++ )
		{
			if ( rStack[ i ]->IsA( &TGetClass( AXUIState ) ) )
			{
				AXUIState* pState = TSTATICCAST( AXUIState, rStack[ i ] );
				PauseChannel( TFALSE, pState->GetChannel(), pState->GetChannelPausedFlag() );
			}
		}
	}
}

// $deBlob: FUNCTION 006087a0
void AMusicManager::ResumeAllAndRestart()
{
	m_bRestartStream = TTRUE;
	ResumeAll();
}

// $deBlob: FUNCTION 006087b0
void AMusicManager::PauseMusicNow()
{
	PauseMusic( TTRUE, 0.0f );
}

// $deBlob: FUNCTION 006087c0
void AMusicManager::AddScore( TINT a_iScore )
{
	m_iScore += a_iScore;

	if ( m_iScore > m_iMaxScore )
		m_iScore = m_iMaxScore;
	else if ( m_iScore < 0 )
		m_iScore = 0;
}

// $deBlob: FUNCTION 00608800
void AMusicManager::ResetScore()
{
	// Only in single player
	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE )
	{
		m_iScore = 0;

		if ( TSound::GetSingleton()->IsInitialised() )
		{
			SnapshotKeyFrame* pKeyFrame = FindSnapshotKeyFrame( "inked" );

			if ( pKeyFrame != TNULL )
				ApplySnapshotKeyFrame( pKeyFrame );
		}
	}
}

// $deBlob: FUNCTION 00608840
TBOOL AMusicManager::PlaySnapshot( const TCHAR* a_szName )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TTRUE;

	SnapshotKeyFrame* pKeyFrame = FindSnapshotKeyFrame( a_szName );

	if ( pKeyFrame != TNULL )
	{
		ApplySnapshotKeyFrame( pKeyFrame );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00608880
TINT AMusicManager::GetNumSnapshotKeyFrames()
{
	return m_pKeyFrameList->m_iNumSnapshotKeyFrames;
}

// $deBlob: FUNCTION 00608890
const TCHAR* AMusicManager::GetSnapshotKeyFrameName( TINT a_iIndex )
{
	TASSERT( a_iIndex >= 0 && a_iIndex <= m_pKeyFrameList->m_iNumSnapshotKeyFrames );
	return m_pKeyFrameList->m_pSnapshotKeyFrames[ a_iIndex ].m_szName;
}

// $deBlob: FUNCTION 00608900
const TCHAR* AMusicManager::GetCurrentSnapshotName()
{
	if ( m_pSnapshotKeyFrame != TNULL )
		return m_pSnapshotKeyFrame->m_szName;

	return TNULL;
}

// $deBlob: FUNCTION 00608910
TBOOL AMusicManager::IsInked()
{
	if ( ( m_ui32StreamFlags & SET_SNAPSHOT ) && m_pSnapshotKeyFrame != TNULL && TStringManager::String8CompareNoCase( "inked", m_pSnapshotKeyFrame->m_szName ) == 0 )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 00608940
TBOOL AMusicManager::IsSnapshotActive()
{
	return m_ui32StreamFlags & SET_SNAPSHOT;
}

// $deBlob: FUNCTION 00608950
TBOOL AMusicManager::IsSnapshot( const TCHAR* a_szName )
{
	if ( ( m_ui32StreamFlags & SET_SNAPSHOT ) && m_pSnapshotKeyFrame != TNULL && TStringManager::String8CompareNoCase( a_szName, m_pSnapshotKeyFrame->m_szName ) == 0 )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 00608990
TBOOL AMusicManager::IsConversation()
{
	if ( ( m_ui32StreamFlags & SET_SNAPSHOT ) && m_pSnapshotKeyFrame != TNULL && TStringManager::String8CompareNoCase( s_szConversation, m_pSnapshotKeyFrame->m_szName ) == 0 )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 006089c0
void AMusicManager::SetPosition( TUINT a_uiPosition )
{
	m_uiPosition = a_uiPosition;

	if ( m_pChannel != TNULL )
		m_pChannel->setPosition( a_uiPosition, FMOD_TIMEUNIT_MS );
}

// $deBlob: FUNCTION 006089f0
TUINT AMusicManager::GetPosition()
{
	TUINT uiPosition = 0;

	if ( m_pChannel == TNULL )
		return 0;

	m_pChannel->getPosition( &uiPosition, FMOD_TIMEUNIT_MS );
	return uiPosition;
}

// $deBlob: FUNCTION 00608a20
TFLOAT AMusicManager::GetScoreMarker( TINT a_iMarkerNumber )
{
	TASSERT( a_iMarkerNumber >= 0 && a_iMarkerNumber < SCORE_MUSIC_NumOF );
	TASSERT( a_iMarkerNumber < m_pScoreMarkerData->m_iNumValues );
	return m_pScoreMarkerData->m_pfValues[ a_iMarkerNumber ];
}

// $deBlob: FUNCTION 00608ab0
void AMusicManager::SetNoPercentage( TBOOL a_bNoPercentage )
{
	if ( a_bNoPercentage )
		m_ui32StreamFlags |= SET_NO_PERCENTAGE;
	else
		m_ui32StreamFlags &= ~SET_NO_PERCENTAGE;
}

// $deBlob: FUNCTION 00608ae0
TINT AMusicManager::GetNumTracks()
{
	if ( m_pStreamSettings != TNULL )
		return m_pStreamSettings->m_iNumTracks;

	return 0;
}

// $deBlob: FUNCTION 00608af0
void AMusicManager::GetTrackLevels( TINT a_iTrackNumber, TFLOAT* a_pLeft, TFLOAT* a_pRight )
{
	if ( m_pStreamSettings != TNULL && m_pStreamSettings->m_iNumTracks > a_iTrackNumber )
	{
		*a_pLeft  = m_pStreamSettings->m_aoTracks[ a_iTrackNumber ].m_afSpeakerLevels[ 0 ];
		*a_pRight = m_pStreamSettings->m_aoTracks[ a_iTrackNumber ].m_afSpeakerLevels[ 1 ];
	}
}

// $deBlob: FUNCTION 00608b30
TBOOL AMusicManager::IsSilent()
{
	return ( m_ui32StreamFlags & SET_SILENT ) != 0;
}

// $deBlob: FUNCTION 00608b40
void AMusicManager::ResetPitch( TFLOAT a_fTime )
{
	APositionalSoundManager::GetSingleton()->StopPitchedSounds();

	m_fTargetPitch    = 1.0f;
	m_fPitchTimer     = 0.0f;
	m_fPitchTime      = a_fTime;
	m_ui32StreamFlags = ( m_ui32StreamFlags & 0xDCDFFFDF ) | SET_PITCH_CHANGE;
}

// $deBlob: FUNCTION 00608b90
void AMusicManager::SetPitch( TFLOAT a_fPitch, TFLOAT a_fTime, TBOOL a_bPauseAfter )
{
	APositionalSoundManager::GetSingleton()->StopPitchedSounds();

	m_ui32StreamFlags = ( m_ui32StreamFlags & 0xDD5FFFDF ) | SET_PITCH_CHANGE | SET_PITCH_CHANGE_KEEP;

	if ( a_bPauseAfter )
		m_ui32StreamFlags |= SET_PITCH_THEN_PAUSE;
	else
		m_ui32StreamFlags &= ~SET_PITCH_THEN_PAUSE;

	TASSERT( a_fPitch > 0.0f );
	m_fTargetPitch = a_fPitch;
	m_fPitchTimer  = 0.0f;
	m_fPitchTime   = a_fTime;
}

// $deBlob: FUNCTION 00608c40
void AMusicManager::AddPitch( TFLOAT a_fPitch, TFLOAT a_fTime )
{
	TFLOAT fPitch = m_fPitch + a_fPitch;

	APositionalSoundManager::GetSingleton()->StopPitchedSounds();

	m_ui32StreamFlags = ( m_ui32StreamFlags & 0xDC5FFFDF ) | SET_PITCH_CHANGE | SET_PITCH_CHANGE_KEEP;
	TASSERT( fPitch > 0.0f && "a_fPitch > 0.0f" );
	m_fTargetPitch = fPitch;
	m_fPitchTimer  = 0.0f;
	m_fPitchTime   = a_fTime;
}

// $deBlob: FUNCTION 00608ce0
TBOOL AMusicManager::IsPitchChangeKept()
{
	return ( m_ui32StreamFlags & SET_PITCH_CHANGE_KEEP ) != 0;
}

// $deBlob: FUNCTION 00608cf0
void AMusicManager::SetPauseLocked( TBOOL a_bLocked )
{
	if ( a_bLocked )
		m_ui32StreamFlags |= SET_PAUSE_LOCKED;
	else
		m_ui32StreamFlags &= ~SET_PAUSE_LOCKED;
}

// $deBlob: FUNCTION 00608d20
void AMusicManager::SetMusicOverride( const TCHAR* a_szName )
{
	TASSERT( T2String8::Length( a_szName ) > 0 && "T2String8::Length(a_pName) > 0" );
	m_ui32StreamFlags |= SET_MUSIC_OVERRIDE;
	m_szMusicOverride = a_szName;
}

// $deBlob: FUNCTION 00608d80
void AMusicManager::SetBossMusicOverride()
{
	const TCHAR* szName = s_szBossTrack;
	TASSERT( T2String8::Length( szName ) > 0 && "T2String8::Length(a_pName) > 0" );
	m_ui32StreamFlags |= SET_MUSIC_OVERRIDE;
	m_szMusicOverride = szName;
}

// $deBlob: FUNCTION 00608de0
void AMusicManager::SetRandomMusicOverride()
{
	// The funk tracks, from a random one
	static const TINT s_aiRandomTracks[ 4 ] = { 1, 2, 6, 7 };
	static TINT       s_iRandomTrack        = ARandom::GetSingleton()->GetRandom().GetInt( 4 );

	const TCHAR* szName = s_aszTrackNames[ s_aiRandomTracks[ s_iRandomTrack ] ];
	TASSERT( T2String8::Length( szName ) > 0 && "T2String8::Length(a_pName) > 0" );

	m_ui32StreamFlags |= SET_MUSIC_OVERRIDE;
	m_szMusicOverride = szName;

	TINT iNext = s_iRandomTrack + 1;

	if ( iNext >= 4 )
		iNext = 0;

	s_iRandomTrack = iNext;
}

// $deBlob: FUNCTION 00608ee0
void AMusicManager::ClearMusicOverride()
{
	m_ui32StreamFlags &= ~SET_MUSIC_OVERRIDE;
	m_szMusicOverride = TNULL;
}

// $deBlob: FUNCTION 00608f00
TBOOL AMusicManager::UsingMusicOverride()
{
	return ( m_ui32StreamFlags & SET_MUSIC_OVERRIDE ) != 0;
}

// $deBlob: FUNCTION 00608f30
void AMusicManager::PlayGameOver()
{
	if ( m_pMusicSFXInfo != TNULL )
	{
		FMOD::Event* pEvent = GetMusicSFXEvent( "game_over", TFALSE );

		if ( pEvent != TNULL )
			pEvent->start();
	}
}

// $deBlob: FUNCTION 00608f50
void AMusicManager::PlayMusicSFX( const TCHAR* a_szName, TUINT32 a_uiFlags, const TVector4* a_pPosition, const TCHAR* a_szSnapshot )
{
	if ( !TSound::GetSingleton()->IsInitialised() || m_pMusicSFXInfo == TNULL )
		return;

	FMOD::Event* pEvent = GetMusicSFXEvent( a_szName, TFALSE );

	if ( pEvent == TNULL )
		return;

	if ( a_uiFlags & SS_START )
	{
		pEvent->start();
	}
	else if ( a_uiFlags & SS_START_POSITION )
	{
		pEvent->start();
		ASoundInterface::SetPosition( pEvent, *a_pPosition );
	}
	else
	{
		TINT iIndex = 0;
		pEvent->getInfo( &iIndex, TNULL, TNULL );
		AddSyncSound( -1, iIndex, 10.0f, a_uiFlags | SS_SFX_PROJECT | SS_GROUP_NAME, a_pPosition, m_pMusicSFXInfo->m_szGroup, a_szSnapshot, 0 );
	}

	if ( ( a_uiFlags & SS_PLOOP ) && ( a_uiFlags & ( SS_START | SS_START_POSITION ) ) )
		AddPLoop( pEvent, 0, 1.0f );
}

// $deBlob: FUNCTION 00609020
void AMusicManager::RemoveMusicSFXSyncSounds( const TCHAR* a_szEvent )
{
	FMOD::Event* pEvent = GetMusicSFXEvent( a_szEvent, TTRUE );

	if ( pEvent == TNULL )
		return;

	TINT iIndex = -1;

	if ( pEvent->getInfo( &iIndex, TNULL, TNULL ) != FMOD_OK )
		return;

	SyncSound* apLists[ 2 ] = { m_pSSUsedList, m_pSSMusicBarUsedList };

	for ( TINT i = 0; i < 2; i++ )
	{
		for ( SyncSound* pNext = apLists[ i ]; pNext != TNULL; )
		{
			SyncSound* pSyncSound = pNext;
			pNext                 = pSyncSound->m_pNext;

			if ( iIndex != pSyncSound->m_iEventID || pSyncSound->m_szGroup != m_pMusicSFXInfo->m_szGroup )
				continue;

			if ( pSyncSound->m_pPrev != TNULL )
			{
				// NOTE: retail doesn't update the m_pPrev of the next sync sound
				pSyncSound->m_pPrev->m_pNext = pNext;
			}
			else if ( m_pSSUsedList == pSyncSound )
			{
				m_pSSUsedList = pNext;

				if ( pNext != TNULL )
					pNext->m_pPrev = TNULL;
			}
			else
			{
				TASSERT( m_pSSMusicBarUsedList == pSyncSound && "m_pSSMusicBarUsedList == pInspectSyncSound" );
				m_pSSMusicBarUsedList = pSyncSound->m_pNext;

				if ( m_pSSMusicBarUsedList != TNULL )
					m_pSSMusicBarUsedList->m_pPrev = TNULL;
			}

			pSyncSound->m_pNext = m_pSSFreeList;

			if ( m_pSSFreeList != TNULL )
				m_pSSFreeList->m_pPrev = pSyncSound;

			m_pSSFreeList       = pSyncSound;
			pSyncSound->m_pPrev = TNULL;
		}
	}
}

// $deBlob: FUNCTION 00609160
TFLOAT AMusicManager::GetMusicSFXLength( const TCHAR* a_szName )
{
	if ( TSound::GetSingleton()->IsInitialised() )
	{
		FMOD::Event* pEvent = GetMusicSFXEvent( a_szName, TFALSE );

		if ( pEvent != TNULL )
		{
			FMOD_EVENT_INFO oInfo = {};
			pEvent->getInfo( TNULL, TNULL, &oInfo );
		}
	}

	// NOTE: retail always returns 0 (the info isn't used)
	return 0.0f;
}

// $deBlob: FUNCTION 006091b0
void AMusicManager::StartLevelMusic()
{
	FreeAllSyncSounds();
	StopPLoops();
	ResetSyncPoints();

	if ( m_pKeyFrameList != TNULL && m_pKeyFrameList->m_bStartSilent )
	{
		m_ui32StreamFlags |= SET_START_SILENT;

		if ( TSound::GetSingleton()->IsInitialised() )
		{
			SnapshotKeyFrame* pKeyFrame = FindSnapshotKeyFrame( "silent" );

			if ( pKeyFrame != TNULL )
				ApplySnapshotKeyFrame( pKeyFrame );
		}

		if ( m_pChannel != TNULL )
		{
			m_pChannel->setPaused( true );
			m_ui32StreamFlags |= SET_START_PAUSED;
		}
	}
	else
	{
		PauseMusic( TFALSE, 0.0f );
		ReleaseSnapshot();
		m_fVolume2D     = 1.0f;
		m_fFadeVolume2D = 1.0f;
		m_fVolume3D     = 1.0f;
		m_fFadeVolume3D = 1.0f;
	}

	APositionalSoundManager::GetSingleton()->StopPitchedSounds();

	m_fTargetPitch    = 1.0f;
	m_fPitchTimer     = 0.0f;
	m_fPitchTime      = 0.0f;
	m_ui32StreamFlags = ( m_ui32StreamFlags & 0xDCDFFFDF ) | SET_PITCH_CHANGE;

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() != AGameStatus::GAMETYPE_MP_NONE )
		StartRainbow();

	if ( m_pChannel != TNULL )
		RestartStream();
}

// $deBlob: FUNCTION 006092d0
void AMusicManager::RegisterGUIScene( TXUIScene* a_pScene )
{
	m_pGUIMusicController->RegisterScene( a_pScene );
}

// $deBlob: FUNCTION 006092e0
void AMusicManager::UnregisterGUIScene( TXUIScene* a_pScene )
{
	m_pGUIMusicController->UnregisterScene( a_pScene );
}

// $deBlob: FUNCTION 00609340
void AMusicManager::ClampScore()
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return;

	TASSERT( 0 < m_pScoreMarkerData->m_iNumValues && "a_iMarkerNumber < m_pScoreMarkerData->m_iNumValues" );
	TINT iScore = TINT( ( m_pScoreMarkerData->m_pfValues[ SCORE_MUSIC_START ] / 100.0f ) * TFLOAT( m_iMaxScore ) );

	if ( m_iScore > iScore )
	{
		m_iScore = iScore;
		GetRegularKeyFrameVolumes( m_pTempKeyFrame->m_pfVolumes, m_fPercentage );
		m_pTempKeyFrame->m_fBlendOutTime = 5.0f;
		m_pTempKeyFrame->m_fBlendInTime  = 0.0f;
		m_pTempKeyFrame->m_fPitch        = 1.0f;
		m_pSnapshotKeyFrame              = m_pTempKeyFrame;
		m_fSnapshotBlendTime             = m_pTempKeyFrame->m_fBlendOutTime;
		m_fSnapshotBlendTimer            = 0.0f;
		m_ui32StreamFlags                = ( m_ui32StreamFlags & ~SET_SNAPSHOT_BLEND_IN ) | SET_SNAPSHOT_BLEND_OUT | SET_SNAPSHOT;
	}
}

// $deBlob: FUNCTION 00609450
void AMusicManager::SetMaxScore( TINT a_iMaxScore )
{
	m_iMaxScore = ( a_iMaxScore > 0 ) ? a_iMaxScore : 10000;
}

// $deBlob: FUNCTION 00609470
void AMusicManager::GetChordName( TUINT32 a_uiChord, TCHAR* a_szName, TSIZE a_uiSize )
{
	const TCHAR* aszNames[ 22 ] = {
		"A", "B", "C", "D", "E", "F", "G", "#", "b", "--", "maj", "sus4", "6th", "7th", "--7", "-7b5", "maj7", "o", "o7", "+", "intro", "ending"
	};

	TCHAR szName[ 256 ];
	T2String8::Format( szName, 256, "" );

	TUINT32 uiBit = 1;

	for ( TINT i = 0; i < 22; i++ )
	{
		if ( uiBit & a_uiChord )
		{
			// The chord types after the first one are separated by '|'
			if ( i >= 11 && i <= 19 )
				T2String8::Format( szName, 256, "%s|%s", szName, aszNames[ i ] );
			else
				T2String8::Format( szName, 256, "%s%s", szName, aszNames[ i ] );
		}

		uiBit = ( uiBit << 1 ) | ( uiBit >> 31 );
	}

	if ( T2String8::Length( szName ) == 0 )
		T2String8::Format( szName, 256, "None" );

	T2String8::Copy( a_szName, szName, a_uiSize );
}

// $deBlob: FUNCTION 00609620
const TCHAR* AMusicManager::GetTrackNameFromDisplayName( const TWCHAR* a_wszName )
{
	for ( TINT i = 0; i < MUSIC_NUM_SP_TRACKS; i++ )
	{
		const TWCHAR* wszDisplayName = T2Locale::GetSingleton()->GetString( s_aiTrackDisplayNames[ i ] );

		if ( T2String16::CompareNoCase( wszDisplayName, a_wszName ) == 0 )
			return s_aszTrackNames[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 00609670
const TCHAR* AMusicManager::GetTrackName( TINT a_iTrackNumber )
{
	TASSERT( a_iTrackNumber >= 0 && a_iTrackNumber < MUSIC_NUM_SP_TRACKS );
	return s_aszTrackNames[ a_iTrackNumber ];
}

// $deBlob: FUNCTION 006096d0
TBOOL AMusicManager::IsTrackUnlocked( const TCHAR* a_szName, TINT a_iLevel )
{
	if ( a_iLevel == 9 )
		return TStringManager::String8CompareNoCase( a_szName, s_aszTrackNames[ 10 ] ) == 0 || TStringManager::String8CompareNoCase( a_szName, s_szBossTrack ) == 0;

	if ( TStringManager::String8CompareNoCase( a_szName, "RANDOM" ) == 0 )
		return TTRUE;

	APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();

	if ( pProfile == TNULL )
		return TFALSE;

	if ( pProfile->AreAllTracksUnlocked() || TStringManager::String8CompareNoCase( a_szName, s_aszTrackNames[ 0 ] ) == 0 || TStringManager::String8CompareNoCase( a_szName, s_aszTrackNames[ 1 ] ) == 0 )
		return TTRUE;

	EnSaveData::SlotData* pData = pProfile->GetData();

	// The last track once the last world has stats
	if ( TStringManager::String8CompareNoCase( a_szName, s_aszTrackNames[ 11 ] ) == 0 )
		return pData->IsLevelFlagSet( 0, 9 ) == TTRUE;

	// The music of the levels played (and of a_iLevel)
	for ( TINT i = 1; i < 10; i++ )
	{
		if ( i == a_iLevel || pData->IsLevelFlagSet( 0, i ) )
		{
			ALevelInformation*                  pLevelInformation = ALevelInformation::GetSingleton();
			ALevelInformation::LevelProperties* pProperties       = pLevelInformation->GetLevelProperties( pLevelInformation->GetLevelIndex( AGameStatus::GetSPLevelName( i ) ) );

			if ( TStringManager::String8CompareNoCase( a_szName, pProperties->m_szMusic ) == 0 )
				return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00609810
void AMusicManager::UpdateVolumes()
{
	if ( m_pMusicProject == TNULL )
		return;

	FMOD::EventSystem* pEventSystem = TSound::GetSingleton()->GetEventSystem();
	TFLOAT             fSFXVolume   = AOptions::GetSingleton()->GetSFXVolume();
	TFLOAT             fMusicVolume = AOptions::GetSingleton()->GetMusicVolume();

	FMOD::EventCategory* pCategory;

	if ( pEventSystem->getCategory( "INGAME", &pCategory ) == FMOD_OK )
		pCategory->setVolume( fSFXVolume );

	if ( pEventSystem->getCategory( "music", &pCategory ) == FMOD_OK )
		pCategory->setVolume( fMusicVolume );
}

// $deBlob: FUNCTION 006098b0
void AMusicManager::StartRainbow()
{
	if ( m_pSound != TNULL )
	{
		FMOD_OPENSTATE eOpenState;
		TUINT          uiPercentBuffered;
		bool           bStarving = false;
		bool           bDiskBusy = false;
		FMOD_RESULT    eResult   = m_pSound->getOpenState( &eOpenState, &uiPercentBuffered, &bStarving, &bDiskBusy );

		if ( eOpenState == FMOD_OPENSTATE_ERROR || eResult == FMOD_ERR_FILE_DISKEJECTED )
		{
			// NOTE: retail calls an empty function here
		}
	}

	AMusicManager* pMusicManager = AMusicManager::GetSingleton();
	pMusicManager->m_ui32StreamFlags |= SET_PLAY_STREAM;
	pMusicManager->m_iScore = 0;

	if ( pMusicManager->m_ui32StreamFlags & SET_START_SILENT )
		pMusicManager->m_ui32StreamFlags |= SET_START_PAUSED | SET_SILENT;

	m_ui32StreamFlags = ( m_ui32StreamFlags & 0x77F1FFFD ) | SET_RAINBOW | SET_NO_PERCENTAGE;

	if ( TSound::GetSingleton()->IsInitialised() )
	{
		SnapshotKeyFrame* pKeyFrame = FindSnapshotKeyFrame( "rainbow" );

		if ( pKeyFrame != TNULL )
			ApplySnapshotKeyFrame( pKeyFrame );
	}
}

// $deBlob: FUNCTION 00609970
void AMusicManager::ResumeMusic()
{
	PauseMusic( TFALSE, 1.0f );
}

// $deBlob: FUNCTION 00609980
const TWCHAR* AMusicManager::GetTrackDisplayName( const TCHAR* a_szName )
{
	TINT iString = -1;

	for ( TINT i = 0; iString == -1; )
	{
		if ( T2String8::CompareNoCase( s_aszTrackNames[ i ], a_szName ) == 0 )
			iString = s_aiTrackDisplayNames[ i ];

		if ( ++i >= MUSIC_NUM_SP_TRACKS )
		{
			if ( iString == -1 )
				return s_wszNoMoodFound;

			break;
		}
	}

	return T2Locale::GetSingleton()->GetString( iString );
}

// $deBlob: FUNCTION 006099e0
void AMusicManager::GetDebugInfo( TINT* a_pBeat, TINT* a_pBar, TUINT* a_pNextBeat, TUINT* a_pNextBar, TUINT* a_pPosition, TBOOL* a_pWaitingForBeat, TBOOL* a_pWaitingForBar, TBOOL* a_pStarving, TUINT* a_pPercentBuffered, TINT* a_pOpenState, TINT* a_pResult )
{
	*a_pBeat            = m_iBeat;
	*a_pBar             = m_iBar;
	*a_pNextBeat        = m_uiNextBeat;
	*a_pNextBar         = m_uiNextBar;
	*a_pPosition        = 0;
	*a_pResult          = 0;
	*a_pOpenState       = 0;
	*a_pPercentBuffered = 0;

	if ( m_pSound != TNULL )
	{
		FMOD_OPENSTATE eOpenState = FMOD_OPENSTATE_READY;
		bool           bDiskBusy  = false;
		*a_pResult                = m_pSound->getOpenState( &eOpenState, a_pPercentBuffered, a_pStarving, &bDiskBusy );
		*a_pOpenState             = eOpenState;
	}
	else
	{
		*a_pResult    = 19; // FMOD_ERR_FILE_BAD
		*a_pOpenState = FMOD_OPENSTATE_CONNECTING;
	}

	if ( m_pChannel != TNULL )
		m_pChannel->getPosition( a_pPosition, FMOD_TIMEUNIT_MS );

	*a_pWaitingForBeat = ( m_ui32StreamFlags & SET_BEAT ) != 0;
	*a_pWaitingForBar  = ( m_ui32StreamFlags & SET_BAR ) != 0;
}

// $deBlob: FUNCTION 00609ac0
void AMusicManager::RestartStream()
{
	m_uiPosition = 0;

	if ( m_pChannel != TNULL )
	{
		m_pChannel->stop();
		m_pChannel = TNULL;
	}

	if ( m_pSound != TNULL )
	{
		m_pSound->release();
		m_pSound = TNULL;
	}

	if ( m_pStreamSettings->m_pChannelGroup != TNULL )
	{
		m_pStreamSettings->m_pChannelGroup->release();
		m_pStreamSettings->m_pChannelGroup = TNULL;
	}

	if ( TSound::GetSingleton()->IsInitialised() && m_pKeyFrameList != TNULL )
		CreateStream( m_pKeyFrameList->m_szStreamName, 1, 1, TTRUE );

	AMusicManager* pMusicManager = AMusicManager::GetSingleton();
	pMusicManager->m_ui32StreamFlags |= SET_PLAY_STREAM;
	pMusicManager->m_iScore = 0;

	if ( pMusicManager->m_ui32StreamFlags & SET_START_SILENT )
		pMusicManager->m_ui32StreamFlags |= SET_START_PAUSED | SET_SILENT;

	pMusicManager->m_ui32StreamFlags |= SET_MUSIC_PAUSED;

	AMusicManager::GetSingleton()->SetPercentage( 0.0f );
	AOptions::GetSingleton()->UpdateVolumes( TTRUE, -1.0f, -1.0f );
}

// $deBlob: FUNCTION 00609bc0
void AMusicManager::RemovePLoop( PLoop* a_pPLoop )
{
	if ( a_pPLoop == TNULL )
		return;

	a_pPLoop->m_pEvent->stop( false );
	a_pPLoop->m_pEvent = TNULL;

	for ( auto it = m_oPLoopList.Begin(); it != m_oPLoopList.End(); it++ )
	{
		if ( it == a_pPLoop )
		{
			m_oPLoopList.Erase( it );
			m_oFreePLoopList.PushBack( a_pPLoop );
			return;
		}
	}

	m_oFreePLoopList.PushBack( a_pPLoop );
}

// $deBlob: FUNCTION 00609c90
void AMusicManager::AddPLoop( FMOD::Event* a_pEvent, TUINT32 a_uiFlags, TFLOAT a_fValue )
{
	// No free PLoop: stops the oldest one
	if ( m_oFreePLoopList.Size() == 0 )
		RemovePLoop( m_oPLoopList.Head() );

	TASSERT( m_oFreePLoopList.Size() > 0 );

	PLoop* pPLoop = m_oFreePLoopList.Head();
	pPLoop->Remove();
	pPLoop->m_pEvent = a_pEvent;

	if ( a_uiFlags & SS_PLOOP_VOLUME )
		pPLoop->m_fValue = a_fValue;
	else
		pPLoop->m_fValue = -1.0f;

	m_oPLoopList.PushBack( pPLoop );
}

// $deBlob: FUNCTION 00609d90
FMOD::Event* AMusicManager::GetMusicSFXEvent( const TCHAR* a_szName, TBOOL a_bInfoOnly )
{
	TSound::GetSingleton()->GetEventSystem();

	FMOD_EVENT_MODE uiMode = FMOD_EVENT_DEFAULT;

	if ( a_bInfoOnly )
		uiMode = FMOD_EVENT_INFOONLY;

	if ( m_pMusicSFXInfo == TNULL || m_pMusicSFXProject == TNULL )
		return TNULL;

	FMOD::EventGroup* pGroup;

	if ( m_pMusicSFXProject->getGroup( m_pMusicSFXInfo->m_szGroup, false, &pGroup ) != FMOD_OK )
		return TNULL;

	// The event of the current chord, or the event
	TCHAR szChord[ 128 ];
	GetChordName( m_uiChord, szChord, 128 );

	TCHAR szName[ 128 ];
	TStringManager::String8Format( szName, 128, "%s%s|%s", m_pMusicSFXInfo->m_szPrefix, a_szName, szChord );

	FMOD::Event* pEvent = TNULL;

	if ( pGroup->getEvent( szName, uiMode, &pEvent ) != FMOD_OK )
	{
		TStringManager::String8Format( szName, 128, "%s%s", m_pMusicSFXInfo->m_szPrefix, a_szName );

		if ( pGroup->getEvent( szName, uiMode, &pEvent ) != FMOD_OK )
			return TNULL;
	}

	return pEvent;
}

// $deBlob: FUNCTION 00609ee0
void AMusicManager::UpdateColourSounds( FMOD::Event* a_pEvent, TUINT32 a_uiSection )
{
	// Stops the colour sounds that ended or aren't of the section
	for ( auto it = m_oColourSoundList.Begin(); it != m_oColourSoundList.End(); )
	{
		ColourSoundEvent* pColourSound = it;
		it++;

		TBOOL bKeep = TFALSE;

		if ( pColourSound->m_pEvent != TNULL )
		{
			FMOD_EVENT_STATE uiState;
			pColourSound->m_pEvent->getState( &uiState );

			if ( uiState & FMOD_EVENT_STATE_CHANNELSACTIVE )
				bKeep = pColourSound->m_uiSection == a_uiSection;
		}

		if ( !bKeep )
		{
			if ( pColourSound->m_pEvent != TNULL )
				pColourSound->m_pEvent->stop( false );

			pColourSound->Remove();
			m_oFreeColourSoundList.PushBack( pColourSound );
		}
	}

	if ( !m_oFreeColourSoundList.IsEmpty() && a_pEvent != TNULL )
	{
		ColourSoundEvent* pColourSound = m_oFreeColourSoundList.Head();
		pColourSound->Remove();
		pColourSound->m_pEvent    = a_pEvent;
		pColourSound->m_uiSection = a_uiSection;
		m_oColourSoundList.PushBack( pColourSound );
	}
}

// $deBlob: FUNCTION 0060a030
void AMusicManager::OnMusicDataStreamed( ATRBLoaderJob* a_pJob, void* a_pUserData )
{
	AAssetStreaming::GetSingleton()->ReleaseJob( a_pJob );
	AMusicManager::GetSingleton()->OnMusicDataLoaded( TUINTPTR( a_pUserData ) & 0xFF );
}

// $deBlob: FUNCTION 0060a060
const TCHAR* AMusicManager::GetMusicString( TINT16 a_iIndex )
{
	if ( m_pMusicStringList != TNULL && m_pMusicStringList->m_iNumStrings > a_iIndex && a_iIndex >= 0 )
		return m_pMusicStringList->m_ppStrings[ a_iIndex ];

	return TNULL;
}

// $deBlob: FUNCTION 0060a090
void AMusicManager::OnMusicDataLoaded( TBOOL a_bLoadProjects )
{
	m_pKeyFrameList = m_oTRBMusicData.CastSymbol<KeyFrameList>( "MusicData" );
	TASSERT( TNULL != m_pKeyFrameList );

	m_pMusicSFXInfo = m_oTRBMusicData.CastSymbol<MusicSFXInfo>( "MusicSFXGroup" );
	TASSERT( TNULL != m_pMusicSFXInfo );

	m_pMusicStringList = m_oTRBMusicData.CastSymbol<StringList>( "MusicStringList" );
	TASSERT( TNULL != m_pMusicStringList );

	if ( a_bLoadProjects )
	{
		// The FMOD projects: the game's, the one of the music events and the one of the music SFX
		TCHAR szProject[ 128 ];
		T2String8::Format( szProject, 128, "%s.fev", ASoundMap::scm_szProjectName );
		ASoundLoadHelper::GetSingleton()->AddProjectToLoad( szProject );

		const TCHAR* szMusicProject;

		if ( T2String8::Length( m_pKeyFrameList->m_szProjectName ) > 0 && TStringManager::String8CompareNoCase( ASoundMap::scm_szProjectName, m_pKeyFrameList->m_szProjectName ) != 0 )
		{
			szMusicProject = m_pKeyFrameList->m_szProjectName;
			T2String8::Format( szProject, 128, "%s.fev", szMusicProject );
			ASoundLoadHelper::GetSingleton()->AddProjectToLoad( szProject );
		}
		else
		{
			szMusicProject = ASoundMap::scm_szProjectName;
		}

		const TCHAR* szSFXProject;

		if ( TStringManager::String8Length( m_pMusicSFXInfo->m_szProject ) > 0 && T2String8::CompareNoCase( ASoundMap::scm_szProjectName, m_pMusicSFXInfo->m_szProject ) != 0 )
		{
			if ( T2String8::CompareNoCase( m_pKeyFrameList->m_szProjectName, m_pMusicSFXInfo->m_szProject ) != 0 )
			{
				szSFXProject = m_pMusicSFXInfo->m_szProject;
				T2String8::Format( szProject, 128, "%s.fev", szSFXProject );
				ASoundLoadHelper::GetSingleton()->AddProjectToLoad( szProject );
			}
			else
			{
				szSFXProject = szMusicProject;
			}
		}
		else
		{
			szSFXProject = ASoundMap::scm_szProjectName;
		}

		ASoundLoadHelper::GetSingleton()->SetState( ASoundLoadHelper::LOADINGPROJECTS );

		// NOTE: no deadline in retail
		while ( ASoundLoadHelper::GetSingleton()->GetState() != ASoundLoadHelper::LOADEDPROJECTS )
			ASoundLoadHelper::GetSingleton()->Update();

		m_pMusicProject    = ASoundInterface::GetEventProject( szMusicProject );
		m_pMusicSFXProject = ASoundInterface::GetEventProject( szSFXProject );
		AMusicManager::GetSingleton()->UpdateVolumes();
	}

	// The tracks of the stream
	m_pStreamSettings                  = TSTATICCAST( StreamSettings, TMalloc( sizeof( StreamSettings ) ) );
	m_pStreamSettings->m_pChannelGroup = TNULL;
	m_pStreamSettings->m_iNumTracks    = m_pKeyFrameList->m_iNumTracks;

	for ( TINT i = 0; i < m_pStreamSettings->m_iNumTracks && i < AMUSICMANAGER_MAX_TRACKS; i++ )
	{
		Track& rTrack     = m_pStreamSettings->m_aoTracks[ i ];
		rTrack.m_ui8Flags = 0;
		rTrack.m_ui8Flags |= ( ( 1 << i ) & m_pKeyFrameList->m_ui3DTracks ) ? TRACK_3D : TRACK_2D;
		rTrack.m_afSpeakerLevels[ 7 ] = 0.0f;
		rTrack.m_afSpeakerLevels[ 0 ] = 0.0f;
		rTrack.m_afSpeakerLevels[ 1 ] = 0.0f;
		rTrack.m_afSpeakerLevels[ 2 ] = 0.0f;
		rTrack.m_afSpeakerLevels[ 3 ] = 0.0f;
		rTrack.m_afSpeakerLevels[ 4 ] = 0.0f;
		rTrack.m_afSpeakerLevels[ 5 ] = 0.0f;
	}

	m_pBuildingColourList = m_oTRBMusicData.CastSymbol<BuildingColourList>( "BuilingColourSounds" );

	if ( m_pKeyFrameList->m_bStartSilent )
	{
		m_ui32StreamFlags |= SET_START_SILENT;

		if ( TSound::GetSingleton()->IsInitialised() )
		{
			SnapshotKeyFrame* pKeyFrame = FindSnapshotKeyFrame( "silent" );

			if ( pKeyFrame != TNULL )
				ApplySnapshotKeyFrame( pKeyFrame );
		}
	}

	// The event groups
	if ( TStringManager::String8Length( m_pMusicSFXInfo->m_szGroup ) > 0 )
		ASoundLoadHelper::GetSingleton()->AddSoundGroupToLoad( m_pMusicSFXInfo->m_szGroup, m_pMusicSFXInfo->m_szProject );

	LoadGroupList* pLoadGroupList = m_oTRBMusicData.CastSymbol<LoadGroupList>( "LoadGroupList" );

	for ( TINT i = 0; i < pLoadGroupList->m_iNumGroups; i++ )
		ASoundLoadHelper::GetSingleton()->AddSoundGroupToLoad( GetMusicString( pLoadGroupList->m_piGroups[ i ] ), m_pKeyFrameList->m_szProjectName );

	m_pSyncPoints      = m_oTRBMusicData.CastSymbol<SyncPoints>( "syncpointstruct" );
	m_pScoreMarkerData = m_oTRBMusicData.CastSymbol<ScoreMarkerData>( "ScoreMarkerData" );
}

// $deBlob: FUNCTION 0060a500
void AMusicManager::ResetSyncPoints()
{
	m_ui32StreamFlags &= 0xDEDEFDDF;
	m_iBossLevel    = -1;
	m_iSection      = 0;
	m_iChord        = 0;
	m_uiPosition    = 0;
	m_uiNextScratch = 0;
	m_iScore        = 0;
	m_iBeat         = 0;
	m_iBar          = 0;
	m_iScratch      = 0;
}

// $deBlob: FUNCTION 0060a570
void AMusicManager::UnloadStreamSettings()
{
	TFree( m_pStreamSettings );
	m_pStreamSettings = TNULL;
	m_oTRBMusicData.Close();
}

// $deBlob: FUNCTION 0060a5a0
void AMusicManager::RestoreStreamBufferSize()
{
	SetStreamBufferSize( m_uiStreamBufferSize );
}

// $deBlob: FUNCTION 0060a5c0
void AMusicManager::DebugCategories( FMOD::EventCategory* a_pCategory )
{
	// NOTE: walks the categories, nothing is done with them
	char* szName = TNULL;
	TINT  iIndex = 0;
	a_pCategory->getInfo( &iIndex, &szName );

	TINT iNumCategories = 0;
	a_pCategory->getNumCategories( &iNumCategories );

	for ( TINT i = 0; i < iNumCategories; i++ )
	{
		FMOD::EventCategory* pCategory = TNULL;
		a_pCategory->getCategoryByIndex( i, &pCategory );
		DebugCategories( pCategory );
	}
}

// $deBlob: FUNCTION 0060a630
TBOOL AMusicManager::FindColourSound( SyncSound* a_pSyncSound )
{
	TINT iColour = a_pSyncSound->m_iParam;

	TASSERT( m_pBuildingColourList );
	TASSERT( m_pBuildingColourList->m_iNumberOfColours == SOUND_COLOUR_NUMOF );

	ColourSound* pSound = TNULL;

	if ( iColour == 16 )
		iColour = 8;

	for ( TINT i = 0; pSound == TNULL && i < m_pBuildingColourList->m_iNumberOfColours; i++ )
	{
		BuildingColour& rColour = m_pBuildingColourList->m_pColours[ i ];

		if ( iColour != rColour.m_iColour )
			continue;

		for ( TINT j = 0; pSound == TNULL && j < rColour.m_iNumSounds; j++ )
		{
			ColourSound* pColourSound = &rColour.m_pSounds[ j ];

			if ( m_uiSection == pColourSound->m_uiSection || pColourSound->m_uiSection == 0 )
				pSound = pColourSound;
		}
	}

	TINT iIndex = -1;

	if ( pSound == TNULL )
		return TFALSE;

	FMOD::Event*      pEvent  = TNULL;
	FMOD::EventGroup* pGroup  = TNULL;
	const TCHAR*      szGroup = GetMusicString( pSound->m_iGroup );
	const TCHAR*      szEvent = GetMusicString( pSound->m_iEvent );

	if ( m_pMusicProject->getGroup( szGroup, false, &pGroup ) == FMOD_OK )
	{
		pGroup->getEvent( szEvent, FMOD_EVENT_INFOONLY, &pEvent );
		pEvent->getInfo( &iIndex, TNULL, TNULL );
	}

	if ( iIndex == -1 )
		return TFALSE;

	a_pSyncSound->m_uiFlags |= SS_GROUP_NAME;
	a_pSyncSound->m_szGroup  = szGroup;
	a_pSyncSound->m_iEventID = TINT16( iIndex );
	a_pSyncSound->m_iGroupID = -1;
	return TTRUE;
}

// $deBlob: FUNCTION 0060a800
void AMusicManager::PlayStream( TBOOL a_bPaused )
{

	FMOD_RESULT eResult = TSound::GetSingleton()->GetSystem()->playSound( FMOD_CHANNEL_FREE, m_pSound, a_bPaused, &m_pChannel );
	m_pChannel->setPriority( 10 );

	if ( m_pStreamSettings->m_pChannelGroup != TNULL )
		m_pChannel->setChannelGroup( m_pStreamSettings->m_pChannelGroup );

	TSound::GetSingleton()->GetSystem()->setStreamBufferSize( m_uiStreamBufferSize, FMOD_TIMEUNIT_RAWBYTES );
	TSound::ErrorCheck( eResult );
}

// $deBlob: FUNCTION 0060a880
AMusicManager::SnapshotKeyFrame* AMusicManager::FindSnapshotKeyFrame( const TCHAR* a_szName )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TNULL;

	if ( TStringManager::String8Length( a_szName ) < 1 )
		return TNULL;

	SnapshotKeyFrame* pKeyFrames = m_pKeyFrameList->m_pSnapshotKeyFrames;

	if ( m_pKeyFrameList->m_iNumSnapshotKeyFrames > 0 )
	{
		// NOTE: the keyframes are sorted, the search stops at the first one with a greater first character
		for ( TINT i = 0; a_szName[ 0 ] >= pKeyFrames[ i ].m_szName[ 0 ]; )
		{
			if ( TStringManager::String8CompareNoCase( a_szName, pKeyFrames[ i ].m_szName ) == 0 )
				return &pKeyFrames[ i ];

			if ( ++i >= m_pKeyFrameList->m_iNumSnapshotKeyFrames )
				return TNULL;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 0060a920
void AMusicManager::ApplySnapshotKeyFrame( SnapshotKeyFrame* a_pKeyFrame )
{
	SnapshotKeyFrame* pOldKeyFrame = m_pSnapshotKeyFrame;

	if ( a_pKeyFrame == pOldKeyFrame )
	{
		if ( !( m_ui32StreamFlags & SET_SNAPSHOT_BLEND_OUT ) )
			return;

		// Blending out of it: blends back in from where it is
		if ( m_fSnapshotBlendTime > 0.0f )
		{
			TFLOAT fTimer         = ( m_fSnapshotBlendTimer / m_fSnapshotBlendTime ) * a_pKeyFrame->m_fBlendInTime;
			m_fSnapshotBlendTimer = ( fTimer <= 0.0f ) ? 0.0f : fTimer;
		}
		else
		{
			m_fSnapshotBlendTimer = 0.0f;
		}
	}
	else
	{
		m_fSnapshotBlendTimer = 0.0f;
	}

	// Blends from the volumes of the previous snapshot
	if ( !( m_ui32StreamFlags & SET_SNAPSHOT_FROM_TEMP ) && pOldKeyFrame != TNULL && pOldKeyFrame->m_eBlendMode == BLENDMODE_LERP && pOldKeyFrame != m_pTempKeyFrame )
	{
		for ( TINT i = 0; i < m_pKeyFrameList->m_iNumTracks; i++ )
			m_pTempKeyFrame->m_pfVolumes[ i ] = m_pSnapshotKeyFrame->m_pfVolumes[ i ];

		m_ui32StreamFlags |= SET_SNAPSHOT_FROM_TEMP;
	}

	m_pSnapshotKeyFrame  = a_pKeyFrame;
	m_fSnapshotBlendTime = a_pKeyFrame->m_fBlendInTime;

	FMOD::ChannelGroup* pChannelGroup = m_pStreamSettings->m_pChannelGroup;

	if ( pChannelGroup != TNULL )
	{
		TFLOAT fPitch = -1.0f;
		pChannelGroup->getPitch( &fPitch );

		if ( a_pKeyFrame->m_fPitch > s_fEpsilon + fPitch || fPitch - s_fEpsilon > a_pKeyFrame->m_fPitch )
			SetPitch( a_pKeyFrame->m_fPitch, a_pKeyFrame->m_fBlendInTime, TFALSE );
	}

	m_ui32StreamFlags = ( m_ui32StreamFlags & ~( SET_SNAPSHOT | SET_SNAPSHOT_BLEND_IN | SET_SNAPSHOT_BLEND_OUT ) ) | SET_SNAPSHOT_BLEND_IN | SET_SNAPSHOT;
}

// $deBlob: FUNCTION 0060aa80
void AMusicManager::UpdateTrackLevels( TFLOAT a_fDeltaTime )
{
	if ( m_pKeyFrameList == TNULL || m_pStreamSettings == TNULL )
		return;

	// The volumes of the regular keyframes
	TFLOAT afVolumes[ AMUSICMANAGER_MAX_TRACKS ];
	TUtil::MemClear( afVolumes, sizeof( afVolumes ) );
	GetRegularKeyFrameVolumes( afVolumes, m_fPercentage );

	// The snapshot keyframe
	if ( m_pSnapshotKeyFrame != TNULL && ( m_ui32StreamFlags & SET_SNAPSHOT ) )
	{
		TFLOAT fBlend = 0.0f;

		if ( m_fSnapshotBlendTimer <= m_fSnapshotBlendTime )
		{
			if ( m_fSnapshotBlendTime <= 0.0f )
				fBlend = 1.0f;
			else
				fBlend = m_fSnapshotBlendTimer / m_fSnapshotBlendTime;
		}
		else
		{
			// The blend is done
			m_fSnapshotBlendTimer = 0.0f;

			if ( m_ui32StreamFlags & SET_SNAPSHOT_BLEND_OUT )
			{
				m_pSnapshotKeyFrame = TNULL;
				m_ui32StreamFlags &= ~SET_SNAPSHOT;
			}

			m_ui32StreamFlags &= ~( SET_SNAPSHOT_BLEND_IN | SET_SNAPSHOT_BLEND_OUT | SET_SNAPSHOT_FROM_TEMP );
		}

		SnapshotKeyFrame* pKeyFrame = m_pSnapshotKeyFrame;

		if ( pKeyFrame != TNULL )
		{
			TFLOAT  afSnapshot[ AMUSICMANAGER_MAX_TRACKS ];
			TFLOAT* pfVolumes = pKeyFrame->m_pfVolumes;

			if ( pKeyFrame->m_eBlendMode == BLENDMODE_MIN )
			{
				for ( TINT i = 0; i < m_pKeyFrameList->m_iNumTracks; i++ )
					afSnapshot[ i ] = ( pfVolumes[ i ] <= afVolumes[ i ] ) ? pfVolumes[ i ] : afVolumes[ i ];
			}
			else if ( pKeyFrame->m_eBlendMode == BLENDMODE_MAX )
			{
				for ( TINT i = 0; i < m_pKeyFrameList->m_iNumTracks; i++ )
					afSnapshot[ i ] = ( afVolumes[ i ] <= pfVolumes[ i ] ) ? pfVolumes[ i ] : afVolumes[ i ];
			}
			else if ( m_pTempKeyFrame != TNULL && ( m_ui32StreamFlags & SET_SNAPSHOT_FROM_TEMP ) )
			{
				for ( TINT i = 0; i < m_pKeyFrameList->m_iNumTracks; i++ )
					afSnapshot[ i ] = pKeyFrame->m_pfVolumes[ i ] * fBlend + m_pTempKeyFrame->m_pfVolumes[ i ] * ( 1.0f - fBlend );
			}
			else
			{
				TUtil::MemCopy( afSnapshot, pfVolumes, m_pKeyFrameList->m_iNumTracks * sizeof( TFLOAT ) );
			}

			TUINT32 uiFlags = m_ui32StreamFlags;

			if ( !( uiFlags & SET_SNAPSHOT_BLEND_IN ) || ( uiFlags & SET_SNAPSHOT_FROM_TEMP ) )
			{
				if ( !( uiFlags & SET_SNAPSHOT_BLEND_OUT ) || ( uiFlags & SET_SNAPSHOT_FROM_TEMP ) )
				{
					if ( uiFlags & SET_SNAPSHOT )
						TUtil::MemCopy( afVolumes, afSnapshot, m_pKeyFrameList->m_iNumTracks * sizeof( TFLOAT ) );
				}
				else
				{
					// Blending out
					SetPLoopVolume( m_fTargetVolume2D * fBlend );

					for ( TINT i = 0; i < m_pKeyFrameList->m_iNumTracks; i++ )
						afVolumes[ i ] = afSnapshot[ i ] * ( 1.0f - fBlend ) + afVolumes[ i ] * fBlend;
				}
			}
			else
			{
				// Blending in
				SetPLoopVolume( m_fTargetVolume2D * ( 1.0f - fBlend ) );

				for ( TINT i = 0; i < m_pKeyFrameList->m_iNumTracks; i++ )
					afVolumes[ i ] = afSnapshot[ i ] * fBlend + ( 1.0f - fBlend ) * afVolumes[ i ];
			}
		}

		m_fSnapshotBlendTimer += a_fDeltaTime;
	}

	// The levels of the tracks
	for ( TINT i = 0; i < m_pKeyFrameList->m_iNumTracks; i++ )
	{
		Track& rTrack = m_pStreamSettings->m_aoTracks[ i ];

		if ( rTrack.m_ui8Flags & TRACK_2D )
		{
			TFLOAT fLeft  = 1.0f;
			TFLOAT fRight = 1.0f;
			TFLOAT fPan   = TMath::Abs( m_pKeyFrameList->m_pfPans[ i ] );
			TMath::Clip( fPan, 0.0f, 1.0f );

			if ( m_pKeyFrameList->m_pfPans[ i ] <= 0.0f )
				fRight = 1.0f - fPan;
			else
				fLeft = 1.0f - fPan;

			for ( TINT j = 0; j < 6; j++ )
				rTrack.m_afSpeakerLevels[ j ] = 0.0f;

			rTrack.m_afSpeakerLevels[ 0 ] = afVolumes[ i ] * fLeft;
			rTrack.m_afSpeakerLevels[ 1 ] = afVolumes[ i ] * fRight;
		}
		else if ( rTrack.m_ui8Flags & TRACK_3D )
		{
			if ( ( rTrack.m_ui8Flags & TRACK_MUTED ) || !( rTrack.m_ui8Flags & TRACK_POSITION ) )
			{
				for ( TINT j = 0; j < 6; j++ )
					rTrack.m_afSpeakerLevels[ j ] = 0.0f;
			}
			else
			{
				Calculate3DTrackLevels( &rTrack );
			}
		}
	}

	// The levels of the speakers
	FMOD_SPEAKERMODE eSpeakerMode;
	TSound::GetSingleton()->GetSystem()->getSpeakerMode( &eSpeakerMode );

	if ( m_pChannel == TNULL )
		return;

	bool bPlaying = false;
	m_pChannel->isPlaying( &bPlaying );

	if ( !bPlaying )
		return;

	for ( TINT iSpeaker = 0; iSpeaker < 8; iSpeaker++ )
	{
		TASSERT( AMUSICMANAGER_MAX_TRACKS >= m_pKeyFrameList->m_iNumTracks );

		TFLOAT afLevels[ AMUSICMANAGER_MAX_TRACKS ];

		for ( TINT i = 0; i < m_pKeyFrameList->m_iNumTracks; i++ )
		{
			Track& rTrack = m_pStreamSettings->m_aoTracks[ i ];

			if ( rTrack.m_ui8Flags & TRACK_2D )
				afLevels[ i ] = rTrack.m_afSpeakerLevels[ iSpeaker ] * m_fVolume2D;
			else
				afLevels[ i ] = rTrack.m_afSpeakerLevels[ iSpeaker ] * m_fVolume3D;
		}

		TASSERT( iSpeaker < FMOD_SPEAKER_MAX && "eSpeaker < FMOD_SPEAKER_MAX" );

		TBOOL bSetLevels;

		if ( eSpeakerMode == FMOD_SPEAKERMODE_5POINT1 )
		{
			bSetLevels = iSpeaker < 2 || iSpeaker == 4 || iSpeaker == 5 || iSpeaker == 2;
		}
		else if ( eSpeakerMode == FMOD_SPEAKERMODE_SURROUND )
		{
			// NOTE: retail adds half of the back speakers' levels to a sum it never uses
			bSetLevels = iSpeaker <= 2;
		}
		else
		{
			bSetLevels = iSpeaker == 0 || iSpeaker == 1;
		}

		if ( bSetLevels )
			TSound::ErrorCheck( m_pChannel->setSpeakerLevels( FMOD_SPEAKER( iSpeaker ), afLevels, m_pKeyFrameList->m_iNumTracks ) );
	}
}

// $deBlob: FUNCTION 0060b0c0
void AMusicManager::Calculate3DTrackLevels( Track* a_pTrack )
{
	// The position and the axes of the camera
	TVector4 vCameraPosition = TVector4::VEC_ZERO;
	TVector4 vCameraForward  = TVector4::VEC_ZERO;
	TVector4 vCameraDown     = TVector4::VEC_ZERO;

	const TMatrix44& rCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
	vCameraPosition          = rCamera.GetTranslation();

	const TMatrix44& rCamera2 = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
	vCameraForward.x          = rCamera2.m_f31;
	vCameraForward.y          = rCamera2.m_f32;
	vCameraForward.z          = rCamera2.m_f33;

	const TMatrix44& rCamera3 = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
	vCameraDown.x             = -rCamera3.m_f21;
	vCameraDown.y             = -rCamera3.m_f22;
	vCameraDown.z             = -rCamera3.m_f23;
	vCameraForward.w          = 1.0f;
	vCameraDown.w             = 1.0f;

	FMOD_SPEAKERMODE eSpeakerMode;
	TSound::GetSingleton()->GetSystem()->getSpeakerMode( &eSpeakerMode );

	TVector4 vDirection;
	vDirection.x = vCameraPosition.x - a_pTrack->m_vPosition.x;
	vDirection.y = vCameraPosition.y - a_pTrack->m_vPosition.y;
	vDirection.z = vCameraPosition.z - a_pTrack->m_vPosition.z;
	vDirection.w = vCameraPosition.w;

	TFLOAT fDistance = vDirection.Magnitude();
	vDirection.Normalise();

	// The front speakers
	static TFLOAT s_fSideAngle = 1.5707964f;

	TVector4 vRight;
	TVector4 vLeft;
	TVector4::RotateArbitaryVector( vRight, vCameraForward, vCameraDown, s_fSideAngle );
	TVector4::RotateArbitaryVector( vLeft, vCameraForward, vCameraDown, -s_fSideAngle );

	TFLOAT* pfLeft  = &a_pTrack->m_afSpeakerLevels[ 0 ];
	TFLOAT* pfRight = &a_pTrack->m_afSpeakerLevels[ 1 ];
	TFLOAT  fLeft   = ( vRight.y * vDirection.y + vRight.x * vDirection.x + vRight.z * vDirection.z + 1.0f ) * 0.5f;
	*pfLeft         = fLeft;
	TFLOAT fRight   = ( vLeft.y * vDirection.y + vLeft.x * vDirection.x + vLeft.z * vDirection.z + 1.0f ) * 0.5f;
	*pfRight        = fRight;

	if ( s_f3DMinDistance < fDistance )
	{
		TFLOAT fAttenuation = ( s_f3DMaxDistance - fDistance ) / ( s_f3DMaxDistance - s_f3DMinDistance );
		*pfLeft             = fLeft * fAttenuation;
		*pfRight            = fRight * fAttenuation;
	}

	TMath::Clip( *pfLeft, 0.0f, 1.0f );
	TMath::Clip( *pfRight, 0.0f, 1.0f );

	if ( eSpeakerMode == FMOD_SPEAKERMODE_SURROUND || eSpeakerMode == FMOD_SPEAKERMODE_5POINT1 )
	{
		// The center and the back speakers
		static TFLOAT s_fBackAngle  = 1.5707964f;
		static TFLOAT s_fBackSpread = 0.61086524f;

		TVector4 vBackLeft;
		TVector4 vBackRight;
		TVector4::RotateArbitaryVector( vBackLeft, vCameraForward, vCameraDown, ( s_fBackSpread - s_fBackAngle ) + 3.1415927f );
		TVector4::RotateArbitaryVector( vBackRight, vCameraForward, vCameraDown, s_fBackSpread + s_fBackAngle + 3.1415927f );

		TFLOAT fBackLeft  = vBackLeft.y * vDirection.y + vBackLeft.x * vDirection.x + vBackLeft.z * vDirection.z;
		TFLOAT fBackRight = vBackRight.y * vDirection.y + vBackRight.x * vDirection.x + vBackRight.z * vDirection.z;
		TFLOAT fCenter    = vCameraForward.y * vDirection.y + vCameraForward.x * vDirection.x + vCameraForward.z * vDirection.z;

		TASSERT( fDistance * fDistance > 0.0f && "fLower > 0.0f" );

		TFLOAT* pfCenter    = &a_pTrack->m_afSpeakerLevels[ 2 ];
		TFLOAT* pfBackLeft  = &a_pTrack->m_afSpeakerLevels[ 4 ];
		TFLOAT* pfBackRight = &a_pTrack->m_afSpeakerLevels[ 5 ];
		fCenter             = ( fCenter + 1.0f ) * 0.5f;
		fBackLeft           = ( fBackLeft + 1.0f ) * 0.5f;
		fBackRight          = ( fBackRight + 1.0f ) * 0.5f;
		*pfCenter           = fCenter;
		*pfBackLeft         = fBackLeft;
		*pfBackRight        = fBackRight;

		if ( s_f3DMinDistance < fDistance )
		{
			TFLOAT fAttenuation = ( s_f3DMaxDistance - fDistance ) / ( s_f3DMaxDistance - s_f3DMinDistance );
			*pfCenter           = fCenter * fAttenuation;
			*pfBackLeft         = fBackLeft * fAttenuation;
			*pfBackRight        = fBackRight * fAttenuation;
		}

		TMath::Clip( *pfCenter, 0.0f, 1.0f );
		TMath::Clip( *pfBackLeft, 0.0f, 1.0f );
		TMath::Clip( *pfBackRight, 0.0f, 1.0f );
	}
}

// $deBlob: FUNCTION 0060b560
void AMusicManager::PlaySyncSound( SyncSound* a_pSyncSound )
{
	if ( ( a_pSyncSound->m_uiFlags & SS_BUILDING_COLOUR ) && !FindColourSound( a_pSyncSound ) )
		return;

	if ( !( a_pSyncSound->m_uiFlags & SS_NO_EVENT ) )
	{
		FMOD::EventSystem*  pEventSystem = TSound::GetSingleton()->GetEventSystem();
		FMOD::Event*        pEvent       = TNULL;
		FMOD::EventProject* pProject     = TNULL;
		FMOD::EventGroup*   pGroup       = TNULL;

		if ( ( a_pSyncSound->m_uiFlags & SS_SFX_PROJECT ) && m_pMusicSFXProject != TNULL )
			pProject = m_pMusicSFXProject;
		else if ( ( a_pSyncSound->m_uiFlags & SS_MUSIC_PROJECT ) && m_pMusicProject != TNULL )
			pProject = m_pMusicProject;
		else
			pEventSystem->getProjectByIndex( 0, &pProject );

		if ( a_pSyncSound->m_uiFlags & SS_GROUP_NAME )
		{
			FMOD_RESULT eNamedGroupResult = pProject->getGroup( a_pSyncSound->m_szGroup, false, &pGroup );
			TASSERT( eNamedGroupResult == FMOD_OK );

			FMOD_RESULT eEventResult = pGroup->getEventByIndex( a_pSyncSound->m_iEventID, FMOD_EVENT_DEFAULT, &pEvent );
			TASSERT( eEventResult == FMOD_OK );
		}
		else
		{
			pEvent = ASoundInterface::GetEvent( ASoundMap::GROUPID( a_pSyncSound->m_iGroupID ), ASoundMap::EVENTID( a_pSyncSound->m_iEventID ), FMOD_EVENT_DEFAULT );
		}

		if ( pEvent != TNULL )
			TSound::ErrorCheck( pEvent->start() );

		if ( a_pSyncSound->m_uiFlags & SS_POSITION )
			ASoundInterface::SetPosition( pEvent, a_pSyncSound->m_vPosition );

		if ( a_pSyncSound->m_uiFlags & SS_BUILDING_COLOUR )
			UpdateColourSounds( pEvent, m_uiSection );

		if ( a_pSyncSound->m_uiFlags & SS_PLOOP )
			AddPLoop( pEvent, a_pSyncSound->m_uiFlags, a_pSyncSound->m_fTimeOut );
	}

	if ( a_pSyncSound->m_pSnapshotKeyFrame != TNULL )
	{
		ApplySnapshotKeyFrame( a_pSyncSound->m_pSnapshotKeyFrame );

		if ( a_pSyncSound->m_uiFlags & SS_RELEASE_SNAPSHOT )
		{
			// Releases the snapshot on the next beat (bar)
			TUINT32 uiFlags = SS_NO_EVENT | SS_RELEASE_SNAPSHOT;

			if ( a_pSyncSound->m_uiFlags & SS_MUSIC_BAR )
				uiFlags |= SS_MUSIC_BAR;

			AddSyncSound( 0, 0, 10.0f, uiFlags, &TVector4::VEC_ZERO, TNULL, TNULL, 0 );
		}
	}
	else if ( a_pSyncSound->m_uiFlags & SS_RELEASE_SNAPSHOT )
	{
		ReleaseSnapshot();
	}

	TSound::GetSingleton()->GetEventSystem()->update();
}

// $deBlob: FUNCTION 0060b770
void AMusicManager::SetPLoopVolume( TFLOAT a_fVolume )
{
	for ( auto it = m_oPLoopList.Begin(); it != m_oPLoopList.End(); it++ )
		it->m_pEvent->setVolume( a_fVolume );
}

// $deBlob: FUNCTION 0060b7d0
void AMusicManager::SetPLoopPaused( TBOOL a_bPaused )
{
	for ( auto it = m_oPLoopList.Begin(); it != m_oPLoopList.End(); it++ )
		it->m_pEvent->setPaused( a_bPaused );
}

// $deBlob: FUNCTION 0060b820
TUINT AMusicManager::GetSection( TUINT a_uiPosition )
{
	TINT iSection = 0;

	// NOTE: retail reads the sections before checking m_pSyncPoints
	if ( a_uiPosition < m_pSyncPoints->m_pSections[ m_iSection ].m_uiTime )
		m_iSection = 0;

	if ( m_pSyncPoints == TNULL || m_pSyncPoints->m_iNumSections == 0 )
		return 0;

	for ( TUINT i = m_iSection; i < TUINT( m_pSyncPoints->m_iNumSections ); i++ )
	{
		if ( a_uiPosition <= m_pSyncPoints->m_pSections[ i ].m_uiTime )
			break;

		iSection = i;
	}

	m_iSection = ( iSection - 1 <= 0 ) ? 0 : iSection - 1;
	return m_pSyncPoints->m_pSections[ iSection ].m_uiValue;
}

// $deBlob: FUNCTION 0060b8c0
TUINT AMusicManager::GetChord( TUINT a_uiPosition )
{
	TINT iChord = 0;

	// NOTE: retail reads the chords before checking m_pSyncPoints
	if ( a_uiPosition < m_pSyncPoints->m_pChords[ m_iChord ].m_uiTime )
		m_iChord = 0;

	if ( m_pSyncPoints == TNULL || m_pSyncPoints->m_iNumChords == 0 )
		return 0;

	for ( TUINT i = m_iChord; i < TUINT( m_pSyncPoints->m_iNumChords ); i++ )
	{
		if ( a_uiPosition <= m_pSyncPoints->m_pChords[ i ].m_uiTime )
			break;

		iChord = i;
	}

	m_iChord = ( iChord - 1 <= 0 ) ? 0 : iChord - 1;
	return m_pSyncPoints->m_pChords[ iChord ].m_uiValue;
}

// $deBlob: FUNCTION 0060b960
TBOOL AMusicManager::ProcessSyncSounds( SyncSound* a_pSyncSound )
{
	if ( a_pSyncSound == TNULL )
		return TFALSE;

	if ( a_pSyncSound->m_uiFlags & SS_PROCESS_ALL )
	{
		// Every sync sound of the list with SS_PROCESS_ALL
		SyncSound* pSyncSound = a_pSyncSound;

		do
		{
			if ( pSyncSound->m_uiFlags & SS_PROCESS_ALL )
			{
				SyncSound* pNext = pSyncSound->m_pNext;
				PlaySyncSound( pSyncSound );
				ReleaseSyncSound( pSyncSound );
				pSyncSound = pNext;
			}
			else
			{
				pSyncSound = pSyncSound->m_pNext;
			}
		} while ( pSyncSound != TNULL );

		return TFALSE;
	}

	PlaySyncSound( a_pSyncSound );
	ReleaseSyncSound( a_pSyncSound );
	return TTRUE;
}

// $deBlob: FUNCTION 0060b9c0
void AMusicManager::SetForcePause( TBOOL a_bForce )
{
	if ( a_bForce )
		m_ui32StreamFlags |= SET_FORCE_PAUSE;
	else
		m_ui32StreamFlags &= ~SET_FORCE_PAUSE;
}

// $deBlob: FUNCTION 0060b9f0
TBOOL AMusicManager::FindNextSyncPoint( TUINT a_uiPosition, TINT* a_pIndex, TUINT* a_pNext, TUINT* a_pPrevious, const TUINT32* a_puiTimes, TINT a_iNumTimes )
{
	if ( m_pSyncPoints == TNULL || m_pChannel == TNULL || m_pSound == TNULL )
		return TFALSE;

	TINT i = *a_pIndex;

	if ( i >= a_iNumTimes )
	{
		*a_pIndex = 0;
		return TFALSE;
	}

	while ( TTRUE )
	{
		TUINT uiTime = a_puiTimes[ i ];

		if ( i > 0 && a_puiTimes[ i - 1 ] > a_uiPosition )
		{
			// The position is before the previous sync point: searches from the start
			i = -1;
		}
		else if ( a_uiPosition <= uiTime )
		{
			*a_pIndex = i;
			*a_pNext  = uiTime;

			if ( *a_pIndex <= 0 )
				*a_pPrevious = 0;
			else
				*a_pPrevious = a_puiTimes[ *a_pIndex - 1 ];

			return TTRUE;
		}

		if ( ++i >= a_iNumTimes )
			return TFALSE;
	}
}

// $deBlob: FUNCTION 0060bad0
TBOOL AMusicManager::GetRegularKeyFrameVolumes( TFLOAT* a_pfVolumes, TFLOAT a_fPercentage )
{
	TASSERT( m_pKeyFrameList->m_iNumRegularKeyFrames > 1 );

	TINT            iNext               = 2;
	VolumeKeyFrame* pVolumeKeyFrame     = &m_pKeyFrameList->m_pRegularKeyFrames[ 0 ];
	VolumeKeyFrame* pVolumeKeyFrameNext = &m_pKeyFrameList->m_pRegularKeyFrames[ 1 ];

	while ( a_fPercentage > pVolumeKeyFrameNext->m_fPercentageComplete )
	{
		if ( iNext >= m_pKeyFrameList->m_iNumRegularKeyFrames )
		{
			// After the last keyframe
			TUtil::MemCopy( a_pfVolumes, pVolumeKeyFrameNext->m_pfVolumes, m_pKeyFrameList->m_iNumTracks * sizeof( TFLOAT ) );
			return TTRUE;
		}

		pVolumeKeyFrame     = pVolumeKeyFrameNext;
		pVolumeKeyFrameNext = &m_pKeyFrameList->m_pRegularKeyFrames[ iNext++ ];
		TASSERT( pVolumeKeyFrame->m_fPercentageComplete < pVolumeKeyFrameNext->m_fPercentageComplete );
	}

	if ( pVolumeKeyFrame->m_fPercentageComplete >= a_fPercentage )
	{
		// Before the first keyframe
		TUtil::MemCopy( a_pfVolumes, pVolumeKeyFrame->m_pfVolumes, m_pKeyFrameList->m_iNumTracks * sizeof( TFLOAT ) );
		return TTRUE;
	}

	TASSERT( pVolumeKeyFrame->m_fPercentageComplete <= a_fPercentage );
	TASSERT( pVolumeKeyFrameNext->m_fPercentageComplete >= a_fPercentage );

	// NOTE: retail interpolates with m_fPercentage, not a_fPercentage (PlayConversation passes the end score marker)
	TFLOAT fRange = pVolumeKeyFrameNext->m_fPercentageComplete - pVolumeKeyFrame->m_fPercentageComplete;
	TFLOAT fPrev  = ( pVolumeKeyFrameNext->m_fPercentageComplete - m_fPercentage ) / fRange;
	TFLOAT fNext  = ( m_fPercentage - pVolumeKeyFrame->m_fPercentageComplete ) / fRange;

	for ( TINT i = 0; i < m_pKeyFrameList->m_iNumTracks; i++ )
		a_pfVolumes[ i ] = pVolumeKeyFrameNext->m_pfVolumes[ i ] * fNext + pVolumeKeyFrame->m_pfVolumes[ i ] * fPrev;

	return TTRUE;
}

// $deBlob: FUNCTION 0060bcd0
void AMusicManager::DebugMemory()
{
	TINT iCurrentAlloced = 0;
	TINT iMaxAlloced     = 0;
	FMOD_Memory_GetStats( &iCurrentAlloced, &iMaxAlloced, 1 );
}

// $deBlob: FUNCTION 0060bd00
TBOOL AMusicManager::ErrorCheck( FMOD_RESULT a_eResult )
{
	return TSound::ErrorCheck( a_eResult );
}
