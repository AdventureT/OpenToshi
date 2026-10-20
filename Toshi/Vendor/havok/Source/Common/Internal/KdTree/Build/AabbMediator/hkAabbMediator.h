/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_AABB_MEDIATOR_H
#define HK_AABB_MEDIATOR_H

#include <Common/Internal/KdTree/Build/hkKdTreeBuilder.h>

// Collidable mediator
class hkAabbMediator : public hkKdTreeBuildInput::Mediator
{
	public:

		hkAabbMediator( const hkAabb* aabbs, int numAabbs, const hkAabb& worldExtents );

		virtual int getNumPrimitives() const;

		virtual void projectPrimitive( hkPrimitiveId primitiveId, hkKdTreeBuildInput::ProjectedEntry& bounds );

		virtual hkPrimitiveId getPrimitiveId( int primitiveIdx ) const;

	public:
		hkAabb  m_worldExtents;
		const hkAabb* m_aabbs;
		int		m_numAabbs;

		hkVector4 m_offset;
		hkVector4 m_scale;
		hkVector4 m_min;
		hkVector4 m_max;

};

#endif //HK_AABB_MEDIATOR_H


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
