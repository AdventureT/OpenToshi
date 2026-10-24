#include "pch.h"
#include "AAIScheduleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AAIScheduleManager ) == 0x14 );

// $deBlob: FUNCTION 007ea410
AAIScheduleManager::AAIScheduleManager()
{
	m_iTask     = 0;
	m_pList     = TNULL;
	m_eSchedule = -1;
	m_fTaskTime = 0.0f;
}

// $deBlob: FUNCTION 007ea450
void AAIScheduleManager::AdvanceTask()
{
	if ( m_pList == TNULL || m_iTask != m_pList->m_iNumTasks - 1 )
	{
		m_iTask++;
		m_fTaskTime = 0.0f;
	}
	else
	{
		TASSERT( !"Can't advance schedule, it's already finished" );
	}
}

// $deBlob: FUNCTION 007ea4a0
void AAIScheduleManager::SetScheduleList( ScheduleList* a_pList, TINT a_eSchedule )
{
	m_pList     = a_pList;
	m_eSchedule = a_eSchedule;
	m_iTask     = 0;
	m_fTaskTime = 0.0f;
}
