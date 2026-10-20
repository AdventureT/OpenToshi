/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_SCENE_PLATFORM_WRITER_H
#define HAVOK_FILTER_SCENE_PLATFORM_WRITER_H

#include <ContentTools/Common/Filters/FilterScene/PlatformWriter/hctPlatformWriterOptions.h>

#include <Common/Serialize/Util/hkStructureLayout.h>

// Short name
typedef hctPlatformWriterOptions OPTTYPE;

class hctPlatformWriterFilter : public hctFilterInterface
{
	public: 

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT);

		hctPlatformWriterFilter(const hctFilterManagerInterface* owner);
		/*virtual*/ ~hctPlatformWriterFilter();

		/*virtual*/ void setOptions(const void* optionData, int optionDataSize, unsigned int version);
		/*virtual*/ HWND showOptions(HWND owner);

		/*virtual*/ void process( class hkRootLevelContainer& data );

		/// Option ptr only valid until the filter is deleted and/or the filter dll is unloaded
		/*virtual*/ int getOptionsSize() const;
		/*virtual*/ void getOptions(void* optionData) const;

		/*virtual*/ void hideOptions();


	// Public for the dialog
	public:

		void updateOptions();
		HWND m_optionsDialog;

		mutable hkString m_assetFolder;
		mutable hkString m_filename;

		mutable hctPlatformWriterOptions m_options;

		// A buffer for storing the options in XML form.
		mutable hkArray<char> m_optionsBuf;


		// Structure layout and presets
		struct OptionToLayoutEntry
		{
			OPTTYPE::Preset preset;
			const hkStructureLayout::LayoutRules* layout;
			char* label;
		};

		static OptionToLayoutEntry m_optionToLayoutTable[];
		static int m_optionToLayoutTableSize;

		// finds a specific preset in m_optionToLayoutTable
		static int findPresetIndex( hctPlatformWriterOptions::Preset p );
		
};

// Version history
// 1.0.1 oldest
// 1.1.0 added "m_tagfile"

class hctPlatformWriterFilterDesc : public hctFilterDescriptor
{
	public:

		/*virtual*/ unsigned int getID() const { return 0xab787565; }
		/*virtual*/ FilterCategory getCategory() const { return HK_CATEGORY_CORE; }
		/*virtual*/ FilterBehaviour getFilterBehaviour() const { return HK_DATA_MUTATES_EXTERNAL; }
		/*virtual*/ const char* getShortName() const { return "Write to Platform"; }
		/*virtual*/ const char* getLongName() const { return "Write binary data for any given platform."; }
		/*virtual*/ unsigned int getFilterVersion() const { return HCT_FILTER_VERSION(1,1,0); }
		/*virtual*/ hctFilterInterface* createFilter(const class hctFilterManagerInterface* owner) const { return new hctPlatformWriterFilter(owner); }

		/*virtual*/ HavokComponentMask getRequiredHavokComponents () const { return HK_COMPONENT_COMMON; }
};

extern hctPlatformWriterFilterDesc g_platformWriterDesc;

#endif // HAVOK_FILTER_SCENE_PLATFORM_WRITER_H

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
