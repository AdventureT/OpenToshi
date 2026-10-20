/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_VERSION_FILE
#	error Please define HK_VERSION_FILE before including this file
#endif
#include <Common/Serialize/hkSerialize.h>

#include <Common/Serialize/Version/hkVersionRegistry.h>

class hkClass;

#define HK_VERSION(CLASSNAME, TAG0, TAG1, FUNC) \
	extern hkUint32 CLASSNAME ## _ ## TAG0; \
	extern hkUint32 CLASSNAME ## _ ## TAG1; \
	extern void HK_CALL FUNC(hkVersionedObject& object, hkPointerMap<void*,void*>& updateMap);
#include HK_VERSION_FILE
#undef HK_VERSION


#define HK_VERSION(CLASSNAME, TAG0, TAG1, FUNC) \
	{ #CLASSNAME, CLASSNAME ## _ ## TAG0, CLASSNAME ## _ ## TAG1, FUNC },
const hkVersionRegistry::Entry hkVersionRegistry::StaticLinkedEntries[] = 
{
#	include HK_VERSION_FILE
	{ HK_NULL, 0, 0, HK_NULL }
};
#undef HK_VERSION

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
