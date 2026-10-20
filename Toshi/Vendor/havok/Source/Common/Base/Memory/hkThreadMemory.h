/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_hkThreadMemory_H
#define HKBASE_hkThreadMemory_H

#include <Common/Base/Config/hkConfigThread.h>
#include <Common/Base/Thread/Thread/hkThreadLocalData.h>
#include <Common/Base/Memory/MemoryClasses/hkMemoryClassDefinitions.h>

class hkMemory;

// This allocator forwards to hkSystemMalloc
#if !defined(HK_PLATFORM_PS3_SPU)
extern void* (HK_CALL *hkSystemMalloc)(int size, int align);
extern void (HK_CALL *hkSystemFree) (void* p);
#define HK_DECLARE_SYSTEM_ALLOCATOR() \
	HK_FORCE_INLINE void* HK_CALL operator new(size_t nbytes)	{ return hkSystemMalloc(int(hkUlong(nbytes)),16); }	\
	HK_FORCE_INLINE void  HK_CALL operator delete(void* p)			{ hkSystemFree(p); }	\
	HK_FORCE_INLINE void* HK_CALL operator new(size_t, void* p)	{ return p; }	\
	HK_FORCE_INLINE void  HK_CALL operator delete(void*, void*)		{ }	\
	class MustEndWithSemiColon
#	define HK_VIRTUAL virtual
#else // SPU
#	define HK_DECLARE_SYSTEM_ALLOCATOR()
#	define virtual
#endif

	/// All memory allocations per thread are handled by this class.
	/// All memory allocations are shared between the threads, that means a block
	/// which is allocated by one hkThreadMemory can be freed by another.
	/// hkThreadMemory forwards requests to the hkMemory singleton.<br>
	///
	/// Notes:
	///    - It is highly optimized for handling memory for a single thread. It
	///      is doing so by caching a limited number of blocks in a thread local
	///      free list. There is a several free lists for different sizes of blocks.
	///    - Each free list (for each size) can hold only a maximum number of items (m_maxNumElemsOnFreeList)
	///    - If maxNumElemsOnFreeList==0 than the free list is not checked at all and the hkThreadMemory
	///		 calls the appropriate hkMemory::getInstance() functions
	///    - If for a given size, there is no free block on the free list, some blocks are fetched by calling
	///      hkMemory::getInstance().allocateBatch()
	///    - If a block is freed, it is assumed that this block was allocated by this memory manager, its
	///      corresponding free list is found (by rounding up the size of the block to the next suitable
	///      block size) and the block added to the free list.
	///		 If the number of elements on this particular free list is exceeding m_maxNumElemsOnFreeList, some
	///      blocks are forwarded to the hkMemory::getInstance().deallocateBatch()
	/// Notes
	///		- all allocations greater than 8 bytes should be 16 byte aligned.
	///		- Allocations of 8 bytes or less should be aligned to 8 bytes.
	///     - Only if you use this class with the pool memory, you are allowed to set maxNumElemsOnFreeList != 0
	///     - This class cannot be replaced like the hkMemory(), but can be disabled.
class hkThreadMemory
{
	public:

		HK_DECLARE_SYSTEM_ALLOCATOR();

			/// Creates a new instance of a thread memory. Initialise the reference count to 1.
		hkThreadMemory( hkMemory* mainMemoryManager );

			/// Allocates a fixed size piece of memory, aligned to 16 bytes.
			/// Allocations of 8 bytes or less are aligned to 8 bytes.
			/// Note that the size is not stored and must be supplied when deallocating.
		void* allocateChunk( int nbytes, HK_MEMORY_CLASS cl );
			/// Deallocates a piece of memory that has been allocated by allocateChunk
		void deallocateChunk(void*, int nbytes, HK_MEMORY_CLASS );

			/// Allocate nbytes bytes, aligned to 16 bytes.
			/// Allocations of 8 bytes or less are aligned to 8 bytes.
			/// Memory classes are defined in 'hkbase/memory/hkMemoryClass.h'.
		void* allocate( int nbytes, HK_MEMORY_CLASS cl );
			/// Deallocates a piece of memory.
		void deallocate(void*);

			/// A very fast inline version allocateChunk() which requires that the size is known at compile time and is less than 8k
		HK_FORCE_INLINE void* allocateChunkConstSize( int nbytes, HK_MEMORY_CLASS cl );

			/// A very fast inline version of deallocateChunk() which requires that the size is known at compile time and is less than 8k, and not null.
		HK_FORCE_INLINE void deallocateChunkConstSize(void* p, int nbytes, HK_MEMORY_CLASS cl );

			/// Allocates a piece of memory from a LIFO pool, aligned to 16 bytes.
		HK_FORCE_INLINE void* allocateStack(int nbytes);

			/// Reduces the size of the top allocated stack
		HK_FORCE_INLINE void shrinkAllocatedStack(void* ptr, int newSize);

			/// Deallocates a piece of memory that has been allocated by allocateStack
		HK_FORCE_INLINE void deallocateStack(void* p);
			/// Allocates nbytes bytes, aligned to 'alignment' which is a power of 2.
			/// Note that the default allocate is already 16 byte aligned.
			/// Memory classes are defined in 'hkbase/memoryclasses/hkMemoryClasses.h'.
		virtual void* alignedAllocate(int alignment, int nbytes, HK_MEMORY_CLASS cl);
			/// Deallocates a piece of memory. This must have been allocated by a call to alignedAllocate.
		virtual void  alignedDeallocate(void* ptr);

			/// Make initial stack allocations come from buf. buf should be 16 byte aligned.
			/// Havok requires temporary memory (called a "stack area") to perform many of its operations.
			/// This memory must be explicitly set after hkBaseSystem has been initialised.
			/// You can call this method any time the stack is empty, e.g. outside of hkpWorld::stepDeltaTime callbacks.
		virtual void setStackArea(void* buf, int nbytes);

			/// Get the thread local instance of this memory manager
		static inline hkThreadMemory* HK_CALL getInstancePtr(); // may be null
		static inline hkThreadMemory& HK_CALL getInstance(); // will assert if null

#if !defined(HK_PLATFORM_PS3_SPU)
			/// Replace the thread local instance of this memory manager
		static void HK_CALL replaceInstance(hkThreadMemory* m);

			/// Adds a reference
		void addReference();

			/// removes a reference
		void removeReference();
#endif

			/// Gives all local memory back to the main memory manager
		virtual void releaseCachedMemory();

			/// Calls releaseCachedMemory
		HK_FORCE_INLINE virtual ~hkThreadMemory(){ releaseCachedMemory(); }

	protected:

			/// Override this function to hook into the frame based allocation.
			/// This function is called from allocateStack when there is not enough
			/// room in the current stack to satisfy the request. It should adjust
			/// m_stack as necessary and return a pointer to a new block of nbytes.<p>
			/// You can disable the frame based allocation completely (for instance if
			/// you are running several threads) by overriding this method
			/// (and onStackUnderflow)to simply return a new block of memory without
			/// updating m_stack. This ensures that allocateStack will always forward
			/// to onStackOverflow.
		virtual void* onStackOverflow(int nbytes);

			/// Override this function to hook into the frame based allocation.
			/// Called when the current stack becomes empty. See also onStackOverflow.
		virtual void onStackUnderflow(void* p);

	public:

		enum
		{
				// We are aligning on a 64 byte boundary,
				// as all the data layout of all major functions is optimized for
				// the 64 bytes cache line width of playstation
			PAGE_ALIGN  = 64,

				// All small rows must align with this shift
			MEMORY_SMALL_BLOCK_RSHIFT_BITS = 4,		
				/// Added before the shift
			MEMORY_SMALL_BLOCK_ADD = 0xf,

				// The number of small chunk sizes
			MEMORY_MAX_SMALL_ROW  = 13,

				// The number of small and large chunk sizes
			MEMORY_MAX_ALL_ROW = (MEMORY_MAX_SMALL_ROW+4),

				// The largest small block we allocate from this pool - this is specially sized to match sizeof(hkpRigidBody)
			MEMORY_MAX_SIZE_SMALL_BLOCK  = 512 + 32,

				// The largest large block we allocate from this pool
			MEMORY_MAX_SIZE_LARGE_BLOCK = 8192,

				// The low bits we ignore when indexing into the large arrays
			MEMORY_LARGE_BLOCK_RSHIFT_BITS = 10,

				// How much we allocate when the small pool becomes full
			MEMORY_SYSTEM_PAGE_SIZE = 8192,

				// Debugging
			MEMORY_MAGIC_NUMBER = 0x3425234
		};

		struct Stack
		{
			Stack()
				:	m_current(HK_NULL),
					m_prev(HK_NULL),
					m_base((char*)-1),
					m_end(HK_NULL)
				// base is used several ways:
				// usually points to base of current block.
			{
			}
			HK_ALIGN16(char* m_current);
			Stack* m_prev;
			char* m_base;
			char* m_end;

			int getFreeBytes() const { return int(m_end - m_current)-16; }
		};

		HK_FORCE_INLINE Stack& HK_CALL getStack();

			/// Get the row of a given size. The size has to be less than the largest small block size
		HK_FORCE_INLINE int getRow(int nbytes) const;

			/// Get the row of a given size. The size has to be less than the largest small block size
			/// This is faster if the size is known at compile time
		static HK_FORCE_INLINE int HK_CALL constSizeToRow( int size );

			/// returns the memory block size of the elements per row
		HK_FORCE_INLINE int rowToSize( int row ) const;

	protected:

			// thread memory talks to main memory in batches of this size
		enum { BATCH_SIZE=4 };

			// Called when an allocation from an empty row is requested.
		void* onRowEmpty(int row, HK_MEMORY_CLASS cl);
			// Called when an deallocation from a full row is requested. p is a ptr that will be added to that row
		void onRowFull(int row, void* p, HK_MEMORY_CLASS cl);
			// Clear all of the pointers on that row. 
		void clearRow(int rowIndex, HK_MEMORY_CLASS cl);

		//
		//	Internal public section
		//
	public:

			// pointer to the main memory
		hkMemory* m_memory;

			// reference count
		int m_referenceCount;

			// a stack based allocation system
		Stack m_stack;
		int m_stackSize;

			// the maximum number of elements on each free list before the parent hkMemory is called
		int m_maxNumElemsOnFreeList;

		struct FreeList
		{
			enum { MAX_FREELIST=0x0fffffff };
			FreeList() : m_head(HK_NULL), m_numElem(0) {}
			void put( void* p )
			{
				m_numElem += 1;
				FreeElem* fl = static_cast<FreeElem*>(p);
				fl->m_next = m_head;
				m_head = fl;
			}
			void* get()
			{
				if(void* p = m_head)
				{
					m_numElem -= 1;
					m_head = m_head->m_next;
					return p;
				}
				return HK_NULL;
			}
			
			struct FreeElem	{ FreeElem* m_next; };
			FreeElem* m_head;
			int m_numElem;
		};

			// free list for blocks of each size
		FreeList m_free_list[MEMORY_MAX_ALL_ROW];

			// a lookup table of size of each block size
		int m_row_to_size_lut[MEMORY_MAX_ALL_ROW];

			// a lookup table of sizes to small block size
		char m_small_size_to_row_lut[(MEMORY_MAX_SIZE_SMALL_BLOCK >> MEMORY_SMALL_BLOCK_RSHIFT_BITS)+1];

			// a lookup table of sizes to large block size
		int m_large_size_to_row_lut[ (MEMORY_MAX_SIZE_LARGE_BLOCK >> MEMORY_LARGE_BLOCK_RSHIFT_BITS) ];
};

#if defined(HK_PLATFORM_PS3_SPU)
#	undef virtual
#endif

hkThreadMemory* HK_CALL hkThreadMemory::getInstancePtr()
{	
	// move this back to a static member when compiler bug fixed
	// TLS slot to store this threads mem instance.
	extern HK_THREAD_LOCAL( hkThreadMemory* ) hkThreadMemory__s_threadMemoryInstance;
	return HK_THREAD_LOCAL_GET(hkThreadMemory__s_threadMemoryInstance);
}

hkThreadMemory& HK_CALL hkThreadMemory::getInstance()
{	
	hkThreadMemory* instance = getInstancePtr();
	
	//HK_ASSERT( 0xf032de34, instance != HK_NULL && "No hkThreadMemory. Did you call hkBaseSystem::initThread() for your new thread?" );
	return *instance;	
}

template <typename TYPE>	HK_FORCE_INLINE TYPE* HK_CALL hkAllocateStack(int n, const char* what = 0);
template <typename TYPE>    HK_FORCE_INLINE void HK_CALL hkShrinkAllocatedStack(TYPE* ptr, int newSize );	// shrink the last piece allocated by hkAllocateStack
template <typename TYPE>	HK_FORCE_INLINE void HK_CALL hkDeallocateStack(TYPE* ptr);

#if defined(HK_COMPILER_MWERKS)
#	define HK_OPERATOR_DELETE
#	define HK_OPERATOR_NONVIRTUAL_DELETE
#else
#	define HK_OPERATOR_DELETE \
		HK_FORCE_INLINE void  HK_CALL operator delete(void*, void*)		{ }	\
		HK_FORCE_INLINE void  HK_CALL operator delete[](void*, void*)	{ HK_BREAKPOINT(0);	}
#	define HK_OPERATOR_NONVIRTUAL_DELETE \
		HK_FORCE_INLINE void  HK_CALL operator delete(void*, void*)		{ } \
		HK_FORCE_INLINE void  HK_CALL operator delete[](void*, void*)	{ }
#endif

#	define HK_MUST_END_WITH_SEMICOLON class MustEndWithSemiColon

#if defined( HK_PLATFORM_PS3_SPU)
#define HK_DECLARE_CLASS_ALLOCATOR_UNCHECKED(TYPE) \
	HK_FORCE_INLINE void* HK_CALL operator new(hk_size_t nbytes)	{ static int q; return &q; }	\
	HK_FORCE_INLINE void  HK_CALL operator delete(void* p)			{ } 	\
	HK_FORCE_INLINE void* HK_CALL operator new[](hk_size_t nbytes)	{ static int q; return &q; }	\
	HK_FORCE_INLINE void  HK_CALL operator delete[](void* p)		{  }	\
	HK_FORCE_INLINE void* HK_CALL operator new(hk_size_t, void* p)	{ return p; }	\
	HK_FORCE_INLINE void* HK_CALL operator new[](hk_size_t, void* p){ return p; }	\
	HK_OPERATOR_DELETE \
	HK_MUST_END_WITH_SEMICOLON

#else
#define HK_DECLARE_CLASS_ALLOCATOR_UNCHECKED(TYPE) \
	HK_FORCE_INLINE void* HK_CALL operator new(size_t nbytes)	{ HK_ASSERT_OBJECT_SIZE_OK(nbytes); hkReferencedObject* b = static_cast<hkReferencedObject*>(hkThreadMemory::getInstance().allocateChunk(static_cast<int>(nbytes),TYPE)); b->m_memSizeAndFlags = static_cast<hkUint16>(nbytes); return b; }	\
	HK_FORCE_INLINE void  HK_CALL operator delete(void* p)			{ hkReferencedObject* b = static_cast<hkReferencedObject*>(p); hkThreadMemory::getInstance().deallocateChunk(p, b->m_memSizeAndFlags,TYPE); }	\
	HK_FORCE_INLINE void* HK_CALL operator new[](size_t nbytes)	{ hkReferencedObject* b = static_cast<hkReferencedObject*>(hkThreadMemory::getInstance().allocateChunk(static_cast<int>(nbytes),TYPE)); b->m_memSizeAndFlags = static_cast<hkUint16>(nbytes); return b; }	\
	HK_FORCE_INLINE void  HK_CALL operator delete[](void* p)		{ hkReferencedObject* b = static_cast<hkReferencedObject*>(p); hkThreadMemory::getInstance().deallocateChunk(p, b->m_memSizeAndFlags,TYPE); }	\
	HK_FORCE_INLINE void* HK_CALL operator new(size_t, void* p)	{ return p; }	\
	HK_FORCE_INLINE void* HK_CALL operator new[](size_t, void* p){ HK_BREAKPOINT(0); return p; }	\
	HK_OPERATOR_DELETE \
	HK_MUST_END_WITH_SEMICOLON
#endif

#define HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR_BY_SIZE_UNCHECKED(TYPE,CLASS_SIZE) \
	HK_FORCE_INLINE void* HK_CALL operator new(size_t nbytes)	{ HK_ASSERT(0x6c787b7f, nbytes == CLASS_SIZE ); return hkThreadMemory::getInstance().allocateChunkConstSize(static_cast<int>(nbytes),TYPE); }	\
	HK_FORCE_INLINE void  HK_CALL operator delete(void* p)			{ if (p) hkThreadMemory::getInstance().deallocateChunkConstSize(p, CLASS_SIZE, TYPE); }								\
	HK_FORCE_INLINE void* HK_CALL operator new[](size_t nbytes)	{ return hkThreadMemory::getInstance().allocate(static_cast<int>(nbytes),TYPE); }												\
	HK_FORCE_INLINE void  HK_CALL operator delete[](void* p)		{ hkThreadMemory::getInstance().deallocate(p); } \
	HK_FORCE_INLINE void* HK_CALL operator new(size_t n, void* p){ HK_ASSERT(0x77bb90a1, n == CLASS_SIZE); return p; } \
	HK_FORCE_INLINE void* HK_CALL operator new[](size_t, void* p){ return p;	} \
	HK_OPERATOR_NONVIRTUAL_DELETE \
	HK_MUST_END_WITH_SEMICOLON
	
//
// In debug, use some compile time trickery to ensure correct allocator is used.
// Use -Wshadow to catch nonvirtual allocators in virtual classes.
//
#if defined(HK_DEBUG)
#   define HK_DECLARE_CLASS_ALLOCATOR(TYPE) \
        HK_FORCE_INLINE const hkReferencedObject* iMustBeDerivedFromReferencedObject() const { return static_cast<const hkReferencedObject*>(this); } \
        HK_DECLARE_CLASS_ALLOCATOR_UNCHECKED(TYPE)
#   define HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(TYPE,THIS_CLASS) \
        HK_FORCE_INLINE void iShouldNotHaveVtable() const { int m_memSizeAndFlags = 0; m_memSizeAndFlags--; /* if you get this error, you derive from hkReferencedObject! */ } \
        HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR_BY_SIZE_UNCHECKED(TYPE, sizeof(THIS_CLASS))
#else
#   define HK_DECLARE_CLASS_ALLOCATOR(TYPE) \
		HK_DECLARE_CLASS_ALLOCATOR_UNCHECKED(TYPE)
#   define HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(TYPE,THIS_CLASS) \
        HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR_BY_SIZE_UNCHECKED(TYPE, sizeof(THIS_CLASS))
#endif

/// Convenience inline function to allocate memory of the correct type
template <typename TYPE>
HK_FORCE_INLINE TYPE* HK_CALL hkAllocate(int numberOfObjects, HK_MEMORY_CLASS cl)
{
	return static_cast<TYPE*>(hkThreadMemory::getInstance().allocate(numberOfObjects*hkSizeOf(TYPE), cl));
}

/// Convenience inline function to deallocate memory of the correct type
template <typename TYPE>
HK_FORCE_INLINE void HK_CALL hkDeallocate(TYPE* ptr)
{
	hkThreadMemory::getInstance().deallocate(static_cast<void *>(ptr));
}

/// Convenience inline function to allocate aligned memory of the correct type
template <typename TYPE>
HK_FORCE_INLINE TYPE* HK_CALL hkAlignedAllocate(int alignment,int numberOfObjects, HK_MEMORY_CLASS cl)
{
	return static_cast<TYPE*>(hkThreadMemory::getInstance().alignedAllocate(alignment,numberOfObjects*hkSizeOf(TYPE), cl));
}

/// Convenience inline function to deallocate memory of the correct type
template <typename TYPE>
HK_FORCE_INLINE void HK_CALL hkAlignedDeallocate(TYPE* ptr)
{
	hkThreadMemory::getInstance().alignedDeallocate(static_cast<void *>(ptr));
}

/// Convenience function to allocate memory of the correct type
template <typename TYPE>
HK_FORCE_INLINE TYPE* HK_CALL hkAllocateChunk(int numberOfObjects, HK_MEMORY_CLASS cl)
{
	return static_cast<TYPE*>(hkThreadMemory::getInstance().allocateChunk(numberOfObjects*hkSizeOf(TYPE), cl));
}

/// Convenience function to deallocate memory of the correct type
template <typename TYPE>
HK_FORCE_INLINE void HK_CALL hkDeallocateChunk(TYPE* ptr, int numberOfObjects, HK_MEMORY_CLASS cl)
{
	hkThreadMemory::getInstance().deallocateChunk(static_cast<void*>(ptr), numberOfObjects*hkSizeOf(TYPE), cl);
}

#if defined(HK_DEBUG) && !defined(HK_PLATFORM_PS3_SPU)
extern void HK_CALL HK_ASSERT_OBJECT_SIZE_OK_FUNC(hk_size_t nbytes);
#	define HK_ASSERT_OBJECT_SIZE_OK(A) HK_ASSERT_OBJECT_SIZE_OK_FUNC(A)
#else
#	define HK_ASSERT_OBJECT_SIZE_OK(A)
#endif



#endif // HKBASE_hkThreadMemory_H


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
