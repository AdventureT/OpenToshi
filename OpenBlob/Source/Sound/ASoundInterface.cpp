#include "pch.h"
#include "ASoundInterface.h"
#include "Console/AStaticCVars.h"
#include "ASoundLoadHelper.h"

#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// called by LoadEventGroup( const TCHAR*,...)
// $deBlob: FUNCTION 00611280
FMOD::EventGroup* ASoundInterface::LoadEventGroup( const TCHAR* a_szProject, const TCHAR* a_szGroup, FMOD_EVENT_RESOURCE a_eResource, FMOD_EVENT_MODE a_uiMode )
{
	FMOD::EventSystem*  pEventSystem = TSound::GetSingleton()->GetEventSystem();
	FMOD::EventProject* pProject     = TNULL;
	FMOD::EventGroup*   pEventGroup  = TNULL;

	if ( pEventSystem->getProject( a_szProject, &pProject ) != FMOD_OK ||
	     pProject->getGroup( a_szGroup, false, &pEventGroup ) != FMOD_OK ||
	     pEventGroup->loadEventData( a_eResource, a_uiMode ) != FMOD_OK )
	{
		return TNULL;
	}

	return pEventGroup;
}

// $deBlob: FUNCTION 006112f0
FMOD::EventGroup* ASoundInterface::LoadEventGroup( const TCHAR* a_szGroup, FMOD_EVENT_RESOURCE a_eResource, FMOD_EVENT_MODE a_uiMode )
{
	TBOOL bTSoundCreated = TSound::GetSingleton() != TNULL;
	TASSERT( TTRUE == bTSoundCreated );

	return LoadEventGroup( ASoundMap::scm_szProjectName, a_szGroup, a_eResource, a_uiMode );
}

// $deBlob: FUNCTION 00611400
FMOD::EventGroup* ASoundInterface::LoadEventGroup( ASoundMap::GROUPID a_eGroupID, FMOD_EVENT_RESOURCE a_eResource, FMOD_EVENT_MODE a_uiMode )
{
	TBOOL bTSoundCreated = TSound::GetSingleton() != TNULL;
	TASSERT( TTRUE == bTSoundCreated );

	FMOD::EventGroup* pEventGroup = GetEventGroup( a_eGroupID, 0 );

	return LoadEventGroup( pEventGroup, a_eResource, a_uiMode ) ? pEventGroup : TNULL;
}

// $deBlob: FUNCTION 006122c0
TBOOL ASoundInterface::LoadEventGroup( FMOD::EventGroup* a_pEventGroup, FMOD_EVENT_RESOURCE a_eResource, FMOD_EVENT_MODE a_uiMode )
{
	TASSERT( TNULL != a_pEventGroup );

	FMOD_RESULT eResult = a_pEventGroup->loadEventData( a_eResource, a_uiMode );

	if ( TSound::ErrorCheck( eResult ) == TTRUE )
	{
		// NOTE: retail gets the group's name for a print it compiled out ("ERROR: Couldn't load event group %s from project %s\n")
		TINT   iIndex;
		TCHAR* szName;
		a_pEventGroup->getInfo( &iIndex, &szName );
		TSound::ErrorCheck( eResult );

		return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00611470
TBOOL ASoundInterface::FreeEventGroup( ASoundMap::GROUPID a_eGroupID, FMOD::Event* a_pEvent )
{
	TBOOL bTSoundCreated = TSound::GetSingleton() != TNULL;
	TASSERT( TTRUE == bTSoundCreated );

	FMOD::EventGroup* pEventGroup = GetEventGroup( a_eGroupID, 0 );
	TASSERT( TNULL != pEventGroup );

	return TSound::ErrorCheck( pEventGroup->freeEventData( a_pEvent, true ) ) != TTRUE;
}

// $deBlob: FUNCTION 00611520
TBOOL ASoundInterface::FreeEventGroup( const TCHAR* a_szGroup, const TCHAR* a_szProject )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TTRUE;

	const TCHAR*        szProject = ( a_szProject != TNULL ) ? a_szProject : ASoundMap::scm_szProjectName;
	FMOD::EventProject* pProject  = TNULL;

	// NOTE: retail bug: both checks are inverted, so a group is only freed when getting it failed (and the event data
	// of the groups ASoundLoadHelper unloads is never freed); kept as is
	if ( TSound::ErrorCheck( TSound::GetSingleton()->GetEventSystem()->getProject( szProject, &pProject ) ) == TTRUE )
	{
		FMOD::EventGroup* pEventGroup = TNULL;

		if ( TSound::ErrorCheck( pProject->getGroup( a_szGroup, false, &pEventGroup ) ) == TTRUE )
		{
			FMOD_RESULT eFreeResult = pEventGroup->freeEventData( TNULL, true );
			TASSERT( TSound::ErrorCheck( eFreeResult ) );

			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00612070
FMOD::EventProject* ASoundInterface::GetEventProject( const TCHAR* a_szProject )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TNULL;

	FMOD::EventSystem*  pEventSystem = TSound::GetSingleton()->GetEventSystem();
	FMOD::EventProject* pProject     = TNULL;
	TSound::ErrorCheck( pEventSystem->getProject( a_szProject, &pProject ) );

	return pProject;
}

// $deBlob: FUNCTION 006115e0
FMOD::EventGroup* ASoundInterface::GetEventGroup( ASoundMap::GROUPID a_eGroupID, TINT a_iProject )
{
	if ( a_eGroupID == ASoundMap::GROUPID_INVALID )
		return TNULL;

	return GetEventGroup( ASoundMap::scm_aGroupNames[ a_eGroupID ], a_iProject );
}

// $deBlob: FUNCTION 00611340
FMOD::EventGroup* ASoundInterface::GetEventGroup( const TCHAR* a_szGroup, TINT a_iProject )
{
	FMOD::EventSystem* pEventSystem = TSound::GetSingleton()->GetEventSystem();
	TASSERT( TNULL != pEventSystem );

	FMOD::EventGroup*   pEventGroup = TNULL;
	FMOD::EventProject* pProject    = TNULL;
	TSound::ErrorCheck( pEventSystem->getProjectByIndex( a_iProject, &pProject ) );
	TSound::ErrorCheck( pProject->getGroup( a_szGroup, true, &pEventGroup ) );
	TASSERT( TNULL != pEventGroup );

	return pEventGroup;
}

// $deBlob: FUNCTION 00611610
FMOD::EventCategory* ASoundInterface::GetEventCategory( ASoundMap::CATEGORYID a_eCategoryID )
{
	FMOD::EventSystem* pEventSystem = TSound::GetSingleton()->GetEventSystem();
	TASSERT( TNULL != pEventSystem );

	FMOD::EventCategory* pEventCategory = TNULL;
	TSound::ErrorCheck( pEventSystem->getCategoryByIndex( ASoundMap::scm_iCategoryIDs[ a_eCategoryID ], &pEventCategory ) );
	TASSERT( TNULL != pEventCategory );

	return pEventCategory;
}

// $deBlob: FUNCTION 006116c0
FMOD::Event* ASoundInterface::GetEvent( ASoundMap::GROUPID a_eGroupID, ASoundMap::EVENTID a_eEventID, FMOD_EVENT_MODE a_uiMode )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TNULL;

	FMOD::EventGroup* pEventGroup = GetEventGroup( a_eGroupID, 0 );

	if ( a_eEventID == ASoundMap::EVENTID_INVALID || pEventGroup == TNULL )
		return TNULL;

	FMOD::Event* pEvent = TNULL;
	TSound::ErrorCheck( pEventGroup->getEventByIndex( ASoundMap::scm_iEventIDs[ a_eEventID ], a_uiMode, &pEvent ) );

	return pEvent;
}

// $deBlob: FUNCTION 00611730
FMOD::Event* ASoundInterface::PlayEvent( const TCHAR* a_szGroup, const TCHAR* a_szEvent, const TCHAR* a_szProject )
{
	FMOD::EventSystem* pEventSystem = TSound::GetSingleton()->GetEventSystem();
	TASSERT( TNULL != pEventSystem );

	FMOD::EventProject* pProject    = TNULL;
	FMOD::EventGroup*   pEventGroup = TNULL;
	FMOD::Event*        pEvent      = TNULL;

	FMOD_RESULT eResult = pEventSystem->getProject( ( a_szProject != TNULL ) ? a_szProject : ASoundMap::scm_szProjectName, &pProject );

	if ( TSound::ErrorCheck( eResult ) == TTRUE || pProject == TNULL )
		return TNULL;

	if ( TSound::ErrorCheck( pProject->getGroup( a_szGroup, false, &pEventGroup ) ) == TTRUE || pEventGroup == TNULL )
		return TNULL;

	if ( TSound::ErrorCheck( pEventGroup->getEvent( a_szEvent, FMOD_EVENT_ERROR_ON_DISKACCESS, &pEvent ) ) == TTRUE || pEvent == TNULL )
		return TNULL;

	pEvent->start();
	return pEvent;
}

// $deBlob: FUNCTION 00611820
FMOD::Event* ASoundInterface::PlayEvent( ASoundMap::GROUPID a_eGroupID, ASoundMap::EVENTID a_eEventID, FMOD_EVENT_MODE a_uiMode, TBOOL a_bErrorCheck )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TNULL;

	FMOD::Event* pEvent = GetEvent( a_eGroupID, a_eEventID, a_uiMode );

	if ( pEvent == TNULL )
		return TNULL;

	FMOD_EVENT_STATE uiState;
	FMOD_RESULT      eResult = pEvent->getState( &uiState );

	if ( eResult == FMOD_ERR_INVALID_HANDLE || TSound::ErrorCheck( eResult ) == TTRUE || ( uiState & FMOD_EVENT_STATE_READY ) == 0 )
		return TNULL;

	eResult = pEvent->start();

	if ( a_bErrorCheck )
		TSound::ErrorCheck( eResult );

	if ( eResult != FMOD_OK )
		return TNULL;

	TBOOL bMute = TFALSE;

	AStaticCVars::cvSoundMuteAllSounds.GetValue( bMute );

	pEvent->setMute( bMute );
	return pEvent;
}

// $deBlob: FUNCTION 006118b0
FMOD::Event* ASoundInterface::PlayEvent( ASoundMap::SOUNDID a_eSoundID, FMOD_EVENT_MODE a_uiMode )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TNULL;

	// NOTE: retail lets a_eSoundID == SOUNDID_COUNT through (the table has SOUNDID_COUNT entries)
	TASSERT( ASoundMap::SOUNDID_INVALID < a_eSoundID && ASoundMap::SOUNDID_COUNT >= a_eSoundID );

	if ( a_eSoundID == ASoundMap::SOUNDID_INVALID )
		return TNULL;

	// NOTE: a_uiMode isn't used, the event is played with FMOD_EVENT_DEFAULT
	return PlayEvent( ASoundMap::scm_aSounds[ a_eSoundID ].eGroupID, ASoundMap::scm_aSounds[ a_eSoundID ].eEventID, FMOD_EVENT_DEFAULT, TTRUE );
}

// $deBlob: FUNCTION 00611930
FMOD::Event* ASoundInterface::PlayEvent( const TCHAR* a_szGroup, TINT a_iEventIndex, FMOD_EVENT_MODE a_uiMode )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TNULL;

	FMOD::EventSystem* pEventSystem = TSound::GetSingleton()->GetEventSystem();
	TASSERT( TNULL != pEventSystem );

	FMOD::EventProject* pProject = TNULL;
	pEventSystem->getProjectByIndex( 0, &pProject );

	return PlayEvent( pProject, a_szGroup, a_iEventIndex, a_uiMode );
}

// $deBlob: FUNCTION 006119b0
FMOD::Event* ASoundInterface::PlayEvent( FMOD::EventProject* a_pProject, const TCHAR* a_szGroup, TINT a_iEventIndex, FMOD_EVENT_MODE a_uiMode )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TNULL;

	FMOD::EventGroup* pEventGroup;
	FMOD_RESULT       eResult = a_pProject->getGroup( a_szGroup, false, &pEventGroup );
	TASSERT( eResult == FMOD_OK );

	TINT iNumEvents = 0;
	pEventGroup->getNumEvents( &iNumEvents );
	TASSERT( iNumEvents > a_iEventIndex );

	FMOD::Event* pEvent;
	pEventGroup->getEventByIndex( a_iEventIndex, a_uiMode, &pEvent );
	TSound::ErrorCheck( pEvent->start() );

	return pEvent;
}

// $deBlob: FUNCTION 00611a90
FMOD::Event* ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID a_eGroupID, ASoundMap::EVENTID a_eEventID, const TVector4& a_rcPosition, FMOD_EVENT_MODE a_uiMode )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TNULL;

	FMOD::Event* pEvent = PlayEvent( a_eGroupID, a_eEventID, a_uiMode, TTRUE );

	if ( pEvent != TNULL )
	{
		FMOD_VECTOR vPosition = GetFMODVector( a_rcPosition );
		FMOD_RESULT eResult   = pEvent->set3DAttributes( &vPosition, TNULL, TNULL );

		if ( eResult != FMOD_ERR_INVALID_HANDLE )
			TSound::ErrorCheck( eResult );
	}

	return pEvent;
}

// NOTE: a_szProject and a_szGroup are only checked to exist, the state isn't tested (always TTRUE)
// $deBlob: FUNCTION 00611c50
TBOOL ASoundInterface::IsEventGroupInState( const TCHAR* a_szProject, const TCHAR* a_szGroup, FMOD_EVENT_STATE a_uiState, FMOD_EVENT_MODE a_uiMode )
{
	FMOD::EventProject* pProject;
	TSound::GetSingleton()->GetEventSystem()->getProject( a_szProject, &pProject );

	FMOD::EventGroup* pEventGroup = TNULL;
	pProject->getGroup( a_szGroup, false, &pEventGroup );
	TASSERT( TNULL != pEventGroup );

	return TTRUE;
}

// $deBlob: FUNCTION 00611b30
TBOOL ASoundInterface::IsEventGroupInState( ASoundMap::GROUPID a_eGroupID, FMOD_EVENT_STATE a_uiState, FMOD_EVENT_MODE a_uiMode )
{
	FMOD::EventGroup* pEventGroup = GetEventGroup( a_eGroupID, 0 );
	TASSERT( TNULL != pEventGroup );

	TINT iNumEvents = 0;

	if ( TSound::ErrorCheck( pEventGroup->getNumEvents( &iNumEvents ) ) == TTRUE )
		return TFALSE;

	FMOD::Event* pEvent = TNULL;

	for ( TINT i = 0; i < iNumEvents; i++ )
	{
		TSound::ErrorCheck( pEventGroup->getEventByIndex( i, a_uiMode, &pEvent ) );
		TASSERT( TNULL != pEvent );

		FMOD_EVENT_STATE uiState;
		FMOD_RESULT      eResult = pEvent->getState( &uiState );

		if ( eResult == FMOD_ERR_INVALID_HANDLE || TSound::ErrorCheck( eResult ) == TTRUE || ( a_uiState & uiState ) == 0 )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00611cd0
TBOOL ASoundInterface::IsEventInState( FMOD::Event* a_pEvent, FMOD_EVENT_STATE a_uiState )
{
	FMOD_EVENT_STATE uiState;
	FMOD_RESULT      eResult = a_pEvent->getState( &uiState );

	return eResult != FMOD_ERR_INVALID_HANDLE && TSound::ErrorCheck( eResult ) != TTRUE && ( uiState & a_uiState ) != 0;
}

// $deBlob: FUNCTION 00611e40
TBOOL ASoundInterface::IsEventLooping( ASoundMap::GROUPID a_eGroupID, ASoundMap::EVENTID a_eEventID )
{
	FMOD::Event* pEvent = GetEvent( a_eGroupID, a_eEventID, FMOD_EVENT_INFOONLY );

	if ( pEvent != TNULL )
	{
		FMOD_EVENT_INFO oInfo = {};

		if ( TSound::ErrorCheck( pEvent->getInfo( TNULL, TNULL, &oInfo ) ) != TTRUE )
			return oInfo.lengthms == -1;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00612150
void ASoundInterface::SetCategoryPausedAll( TBOOL a_bPaused )
{
	if ( TSound::GetSingleton()->IsInitialised() )
		GetEventCategory( ASoundMap::CATEGORYID_MASTER )->setPaused( a_bPaused );
}

// $deBlob: FUNCTION 00611d10
TBOOL ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID a_eCategoryID, TBOOL a_bPaused )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TTRUE;

	FMOD::EventCategory* pEventCategory = GetEventCategory( a_eCategoryID );
	TASSERT( TNULL != pEventCategory );

	return TSound::ErrorCheck( pEventCategory->setPaused( a_bPaused ) ) != TTRUE;
}

// $deBlob: FUNCTION 00611d80
TBOOL ASoundInterface::SetCategoryVolume( ASoundMap::CATEGORYID a_eCategoryID, TFLOAT a_fVolume )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TFALSE;

	TASSERT( 0.0f <= a_fVolume && 1.0f >= a_fVolume );

	FMOD::EventCategory* pEventCategory = GetEventCategory( a_eCategoryID );
	TASSERT( TNULL != pEventCategory );

	return TSound::ErrorCheck( pEventCategory->setVolume( a_fVolume ) ) != TTRUE;
}

// $deBlob: FUNCTION 00612130
void ASoundInterface::SetMasterVolume( TFLOAT a_fVolume )
{
	GetEventCategory( ASoundMap::CATEGORYID_MASTER )->setVolume( a_fVolume );
}

// $deBlob: FUNCTION 00612180
TFLOAT ASoundInterface::GetMasterVolume()
{
	TFLOAT fVolume;
	GetEventCategory( ASoundMap::CATEGORYID_MASTER )->getVolume( &fVolume );

	return fVolume;
}

// $deBlob: FUNCTION 00611f20
void ASoundInterface::MuteAllSounds( TBOOL a_bMute )
{
	TINT iNumProjects = 0;
	TSound::GetSingleton()->GetEventSystem()->getNumProjects( &iNumProjects );

	if ( iNumProjects != 0 )
	{
		for ( TINT i = 0; i < ASoundMap::CATEGORYID_NUMOF; i++ )
		{
			FMOD::EventCategory* pEventCategory = GetEventCategory( ASoundMap::CATEGORYID( i ) );
			TASSERT( TNULL != pEventCategory );

			bool bMuted = false;
			TSound::ErrorCheck( pEventCategory->getMute( &bMuted ) );

			if ( bMuted != ( a_bMute != TFALSE ) )
				TSound::ErrorCheck( pEventCategory->setMute( a_bMute ) );
		}
	}
}

// $deBlob: FUNCTION 00611ea0
void ASoundInterface::SetPosition( FMOD::Event* a_pEvent, const TVector4& a_rcPosition )
{
	FMOD_VECTOR vPosition = GetFMODVector( a_rcPosition );
	FMOD_RESULT eResult   = a_pEvent->set3DAttributes( &vPosition, TNULL, TNULL );

	if ( eResult != FMOD_ERR_INVALID_HANDLE )
		TSound::ErrorCheck( eResult );
}

// $deBlob: FUNCTION 006120b0
FMOD_RESULT ASoundInterface::SetPosition( FMOD::Event* a_pEvent, const TVector4& a_rcPosition, const TVector4& a_rcVelocity )
{
	FMOD_VECTOR vPosition = GetFMODVector( a_rcPosition );
	FMOD_VECTOR vVelocity = GetFMODVector( a_rcVelocity );

	return a_pEvent->set3DAttributes( &vPosition, &vVelocity, TNULL );
}

// $deBlob: FUNCTION 00611f00
FMOD_VECTOR ASoundInterface::GetFMODVector( const TVector4& a_rcVector )
{
	FMOD_VECTOR vVector;
	vVector.x = a_rcVector.x;
	vVector.y = a_rcVector.y;
	vVector.z = a_rcVector.z;

	return vVector;
}

// $deBlob: FUNCTION 006121a0
void ASoundInterface::DebugMemory()
{
	TINT iCurrentAlloced = 0;
	TINT iMaxAlloced     = 0;
	FMOD::Memory_GetStats( &iCurrentAlloced, &iMaxAlloced, true );
}

// $deBlob: FUNCTION 006121f0
void ASoundInterface::LoadEventSystem()
{
	if ( TSound::GetSingleton()->IsInitialised() )
	{
		FMOD::EventSystem* pEventSystem = TSound::GetSingleton()->GetEventSystem();

		TString8 strMediaPath( "Data/Sound/" );
		strMediaPath.Concat( "EN/" );

		pEventSystem->setMediaPath( strMediaPath.GetString() );
	}
}

// $deBlob: FUNCTION 006121d0
void ASoundInterface::UnloadEventSystem()
{
	if ( ASoundLoadHelper::GetSingleton() != TNULL )
	{
		ASoundLoadHelper::GetSingleton()->UnloadSoundEventGroups();
		ASoundLoadHelper::GetSingleton()->UnloadEventProjects();
	}
}
