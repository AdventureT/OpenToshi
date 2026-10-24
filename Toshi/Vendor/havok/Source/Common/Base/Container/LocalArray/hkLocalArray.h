/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_HKLOCALARRAY_H
#define HKBASE_HKLOCALARRAY_H

/// A array class which uses hkMemory stack based allocations.
/// Stack allocation patterns can be much faster than heap allocations.
/// See the hkMemory/Frame based allocation user guide section.
/// When an hkLocalArray grows beyond its original specified capacity, it
/// falls back on heap allocations so it is important to specify a good
/// initial capacity.
/// hkLocalArray should almost always be a local variable in a
/// method and almost never a member of an object.
template <typename T>
class hkLocalArray : public hkArray<T>
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ARRAY, hkLocalArray<T>);

			/// Create an array with zero size and given capacity.
			/// For maximum efficiency, the arrays capacity should never grow
			/// beyond the specified capacity.  If it does, the array falls back
			/// to using heap allocations which is safe, but the speed advantage
			/// of using the stack based allocation methods is lost.
		HK_FORCE_INLINE hkLocalArray( int capacity )
		{
			this->m_data = hkAllocateStack<T>(capacity);
			this->m_capacityAndFlags = capacity | hkArray<T>::DONT_DEALLOCATE_FLAG;
			m_localMemory = this->m_data;
		}

			/// Discard the current contents and resize to the given size. Memory is uninitialised.
		HK_FORCE_INLINE void discardAndResize(int newsize)
		{
			HK_ASSERT2(0x2cd92d9d, m_localMemory == this->m_data, "You cannot call discardAndResize on a localarray that has grown beyond its original specified capacity.");
			hkDeallocateStack<T>( m_localMemory );
			m_localMemory = this->m_data = hkAllocateStack<T>(newsize);
			this->m_size = newsize;
			this->m_capacityAndFlags = newsize | hkArray<T>::DONT_DEALLOCATE_FLAG;
		}

		void assertNotResized()
		{
			HK_ASSERT2(0x5f792e08, m_localMemory == this->m_data, "A localarray grew beyond its original specified capacity.");
		}

		hkBool wasReallocated() const
		{
			return m_localMemory != this->m_data;
		}

			/// Destroy the array.
		HK_FORCE_INLINE ~hkLocalArray()
		{
			hkDeallocateStack<T>( m_localMemory );
		}

		T* m_localMemory;
};

#endif // HKBASE_HKLOCALARRAY_H

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
