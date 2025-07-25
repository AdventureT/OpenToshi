#include "ToshiPCH.h"
#include "TSystem.h"
#include "TScheduler.h"

#include "Toshi/Strings/TPString8.h"

TOSHI_NAMESPACE_START

TSystemManager::TSystemManager()
    : m_Emitter(this)
{
	m_Scheduler  = TNULL;
	m_Paused     = TFALSE;
	m_Second     = 0;
	m_AverageFps = 0;
	m_FrameCount = 0;
	m_Unk7       = TFALSE;
}

void TSystemManager::Update()
{
	// Calculating average fps
	m_Timer.Update();
	TFLOAT deltaTime = m_Timer.GetDelta();
	m_Second += deltaTime;

	if (m_Second > 1.0f)
	{
		m_Second     = 0.0;
		m_AverageFps = (1.0f / deltaTime) * 0.5f + m_AverageFps * 0.5f;
	}

	m_FrameCount += 1;

	// Update the scheduler
	m_Scheduler->Update();
}

TBOOL TSystemManager::Create()
{
	TSystemManager::CreateSingleton()->m_Scheduler = new TScheduler();
	return TTRUE;
}

TOSHI_NAMESPACE_END
