#pragma once
#include "TEvent.h"
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Core/THPTimer.h"
#include "Toshi/Core/TLString.h"
#include "Toshi/Strings/TPWString.h"

#include <utility>

namespace Toshi
{
	class TScheduler;
	
	class TCStringPool;

	class TSystem
	{
	public:
		static unsigned char* GetScratchMem() { return ms_aScratchMem; }
		static TCStringPool* GetCStringPool();
		static TWStringPool* GetWStringPool();
		static TLocale* GetLocale();

		static const wchar_t* StringCharToUnicode(wchar_t* a_UnicodeString, const char* a_CharString, int a_iLength);
		static int StringLength(const char* a_String);

		static TCStringPool* CreateCStringPoolExplicit(int unk, int unk2);
		static TCStringPool* CreateCStringPoolExplicit(const char* a_szFileName, int unk, int unk2);
	private:
		inline static unsigned char ms_aScratchMem[0x400];
		static inline TCStringPool* ms_poTCStringPool = TNULL;
		static inline TWStringPool* ms_poTWStringPool = TNULL;
		static inline TLocale* ms_poLocale = TNULL;
	};


	class TSystemManager : public TSingleton<TSystemManager>
	{
	public:
		TSystemManager();
	
		void Update();

		TBOOL Pause(TBOOL pause)
		{
			TBOOL oldState = m_Paused;

			m_Paused = pause;
			m_Emitter.Throw(&pause);

			return oldState;
		}

		THPTimer* GetTimer()
		{
			return &m_Timer;
		}

		TScheduler* GetScheduler() const
		{
			return m_Scheduler;
		}

		float GetAverageFps() const
		{
			return m_AverageFps;
		}

	public:
		static TBOOL Create();

		

		

	private:
		TEmitter<TSystemManager, TBOOL> m_Emitter; // 0x00
		TScheduler* m_Scheduler;
		TBOOL m_Paused;
		uint32_t m_Unk2;
		THPTimer m_Timer;
		uint32_t m_Unk3;
		float m_Second;
		float m_AverageFps;
		uint32_t m_FrameCount;
		TBOOL m_Unk7;
	};
}


