/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_MODELESS_FILTER_DESCRIPTOR_H
#define HAVOK_MODELESS_FILTER_DESCRIPTOR_H

class hctModelessFilter;

// The descriptor of a modeless filter tracks the active instances of the filter
class hctModelessFilterDescriptor : public hctFilterDescriptor
{
	public:
		
			/// Constructor
		hctModelessFilterDescriptor();

			/// Destructor
		virtual ~hctModelessFilterDescriptor();

		/*virtual*/ const char* getShortName() const { return "Modeless Filter"; }

			/// From hctFilterDescriptor, returns how many modeless filters are active
		/*virtual*/ int countModelessFilters (int& numActiveOut, int& numClosingout) const;

			/// From hctFilterDescriptor, tells all modeless filters to close asap
		/*virtual*/ void askModelessFiltersToClose () const;

			/// Adds a filter to the "active filters list", i.e., keeps track of it
			/// 
		void addFilterToActiveList (hctModelessFilter* filter);

			/// Removes a filter from the "active filters list"
		void removeFilterFromActiveList (hctModelessFilter* filter);

			/// Returns the i-th filter in the active filters list
		hctModelessFilter* getActiveFilter (int i) const;

	private:

			// Linked list of active instances of this filter
		struct ActiveFilterList
		{
			hctModelessFilter* m_activeFilter;
			ActiveFilterList* m_next;
		};

		ActiveFilterList* m_activeFilterList;
		mutable HANDLE m_activeFiltersMutex;

		// Creates the mutex if not already created
		void createMutexIfNecessary () const;

};


#endif // HAVOK_MODELESS_FILTER_DESCRIPTOR_H

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
