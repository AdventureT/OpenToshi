/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_INTERFACE__H
#define HAVOK_FILTER_INTERFACE__H

/// The interface for all filters. Each filter needs to implement the methods below,
/// the most important of which is process() - the one that does the job.
/// Internally the actual filter is usually memory managed by Havok for instance,
/// but any filter can use memory management as it sees fit.
class hctFilterInterface : public hkReferencedObject
{
	public:
		
			/// Constructor. It takes a pointer to the filter manager that owns it.
		hctFilterInterface (const class hctFilterManagerInterface* owner);

			/// Virtual destructor
		virtual ~hctFilterInterface ();

			/// Bring up a child dialog to allow the user to set options for this filter.
			/// It should return a handle to the dialog (that may get repositioned).
			/// hideOptions() will be called on it sometime in the future and it can delete the window.
			/// By default returns HK_NULL (no options to display).
		virtual HWND showOptions ( HWND owner );

			/// Hide the dialog brought up during showOptions.
		virtual void hideOptions ();

			/// Set the options for this filter based on the given buffer.
		virtual void setOptions (const void* optionData, int optionDataSize, unsigned int version);

			/// Must return the buffer size required to store the options for this filter.
		virtual int getOptionsSize () const;

			/// Must fill the given buffer with the option data for this filter (this will be stored with the
			/// configuration. This pointer is only valid until the filter is deleted and/or the filter dll is unloaded.
		virtual void getOptions (void* optionData) const;
		
			/// Process the given contents. The main method in any filter. There are different process modes, see filterInterafce->getProcessMode to 
			///	work out if you should show UI or not during processing.
		virtual void process ( class hkRootLevelContainer& contents ) = 0;

			/// Returns a pointer to the filter manager that created this filter.
		const hctFilterManagerInterface* getFilterManager () const;

	protected:

		const class hctFilterManagerInterface* m_filterManager;
};


#endif // HAVOK_FILTER_INTERFACE__H

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
