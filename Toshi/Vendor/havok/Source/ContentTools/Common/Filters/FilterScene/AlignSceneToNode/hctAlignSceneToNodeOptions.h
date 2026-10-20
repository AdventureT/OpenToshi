/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKFILTERSCENE_ALIGNSCENETONODEOPTIONS_HKCLASS_H
#define HKFILTERSCENE_ALIGNSCENETONODEOPTIONS_HKCLASS_H

/// hctAlignSceneToNodeOptions meta information
extern const class hkClass hctAlignSceneToNodeOptionsClass;

/// Describes delta compression settings
class hctAlignSceneToNodeOptions
{
	public:
	
		HK_DECLARE_REFLECTION();

			/// Default constructor
		hctAlignSceneToNodeOptions();

		void setNodeName( const char* );
		const char* getNodeName();

		hkBool needToDecompose();
		
		//
		// Members
		//
	public:

		hkBool m_invert;
		hkBool m_transformPositionX;
		hkBool m_transformPositionY;
		hkBool m_transformPositionZ;
		hkBool m_transformRotation;
		hkBool m_transformScale;
		hkBool m_transformSkew;
		hkInt32 m_keyframe;

	private:

		hkArray< hkInt8 > m_nodeName;

};

#endif // HKFILTERSCENE_ALIGNSCENETONODEOPTIONS_HKCLASS_H

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
