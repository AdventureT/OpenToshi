/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKFILTERSCENE_HKFILTERSCENEALTERMESHOPTIONS_HKCLASS_H
#define HKFILTERSCENE_HKFILTERSCENEALTERMESHOPTIONS_HKCLASS_H

/// hctAlterMeshOptions meta information
extern const class hkClass hctAlterMeshOptionsClass;

/// Options describing what to do to any meshes found.
class hctAlterMeshOptions
{
	public:
	
		HK_DECLARE_REFLECTION();
		
			/// Default constructor
		hctAlterMeshOptions() { }
		
		//
		// Members
		//
	public:
		
			/// Remove indices from the meshes (unroll into the vertex buffer)
		hkBool m_removeIndices; //+default(true)
};

#endif // HKFILTERSCENE_HKFILTERSCENEALTERMESHOPTIONS_HKCLASS_H

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
