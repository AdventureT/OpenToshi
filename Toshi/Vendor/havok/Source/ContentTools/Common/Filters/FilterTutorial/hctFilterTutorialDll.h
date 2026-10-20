/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_TUTORIAL_DLL_H
#define HAVOK_FILTER_TUTORIAL_DLL_H

#include <ContentTools/Common/Filters/Common/Filter/hctFilterDll.h>

// This class describes our filter DLL : It provides access to the filter descriptor
// for each filter implemented in the DLL, and registers all classes possibly created in those
// filters.
class hctFilterTutorialDll : public hctFilterDll
{
	public:

		hctFilterTutorialDll (HMODULE dllModule);

		// Must return how many types of filters are implemented in this DLL
		/*virtual*/ int getNumberOfFilters() const;

		// For each filter implemented in this DLL, we must return a filter descriptor
		/*virtual*/ hctFilterDescriptor* getFilterDescriptor (int index) const;

		// Since our filter creates objects, we must register our classes
		/*virtual*/ void registerClasses (hctFilterClassRegistry& classReg) const;

		// There are some statics in the Physics SDK that we want to make sure are initialized
		/*virtual*/ void initDll ( const BaseSystemInfo* baseSystemInfo );

};


// Function exported by the DLL
__declspec( dllexport ) hctFilterDll* getFilterDll (HMODULE dllModule);

#endif // HAVOK_FILTER_TUTORIAL_DLL_H

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
