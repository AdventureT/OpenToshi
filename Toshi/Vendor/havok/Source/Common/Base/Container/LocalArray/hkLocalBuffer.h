/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#if !defined(HKBASE_HK_LOCAL_BUFFER_H) 
#define HKBASE_HK_LOCAL_BUFFER_H


template <typename T>
class hkFixedArray
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ARRAY, hkFixedArray<T>);

			/// Read/write access to the i'th element.
		HK_FORCE_INLINE T& operator[] (int i);

			/// Read only access to the i'th element.
		HK_FORCE_INLINE const T& operator[] (int i) const;

			/// Returns an STL-like iterator to the first element.
		HK_FORCE_INLINE T* begin();

			/// Returns an STL-like iterator to the 'one past the last' element.
		HK_FORCE_INLINE const T* begin() const;

#		if defined(HK_DEBUG)
		HK_FORCE_INLINE int getSizeDebug() const { return m_debugSize; }
#		endif

	protected:
		HK_FORCE_INLINE	hkFixedArray(){}

		hkPadSpu<T*> m_data;
		int m_debugSize;	// only set in debug mode
};


/// A buffer going to stack memory.
/// Similar to C-style arrays but with bounds checking.
template <typename T>
class hkLocalBuffer: public hkFixedArray<T>
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ARRAY, hkLocalBuffer<T>);

			/// Creates a buffer of size n. All elements are uninitialized.
		HK_FORCE_INLINE hkLocalBuffer(int n, const char* what = 0 );

			/// Deallocates array memory.
		HK_FORCE_INLINE ~hkLocalBuffer();
};

template <typename T>
HK_FORCE_INLINE hkLocalBuffer<T>::hkLocalBuffer( int capacity, const char* what )
{
	this->m_data = hkAllocateStack<T>(capacity);
	HK_ON_DEBUG( this->m_debugSize = capacity );
}

template <typename T>
HK_FORCE_INLINE hkLocalBuffer<T>::~hkLocalBuffer()
{
	hkDeallocateStack( this->m_data.val() );
}


template <typename T>
HK_FORCE_INLINE T& hkFixedArray<T>::operator[] (int i)
{
	HK_ASSERT(0x394e9c6c,  i >= 0 && i < m_debugSize );
	return m_data[i];
}

template <typename T>
HK_FORCE_INLINE const T& hkFixedArray<T>::operator[] (int i) const
{
	HK_ASSERT(0x264718f3,  i >= 0 && i < m_debugSize  );
	return m_data[i];
}

template <typename T>
T* hkFixedArray<T>::begin() 
{
	return m_data;
}

template <typename T>
const T* hkFixedArray<T>::begin() const
{
	return m_data;
}

#endif // HKBASE_HK_LOCAL_BUFFER_H


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
