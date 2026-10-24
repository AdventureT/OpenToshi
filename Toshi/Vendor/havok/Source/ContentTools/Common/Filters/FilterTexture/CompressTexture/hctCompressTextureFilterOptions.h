/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKFILTERTEXTURES_HKFILTERTEXTURESCOMPRESS_HKCLASS_H
#define HKFILTERTEXTURES_HKFILTERTEXTURESCOMPRESS_HKCLASS_H

/// hctCompressTextureFilterOptions meta information
extern const class hkClass hctCompressTextureFilterOptionsClass;

/// Options detailing texture size reduction.
class hctCompressTextureFilterOptions
{
	public:
	
		HK_DECLARE_REFLECTION();
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT, hctCompressTextureFilterOptions); // required as we have a finsih ctor

			/// Default constructor
		hctCompressTextureFilterOptions() { }
		
		hctCompressTextureFilterOptions(hkFinishLoadedObjectFlag f) : m_perTextureOptions(f) { }

		//
		// Members
		//
	public:
		
		enum OutputFormat	
		{
			OF_RGB = 0, // Don't compress, just make into DDS (and optionaly mipmap)
			OF_DXT1,
			OF_DXT1a,   // DXT1 with binary alpha.
			OF_DXT3,
			OF_DXT5,
			OF_DXT5n,   // Compressed HILO: R=0, G=x, B=0, A=y

			// DX10 formats.
			OF_BC1, // DXT1
			OF_BC1a,
			OF_BC2, // DXT3
			OF_BC3, // DXT5
			OF_BC3n, 
			OF_BC4, // ATI1
			OF_BC5  // 3DC, ATI2
		};

		enum OutputQuality
		{
			OQ_Fastest = 0,
			OQ_Normal,
			OQ_Production,
			OQ_Highest
		};

		enum MipmapFilter
		{
			MF_NoMipmaps = 0,
			MF_Box,
			MF_Triangle,
			MF_Kaiser
		};

		struct ConvertOptions
		{ 
			HK_DECLARE_REFLECTION();
			
			char* m_textureName; // the texture these options are for

			hkEnum<enum OutputFormat, hkUint8> m_format;
			hkEnum<enum OutputQuality, hkUint8> m_quality;
			hkEnum<enum MipmapFilter, hkUint8> m_mipMapFilter;

			hkBool m_flipX;
			hkBool m_flipY;

			hkBool m_useOriginalFilename; // always use the original file as the source to load
		};
		
		// If sizeof m_options is 1, and texture name is null, then will be applied to all textures
		hkArray< struct ConvertOptions > m_perTextureOptions;

		char* m_outputTexturePath; // where to place file based textures (path will be stripped and this added, along with new file ext (.dds))
		hkBool m_allowOverwrite; 
};

#endif // HKFILTERTEXTURES_HKFILTERTEXTURESCOMPRESS_HKCLASS_H

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
