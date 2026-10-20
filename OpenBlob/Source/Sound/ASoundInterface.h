#pragma once
#include "ASoundMap.h"

#include <Sound/TSound.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// ASoundInterface: the game's helpers around the FMOD Event system of TSound (no instance, static functions).
//-----------------------------------------------------------------------------
class ASoundInterface
{
public:
	static void LoadEventSystem();
	static void UnloadEventSystem();
	static void DebugMemory();

	static FMOD::EventGroup* LoadEventGroup( const TCHAR* a_szProject, const TCHAR* a_szGroup, FMOD_EVENT_RESOURCE a_eResource, FMOD_EVENT_MODE a_uiMode );
	static FMOD::EventGroup* LoadEventGroup( const TCHAR* a_szGroup, FMOD_EVENT_RESOURCE a_eResource, FMOD_EVENT_MODE a_uiMode );
	static FMOD::EventGroup* LoadEventGroup( ASoundMap::GROUPID a_eGroupID, FMOD_EVENT_RESOURCE a_eResource, FMOD_EVENT_MODE a_uiMode );
	static TBOOL             LoadEventGroup( FMOD::EventGroup* a_pEventGroup, FMOD_EVENT_RESOURCE a_eResource, FMOD_EVENT_MODE a_uiMode );
	static TBOOL             FreeEventGroup( ASoundMap::GROUPID a_eGroupID, FMOD::Event* a_pEvent );
	static TBOOL             FreeEventGroup( const TCHAR* a_szGroup, const TCHAR* a_szProject );

	static FMOD::EventProject*  GetEventProject( const TCHAR* a_szProject );
	static FMOD::EventGroup*    GetEventGroup( ASoundMap::GROUPID a_eGroupID, TINT a_iProject );
	static FMOD::EventGroup*    GetEventGroup( const TCHAR* a_szGroup, TINT a_iProject );
	static FMOD::EventCategory* GetEventCategory( ASoundMap::CATEGORYID a_eCategoryID );
	static FMOD::Event*         GetEvent( ASoundMap::GROUPID a_eGroupID, ASoundMap::EVENTID a_eEventID, FMOD_EVENT_MODE a_uiMode );

	static FMOD::Event* PlayEvent( const TCHAR* a_szGroup, const TCHAR* a_szEvent, const TCHAR* a_szProject );
	static FMOD::Event* PlayEvent( ASoundMap::GROUPID a_eGroupID, ASoundMap::EVENTID a_eEventID, FMOD_EVENT_MODE a_uiMode, TBOOL a_bErrorCheck );
	static FMOD::Event* PlayEvent( ASoundMap::SOUNDID a_eSoundID, FMOD_EVENT_MODE a_uiMode );
	static FMOD::Event* PlayEvent( const TCHAR* a_szGroup, TINT a_iEventIndex, FMOD_EVENT_MODE a_uiMode );
	static FMOD::Event* PlayEvent( FMOD::EventProject* a_pProject, const TCHAR* a_szGroup, TINT a_iEventIndex, FMOD_EVENT_MODE a_uiMode );
	static FMOD::Event* PlayEventAtPosition( ASoundMap::GROUPID a_eGroupID, ASoundMap::EVENTID a_eEventID, const Toshi::TVector4& a_rcPosition, FMOD_EVENT_MODE a_uiMode );

	static TBOOL IsEventGroupInState( const TCHAR* a_szProject, const TCHAR* a_szGroup, FMOD_EVENT_STATE a_uiState, FMOD_EVENT_MODE a_uiMode );
	static TBOOL IsEventGroupInState( ASoundMap::GROUPID a_eGroupID, FMOD_EVENT_STATE a_uiState, FMOD_EVENT_MODE a_uiMode );
	static TBOOL IsEventInState( FMOD::Event* a_pEvent, FMOD_EVENT_STATE a_uiState );
	static TBOOL IsEventLooping( ASoundMap::GROUPID a_eGroupID, ASoundMap::EVENTID a_eEventID );

	static void   SetCategoryPausedAll( TBOOL a_bPaused );
	static TBOOL  SetCategoryPaused( ASoundMap::CATEGORYID a_eCategoryID, TBOOL a_bPaused );
	static TBOOL  SetCategoryVolume( ASoundMap::CATEGORYID a_eCategoryID, TFLOAT a_fVolume );
	static void   SetMasterVolume( TFLOAT a_fVolume );
	static TFLOAT GetMasterVolume();
	static void   MuteAllSounds( TBOOL a_bMute );

	static void        SetPosition( FMOD::Event* a_pEvent, const Toshi::TVector4& a_rcPosition );
	static FMOD_RESULT SetPosition( FMOD::Event* a_pEvent, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcVelocity );
	static FMOD_VECTOR GetFMODVector( const Toshi::TVector4& a_rcVector );
};
