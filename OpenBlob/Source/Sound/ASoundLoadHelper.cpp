#include "pch.h"
#include "ASoundLoadHelper.h"
#include "ASoundInterface.h"
#include "ARandom.h"
#include "Options/AOptions.h"

#include <Toshi/T2String8.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Loads the event groups blocking instead of FMOD_EVENT_NONBLOCKING; NOTE: only ever cleared in retail
static TBOOL s_bLoadBlocking = TFALSE;

// $deBlob: FUNCTION 00612350
ASoundLoadHelper::ASoundLoadHelper()
{
	m_pLoadingLoopEvent    = TNULL;
	m_pLoadingEventGroup   = TNULL;
	m_eState               = UNLOADED;
	m_pEventProjects       = TNULL;
	m_itEventProject       = TNULL;
	m_itEventGroup         = TNULL;
	m_bLoadingSoundsLoaded = TFALSE;
	m_bLoadingLoopLoaded   = TFALSE;
	m_bGUILoaded           = TFALSE;
	m_bFrontEndLoaded      = TFALSE;
	m_bPlayLoadingLoop     = TFALSE;
	m_iLoadingLoop         = -1;

	MakeStringToIDHashTable();

	m_pEventGroups = new EventGroup[ MAX_EVENTGROUPS ];

	for ( TINT i = 0; i < MAX_EVENTGROUPS; i++ )
		m_oFreeEventGroups.PushBack( &m_pEventGroups[ i ] );

	m_pEventProjects = new EventProject[ MAX_EVENTPROJECTS ];

	for ( TINT i = 0; i < MAX_EVENTPROJECTS; i++ )
		m_oFreeEventProjects.PushBack( &m_pEventProjects[ i ] );

	m_itEventGroup = m_oEventGroups.Begin();
}

// $deBlob: FUNCTION 00612520
// $deBlob: FUNCTION 00613d60
ASoundLoadHelper::~ASoundLoadHelper()
{
	m_oEventGroupHashTable.Destroy();
	m_oEventGroups.Clear();
	m_oFreeEventGroups.Clear();
	m_oFreeEventProjects.Clear();
	m_oEventProjects.Clear();

	if ( m_pEventGroups != TNULL )
		delete[] m_pEventGroups;

	if ( m_pEventProjects != TNULL )
		delete[] m_pEventProjects;

	m_pEventGroups = TNULL;
}

// $deBlob: FUNCTION 00612670
TBOOL ASoundLoadHelper::IsEventGroupLoaded( ASoundMap::GROUPID a_eGroupID )
{
	if ( a_eGroupID >= ASoundMap::GROUPID_NUMOF )
		return TFALSE;

	const TCHAR* szGroup = ASoundMap::scm_aGroupNames[ a_eGroupID ];

	for ( auto it = m_oEventGroups.Begin(); it != m_oEventGroups.End(); it++ )
	{
		if ( T2String8::Compare( it->szGroup, szGroup ) == 0 )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00612720
void ASoundLoadHelper::LoadSounds( TINT a_iNumGroups, TCHAR** a_ppGroups )
{
	TASSERT( UNLOADED == m_eState || LOADEDPROJECTS == m_eState );

	for ( TINT i = 0; i < a_iNumGroups; i++ )
		AddSoundGroupToLoad( a_ppGroups[ i ], TNULL );

	m_eState = LOADGROUP;
}

// $deBlob: FUNCTION 006127a0
void ASoundLoadHelper::UnloadLoadingLoopSounds()
{
	for ( auto it = m_oEventGroups.Begin(); it != m_oEventGroups.End(); it++ )
	{
		if ( T2String8::CompareNoCase( it->szGroup, "sfx/loading_loop", T2String8::Length( "sfx/loading_loop" ) ) == 0 )
		{
			ASoundInterface::FreeEventGroup( it->szGroup, it->szProject );

			EventGroup* pEventGroup = it;
			pEventGroup->Remove();
			m_oFreeEventGroups.PushBack( pEventGroup );
			return;
		}
	}

	m_bPlayLoadingLoop = TFALSE;
}

// $deBlob: FUNCTION 00612860
void ASoundLoadHelper::UnloadSoundEventGroups()
{
	for ( auto it = m_oEventGroups.Begin(); it != m_oEventGroups.End(); )
	{
		EventGroup* pEventGroup = it;
		ASoundInterface::FreeEventGroup( pEventGroup->szGroup, pEventGroup->szProject );

		it++;
		pEventGroup->Remove();
		m_oFreeEventGroups.PushBack( pEventGroup );
	}

	m_itEventGroup         = m_oEventGroups.Begin();
	m_bLoadingSoundsLoaded = TFALSE;
	m_bLoadingLoopLoaded   = TFALSE;
	m_bGUILoaded           = TFALSE;
	m_bFrontEndLoaded      = TFALSE;
	m_bPlayLoadingLoop     = TFALSE;
	m_iLoadingLoop         = -1;
	m_eState               = UNLOADED;
}

// $deBlob: FUNCTION 00612930
void ASoundLoadHelper::UnloadEventProjects()
{
	TSound::GetSingleton()->GetEventSystem()->unload();

	for ( auto it = m_oEventProjects.Begin(); it != m_oEventProjects.End(); )
	{
		EventProject* pEventProject = it;

		it++;
		pEventProject->Remove();
		m_oFreeEventProjects.PushBack( pEventProject );
	}

	m_itEventProject = m_oEventProjects.Begin();
}

// $deBlob: FUNCTION 00612a20
void ASoundLoadHelper::Update()
{
	switch ( m_eState )
	{
		case LOADINGPROJECTS:
			LoadingUpdate();
			break;
		case LOADGROUP:
			LoadNextEventGroup();
			break;
		case LOADINGGROUP:
			LoadingEventGroupUpdate();
			break;
	}
}

// $deBlob: FUNCTION 00612a40
void ASoundLoadHelper::LoadFrontEndSounds()
{
	AddLoadingLoopSounds();
	AddSoundGroupToLoad( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_FRONT_END ], TNULL );
	AddSoundGroupToLoad( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_GUI ], TNULL );
}

// $deBlob: FUNCTION 00612a70
void ASoundLoadHelper::LoadInGameSounds()
{
	AddSoundGroupToLoad( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_COMMON ], TNULL );
	AddSoundGroupToLoad( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_LOADING_SOUNDS ], TNULL );
	AddSoundGroupToLoad( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_GUI ], TNULL );
	AddSoundGroupToLoad( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_INTERACTABLES ], TNULL );
	AddSoundGroupToLoad( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_BLOB_SFX ], TNULL );
	AddSoundGroupToLoad( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_LEVELS_COMMON_LEVELS ], TNULL );
	AddSoundGroupToLoad( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_PHYSICS_SOUNDS ], TNULL );
	AddSoundGroupToLoad( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_PLATFORMS ], TNULL );
}

// one of the four loading loops (random) and the loading sounds go to the front of the list
// $deBlob: FUNCTION 00612af0
void ASoundLoadHelper::AddLoadingLoopSounds()
{
	if ( !m_oFreeEventGroups.IsEmpty() )
	{
		m_iLoadingLoop = ARandom::GetSingleton()->GetRandom().GetInt( 4 ) + 1;

		EventGroup* pEventGroup = m_oFreeEventGroups.Head();
		pEventGroup->Remove();
		T2String8::Format( pEventGroup->szGroup, "SFX/LOADING_LOOP_%d", m_iLoadingLoop );
		T2String8::Copy( pEventGroup->szProject, ASoundMap::scm_szProjectName );
		m_oEventGroups.PushFront( pEventGroup );

		pEventGroup = m_oFreeEventGroups.Head();
		pEventGroup->Remove();
		T2String8::Copy( pEventGroup->szGroup, ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_LOADING_SOUNDS ] );
		T2String8::Copy( pEventGroup->szProject, ASoundMap::scm_szProjectName );
		m_oEventGroups.PushFront( pEventGroup );

		m_itEventGroup = m_oEventGroups.Begin();
	}
}

// $deBlob: FUNCTION 00612c10
void ASoundLoadHelper::StopLoadingLoop()
{
	if ( m_pLoadingLoopEvent != TNULL )
	{
		m_pLoadingLoopEvent->stop( false );
		m_pLoadingLoopEvent = TNULL;
	}
}

// $deBlob: FUNCTION 00612c30
void ASoundLoadHelper::PlayLoadingSound()
{
	if ( m_bLoadingSoundsLoaded )
		ASoundInterface::PlayEvent( ASoundMap::GROUPID_LOADING_SOUNDS, ASoundMap::EVENTID( 282 ), FMOD_EVENT_DEFAULT, TTRUE );
}

// $deBlob: FUNCTION 00612c50
void ASoundLoadHelper::PlayLoadingLoop()
{
	TCHAR szGroup[ 128 ];
	TCHAR szEvent[ 128 ];
	T2String8::Format( szGroup, "SFX/LOADING_LOOP_%d", m_iLoadingLoop );
	T2String8::Format( szEvent, "loading_loop_%d", m_iLoadingLoop );

	if ( m_bLoadingLoopLoaded )
		m_pLoadingLoopEvent = ASoundInterface::PlayEvent( szGroup, szEvent, TNULL );
}

// $deBlob: FUNCTION 00612cc0
ASoundMap::GROUPID ASoundLoadHelper::GetEventGroupID( const TCHAR* a_szGroup )
{
	TCHAR szName[ 128 ];
	TStringManager::String8Copy( szName, a_szGroup, sizeof( szName ) );

	THashTable::HashNode* pNode = m_oEventGroupHashTable.Find( szName );

	return ( pNode != TNULL ) ? ASoundMap::GROUPID( pNode->iItemIndex ) : ASoundMap::GROUPID_INVALID;
}

// $deBlob: FUNCTION 00612d30
void ASoundLoadHelper::AddProjectToLoad( const TCHAR* a_szProject )
{
	for ( auto it = m_oEventProjects.Begin(); it != m_oEventProjects.End(); it++ )
	{
		if ( T2String8::CompareNoCase( it->szName, a_szProject ) == 0 )
			return;
	}

	if ( !m_oFreeEventProjects.IsEmpty() )
	{
		EventProject* pEventProject = m_oFreeEventProjects.Head();
		pEventProject->Remove();
		T2String8::Copy( pEventProject->szName, a_szProject );
		pEventProject->pProject = TNULL;
		m_oEventProjects.PushBack( pEventProject );

		m_itEventProject = m_oEventProjects.Begin();
	}
}

// $deBlob: FUNCTION 00612e70
void ASoundLoadHelper::AddSoundGroupToLoad( const TCHAR* a_szGroup, const TCHAR* a_szProject )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return;

	FMOD::EventSystem*  pEventSystem = TSound::GetSingleton()->GetEventSystem();
	FMOD::EventGroup*   pEventGroup  = TNULL;
	FMOD::EventProject* pProject;
	TINT                iGroupIndex = -1;
	TCHAR*              szGroupName;

	FMOD_RESULT eResult = pEventSystem->getProject( ( a_szProject != TNULL ) ? a_szProject : ASoundMap::scm_szProjectName, &pProject );

	if ( eResult != FMOD_OK )
	{
		TINT iNumProjects = 0;
		eResult           = pEventSystem->getNumProjects( &iNumProjects );
		TASSERT( eResult == FMOD_OK );

		// NOTE: retail gets the infos of every loaded project for a print it compiled out
		for ( TINT i = 0; i < iNumProjects; i++ )
		{
			FMOD::EventProject* pLoadedProject;
			pEventSystem->getProjectByIndex( i, &pLoadedProject );

			FMOD_EVENT_PROJECTINFO oLoadedInfo;
			oLoadedInfo.name[ 0 ] = '\0';
			pLoadedProject->getInfo( &oLoadedInfo );
		}
	}

	TASSERT( eResult == FMOD_OK, "ASoundLoadHelper::AddSoundGroupToLoad unable to get group becaues project not loaded" );

	FMOD_EVENT_PROJECTINFO oInfo;
	oInfo.index = 0;
	eResult     = pProject->getInfo( &oInfo );
	TASSERT( eResult == FMOD_OK, "ASoundLoadHelper::AddSoundGroupToLoad unable to get info from project file" );

	eResult = pProject->getGroup( a_szGroup, false, &pEventGroup );
	TASSERT( eResult == FMOD_OK, "ASoundLoadHelper::AddSoundGroupToLoad unable to get group %s" );

	pEventGroup->getInfo( &iGroupIndex, &szGroupName );

	for ( auto it = m_oEventGroups.Begin(); it != m_oEventGroups.End(); it++ )
	{
		if ( T2String8::CompareNoCase( it->szProject, oInfo.name ) == 0 && T2String8::CompareNoCase( it->szGroup, a_szGroup ) == 0 )
			return;
	}

	if ( !m_oFreeEventGroups.IsEmpty() )
	{
		EventGroup* pFreeEventGroup = m_oFreeEventGroups.Head();
		pFreeEventGroup->Remove();
		T2String8::Copy( pFreeEventGroup->szGroup, a_szGroup );
		T2String8::Copy( pFreeEventGroup->szProject, oInfo.name );
		m_oEventGroups.PushBack( pFreeEventGroup );

		m_itEventGroup = m_oEventGroups.Begin();
	}
}

// $deBlob: FUNCTION 006131e0
void ASoundLoadHelper::SetState( STATE a_eState )
{
	if ( a_eState == LOADEDPROJECTS )
		AOptions::GetSingleton()->UpdateVolumes( TTRUE, -1.0f, -1.0f );

	m_eState = a_eState;
}

// $deBlob: FUNCTION 00613220
void ASoundLoadHelper::LoadGroupID( ASoundMap::GROUPID a_eGroupID )
{
	TASSERT( ASoundMap::GROUPID_INVALID != a_eGroupID );
	AddSoundGroupToLoad( ASoundMap::scm_aGroupNames[ a_eGroupID ], TNULL );
}

// $deBlob: FUNCTION 00613290
void ASoundLoadHelper::UnloadEventSystem()
{
	TSound::GetSingleton()->GetEventSystem()->unload();
	m_itEventProject = m_oEventProjects.Begin();
}

// $deBlob: FUNCTION 006132c0
void ASoundLoadHelper::PlayLoadingLoop( const TCHAR* a_szGroup, const TCHAR* a_szEvent )
{
	if ( m_bLoadingLoopLoaded )
		m_pLoadingLoopEvent = ASoundInterface::PlayEvent( a_szGroup, a_szEvent, TNULL );
}

// Starts loading the next event group (FMOD_EVENT_NONBLOCKING)
// $deBlob: FUNCTION 006132f0
void ASoundLoadHelper::LoadNextEventGroup()
{
	if ( !m_oEventGroups.IsEmpty() )
	{
		if ( !TSound::GetSingleton()->IsInitialised() )
			m_itEventGroup = m_oEventGroups.End();

		if ( m_itEventGroup != m_oEventGroups.End() )
		{
			EventGroup*         pEventGroup = m_itEventGroup;
			FMOD_EVENT_RESOURCE eResource   = FMOD_EVENT_RESOURCE_SAMPLES;

			// NOTE: retail compares the group names with the project name of the entry, so this never matches (and
			// scm_bLoadStreamsOnly is all TFALSE anyway)
			for ( TINT i = 0; i < ASoundMap::GROUPID_NUMOF; i++ )
			{
				if ( T2String8::CompareNoCase( ASoundMap::scm_aGroupNames[ i ], pEventGroup->szProject ) == 0 && ASoundMap::scm_bLoadStreamsOnly[ i ] )
				{
					eResource = FMOD_EVENT_RESOURCE_STREAMS;
					break;
				}
			}

			m_pLoadingEventGroup = ASoundInterface::LoadEventGroup( pEventGroup->szProject, pEventGroup->szGroup, eResource, s_bLoadBlocking ? FMOD_EVENT_DEFAULT : FMOD_EVENT_NONBLOCKING );
			m_eState             = LOADINGGROUP;
			return;
		}
	}

	m_eState = LOADED;
}

// Waits for the event group to be loaded
// $deBlob: FUNCTION 00613400
void ASoundLoadHelper::LoadingEventGroupUpdate()
{
	if ( m_itEventGroup == m_oEventGroups.End() )
	{
		s_bLoadBlocking = TFALSE;
		m_eState        = LOADED;
		return;
	}

	EventGroup* pEventGroup = m_itEventGroup;

	// NOTE: retail stays in this state if LoadEventGroup failed (m_pLoadingEventGroup is TNULL)
	if ( m_pLoadingEventGroup == TNULL )
		return;

	FMOD_EVENT_STATE uiState = FMOD_EVENT_STATE_ERROR;
	m_pLoadingEventGroup->getState( &uiState );

	if ( uiState & FMOD_EVENT_STATE_LOADING )
		return;

	if ( uiState & FMOD_EVENT_STATE_ERROR )
	{
		m_pLoadingEventGroup->loadEventData( FMOD_EVENT_RESOURCE_SAMPLES, FMOD_EVENT_NONBLOCKING );
		return;
	}

	m_pLoadingEventGroup = TNULL;

	if ( !m_bGUILoaded && T2String8::CompareNoCase( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_GUI ], pEventGroup->szGroup ) == 0 )
	{
		m_bGUILoaded = TTRUE;
	}
	else if ( !m_bFrontEndLoaded && T2String8::CompareNoCase( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_FRONT_END ], pEventGroup->szGroup ) == 0 )
	{
		m_bFrontEndLoaded = TTRUE;
	}
	else if ( !m_bLoadingSoundsLoaded && T2String8::CompareNoCase( ASoundMap::scm_aGroupNames[ ASoundMap::GROUPID_LOADING_SOUNDS ], pEventGroup->szGroup ) == 0 )
	{
		m_bLoadingSoundsLoaded = TTRUE;
	}
	else if ( !m_bLoadingLoopLoaded && m_iLoadingLoop > 0 )
	{
		m_bLoadingLoopLoaded = TTRUE;

		if ( m_bPlayLoadingLoop )
		{
			m_bPlayLoadingLoop = TFALSE;

			TCHAR szGroup[ 128 ];
			T2String8::Format( szGroup, "SFX/LOADING_LOOP_%d", m_iLoadingLoop );

			if ( T2String8::CompareNoCase( szGroup, pEventGroup->szGroup ) == 0 )
				PlayLoadingLoop();
		}
	}

	m_itEventGroup++;
	m_eState = LOADGROUP;
}

// Loads the next event project (.fev)
// $deBlob: FUNCTION 006135f0
void ASoundLoadHelper::LoadingUpdate()
{
	if ( !m_oEventProjects.IsEmpty() )
	{
		if ( !TSound::GetSingleton()->IsInitialised() )
			m_itEventProject = m_oEventProjects.End();

		if ( m_itEventProject != m_oEventProjects.End() )
		{
			EventProject* pEventProject = m_itEventProject;

			if ( TSound::GetSingleton()->GetEventSystem()->load( pEventProject->szName, TNULL, &pEventProject->pProject ) != FMOD_OK )
			{
				// NOTE: retail unloads every project and starts again (forever if a project can't be loaded)
				TSound::GetSingleton()->GetEventSystem()->unload();
				m_itEventProject = m_oEventProjects.Begin();
			}
			else
			{
				m_itEventProject++;
			}

			return;
		}
	}

	AOptions::GetSingleton()->UpdateVolumes( TTRUE, -1.0f, -1.0f );
	m_eState = LOADEDPROJECTS;
}

// $deBlob: FUNCTION 00613720
void ASoundLoadHelper::MakeStringToIDHashTable()
{
	m_oEventGroupHashTable.Create( ASoundMap::GROUPID_NUMOF, 128, ASoundMap::GROUPID_NUMOF, ASoundMap::GROUPID_NUMOF );
	m_oEventGroupHashTable.SetItemCompareFunction( EventGroupNameHashCompareFunc );

	for ( TINT i = 0; i < ASoundMap::GROUPID_NUMOF; i++ )
	{
		TCHAR szName[ 128 ];
		TStringManager::String8Copy( szName, ASoundMap::scm_aGroupNames[ i ], sizeof( szName ) );
		m_oEventGroupHashTable.Insert( szName );
	}
}

// $deBlob: FUNCTION 006137a0
TBOOL ASoundLoadHelper::EventGroupNameHashCompareFunc( void* a_pMem1, void* a_pMem2, TINT a_iSize )
{
	if ( TStringManager::String8Compare( TSTATICCAST( TCHAR, a_pMem1 ), TSTATICCAST( TCHAR, a_pMem2 ) ) == 0 )
		return TTRUE;

	return TFALSE;
}
