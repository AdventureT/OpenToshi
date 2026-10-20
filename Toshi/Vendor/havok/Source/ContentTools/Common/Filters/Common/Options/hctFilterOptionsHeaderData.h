/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKFILTERMANAGER_HKFILTEROPTIONSHEADERDATA_HKCLASS_H
#define HKFILTERMANAGER_HKFILTEROPTIONSHEADERDATA_HKCLASS_H

/// hctFilterData meta information.
extern const class hkClass hctFilterDataClass;
/// hctConfigurationData meta information.
extern const class hkClass hctConfigurationDataClass;
/// hctConfigurationSetData meta information.
extern const class hkClass hctConfigurationSetDataClass;

/// This class describes a filter; it is used internally by the filter manager as a header for each filter options.
class hctFilterData
{
	public:

		HK_DECLARE_REFLECTION();

		// Struct holding default values

		struct DefaultStruct;

			/// Default constructor
		hctFilterData() { }

		//
		// Members
		//
	public:
			/// The filter's unique id.
		unsigned m_id;
			/// The version of the filter.
		unsigned m_ver;
			/// Whether the filter has options or not.
		hkBool m_hasOptions;
};

/// This class describes a single configuration ; it is used internally by the filter manager.
class hctConfigurationData
{
	public:

		HK_DECLARE_REFLECTION();

		// Struct holding default values

		struct DefaultStruct;

			/// Default constructor
		hctConfigurationData() { }

		//
		// Members
		//
	public:
			/// The name of the configuration.
		const char* m_configurationName;

			/// The number of filters in the configuration.
		int m_numFilters;
};

/// This class describes a configuration set - the information used internally by the filter manager and stored 
/// with the max/maya/xsi asset.
class hctConfigurationSetData
{
	public:

		HK_DECLARE_REFLECTION();

		// Struct holding default values

		struct DefaultStruct;

			/// Default constructor
		hctConfigurationSetData() { }

		//
		// Members
		//
	public:
			/// The filter manager version used to create the configuration set.
		unsigned m_filterManagerVersion;
			/// The configuration to display in the filter manager when the set is loaded.
		int m_activeConfiguration;
};

#endif // HKFILTERMANAGER_HKFILTEROPTIONSHEADERDATA_HKCLASS_H

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
