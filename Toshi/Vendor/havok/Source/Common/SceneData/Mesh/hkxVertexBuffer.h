/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEDATA_MESH_HKXVERTEXBUFFER_HKCLASS_H
#define HKSCENEDATA_MESH_HKXVERTEXBUFFER_HKCLASS_H

#include <Common/SceneData/Mesh/hkxVertexDescription.h>

class hkxVertexDescription;

/// hkxVertexBuffer meta information
extern const class hkClass hkxVertexBufferClass;

/// The information needed to construct a vertex buffer. This structure binds a
/// chunk of memory to a vertex format
class hkxVertexBuffer
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexBuffer );
		HK_DECLARE_REFLECTION();
	
			/// Default constructor
		inline hkxVertexBuffer();
		
		inline int getNumVertices() const;

		inline const char* getVertexData() const;
		inline char* getVertexData();

		inline const hkxVertexDescription* getVertexDesc() const;
		inline hkxVertexDescription* getVertexDesc();
		
		inline const hkClass* getVertexDataClass() const;

	public:
	
		inline void setVertexDataClass( const hkClass* klass );
		inline void setVertexDataPtr( void* data, int numDataItems );
		inline void setVertexCount(int c); 
		inline void setVertexDesc(hkxVertexDescription* desc);

	protected:
		
			/// To work out what class to put here (only required if you want to serialize this buffer)
			/// ask the vertexDesc to get one for you / make a hkClass for your own vert format
		const hkClass* m_vertexDataClass;
		void* m_vertexData;
		hkInt32 m_numVertexData;

			/// The pointer to the vertex data layout description.
			/// This should describe the buffer format in the same way as the hkClass, just in a more user friendly way
		class hkxVertexDescription* m_vertexDesc;
};

#include <Common/SceneData/Mesh/hkxVertexBuffer.inl>

#endif // HKSCENEDATA_MESH_HKXVERTEXBUFFER_HKCLASS_H


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
