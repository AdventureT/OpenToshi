/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_ALIGN_SCENE_TO_NODE_H
#define HAVOK_FILTER_ALIGN_SCENE_TO_NODE_H

#include <ContentTools/Common/Filters/FilterScene/AlignSceneToNode/hctAlignSceneToNodeOptions.h>
#include <ContentTools/Common/Filters/FilterScene/AlignSceneToNode/hctAlignSceneToNodeFilter.h>
#include <Common/SceneData/Scene/hkxSceneUtils.h>

/// This class is a duplicate of hkAlignSceneToNodeOptions.h.
/// Please do not change one without changing the other.
/// For further information, please refer to EXP-1655 and EXP-1656

class hctAlignSceneToNodeFilter : public hctFilterInterface
{
	public: 

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT);

		hctAlignSceneToNodeFilter(const hctFilterManagerInterface* owner);
		/*virtual*/ ~hctAlignSceneToNodeFilter();

		/*virtual*/ void setOptions(const void* optionData, int optionDataSize, unsigned int version);
		/*virtual*/ HWND showOptions(HWND owner);

		/*virtual*/ void process( class hkRootLevelContainer& data );

		/// Option ptr only valid until the filter is deleted and/or the filter dll is unloaded
		/*virtual*/ int getOptionsSize() const;
		/*virtual*/ void getOptions(void* optionData) const;

		/*virtual*/ void hideOptions();

	// Public for the dialog
	public:

		HWND m_optionsDialog;
		hkBool m_fillingControls; // True when we are setting up the data in the controls; avoids recursion
		hkBool m_doNotRefreshEdit; // To avoid the custom samples edit box being updated constantly and cursor being lost

		hctAlignSceneToNodeOptions m_options;

		// A buffer for storing the options in XML form.
		mutable hkArray<char> m_optionsBuf;

		// Dialog handling stuff
		void setControlsFromData();
		void setDataFromControls();

	private:
		// Similarity transform: result = T^-1 M T
		static void HK_CALL similarity( hkMatrix4& result, const hkMatrix4& T, const hkMatrix4& M );
};

class hctAlignSceneToNodeFilterDesc : public hctFilterDescriptor
{
	public:

		/*virtual*/ unsigned int getID() const { return 0x5010d60b; }
		/*virtual*/ FilterCategory getCategory() const { return HK_CATEGORY_CORE; }
		/*virtual*/ FilterBehaviour getFilterBehaviour() const { return HK_DATA_MUTATES_INPLACE; }
		/*virtual*/ const char* getShortName() const { return "Align Scene To Node"; }
		/*virtual*/ const char* getLongName() const { return "Align Scene To Node Filter"; }
		/*virtual*/ unsigned int getFilterVersion() const { return HCT_FILTER_VERSION(1,0,0); }
		/*virtual*/ hctFilterInterface* createFilter(const class hctFilterManagerInterface* owner) const { return new hctAlignSceneToNodeFilter(owner); }

		/*virtual*/ HavokComponentMask getRequiredHavokComponents () const { return HK_COMPONENT_COMMON; }
};

extern hctAlignSceneToNodeFilterDesc g_alignSceneToNodeDesc;

#endif // HAVOK_FILTER_ALIGN_SCENE_TO_NODE_H

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
