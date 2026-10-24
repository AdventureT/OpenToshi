/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEDATA_MESH_FORMATS_HKXVERTEXP4N4T4B4C1T2_HKCLASS_H
#define HKSCENEDATA_MESH_FORMATS_HKXVERTEXP4N4T4B4C1T2_HKCLASS_H

extern const class hkClass hkxVertexP4N4T4B4C1T2Class;
extern const class hkClass hkxVertexP4N4T4B4T4Class;
extern const class hkClass hkxVertexP4N4T4B4C1T6Class;
extern const class hkClass hkxVertexP4N4T4B4C1T10Class;




// 1 TCoord
class hkxVertexP4N4T4B4C1T2
{
	// +version(1) 

	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexP4N4T4B4C1T2 );
		HK_DECLARE_REFLECTION();
		
	public:
		
		hkVector4 m_position;
		hkVector4 m_normal;
		hkVector4 m_tangent;
		hkVector4 m_binormal;

		hkUint32 m_diffuseA;
		hkReal m_u;
		hkReal m_v;
		hkUint32 m_diffuseB;
};

class hkxVertexP4N4T4B4T4
{
public:

	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexP4N4T4B4T4 );
	HK_DECLARE_REFLECTION();

public:

	hkVector4 m_position;
	hkVector4 m_normal;
	hkVector4 m_tangent;
	hkVector4 m_binormal;

	hkReal m_u0;
	hkReal m_v0;
	hkReal m_u1;
	hkReal m_v1;
};


// 2 or 3 TCoord
class hkxVertexP4N4T4B4C1T6
{
	// +version(1) 

	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexP4N4T4B4C1T6 );
		HK_DECLARE_REFLECTION();
		
	public:
		
		hkVector4 m_position;
		hkVector4 m_normal;
		hkVector4 m_tangent;
		hkVector4 m_binormal;

		hkUint32 m_diffuseA;
		hkReal m_u0;
		hkReal m_v0;
		hkReal m_u1;
		hkReal m_v1;
		hkReal m_u2;
		hkReal m_v2;
		hkUint32 m_diffuseB;
};

// 4 or 5 TCoord
class hkxVertexP4N4T4B4C1T10
{
	// +version(1) 

	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexP4N4T4B4C1T10 );
		HK_DECLARE_REFLECTION();
		
	public:
		
		hkVector4 m_position;
		hkVector4 m_normal;
		hkVector4 m_tangent;
		hkVector4 m_binormal;

		hkUint32 m_diffuseA;
		hkReal m_u0;
		hkReal m_v0;
		hkReal m_u1;
		hkReal m_v1;
		hkReal m_u2;
		hkReal m_v2;
		hkReal m_u3;
		hkReal m_v3;
		hkReal m_u4;
		hkReal m_v4;
		hkUint32 m_diffuseB;
};
#endif // HKSCENEDATA_MESH_FORMATS_HKXVERTEXP4N4T4B4C1T2_HKCLASS_H

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
