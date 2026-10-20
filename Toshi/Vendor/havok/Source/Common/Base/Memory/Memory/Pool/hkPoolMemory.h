/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_POOL_MEMORY
#define HK_POOL_MEMORY

#include <Common/Base/Memory/Memory/hkMemory.h>
#include <Common/Base/Thread/CriticalSection/hkCriticalSection.h>
#include <Common/Base/Config/hkConfigMemoryStats.h>

//This table is created if the statistics are enabled
//it is not accessed otherwise
#include <Common/Base/Memory/MemoryClasses/hkMemoryClassesTable.h>
extern hkMemoryClassInfo hkMemoryClassesTable[];



/// The default Havok memory manager.
/// This manager requests fixed size blocks from the system allocator
/// (hkSystemMalloc) and manages these blocks. Memory is not returned
/// until the destructor. Blocks larger than this block size (currently
/// 8192 bytes) are forwarded directly to the system allocator.
/// The pool can be preloaded based on previous runs. See the user guide.
/// This memory manager fills free and uninitialized memory when HK_DEBUG
/// is defined. See hkMemory::MemoryFill for the values.
class hkPoolMemory : public hkMemory
{

public:
    HK_DECLARE_SYSTEM_ALLOCATOR();

	hkPoolMemory( int maxNumElemsOnThreadMemoryFreeList = hkMemory::DEFAULT_MAX_NUM_ELEMS_ON_THREAD_MEMORY_FREE_LIST );

	~hkPoolMemory();

	virtual void allocateChunkBatch(void** blocksOut, int nblocks, int nbytes, HK_MEMORY_CLASS cl );

	virtual void deallocateChunkBatch(void** blocks, int nblocks, int nbytes, HK_MEMORY_CLASS cl );

	virtual void* allocateChunk(int nbytes, HK_MEMORY_CLASS cl);

	virtual void deallocateChunk(void*, int nbytes, HK_MEMORY_CLASS cl);

	virtual void printStatistics(hkOstream* c);

	virtual int getAllocatedSize( int nbytes );

	/// Preallocate and reserves a number of memory pages.
	/// Reserving memory in this way should avoid memory fragmentation.
	virtual void reservePagesFromSystemMemory( int numBytes );

	/// This call tries to find completely empty pages in the Havok memory
	/// and adds them to the reserved list.
    virtual void garbageCollect( void );

	/// This system frees the memory reserved on the reserved list back to the system.
	virtual void releaseUnusedPagesToSystemMemory( void );

	virtual void preAllocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl);

	virtual void* allocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl);

	virtual void deallocateRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl);

	virtual void provideRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl);

	virtual void freeRuntimeBlocks();

    virtual void calculateStatistics(hkMemoryStatistics& stats);

    virtual void setLimitedMemoryListener(hkLimitedMemoryListener* listener);

    virtual hkLimitedMemoryListener* getLimitedMemoryListener();

    virtual void setMemorySoftLimit(hk_size_t maxMemory);

    virtual hk_size_t getMemorySoftLimit();

    virtual void setMemoryHardLimit(hk_size_t maxMemory);
    virtual hk_size_t getMemoryHardLimit();

    virtual hkBool hasMemoryAvailable(hk_size_t size);

	virtual void optimize();

	inline int getMemoryUsed() const
	{
		return m_pageMemoryUsed + m_sysAllocsSize;
	}

protected:

    void _setMemorySoftLimit(hk_size_t maxMemory);
    void _setMemoryHardLimit(hk_size_t maxMemory);

	void putOnFreeList(void* p, int row);

	//update the stats when an allocate occurs (will do nothing if stats not enabled)
	void updateStatsAllocate(int nbytes, HK_MEMORY_CLASS cl);

	//update the stats when a deallocate occurs (will do nothing if stats not enabled)
	void updateStatsDeallocate(int nbytes, HK_MEMORY_CLASS cl);

	//fetch statistics info for a class (will do nothing if stats not enabled)
	hkMemoryClassInfo* getInfo(int classID);

	//when collating statistics, the stats should be propagated from subclasses to superclasses
	void propogateStatsUp();

	// When finished collating, we must undo these additions
	void undoPropogate();

	// Print out the memory class hierarchy
	void printMemoryClassTree(hkOstream* c,int id, int level);

	// return the row of the free list for a given size
	HK_FORCE_INLINE int getRow(int nbytes)
	{
		return (nbytes <= hkThreadMemory::MEMORY_MAX_SIZE_SMALL_BLOCK)
			? int(m_small_size_to_row_lut[nbytes])
			: m_large_size_to_row_lut[ (nbytes-1) >> hkThreadMemory::MEMORY_LARGE_BLOCK_RSHIFT_BITS];
	}

public:

	struct MemoryElem
	{
		// NEVER NEW THIS
		MemoryElem *m_next;
	};

	/// A header for each system allocation
	struct hkMemoryPage
	{
		// NEVER NEW THIS
		hkMemoryPage* m_next;
		hkInt32 m_pad[(hkThreadMemory::PAGE_ALIGN - hkSizeOf(hkMemoryPage*))/ hkSizeOf(hkInt32)];
	};

	enum { CHOP_SIZE = 512 };

private:

	HK_FORCE_INLINE hkMemoryPage* getNewPage ( void );
	HK_FORCE_INLINE void* _allocate_and_chop_page( int row );
	HK_FORCE_INLINE void _putOnFreeList(void* p, int row);
	HK_FORCE_INLINE void _updateStatsAllocate(int nbytes, HK_MEMORY_CLASS cl);
	HK_FORCE_INLINE void _updateStatsDeallocate(int nbytes, HK_MEMORY_CLASS cl);
	HK_FORCE_INLINE hkMemoryClassInfo* _getInfo(int classID);

protected:
    inline int getSizeAllocatedByMemoryManager() const
    {
        return (m_sizeOfPage + m_pageOverhead) * m_numPages + m_sysAllocsSize;
    }
        /// Returns the size which is allocated within the memory manager.
        /// Note: If you allocate a block of size 3, it will be padded to 4 bytes
        /// and 4 will be added to m_pageMemoryUsed
    

        /// Works out the size in bytes of all the memory in use for page memory
    hk_size_t _calculatePageSizeInUse();

	/// List of all pages allocated
	hkMemoryPage* m_allocated_memory_pages;

	/// List of all 8K pages which are allocated, which
	/// are not in use by the pool memory.
	hkMemoryPage* m_reserved_memory_pages;

	// Pointers to the start,end and current position in the current page
	// we make them char* instead of void* because ansi c++ does not permit
	// pointer arithmetic on void pointers
	char* m_current_page_start;
	char* m_current_page_end;
	char* m_current_page_space;

	hkCriticalSection m_criticalSection;

	/// Free list for blocks of each size
	MemoryElem* m_free_list[hkThreadMemory::MEMORY_MAX_ALL_ROW];

	/// A lookup table of size of each block size
	int m_row_to_size_lut[hkThreadMemory::MEMORY_MAX_ALL_ROW];

	/// A lookup table of sizes to small block size
	char m_small_size_to_row_lut[hkThreadMemory::MEMORY_MAX_SIZE_SMALL_BLOCK+1];

	/// A lookup table of sizes to large block size
	int m_large_size_to_row_lut[ (hkThreadMemory::MEMORY_MAX_SIZE_LARGE_BLOCK >> hkThreadMemory::MEMORY_LARGE_BLOCK_RSHIFT_BITS) ];

	/// Statistics for blocks of each size
	int m_blocks_in_use[hkThreadMemory::MEMORY_MAX_ALL_ROW];

        /// The total number of memory allocations bypassing the pool memory and going
        /// directly to the system memory. If you use the hkPoolMemory, then very big
        /// allocations (greater then 8K) can't be handled by the pool and have to go
        /// to the system memory
    int m_numSysAllocs;
        /// The total number of bytes allocated by the system memory
    int m_sysAllocsSize;
        /// The current maximum of the total number of bytes allocated by the system memory
    int m_sysAllocsHighMark;
        /// The number of pages allocated by the pool memory
    int m_numPages;
        /// Size of one page without overhead
    int m_sizeOfPage;
        /// Size of the overhead
    int m_pageOverhead;
        /// Bytes allocated within the pool memory
    int m_pageMemoryUsed;

        /// Limited memory listener
    hkLimitedMemoryListener* m_listener;

        /// The limit of memory available. If 0 there is no imposed limit
    hk_size_t m_hardLimit;

    hk_size_t m_softLimit;
};


#endif // HK_POOL_MEMORY

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
