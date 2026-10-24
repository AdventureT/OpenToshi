#pragma once

//-----------------------------------------------------------------------------
// Purpose: the progress of an AAISchedule through the tasks of its schedule list (Projects\Entities\Ai\Schedule\
// AAIScheduleManager.cpp).
//-----------------------------------------------------------------------------
class AAIScheduleManager
{
public:
	// A schedule (AAIScheduleList entry): its tasks
	struct ScheduleList
	{
		struct Task
		{
			TINT   m_eTask;  // 0x00 AAITasks::AITASKS
			TFLOAT m_fParam; // 0x04 (a time for most tasks, -1: none)
		};

		// Ai\Schedule\AAIScheduleList.cpp
		void Set( TINT a_eSchedule, const Task* a_pTasks, TINT a_iTaskCount, TINT a_ePriority, const TCHAR* a_szName );

		const Task*  m_pTasks;    // 0x00
		TINT         m_iNumTasks; // 0x04
		TINT         m_iPriority; // 0x08 AAISchedules::PRIORITY
		const TCHAR* m_szName;    // 0x0C
	};

public:
	AAIScheduleManager();

	// $deBlob: FUNCTION 007ea510
	virtual void Update( TFLOAT a_fDeltaTime ) { m_fTaskTime += a_fDeltaTime; } // +0x00

	void AdvanceTask();
	void SetScheduleList( ScheduleList* a_pList, TINT a_eSchedule );

	// The current task (AAITasks::AITASKS), -1 without a schedule
	TINT GetCurrentTask() const { return m_pList ? m_pList->m_pTasks[ m_iTask ].m_eTask : -1; }
	// The current schedule (AAISchedules::AAISCHEDULE), -1 without a schedule
	TINT GetSchedule() const { return m_pList ? m_eSchedule : -1; }

	ScheduleList* GetScheduleList() const { return m_pList; }
	TINT          GetTaskIndex() const { return m_iTask; }
	TFLOAT        GetTaskTime() const { return m_fTaskTime; }

	void Reset()
	{
		m_iTask     = 0;
		m_fTaskTime = 0.0f;
	}

	void ResetTaskTime() { m_fTaskTime = 0.0f; }

private:
	TINT          m_iTask;     // 0x04 index in m_pList
	ScheduleList* m_pList;     // 0x08
	TINT          m_eSchedule; // 0x0C
	TFLOAT        m_fTaskTime; // 0x10
};
