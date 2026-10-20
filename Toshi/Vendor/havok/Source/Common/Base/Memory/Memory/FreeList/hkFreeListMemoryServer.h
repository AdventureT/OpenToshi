/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_FREE_LIST_MEMORY_SERVER_H
#define HK_FREE_LIST_MEMORY_SERVER_H

// Defines the interface that the hkFreeList uses to access underlying memory
// The hkLargeBlockAllocator implements this interface

class hkFreeListMemoryServer
{
    public:
        virtual ~hkFreeListMemoryServer() {}

            /// Allocate some memory
        virtual void* alloc(hk_size_t size) = 0;

            /// Free some memory
        virtual void free(void* data)=0;

            /// Returns true if this appears to be a valid allocation.
            /// Does some fairly comprehesive tests, so will be slow in general.
        virtual hkBool isValidAlloc(void* data) = 0;

            /// Returns the total amount of memory available in an allocated block
            /// This extra memory can be used if needs be. The value returned will always be greater
            /// than or equal to the size passed in. The size is the size that the client believes the block
            /// to be. It must be either the size requested when the block was allocated, or a size returned
            /// by this method, otherwise the result is undefined.
            ///
            /// NOTE an implementation that does not implement this functionality - can always just return the
            /// size passed in.
        virtual hk_size_t getAllocSize(void* p,hk_size_t size) = 0;

            /// Returns the total amount of bytes taken up by an allocation
            /// The size passed in is the size of the block - it can be either the size passed in the original
            /// allocation or the size returned from getAllocSize.
        virtual hk_size_t getAllocTotalSize(void* p,hk_size_t size) = 0;

            /// Allocate memory, if can't find memory internally will fallback to the servers critical memory system
            /// Memory must be freed in the opposite order to allocation. Allocations should only be used for the
            /// shortest possible time, as the underlying memory may have severe restrictions
        virtual void* criticalAlloc(hk_size_t size) = 0;

            /// To free a critical memory allocation
        virtual void criticalFree(void* ptr,hk_size_t size) = 0;
};


/// Implementation of the hkFreeListAllocator interface that just uses the hkThreadMemory for its
/// implementation. Allows you to use freelists as a fast memory allocation system separately from the
/// the hkFreeListMemory system.
///
/// Performance improvements come from not having to lookup the appropriate freelist from the size of
/// an allocation as, well as not having to have the freelist allocate from inside of a critical section
/// as is the case inside of the hkFreeListMemory implementation

class hkDefaultFreeListMemoryServer: public hkReferencedObject, public hkSingleton<hkDefaultFreeListMemoryServer>,public hkFreeListMemoryServer
{
    public:
        HK_DECLARE_SYSTEM_ALLOCATOR();

        hkDefaultFreeListMemoryServer(HK_MEMORY_CLASS cls = HK_MEMORY_CLASS_ROOT)
        : m_memoryClass(cls)
        {
        }

        /// implementing hkFreeListMemoryServer
        virtual void* alloc(hk_size_t size);
        virtual void free(void* data);
        virtual hkBool isValidAlloc(void* data) { return true; }
        virtual hk_size_t getAllocSize(void* p,hk_size_t size) { return size; }
        virtual hk_size_t getAllocTotalSize(void* p, hk_size_t size) { return size; }
        virtual void* criticalAlloc(hk_size_t size) { return alloc(size); }
        virtual void criticalFree(void* ptr,hk_size_t size) { free(ptr); }

        HK_MEMORY_CLASS m_memoryClass;
};


#endif // HK_FREE_LIST_MEMORY_SERVER_H

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
