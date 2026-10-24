/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_DESCRIPTOR_H
#define HAVOK_FILTER_DESCRIPTOR_H

#define HCT_FILTER_VERSION(major, minor ,point) ( ((major) << 16) | ((minor) << 8) | (point) )

	/// Filter Descriptors act as virtual class members and factories for all filters. They provide 
	/// information about the filter name, type and description, as well as the ability to create instances of the actual filter.
	/// In that way we can find out properties of the filter before actually (optionally) creating it.
class hctFilterDescriptor
{
	public:
		
			/// This enum is used to place the filters in tabs in the filter manager.
			/// User-defined filters should use HK_USER.
		enum FilterCategory
		{
				/// Core Category
			HK_CATEGORY_CORE = 0,
				/// Deprecated category, use HK_PHYSICS instead.
			HK_CATEGORY_COLLISION_DEPRECATED, 
				/// Physics Category.
			HK_CATEGORY_PHYSICS,
				/// Cloth Category.
			HK_CATEGORY_CLOTH,
				/// Destruction Category.
			HK_CATEGORY_DESTRUCTION,
				/// FX Category, no longer used
			HK_CATEGORY_FX_PHYSICS_DEPRECATED,
				/// Animation Category.
			HK_CATEGORY_ANIMATION,
				/// Graphics Category.
			HK_CATEGORY_GRAPHICS,
				/// User Category.
			HK_CATEGORY_USER
				
		};

			/// Describes how the filter operates with the data (does it change it or no).
		enum FilterBehaviour	
		{
				/// Filter does not alter data, can be ignored in pure data runs, such as the previewer filters.
			HK_DATA_PASSTHRU = 0, 

				/// The filter changes or adds data in the stream - most filters are like this.
			HK_DATA_MUTATES_INPLACE = 1, 

				/// The filter changes or adds data to a external file, not the stream (such as the platform writer).
			HK_DATA_MUTATES_EXTERNAL = 2 
		
		};

		typedef int HavokComponentMask;

		enum HavokComponent
		{
			HK_COMPONENT_COMMON = 0,
			HK_COMPONENT_ANIMATION = 1,
			HK_COMPONENT_PHYSICS = 2,
			HK_COMPONENT_CLOTH = 4,
			HK_COMPONENT_DESTRUCTION = 8,
			// = 32, etc

			// Product combinations
			HK_COMPONENTS_COMPLETE = HK_COMPONENT_COMMON | HK_COMPONENT_ANIMATION | HK_COMPONENT_PHYSICS,
			
			// All
			HK_COMPONENTS_ALL = HK_COMPONENTS_COMPLETE | HK_COMPONENT_CLOTH | HK_COMPONENT_DESTRUCTION
		};

			/// Get unique ID of the filter (32 bits). Manager will warn when two alike are found.
		virtual unsigned int getID() const = 0;

			/// Get the rough category of the filter.
		virtual FilterCategory getCategory() const = 0;

			/// Find out at a the filter does with the contents (so we know whether we should run it.
			/// in trial or option runs etc).
		virtual FilterBehaviour getFilterBehaviour() const = 0; 

			/// Get the short display name for the filter. Should be unique (used by the options lookup) within its category. If this proves too hard then we will have to revert to a filter id (32 bit int say).
		virtual const char* getShortName() const = 0;

			/// Get the long name for the filter.
		virtual const char* getLongName() const = 0;

			/// Return the version of the filter (used by options etc).
		virtual unsigned int getFilterVersion() const = 0; 
	
			/// Must return a mask of HavokComponent values matching the SDK components (libraries) required to use/load
			/// the output of this filter.
		virtual HavokComponentMask getRequiredHavokComponents () const = 0;

			/// Create the filter. The pointer returned my be memory managed in any way, as 
			/// deletion will be handled through destruct. Do not call delete on the filter.
		virtual class hctFilterInterface* createFilter(const class hctFilterManagerInterface* ownerInstance) const = 0;

			/// Should tell all modeless filters to close and return immediately.
			/// Default implementation (for modal filters) does nothing.
		virtual void askModelessFiltersToClose () const;

			/// Returns how many modeless filters are present. It must split the count between "active" and "closing" down.
			/// Default implementation (for modal filters) should return 0 and set both parameters to 0.
		virtual int countModelessFilters (int& numActiveOut, int& numClosingout) const;

};


#endif // HAVOK_FILTER_DESCRIPTOR_H

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
