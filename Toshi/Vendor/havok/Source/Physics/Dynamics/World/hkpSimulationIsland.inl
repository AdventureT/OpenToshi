/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


hkpWorld* hkpSimulationIsland::getWorld()
{
	return m_world;
}


inline hkBool hkpSimulationIsland::wasActiveLastFrame() const
{
	return m_isInActiveIslandsArray;
}

inline hkBool hkpSimulationIsland::isFixed() const
{
	return m_storageIndex == HK_INVALID_OBJECT_INDEX;
}

inline const hkArray<hkpEntity*>& hkpSimulationIsland::getEntities() const 
{ 
	return m_entities; 
}

inline const hkArray<hkpAction*>& hkpSimulationIsland::getActions() const 
{ 
	return m_actions; 
}


int hkpSimulationIsland::getStorageIndex()
{
	return m_storageIndex;
}


inline int hkpSimulationIsland::getMemUsageForIntegration()
{
	int sizeForElemTemp = m_constraintInfo.m_numSolverElemTemps * hkSizeOf(hkpSolverElemTemp) + 2 * hkSizeOf(hkpSolverElemTemp);
	int sizeForAccumulators = getEntities().getSize() * hkSizeOf(hkpVelocityAccumulator) + hkSizeOf(hkpVelocityAccumulator) + 16; // fixed rigid body + end tag (16 byte aligned)

	return	sizeForAccumulators + 
			sizeForElemTemp + 
			m_constraintInfo.m_sizeOfSchemas + 
			HK_SIZE_OF_JACOBIAN_END_SCHEMA;
}


inline void hkpSimulationIsland::markAllEntitiesReadOnly() const
{
#ifdef HK_DEBUG_MULTI_THREADING
	for (int i =0; i < m_entities.getSize(); i++)
	{
		m_entities[i]->markForRead();
	}
#endif
}

// helper functions for debugging multithreading
inline void hkpSimulationIsland::unmarkAllEntitiesReadOnly() const
{
#ifdef HK_DEBUG_MULTI_THREADING
	for (int i =0; i < m_entities.getSize(); i++)
	{
		m_entities[i]->unmarkForRead();
	}
#endif
}

inline void hkpSimulationIsland::unmarkForWrite()
{
	m_multiThreadCheck.unmarkForWrite();
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
