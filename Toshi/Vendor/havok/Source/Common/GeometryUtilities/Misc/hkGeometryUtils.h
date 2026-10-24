/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_GEOMETRY_UTILS_H
#define HK_GEOMETRY_UTILS_H

struct hkGeometry;

	/// Utility class, contains methods that operate with hkGeometry objects.
class hkGeometryUtils
{
	public:		

			/// Given an hkGeometry, it finds identical vertices and collapses them, updating triangles as well.
		static void HK_CALL weldVertices (struct hkGeometry& meshGeometry, hkReal threshold = 1e-3f);

			/// Given an hkGeometry, it finds identical vertices and collapses them, updating triangles as well.
			/// This version returns a map from the original vertex indices to the new vertex indices.
		static void HK_CALL weldVertices (struct hkGeometry& meshGeometry, hkReal threshold, hkArray<int>& vertexRemapOut, hkArray<int>& triangleRemapOut);

			/// Creates an hkGeometry that has the shape of a capsule.
		static void HK_CALL createCapsuleGeometry(const hkVector4& top, const hkVector4& bottom, hkReal radius, int numHeightSegments, int numSides, const hkTransform& transform, hkGeometry*& geometryOut);

			/// Transform a geometry
		static void HK_CALL transformGeometry (const hkMatrix4& transform, hkGeometry& geometryInOut);

			/// Computes the volume of the given geometry
		static hkReal HK_CALL computeVolume (const struct hkGeometry& geometry);
};

#endif //HK_GEOMETRY_UTILS_H

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
