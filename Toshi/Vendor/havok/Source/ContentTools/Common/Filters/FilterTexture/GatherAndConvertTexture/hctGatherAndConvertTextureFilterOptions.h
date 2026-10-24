/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKFILTERTEXTURES_HKFILTERTEXTURESGATHERANDCONVERT_HKCLASS_H
#define HKFILTERTEXTURES_HKFILTERTEXTURESGATHERANDCONVERT_HKCLASS_H

/// hctGatherAndConvertTextureOptions meta information
extern const class hkClass hctGatherAndConvertTextureFilterOptionsClass;

/// Options detailing texture size reduction.
class hctGatherAndConvertTextureFilterOptions
{
	public:
	
		HK_DECLARE_REFLECTION();
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT, hctGatherAndConvertTextureFilterOptions); // required as we have a finsih ctor

			/// Default constructor
		hctGatherAndConvertTextureFilterOptions() { }
		
		hctGatherAndConvertTextureFilterOptions(hkFinishLoadedObjectFlag f) : m_perTextureOptions(f) { }
		//
		// Members
		//
	public:
		
		// A reasonable subset of the FreeImage 3.10 supported formats:
		enum OutputFormat
		{
			OF_BMP		= 0,  // Supported by PC based HKG platforms
			OF_JPEG	    = 2,
			OF_PNG		= 13, // Supported by all HKG platforms
			OF_TARGA	= 17, // Supported by all HKG platforms if one of the common uncomressed / simple styles
			OF_TIFF	    = 18,
			OF_HDR		= 26
		};
		
		enum DitherAlgo
		{
			DTH_NONE		= -1,
			DTH_FS			= 0,	
			DTH_BAYER4x4	= 1,	
			DTH_BAYER8x8	= 2,	
			DTH_CLUSTER6x6	= 3,	
			DTH_CLUSTER8x8	= 4,	
			DTH_CLUSTER16x16= 5,	
			DTH_BAYER16x16	= 6		
		};

		enum ToneMapAlgo
		{
			TM_NONE       = -1,
			TM_DRAGO03	 = 0,	
			TM_REINHARD05 = 1,	
			TM_FATTAL02	
		};

		enum ResizeFilter
		{
			RF_NONE       = -1,
			RF_BOX		  = 0,	
			RF_BICUBIC	  = 1,	
			RF_BILINEAR   = 2,	
			RF_BSPLINE	  = 3,	
			RF_CATMULLROM = 4,	
			RF_LANCZOS3	 
		};

		struct ConvertOptions
		{ 
			HK_DECLARE_REFLECTION();
			
			char* m_textureName; // the texture these options are for
			
			hkEnum<enum OutputFormat, hkInt8> m_format;

			hkEnum<enum DitherAlgo, hkInt8> m_dither;
		
			hkEnum<enum ResizeFilter, hkInt8> m_resizeFilter;
			hkUint16 m_xTexels;
			hkUint16 m_yTexels;

			hkBool m_useOriginalFilename; // always use the original file as the source to load

		};

		// If sizeof m_options is 1, and texture name is null, then will be applied to all textures
		hkArray< struct ConvertOptions > m_perTextureOptions;

		char* m_outputTexturePath; // where to place file based textures (path will be stripped and this added, along with new file ext if one)
		hkBool m_allowOverwrite; 
};

#endif // HKFILTERTEXTURES_HKFILTERTEXTURESGATHERANDCONVERT_HKCLASS_H

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
