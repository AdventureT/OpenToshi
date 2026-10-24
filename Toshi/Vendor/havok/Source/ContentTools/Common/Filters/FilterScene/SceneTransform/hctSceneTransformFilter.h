/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_SCENE_TRANSFORM_COMPRESSION_H
#define HAVOK_FILTER_SCENE_TRANSFORM_COMPRESSION_H

#include <ContentTools/Common/Filters/FilterScene/SceneTransform/hctSceneTransformOptions.h>
#include <Common/SceneData/Scene/hkxSceneUtils.h>

class hctSceneTransformFilter : public hctFilterInterface
{
	private:

		struct ResourceDataTransformInfo : public hkxSceneUtils::TransformInfo
		{
			public:

				HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hctSceneTransformFilter::ResourceDataTransformInfo );

				hkReal m_floatScale;

				hkReal m_angleMultiplier;
		};

	public: 

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT);

		hctSceneTransformFilter(const hctFilterManagerInterface* owner);
		/*virtual*/ ~hctSceneTransformFilter();

		/*virtual*/ void setOptions(const void* optionData, int optionDataSize, unsigned int version);
		/*virtual*/ HWND showOptions(HWND owner);

		/*virtual*/ void process( class hkRootLevelContainer& data  );

		/// Option ptr only valid until the filter is deleted and/or the filter dll is unloaded
		/*virtual*/ int getOptionsSize() const;
		/*virtual*/ void getOptions(void* optionData) const;

		/*virtual*/ void hideOptions();

	private:

		void transformResourceData(const hkxSceneUtils::SceneTransformOptions& options, class hkResourceContainer* resourceContainer);
		void transformResourceMember(ResourceDataTransformInfo& resourceDataTransformInfo, const hkClassMember& member, void* object, hkPointerMap<void*, int>& processedObjects);

	// Public for the dialog
	public:

		HWND m_optionsDialog;
		hkBool m_fillingControls; // True when we are setting up the data in the controls; avoids recursion
		hkBool m_doNotRefreshEdit; // To avoid the custom samples edit box being updated constantly and cursor being lost

		hctSceneTransformOptions m_options;

		// A buffer for storing the options in XML form.
		mutable hkArray<char> m_optionsBuf;

		// Dialog handling stuff
		void setControlsFromData();
		void setDataFromControls();	
};

class hctSceneTransformFilterDesc : public hctFilterDescriptor
{
	public:

		/*virtual*/ unsigned int getID() const { return 0xaa786565; }
		/*virtual*/ FilterCategory getCategory() const { return HK_CATEGORY_CORE; }
		/*virtual*/ FilterBehaviour getFilterBehaviour() const { return HK_DATA_MUTATES_INPLACE; }
		/*virtual*/ const char* getShortName() const { return "Transform Scene"; }
		/*virtual*/ const char* getLongName() const { return "Scene Transformation Filter"; }
		/*virtual*/ unsigned int getFilterVersion() const { return HCT_FILTER_VERSION(1,2,1); }
		/*virtual*/ hctFilterInterface* createFilter(const class hctFilterManagerInterface* owner) const { return new hctSceneTransformFilter(owner); }

		/*virtual*/ HavokComponentMask getRequiredHavokComponents () const { return HK_COMPONENT_COMMON; }
};

extern hctSceneTransformFilterDesc g_sceneTransformDesc;

#endif // HAVOK_FILTER_SCENE_TRANSFORM_COMPRESSION_H

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
