/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_HKSTRINGREF_H
#define HKBASE_HKSTRINGREF_H

#include <Common/Base/Types/hkBaseTypes.h>
#include <Common/Base/Memory/hkThreadMemory.h>
#include <Common/Base/Container/String/hkString.h>

	/// This class is used to reflect char* c-string.
	/// It automatically handles allocations and deallocation of
	/// stored c-string.
class hkStringPtr
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_BASE, hkStringPtr );

			/// Default constructor.
		HK_FORCE_INLINE hkStringPtr();

			/// Construct hkStrRef from const char*.
		HK_FORCE_INLINE hkStringPtr(const char* string);

			/// Copy constructor.
		HK_FORCE_INLINE hkStringPtr(const hkStringPtr& strRef);

			/// Return const pointer to stored string.
		HK_FORCE_INLINE const char* val() const;

			/// Destructor.
		~hkStringPtr();

			/// Assignment operator.
		HK_FORCE_INLINE hkStringPtr& operator=(const char* string);

			/// const index operator.
		HK_FORCE_INLINE const char& operator[](int idx) const;

			/// const char* const operator.
		HK_FORCE_INLINE operator const char*() const;

			/// Finish constructor.
		HK_FORCE_INLINE hkStringPtr(hkFinishLoadedObjectFlag f);

	public:

		enum StringFlags
		{
			HEAP_FLAG = 0x1,
			FLAGS_MASK = 0x1
		};

	private:
		char* m_string;
};

#include <Common/Base/Container/String/hkStringPtr.inl>

#endif // HKBASE_HKSTRINGREF_H

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
