/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SPU_MONITOR_CACHE__H
#define HK_SPU_MONITOR_CACHE__H

#include <Common/Base/Thread/CriticalSection/hkCriticalSection.h>
#include <Common/Base/Monitor/hkMonitorStream.h>
#include <Common/Base/Thread/JobQueue/hkJobQueue.h>


/// This monitor cache represents a cache in main memory that a single spu can
/// use to write timers into.  The PPU and SPU calls are not locked, it is up to the
/// user of this class to make sure that SPU and PPU do not call this class concurrently.
class hkSpuMonitorCache
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE_CLASS, hkSpuMonitorCache);


		//
		// PPU functions
		//

			/// Initialize the cache with a buffer size. PPU only.
		hkSpuMonitorCache( int monitorBufSize );

			/// Deallocates the buffer.
		~hkSpuMonitorCache();

			/// Fix up the timer stream in place. Strings in the stream are replaced with
			/// strings taken from the spurs elfs loaded in main memory.
		void fixUpStringsPpu( void** spursElfsInMainMemory );

			/// Clear the ppu buffer
		void clearTimersPpu();

		const char* getTimerDataStartPpu() { return m_start; }
		const char* getTimerDataEndPpu() { return m_current; }


	protected:

		enum
		{
			MIN_SIZE_TO_DMA = 512
		};

		hkPadSpu<char*> m_current; // so it can be dma'd by itself. Assumed to be at start of struct
		hkPadSpu<char*> m_start; 	// points to the start of the cache. Setup only.
		hkPadSpu<char*> m_end; 	// points to the end of the whole cache. Do not exceed.
		hkBool m_stringsFixedUp;

};

#include <Common/Base/Monitor/Spu/hkSpuMonitorCache.inl>

#endif // HK_SPU_MONITOR_CACHE__H

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
