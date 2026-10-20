/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_MALLOC_MEMORY
#define HK_MALLOC_MEMORY

#include <Common/Base/Memory/Memory/hkMemory.h>

/// Simple manager which forwards all calls to malloc and free.
class hkMallocMemory : public hkMemory
{
	public:

		virtual void allocateChunkBatch(void** blocksOut, int nblocks, int nbytes)
		{
			for( int i = 0; i < nblocks; ++i )
			{
				blocksOut[i] = allocateChunk(nbytes, HK_MEMORY_CLASS_ROOT);
			}
		}

		virtual void deallocateChunkBatch(void** blocksOut, int nblocks, int nbytes)
		{
			for( int i = 0; i < nblocks; ++i )
			{
				deallocateChunk(blocksOut[i], nbytes, HK_MEMORY_CLASS_ROOT);
			}
		}

		virtual void* allocateChunk(int nbytes, HK_MEMORY_CLASS cl)
		{
			return hkSystemMalloc(nbytes, 16);
		}

		virtual void deallocateChunk(void* p, int nbytes, HK_MEMORY_CLASS )
		{
			if(p)
			{
				hkSystemFree(p);
			}
		}

		virtual void printStatistics(hkOstream* c)
		{
		}
		
		virtual void calculateStatistics(hkMemoryStatistics& stats )
		{
		}

		virtual void preAllocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl)
		{
			HK_WARN_ALWAYS(0xaf55ad00, "Not implemented for hkMallocMemory. Doing nothing.");
		}

		virtual void* allocateRuntimeBlock(int nbytes, HK_MEMORY_CLASS cl)
		{
			HK_WARN_ALWAYS(0xaf55ad01, "Not implemented for hkMallocMemory. Forwarding to OS.");
			void* memory = allocateChunk(nbytes, cl);
			return memory;
		}

		virtual void deallocateRuntimeBlock(void* p, int nbytes, HK_MEMORY_CLASS cl)
		{
			HK_WARN_ALWAYS(0xaf55ad02, "Not implemented for hkMallocMemory. Forwarding to OS.");
			deallocateChunk(p, nbytes, cl);
		}

		virtual void provideRuntimeBlock(void*, int nbytes, HK_MEMORY_CLASS cl)
		{
			HK_WARN_ALWAYS(0xaf55ad03, "Not implemented for hkMallocMemory. Doing nothing.");
		}

		virtual void freeRuntimeBlocks()
		{
			HK_WARN_ALWAYS(0xaf55ad03, "Not implemented for hkMallocMemory. Doing nothing.");
		}

};

#endif // HK_MALLOC_MEMORY


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
