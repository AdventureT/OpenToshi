/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_MEMORY_TRACKER__H
#define HAVOK_FILTER_MEMORY_TRACKER__H

#include <Common/Base/Memory/Memory/Pool/hkPoolMemory.h>

	/// This memory implementation tracks all allocations and 
	/// will automatically deallocate them - it is used during filter processing
	/// to make memory management easier. When a modal filter is executed (through its hctFilterInterface::process() method),
	/// this memory manager is usually in operation (has previously been set by hctFilterDll::pushMemoryManager() ), so 
	/// calls to new and delete will be processed by this memory manager.
class hctFilterMemoryTracker : public hkPoolMemory
{
public:

		/// Constructor
	hctFilterMemoryTracker();

		/// Destructor - deallocates all memory previously allocated
	~hctFilterMemoryTracker();

		/// Deallocates memory previously allocated with this memory manager
	inline virtual void deallocate(void* p);

		/// Allocates memory with the given alignment
	inline virtual void* alignedAllocate(int alignment, int nbytes, HK_MEMORY_CLASS cl);

		/// Deallocates memory previously allocated with alignedAllocate()
	inline virtual void alignedDeallocate(void* p);

		/// Allocates an individual chunk of memory
	inline virtual void* allocateChunk(int nbytes, HK_MEMORY_CLASS cl);

		/// Deallocates a chunk of memory previously allocated with allocateChunk()
	inline virtual void deallocateChunk(void* p, int nbytes, HK_MEMORY_CLASS cl);

		/// Allocates n bytes of memory
	inline virtual void* allocate(int nbytes, HK_MEMORY_CLASS cl);
	
		/// Deallocates all memory allocated by this memory manager
	void deallocateAll();

protected:

	// So that we can track hkAllocateChunk calls too
    struct MemoryChunk
	{
		HK_MEMORY_CLASS m_memClass;
		unsigned int m_memSize;
		unsigned char* m_mem;	
		MemoryChunk* m_next;
	};

	struct MemoryAlloc
	{
		unsigned char* m_mem;	
		MemoryAlloc* m_next;
	};

	MemoryChunk* m_chunkAllocs;
	MemoryAlloc* m_allocs;
	MemoryAlloc* m_alignedAllocs;
};

#include <ContentTools/Common/Filters/Common/Memory/hctFilterMemoryTracker.inl>

#endif // HAVOK_FILTER_MANAGER_INTERFACE__H

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
