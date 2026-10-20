/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEDATA_MATERIAL_HKXMATERIAL_HKCLASS_H
#define HKSCENEDATA_MATERIAL_HKXMATERIAL_HKCLASS_H

#include <Common/SceneData/Attributes/hkxAttributeHolder.h>

/// hkxMaterial meta information
extern const class hkClass hkxMaterialClass;

/// A serialization wrapper for the color and texture layers of a mesh section.
class hkxMaterial : public hkxAttributeHolder
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxMaterial );
		HK_DECLARE_REFLECTION();
	
			/// Used by textures to hint at their usage.
		enum TextureType
		{
				/// 
			TEX_UNKNOWN,
				/// 
			TEX_DIFFUSE,
				/// 
			TEX_REFLECTION,
				/// 
			TEX_BUMP,
				/// 
			TEX_NORMAL,
				/// 
			TEX_DISPLACEMENT,
				/// 
			TEX_SPECULAR, // Specular Level map
				/// 
			TEX_SPECULARANDGLOSS, // Specular Level map with the Gloss (power) in the Alpha channel
				///
			TEX_OPACITY, // Opacity (transparency) map. Normally not used, just use the alpha channel in one of the diffuse maps instead.
				/// 
			TEX_EMISSIVE, // Emissive (self illumination) map
				///
			TEX_REFRACTION,
				///	
			TEX_GLOSS, // Specular Power map, normally not used (alpha in specmap quicker)
				///
			TEX_NOTEXPORTED
		};
		
			/// The information needed to texture an object in the correct way. A stage contains
			/// just a texture map (one of two types, so a hkVariant) and some hint information.
		struct TextureStage
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxMaterial::TextureStage );
			HK_DECLARE_REFLECTION();

				/// The texture map. Usually an inplace or file texture pointer
			hkVariant m_texture;
			
				/// What the texture is meant for. Is it a normal map etc. An int so not assumed to
				/// be one of the default above enums.
			hkEnum< TextureType, hkInt32 > m_usageHint;

				/// Which of the texture coords it meant to use in the vertex buffer. Starts at 0. -1 for unknown.
			hkInt32 m_tcoordChannel; //+default(-1)
		};
		
		void sortTextureStageOrder();
		
		//
		// Members
		//
	public:
		
			/// The name of the material as seen in the modeler
		char* m_name;

			/// Ordered list of textures. Stages (or samplers) in a graphics engine.
		struct TextureStage* m_stages;
		hkInt32 m_numStages;
		
			/// Diffuse RGBA == the vector XYZW
		hkVector4 m_diffuseColor;
		
			/// Ambient RGB == the vector XYZ
		hkVector4 m_ambientColor;
		
			/// Specular RGB == the vector XYZ. Specular Power (shininess in ogl) is the W
			/// (0..20..100 are normal for the power)
		hkVector4 m_specularColor;
		
			/// Emissive RGB == the vector XYZ
		hkVector4 m_emissiveColor;
		
			/// Sub materials
		class hkxMaterial** m_subMaterials;
		hkInt32 m_numSubMaterials;

			/// extra material info such as a shader (FX file or whatever, usually a hkxMaterialEffect if it comes from our exporters, or a hkxMaterialShaderSet if specific single pass shader sets)
		hkVariant m_extraData;
};

#endif // HKSCENEDATA_MATERIAL_HKXMATERIAL_HKCLASS_H

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
