/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */



#ifndef HK_DYNAMICS2_DEFAULT_WORLD_MEMORY_WATCH_DOG_H
#define HK_DYNAMICS2_DEFAULT_WORLD_MEMORY_WATCH_DOG_H

#include <Physics/Dynamics/World/Memory/hkpWorldMemoryAvailableWatchDog.h>

class hkpWorld;
class hkStepInfo;
class hkpSimulationIsland;

	/// The purpose of this class is to remove objects from the physics to
	/// free memory. To do this class simply iterates over all islands, finds the objects
	/// with the highest autoRemoveLevel and removes those objects until
    /// the memroy system has 'm_minAmountOfMemoryToFreeAtTime' available.
class hkpDefaultWorldMemoryWatchDog : public hkWorldMemoryAvailableWatchDog
{
	public:
			/// Constructor
			/// \param minMemoryToFree  The minimum amount of memory that has to be available.
			///							Entities will be released until the memory system claims to have that amount available.
		hkpDefaultWorldMemoryWatchDog( hkInt32 minMemoryAvailable = 20000);

			/// Free memory
		void freeMemory( hkpWorld* world );

	protected:
			/// Remove some entities from one island
		static void removeObjectsFromIsland( hkpSimulationIsland* island, int minAutoRemoveLevelToProcess, hkInt32 targetMemoryAvailable, int & maxFoundAutoRemoveLevelOut );

};


#endif // HK_DYNAMICS2_DEFAULT_WORLD_MEMORY_WATCH_DOG_H

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
