/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */




template <typename T>
HK_FORCE_INLINE T& hkArray<T>::operator[] (int i)
{
	HK_ASSERT(0x394e9c6c,  i >= 0 && i < m_size );
	return m_data[i];
}

template <typename T>
HK_FORCE_INLINE const T& hkArray<T>::operator[] (int i) const
{
	HK_ASSERT(0x264718f3,  i >= 0 && i < m_size  );
	return m_data[i];
}

template <typename T>
HK_FORCE_INLINE T& hkArray<T>::back()
{
	HK_ASSERT(0x52595f9b, m_size );
	return m_data[ m_size - 1 ];
}

template <typename T>
HK_FORCE_INLINE const T& hkArray<T>::back() const
{
	HK_ASSERT(0x6e984e36, m_size );
	return m_data[ m_size -1 ];
}

template <typename T>
HK_FORCE_INLINE int hkArray<T>::getSize() const
{
	return m_size;
}

template <typename T>
HK_FORCE_INLINE int hkArray<T>::getCapacity() const
{
	return (m_capacityAndFlags & static_cast<int>(CAPACITY_MASK));
}

template <typename T>
HK_FORCE_INLINE int hkArray<T>::getCapacityAndFlags() const
{
    return m_capacityAndFlags;
}

template <typename T>
HK_FORCE_INLINE hkBool hkArray<T>::isEmpty() const
{
	return m_size == 0;
}

template <typename T>
HK_FORCE_INLINE void hkArray<T>::clear()
{
	m_size = 0;
}



template <typename T>
HK_FORCE_INLINE void HK_CALL hkArray<T>::copy(T* dst, const T* src, int n)
{
	HK_ASSERT(0x4543e433, (	dst <= src) || (src+n <= dst) );
	for(int i = 0; i < n; ++i)
	{
		dst[i] = src[i];
	}
}

template <typename T>
HK_FORCE_INLINE void HK_CALL hkArray<T>::copyBackwards(T* dst, const T* src, int n)
{
	HK_ASSERT(0x4543e434, (dst >= src) || (dst+n <= src) );
	for(int i = n-1; i >= 0; --i)
	{
		dst[i] = src[i];
	}
}



template <typename T>
HK_FORCE_INLINE hkArray<T>::hkArray()
	:	m_data(HK_NULL),
		m_size(0),
		m_capacityAndFlags(DONT_DEALLOCATE_FLAG)
{
}

template <typename T>
HK_FORCE_INLINE hkArray<T>::hkArray(int n)
	:	m_data( static_cast<T*>(hkArrayAllocator.allocateChunk(n * hkSizeOf(T), HK_MEMORY_CLASS_ARRAY))),
		m_size(n),
		m_capacityAndFlags(n)
{
}

template <typename T>
HK_FORCE_INLINE hkArray<T>::hkArray(int n, const T& t)
	:	m_data( static_cast<T*>(hkArrayAllocator.allocateChunk(n * hkSizeOf(T), HK_MEMORY_CLASS_ARRAY))),
		m_size(n),
		m_capacityAndFlags(n)
{
	for(int i = 0; i < n; ++i)
	{
		m_data[i] = t;
	}
}

template <typename T>
HK_FORCE_INLINE hkArray<T>::hkArray(const hkArray& a)
{
	m_data = static_cast<T*>( hkArrayAllocator.allocateChunk( a.getSize() * hkSizeOf(T), HK_MEMORY_CLASS_ARRAY));
	m_size = a.m_size;
	m_capacityAndFlags = a.m_size;
	copy(m_data, a.m_data, a.m_size);
}

template <typename T>
HK_FORCE_INLINE void hkArray<T>::releaseMemory()
{
	if( (m_capacityAndFlags & DONT_DEALLOCATE_FLAG) == 0)
	{
		hkArrayAllocator.deallocateChunk( m_data, getCapacity() * hkSizeOf(T), HK_MEMORY_CLASS_ARRAY);
	}
}

template <typename T>
void hkArray<T>::useExternalArray(T* buf, int capacity)
{
	HK_ASSERT(0x32f5348d, (m_data==HK_NULL) || (m_capacityAndFlags&DONT_DEALLOCATE_FLAG) );
	m_data = static_cast<T*>(buf);
	m_size = 0;
	m_capacityAndFlags = capacity | DONT_DEALLOCATE_FLAG;
}

template <typename T>
void hkArray<T>::useExternalBuffer(void* buf, int numBytes)
{
	useExternalArray( static_cast<T*>(buf), numBytes/sizeof(T) );
}


template <typename T>
HK_FORCE_INLINE hkArray<T>& hkArray<T>::operator= (const hkArray<T>& a)
{
	if(getCapacity() < a.m_size)
	{
#ifdef HK_DEBUG
		if ( isLocked() )
			HK_WARN(0xf3768206, "hkArray::operator= to array of larger size on a locked array. Destructor (memory dealloc) will not be called.");
#endif
		releaseMemory();
		m_data = static_cast<T*>( hkArrayAllocator.allocateChunk( a.m_size * hkSizeOf(T), HK_MEMORY_CLASS_ARRAY));
		m_capacityAndFlags = a.m_size | (m_capacityAndFlags & static_cast<int>(LOCKED_FLAG)); // keep locked flag
	}
	m_size = a.m_size;
	copy(m_data, a.m_data, m_size);
	return *this;
}

template <typename T, unsigned N>
hkArray<T>& hkInplaceArray<T,N>::operator= (const hkArray<T>& a)
{
	return hkArray<T>::operator=(a);
}

template <typename T, unsigned N>
hkArray<T>& hkInplaceArray<T,N>::operator= (const hkInplaceArray<T,N>& a)
{
	return hkArray<T>::operator=(a);
}

template <typename T, unsigned N>
hkBool hkInplaceArray<T,N>::wasReallocated() const
{
	return this->m_data != m_storage;
}

template <typename T, unsigned N>
int hkInplaceArray<T,N>::stillInplaceUsingMask() const
{
	return hkArray<T>::m_capacityAndFlags & hkArray<T>::DONT_DEALLOCATE_FLAG;
}

template <typename T, unsigned N>
hkArray<T>& hkInplaceArrayAligned16<T,N>::operator= (const hkArray<T>& a)
{
	return hkArray<T>::operator=(a);
}

template <typename T, unsigned N>
hkBool hkInplaceArrayAligned16<T,N>::wasReallocated() const
{
	return hkUlong(this->m_data) != hkUlong(m_storage);
}

template <typename T, unsigned N>
hkArray<T>& hkInplaceArrayAligned16<T,N>::operator= (const hkInplaceArrayAligned16<T,N>& a)
{
	return hkArray<T>::operator=(a);
}

template <typename T>
HK_FORCE_INLINE hkArray<T>::hkArray(T* ptr, int size, int capacity)
	:	m_data(ptr),
		m_size(size),
		m_capacityAndFlags(capacity | DONT_DEALLOCATE_FLAG)
{
}

template <typename T>
HK_FORCE_INLINE hkArray<T>::~hkArray()
{
	releaseMemory();
}



template <typename T>
HK_FORCE_INLINE void hkArray<T>::clearAndDeallocate()
{
	releaseMemory();
	m_data = HK_NULL;
	m_size = 0;
	m_capacityAndFlags = DONT_DEALLOCATE_FLAG | (m_capacityAndFlags & LOCKED_FLAG);
}

#define HK_COMPUTE_OPTIMIZED_CAPACITY( size, numFreeElemsLeft, shrinkExact ) (shrinkExact) ? (size + numFreeElemsLeft) : hkNextPowerOf2(size + numFreeElemsLeft)

template <typename T>
HK_FORCE_INLINE void hkArray<T>::optimizeCapacity( int numFreeElemsLeft, hkBool32 shrinkExact )
{
	int totalCapacity = HK_COMPUTE_OPTIMIZED_CAPACITY(m_size, numFreeElemsLeft, shrinkExact);
	if ( totalCapacity <= getCapacity() )
	{
#ifdef HK_DEBUG
		if ( isLocked() )
			HK_WARN(0xf3768206, "hkArray::optimizeCapacity on locked array. Destructor (memory dealloc) will not be called.");
#endif
		hkArrayUtil::_reduce( this, hkSizeOf(T), HK_NULL, totalCapacity );
	}
}

template <typename T>
HK_FORCE_INLINE void hkArray<T>::removeAt(int i)
{
	HK_ASSERT(0x63bab20a,  i >= 0 && i < m_size);
	m_size--;
	m_data[i] = m_data[m_size];
}

template <typename T>
HK_FORCE_INLINE void hkArray<T>::removeAtAndCopy(int index)
{
	HK_ASSERT(0x453a6437,  index >= 0 && index < m_size);
	m_size--;
	for(int i = index; i < m_size; ++i)
	{
		m_data[i] = m_data[i+1];
	}
}

template <typename T>
HK_FORCE_INLINE void hkArray<T>::removeAtAndCopy(int index, int numToRemove)
{
	HK_ASSERT(0x453a6436,  numToRemove >= 0);
	HK_ASSERT(0x453a6437,  index >= 0 && ( (index + numToRemove) <= m_size) );
	
	m_size -= numToRemove;
	for(int i = index; i < m_size; ++i)
	{
		m_data[i] = m_data[i+numToRemove];
	}
}

template <typename T>
HK_FORCE_INLINE int hkArray<T>::indexOf(const T& t, int startIdx, int endIdx) const
{
	if( endIdx < 0 )
	{
		endIdx = m_size;
	}
	for(int i = startIdx; i < endIdx; ++i)
	{
		if( m_data[i] == t )
		{
			return i;
		}
	}
	return -1;
}

template <typename T>
HK_FORCE_INLINE int hkArray<T>::lastIndexOf(const T& t) const
{
	for(int i = m_size-1; i >=0; --i)
	{
		if( m_data[i] == t )
		{
			return i;
		}
	}
	return -1;
}

template <typename T>
HK_FORCE_INLINE void hkArray<T>::popBack( int numRemove )
{
	HK_ASSERT(0x5b57310e, m_size >= numRemove );
	m_size -= numRemove;
}


template <typename T>
HK_FORCE_INLINE hkArray<T> hkArray<T>::getSubarray(int b, int e)
{
	HK_ASSERT(0x3068c6ab,  b >= 0 && b <= e && e < m_size);
	return hkArray(m_data+b, e-b);
}


template <typename T>
HK_FORCE_INLINE void hkArray<T>::reserveExactly(int n)
{
	if(getCapacity() < n)
	{
#ifdef HK_DEBUG
		if ( isLocked() )
			HK_WARN(0xf3768206, "hkArray::reserveExactly on locked array. Destructor (memory dealloc) will not be called.");
#endif

		hkArrayUtil::_reserve(this, n, sizeof(T));
	}
}

template <typename T>
HK_FORCE_INLINE void hkArray<T>::reserve(int n)
{
	const int capacity = getCapacity();
	if( capacity < n)
	{
#ifdef HK_DEBUG
		if ( isLocked() )
			HK_WARN(0xf3768206, "hkArray::reserve on locked array. Destructor (memory dealloc) will not be called.");
#endif
		int cap2 = 2 * capacity;
		int newSize = (n < cap2) ? cap2 : n;
		hkArrayUtil::_reserve(this, newSize, sizeof(T));
	}
}

template <typename T>
HK_FORCE_INLINE void hkArray<T>::pushBack(const T& t)
{
	if(m_size == getCapacity())
	{
#ifdef HK_DEBUG
		if ( isLocked() )
			HK_WARN(0xf3768206, "hkArray::pushBack on locked array. Destructor (memory dealloc) will not be called.");
#endif
		HK_ASSERT2( 0x76e453e4, ! ( ( &t >= m_data ) && ( &t < (m_data + m_size) ) ), "hkArray::pushBack can't push back element of same array during resize" );
		hkArrayUtil::_reserveMore( this, sizeof(T) );
	}
	m_data[m_size++] = t;
}

template <typename T>
HK_FORCE_INLINE void hkArray<T>::pushBackUnchecked(const T& t)
{
	HK_ASSERT(0x3a2b4abb, m_size < getCapacity());
	m_data[m_size++] = t;
}


template <typename T>
HK_FORCE_INLINE void hkArray<T>::setSize(int n)
{
	reserve(n); // will warn if locked
	m_size = n;
}

template <typename T>
HK_FORCE_INLINE void hkArray<T>::setSizeUnchecked(int n)
{
	HK_ASSERT(0x39192e68, n <= getCapacity());
	m_size = n;
}

template <typename T>
HK_FORCE_INLINE T* hkArray<T>::expandBy( int n )
{
	int oldsize = m_size;
	setSize( oldsize + n ); // will warn if too big.
	return m_data+oldsize;
}

template <typename T>
HK_FORCE_INLINE T& hkArray<T>::expandOne( )
{
	if(m_size == getCapacity())
	{
#ifdef HK_DEBUG
		if ( isLocked() )
			HK_WARN(0xf3768206, "hkArray::expandOne on locked array. Destructor (memory dealloc) will not be called.");
#endif
		hkArrayUtil::_reserveMore( this, sizeof(T) );
	}
	return m_data[m_size++];
}


template <typename T>
HK_FORCE_INLINE T* hkArray<T>::expandByUnchecked( int n )
{
	int oldsize = m_size;
	setSizeUnchecked( oldsize + n ); // will warn if locked.
	return m_data+oldsize;
}

template <typename T>
HK_FORCE_INLINE void hkArray<T>::setSize(int n, const T& fill)
{
	if(n > m_size)
	{
		int oldsize = m_size;
		reserve(n); // will warn if locked
		for(int i = oldsize; i < n; ++i)
		{
			m_data[i] = fill;
		}
	}
	m_size = n;
}

template <typename T>
void hkArray<T>::spliceInto(int index, int numdel, const T* p, int numtoinsert)
{
	HK_ASSERT(0x4cbc67c6, index >= 0 && index <= getSize() );
	HK_ASSERT(0x4cbc67c7, numdel >= 0 && (index+numdel) <= getSize() );
	const int newsize     = numtoinsert + m_size - numdel;
	const int numtomove   = m_size - index - numdel;
	if(newsize > getCapacity())
	{
		// note double copy from [i:end] not a problem in practice
		reserve(newsize); // will warn if locked.
	}
	if( numtoinsert >= numdel )
	{
		copyBackwards(m_data + index + numtoinsert, m_data + index + numdel, numtomove);
	}
	else
	{
		copy( m_data + index + numtoinsert, m_data + index + numdel, numtomove );
	}
	copyBackwards(m_data + index, p, numtoinsert);
	m_size = newsize;
}

template <typename T>
void hkArray<T>::insertAt(int index, const T* p, int numtoinsert)
{
	spliceInto(index, 0, p, numtoinsert);
}



template <typename T>
HK_FORCE_INLINE T* hkArray<T>::expandAt(int index, int numtoinsert)
{
	HK_ASSERT(0x2723cc08,  index >= 0 && index <= m_size );

	const int newsize = numtoinsert + m_size;
	const int numtomove = m_size - index;
	if(newsize > getCapacity())
	{
		// note double copy from [i:end] not a problem in practice
		reserve(newsize); // will warn if locked.
	}
	copyBackwards(m_data + index + numtoinsert, m_data + index, numtomove);
	m_size = newsize;
	return m_data + index;
}



template <typename T>
void hkArray<T>::insertAt(int i, const T& t)
{
	insertAt(i, &t, 1 );
}

template <typename T>
void hkArray<T>::swap(hkArray<T>& a)
{
	HK_ASSERT( 0xf032e612, 0==(m_capacityAndFlags&DONT_DEALLOCATE_FLAG) || m_data == HK_NULL );
	HK_ASSERT( 0xf032e613, 0==(a.m_capacityAndFlags&DONT_DEALLOCATE_FLAG) || a.m_data == HK_NULL );

	T* d = m_data; // swap data
	m_data = a.m_data;
	a.m_data = d;
	int s = m_size; // swap size
	m_size = a.m_size;
	a.m_size = s;
	int c = m_capacityAndFlags; // swap cap
	m_capacityAndFlags = a.m_capacityAndFlags;
	a.m_capacityAndFlags = c;
}

//
// Inplace array
//

template <typename T, unsigned N>
HK_FORCE_INLINE hkInplaceArray<T,N>::hkInplaceArray(int size)
	: hkArray<T>(m_storage, size, N)
{
}

template <typename T, unsigned N>
HK_FORCE_INLINE hkInplaceArray<T,N>::hkInplaceArray(const hkInplaceArray<T,N>& a)
	: hkArray<T>(m_storage, 0, N)
{
	*this = a;
}

template <typename T, unsigned N>
HK_FORCE_INLINE void hkInplaceArray<T,N>::optimizeCapacity( int numFreeElemsLeft, hkBool32 shrinkExact )
{
	if( (this->m_capacityAndFlags & hkArray<T>::DONT_DEALLOCATE_FLAG) == 0)
	{
		int totalCapacity = HK_COMPUTE_OPTIMIZED_CAPACITY(this->m_size, numFreeElemsLeft, shrinkExact);
		if( totalCapacity < int(N) )
		{
#ifdef HK_DEBUG
			if ( this->isLocked() )
				HK_WARN(0xf3768206, "hkInplaceArray::optimizeCapacity on locked array. Destructor (memory dealloc) will not be called.");
#endif
			// reducing to a capacity < N, we can copy back to local storage
			hkArrayUtil::_reduce( this, hkSizeOf(T), (char*)(&m_storage[0]), N );
		}
		else if( totalCapacity < this->getCapacity() )
		{
#ifdef HK_DEBUG
			if ( this->isLocked() )
				HK_WARN(0xf3768206, "hkArray::optimizeCapacity on locked array. Destructor (memory dealloc) will not be called.");
#endif
			hkArrayUtil::_reduce( this, hkSizeOf(T), HK_NULL, totalCapacity );
		}
	}
}



template <typename T, unsigned N>
HK_FORCE_INLINE hkInplaceArrayAligned16<T,N>::hkInplaceArrayAligned16(int size)
	: hkArray<T>( (T*)&m_storage[0], size, N)
{
}


template <typename T>
typename hkArray<T>::iterator hkArray<T>::begin()
{
	return m_data;
}


template <typename T>
typename hkArray<T>::iterator hkArray<T>::end()
{
	return m_data + m_size;
}


template <typename T>
typename hkArray<T>::const_iterator hkArray<T>::begin() const
{
	return m_data;
}


template <typename T>
typename hkArray<T>::const_iterator hkArray<T>::end() const
{
	return m_data + m_size;
}

template <typename T>
hkBool hkArray<T>::isLocked()
{
	return (m_capacityAndFlags & LOCKED_FLAG) != 0;
}

template <typename T>
void hkArray<T>::setLocked( bool locked )
{
	if (locked)
		m_capacityAndFlags |= LOCKED_FLAG;	// turn on
	else
		m_capacityAndFlags &= ~LOCKED_FLAG;	// turn off
}

template <typename T>
void hkArray<T>::setOwnedData(T* ptr, int size, int capacity)
{
	m_data = ptr;
	m_size = size;
	m_capacityAndFlags = capacity;
}

#undef HK_COMPUTE_OPTIMIZED_CAPACITY

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
