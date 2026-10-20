/* 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_FILTER_OPTION_REGISTRY_UTILS_H
#define HK_FILTER_OPTION_REGISTRY_UTILS_H

	/// This class provides utilities used to access the windows registry - it is used to store processing options
	/// associated to the user/machine rather than the asset. For example, this is used by the Preview Scene
	/// filter in order to store options regarding scene navigation and interaction.
class hctOptionsRegistry
{
	public:

			/// Constructor - takes the name of the filter, which will be used in order to construct the key used
			/// to store/read the values in the registry.
		hctOptionsRegistry (const char* filterName);

			/// Returns an integer value associated with "option" stored in the registry. If not found, it returns iDefault.
		int getIntWithDefault (const char* option, int iDefault);

			/// Stores an integer value associated with "option" in the registry.
		void setInt (const char* option, int i);

			/// Returns a boolean value associated with "option" stored in the registry. If not found, it returns bDefault.		
		bool getBoolWithDefault (const char* option, bool bDefault);

			/// Stores a boolean value associated with "option" in the registry.
		void setBool (const char* option, bool b);

	private:

		HKEY m_registryKey;
};

#endif // HK_FILTER_OPTION_REGISTRY_UTILS_H

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
