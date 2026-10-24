/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
template <typename KEY, typename VALUE>
int hkPointerMultiMap<KEY,VALUE>::getFreeIndex()
{
	int i;
	if( m_freeChainStart != -1 )
	{
		i = m_freeChainStart;
		m_freeChainStart = m_valueChain[m_freeChainStart].next;
	}
	else
	{
		i = m_valueChain.getSize();
		m_valueChain.expandOne();
	}
	return i;
}

template <typename KEY, typename VALUE>
int hkPointerMultiMap<KEY,VALUE>::getFirstIndex( KEY k ) const
{
	return m_indexMap.getWithDefault(k, -1);
}

template <typename KEY, typename VALUE>
hkBool32 hkPointerMultiMap<KEY,VALUE>::insert( KEY k, const VALUE& value )
{
	int chainIndex = m_indexMap.getWithDefault(k, -1);
	int thisIndex = getFreeIndex();
	Value& ref = m_valueChain[thisIndex];
	ref.value = value;
	ref.next = chainIndex;
	return m_indexMap.insert( k, thisIndex );
}

template <typename KEY, typename VALUE>
int hkPointerMultiMap<KEY,VALUE>::addPendingValue( const VALUE& value, int nextIndex )
{
	int thisIndex = getFreeIndex();
	Value& v = m_valueChain[thisIndex];
	v.value = value;
	v.next = nextIndex;
	return thisIndex;
}

template <typename KEY, typename VALUE>
void hkPointerMultiMap<KEY,VALUE>::realizePendingKey( KEY k, int index )
{
	HK_ASSERT(0x66e29b43, m_indexMap.hasKey(k) == false );
	HK_ASSERT(0x71031385, m_valueChain[index].next || 1 ); // assert index ok
	m_indexMap.insert(k, index);
}

template <typename KEY, typename VALUE>
int hkPointerMultiMap<KEY,VALUE>::getNumKeys() const
{
	return m_indexMap.getSize();
}

template <typename KEY, typename VALUE>
void hkPointerMultiMap<KEY,VALUE>::clear()
{
	m_valueChain.clear();
	m_indexMap.clear();
	m_freeChainStart = -1;
}

template <typename KEY, typename VALUE>
int hkPointerMultiMap<KEY,VALUE>::removeByIndex( KEY key, int removeIndex )
{
#ifdef HK_DEBUG
	{
		int i = m_indexMap.getWithDefault( key, -2 );
		HK_ASSERT2(0, i != -2, "key is not in map");
		while( i != -1 && i != removeIndex )
		{
			i = m_valueChain[i].next;
		}
		HK_ASSERT2(0, i != -1, "value is not accessible from key");
	}
#endif
	// To remove from singly linked list, overwrite
	// current with next and actually remove next.
	// Thus we avoid linear search for parent pointer.
	int nextIndex = m_valueChain[removeIndex].next; // retIndex == index to return
	int freeIndex = nextIndex; // freeIndex == index to add to free list at end
	if( nextIndex != -1 )
	{
		m_valueChain[removeIndex] = m_valueChain[nextIndex];
		nextIndex = removeIndex;
	}
	else // we are the tail element in chain
	{
		hkPointerMap<void*, int>::Iterator it = m_indexMap.findKey(key);
		int headIndex = m_indexMap.getValue(it);
		if( headIndex == removeIndex )
		{
			// head == tail. list empty after removal
			m_indexMap.setValue(it, -1);
			freeIndex = removeIndex;
			removeIndex = -1;
		}
		else if( m_valueChain[headIndex].next == removeIndex )
		{
			// head.next == tail. only one element (head) left after removal, we can take a shortcut
			m_valueChain[headIndex].next = -1;
			freeIndex = removeIndex;
		}
		else
		{
			// >=1 elements between head and tail. head moves to tail.
			m_indexMap.setValue(it, m_valueChain[headIndex].next);
			m_valueChain[removeIndex].value = m_valueChain[headIndex].value;
			freeIndex = headIndex;
		}
	}
	// add the freed index to the free list
	m_valueChain[freeIndex].next = m_freeChainStart;
	m_freeChainStart = freeIndex;
	return nextIndex;
}

template <typename KEY, typename VALUE>
void hkPointerMultiMap<KEY,VALUE>::removeByValue( KEY key, const VALUE& v )
{
	for( int i = getFirstIndex(key);
			i != -1;
			i = getNextIndex(i) )
	{
		if( getValue(i) == v )
		{
			removeByIndex(key, i);
			return;
		}
	}
	HK_ASSERT2(0x72ca9f58, 0, "value not in map");
}

template <typename KEY, typename VALUE>
void hkPointerMultiMap<KEY,VALUE>::removeKey( KEY key )
{
	hkPointerMap<void*, int>::Iterator it = m_indexMap.findKey(key);
	HK_ASSERT(0x17f621ad, m_indexMap.isValid(it) );
	int start = m_indexMap.getValue(it);
	m_indexMap.remove(it);
	if( start != -1 )
	{
		int end = start;
		while( m_valueChain[end].next != -1 )
		{
			end = m_valueChain[end].next;
		}
		m_valueChain[end].next = m_freeChainStart;
		m_freeChainStart = start;
	}
}

template <typename KEY, typename VALUE>
void hkPointerMultiMap<KEY,VALUE>::changeKey( KEY oldKey, KEY newKey )
{
	hkPointerMap<void*, int>::Iterator it = m_indexMap.findKey(oldKey);
	HK_ASSERT(0x17f621ad, m_indexMap.isValid(it) );
	int start = m_indexMap.getValue(it);
	m_indexMap.remove(it);
	HK_ASSERT(0x6ebd7acf, m_indexMap.hasKey(newKey) == false);
	m_indexMap.insert( newKey, start );
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
