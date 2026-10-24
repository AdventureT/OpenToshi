/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKFILTERTEXTURES_HKFILTERTEXTURESCREATETANGENTSOPTIONS_HKCLASS_H
#define HKFILTERTEXTURES_HKFILTERTEXTURESCREATETANGENTSOPTIONS_HKCLASS_H

/// hctCreateTangentsOptions meta information
extern const class hkClass hctCreateTangentsOptionsClass;

/// Contains a list of the meshes to autogenerate the tangents and binormals for.
class hctCreateTangentsOptions
{
public:

	HK_DECLARE_REFLECTION();

	// Struct holding default values

	struct DefaultStruct;

		/// Default constructor
	hctCreateTangentsOptions() { }

	//
	// Members
	//
public:

		/// An array containing the names of the nodes that reference specific meshes. Separated by ';'.
	char* m_meshes;

		/// If true, it will detect and split vertices with mirrored UVs (see EXP-582)
	bool m_splitVertices;

};

#endif // HKFILTERTEXTURES_HKFILTERTEXTURESCREATETANGENTSOPTIONS_HKCLASS_H

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
