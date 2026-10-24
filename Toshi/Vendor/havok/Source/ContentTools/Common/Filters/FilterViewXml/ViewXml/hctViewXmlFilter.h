/* 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_XML_PREVIEW__H
#define HAVOK_FILTER_XML_PREVIEW__H

#include <ContentTools/Common/Filters/FilterViewXml/ViewXml/hctViewXmlOptions.h>

class hctViewXmlFilter : public hctModelessFilter
{
	public: 

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT);

		hctViewXmlFilter(const hctFilterManagerInterface* owner);
		/*virtual*/ ~hctViewXmlFilter();

		/*
		** From hkModelessfilter
		*/

		/// Access to the descriptor.
		/*virtual*/ hctModelessFilterDescriptor& getDescriptor() const;

		// Processing
		/*virtual*/ void modalProcess ( class hkRootLevelContainer& data );

		/// The filter manager may request us to close as soon as possible
		/*virtual*/ void tryToClose();

		/// Returns true if the filter is closing down
		/*virtual*/ bool isClosingDown () const;

		/// Modeless filters can be treated as modal if desired. Override this method in order to do so.
		/// Default implementation returns false;
		/*virtual*/ bool behaveAsModal () const;

		/*
		** From hctFilterInterface (options)
		*/

		/*virtual*/ void setOptions ( const void* optionData, int optionDataSize, unsigned int version );
		/*virtual*/ HWND showOptions ( HWND owner );

		/*virtual*/ int getOptionsSize() const;
		/*virtual*/ void getOptions(void* optionData) const;
		/*virtual*/ void hideOptions();

	public : // public for dialog

		/// True if the user is trying to close us
		bool m_closeRequested;

		/// Options
		hctViewXmlOptions m_options;

		HWND m_optionsDialog;
		void updateOptions();

		// A buffer for storing the options in XML form.
		mutable hkArray<char> m_optionsBuf;
};


class hctViewXmlFilterDesc : public hctModelessFilterDescriptor
{
	public:

		/*virtual*/ unsigned int getID() const { return 0x01f160c1; }
		/*virtual*/ FilterCategory getCategory() const { return HK_CATEGORY_CORE; }
		/*virtual*/ FilterBehaviour getFilterBehaviour() const { return HK_DATA_PASSTHRU; }
		/*virtual*/ const char* getShortName() const { return "View XML"; }
		/*virtual*/ const char* getLongName() const { return "Inspect the scene during export as an XML file."; }
		/*virtual*/ unsigned int getFilterVersion() const { return HCT_FILTER_VERSION(1,1,0); }
		/*virtual*/ hctFilterInterface* createFilter(const hctFilterManagerInterface* owner) const { return new hctViewXmlFilter(owner); }

		/*virtual*/ HavokComponentMask getRequiredHavokComponents () const { return HK_COMPONENT_COMMON; }
};

extern hctViewXmlFilterDesc g_viewXmlDesc;

#endif // HAVOK_FILTER_XML_PREVIEW__H

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
