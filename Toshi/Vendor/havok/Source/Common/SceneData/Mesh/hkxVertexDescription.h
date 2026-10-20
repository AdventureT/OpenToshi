/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEDATA_MESH_HKXVERTEXDESCRIPTION__H
#define HKSCENEDATA_MESH_HKXVERTEXDESCRIPTION__H

/// hkxVertexDescription meta information
extern const class hkClass hkxVertexDescriptionClass;

/// This structure describes the memory layout and format of a vertex buffer.
class hkxVertexDescription
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexDescription );
		HK_DECLARE_REFLECTION();

			/// Default constructor
		inline hkxVertexDescription();
		
		enum DataType
		{
			HKX_DT_NONE = 0,
			HKX_DT_UINT8, // only used for four contiguous hkUint8s, hkUint8[4]
			HKX_DT_INT16, // only used for old style quantized tcoords (0x7fff maps to 10.0f), so div by 3276.7f to get the float tcoords. Deprecated.
			HKX_DT_UINT32,
			HKX_DT_FLOAT,
			HKX_DT_FLOAT2, // for tex coords 
			HKX_DT_FLOAT3, // will always be 16byte aligned, so you can treat as a hkVector4 (with undefined w, with SIMD enabled etc)
			HKX_DT_FLOAT4  // will always be 16byte aligned, so you can treat as a hkVector4 (with SIMD enabled etc)
		};

		enum DataUsage
		{
			HKX_DU_NONE = 0,
			HKX_DU_POSITION = 1,
			HKX_DU_COLOR = 2,    // first color always can be assumed to be per vertex Diffuse, then per vertex Specular (rare)
			HKX_DU_NORMAL = 4,
			HKX_DU_TANGENT = 8,
			HKX_DU_BINORMAL = 16, // aka BITANGENT
			HKX_DU_TEXCOORD = 32, // in order, 0,1,2, etc of the texture channels. Assumed to be 2D, [u,v], in most cases
			HKX_DU_BLENDWEIGHTS = 64,  // usually 4 weights, but 3 can be stored with 1 implied. Can be stored as 4*uint8, so quantized where 1.0f => 0xff (255),
			HKX_DU_BLENDINDICES = 128, // usually 4 hkUint8s in a row. So can reference 256 blend transforms (bones)
			HKX_DU_USERDATA = 256
		};

		struct ElementDecl
		{
			HK_DECLARE_REFLECTION();

			hkUint32					m_byteOffset; // from start of a vertex
			hkEnum<DataType,  hkUint16>	m_type;
			hkEnum<DataUsage, hkUint16>	m_usage;
		};


		inline hkUint32 getUsageCount( DataUsage usage ) const; // 0 if not used.
		
		inline const ElementDecl* getElementDeclByIndex( int index ) const; //Will return NULL on error
		inline const ElementDecl* getElementDecl( DataUsage usage, int index ) const; // index == cardinal of that usage, eg TexCoord0, TexCoord1 etc. Will return NULL on error		
		inline ElementDecl* getElementDeclByIndex( int index ); //Will return NULL on error
		inline ElementDecl* getElementDecl( DataUsage usage, int index ); // index == cardinal of that usage, eg TexCoord0, TexCoord1 etc. Will return NULL on error
		
			// as used quite often:
		inline hkUint32 getElementByteOffset( DataUsage usage, int index ) const; 
		inline DataType getElementType( DataUsage usage, int index ) const; 
		
		inline hkUint32 getMask() const;

			// Gets the vertex class that will match the decl (as best it can). Will remove the Elems that it was not able to cater for
			// with the limited vertex formats we have compiled in to support easy serialization.
		static const hkClass* HK_CALL getBuiltInVertexClass( const hkArray<hkxVertexDescription::DataUsage>& desiredElems, hkArray<ElementDecl>& classElems );
		static void HK_CALL getClassElements( const hkClass* klass, const hkArray<hkxVertexDescription::DataUsage>& desiredElems, hkArray<ElementDecl>& classElems );

	public:
		
			/// The byte offsets between two consecutive vertices
		hkUint32 m_stride;
		
			/// The vertex layout
		hkArray<struct ElementDecl> m_decls;
};

// The types are set for the common built in types, so you just get to choose the data usage (num tex coords, tangents etc)
static hkxVertexDescription* HK_CALL hkxCreateSerializableVertexDescription( const hkArray<hkxVertexDescription::DataUsage>& desiredElems );

#include <Common/SceneData/Mesh/hkxVertexDescription.inl>

#endif // HKSCENEDATA_MESH_HKXVERTEXDESCRIPTION__H

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
