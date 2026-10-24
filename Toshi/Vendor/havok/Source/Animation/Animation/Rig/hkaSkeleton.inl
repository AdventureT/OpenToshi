/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */



HK_FORCE_INLINE hkaSkeleton::hkaSkeleton():
	m_name(HK_NULL), 
	m_parentIndices(HK_NULL), 
	m_numParentIndices(0),
	m_bones(HK_NULL), 
	m_numBones(0), 
	m_referencePose(HK_NULL), 
	m_numReferencePose(0),
	m_floatSlots(HK_NULL), 
	m_numFloatSlots(0),
	m_localFrames(HK_NULL),
	m_numLocalFrames(0)
{
}

HK_FORCE_INLINE hkLocalFrame* hkaSkeleton::getLocalFrameForBone( int boneIndex ) const
{
	
	

	for( int i = 0; i < m_numLocalFrames; i++ )
	{
		if ( m_localFrames[i].m_boneIndex == boneIndex )
		{
			return m_localFrames[i].m_localFrame;
		}
		else if ( m_localFrames[i].m_boneIndex > boneIndex )
		{
			break;
		}
	}

	return HK_NULL;
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
