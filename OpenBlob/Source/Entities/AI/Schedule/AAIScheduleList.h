#pragma once
#include "AAIScheduleManager.h"

//-----------------------------------------------------------------------------
// Purpose: a block of CAPACITY schedules, numbered from m_iScheduleOffset (Projects\Entities\AI\Schedule\
// AAIScheduleList.h): AAIBaseSchedules::sm_BaseSchedules (the base schedules), AEnemySchedule's.
// each instantiation is its own retail code (the markers are stacked)
//-----------------------------------------------------------------------------
template <TINT CAPACITY>
class AAIScheduleList
{
public:
	using ScheduleList = AAIScheduleManager::ScheduleList;

	// AAISchedules::PRIORITY
	static TCONSTEXPR_VAR TINT PRIORITY_INVALID = -1;
	static TCONSTEXPR_VAR TINT PRIORITY_COUNT   = 7;

public:
	// $deBlob: FUNCTION 007ea7b0
	// $deBlob: FUNCTION 007d2890
	void AddSchedule( TINT a_eSched, const ScheduleList::Task* a_pTasks, TINT a_iTaskCount, TINT a_ePriority, const TCHAR* a_szName )
	{
		TASSERT( m_iScheduleOffset != -1 );
		TASSERT( a_eSched >= m_iScheduleOffset );
		TASSERT( PRIORITY_INVALID < a_ePriority && a_ePriority < PRIORITY_COUNT );

		TINT iIndex = a_eSched - m_iScheduleOffset;
		TASSERT( iIndex >= 0 && iIndex < CAPACITY );
		m_aSchedules[ iIndex ].Set( a_eSched, a_pTasks, a_iTaskCount, a_ePriority, a_szName );
	}

	// $deBlob: FUNCTION 007e4130
	// $deBlob: FUNCTION 007d2750
	ScheduleList* GetSchedule( TINT a_iIndex )
	{
		TASSERT( m_iScheduleOffset != -1 );
		TASSERT( a_iIndex >= m_iScheduleOffset );

		TINT iIndex = a_iIndex - m_iScheduleOffset;
		TASSERT( iIndex >= 0 && iIndex < CAPACITY );
		return &m_aSchedules[ iIndex ];
	}

	TBOOL IsScheduleInRange( TINT a_eSchedule ) const { return a_eSchedule >= m_iScheduleOffset && a_eSchedule < m_iScheduleOffset + CAPACITY; }

	void SetScheduleOffset( TINT a_iOffset ) { m_iScheduleOffset = a_iOffset; }

private:
	TINT         m_iScheduleOffset = -1;   // 0x00
	ScheduleList m_aSchedules[ CAPACITY ]; // 0x04
};
