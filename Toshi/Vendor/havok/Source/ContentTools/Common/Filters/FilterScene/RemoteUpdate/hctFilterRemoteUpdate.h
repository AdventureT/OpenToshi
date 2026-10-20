/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_TOOLS_FILTER_REMOTE_IMPL__H
#define HAVOK_TOOLS_FILTER_REMOTE_IMPL__H

#include <ContentTools/Common/Filters/FilterScene/RemoteUpdate/hctFilterRemoteUpdateOptions.h>

class hctFilterToolStubRemote : public hctFilterInterface
{
	public: 

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT);

		hctFilterToolStubRemote(const hctFilterManagerInterface* owner);
		/*virtual*/ ~hctFilterToolStubRemote();

		/*
		** From hctFilterInterface
		*/

		/*virtual*/ void setOptions( const void* optionData, int optionDataSize, unsigned int version);
		/*virtual*/ HWND showOptions( HWND owner );

		/*virtual*/ void process( hkRootLevelContainer& contents);
		
		/// Option ptr only valid until the filter is deleted and/or the filter dll is unloaded
		/*virtual*/ int getOptionsSize() const;
		/*virtual*/ void getOptions(void* optionData) const;

		/*virtual*/ void hideOptions();

		// public for the dialog
	public:


		void updateOptions();

		hkString m_connectAddrStorage;
		hctFilterToolStubRemoteOptions m_options;

		// A buffer for storing the options in XML form during read / write
		mutable hkArray<char> m_optionsBuf;

		HWND m_ownerWindow;
		class hkRootLevelContainer* m_currentContainer;

		HWND m_optionsDialog;	
};

class hctFilterToolStubRemoteDesc : public hctFilterDescriptor
{
	public:

		virtual unsigned int getID() const { return 0x700030cc; }
		virtual FilterCategory getCategory() const { return HK_CATEGORY_GRAPHICS; }
		virtual FilterBehaviour getFilterBehaviour() const { return HK_DATA_PASSTHRU; }
		virtual const char* getShortName() const { return "Remote Preview Update"; }
		virtual const char* getLongName() const { return "Use this filter to update a currently running Havok Preview Tool."; }
		virtual unsigned int getFilterVersion() const { return HCT_FILTER_VERSION(1,2,1); }
		virtual hctFilterInterface* createFilter(const hctFilterManagerInterface* owner) const { return new hctFilterToolStubRemote(owner); }
		virtual HavokComponentMask getRequiredHavokComponents () const { return HK_COMPONENT_COMMON; }
};

extern hctFilterToolStubRemoteDesc g_remoteToolFilterDesc;

#endif // HAVOK_TOOLS_FILTER_REMOTE_IMPL__H

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090704)
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
