/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_KDTREE_AABB_QUERY_UTILS_H
#define HK_KDTREE_AABB_QUERY_UTILS_H

#include <Common/Internal/KdTree/hkKdTree.h>

class hkKdTreeAabbQueryUtils
{
public:
	enum {DEFAULT_STACK_SIZE = 128};

	static void HK_CALL queryAabbRecursive( const hkKdTree* root, const hkAabb& aabb, hkArray<hkPrimitiveId>& hits);

	static void HK_CALL queryAabbIterative( const hkKdTree* root, const hkAabb& aabb, hkArray<hkPrimitiveId>& hits);
	static hkUint32 HK_CALL queryAabbIterative( const hkKdTree* root, const hkAabb& aabb, hkPrimitiveId* hitsOut, int maxHits);

private:
	static hkUint32 HK_CALL queryAabbInternal( const hkKdTreeNode* root, const hkKdTreeBuildInput::ProjectedEntry* projectedEntries, hkVector4Parameter scale, hkVector4Parameter offset, const hkAabb& aabb, int treeDepth, hkArray<hkPrimitiveId>* hitArray, hkPrimitiveId* hitsOut, int maxHits);
	static void HK_CALL _queryAabbRecursive( const hkKdTreeNode* root, const hkKdTreeNode* node, const hkKdTreeBuildInput::ProjectedEntry* projectedEntries, hkVector4Parameter scale, hkVector4Parameter offset, const hkAabb& aabb, hkArray<hkPrimitiveId>& hits);


};

#endif // HK_KDTREE_AABB_QUERY_UTILS_H


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
