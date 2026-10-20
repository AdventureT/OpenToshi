#pragma once

//-----------------------------------------------------------------------------
// ASoundMap: the ids of the FMOD Event project Blob07.fev (Data/Sound/EN): its event
// groups, the events (index of the event in its group) and the categories.
// Event ids are numbers (their names are in the .fev).
//-----------------------------------------------------------------------------
class ASoundMap
{
public:
	enum GROUPID : TINT
	{
		GROUPID_INVALID = -1,
		GROUPID_BLOB_SFX,
		GROUPID_COMMON,
		GROUPID_FRONT_END,
		GROUPID_GRAYDIAN,
		GROUPID_GUI,
		GROUPID_INTERACTABLES,
		GROUPID_LEVELS_ABYSS,
		GROUPID_LEVELS_BRIDGE,
		GROUPID_LEVELS_COMMON_LEVELS,
		GROUPID_LEVELS_DAM,
		GROUPID_LEVELS_GUGGENTRAZ,
		GROUPID_LEVELS_INKY_DOCKS,
		GROUPID_LEVELS_INKY_FUNPARK,
		GROUPID_LEVELS_LAST_RESORT,
		GROUPID_LEVELS_LM_COMMON,
		GROUPID_LEVELS_MILITARY,
		GROUPID_LEVELS_MULTIPLAYER,
		GROUPID_LEVELS_SLUMS,
		GROUPID_LEVELS_SPACESHIP,
		GROUPID_LEVELS_SPRINTS,
		GROUPID_LOADING_LOOP_1,
		GROUPID_LOADING_LOOP_2,
		GROUPID_LOADING_LOOP_3,
		GROUPID_LOADING_LOOP_4,
		GROUPID_LOADING_SOUNDS,
		GROUPID_NMES_INKY_BATON,
		GROUPID_NMES_INKY_BIKE,
		GROUPID_NMES_INKY_BLIMP,
		GROUPID_NMES_INKY_PEN,
		GROUPID_NMES_INKY_TANK,
		GROUPID_NMES_INKY_TURRET,
		GROUPID_NMES_LEECH_BOT,
		GROUPID_NMES_INKY_ELITE,
		GROUPID_NMES_BOSS_MECH,
		GROUPID_PHYSICS_SOUNDS,
		GROUPID_PLATFORMS,
		GROUPID_RAYDIAN,
		GROUPID_REVOLUTIONARIES_REVOLUTIONARY,
		GROUPID_REVOLUTIONARIES_REV_ORANGE,
		GROUPID_REVOLUTIONARIES_REV_BLUE,
		GROUPID_REVOLUTIONARIES_REV_PROF,
		GROUPID_VEHICLES_CAR,
		GROUPID_VEHICLES_BOAT,
		GROUPID_VEHICLES_MONORAIL,
		GROUPID_VEHICLES_PRISON_CAR,
		GROUPID_NUMOF
	};

	// Index into scm_iEventIDs (the index of the event inside its group)
	enum EVENTID : TINT
	{
		EVENTID_INVALID = -1,
		EVENTID_COUNT   = 466
	};

	// Index into scm_aSounds (a group and one of its events)
	enum SOUNDID : TINT
	{
		SOUNDID_INVALID = -1,
		SOUNDID_COUNT   = 466
	};

	// The categories of Blob07.fev (scm_iCategoryIDs: the index for EventSystem::getCategoryByIndex)
	enum CATEGORYID : TINT
	{
		CATEGORYID_MASTER,
		CATEGORYID_MUSIC,
		CATEGORYID_INGAME,
		CATEGORYID_SPEECH,
		CATEGORYID_GUI,
		CATEGORYID_NUMOF
	};

	struct Sound
	{
		GROUPID eGroupID;
		EVENTID eEventID;
	};

public:
	static const TCHAR* scm_szProjectName; // "Blob07"
	static const TCHAR* scm_aGroupNames[ GROUPID_NUMOF ];
	static const TINT   scm_iEventIDs[ EVENTID_COUNT ];
	static const TINT   scm_iCategoryIDs[ CATEGORYID_NUMOF ];
	static const Sound  scm_aSounds[ SOUNDID_COUNT ];
	static const TBOOL  scm_bLoadStreamsOnly[ GROUPID_NUMOF ]; // (all TFALSE)
};
