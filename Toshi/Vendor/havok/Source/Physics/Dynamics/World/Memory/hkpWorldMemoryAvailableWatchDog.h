/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_WORLD_MEMORY_AVAILABLE_WATCH_DOG_H
#define HK_DYNAMICS2_WORLD_MEMORY_AVAILABLE_WATCH_DOG_H

class hkpWorld;
class hkStepInfo;
#include <Common/Base/Monitor/hkMonitorStream.h>
#include <Common/Base/Memory/Memory/hkMemory.h>

/// You should use this class to remove objects from your game, if memory is running low.
class hkWorldMemoryAvailableWatchDog : public hkReferencedObject
{
	public:

		HK_DECLARE_REFLECTION();

			/// Constructor takes a memory limit threshold above which freeMemory() will be called.
        inline hkWorldMemoryAvailableWatchDog( hkInt32 minMemoryAvailable );

            /// A utility function to check how much memory is available. This calls freeMemory() if
			/// the memory usage exceeds the memory limit.
		HK_FORCE_INLINE void watchMemory( hkpWorld* world );

			/// This function is called when the memory limit is exceeded. You should derive
			/// from this class and implement this function. Typically, you should remove
			/// unimportant rigid bodies from the world in this function to reduce the memory
			/// consumption of the engine.
		virtual void freeMemory( hkpWorld* world ) = 0;

	public:

			// Once the used memory breaches this limit, the watchdog should start to delete hkWorldObjects
        hkInt32 m_minMemoryAvailable;

};

#include <Physics/Dynamics/World/Memory/hkpWorldMemoryAvailableWatchdog.inl>

#endif // HK_DYNAMICS2_WORLD_MEMORY_AVAILABLE_WATCH_DOG_H

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
