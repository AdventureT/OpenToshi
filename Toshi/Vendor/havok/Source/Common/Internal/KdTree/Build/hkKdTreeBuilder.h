/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_KDTREE_BUILDER_H
#define HK_KDTREE_BUILDER_H

#include <Common/Base/hkBase.h>
#include <Common/Internal/KdTree/hkKdTree.h>
#include <Common/Internal/KdTree/Build/hkKdTreeBuildInput.h>
#include <Common/Base/Thread/Semaphore/hkSemaphoreBusyWait.h>


enum hkSplitPrimitivesStatus
{
	SPLIT_PRIMITIVES_OK,
	SPLIT_PRIMITIVES_INDISTINGUISHABLE
};

template <typename ReadIter, typename WriteIter, const bool streaming>
class hkKdTreeBuilder
{
public:

	struct SplitPrimitivesResults
	{
		HK_PAD_ON_SPU(int)			m_splitAxis;
		HK_PAD_ON_SPU(int)			m_splitIndex;
		HK_PAD_ON_SPU(hkSplitType)	m_splitMin;
		HK_PAD_ON_SPU(hkSplitType)	m_splitMax;
	};


	// Bucketed (fast) build
	static hkSplitPrimitivesStatus HK_CALL splitPrimitives( const hkKdTreeBuildInput::WorkingSet& buffersIn, int startPrimIdx, int endPrimIdx, SplitPrimitivesResults& results, 
		HK_PAD_ON_SPU(int)& numAvailableEmptyNodes, hkKdTreeBuildInput::WorkingSet& buffersOut, const hkKdTreeBuildInput::ProjectedEntry& bounds, hkVector4ComparisonParameter allowedEmptySplits);
	
	static int HK_CALL buildTreeRecursive( const hkKdTreeBuildInput::BuildRecursiveInput& input, hkKdTreeBuildInput::TreeStream* streamOut, hkKdTreeBuildInput::ProjectedEntry* peOut, HK_PAD_ON_SPU(int)& numAllocatedNodes, HK_PAD_ON_SPU(int)& numEmptyNodesRemaining);

	static void HK_CALL checkBuildRecursiveInput( const hkKdTreeBuildInput::BuildRecursiveInput& subJob );

	static hkSplitPrimitivesStatus HK_CALL splitPrimitivesExactly( const hkKdTreeBuildInput::WorkingSet& buffersIn, int startPrimIdx, int endPrimIdx, 
		SplitPrimitivesResults& results, hkKdTreeBuildInput::WorkingSet& buffersOut, const hkKdTreeBuildInput::ProjectedEntry& bounds );

	static void HK_CALL splitTreeBuilding( const hkKdTreeBuildInput::WorkingSet& buffersIn, hkKdTreeNode* treeOut, int treeSize, int numPrimitives,  hkKdTreeBuildInput::BuildRecursiveInput* subJobs, int subTreeSizes[4], const hkKdTreeBuildInput::ProjectedEntry& bounds );

	static void HK_CALL writeEmptyLeaf(hkKdTreeNode* treeRoot, int nodeIdx, class hkKdTreeBuilderSparseWriter* writer);
	static void HK_CALL writeSinglePrimitiveToLeaf(hkKdTreeNode* treeRoot, hkKdTreeBuildInput::ProjectedEntry* entriesIn, hkKdTreeBuildInput::ProjectedEntry* projEntOut, int nodeIdx, int primIdx, hkKdTreeBuilderSparseWriter* writer);

	// returns the number of tree nodes that are "saved" by writing out the multiple leaves. These can be added to the number of available free nodes.
	static int HK_CALL writeMultiplePrimitivesToLeaf(hkKdTreeNode* HK_RESTRICT treeRoot, const hkKdTreeBuildInput::ProjectedEntry* entriesIn, hkKdTreeBuildInput::ProjectedEntry* HK_RESTRICT projEntOut, int nodeIdx, int primIdx, int numPrims, hkKdTreeBuilderSparseWriter* writer);
};


// It's safe to add __restrict on the write iterators, since they're write-only.
typedef const hkKdTreeBuildInput::ProjectedEntry* ProjectedEntryDirectReadIterator;
typedef hkKdTreeBuildInput::ProjectedEntry* HK_RESTRICT ProjectedEntryDirectWriteIterator;
typedef hkKdTreeBuilder< ProjectedEntryDirectReadIterator,    ProjectedEntryDirectWriteIterator, false> hkKdTreeBuilderDirect;

// We never use this on non-SPU platforms, but putting it here helps keep the recursion code a bit cleaner
typedef hkKdTreeBuilder< ProjectedEntryDirectReadIterator,    ProjectedEntryDirectWriteIterator, false> hkKdTreeBuilderStreaming;

class hkKdTreeBuilderSparseWriter
{
public:
	hkKdTreeBuilderSparseWriter();
	
	// This ensures that the writes finish when the writer goes out of scope.
	~hkKdTreeBuilderSparseWriter() { flush(); }

	void putNodeToMainMemory(hkKdTreeNode* root, int currentNodeIdx, hkKdTreeNode& node);
	void putProjEntToMainMemory(hkKdTreeBuildInput::ProjectedEntry* base, int peIdx, const hkKdTreeBuildInput::ProjectedEntry& peToWrite);

	enum { BUFFER_SIZE = 16 };
protected:

	void flush();

	hkVector4 m_writeBuffer[ BUFFER_SIZE ];
	HK_PAD_ON_SPU(int) m_index;
};

#endif //HK_KDTREE_BUILDER_H


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
