/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#ifndef HK_COLLIDE2_AABB_UTIL_H
#define HK_COLLIDE2_AABB_UTIL_H

#include <Common/Base/Types/Physics/MotionState/hkMotionState.h>	// needed for .inl file

class hkAabb;
class hkTransform;
class hkVector4;


/// A utility class for creating AABBs that contain various geometric objects.
class hkpAabbUtil
{
	public:
		//
		// Non-continuous aabb calculations
		//

			/// Calculates an AABB from an array of vertices.
		static void HK_CALL calcAabb( const float* vertexArray, int numVertices, int striding, hkAabb& aabbOut );

			/// Calculates an AABB from an array of hkVector4.
		static void HK_CALL calcAabb( const hkVector4* vertexArray, int numVertices, hkAabb& aabbOut );

			/// Calculates an AABB from an OBB specified by a transform, a center, and an halfExtents vector and an extra radius
		static inline void HK_CALL calcAabb( const hkTransform& BvToWorld, const hkVector4& halfExtents, const hkVector4& center, float extraRadius, hkAabb& aabbOut);

			/// Calculates an AABB from an OBB specified by a transform, and an halfExtents vector.
		static inline void HK_CALL calcAabb( const hkTransform& BvToWorld, const hkVector4& halfExtents, float extraRadius, hkAabb& aabbOut);


	public:

		//
		// Continuous aabb calculations
		//

		struct OffsetAabbInput
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_COLLIDE, hkpAabbUtil::OffsetAabbInput );

			hkPadSpu<const hkMotionState*> m_motionState;

			// intermediate transforms
			hkTransform m_transforms[4];
			hkPadSpu<int> m_numTransforms;

			// start-end transforms
			hkTransform m_startTransform;
			hkTransform m_endTransformInv;
		};

			/// Sweep a discrete aabb and convert it into a 'continuous' swept aabb
		HK_FORCE_INLINE static void HK_CALL sweepAabb(const hkMotionState* motionstate, hkReal tolerance, const hkAabb& aabbIn, hkAabb& aabbOut);

			/// Calculate OffsetAabbInput
		static void HK_CALL initOffsetAabbInput(const hkMotionState* motionstate, OffsetAabbInput& input);

			/// Sweep a discrete aabb which has a huge offset from the mass center and convert it into a 'continuous' swept aabb
		static void HK_CALL sweepOffsetAabb(const OffsetAabbInput& input, const hkAabb& aabbIn, hkAabb& aabbOut);


};

#include <Physics/Collide/Util/hkpAabbUtil.inl>

#endif // HK_COLLIDE2_AABB_UTIL_H

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
