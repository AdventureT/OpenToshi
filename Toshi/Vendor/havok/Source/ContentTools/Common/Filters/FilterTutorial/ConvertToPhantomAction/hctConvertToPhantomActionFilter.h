/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_CONVERT_TO_PHANTOM_ACTION_H
#define HAVOK_FILTER_CONVERT_TO_PHANTOM_ACTION_H

#include <ContentTools/Common/Filters/FilterTutorial/ConvertToPhantomAction/hctConvertToPhantomActionOptions.h>

#include <Common/Base/Container/Array/hkObjectArray.h>

/*
** This filter is part of the "Toolchain Integration" tutorial of the Havok Content Tools.
** Check the Havok Content Tools documentation for details
*/
class hctConvertToPhantomActionFilter : public hctFilterInterface
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT);

		// Constructor - takes a pointer to the filter manager that created the filter
		hctConvertToPhantomActionFilter(const hctFilterManagerInterface* owner);

		// Destructor
		/*virtual*/ ~hctConvertToPhantomActionFilter();

		// The main method in any filter, it does the job
		/*virtual*/ void process( class hkRootLevelContainer& data  );

		// Asks the filter to show its UI for options
		/*virtual*/ HWND showOptions(HWND owner);

		// Asks the fitler to close the options UI (and store any changes)
		/*virtual*/ void hideOptions();

		// Storage of option data
		/*virtual*/ void setOptions(const void* optionData, int optionDataSize, unsigned int version);
		/*virtual*/ int getOptionsSize() const;
		/*virtual*/ void getOptions(void* optionData) const;

	public:

		// The options dialog window
		HWND m_optionsDialog;

		// Called by other methods, it makes sure the stored options (m_options) match what the values in the UI
		void updateOptions();

		// The options associated with this filter
		hctConvertToPhantomActionOptions m_options;

		// A buffer for storing the options in XML form.
		mutable hkArray<char> m_optionsBuf;

};

// The filter descriptor is used by the filter manager to query information about the filter (name, category, etc)
// as well as to create individual instances.
class hctConvertToPhantomActionFilterDesc : public hctFilterDescriptor
{
	public:

		/*virtual*/ unsigned int getID() const { return 0x9376be9a; }
		/*virtual*/ FilterCategory getCategory() const { return HK_CATEGORY_USER; }
		/*virtual*/ FilterBehaviour getFilterBehaviour() const { return HK_DATA_MUTATES_INPLACE; }
		/*virtual*/ const char* getShortName() const { return "Convert To Phantom Action"; }
		/*virtual*/ const char* getLongName() const { return "[TUTORIAL] Converts specific rigid bodies to phantom actions. Must be placed after the Create Rigid Bodies filter. "; }
		/*virtual*/ unsigned int getFilterVersion() const { return HCT_FILTER_VERSION(1,0,0); }
		/*virtual*/ hctFilterInterface* createFilter(const class hctFilterManagerInterface* owner) const { return new hctConvertToPhantomActionFilter(owner); }

		/*virtual*/ HavokComponentMask getRequiredHavokComponents () const { return HK_COMPONENT_PHYSICS; }
};

extern hctConvertToPhantomActionFilterDesc g_convertToPhantomActionDesc;

#endif // HAVOK_FILTER_CONVERT_TO_PHANTOM_ACTION_H

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
