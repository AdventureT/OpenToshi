/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_MESHSECTIONUTIL_H
#define HKANIMATION_MESHSECTIONUTIL_H

#include <Common/SceneData/Mesh/hkxMeshSection.h>
#include <Common/SceneData/Mesh/hkxIndexBuffer.h>

class hkxMeshSectionUtil
{
	public:
		
		typedef hkxIndexBuffer* (HK_CALL *hkxCreateIndexBuffer)(int numIndices, hkBool use32BitStorage, hkxIndexBuffer::IndexType type );
		typedef hkxVertexBuffer* (HK_CALL *hkxCreateVertexBuffer)(int numVertices, class hkxVertexDescription* desc );

		typedef hkArray<hkInt16> hkBoneRemapping; 

			/// Compute a new grouping for the hkxMeshSection that respects
			/// the fact that the number of bones at any one time in the 
			/// rendering engine may be limited. Usually this limit can be
			/// 4 for non indexed matrix modes or around 20 indexable bones
			/// in the case of some PC Vertex Shaders. Delete the bone mappings
			/// arrays when you are done (one per output index buffer), you need 
			/// this remapping to know which of the original bones each index buf
			/// requires and in what order.
			/// This function is slow and uses memory. Ideally this should
			/// be done as an optimization step on export and the resultant
			/// mesh used if the hardware supports / requires it.
			/// Returns true if it had to do anything.
		static void computeLimitedBoneSection(const hkxMeshSection& section, 
			hkUint32 indexedBoneLimit, hkxCreateIndexBuffer indexFunc, hkxCreateVertexBuffer vertexFunc,
			hkxMeshSection& newSection, hkArray<hkBoneRemapping*>& boneMatrixMap );

	protected:

		hkxMeshSectionUtil() { }
};

#endif // HKANIMATION_MESHSECTIONUTIL_H

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
