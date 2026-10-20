/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

template <typename T>
inline hkTree<T>::hkTree(destructFunc destruct)
	: m_destruct(destruct), m_tree( sizeof(T) )
{
}

template <typename T>
inline hkTree<T>::~hkTree()
{
	clear();
}

template <typename T>
inline void hkTree<T>::clear()
{
	Iter i = m_tree.iterGetRoot();
	while( i != HK_NULL )
	{
		i = m_tree.remove( i, m_destruct);
	}
}

template <typename T>
inline typename hkTree<T>::Iter hkTree<T>::append(Iter i, const T& t)
{
	return m_tree.append(i, &t, sizeof(T));
}

template <typename T>
inline typename hkTree<T>::Iter hkTree<T>::remove(Iter i)
{
	return m_tree.remove(i, m_destruct);
}

template <typename T>
inline void hkTree<T>::setValue(Iter i, const T& t )
{
	m_tree.setValue(i, &t, sizeof(T));
}

template <typename T>
inline const T& hkTree<T>::getValue(Iter i) const
{
	return *static_cast<const T*>( m_tree.getValue(i) );
}

template <typename T>
inline int hkTree<T>::getDepth(Iter i) const
{
	return m_tree.getDepth(i);
}

template <typename T>
inline int hkTree<T>::getNumChildren(Iter i) const
{
	return m_tree.getNumChildren(i);
}

template <typename T>
inline typename hkTree<T>::Iter hkTree<T>::iterGetRoot() const
{
	return m_tree.iterGetRoot();
}

template <typename T>
inline typename hkTree<T>::Iter hkTree<T>::iterNextPreOrder(Iter i) const
{
	return m_tree.iterNextPreOrder(i);
}

template <typename T>
inline typename hkTree<T>::Iter hkTree<T>::iterNext(Iter i) const
{
	return m_tree.iterNext(i);
}

template <typename T>
inline typename hkTree<T>::Iter hkTree<T>::iterParent(Iter i) const
{
	return m_tree.iterParent(i);
}

template <typename T>
inline typename hkTree<T>::Iter hkTree<T>::iterChildren(Iter i) const
{
	return m_tree.iterChildren(i);
}

template <typename T>
inline void hkTree<T>::defaultDestruct(void* p)
{
	static_cast<T*>(p)->~T();
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
