/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <ContentTools/Common/Filters/FilterTutorial/hctFilterTutorial.h>

#include <ContentTools/Common/Filters/FilterTutorial/hctFilterTutorialDll.h>

#include <Common/Serialize/Util/hkBuiltinTypeRegistry.h>

#include <Common/Serialize/Version/hkVersionRegistry.h>

#include <Physics/Collide/Util/Welding/hkpWeldingUtility.h>
#include <Physics/Dynamics/World/hkpWorldCinfo.h>


// Filters we want to expose in this dll:
#include <ContentTools/Common/Filters/FilterTutorial/ConvertToPhantomAction/hctConvertToPhantomActionFilter.h>

__declspec( dllexport ) hctFilterDll* getFilterDll (HMODULE dllModule)
{
	static hctFilterTutorialDll gFilterAssetDll (dllModule);

	return &gFilterAssetDll;
}


hctFilterTutorialDll::hctFilterTutorialDll (HMODULE dllModule) : hctFilterDll (dllModule) 
{

}

/*virtual*/ int hctFilterTutorialDll::getNumberOfFilters() const
{ 
	return 1; 
}

/*virtual*/ hctFilterDescriptor* hctFilterTutorialDll::getFilterDescriptor (int index) const
{
	static hctFilterDescriptor* m_filterDescs[] = 
	{
		&g_convertToPhantomActionDesc,
	};

	return m_filterDescs[index];
}

/*virtual*/ void hctFilterTutorialDll::initDll ( const BaseSystemInfo* baseSystemInfo )
{
	hctFilterDll::initDll(baseSystemInfo);

	// HVK-3632
	if (isInitialized())
	{
		hkpWorldCinfo defaults;
		hkpWeldingUtility::initWeldingTable(defaults.m_snapCollisionToConvexEdgeThreshold, defaults.m_snapCollisionToConcaveEdgeThreshold);
	}
}

/*virtual*/ void hctFilterTutorialDll::registerClasses (hctFilterClassRegistry& classReg) const
{
	classReg.registerClasses (hkBuiltinTypeRegistry::StaticLinkedTypeInfos, hkBuiltinTypeRegistry::StaticLinkedClasses);

	#define HK_CUSTOM_CLASS(C) \
		extern const hkTypeInfo C##TypeInfo;					\
		extern const hkClass C##Class;							\
		classReg.registerClass( &(C##TypeInfo), &(C##Class) );	

	// Our phantom class
	HK_CUSTOM_CLASS(MyPhantomShape)
}


// REGISTER ALL COMPLETE CLASSES
#define HK_CLASSES_FILE <Physics/Collide/Classes/hkpCollideClasses.h>
#include <Common/Serialize/Util/hkBuiltinTypeRegistry.cxx>

// NO COMPATIBILITY NEEDED
#include <Common/Compat/hkCompat_None.cxx>

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
