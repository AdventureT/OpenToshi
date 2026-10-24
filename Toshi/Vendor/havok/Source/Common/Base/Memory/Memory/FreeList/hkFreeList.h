/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_FREELIST
#define HK_FREELIST

#include <Common/Base/Memory/Memory/hkMemory.h>

#include <Common/Base/Memory/Memory/FreeList/hkFreeListMemoryServer.h>

/// Fast fixed memory size allocations - used in the hkFreeListMemory for small memory block allocations.
///
/// Can be used where ever you want fast same block size allocations, and you want to be able to free blocks
/// in any order when they are not in use.
///
/// This implementation is particularly fast at initial allocation - as it works just like a 'region'.
/// If memory is allocated and freed, the freelist uses a linked list of available blocks which whilst fast isn't
/// as fast as region style allocation. Doing a a findGarbage will turn all memory that can be used as a region
/// into a region.
///
/// NOTE! Freeing freelist blocks does not free memory back to the allocator. Memory is only given back to the
/// hkFreeListAllocator when a garbageCollect or freeAllFreeBlocks is called.

class hkFreeList
{
    public:
        HK_DECLARE_SYSTEM_ALLOCATOR();

            /// The size of an element, and the alignment all elements must have
        hkFreeList(hk_size_t elementSize,hk_size_t alignment,hk_size_t blockSize,
            hkFreeListMemoryServer* alloc = &hkDefaultFreeListMemoryServer::getInstance());
        ~hkFreeList() { freeAllMemory(); }

        struct Element
        {
            Element* m_next;
        };

            // The block is 16 byte aligned, as are allocations from the large block allocator. Which is nice - and a
            // requirement for the alignment.
        struct Block
        {
                // The next block in the list, or NULL if there isn't one
            Block* m_next;

                // Total amount of memory in bytes allocated to this block
            hk_size_t m_blockSize;

                // The start of the payload
            hkUint8* m_start;

                // Number of elements in this block
            hk_size_t m_numElements;
        };

            /// Allocates a chunk of memory
        HK_FORCE_INLINE void* alloc();

            /// Returns the chunk of memory to the pool. The chunk _must_ have been allocated from this pool
        HK_FORCE_INLINE void free(void* data);

            /// Allocate a batch
        HK_FORCE_INLINE void allocBatch(void** out,int sizeIn);

            /// Free a batch
        HK_FORCE_INLINE void freeBatch(void** in,int sizeIn);

            /// Frees all of the memory used by the memory pool
        void freeAllMemory();

            /// Makes all of the memory allocated available
        void freeAll();

            /// Get the element size
        hk_size_t getElementSize() const { return m_elementSize; }

            /// Does checks to see if this memory is ok
        hkBool isOk() { return _checkFreeBlocks(); }

            /// Returns true if there is an element available (without extra allocation)
        hkBool isFreeElementAvailable() const { return m_free || m_top < m_blockEnd; }

            /// Simple memory stats
        void calculateStatistics(hkMemoryStatistics& stats);

            /// Finds blocks which are not used, and puts them on the 'freeBlocks' list
            /// Returns number of blocks found, or <0 if couldn't do the work
        int findGarbage();

            /// Collect
        inline void garbageCollect();

            /// Frees all of the free blocks
        int freeAllFreeBlocks();

            /// Returns true if there are freeblocks in the list
        hkBool hasFreeBlocks() const { return m_freeBlocks != HK_NULL; }

            /// Free any blocks smaller than minSize
        void freeSmallFreeBlocks(hk_size_t minSize);

            /// Get the total amount of free elements that are available
        hk_size_t getNumFreeElements() const { return m_numFreeElements; }

            /// Get the total mumber of elements, free and used that have been allocated
        hk_size_t getTotalNumElements() const { return m_totalNumElements; }

            /// Get the total number of elements that have been used
        hk_size_t getNumUsedElements() const { return m_totalNumElements - m_numFreeElements; }

            /// Get the current allocation block size
        hk_size_t getBlockSize() const { return m_blockSize; }

        int _getTotalNumBlocks(Block* blocks) const
        {
            int num = 0;
            while (blocks)
            {
                num++;
                blocks = blocks->m_next;
            }
            return num;
        }

            /// returns the total amount of blocks (NOT ELEMENTS)
        int getTotalNumBlocks() const { return _getTotalNumBlocks(m_freeBlocks) + _getTotalNumBlocks(m_blocks); }

            /// returns pointers to all the allocations large allocations (NOT ELEMENTS), numBlocks must be the same as what was returned
            /// from getTotalNumBlocks
        void getBlocks(void** blocks,int numBlocks);

            /// Will callback for each free and allocated memory block
        void walkMemory(hkMemoryWalkCallback callback,int pool,void* param);

            /// Blocksize for elements
        static hk_size_t bestBlockSize(hk_size_t elementSpace,hk_size_t align);

        protected:

            // Make more space and return a new block from the space that was allocated. If not possible to allocate
            // return HK_NULL
        void* addSpace();

            // Returns the amount free elements
        hk_size_t _calcNumFreeElements() const;

            // Returns the total amount elements, free or used
        hk_size_t _calcTotalNumElements() const;

            // Returns the number of used elements
        hk_size_t _calcNumUsedElements() const { return _calcTotalNumElements() - _calcNumFreeElements(); }

            // Goes down the linked list working out how many elements there are
        static hk_size_t _calcTotalNumElements(Block* cur);

            // Frees a link list of blocks
        int _freeBlocks(Block* cur);

            // Calculates the sum size of a linked list of blocks
        hk_size_t _calculateBlocksTotalSize(Block* cur);

        static HK_FORCE_INLINE hkBool _compareBlocks( const Block* a, const Block* b ) { return a < b; }
        static HK_FORCE_INLINE hkBool _compareElements( const Element* a, const Element* b ) { return a < b; }

            /// Add all the elements of block to the free list
        void _addBlockElements(Block* block)
        {
            m_top = block->m_start;
            m_blockEnd = block->m_start + block->m_numElements * m_elementSize;
        }

        void  _walkMemoryBlockList(Block* block,hkMemoryWalkCallback callback,int pool,void* param);

        hkBool _checkFreeBlocks();

    protected:

            // Singly linked list of free elements. NULL terminated
        Element* m_free;

            // The size of a single element
        hk_size_t m_elementSize;

            // The active blocks
        Block* m_blocks;

            // The free blocks (blocks which haven't seen use in current allocation)
        Block* m_freeBlocks;

        hk_size_t m_blockSize;
        hk_size_t m_align;
        hk_size_t m_maxBlockSize;

            // The top of the current block, above it memory is free up until blockEnd is hit
        hkUint8* m_top;
        hkUint8* m_blockEnd;

            // A cache of the total amount of free elements
        hk_size_t m_numFreeElements;
            // A cached value of the total amount of elements
        hk_size_t m_totalNumElements;

            // The large block allocator
        hkFreeListMemoryServer* m_memoryServer;
};

#include <Common/Base/Memory/Memory/FreeList/hkFreeList.inl>

#endif // HK_FREELIST

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
