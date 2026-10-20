/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_KDTREE_BUILD_COMMON_H
#define HK_KDTREE_BUILD_COMMON_H

#include <Common/Base/hkBase.h>
#include <Common/Internal/KdTree/hkKdTree.h>
#include <Common/Base/Thread/Semaphore/hkSemaphoreBusyWait.h>


class hkKdTreeBuilderSparseWriter;

namespace hkKdTreeBuildInput
{
	class Mediator : public hkReferencedObject
	{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_COLLIDE);

		virtual int getNumPrimitives() const = 0;

		virtual hkPrimitiveId getPrimitiveId( int primitiveIdx ) const = 0;

		virtual void projectPrimitive( hkPrimitiveId primitiveId, hkKdTreeBuildInput::ProjectedEntry& bounds ) = 0;
	};
	
	struct TreeStream
	{
		HK_PAD_ON_SPU(hkKdTreeNode*)	m_root;
	};

	enum Splitting {
		SURFACE_AREA_HEURISTIC,
		MEDIAN
	};

	enum {	MIN_NODES_FOR_DISTRIBUTED_BUILD = 8	};

	enum {  MAX_NODES_FOR_DIRECT_BUILD = 512 };

	// ProjectedEntry definition moved to KdTree.h
	
	struct WorkingSet
	{
		HK_PAD_ON_SPU(ProjectedEntry*) m_entriesIn;
		HK_PAD_ON_SPU(ProjectedEntry*) m_entriesOut;
	};

	struct BuildRecursiveInput
	{
		HK_PAD_ON_SPU(WorkingSet*)	m_buffers;
		HK_PAD_ON_SPU(int)			m_currentNodeIdx;
		HK_PAD_ON_SPU(int)			m_maxNodeIdx;		// the maximum number of nodes allowed to allocate, equal to the size of the node buffer
		HK_PAD_ON_SPU(int)			m_startPrimIdx;
		HK_PAD_ON_SPU(int)			m_endPrimIdx;
		hkKdTreeBuildInput::ProjectedEntry			m_bounds;
		HK_PAD_ON_SPU(hkKdTreeBuilderSparseWriter*) m_writer;
		hkVector4Comparison			m_allowedEmptySplits;	// prevent creation of redundant empty splits

		BuildRecursiveInput() : m_maxNodeIdx(-1)
		{
			m_allowedEmptySplits.set(hkVector4Comparison::MASK_XYZ);
		}
	};

}


#endif //HK_KDTREE_BUILD_COMMON_H


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
