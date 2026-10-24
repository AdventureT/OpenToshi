#pragma once
#include "AAIScheduleList.h"
#include <Toshi/TObject.h>

//-----------------------------------------------------------------------------
// Purpose: the schedules every AI has (BASE_Idle...BASE_Alert, 1-11), set up by the class's initialisation (its TClass
// has InitialiseStatic as InitialiseClass; AApplication::OnCreate initialises every class).
//-----------------------------------------------------------------------------
class AAIBaseSchedules : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AAIBaseSchedules, Toshi::TObject );

public:
	enum AAISCHEDULE : TINT
	{
		BASE_Idle       = 1,
		BASE_Fidget     = 2,
		BASE_Patrol     = 3,
		BASE_Recoil     = 4,
		BASE_Attack     = 5,
		BASE_Block      = 6,
		BASE_GotoLeader = 7,
		BASE_Flee       = 9,
		BASE_Alert      = 10,
		BASE_Death      = 11,
	};

	static TCONSTEXPR_VAR TINT NUM_SCHEDULES = 11;

public:
	// $deBlob: FUNCTION 007ea780
	~AAIBaseSchedules() {}

	static void InitialiseStatic();

public:
	static AAIScheduleList<NUM_SCHEDULES> sm_BaseSchedules;
};
