/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKFILTERSCENE_HKFILTERSCENEPLATFORMWRITEROPTIONS_HKCLASS_H
#define HKFILTERSCENE_HKFILTERSCENEPLATFORMWRITEROPTIONS_HKCLASS_H

/// hctPlatformWriterOptions meta information
extern const class hkClass hctPlatformWriterOptionsClass;

/// Describes the platform writer options.
class hctPlatformWriterOptions
{
	public:
	
		HK_DECLARE_REFLECTION();
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT, hctPlatformWriterOptions);

			/// 
		enum Preset
		{
				/// 
			CUSTOM,
				/// 
			MSVC_WIN32,
				/// 
			MSVC_XBOX,
				/// 
			MSVC_AMD64,
				/// 
			CW_PS2,
				/// 
			CW_GAMECUBE,
				/// 
			CW_PSP,
				///
			CW_WII,
				///
			GCC33_PS2,
				/// 
			GCC32_PS2,
				/// 
			GCC295_PS2,
				/// 
			SN31_PS2,
				/// 
			SN393_GAMECUBE,
				/// 
			SNC_PSP,
				/// 
			GCC151_PSP,
				/// 
			X360,
				/// 
			GCC_PS3,
				///
			MAC_PPC,
				///
			MAC_386,
				/// 
			XML
		};
		
			/// Default constructor
		hctPlatformWriterOptions() :
			m_filename(HK_NULL), m_tagfile(false),
			m_preset(hctPlatformWriterOptions::MSVC_WIN32),
			m_bytesInPointer(4),
			m_littleEndian(1),
			m_reusePaddingOptimized(0),
			m_emptyBaseClassOptimized(1),
			m_removeMetadata(false),
			m_userTag(0),
			m_saveEnvironmentData(false) { }

			/// Finish constructor
		hctPlatformWriterOptions(hkFinishLoadedObjectFlag f) { }
		
		//
		// Members
		//
	public:
		
			/// The filename to export as. It is the name less the 'common path' between what
			/// was entered / selected and what the current asset is called.
		char* m_filename;

			/// Use tagfile format
		hkBool m_tagfile; //+default(false)
		
			/// Chosen layout preset
		hkEnum<Preset, hkInt8> m_preset; //+default(hctPlatformWriterOptions::MSVC_WIN32)
		
			/// Number of bytes in a pointer
		hkInt8 m_bytesInPointer; //+default(4)
		
			/// Is the platform little endian. Eg: pc, xbox and PlayStation(R)2 are 1, and 0 for powerpc
			/// based.
		hkInt8 m_littleEndian; //+default(1)
		
			/// Does the reuse padding optimization.
		hkInt8 m_reusePaddingOptimized; //+default(0)
		
			/// Does the empty base class optimization.
		hkInt8 m_emptyBaseClassOptimized; //+default(1)
		
			/// Should we remove hkClass(es)
		hkBool m_removeMetadata; //+default(true)
		
			/// An optional tag to the file
		hkUint32 m_userTag; //+default(0)

			/// Whether environment data is saved
		hkBool m_saveEnvironmentData; //+default(false)
};

#endif // HKFILTERSCENE_HKFILTERSCENEPLATFORMWRITEROPTIONS_HKCLASS_H

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
