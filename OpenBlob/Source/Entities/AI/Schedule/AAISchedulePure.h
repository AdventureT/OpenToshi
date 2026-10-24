#pragma once
#include "Entities/AEntityComponent2.h"
#include "AAIScheduleManager.h"

//-----------------------------------------------------------------------------
// Purpose: the abstract interface of the AI schedules between AEntityComponent2 and AAISchedule.
// retail only creates it as AAISchedule's base
//-----------------------------------------------------------------------------
class AAISchedulePure
    : public AEntityComponent2
{
public:
	// $deBlob: FUNCTION 007e4070
	AAISchedulePure( ABaseEntity* a_pEntity )
	    : AEntityComponent2( a_pEntity )
	{}

	// $deBlob: FUNCTION 007e4330
	virtual ~AAISchedulePure() override {} // +0x00

	virtual AAIScheduleManager::ScheduleList* TranslateSchedule( TINT a_eSchedule ) = 0; // +0x14
};
