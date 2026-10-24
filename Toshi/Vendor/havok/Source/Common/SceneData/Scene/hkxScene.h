/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEDATA_SCENE_HKXSCENE_HKCLASS_H
#define HKSCENEDATA_SCENE_HKXSCENE_HKCLASS_H

/// hkxScene meta information
extern const class hkClass hkxSceneClass;

/// A simple info storage class to allow the exporters to specify extra information
/// that might be useful overall (frames per second, total scene length, etc)
class hkxScene
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxScene );
		HK_DECLARE_REFLECTION();

		
		//
		// Members
		//
	public:
		
			/// A text string describing the modeller used to create this scene
		char* m_modeller;
		
			/// The full path and filename of the asset that was exported to make this scene.
		char* m_asset;
		
			/// The total length of the scene, in seconds. Assumes that when you are given
			/// key frames, they will be evenly spaced across this period.
		hkReal m_sceneLength;
		
			/// Scene Graph root node
		class hkxNode* m_rootNode;
		
			/// Array of selection sets in the scene
		class hkxNodeSelectionSet** m_selectionSets;	
		hkInt32 m_numSelectionSets;

			/// Array of cameras in the scene
		class hkxCamera** m_cameras;
		hkInt32 m_numCameras;
		
			/// Array of cameras in the scene
		class hkxLight** m_lights;
		hkInt32 m_numLights;
		
			/// Meshes in the scene
		class hkxMesh** m_meshes;
		hkInt32 m_numMeshes;
		
			/// Materials in the scene
		class hkxMaterial** m_materials;
		hkInt32 m_numMaterials;
		
			/// In place (loaded) textures in the scene
		class hkxTextureInplace** m_inplaceTextures;
		hkInt32 m_numInplaceTextures;
		
			/// External texture filenames
		class hkxTextureFile** m_externalTextures;
		hkInt32 m_numExternalTextures;
		
			/// Extra mesh info required to bind a skin to a hierarchy.
		class hkxSkinBinding** m_skinBindings;
		hkInt32 m_numSkinBindings;
		
			/// Transform applied to the scene
		hkMatrix3 m_appliedTransform; //+default(1 0 0 0  0 1 0 0  0 0 1 0)

			/// Constructor, initializes to zero/NULL/identity
		hkxScene();

			/// Recursively looks for a node by name (case insensitive), using depth-first search.
			/// Returns HK_NULL if not found, the node otherwise
		hkxNode* findNodeByName (const char* name) const;

			/// Constructs the full path (parent-first list of nodes from the root to the node itself, both included)
			/// Returns HK_FAILURE if the node doesn't belong to this scene
		hkResult getFullPathToNode (const hkxNode* theNode, hkArray<const hkxNode*>& pathOut) const;

			/// Gets the worldFromNode transform of the given node, by concatenating
			/// the transforms of its parents. By default, it uses the first stored key.
			/// Returns HK_FAILURE if the node doesn't belong to this scene
		hkResult getWorldFromNodeTransform (const hkxNode* theNode, class hkMatrix4& worldFromNodeOut, int key=0) const;
};

#endif // HKSCENEDATA_SCENE_HKXSCENE_HKCLASS_H

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
