/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SYSTEM_MEMORY_BLOCK_SERVER_H
#define HK_SYSTEM_MEMORY_BLOCK_SERVER_H

#include <Common/Base/Memory/Memory/FreeList/hkLargeBlockAllocator.h>

class hkSystemMemoryBlockServer:public hkMemoryBlockServer
{
public:
	HK_DECLARE_SYSTEM_ALLOCATOR();

    hkSystemMemoryBlockServer(hk_size_t minBlockSize = 0);

    /// implementing hkMemoryBlockServer
    virtual hkBool isSingleBlockServer() { return false; }
    virtual hk_size_t recommendSize(hk_size_t size);
    virtual void* allocate(hk_size_t size,hk_size_t& sizeOut);
    virtual void free(void* data,hk_size_t size);
        /// This implementation can't extend
    virtual hkBool resize(void* data,hk_size_t oldSize,hk_size_t newSize,hk_size_t& sizeOut) { return false; }

        /// Returns the total amount of memory available from this allocator (approx)
        /// Here its just the difference between the limit and whats allocated.
        /// If there is no limit, method will return hkMemoryStatistics::INIFINITE_SIZE
    virtual hk_size_t getTotalAvailableMemory();

        /// Get the memory limit
    virtual hk_size_t getMemoryLimit();
        /// Set the memory limit
    virtual hkBool setMemoryLimit(hk_size_t size);

    virtual void* criticalAlloc(hk_size_t size);
    virtual void criticalFree(void* data,hk_size_t size);

    protected:
        /// The total amount of bytes allocated
    hk_size_t m_allocated;
        /// The memory limit
    hk_size_t m_memoryLimit;
};

#endif // HK_SYSTEM_MEMORY_BLOCK_SERVER_H

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
