/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


HK_FORCE_INLINE void hkStringPtrArray::construct(hkStringPtr* t, int n)
{
	for(int i = 0; i < n; ++i)
	{
		new (t+i) hkStringPtr;
	}
}

HK_FORCE_INLINE void hkStringPtrArray::construct(hkStringPtr* t, int n, const hkStringPtr& tcopy)
{
	for(int i = 0; i < n; ++i)
	{
		new (t+i) hkStringPtr(tcopy);
	}
}

HK_FORCE_INLINE void hkStringPtrArray::destruct(hkStringPtr* t, int n)
{
	for(int i = 0; i < n; ++i)
	{
		t[i].~hkStringPtr();
	}
}



HK_FORCE_INLINE hkStringPtrArray::hkStringPtrArray()
{
}

HK_FORCE_INLINE hkStringPtrArray::hkStringPtrArray( int initialSize )
	:	m_array(initialSize)
{
	construct( m_array.begin(), initialSize);
}

HK_FORCE_INLINE hkStringPtrArray::hkStringPtrArray( hkFinishLoadedObjectFlag f )
	: m_array(f)
{
}

HK_FORCE_INLINE hkStringPtrArray::hkStringPtrArray( int initialSize, const hkStringPtr& tinit )
	: m_array( initialSize )
{
	construct( m_array.begin(), initialSize, tinit);
}

HK_FORCE_INLINE hkStringPtrArray::hkStringPtrArray( const hkStringPtrArray& a )
	:	m_array( a.getSize() )
{
	hkStringPtr* t = m_array.begin();
	for(int i = 0; i < a.getSize(); ++i)
	{
		new (t+i) hkStringPtr(a[i]);
	}
}

HK_FORCE_INLINE hkStringPtrArray& hkStringPtrArray::operator= ( const hkStringPtrArray& a )
{
	if( a.getSize() > m_array.getCapacity() )
	{
		// need to reallocate, destroy the entire array
		// and start from scratch
		destruct( m_array.begin(), m_array.getSize() );
		m_array.setSize(a.getSize());
		hkStringPtr* t = m_array.begin();
		for( int i = 0; i < a.getSize(); i++ )
		{
			new (t + i) hkStringPtr( a[i]);
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
		hkStringPtr* t = m_array.begin();
		for( ; i < a.getSize(); ++i)
		{
			new (t + i) hkStringPtr( a[i]);
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
			m_array[i].~hkStringPtr();
		}
		m_array.setSizeUnchecked( a.getSize() );
	}
	return *this;
}

HK_FORCE_INLINE hkStringPtrArray::~hkStringPtrArray()
{
	int size = m_array.getSize();
	for( int i = 0; i < size; ++i)
	{
		hkStringPtr& del = m_array[i];
		del.~hkStringPtr();
	}
}



HK_FORCE_INLINE hkStringPtr& hkStringPtrArray::operator[] (int i)
{
	HK_ASSERT(0x61001bf9, i<m_array.getSize());
	return m_array[i];
}

HK_FORCE_INLINE const hkStringPtr& hkStringPtrArray::operator[] (int i) const
{
	HK_ASSERT(0x4e99fe23, i<m_array.getSize());
	return m_array[i];
}

HK_FORCE_INLINE hkStringPtr& hkStringPtrArray::back()
{
	return m_array.back();
}

HK_FORCE_INLINE const hkStringPtr& hkStringPtrArray::back() const
{
	return m_array.back();
}

HK_FORCE_INLINE int hkStringPtrArray::getSize() const
{
	return m_array.getSize();
}

HK_FORCE_INLINE int hkStringPtrArray::getCapacity() const
{
	return m_array.getCapacity();
}

HK_FORCE_INLINE hkBool hkStringPtrArray::isEmpty() const
{
	return 0 == getSize();
}

HK_FORCE_INLINE void hkStringPtrArray::clear()
{
	destruct( m_array.begin(), m_array.getSize() );
	m_array.clear();
}

HK_FORCE_INLINE void hkStringPtrArray::clearAndDeallocate()
{
	destruct( m_array.begin(), m_array.getSize() );
	m_array.clearAndDeallocate();
}

HK_FORCE_INLINE void hkStringPtrArray::removeAt(int index)
{
	m_array[index].~hkStringPtr();
	int s = m_array.getSize() - 1;
	hkString::memCpy(hkAddByteOffset(m_array.begin(), hkSizeOf(hkStringPtr)*index),
						hkAddByteOffset(m_array.begin(), hkSizeOf(hkStringPtr)*(s)),
						hkSizeOf(hkStringPtr));
	m_array.setSizeUnchecked(s);
}

HK_FORCE_INLINE void hkStringPtrArray::removeAtAndCopy(int index)
{
	m_array[index].~hkStringPtr();
	int s = m_array.getSize() - 1;
	hkString::memCpy(hkAddByteOffset(m_array.begin(), hkSizeOf(hkStringPtr)*index),
						hkAddByteOffset(m_array.begin(), hkSizeOf(hkStringPtr)*(index+1)),
						(s-index)*hkSizeOf(hkStringPtr));
	m_array.setSizeUnchecked(s);
}

HK_FORCE_INLINE int hkStringPtrArray::indexOf(const hkStringPtr& t) const
{
	return m_array.indexOf(t);
}

HK_FORCE_INLINE int hkStringPtrArray::lastIndexOf(const hkStringPtr& t) const
{
	return m_array.lastIndexOf(t);
}

HK_FORCE_INLINE void hkStringPtrArray::popBack()
{
	m_array[m_array.getSize()-1].~hkStringPtr();
	m_array.popBack();
}

HK_FORCE_INLINE void hkStringPtrArray::reserve( int minCapacity )
{
	if( minCapacity > m_array.getCapacity() )
	{
		int cap2 = 2 * m_array.getCapacity();
		int newCap = (minCapacity < cap2) ? cap2 : minCapacity;
		int oldSize = m_array.getSize();

		hkStringPtrArray old;
		old.swap(*this);

		m_array.reserve(newCap);
		hkString::memCpy(m_array.begin(), old.begin(), old.getSize()*hkSizeOf(hkStringPtr));
		m_array.setSizeUnchecked(oldSize);
		old.m_array.setSizeUnchecked(0);
	}
}

HK_FORCE_INLINE void hkStringPtrArray::pushBack( const hkStringPtr& t )
{
	int size = m_array.getSize();
	if( size == m_array.getCapacity() )
	{
		reserve( size ? size*2 : 1 );
	}
	m_array.setSizeUnchecked(size+1);
	new (&m_array[size]) hkStringPtr(t);
}

HK_FORCE_INLINE void hkStringPtrArray::setSize( int n )
{
	int mysize = m_array.getSize();
	if( n < mysize )
	{
		for( int i = n; i < mysize; i++ )
		{
			m_array[i].~hkStringPtr();
		}
	}
	else
	{
		reserve( n );
		hkStringPtr* p = m_array.begin();
		for( int i = mysize; i < n; ++i )
		{
			// place variable req'd for broken sn compilers
			hkStringPtr *place = p + i;
			new (place) hkStringPtr();
		}
	}
	m_array.setSizeUnchecked( n );
}

HK_FORCE_INLINE hkStringPtr& hkStringPtrArray::expandOne( )
{
	return *expandBy(1);
}

HK_FORCE_INLINE hkStringPtr* hkStringPtrArray::expandBy( int n )
{
	int oldsize = m_array.getSize();
	setSize( oldsize + n );
	return m_array.begin() + oldsize;
}

HK_FORCE_INLINE void hkStringPtrArray::setSize( int n, const hkStringPtr& fill )
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

HK_FORCE_INLINE void hkStringPtrArray::swap(hkStringPtrArray& a)
{
	m_array.swap(a.m_array);
}

// Iterators

HK_FORCE_INLINE hkStringPtrArray::iterator hkStringPtrArray::begin() 
{
	return m_array.begin();
}

HK_FORCE_INLINE hkStringPtrArray::iterator hkStringPtrArray::end()
{
	return m_array.begin() + m_array.getSize();
}

HK_FORCE_INLINE hkStringPtrArray::const_iterator hkStringPtrArray::begin() const
{
	return m_array.begin();
}

HK_FORCE_INLINE hkStringPtrArray::const_iterator hkStringPtrArray::end() const
{
	return m_array.begin() + m_array.getSize();
}

HK_FORCE_INLINE hkOstream& operator<< (hkOstream& os, const hkStringPtrArray& a)
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
