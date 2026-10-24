/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_KDTREE_BUILDING_UTILS_H
#define HK_KDTREE_BUILDING_UTILS_H

#include <Common/Base/hkBase.h>
#include <Common/Internal/KdTree/hkKdTree.h>
#include <Common/Internal/KdTree/Build/hkKdTreeBuilder.h>
#include <Common/Internal/KdTree/Build/hkKdTreeBuildInput.h>

class hkKdTreeBuildingUtils
{
public:

		/// Allocate space for the buffers required to build the tree
	static void HK_CALL allocateBuffers( struct hkKdTreeBuildInput::WorkingSet& buffersOut, int numPrimitives );

		/// Deallocate the space for the buffers
	static void HK_CALL deallocateBuffers( struct hkKdTreeBuildInput::WorkingSet& buffers, int numPrimitives );

		/// Build the tree from the buffers
	static void HK_CALL buildTree( hkKdTreeBuildInput::WorkingSet& buffers, hkKdTreeNode* treeOut, hkKdTreeBuildInput::ProjectedEntry* projEntOut, int treeOutSize, int numPrims, int _numAvailableEmptyNodes, int& maxDepthOut );
	static void HK_CALL buildTree( hkKdTreeBuildInput::WorkingSet& buffers, hkKdTree* treeOut);
	
		/// Call this to patch up the independent results from a distributed build
	static void HK_CALL completeDistributedBuild( hkKdTree* treeInOut, int numPrimitives, int* depths, int* subJobSizes );

		/// For fastbuilds
	static void HK_CALL projectPrimitives(hkKdTreeBuildInput::Mediator* HK_RESTRICT mediator, int startIdx, int endIdx, hkKdTreeBuildInput::ProjectedEntry* HK_RESTRICT output );

		// Emulated distributed building in single threaded mode for testing
	static void HK_CALL buildTreeDistributed( hkKdTreeBuildInput::Mediator& mediator, hkKdTree* tree );
};

#endif //HK_KDTREE_BUILDING_UTILS_H


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
