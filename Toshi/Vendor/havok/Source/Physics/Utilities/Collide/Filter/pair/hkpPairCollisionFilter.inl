/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


void hkpPairCollisionFilter::calcKey(const hkpEntity* entityA, const hkpEntity* entityB, PairFilterKey& keyOut) const
{
	// expand any pointer size to 64 bit
	hkUint64 ptrA = hkUint64(entityA);
	hkUint64 ptrB = hkUint64(entityB);

#if HK_POINTER_SIZE == 4
	// fold two 32 bit values into one 64 bit
	if ( ptrA < ptrB )
	{
		keyOut = (ptrA << 32) + ptrB;
	}
	else
	{
		keyOut = (ptrB << 32) + ptrA;
	}
#else
	// store both 64 bit
	if ( ptrA < ptrB )
	{
		keyOut.m_a = ptrA;
		keyOut.m_b = ptrB;
	}
	else
	{
		keyOut.m_a = ptrB;
		keyOut.m_b = ptrA;
	}
#endif
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
