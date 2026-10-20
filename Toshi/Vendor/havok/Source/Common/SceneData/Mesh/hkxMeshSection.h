/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEDATA_MESH_HKXMESHSECTION_HKCLASS_H
#define HKSCENEDATA_MESH_HKXMESHSECTION_HKCLASS_H

/// hkxMeshSection meta information
extern const class hkClass hkxMeshSectionClass;

/// A serialization wrapper for the relationship between a Vertex buffer and a set
/// of primitives.
class hkxMeshSection
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxMeshSection );
		HK_DECLARE_REFLECTION();
	
		//
		// Members
		//
	public:
		
			/// The vertex buffer used by this section of the mesh. This is a typed vertex buffer 
			/// which has a limited set of pre-compiled vertex formats but you are free to formulate
			/// any vertex description for it. If the vertex does not have a hkClass to define it
			/// it will not serialize correctly, but will work fine if just for runtime.
		class hkxVertexBuffer* m_vertexBuffer;
		
			/// The set of primitives that use the vertex buffer.
		class hkxIndexBuffer** m_indexBuffers;
		hkInt32 m_numIndexBuffers;
		
			/// The material to use for this mesh. May be a multi material or a plain single
			/// material, or null.
		class hkxMaterial* m_material;

			/// User channels are expected to contain per-vertex, per-edge or per-face information
		hkVariant* m_userChannels;
		hkInt32 m_numUserChannels;
		
	public:

		/// Returns the number of triangles
		hkUint32 getNumTriangles () const;

		void getTriangleIndices (hkUint32 triIndex, hkUint32& indexAOut, hkUint32& indexBOut, hkUint32& indexCOut) const;


		//
		// Utility methods
		//

			/// Collects all vertex positions to the given array.
		void collectVertexPositions (hkArray<hkVector4>& verticesOut) const;

			/// Constructs an hkGeometry object based on this mesh section
		void constructGeometry (struct hkGeometry& geometryOut);

};

#endif // HKSCENEDATA_MESH_HKXMESHSECTION_HKCLASS_H

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
