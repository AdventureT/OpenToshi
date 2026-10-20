/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkArray<hkpCollidable*>& hkpAabbPhantom::getOverlappingCollidables()
{
	HK_WARN_ONCE_IF( m_orderDirty, 0x5fe59c18, "Phantom queried for collidables without call to ensureDeterministicOrder(). The result is potentially nondeterministic.");
	return m_overlappingCollidables;
}


inline const hkAabb& hkpAabbPhantom::getAabb( ) const
{
	return m_aabb;
}


inline hkpAabbPhantom::hkpAabbPhantom( class hkFinishLoadedObjectFlag flag ) 
	: hkpPhantom ( flag )
{
	if( flag.m_finishing )
	{
		ensureSpuAlignment();
	}
}


inline void hkpAabbPhantom::ensureSpuAlignment()
{
	const int capacity = m_overlappingCollidables.getCapacity(); 
	hkpCollidable** collidableArray = m_overlappingCollidables.begin();

	// Clear any unnecessary data
	if( capacity == 0 ||
		( (hkUlong) collidableArray & 0x0f ) != 0 ) 
	{
		const int newCapacity = hkMath::max2( 4, capacity + 1 );
		m_overlappingCollidables.reserveExactly( newCapacity );
	}

	// Check the new collidable array
	HK_ASSERT2( 0xdafe0001, ( ( (hkUlong) m_overlappingCollidables.begin() ) & 0x0f ) == 0, "Could not align collidables" );
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
