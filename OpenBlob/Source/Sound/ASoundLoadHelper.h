#pragma once
#include "ASoundMap.h"

#include <Sound/TSound.h>
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Toshi/THashTable.h>

//-----------------------------------------------------------------------------
// ASoundLoadHelper (0x78): loads the FMOD Event projects (.fev) and then their
// event groups, one per update.
//-----------------------------------------------------------------------------
class ASoundLoadHelper : public Toshi::TSingleton<ASoundLoadHelper>
{
public:
	friend class ARegionManager;

	enum STATE : TINT
	{
		UNLOADED,
		LOADINGPROJECTS,
		LOADEDPROJECTS,
		LOADGROUP,
		LOADINGGROUP,
		LOADED,
	};

	static TCONSTEXPR_VAR TINT MAX_EVENTGROUPS   = 35;
	static TCONSTEXPR_VAR TINT MAX_EVENTPROJECTS = 5;

	// 0x108
	struct EventGroup : public Toshi::T2DList<EventGroup>::Node
	{
		EventGroup() {}

		TCHAR szProject[ 128 ]; // 0x08
		TCHAR szGroup[ 128 ];   // 0x88
	};

	// 0x8C
	struct EventProject : public Toshi::T2DList<EventProject>::Node
	{
		EventProject() {}

		TCHAR               szName[ 128 ]; // 0x08
		FMOD::EventProject* pProject;      // 0x88
	};

public:
	ASoundLoadHelper();
	virtual ~ASoundLoadHelper();

	void Update();

	// The projects
	void AddProjectToLoad( const TCHAR* a_szProject );
	void UnloadEventProjects();
	void UnloadEventSystem();

	// The event groups
	void LoadSounds( TINT a_iNumGroups, TCHAR** a_ppGroups );
	void LoadGroupID( ASoundMap::GROUPID a_eGroupID );
	void AddSoundGroupToLoad( const TCHAR* a_szGroup, const TCHAR* a_szProject );
	void LoadFrontEndSounds();
	void LoadInGameSounds();
	void AddLoadingLoopSounds();
	void UnloadLoadingLoopSounds();
	// $deBlob: FUNCTION 005ca700
	void  SetPlayLoadingLoop( TBOOL a_bPlay ) { m_bPlayLoadingLoop = a_bPlay; }
	void  UnloadSoundEventGroups();
	TBOOL IsEventGroupLoaded( ASoundMap::GROUPID a_eGroupID );

	ASoundMap::GROUPID GetEventGroupID( const TCHAR* a_szGroup );

	// The loading screen sounds
	void PlayLoadingSound();
	void PlayLoadingLoop();
	void PlayLoadingLoop( const TCHAR* a_szGroup, const TCHAR* a_szEvent );
	void StopLoadingLoop();

	void  SetState( STATE a_eState );
	STATE GetState() const { return m_eState; }

	// $deBlob: FUNCTION 00629de0
	TBOOL IsLoadingSoundsLoaded() const { return m_bLoadingSoundsLoaded; }
	// $deBlob: FUNCTION 0063a470
	TBOOL IsLoadingLoopLoaded() const { return m_bLoadingLoopLoaded; }
	// $deBlob: FUNCTION 00626ea0
	TBOOL IsGUILoaded() const { return m_bGUILoaded; }
	// $deBlob: FUNCTION 0062aea0
	TBOOL IsFrontEndLoaded() const { return m_bFrontEndLoaded; }

private:
	void LoadingUpdate();
	void LoadNextEventGroup();
	void LoadingEventGroupUpdate();
	void MakeStringToIDHashTable();

	static TBOOL EventGroupNameHashCompareFunc( void* a_pMem1, void* a_pMem2, TINT a_iSize );

private:
	FMOD::Event*                           m_pLoadingLoopEvent;    // 0x04
	FMOD::EventGroup*                      m_pLoadingEventGroup;   // 0x08
	STATE                                  m_eState;               // 0x0C
	Toshi::T2DList<EventGroup>             m_oEventGroups;         // 0x10
	Toshi::T2DList<EventGroup>             m_oFreeEventGroups;     // 0x18
	EventGroup*                            m_pEventGroups;         // 0x20
	Toshi::T2DList<EventProject>           m_oEventProjects;       // 0x24
	Toshi::T2DList<EventProject>           m_oFreeEventProjects;   // 0x2C
	EventProject*                          m_pEventProjects;       // 0x34
	Toshi::T2DList<EventProject>::Iterator m_itEventProject;       // 0x38
	Toshi::T2DList<EventGroup>::Iterator   m_itEventGroup;         // 0x3C
	Toshi::THashTable                      m_oEventGroupHashTable; // 0x40
	TBOOL                                  m_bLoadingSoundsLoaded; // 0x6C
	TBOOL                                  m_bLoadingLoopLoaded;   // 0x6D
	TBOOL                                  m_bGUILoaded;           // 0x6E
	TBOOL                                  m_bFrontEndLoaded;      // 0x6F
	TBOOL                                  m_bPlayLoadingLoop;     // 0x70
	TINT                                   m_iLoadingLoop;         // 0x74
};
