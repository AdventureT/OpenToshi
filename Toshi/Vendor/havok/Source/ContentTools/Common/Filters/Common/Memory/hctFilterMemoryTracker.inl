/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline void* hctFilterMemoryTracker::allocate(int nbytes, HK_MEMORY_CLASS cl)
{
	unsigned char* val = hkAllocate<unsigned char>(nbytes, cl);
	MemoryAlloc* ma = hkAllocateChunk<MemoryAlloc>(1, HK_MEMORY_CLASS_EXPORT);
	ma->m_mem = val;
	ma->m_next = m_allocs;
	m_allocs = ma;
	return val;
}

inline void* hctFilterMemoryTracker::allocateChunk(int nbytes, HK_MEMORY_CLASS cl)
{
	unsigned char* val = hkAllocateChunk<unsigned char>(nbytes, cl);
	MemoryChunk* ca = hkAllocateChunk<MemoryChunk>(1, HK_MEMORY_CLASS_EXPORT);
	ca->m_mem = val;
	ca->m_memSize = nbytes;
	ca->m_memClass = cl;
	ca->m_next = m_chunkAllocs;
	m_chunkAllocs = ca;
	return val;
}

inline void* hctFilterMemoryTracker::alignedAllocate(int alignment, int nbytes, HK_MEMORY_CLASS cl)
{
	unsigned char* val = hkAlignedAllocate<unsigned char>(alignment, nbytes, cl);
	MemoryAlloc* ma = hkAllocateChunk<MemoryAlloc>(1, HK_MEMORY_CLASS_EXPORT);
	ma->m_mem = val;
	ma->m_next = m_alignedAllocs;
	m_alignedAllocs = ma;
	return val;
}
	
inline void hctFilterMemoryTracker::deallocate(void* p)
{
	// stop tracking it
	MemoryAlloc* prev = HK_NULL; 
	MemoryAlloc* ma = m_allocs;
	while (ma)
	{
		if (ma->m_mem == p)
		{
			hkDeallocate(ma->m_mem);
			if (prev) prev->m_next = ma->m_next;
			if (m_allocs == ma) m_allocs = ma->m_next;
			hkDeallocateChunk(ma, 1, HK_MEMORY_CLASS_EXPORT);
			return;
		}
		prev = ma;
		ma = ma->m_next;
	}
}

inline void hctFilterMemoryTracker::alignedDeallocate(void* p)
{
	// stop tracking it
	MemoryAlloc* prev = HK_NULL; 
	MemoryAlloc* ma = m_alignedAllocs;
	while (ma)
	{
		if (ma->m_mem == p)
		{
			hkAlignedDeallocate(ma->m_mem);
			if (prev) prev->m_next = ma->m_next;
			if (m_alignedAllocs == ma) m_alignedAllocs = ma->m_next;
			hkDeallocateChunk(ma, 1, HK_MEMORY_CLASS_EXPORT);
			return;
		}
		prev = ma;
		ma = ma->m_next;
	}
}

inline void hctFilterMemoryTracker::deallocateChunk(void* p, int nbytes, HK_MEMORY_CLASS cl)
{
	// stop tracking it
	MemoryChunk* prev = HK_NULL; 
	MemoryChunk* ma = m_chunkAllocs;
	while (ma)
	{
		if (ma->m_mem == p)
		{
			hkDeallocateChunk(ma->m_mem, ma->m_memSize, ma->m_memClass);
			if (prev) prev->m_next = ma->m_next;
			if (m_chunkAllocs == ma) m_chunkAllocs = ma->m_next;
			hkDeallocateChunk(ma, 1, HK_MEMORY_CLASS_EXPORT);
			return;
		}
		prev = ma;
		ma = ma->m_next;
	}
}

	


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
