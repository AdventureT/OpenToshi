/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


hkpFixedBufferCdPointCollector::hkpFixedBufferCdPointCollector(hkpRootCdPoint* buffer, int capacity) : hkpCdPointCollector()
{
	m_pointsArrayBase	= buffer;
	m_nextFreePoint		= buffer;
	m_capacity			= capacity;
	m_numPoints			= 0;
}


const hkpRootCdPoint* hkpFixedBufferCdPointCollector::getHits() const
{
	return m_pointsArrayBase;
}


int hkpFixedBufferCdPointCollector::getNumHits()
{
	return m_numPoints;
}


hkBool hkpFixedBufferCdPointCollector::hasHit() const
{
	return m_numPoints > 0;
}


void hkpFixedBufferCdPointCollector::addCdPoint(const hkpCdPoint& event)
{
	HK_ASSERT2(0xaf635e1f, g_FixedBufferCdPointCollectorAddCdPointCallbackFunc, "You have to register either the default addCdPoint() function using hkpFixedBufferCdPointCollector::registerDefaultAddCdPointFunction() or a custom function using registerFixedBufferCdPointCollectorAddCdPointCallbackFunction().");
	g_FixedBufferCdPointCollectorAddCdPointCallbackFunc(event, this); 
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
