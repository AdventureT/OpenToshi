/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKP_SHAPE_SKINNING_UTIL_H
#define HKP_SHAPE_SKINNING_UTIL_H

#include <Common/GeometryUtilities/Mesh/Utils/FindVertexWeightsUtil/hkFindVertexWeightsUtil.h>

class hkpConvexShape;
class hkpConvexVerticesShape;


/// Utility class that works out bone indices and weights from a shapes

class hkpShapeSkinningUtil
{
	public:
		typedef hkFindVertexWeightsUtil::Entry Entry;

        HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_COLLIDE, hkpShapeSkinningUtil );

        struct Input
        {
            Input()
            {
                m_maxOutside = 1.0f;
                m_maxInside = 1.0f;
                m_collisionDispatcher = HK_NULL;
                m_shapes = HK_NULL;
                m_transforms = HK_NULL;
                m_numBones = 0;
                m_bonesPerVertex = 4;
                m_vertexPositions = HK_NULL;
                m_numVertices = 0;
            }

            hkReal m_maxInside;                                 
            hkReal m_maxOutside;                                
            hkpCollisionDispatcher* m_collisionDispatcher;      
            const hkpShape** m_shapes;                          
            const hkTransform* m_transforms;                    
            int m_numBones;                                     
            int m_bonesPerVertex;                               
            hkVector4* m_vertexPositions;                       
            int m_numVertices;                                  
        };

            /// Find a list of shapes and transforms, finds the one which is closest. If non are in maxDistance range returns -1.
        static int HK_CALL findClosestShape(hkpCollisionDispatcher* collisionDispatcher, const hkArray<hkTransform>& shapeTransforms, const hkArray<const hkpShape*>& shapes, hkReal maxDistance, const hkVector4& point);

            /// Calculates the weights and bone indices based on the input. There will be m_bonesPerVertex * m_numVertices entries - each m_bonesPerVertex run
			/// for a vertex. A bone index in an entry = -1 means that a bone close enough could not be found.
		static void HK_CALL calculateSkinning(const Input& input, hkArray<Entry>& entries);

            /// Set the skinning values on a vertex buffer
        static hkResult HK_CALL setSkinningValues(const Input& inputIn, hkMeshVertexBuffer* buffer);

            /// Set the skinning values on a shape
        static hkResult HK_CALL setSkinningValues(const Input& inputIn, hkMeshShape* meshShape);

};


#endif // HKP_SHAPE_SKINNING_UTIL_H

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
