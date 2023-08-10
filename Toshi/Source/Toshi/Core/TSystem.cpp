#include "ToshiPCH.h"
#include "TSystem.h"
#include "TScheduler.h"

#include "Toshi/Strings/TPCString.h"

namespace Toshi
{
	TSystemManager::TSystemManager() : m_Emitter(this)
	{
		m_Scheduler = TNULL;
		m_Paused = TFALSE;
		m_Second = 0;
		m_AverageFps = 0;
		m_FrameCount = 0;
		m_Unk7 = TFALSE;
	}

	void TSystemManager::Update()
	{
		// Calculating average fps
		m_Timer.Update();
		float deltaTime = m_Timer.GetDelta();
		m_Second += deltaTime;

		if (m_Second > 1.0f)
		{
			m_Second = 0.0;
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

	TCStringPool* TSystem::GetCStringPool()
	{
		if (ms_poTCStringPool == TNULL)
		{
			ms_poTCStringPool = new TCStringPool(0x400, 0);
		}
		return ms_poTCStringPool;
	}

	const wchar_t* TSystem::StringCharToUnicode(wchar_t* a_UnicodeString, const char* a_CharString, int a_iLength)
	{
		TASSERT((a_UnicodeString != TNULL) && (a_CharString != TNULL));
		StringLength(a_CharString);
		TIMPLEMENT();
		return nullptr;
	}

	int TSystem::StringLength(const char* a_String)
	{
		TASSERT(a_String != TNULL);
		int iLength;
		for (iLength = 0; *a_String++ != '\0'; iLength++);
		return iLength;
	}

	TCStringPool* TSystem::CreateCStringPoolExplicit(int unk, int unk2)
	{
		if (ms_poTCStringPool == TNULL)
		{
			ms_poTCStringPool = new TCStringPool(unk, unk2);
		}
		return ms_poTCStringPool;
	}

	TCStringPool* TSystem::CreateCStringPoolExplicit(const char* a_szFileName, int unk, int unk2)
	{
		if (ms_poTCStringPool == TNULL)
		{
			ms_poTCStringPool = new TCStringPool(a_szFileName, unk, unk2);
		}
		return ms_poTCStringPool;
	}
}