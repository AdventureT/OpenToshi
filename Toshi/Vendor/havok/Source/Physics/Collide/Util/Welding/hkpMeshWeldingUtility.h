/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_MESHWELDINGUTILITY_H
#define HK_MESHWELDINGUTILITY_H

#include <Common/Base/hkBase.h>
#include <Common/Base/Algorithm/Sort/hkSort.h>


class hkpBvTreeShape;
class hkpTriangleShape;
class hkpExtendedMeshShape;


	/// Utility functions for building runtime welding information for triangular meshes
class hkpMeshWeldingUtility
{
	public:
			/// specify consistency tests
		enum WindingConsistency
		{
			WINDING_IGNORE_CONSISTENCY,		///
			WINDING_TEST_CONSISTENCY,		///
		};


		struct ShapeInfo
		{
			hkTransform m_transform;
			const hkpBvTreeShape* m_shape;
		};
	
		static hkResult HK_CALL computeWeldingInfo( const hkTransform& meshTransform, hkpExtendedMeshShape* mesh, hkArray< ShapeInfo >& shapes, bool weldOpenEdges, WindingConsistency testConsistency );
		

			/// A function to generate welding information for a shape in a mesh. This welding should be
			/// stored by the mesh and set as the welding information in the triangle returned by getChildShape() with that shape key.
		static hkResult HK_CALL calcWeldingInfoForTriangle( hkpShapeKey shapeKey, const hkpBvTreeShape* bvTreeShape, WindingConsistency testConsistency, hkUint16& info );

			/// A function to check if winding is consistent between a triangle and its neighbors.
		static hkBool HK_CALL isTriangleWindingValid( hkpShapeKey shapeKey, const hkpBvTreeShape* bvTreeShape );
	

	public:

			// Helper for calcWeldingInfoForTriangle
		static hkResult HK_CALL calcBitcodeForTriangleEdge(const hkpBvTreeShape* bvTreeShape, const hkpTriangleShape* triangleShape, hkpShapeKey triangleShapeKey, int edgeIndex, WindingConsistency testConsistency, hkInt16& combinedBitcodesOut );

			// Helper to extract a single bitcode for a specific edge
		static int HK_CALL calcSingleEdgeBitcode(hkUint16 triangleEdgesBitcode, int edgeIndex );

			// Helper for calcWeldingInfoForTriangle
		static int HK_CALL createSingularVertexArray(const hkVector4 *vertices0, const hkVector4 *vertices1, int edgeIndex, hkVector4* vertexArrayOut, int orderedEdgeVerticesOnTriangle1[2]);

			// Helper for calcWeldingInfoForTriangle
		static hkUint16 HK_CALL modifyCombinedEdgesBitcode(hkUint16 combinedBitcode, int edgeIndex, int bitcode);

			// Helper for calcWeldingInfoForTriangle
		static int HK_CALL calcEdgeAngleBitcode(const hkVector4* vertices);

			// Helper for calcWeldingInfoForTriangle
		static int HK_CALL calcEdgeAngleBitcode(hkReal angle);

			// Helper for calcWeldingInfoForTriangle
		static void HK_CALL calcAntiClockwiseTriangleNormal(const hkVector4& vertex0, const hkVector4& vertex1, const hkVector4& vertex2, hkVector4& normal);

			// Helper for calcWeldingInfoForTriangle
		static hkReal HK_CALL calcAngleFromVertices(const hkVector4* vertices, hkReal& sinAngleOut, hkReal& cosAngleOut);

			// Helper for computeWeldingInfo 
		static hkReal HK_CALL calcAngleForEdge(hkVector4Parameter edgeNormal, hkVector4Parameter edgeOrtho, hkVector4Parameter triangleNormal);

};

#endif // HK_MESHWELDINGUTILITY_H

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
