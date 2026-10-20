/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEDATA_MESH_HKXMESH_HKCLASS_H
#define HKSCENEDATA_MESH_HKXMESH_HKCLASS_H

#include <Common/SceneData/Attributes/hkxAttributeHolder.h>

/// hkxMesh meta information
extern const class hkClass hkxMeshClass;

/// The information needed to construct a mesh.
class hkxMesh
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxMesh );
		HK_DECLARE_REFLECTION();

		struct UserChannelInfo : public hkxAttributeHolder
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxMesh::UserChannelInfo );
			HK_DECLARE_REFLECTION();
			const char* m_name;
			const char* m_className;
		};

		//
		// Members
		//
	public:
		
			/// The vertex buffers
		class hkxMeshSection** m_sections;
		hkInt32 m_numSections;

		struct UserChannelInfo* m_userChannelInfos;
		hkInt32 m_numUserChannelInfos;

		
		//
		// Utility methods
		//

			/// Traverses all the mesh sections and collects all vertex positions to the given array.
		void collectVertexPositions (hkArray<hkVector4>& verticesOut) const;

			/// Traverses all the mesh sections and constructs a single geometry out of them
		void constructGeometry (struct hkGeometry& geometryOut) const;
};


#endif // HKSCENEDATA_MESH_HKXMESH_HKCLASS_H

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
