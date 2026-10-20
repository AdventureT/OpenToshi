/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEDATA_MESH_FORMATS_HKXVERTEXP4N4W4I4C1Q2_HKCLASS_H
#define HKSCENEDATA_MESH_FORMATS_HKXVERTEXP4N4W4I4C1Q2_HKCLASS_H

extern const class hkClass hkxVertexP4N4W4I4C1Q2Class;
extern const class hkClass hkxVertexP4N4W4I4T2Class;
extern const class hkClass hkxVertexP4N4W4I4C1T4Class;
extern const class hkClass hkxVertexP4N4W4I4T6Class;
extern const class hkClass hkxVertexP4N4W4I4C1T8Class;
extern const class hkClass hkxVertexP4N4W4I4C1T12Class;

// For backward compat, [Deprecated, quantized tcoords not much use]
class hkxVertexP4N4W4I4C1Q2
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexP4N4W4I4C1Q2 );
		HK_DECLARE_REFLECTION();

	public:

		hkVector4 m_position;
		hkVector4 m_normal;
		
		hkUint32 m_weights;
		hkUint32 m_indices;
		hkUint32 m_diffuse;
		hkInt16 m_qu; 
		hkInt16 m_qv;
};


// 1 or 2 Tcoords
class hkxVertexP4N4W4I4C1T4
{
	// +version(1) 

	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexP4N4W4I4C1T4 );
		HK_DECLARE_REFLECTION();

	public:
		
		hkVector4 m_position;
		hkVector4 m_normal;
		
		hkUint8 m_w0;
		hkUint8 m_w1;
		hkUint8 m_w2;
		hkUint8 m_w3;
		hkUint8 m_i0;
		hkUint8 m_i1;
		hkUint8 m_i2;
		hkUint8 m_i3;
		hkUint32 m_diffuseA;
		hkReal m_u0;

		hkReal m_v0;
		hkReal m_u1;
		hkReal m_v1;
		hkUint32 m_diffuseB;
};

// No vcolor, so no need for padding
class hkxVertexP4N4W4I4T2
{
public:

	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexP4N4W4I4T2 );
	HK_DECLARE_REFLECTION();

public:

	hkVector4 m_position;
	hkVector4 m_normal;

	hkUint8 m_w0;
	hkUint8 m_w1;
	hkUint8 m_w2;
	hkUint8 m_w3;
	hkUint8 m_i0;
	hkUint8 m_i1;
	hkUint8 m_i2;
	hkUint8 m_i3;

	hkReal m_u0;
	hkReal m_v0;
};

// 3 Tcoords, no vcolor
class hkxVertexP4N4W4I4T6
{
public:

	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexP4N4W4I4T6 );
	HK_DECLARE_REFLECTION();

public:

	hkVector4 m_position;
	hkVector4 m_normal;


	hkUint8 m_w0;
	hkUint8 m_w1;
	hkUint8 m_w2;
	hkUint8 m_w3;
	hkUint8 m_i0;
	hkUint8 m_i1;
	hkUint8 m_i2;
	hkUint8 m_i3;
	hkReal m_u0;
	hkReal m_v0;

	hkReal m_u1;
	hkReal m_v1;
	hkReal m_u2;
	hkReal m_v2;
};

// 3 or 4 Tcoords
class hkxVertexP4N4W4I4C1T8
{
	// +version(1) 

	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexP4N4W4I4C1T8 );
		HK_DECLARE_REFLECTION();

	public:
		
		hkVector4 m_position;
		hkVector4 m_normal;

		
		hkUint8 m_w0;
		hkUint8 m_w1;
		hkUint8 m_w2;
		hkUint8 m_w3;
		hkUint8 m_i0;
		hkUint8 m_i1;
		hkUint8 m_i2;
		hkUint8 m_i3;
		hkUint32 m_diffuseA;
		hkReal m_u0;

		hkReal m_v0;
		hkReal m_u1;
		hkReal m_v1;
		hkReal m_u2;

		hkReal m_v2;
		hkReal m_u3;
		hkReal m_v3;
		hkUint32 m_diffuseB;
};


// 5 or 6 Tcoords
class hkxVertexP4N4W4I4C1T12
{
	// +version(1) 

	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxVertexP4N4W4I4C1T12 );
		HK_DECLARE_REFLECTION();

	public:
		
		hkVector4 m_position;
		hkVector4 m_normal;

		
		hkUint8 m_w0;
		hkUint8 m_w1;
		hkUint8 m_w2;
		hkUint8 m_w3;
		hkUint8 m_i0;
		hkUint8 m_i1;
		hkUint8 m_i2;
		hkUint8 m_i3;
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
		hkReal m_u5;
		hkReal m_v5;
		hkUint32 m_diffuseB;
};

#endif // HKSCENEDATA_MESH_FORMATS_HKXVERTEXP4N4W4I4C1Q2_HKCLASS_H

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
