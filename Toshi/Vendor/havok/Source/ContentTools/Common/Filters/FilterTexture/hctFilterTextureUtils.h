/* 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_TEXTURES_UTILS__H
#define HAVOK_FILTER_TEXTURES_UTILS__H

#include <Common/SceneData/Material/hkxMaterial.h>
class hkxTexture;
class hkxScene;

class hctFilterTextureUtils 
{
	public:

		static void HK_CALL findSceneMaterials( hkxScene& scene, hkArray<hkxMaterial*>& sceneMaterials );
		static void HK_CALL findFileTextures( hkxScene& scene, hkArray<hkxTextureFile*>& fileBasedTextures, hkArray<hkxMaterial::TextureType>* typeInfo = HK_NULL);
		static void HK_CALL findInplaceTextures( hkxScene& scene, hkArray<hkxTextureInplace*>& inplaceTextures, hkArray<hkxMaterial::TextureType>* typeInfo = HK_NULL );
		static void HK_CALL getFullTexturePath( const char* filename, hkRootLevelContainer& data, hkString& fullPath );
		static const char* HK_CALL hintToText(hkxMaterial::TextureType t);
};

#endif //HAVOK_FILTER_TEXTURES_UTILS__H

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
