/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::construct(T* t, int n)
{
	for(int i = 0; i < n; ++i)
	{
		new (t+i) T;
	}
}

template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::construct(T* t, int n, const T& tcopy)
{
	for(int i = 0; i < n; ++i)
	{
		new (t+i) T(tcopy);
	}
}

template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::destruct(T* t, int n)
{
	for(int i = 0; i < n; ++i)
	{
		t[i].~T();
	}
}



template <typename T>
HK_FORCE_INLINE hkObjectArray<T>::hkObjectArray()
{
}

template <typename T>
HK_FORCE_INLINE hkObjectArray<T>::hkObjectArray( int initialSize )
	:	m_array(initialSize)
{
	construct( m_array.begin(), initialSize);
}

template <typename T>
HK_FORCE_INLINE hkObjectArray<T>::hkObjectArray( int initialSize, const T& tinit )
	: m_array( initialSize )
{
	construct( m_array.begin(), initialSize, tinit);
}

template <typename T>
HK_FORCE_INLINE hkObjectArray<T>::hkObjectArray( const hkObjectArray& a )
	:	m_array( a.getSize() )
{
	T* t = m_array.begin();
	for(int i = 0; i < a.getSize(); ++i)
	{
		new (t+i) T(a[i]);
	}
}

template <typename T>
HK_FORCE_INLINE hkObjectArray<T>& hkObjectArray<T>::operator= ( const hkObjectArray<T>& a )
{
	if( a.getSize() > m_array.getCapacity() )
	{
		// need to reallocate, destroy the entire array
		// and start from scratch
		destruct( m_array.begin(), m_array.getSize() );
		m_array.setSize(a.getSize());
		T* t = m_array.begin();
		for( int i = 0; i < a.getSize(); i++ )
		{
			new (t + i) T( a[i]);
		}
	}
	else if( a.getSize() > m_array.getSize() )
	{
		// have the capacity, other is bigger
		// copy as many as possible, construct the end ones.
		int i;
		for(i = 0; i < m_array.getSize(); ++i)
		{
			m_array[i] = a[i];
		}
		T* t = m_array.begin();
		for( ; i < a.getSize(); ++i)
		{
			new (t + i) T( a[i]);
		}
		m_array.setSizeUnchecked( a.getSize() );
	}
	else //( a.getSize() < m_array.getSize() )
	{
		// have the capacity, i am bigger
		// copy as many as possible, destroy the end ones.
		int i;
		for(i = 0; i < a.getSize(); ++i)
		{
			m_array[i] = a[i];
		}
		for( ; i < m_array.getSize(); ++i)
		{
			m_array[i].~T();
		}
		m_array.setSizeUnchecked( a.getSize() );
	}
	return *this;
}


template <typename T>
HK_FORCE_INLINE hkObjectArray<T>::~hkObjectArray()
{
	int size = m_array.getSize();
	for( int i = 0; i < size; ++i)
	{
		T& del = m_array[i];
		del.~T();
	}
}




template <typename T>
HK_FORCE_INLINE T& hkObjectArray<T>::operator[] (int i)
{
	HK_ASSERT(0x61001bf9, i<m_array.getSize());
	return m_array[i];
}

template <typename T>
HK_FORCE_INLINE const T& hkObjectArray<T>::operator[] (int i) const
{
	HK_ASSERT(0x4e99fe23, i<m_array.getSize());
	return m_array[i];
}

template <typename T>
HK_FORCE_INLINE T& hkObjectArray<T>::back()
{
	return m_array.back();
}

template <typename T>
HK_FORCE_INLINE const T& hkObjectArray<T>::back() const
{
	return m_array.back();
}

template <typename T>
HK_FORCE_INLINE int hkObjectArray<T>::getSize() const
{
	return m_array.getSize();
}

template <typename T>
HK_FORCE_INLINE int hkObjectArray<T>::getCapacity() const
{
	return m_array.getCapacity();
}

template <typename T>
HK_FORCE_INLINE hkBool hkObjectArray<T>::isEmpty() const
{
	return 0 == getSize();
}

template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::clear()
{
	destruct( m_array.begin(), m_array.getSize() );
	m_array.clear();
}

template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::clearAndDeallocate()
{
	destruct( m_array.begin(), m_array.getSize() );
	m_array.clearAndDeallocate();
}



template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::removeAt(int i)
{
	int s = m_array.getSize() - 1;
	m_array[i] = m_array[s];
	m_array[s].~T();
	m_array.popBack();
}

template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::removeAtAndCopy(int index)
{
	int s = m_array.getSize() - 1;
	for( int i = index; i < s; i++ )
	{
		m_array[i] = m_array[i+1];
	}
	m_array[s].~T();
	m_array.setSizeUnchecked(s);
}

template <typename T>
HK_FORCE_INLINE int hkObjectArray<T>::indexOf(const T& t) const
{
	return m_array.indexOf(t);
}

template <typename T>
HK_FORCE_INLINE int hkObjectArray<T>::lastIndexOf(const T& t) const
{
	return m_array.lastIndexOf(t);
}

template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::popBack()
{
	m_array[m_array.getSize()-1].~T();
	m_array.popBack();
}

template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::reserve( int minCapacity )
{
	if( minCapacity > m_array.getCapacity() )
	{
		int cap2 = 2 * m_array.getCapacity();
		int newCap = (minCapacity < cap2) ? cap2 : minCapacity;
		int oldSize = m_array.getSize();

		hkObjectArray old;
		old.swap(*this);

		m_array.reserve(newCap);
		T* p = m_array.begin();
		for( int i = 0; i < oldSize; ++i )
		{
			// place variable req'd for broken sn compilers
			T *place = p + i;
			new (place) T( old[i] );
		}
		m_array.setSizeUnchecked(oldSize);
	}
}


template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::pushBack( const T& t )
{
	int size = m_array.getSize();
	if( size == m_array.getCapacity() )
	{
		reserve( size ? size*2 : 1 );
	}
	m_array.setSizeUnchecked(size+1);
	new (&m_array[size]) T(t);	
}




template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::setSize( int n )
{
	int mysize = m_array.getSize();
	if( n < mysize )
	{
		for( int i = n; i < mysize; i++ )
		{
			m_array[i].~T();
		}
	}
	else
	{
		reserve( n );
		T* p = m_array.begin();
		for( int i = mysize; i < n; ++i )
		{
			// place variable req'd for broken sn compilers
			T *place = p + i;
			new (place) T();
		}
	}
	m_array.setSizeUnchecked( n );
}

template <typename T>
HK_FORCE_INLINE T& hkObjectArray<T>::expandOne( )
{
	return *expandBy(1);
}

template <typename T>
HK_FORCE_INLINE T* hkObjectArray<T>::expandBy( int n )
{
	int oldsize = m_array.getSize();
	setSize( oldsize + n );
	return m_array.begin() + oldsize;
}



template <typename T>
HK_FORCE_INLINE void hkObjectArray<T>::setSize( int n, const T& fill )
{
	
	int oldsize = m_array.getSize();
	setSize( n );
	if( oldsize < n )
	{
		for( int i = oldsize; i < n; i++ )
		{
			m_array[i] = fill;
		}
	}
}


template <typename T>
void hkObjectArray<T>::swap(hkObjectArray<T>& a)
{
	m_array.swap(a.m_array);
}
 

template <typename T>
typename hkObjectArray<T>::iterator hkObjectArray<T>::begin() 
{
	return m_array.begin();
}


template <typename T>
typename hkObjectArray<T>::iterator hkObjectArray<T>::end()
{
	return m_array.begin() + m_array.getSize();
}


template <typename T>
typename hkObjectArray<T>::const_iterator hkObjectArray<T>::begin() const
{
	return m_array.begin();
}


template <typename T>
typename hkObjectArray<T>::const_iterator hkObjectArray<T>::end() const
{
	return m_array.begin() + m_array.getSize();
}


template <typename T>
hkOstream& operator<< (hkOstream& os, const hkObjectArray<T>& a)
{
	os << "#[";
	for(int i=0; i < a.getSize(); ++i)
	{
		os << a[i] << ' ';
	}
	os << "]";
	return os;
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
