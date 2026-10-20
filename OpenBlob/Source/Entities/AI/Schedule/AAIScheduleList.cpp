#include "pch.h"
#include "AAIScheduleList.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AAIScheduleManager::ScheduleList ) == 0x10 );

// $deBlob: FUNCTION 007e5df0
void AAIScheduleManager::ScheduleList::Set( TINT a_eSchedule, const Task* a_pTasks, TINT a_iTaskCount, TINT a_ePriority, const TCHAR* a_szName )
{
	TASSERT( a_iTaskCount > 0 && a_iTaskCount < 100 );

	m_pTasks    = a_pTasks;
	m_szName    = a_szName;
	m_iNumTasks = a_iTaskCount;
	m_iPriority = a_ePriority;
}
