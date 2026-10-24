/* 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_TEXTURES_CREATE_TANGENTS_H
#define HAVOK_FILTER_TEXTURES_CREATE_TANGENTS_H

class hctCreateTangentsFilter : public hctFilterInterface
{
	public: 

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT);

		hctCreateTangentsFilter(const hctFilterManagerInterface* owner);
		/*virtual*/ ~hctCreateTangentsFilter();

		/*virtual*/ void setOptions(const void* optionData, int optionDataSize, unsigned int version);
		/*virtual*/ HWND showOptions(HWND owner);

		/*virtual*/ void process( class hkRootLevelContainer& data );

		/// Option ptr only valid until the filter is deleted and/or the filter dll is unloaded
		/*virtual*/ int getOptionsSize() const;
		/*virtual*/ void getOptions(void* optionData) const;

		/*virtual*/ void hideOptions();

	public:
			
		void updateOptions();

		HWND m_optionsDialog;

		bool m_splitVertices;
		hkObjectArray< hkString > m_meshList; // the name of the nodes that reference specific meshes.

		// A buffer for storing the options in XML form.
		mutable hkArray<char> m_optionsBuf;
};

class hctCreateTangentsFilterDesc : public hctFilterDescriptor
{
	public:

		/*virtual*/ unsigned int getID() const { return 0x5382aca8; }
		/*virtual*/ FilterCategory getCategory() const { return HK_CATEGORY_GRAPHICS; }
		/*virtual*/ FilterBehaviour getFilterBehaviour() const { return HK_DATA_MUTATES_INPLACE; }
		/*virtual*/ const char* getShortName() const { return "Create Tangents"; }
		/*virtual*/ const char* getLongName() const { return "Using a given set of texture coordinates, auto generate the tangents and binormals for selected meshes."; }
		/*virtual*/ unsigned int getFilterVersion() const { return HCT_FILTER_VERSION(1,0,2); }
		/*virtual*/ hctFilterInterface* createFilter(const class hctFilterManagerInterface* ownerInstance) const { return new hctCreateTangentsFilter(ownerInstance); }

		/*virtual*/ HavokComponentMask getRequiredHavokComponents () const { return HK_COMPONENT_COMMON; }
};

extern hctCreateTangentsFilterDesc g_createTangentsDesc;

#endif // HAVOK_FILTER_TEXTURES_CREATE_TANGENTS_H

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
