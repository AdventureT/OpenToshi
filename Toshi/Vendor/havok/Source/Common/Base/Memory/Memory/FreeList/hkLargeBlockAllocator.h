/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_LARGE_BLOCK_ALLOCATOR_H
#define HK_LARGE_BLOCK_ALLOCATOR_H

// The hkLargeBlockAllocator implements the hkFreeListMemoryServer interface

#include <Common/Base/Memory/Memory/FreeList/hkFreeListMemoryServer.h>

struct hkMemoryStatistics;

    /// Used for finding all of the used blocks
typedef void (HK_CALL *hkMemBlockCallback)(void* start, hk_size_t blockSize,void* param);

/// This class is an interface which the hkLargeBlockAllocator uses to get the underlying memory
class hkMemoryBlockServer
{
	public:
		HK_DECLARE_SYSTEM_ALLOCATOR();

			/// All allocs are guaranteed to fit this alignment
		enum { ALIGN = 16 };

			/// Dtor
			/// We'll have a virtual dtor so safe destruction through this interface
		virtual ~hkMemoryBlockServer() {}

			/// Returns true if this server is the single block style.
			/// There are two styles of servers supported. Ones which allocate a single block. The amount used can
			/// be varied potentially via the 'resize' mechanism. If it isn't the single block style then it is
			/// the multi-block style. Multi block allocators can allocate many blocks. Generally they do not support
			/// block resizing.
		virtual hkBool isSingleBlockServer() =0;

			/// Recommend a size that the server is best able to deal with.
			/// The server may deal want to deal with different sizes of allocations in different
			/// ways. This call for a given size will return a size either the same or greater that
			/// this server recommends for allocation. It is best to use what it returns because
			/// it will likely have less wastage too.
		virtual hk_size_t recommendSize(hk_size_t size) = 0;

			/// Allocates a chunk. All allocations are 16 byte aligned. Returns the actual size of the allocated block.
		virtual void* allocate(hk_size_t size, hk_size_t& sizeOut) =0;

			/// Frees a block allocated through alloc. The size must be the size returned from alloc.
		virtual void free(void* data, hk_size_t size) = 0;

			/// Given a block allocated with allocate will try and make it bigger or smaller. This can be thought of a generalization
			/// of sbrk. Returns true if the resize was successful - sizeOut is the new size of the block.
		virtual hkBool resize(void* data, hk_size_t oldSize, hk_size_t newSize, hk_size_t& sizeOut) = 0;

			/// Returns the recommended minimum allocation size. Doing allocations smaller that this could have
			/// messy consequences.
		hk_size_t getMinAllocSize() const { return m_minAllocSize; }

			/// Returns the total amount of unallocated memory available. If this isn't meaningful (for example
			/// with virtual memory its effectively infinate) the method will return hkMemoryStatistics::INFINTATE
		virtual hk_size_t getTotalAvailableMemory() = 0;

			/// Get the currently set memory limit. If 0, there is no memory limit
		virtual hk_size_t getMemoryLimit() = 0;

			/// Set the memory limit, will return true if successful
			/// Note the actual limit set may be slightly different than the value requested, to take into account alignment
			/// or other low level memory issues. Use getMemoryLimit to find out what the limit ended up being.
		virtual hkBool setMemoryLimit( hk_size_t ) =0;

			/// When memory becomes restricted, it may be necessary to allocate a small amount of space to
			/// clean up the 'mess'. A fast garbage collector often needs extra space to do its work for example.
			/// This functionality allows for allocations of 'short lived' and stack allocated style memory allocations,
			/// that can be used in this situation. Deallocations MUST be in the opposite order to allocations.
		virtual void* criticalAlloc(hk_size_t size) =0;

			/// To free a critical memory allocation
		virtual void criticalFree(void* ptr, hk_size_t size) =0;

		protected:

		hk_size_t m_minAllocSize;
};

class hkLargeBlockLimitedMemoryListener
{
	public:
		HK_DECLARE_SYSTEM_ALLOCATOR();

		virtual ~hkLargeBlockLimitedMemoryListener(){}

			/// Called when a request is denied. An implementation can free up memory, and on
			/// returning from this call the allocator will try and allocate again.
		virtual void cannotAllocate(hk_size_t size) =0;

			/// This is called when the allocator has given up allocating. Generally this will be called after
			/// a call to cannotAllocate, although that is not a requirement of an implementation.
		virtual void allocationFailure(hk_size_t size) = 0;
};

/* This is a large memory block allocator, which is designed to handle allocations 256 bytes or larger. Smaller
allocations will be serviced but will internally degenerate to a 256 byte block. Thus this allocator is not
best used for small high performance chunks of memory. In that situation a freelist or pool memory style
allocator is more suitable.

What this allocator does allow for is good usage of varying sized chunks of memory. A 128k chunk of memory or a 2.7k
chunk of memory both have the same same small overhead.

The allocator is designed to be fast and efficient. When a block is requested a search is performed down a tree
of available blocks to find the one most suitable. Most of the complexity of the algorithm comes from this inplace
tree traversal.

The allocator is designed to have good cache usage

Some notes on the algorithm:

o Adjacent free chunks are always merged - thus the chunks adjacent to a free chunk must be in use.
o Each chunk consists of a header, which records the size of this block, and therefore implicitly the start of the next
block.
o Each header records
  + If this chunk is in use
  + If the previous chunk is in use
  + If the previous chunk is NOT in use, how large the previous chunk is
o When a chunk is not in use its header is 'extended' into the payload - such that the block can be a tree member
o There is the concept of the 'top block' for allocations in a contiguous chunk of memory, this is where an
  allocation would take place when all else fails
  + In this algorithm the top block is special because its contents may not be correctly constructed
o Allocations of actual memory go through the hkMemoryBlockServer
  + Multiple non contiguous allocations are possible :)
  + When this happens internally blocks are set up so the chunks of memory between the blocks owned by this allocator
    appear as 'inuse', when in reality they don't belong to this allocator
o Allocations from the hkMemoryBlockServer are held in hkMemPages
*/

/*

Possible future improvements:

o Support for realloc style constructs
  + For example returning if a block can be extended, and its current actual size, extending into a freeblock
o Ideally there would be a realloc in there somewhere
o Debugging support
  + Having headers and footers
*/

typedef unsigned int hkBinIndex;
typedef unsigned int hkBinMap;
    /// Has the same amount of bits as a hk_size_t but is signed.
typedef int hkSignedSizeT;

struct hkMemChunk
{
	static const hk_size_t PINUSE_BIT = 1;
	static const hk_size_t CINUSE_BIT = 2;
	static const hk_size_t INUSE_BITS = 3;

    static const hk_size_t ALIGN = 16;
	static const hk_size_t ALIGN_MASK = ALIGN-1;

		/// The bytes from the hkMemChunk to the payload inside
		/// Would be better to do sizeof(hkMemChunk), but I can't do that here - so I do sizeof(hk_size_t)*2
    static const hk_size_t PAYLOAD_OFFSET = (sizeof(hk_size_t)*2 + ALIGN_MASK)&~ALIGN_MASK;

			/// Returns true if the previous block is in use
	HK_FORCE_INLINE bool isPinuse() const { return (head&PINUSE_BIT)!=0; }

			/// Returns true if this block is in use
	HK_FORCE_INLINE bool isInuse() const { return (head&CINUSE_BIT)!=0; }

        /// The chunk size is the total size (including the hkMallocChunk data)
	HK_FORCE_INLINE hk_size_t getChunkSize() const { return (head&~INUSE_BITS); }

        /// Clear the previous in use flag
	HK_FORCE_INLINE void clearPinuse() { head &= ~PINUSE_BIT; }

        /// Clear the in use flag
	HK_FORCE_INLINE void clearInuse() { head &= ~CINUSE_BIT; }

		/// Returns the next chunks previous in use
	HK_FORCE_INLINE bool isNextPinuse() { return nextChunk()->isPinuse(); }

        /// Get the address of the next chunk
	HK_FORCE_INLINE hkMemChunk* nextChunk() { return (hkMemChunk*)(((char*)this) + getChunkSize()); }

        /// Get the address of the previous chunk
	HK_FORCE_INLINE hkMemChunk* previousChunk() { return (hkMemChunk*)(((char*)this) - prevFoot); }

        /// Return memory as chunk x bytes after the chunk
	HK_FORCE_INLINE hkMemChunk* chunkPlusOffset(hk_size_t s) { return (hkMemChunk*)(((char*)this)+s); }

        /// Return memory as chunk x bytes before the chunk
	HK_FORCE_INLINE hkMemChunk* chunkMinusOffset(hk_size_t s) { return (hkMemChunk*)(((char*)this)-s); }

        /// Return the address of the contained data
    HK_FORCE_INLINE void* getPayload() { return (void*)(((char*)this) + PAYLOAD_OFFSET); }

        /// Get the size of the payload
    HK_FORCE_INLINE hk_size_t getPayloadSize() { return getChunkSize() - PAYLOAD_OFFSET; }

        /// Turn an address into a memory block
	HK_FORCE_INLINE static hkMemChunk* toChunk(void* in) { return (hkMemChunk*)(((char*)in) - PAYLOAD_OFFSET); }

        /// Set cinuse and pinuse of this chunk and pinuse of next chunk
	HK_FORCE_INLINE void setInuseAndPinuse(hk_size_t s)
	{
		head = (s|PINUSE_BIT|CINUSE_BIT);
		hkMemChunk* next = chunkPlusOffset(s);
		next->head |= PINUSE_BIT;
	}

			/// Set inuse
    HK_FORCE_INLINE void setInuse(hk_size_t s)
	{
		head = (head & PINUSE_BIT)|s|CINUSE_BIT;
		hkMemChunk* next = chunkPlusOffset(s);
		next->head |= PINUSE_BIT;
	}

			/// Set ths size, inuse and pinuse of the chunk
	HK_FORCE_INLINE void setSizeAndPinuseOfInuseChunk(hk_size_t s)
	{
		head = (s|PINUSE_BIT|CINUSE_BIT);
	}

			/// Get size at footer
	HK_FORCE_INLINE hk_size_t getFoot(hk_size_t s)  { return ((hkMemChunk*)(((char*)this)+s))->prevFoot; }

			/// Set the footer size
	HK_FORCE_INLINE void setFoot(hk_size_t s) { ((hkMemChunk*)(((char*)this) + s))->prevFoot = s; }

			/// Set size, pinuse bit, and foot
	HK_FORCE_INLINE void setSizeAndPinuseOfFreeChunk(hk_size_t s)
	{
		head = (s|PINUSE_BIT);
		setFoot(s);
	}
        /// Set size, pinuse bit, foot, and clear next pinuse
	HK_FORCE_INLINE void setFreeWithPinuse(hk_size_t s,hkMemChunk* n)
	{
		n->clearPinuse();
		setSizeAndPinuseOfFreeChunk(s);
	}

        /// Returns true if a pointer is aligned appropriately
    static bool isAligned(void* ptr) { return (((hk_size_t)(ptr))&ALIGN_MASK)==0; }

	/// Members

        /// Size of previous chunk including header etc (if free).
	hk_size_t prevFoot;
        /// Size and inuse bits.
	hk_size_t head;
};


struct hkMemPage
{
			/// The previous memory block
		hkMemPage* m_prev;
			/// The next memory block
		hkMemPage* m_next;

			/// Stores the amount of allocations in this page
		int m_numAllocs;
			/// The total size as passed back from the interface
		hk_size_t m_size;

			/// The payload start
		char* m_start;
			/// The payload end
		char* m_end;

			/// The first chunk in this page
		hkMemChunk* getFirstChunk() const { return (hkMemChunk*)m_start; }
			/// The last chunk in this page
		hkMemChunk* getFooter() const { return (hkMemChunk*)(m_end - hkMemChunk::PAYLOAD_OFFSET); }

			/// If there is nothing in the page this is the size of a chunk from the start of available space
			/// up to the footer. Its the largest free block that can be stored in a page.
		hk_size_t getMaxChunkSize() const { return  (m_end - m_start) - hkMemChunk::PAYLOAD_OFFSET; }

			// The contents size
		hk_size_t getContentsSize() const { return hk_size_t(m_end - m_start); }
			/// Gets the start of the contents
		char* getStart() const { return m_start;  }
			/// Gets a byte past the end
		char* getEnd() { return m_end; }
};

/// Assumes the fields are placed after hkMemChunk

struct hkFreeMemChunk:public hkMemChunk
{
		///  double links -- used only if free.
		hkFreeMemChunk* next;
		hkFreeMemChunk* prev;
};

/*

Description of the underlying structure
----------------------------------------

Based on large block strategy of the Doug Lea allocator.

Larger chunks are kept in a form of bitwise digital trees (aka
tries) keyed on chunksizes.  Because 'tree chunks' are only for
free chunks greater than 256 bytes, their size doesn't impose any
constraints on user chunk sizes.  Each node looks like:

chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|             Size of previous chunk                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
`head:' |             Size of chunk, in bytes                         |P|
mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|             Forward pointer to next chunk of same size        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|             Back pointer to previous chunk of same size       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|             Pointer to left child (child[0])                  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|             Pointer to right child (child[1])                 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|             Pointer to parent                                 |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|             bin index of this chunk                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|             Unused space                                      .
.                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
`foot:' |             Size of chunk, in bytes                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

Each tree holding treenodes is a tree of unique chunk sizes.  Chunks
of the same size are arranged in a circularly-linked list, with only
the oldest chunk (the next to be used, in our FIFO ordering)
actually in the tree.  (Tree members are distinguished by a non-null
parent pointer.)  If a chunk with the same size an an existing node
is inserted, it is linked off the existing node using pointers that
work in the same way as fd/bk pointers of small chunks.

Each tree contains a power of 2 sized range of chunk sizes (the
smallest is 0x100 <= x < 0x180), which is is divided in half at each
tree level, with the chunks in the smaller half of the range (0x100
<= x < 0x140 for the top nose) in the left subtree and the larger
half (0x140 <= x < 0x180) in the right subtree.  This is, of course,
done by inspecting individual bits.

Using these rules, each node's left subtree contains all smaller
sizes than its right subtree.  However, the node at the root of each
subtree has no particular ordering relationship to either.  (The
dividing line between the subtree sizes is based on trie relation.)
If we remove the last chunk of a given size from the interior of the
tree, we need to replace it with a leaf node.  The tree ordering
rules permit a node to be replaced by any leaf below it.

The smallest chunk in a tree (a common operation in a best-fit
allocator) can be found by walking a path to the leftmost leaf in
the tree.  Unlike a usual binary tree, where we follow left child
pointers until we reach a null, here we follow the right child
pointer any time the left one is null, until we reach a leaf with
both child pointers null. The smallest chunk in the tree will be
somewhere along that path.

The worst case number of steps to add, find, or remove a node is
bounded by the number of bits differentiating chunks within
bins. Under current bin calculations, this ranges from 6 up to 21
(for 32 bit sizes) or up to 53 (for 64 bit sizes). The typical case
is of course much better.
*/

struct hkMemTreeChunk:public hkFreeMemChunk
{
		hkMemTreeChunk* child[2];
		hkMemTreeChunk* parent;
		hkBinIndex index;

		/// The leftmost child
		hkMemTreeChunk* leftMostChild() const { return child[0]?child[0]:child[1]; }
};

class hkLargeBlockAllocator:public hkFreeListMemoryServer
{
    public:
		enum AllocType { CRITICAL_ALLOC,NORMAL_ALLOC };
		HK_DECLARE_SYSTEM_ALLOCATOR();

            /// Ctor
        hkLargeBlockAllocator(hkMemoryBlockServer* server);

            /// Dtor
        ~hkLargeBlockAllocator();

        /// implementing hkFreeListMemoryServer
        virtual void* alloc(hk_size_t size);
        virtual void free(void* data);
        virtual hkBool isValidAlloc(void* alloc);
        virtual hk_size_t getAllocSize(void* p,hk_size_t size);
        virtual hk_size_t getAllocTotalSize(void* p,hk_size_t size);
        virtual void* criticalAlloc(hk_size_t size);
        virtual void criticalFree(void* ptr,hk_size_t size);


            /// Frees all allocations
        void freeAll();

            /// Called for all of the allocated chunks of memory
        void forAllAllocs(hkMemBlockCallback callback,void* param);

            /// Get the estimated size of an allocation
        hk_size_t getEstimatedAllocSize(hk_size_t size) const;

            /// Resizes a block - is guarenteed to succeed if the size is less than or equal to the size returned by getSize.
            /// Does not allow to make a block bigger than what was returned from getAllocSize
            /// disabled - has a nasty bug
        ///void resizeAlloc(void* p,hk_size_t newSize);

            /// Determines if mem points to a valid block
        //hkBool checkAlloc(void* mem);

        /// Runs thru all the allocations making sure they appear valid. Will return false if any seems invalid
        hkBool checkAllocations(void** allocs,int size);

            /// Get the statistics
        void calculateStatistics(hkMemoryStatistics& stats);

            /// Free up any unused space
        void garbageCollect();

            /// Set the limited memory listener
        void setLimitedMemoryListener(hkLargeBlockLimitedMemoryListener* listener) { m_limitedListener = listener; }

            /// Get the limited memory listener
        hkLargeBlockLimitedMemoryListener* getLimitedMemoryListener() const { return m_limitedListener; }

            /// Works out what the largest block available is
        hk_size_t findLargestBlockSize();

            /// Get the top block size
        hk_size_t getTopBlockSize() const { return m_topsize; }

            /// Returns the total amount of memory used in bytes
        hk_size_t getTotalMemoryUsed() const { return m_used; }

            /// Returns the total amount of memory allocated in bytes
        hk_size_t calculateTotalMemoryAllocated() const;

        class Iterator
        {
            public:
            Iterator(hkMemChunk* chunk,hkMemPage* page,hkMemChunk* end):
                m_chunk(chunk),
                m_page(page),
                m_end(end)
            {}
            Iterator():
                m_chunk(HK_NULL)    /// Make invalid
            {}
            hkBool isValid() const { return m_chunk != HK_NULL; }
            hk_size_t getSize() const { return m_chunk->getChunkSize(); }
            void* getAddress() const { return m_chunk->getPayload(); }
            hkBool isInuse() const { return m_chunk->isInuse(); }

            hkMemChunk* m_chunk;
            hkMemPage* m_page;
            hkMemChunk* m_end;
        };

            /// Gets an iterator
            /// Doing any allocations/frees will cause undefined behaviour
            /// Blocks will be returned in order from lowest to highest in memory
        Iterator getIterator();

            /// Moves the iterator to the next block of memory. Returns false if the iterator is now invalid
        hkBool nextBlock(Iterator& iter);

            /// Get the block server
        hkMemoryBlockServer* getBlockServer() const { return m_server; }

            /// Ctor
        hkLargeBlockAllocator();

#ifdef HK_DEBUG
        static void HK_CALL selfTest();
        static void HK_CALL allocatorTest(hkLargeBlockAllocator& allocator,int testSize);
#endif

protected:
                /// Number of tree bins
        static const unsigned int NTREEBINS =  (32U);
                /// The shift used to get tree bins
        static const unsigned int TREEBIN_SHIFT = (8U);
                /// The miniumum large block size
        static const hk_size_t MIN_LARGE_SIZE = hk_size_t(1)<< TREEBIN_SHIFT;

        /// Amount of bits in a size_t
        static const hk_size_t SIZE_T_BITSIZE = (sizeof(hk_size_t) << 3);

            /// Returns true if an an allocated block looks correctly formed
        hkBool _checkUsedAlloc(void* mem);

            /// If called after a successful free should return true
        hkBool _checkFreeChunk( hkMemChunk* p);

            /// The amount of memory used is cached - and returned by getTotalMemoryUsed. This method works out by traversing
            /// all the blocks how much memory is used
        hk_size_t _calculateUsed() const;

        hk_size_t _findLargestTreeBlockSize(hkMemTreeChunk* t,hk_size_t largest);

        void _markTreeMap(hkBinIndex i) { m_treemap |= (1<<i); }
        void _clearTreeMap(hkBinIndex i) { m_treemap &= ~(1<<i); }
        bool _isTreeMapMarked(hkBinIndex i) const { return (m_treemap&(1<<i))!=0; }

        void _insertLargeChunk( hkMemTreeChunk* x, hk_size_t s);
        void _unlinkLargeChunk( hkMemTreeChunk* x);

        void* _allocLarge( hk_size_t nb);
        void* _allocFromTop(hk_size_t bytes);

        void _makeTopValid() const;
        void* _alloc(hk_size_t nb);

        void _init();

            // How the hades does this work?
        static hkBinIndex _computeTreeIndex(hk_size_t s)
        {
            hk_size_t x = s >> TREEBIN_SHIFT;
                if (x == 0) { return 0; }
                if (x > 0xFFFF) { return NTREEBINS-1; }

            unsigned int y = (unsigned int)x;
            unsigned int n = ((y - 0x100) >> 16) & 8;
            unsigned int k = (((y <<= n) - 0x1000) >> 16) & 4;
            n += k;
            n += k = (((y <<= k) - 0x4000) >> 16) & 2;
            k = 14 - n + ((y <<= k) >> 15);
            return (hkBinIndex)((k << 1) + ((s >> (k + (TREEBIN_SHIFT-1)) & 1)));
        }

        static hkBinIndex _bitToIndex(unsigned int x)
        {
            unsigned int y = x - 1;
            unsigned int k = y >> (16-4) & 16;
            unsigned int n = k;        y >>= k;
            n += k = y >> (8-3) &  8;  y >>= k;
            n += k = y >> (4-2) &  4;  y >>= k;
            n += k = y >> (2-1) &  2;  y >>= k;
            n += k = y >> (1-0) &  1;  y >>= k;
            return n+y;
        }

        /// pad request bytes into a usable size
        HK_FORCE_INLINE static hk_size_t _padRequest(hk_size_t req)
        {
            return (req + hkMemChunk::PAYLOAD_OFFSET + hkMemChunk::ALIGN_MASK)&~hkMemChunk::ALIGN_MASK;
        }

                /// Bit representing maximum resolved size in a treebin at i
        static int _bitForTreeIndex(int i)
        {
            return (i == NTREEBINS-1)? (SIZE_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_SHIFT - 2);
        }
                /// Shift placing maximum resolved bit in a treebin at i as sign bit
        static int _leftShiftForTreeIndex(int i)
        {
            return ((i == NTREEBINS-1)? 0 : ((SIZE_T_BITSIZE-hk_size_t(1)) - ((i >> 1) + TREEBIN_SHIFT - 2)));
        }
                /// The size of the smallest chunk held in bin with index i
        static hk_size_t _minSizeForTreeIndex(int i)
        {
            return ((hk_size_t(1) << (((i) >> 1) + TREEBIN_SHIFT)) |  (((hk_size_t)((i) & hk_size_t(1))) << (((i) >> 1) + TREEBIN_SHIFT - 1)));
        }

            /// Trys to resize the page allocated from a SingleBlockServer. If successful will fix up the top block etc
        hkBool _resizeSingleBlockServerPage(hk_size_t newSize);

            /// Returns true if n is greater than p
        template <typename S,typename T>
        static bool _okNext(const S* p,const T* n) { return ((const char*)(p) < (const char*)(n)); }

            /// Does noddy checks to see if an address is possibly correct
        HK_FORCE_INLINE bool _isOkAddress(void *a) { return ((char*)(a) >= m_leastAddr); }

            /// isolate the least set bit of a bitmap
        HK_FORCE_INLINE static int _leastBit(int x) { return ((x) & -(x)); }

            /// mask with all bits to left of least bit of x on
        HK_FORCE_INLINE static int _leftBits(int x) { return ((x<<1) | -(x<<1)); }

            /// An index to a set bit
        HK_FORCE_INLINE static int _indexToBit(int x) { return 1<<x; }

            /// mask with all bits to left of or equal to least bit of x on
        HK_FORCE_INLINE static int _sameOrLeftBits(int x) { return ((x) | -(x)); }

        static HK_FORCE_INLINE hkBool _comparePointers( void* a,void* b)
        {
            return (char*)a < (char*)b;
        }

            /// True if there are no allocated pages
        hkBool _hasPages() { return m_pages.m_next != &m_pages; }

        static void HK_CALL _addAlloc(void* data,hk_size_t size,void* param);

	protected:
		    // A listener which handles situations when memory allocation gets tight
        hkLargeBlockLimitedMemoryListener* m_limitedListener;

			// A bit field with a bit indicating if a tree bucket has contents or not
        hkBinMap m_treemap;

			//
        char* m_leastAddr;

			// The 'topmost memory' chunk
        hkMemChunk* m_top;

			// Cached size of topmost block
        hk_size_t m_topsize;

			// The tree bins
        hkMemTreeChunk* m_treebins[NTREEBINS];

			// Server for getting large memory blocks
        hkMemoryBlockServer* m_server;

			// Used as top when starting up
		hkMemChunk m_zeroChunk;

			// This dummy page is the start of a doubly linked list of memory pages allocated through the server interface
		hkMemPage m_pages;

			// The total amount of used (allocated) memory
		hk_size_t m_used;
};

#endif // HK_LARGE_BLOCK_ALLOCATOR_H

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
