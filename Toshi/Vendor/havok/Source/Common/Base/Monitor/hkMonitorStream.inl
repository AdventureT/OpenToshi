/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#if defined( HK_PLATFORM_PS3_SPU)
#include <spu_intrinsics.h>
#endif

#if defined (HK_PLATFORM_PS3_SPU)
#define HK_PERF_BOOKMARK( VAL ) { uint32_t v = VAL; __asm__ volatile (" wrch $ch69, %0" :: "r"(v)); __asm__ volatile ("nop;nop;nop;nop;nop;nop;nop;nop"); __asm__ volatile ("nop;nop;nop;nop;nop;nop;nop;nop");}
#elif defined(HK_PLATFORM_PS3_PPU)
#include <cell/perf/performance.h>
#define HK_PERF_BOOKMARK( VAL ) cellPerfInsertCBEpmBookmark( VAL );
#else
#define HK_PERF_BOOKMARK( VAL )
#endif



void hkMonitorStream::TimerCommand::setTime()
{
		hkUint32 ticks;
		__asm {
			//push ebx
			//cpuid
			//pop ebx
			rdtsc
			mov dword ptr[ticks], eax
			//mov dword ptr[ticks+4], edx
		}
		m_time0 = ticks;
}


#define HK_MONITOR_TYPE_TIMER "t"
#define HK_MONITOR_TYPE_SINGLE "f"
#define HK_MONITOR_TYPE_INT "i"

#define HK_MONITOR_COMMAND_PUSH_DIR "Pd"
#define HK_MONITOR_COMMAND_POP_DIR "pd"
#define HK_MONITOR_COMMAND_BEGIN_TIMER "T"

#define HK_MONITOR_COMMAND_END_TIMER "E"
#define HK_MONITOR_COMMAND_BEGIN_LIST "L"
#define HK_MONITOR_COMMAND_SPLIT_LIST "S"
#define HK_MONITOR_COMMAND_END_LIST "l"

#define HK_MONITOR_COMMAND_NOP "N" // nops added when monitor stream transfers have to be of certain alignment (eg: on SPUs)

#if HK_CONFIG_MONITORS == HK_CONFIG_MONITORS_ENABLED

#	define HK_MONITOR_ADD_VALUE( NAME, VALUE, MONITOR_TYPE ){   \
		hkMonitorStream& mStream = hkMonitorStream::getInstance();			\
		if ( mStream.memoryAvailable() )							\
		{																	\
			 hkMonitorStream::AddValueCommand* h = reinterpret_cast<hkMonitorStream::AddValueCommand*>(mStream.getEnd());	\
			 h->m_commandAndMonitor = "M" MONITOR_TYPE NAME;				\
			 h->m_value = VALUE;											\
			 mStream.setEnd( (char*)(h+1) );						\
		}	}

#	define HK_TIMER_INTERN( NAME, COMMAND, OBJECT )	{						\
		hkMonitorStream& mStream = hkMonitorStream::getInstance();			\
		if ( mStream.memoryAvailable() )										\
		{																	\
			hkMonitorStream::TimerCommand* h = reinterpret_cast<hkMonitorStream::TimerCommand*>(mStream.getEnd());	\
			 h->m_commandAndMonitor = COMMAND HK_MONITOR_TYPE_TIMER NAME;	\
			 h->setTime();													\
			 mStream.setEnd( (char*)(h+1) );						\
		} }

#	define HK_TIMER_INTERN_LIST( NAME1, NAME2, COMMAND, OBJECT ) {			\
		hkMonitorStream& mStream = hkMonitorStream::getInstance();			\
		if ( mStream.memoryAvailable() )										\
		{																	\
			hkMonitorStream::TimerBeginListCommand* h = reinterpret_cast<hkMonitorStream::TimerBeginListCommand*>(mStream.getEnd());	\
			 h->m_commandAndMonitor = COMMAND HK_MONITOR_TYPE_TIMER NAME1;	\
			 h->m_nameOfFirstSplit = HK_MONITOR_COMMAND_SPLIT_LIST HK_MONITOR_TYPE_TIMER NAME2; \
			 h->setTime();													\
			 mStream.setEnd( (char*)(h+1) );						\
		} }

#	define HK_TIMER_BEGIN( NAME, OBJECT )	HK_TIMER_INTERN( NAME, HK_MONITOR_COMMAND_BEGIN_TIMER, OBJECT	)
#	define HK_TIMER_END(  )	HK_TIMER_INTERN( "", HK_MONITOR_COMMAND_END_TIMER, HK_NULL	)

	/// Timer end call which also checks for matching timer begin call
#	define HK_TIMER_NAMED_END( NAME )	HK_TIMER_INTERN( NAME, HK_MONITOR_COMMAND_END_TIMER, HK_NULL	)

#	define HK_TIMER_BEGIN_LIST( NAME, FIRSTITEM )	HK_TIMER_INTERN_LIST( NAME, FIRSTITEM, HK_MONITOR_COMMAND_BEGIN_LIST, HK_NULL	)
#	define HK_TIMER_SPLIT_LIST( NEXTITEM )			HK_TIMER_INTERN( NEXTITEM, HK_MONITOR_COMMAND_SPLIT_LIST, HK_NULL	)
#	define HK_TIMER_END_LIST( )						HK_TIMER_INTERN( "", HK_MONITOR_COMMAND_END_LIST, HK_NULL	)

#	define HK_POSSIBLY_UNUSED

class HK_POSSIBLY_UNUSED hkTimeFunctionHelper
{
	public:
		HK_FORCE_INLINE ~hkTimeFunctionHelper() { HK_TIMER_END(); }
};

#define HK_TIME_CODE_BLOCK( NAME, OBJECT ) HK_TIMER_BEGIN(NAME, OBJECT); hkTimeFunctionHelper HK_PREPROCESSOR_JOIN_TOKEN(timeblock, __LINE__)

#	define HK_MONITOR_PUSH_DIR( PATH )	{							\
	hkMonitorStream& mStream = hkMonitorStream::getInstance();		\
	if ( mStream.memoryAvailable() )									\
	{																\
		 hkMonitorStream::Command* h = (hkMonitorStream::Command*)( mStream.expandby( sizeof(hkMonitorStream::Command) ) );	\
		 h->m_commandAndMonitor = HK_MONITOR_COMMAND_PUSH_DIR PATH;	\
	} }

#	define HK_MONITOR_POP_DIR(  )	{								\
	hkMonitorStream& mStream = hkMonitorStream::getInstance();		\
	if ( mStream.memoryAvailable() )									\
	{																\
		 hkMonitorStream::Command* h = (hkMonitorStream::Command*)( mStream.expandby( sizeof(hkMonitorStream::Command) ) );	\
		 h->m_commandAndMonitor = HK_MONITOR_COMMAND_POP_DIR;		\
	}	}

#	define HK_MONITOR_NOP()	{										\
	hkMonitorStream& mStream = hkMonitorStream::getInstance();		\
	if ( mStream.getEnd() <= (mStream.getCapacity() - sizeof(hkMonitorStream::Command)) )									\
	{																\
		hkMonitorStream::Command* h = (hkMonitorStream::Command*)( mStream.expandby( sizeof(hkMonitorStream::Command) ) );	\
		h->m_commandAndMonitor = HK_MONITOR_COMMAND_NOP;	\
	}	}

#else // HK_CONFIG_MONITORS == HK_CONFIG_MONITORS_DISABLED

#	define HK_MONITOR_PUSH_DIR( PATH )
#	define HK_MONITOR_POP_DIR()
#	define HK_MONITOR_ADD_VALUE( NAME, VALUE, MONITOR_TYPE )
#	define HK_MONITOR_NOP()

#	define HK_TIMER_BEGIN( NAME, OBJECT )
#	define HK_TIMER_END()
#	define HK_TIMER_NAMED_END(NAME)
#	define HK_TIMER_BEGIN_LIST( NAME, OBJECT )
#	define HK_TIMER_SPLIT_LIST( NAME )
#	define HK_TIMER_END_LIST(  )

#define HK_TIME_CODE_BLOCK( NAME, OBJECT )

#endif


// Set this to 1 for extra detailed timers

#	define HK_INTERNAL_TIMER_BEGIN(NAME,OBJECT)		{}
#	define HK_INTERNAL_TIMER_END()				{}
#	define HK_INTERNAL_TIMER_BEGIN_LIST(a,b)	{}
#	define HK_INTERNAL_TIMER_SPLIT_LIST(a)		{}
#	define HK_INTERNAL_TIMER_END_LIST()		{}
#	define HK_INTERNAL_MONITOR_ADD_VALUE(NAME, VALUE, MONITOR_TYPE) {}

#	define HK_USER_TIMER_BEGIN(NAME,OBJECT)		HK_TIMER_BEGIN(NAME,HK_NULL)
#	define HK_USER_TIMER_END				HK_TIMER_END
#	define HK_USER_TIMER_BEGIN_LIST(a,b)	HK_TIMER_BEGIN_LIST(a,b)
#	define HK_USER_TIMER_SPLIT_LIST(a)		HK_TIMER_SPLIT_LIST(a)
#	define HK_USER_TIMER_END_LIST			HK_TIMER_END_LIST



/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090216)
* 
* Confidential Information of Havok.  (C) Copyright 1999-2009
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
* 
* Use of this software for evaluation purposes is subject to and indicates
* acceptance of the End User licence Agreement for this product. A copy of
* the license is included with this software and is also available at www.havok.com/tryhavok.
* 
*/
