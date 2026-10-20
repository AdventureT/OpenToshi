/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SCENE_EXPORT_UTILS_H
#define HK_SCENE_EXPORT_UTILS_H

class hctFilterClassRegistry;

#include <Common/SceneData/Graph/hkxNode.h>

	/// Minor utilities used during export.
namespace hkSceneExportUtils
{
		/// Convert RGB and alpha float values to a single 32bit ARGB.
	inline unsigned floatsToARGB( const float r, const float g, const float b, const float a = 1.0f  ) 
	{
		return ((unsigned char)( a * 255.0f ) << 24 ) |
		((unsigned char)( r * 255.0f ) << 16 ) |
		((unsigned char)( g * 255.0f ) << 8 ) |
		((unsigned char)( b * 255.0f ) );
	}

		/// Convert RGB and alpha float values to a single 32bit ARGB and set out-of-range colors to 80% grey full opaque
	inline unsigned floatsToARGB_grey( const float r, const float g, const float b, const float a = 1.0f  ) 
	{
		const float rr = (r >= 0.0f) ? ( (r <= 1.0f) ? r : 0.8f ) : 0.8f;
		const float gg = (g >= 0.0f) ? ( (g <= 1.0f) ? g : 0.8f ) : 0.8f;
		const float bb = (b >= 0.0f) ? ( (b <= 1.0f) ? b : 0.8f ) : 0.8f;
		const float aa = (a >= 0.0f) ? ( (a <= 1.0f) ? a : 1.0f ) : 1.0f;
		return ((unsigned char)( aa * 255.0f ) << 24 ) |
			((unsigned char)( rr * 255.0f ) << 16 ) |
			((unsigned char)( gg * 255.0f ) << 8 ) |
			((unsigned char)( bb * 255.0f ) );
	}

		/// Convert RGB and alpha float values to a single 32bit ARGB and saturate out-of-range colors while making full opaque
	inline unsigned floatsToARGB_saturate( const float r, const float g, const float b, const float a = 1.0f  ) 
	{
		const float rr = (r >= 0.0f) ? ( (r <= 1.0f) ? r : 1.0f ) : 0.0f;
		const float gg = (g >= 0.0f) ? ( (g <= 1.0f) ? g : 1.0f ) : 0.0f;
		const float bb = (b >= 0.0f) ? ( (b <= 1.0f) ? b : 1.0f ) : 0.0f;
		const float aa = (a >= 0.0f) ? ( (a <= 1.0f) ? a : 1.0f ) : 1.0f;
		return ((unsigned char)( aa * 255.0f ) << 24 ) |
			((unsigned char)( rr * 255.0f ) << 16 ) |
			((unsigned char)( gg * 255.0f ) << 8 ) |
			((unsigned char)( bb * 255.0f ) );
	}

		/// Replaces any "<" and ">" character with underscores so the name can be part of an XML file.
	inline void getSerializableName( const char* nodeName, hkString& newName )
	{
		newName = nodeName;
		newName = newName.replace('<', '_'); // no xml tag parts
		newName = newName.replace('>', '_');
	}

		/// As getSerializableName, but also replacing spaces with underscores.
	inline void getReducedName( const char* nodeName, hkString& newName )
	{
		getSerializableName(nodeName, newName);
		newName = newName.replace(' ', '_'); // no spaces 
	}

	inline void reportSceneData (const hkxScene* scene)
	{
		const int totalNodes = scene->m_rootNode ? (1 + scene->m_rootNode->getNumDescendants()) : 0;

		HK_REPORT ("Exported "<<totalNodes<<" nodes, "<<scene->m_numSelectionSets<<" node selection sets, "
			<<scene->m_numMaterials<<" materials, " <<scene->m_numMeshes<<" meshes, "
			<<scene->m_numLights<<" lights, "<<scene->m_numCameras<<" cameras, "<<scene->m_numSkinBindings<<" skin bindings.");

	}

}

#endif //HK_SCENE_EXPORT_UTILS_H

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
