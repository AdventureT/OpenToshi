/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_HKMEMORY_H
#define HKBASE_HKMEMORY_H

#include <Common/Base/Memory/MemoryClasses/hkMemoryClassDefinitions.h>

class hkOstream;
class hkThreadMemory;

#include <Common/Base/Config/hkConfigMemory.h>

	/// A struct holding memory allocation information
struct hkMemoryStatistics
{
        /// Infinite as far as the memory system is concerned.
    static const hk_size_t INFINITE_SIZE = (hk_size_t(0)-1);

        /// Total amount of allocated memory. Allocated memory means that it is 'owned' by the hkMemory reporting it -
        /// but the actual memory inside may be available or used. Thus m_used+m_available = m_allocated.
    hk_size_t m_allocated;
        /// Total used memory
    hk_size_t m_used;
        /// Total available from the _allocated_ memory. This is NOT the total amount of memory that is potentially available,
        /// only the amount that remains available from what has been allocated.
    hk_size_t m_available;
        /// The approx total amount of memory that is available. The value is the amount of available memory (m_available)
        /// plus the approx amount of memory that is potentially available but not yet allocated.
        /// Depending on the allocator being used, this value may be meaningless. For example for an allocator with virtual memory
        /// the 'total amount of available' is somewhat meaningless. In those scenarios, m_totalMemory will be INFINITE_SIZE
    hk_size_t m_totalAvailable;
        /// Largest block available. On some memory allocators this is meaningless (for example with virtual memory). In that
        /// case this will be set to INFINITE_SIZE
    hk_size_t m_largestBlock;

	HK_DECLARE_SYSTEM_ALLOCATOR();
};

    /// This listener can be used to inform you application about low memory situations within the memory manager.
    /// Ideally on getting the cannotAllocate call, you can free up resources such that an allocation can be made.
    /// If an allocation still isn't possible a call to allocationFailure will be made. The memory the hkMemory
    /// should ideally then return HK_NULL to indicate a successful allocation did not complete.
    /// You do not need to call the garbage collector in this call, as the garbage collector will be run
    /// after the call to cannotAllocate is made.
    /// If in order to free some memory you need some memory you can call garbageCollect, but garbage collection
    /// whilst not slow, isn't that nippy either, so trying to minimise your collects is probably wise.

class hkLimitedMemoryListener
{
public:
	HK_DECLARE_SYSTEM_ALLOCATOR();

	virtual ~hkLimitedMemoryListener(){}
        /// Called when a request is denied. An implementation can free up memory, and on
        /// returning from this call the allocator will try and allocate again.
    virtual void cannotAllocate(hkMemory* mem,hk_size_t size) =0;
        /// This is called when the allocator has given up allocating. Generally this will be called after
        /// a call to cannotAllocate, although that is not a requirement of an implementaion.
    virtual void allocationFailure(hkMemory* mem,hk_size_t size) = 0;
};

/// Used to traverse over all of the memory.
/// start is the start of a block
/// size is the size of the block in bytes (note the size may be larger than requested in an allocation)
/// If allocated is true then the block is allocated, else its free
/// pool identifies which 'pool' a block of memory belongs to. What that means is implementation specific
/// param is the parameter passed into the memory walking method

typedef void (HK_CALL *hkMemoryWalkCallback)(void* start, hk_size_t size,hkBool allocated,int pool,void* param);

	/// All memory allocations in Havok are handled through an instance of this class.
	/// Note that all allocations greater than 8 bytes should be 16 byte aligned.
	/// Allocations of 8 bytes or less should be aligned to 8 bytes.
	/// We distinguish between allocations where the size is explicitly
	/// stored or not stored. See the hkBase user guide for more details.

class hkMemory
{
	public:

		HK_DECLARE_SYSTEM_ALLOCATOR();

		friend class hkThreadMemory;

		enum 
		{
			DEFAULT_MAX_NUM_ELEMS_ON_THREAD_MEMORY_FREE_LIST = 16,
		};

			/// Initialize the reference count to 1.
			/// \param maxNumElemsOnThreadMemoryFreeList: specifies how many objects can be stored on hkThreadMemory's free list
			///												before those blocks are given back to the main memory manager.
			///												If you set maxNumElemsOnThreadMemoryFreeList to 0, you effectively disable hkThreadMemory's
			///												memory manager.
		hkMemory( int maxNumElemsOnThreadMemoryFreeList = DEFAULT_MAX_NUM_ELEMS_ON_THREAD_MEMORY_FREE_LIST );

			/// Cleanup.
		virtual ~hkMemory();

			/// Allocate nblocks of nbytes. This is equivalent to nblocks calls to
			/// allocateChunk but is more lock friendly.
		virtual void allocateChunkBatch(void** blocksOut, int nblocks, int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// Deallocate nblocks of nbytes. This is equivalent to nblocks calls to
			/// deallocateChunk but is more lock friendly.
		virtual void deallocateChunkBatch(void** blocksOut, int nblocks, int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// Allocates a fixed size piece of memory, aligned to 16 bytes.
			/// Allocations of 8 bytes or less are aligned to 8 bytes.
			/// Note that the size is not stored and must be supplied when deallocating.
		virtual void* allocateChunk( int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// Deallocates a piece of memory that has been allocated by allocateChunk
		virtual void deallocateChunk( void*, int nbytes, HK_MEMORY_CLASS cl ) = 0;

			/// Allocates a block of size nbytes of fast runtime memory
			/// If a matching block is available this function will return it.
			/// If no matching block could be found this function will allocate a new block and issue a warning,
			/// add it to its managed array of blocks and return a pointer to it.
			///
			/// General info on fast runtime blocks:
			/// Regular stack memory is not working properly in a multi-threaded environment. To cater to the need for
			/// fast memory allocations during simulation, the concept of dedicated runtime memory blocks (working
			/// as a stack memory replacement) has been devised.
			/// Runtime block functions provide access to very fast memory allocation by internally
			/// storing an array of preallocated memory blocks that are re-used on the fly as soon
			/// as they have been declared available again by explicitly deallocating them. New
			/// allocations are only performed if no available memory block of matching size could
			/// be found in the internal array of managed block.
			/// Internally these runtime blocks are only used inside the simulation (i.e. during a call to
			/// stepDeltaTime() or stepProcessMt()) and can therefore be used freely outside by the user.
		virtual void* allocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// Deallocates a block of fast runtime memory that has been allocated by allocateRuntimeBlock()
			/// For more information on runtime blocks see allocateRuntimeBlock()
		virtual void deallocateRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// Preallocates a fast runtime memory block of size nbytes
			/// For more information on runtime blocks see allocateRuntimeBlock()
		virtual void preAllocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// This allows the user to provide his own block(s) of memory for fast runtime memory
			/// Note that memory blocks that have been provided by the user will not be deallocated by a call to
			/// freeRuntimeBlocks(). The user has to take care of deallocation himself.
			/// For more information on runtime blocks see allocateRuntimeBlock()
		virtual void provideRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl) = 0;

			/// Deallocate all fast runtime memory blocks that are still allocated and that have NOT been provided
			/// by the user (by using provideRuntimeBlock())
			/// For more information on runtime blocks see allocateRuntimeBlock()
		virtual void freeRuntimeBlocks() = 0;

			/// See hkPoolMemory::reservePagesFromSystemMemory()
		virtual void reservePagesFromSystemMemory( int numBytes ) {}

            /// Depending on the implementation, will try and free up memory resources to make more memory available.
            /// Concretely in many implementations, lists of small sized blocks are kept, so they can be made available
            /// rapidly. These small blocks although not used, may stop larger blocks being allocated, the collector will
            /// free up such pages so they can be used as part of other allocations.
        virtual void garbageCollect() {}

            /// Optimizing allows the memory manager to look the memory usage optimize its usage - for performance
            /// primarily. The process is similar to a garbage collect in terms of finding memory thats free,
            /// an optimize can decide to make this memory available for a commonly used block size for example.
            /// You shouldn't call this method every frame, but calling between levels may help overall memory
            /// usage. In terms of cost, doing an optimize requires a similar amount of work to a garbage collection.
        virtual void optimize() {}

			/// See hkPoolMemory::releaseUnusedPagesToSystemMemory()
		virtual void releaseUnusedPagesToSystemMemory( void ) {}

			/// Prints some statistics to the specified console.
		virtual void printStatistics(hkOstream* c) = 0;

			/// Find the memory rounded up by the memory allocator for a requested memory allocation
		virtual int getAllocatedSize( int size );

			/// Returns true if this is a debug memory manager, false otherwise
		virtual bool isDebugMemory(){ return false; }

            /// Work out memory statistics. This function tries to work out fairly accurately details about memory usage -
            /// this functions performance may not be great depending on the underlying implementation.
            /// See hkMemoryStatisticcs for details of the information returned.
            /// NOTE! That if you have a memory limit set (via setMemoryLimit) the values returned will reflect that limit
            /// you must have the limit set to zero if you want to find out the 'true' memory statistics.
        virtual void calculateStatistics(hkMemoryStatistics& stats ) =0;

            /// There are two styles of limit in the memory allocator, a soft limit and a hard limit.
            /// If the soft limit is exceeded it will _NOT_ stop an allocation being performed if it can be performed.
            /// The way to test to see if the soft limit has been exceeded is to call hasMemoryAvailable, which will
            /// return false if the soft limit has been passed.
            /// The other type of limit is a 'hard' limit. The hard limit cannot be passed - if you try and allocate
            /// past the hard limit allocation will fail and HK_NULL will be returned. Passing the hard limit is also
            /// what will produce calls to the hkLimitedMemoryListener. There is no listener mechanism around the
            /// soft limit, for performance reasons.
            /// Allow the setting of a memory usage limit (in bytes). Depending on the implementation the limit
            /// may or may not be honored. Look at the docs on specific implementations to see if memory limiting
            /// is supported. If limiting is not supported the method will have no effect. If the memory requested was
            /// unfeasibly low (for example more than that amount of memory has already been allocated) the
            /// actual setting does not change. Check getMemoryLimit to see if the change has been accepted.
            /// Setting the limit to 0, will set the limit to as large as possible
        virtual void setMemoryHardLimit(hk_size_t maxMemory) { }
            /// Returns what the limit is set to (in bytes). If there is no limit the method returns 0.
        virtual hk_size_t getMemoryHardLimit() { return 0; }
            /// set a soft memory limit. Setting 0 will mean there is no soft limit.
            /// See setMemoryHardLimit for a description of the difference between a soft and hard limit
        virtual void setMemorySoftLimit(hk_size_t maxMemory) { }

            /// Returns what the soft memory limit is. 0 means there is no soft memory limit.
        virtual hk_size_t getMemorySoftLimit() { return 0; }

            /// Set the limited memory listener. May not be implemented on all implementations - you can check
            /// by doing a get, if it is HK_NULL and you tried to set it to something, support for the listener isn't implemented
            /// on this platform.
        virtual void setLimitedMemoryListener(hkLimitedMemoryListener* listener) {}
            /// Get the limited memory listener. Not all memory systems support listeners.
        virtual hkLimitedMemoryListener* getLimitedMemoryListener() { return HK_NULL; }

            /// Returns true if an amount of memory of at least 'size' is available. Why not just have a method that returns
            /// the amount of memory available? Because with some allocators its meaningless - for when you have virtual
            /// memory, and because the implementation can decide broadly what 'enough memory' means - as it has internal
            /// knowledge about how memory works. This method is designed to be fast so can be called rapidly if needs be,
            /// but recommended use would be to call infrequently and check that larger allocations are available.
            /// Passing in size as zero well return true if some 'nominal' amount of memory required for normal function
            /// is available.
        virtual hkBool hasMemoryAvailable(hk_size_t size) { return true; }

			/// Some implementations (the DebugMemory for instance) can do checks on the current
			/// allocated pool. If the implementation does not support the check it will just return true.
		virtual hkBool isOk() const;

			/// Using the hkDebugMemory implementation you can use this function to search for the originally allocated memory block
			/// within which the supplied memory block is located. This function returns the originally allocated memory block's base
			/// address. It will assert if it cannot find a matching block.
		virtual void* findBaseAddress(void* p, int nbytes) { return HK_NULL; }

			/// Using the hkDebugMemory you can explicitly lock allocated memory. Upon freeing a memory block this lock counter will be
			/// checked and the debug memory will assert if there is still a lock left.
		virtual void lockBlock(void* p) {}

			/// Unlock memory in hkDebugMemory.
		virtual void unlockBlock(void* p) {}

            /// Traverses all of the memory used by the subsystem (allocated and free). The order that allocations
            /// are returned is undefined. Not all memory allocators can walk memory - if an implementation cannot
            /// it will return HK_FAILURE from this method.
        virtual hkResult walkMemory(hkMemoryWalkCallback callback,void* param) { return HK_FAILURE; }

			/// Status of the memory manager.
		enum MemoryState
		{
			MEMORY_STATE_OK,
			MEMORY_STATE_OUT_OF_MEMORY
		};

		MemoryState m_memoryState;
		int m_criticalMemoryLimit;
        int m_memoryUsed;

		static inline hkMemory& HK_CALL getInstance();

		static void HK_CALL replaceInstance(hkMemory* m);

		HK_FORCE_INLINE void addReference();

		HK_FORCE_INLINE void removeReference();

		HK_FORCE_INLINE int getMaxNumElemsOnThreadMemoryFreeList() const;

			/// Constant values used in debug modes.
		enum MemoryFill
		{
				/// For guard bytes before and after allocations.
			MEMORY_FILL_GUARD_AREA = 0xFD,
				/// Free memory owned by the memory manager.
			MEMORY_FILL_FREE_SPACE = 0xDD,
				/// Newly allocated memory to be returned.
			MEMORY_FILL_RETURNED_TO_USER = 0xCD
		};

	protected:

		static hkMemory* s_instance;
		int m_referenceCount;

		// This value is used by all hkThreadMemory instances
		const int m_maxNumElemsOnThreadMemoryFreeList ;

		struct BigBlockData
		{
			hkBool m_available;
			int m_size;
			void* m_memory;
			HK_MEMORY_CLASS m_class;
			hkBool m_providedByOutside;
		};

		enum { BIG_BLOCKS_SLOTS=128 };
		int m_numValidBigBlocks;
		BigBlockData m_bigBlockDataArray[BIG_BLOCKS_SLOTS];

	public:

			/// Value to fill guard bytes before and after allocations. By default, hkMemory::MEMORY_FILL_GUARD_AREA.
		static hkInt32 s_fillGuardArea;
			/// Value to fill free memory owned by the memory manager. By default, hkMemory::MEMORY_FILL_FREE_SPACE.
		static hkInt32 s_fillFreeSpace;
			/// Value to fill allocated memory to be returned. By default, hkMemory::MEMORY_FILL_RETURNED_TO_USER.
		static hkInt32 s_fillReturnedToUser;
};

hkMemory& HK_CALL hkMemory::getInstance()
{
	return *s_instance;
}

void hkMemory::removeReference()
{
	--m_referenceCount;
	if(m_referenceCount==0)
	{
		delete this;
	}
}

void hkMemory::addReference()
{
	++m_referenceCount;
}

int hkMemory::getMaxNumElemsOnThreadMemoryFreeList() const
{
	return m_maxNumElemsOnThreadMemoryFreeList;
}

#endif // HKBASE_HKMEMORY_H

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
