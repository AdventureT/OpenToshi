/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SPU_STACK_H
#define HK_SPU_STACK_H


extern struct hkSpuStack g_spuStackInstance;

#if defined (HK_PLATFORM_SIM)
#	include <Common/Base/Container/String/hkString.h>
#endif


/// A simple stack implementation to be used on spu only.
///
/// This stack implementation simply uses a chunk of memory passed in by the user and increases/decreases
/// its internal memory pointer on each allocation/deallocation.
struct hkSpuStack
{
	public:
#if !defined(HK_PLATFORM_PS3_SPU)
		HK_FORCE_INLINE hkSpuStack();

		HK_FORCE_INLINE ~hkSpuStack();
#endif

		HK_FORCE_INLINE void initMemory(void* p, int size);

		static HK_FORCE_INLINE hkSpuStack& getInstance() { return g_spuStackInstance; }

		HK_FORCE_INLINE void* allocateStackRoundSizeTo128(int numBytes, const char* what);

		// This is the same as allocateStackRoundSizeTo128 but with a critical assert that the free space is sufficient
		HK_FORCE_INLINE void* allocateStackRoundSizeTo128WithCriticalAssert(int numBytes, const char* what);

		HK_FORCE_INLINE void* allocateStack(int numBytes, const char* what);

		HK_FORCE_INLINE void shrinkAllocatedStack(void* p, int newSize );

		HK_FORCE_INLINE void  deallocateStack(void* p);

		HK_FORCE_INLINE void  deallocateStack(int numBytes);

		HK_FORCE_INLINE int getFreeStackSize() { return int(hkGetByteOffset(m_next, m_end)); }

		HK_FORCE_INLINE const void* getStackEnd() { return m_end; }

		HK_FORCE_INLINE const void* getStackNext() { return m_next; }

#if defined (HK_PLATFORM_SIM)
		HK_FORCE_INLINE int getUsedStackSize() { return (m_maxStackSize - getFreeStackSize()); }
#endif

	protected:
			// pointer to the next free area on stack
		hkPadSpu<void*> m_next;

		hkPadSpu<void*> m_end;

#if defined (HK_PLATFORM_SIM)
#	define HK_SIMPLE_STACK_MAX_NUM_ALLOC_INFOS 64
	protected:
		struct AllocInfo
		{
			void* m_p;
			int   m_size;
			char m_what[64-sizeof(void*)-sizeof(int)];
		};

	protected:
		int m_numAllocInfos;
		AllocInfo m_allocInfos[HK_SIMPLE_STACK_MAX_NUM_ALLOC_INFOS];
		int m_maxStackSize;
		static int m_numBytesAllocatedHighMark;
#endif
};



#include <Common/Base/Memory/PlatformUtils/Spu/SpuStack/hkSpuStack.inl>

#endif // HK_SPU_STACK_H

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090704)
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
