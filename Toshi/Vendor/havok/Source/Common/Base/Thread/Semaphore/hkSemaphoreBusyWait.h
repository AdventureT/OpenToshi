/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_HK_SEMAPHORE_BUSY_WAIT__H
#define HKBASE_HK_SEMAPHORE_BUSY_WAIT__H

#include <Common/Base/Config/hkConfigThread.h>

#if ( defined(HK_PLATFORM_PS3_PPU) || defined(HK_PLATFORM_PS3_SPU)) && (HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED)

	/// A wrapper class for a semaphore that can work on both SPU and PPU
class hkSemaphoreBusyWait
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE, hkSemaphoreBusyWait);


		/// Create a semaphore with an initial count and a maximum count. Max count currently ignored.
		hkSemaphoreBusyWait( int initialCount, int maxCount );

		hkSemaphoreBusyWait( hkFinishLoadedObjectFlag f) {}

			/// Destruct the Semaphore
		~hkSemaphoreBusyWait();

			/// This call will block until the semaphore is released.
		void acquire();
			
			/// Release the semaphore. Releases a thread blocked by acquire().
		void release(int count = 1);


			// These static functions work on both spu and ppu.
		static void HK_CALL acquire(hkSemaphoreBusyWait* semaphoreOnPpu);
		static void HK_CALL release(hkSemaphoreBusyWait* semaphoreOnPpu, int count = 1);

	public:

		static HK_ALIGN( hkUint32 m_cacheLine[128/4], 128 );

	protected:

		hkUint32 m_semphoreValue;			// for PPU and SPU
		HK_CPU_PTR(hkUint32*) m_semaphore;	// for SPU, points to shared int above
		hkUint32 m_ppuEventPort;				// for sleeping PPU threads
		hkUint32 m_eventQueue;
};

#else

#include <Common/Base/Thread/Semaphore/hkSemaphore.h>

// fallback for non Cell platforms, or non threaded Cell builds
typedef hkSemaphore hkSemaphoreBusyWait;

#endif

#endif // HKBASE_HK_SEMAPHORE_BUSY_WAIT__H

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
