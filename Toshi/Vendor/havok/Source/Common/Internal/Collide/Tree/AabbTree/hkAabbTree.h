/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_AABB_TREE_H
#define HK_AABB_TREE_H

#include <Common/Internal/Collide/Tree/BoundingVolumeTree/hkBoundingVolumeTree.h>

	/// Simple Aabb tree to be used in tools or in demos.
class hkAabbTree 
{
	public:

		/// A tree node (either leaf or internal node)
		struct AabbNode : public hkBoundingVolumeTree::Node
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DEMO, AabbNode );
			hkAabb m_volume;			///
		};

		/// Constructor
		hkAabbTree();

		/// Destructor
		~hkAabbTree();


		//
		//	Working with the tree
		//

		/// Build a tree from scratch. This typically results in a better tree than using add().
		/// \a preAllocatedInternalNodes can be used to pass in preallocated nodes, which will be used
		/// first before new internal nodes get allocated. By passing in \a numLeaves-1 preallocated nodes,
		/// the function will not do a single allocation. You can only call this function on an empty tree.
		void build( AabbNode** leaves, int numLeaves, hkArray<AabbNode*>& preAllocatedInternalNodes);	

		/// Remove and deallocate all nodes.
		void clear();

		/// Add a single node to the tree. You get the best tree if nodes are not added spatially sorted.
		/// If you add lots of spatially sorted nodes, your tree might get unbalanced. 
		/// To avoid this you can either use build() to build the tree from scratch or
		/// call rebuildOptimized() after you added all the nodes.
		AabbNode* add( const hkAabb& aabb, void* m_userData );	

		/// Remove a leaf.
		void remove( AabbNode* leaf );

		/// Update a leaf.
		void update( AabbNode* leaf, const hkAabb& aabb );

		/// Update a moving leaf.
		/// This function is optimized for moving objects. It checks whether the \a aabb is
		/// still completely inside the leaf's aabb, and if not it updates the
		/// leaf's aabb with the expanded input \a aabb. It uses \a expansionDirection and the \a expansionMargin to do this expansion.
		/// The expansionDirection is good for linear moving objects, the expansionMargin helps with rotating or curving objects.
		void update( AabbNode* leaf, const hkAabb& aabb, const hkVector4& expansionDirection, hkReal expansionMargin );

		/// Rebuild the tree.
		void rebuildOptimized();


		//
		//	Query tree functions
		//

		/// Enumerate all leaves which intersect with aabb.
		void queryAabb( const hkAabb& aabb, hkBoundingVolumeTree::Collector& collector );

		/// Cast a ray.
		/// 
		/// Details if you really want to push performance:
		/// This implementation is optimized for brute force speed and will not try to optimize
		/// the search in a way that it searches the tree closer to the ray start first.
		/// To get the optimal performance, you should remember the hitRatio for each ray you fire
		/// and in the next frame initialize \a initialEarlyOutFraction with the slightly increased value from the previous frame.
		/// Only if you do not get a hit, recast the ray with \a initialEarlyOutFraction set to 1.0f.
		/// 
		///	Example: You are standing in front of a wall and want to check whether you can see anything behind it.
		/// This function can now potentially start checking against lots of object behind the wall first.
		/// To avoid this in subsequent frames you simple cast a short ray from your eye's position to the last
		/// hitpoint on the wall, plus 1 meter. Only if this shortened ray does not hit, you will have to recast
		/// using the full ray length.
		void castRay( const hkBoundingVolumeTree::CastRayInput& ray, hkBoundingVolumeTree::Collector& collector, hkReal initialEarlyOutFraction = 1.0f );

		/// Find the closest point to an object.
		void findClosestPoint( const hkVector4& point, hkBoundingVolumeTree::Collector& collector );

	protected:

		void* m_impl;
};


#endif // HK_INTERNALPCH_H

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
