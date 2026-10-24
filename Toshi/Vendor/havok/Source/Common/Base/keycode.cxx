/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <Common/Base/KeyCode.h>

#if defined(HK_PLATFORM_GC)
#	define KEYCODE_ATTRIBUTES __attribute__((section(".sdata")))
#else
#	define KEYCODE_ATTRIBUTES
#endif

static int hkGetKeyValue(const char* keyCode)
{
	char keyValue[11];
	int i=0;
	for (; (i < 10) && keyCode[i] && (keyCode[i] != ':'); i++)
		keyValue[i] = keyCode[i];
	keyValue[i] = '\0';

	return hkString::atoi(keyValue, 16);
}

//
// PHYSICS
//
extern const char         HK_PHYSICS_KEYCODE[] KEYCODE_ATTRIBUTES = HAVOK_PHYSICS_KEYCODE;
extern const unsigned int HK_PHYSICS_KEYVALUE  KEYCODE_ATTRIBUTES = hkGetKeyValue(HAVOK_PHYSICS_KEYCODE);

//
// ANIMATION
//
extern const char         HK_ANIMATION_KEYCODE[] KEYCODE_ATTRIBUTES = HAVOK_ANIMATION_KEYCODE;
extern const unsigned int HK_ANIMATION_KEYVALUE  KEYCODE_ATTRIBUTES = hkGetKeyValue(HAVOK_ANIMATION_KEYCODE);

//
// BEHAVIOR
//
extern const char         HK_BEHAVIOR_KEYCODE[] KEYCODE_ATTRIBUTES = HAVOK_BEHAVIOR_KEYCODE;
extern const unsigned int HK_BEHAVIOR_KEYVALUE  KEYCODE_ATTRIBUTES = hkGetKeyValue(HAVOK_BEHAVIOR_KEYCODE);

//
// CLOTH
//
extern const char         HK_CLOTH_KEYCODE[] KEYCODE_ATTRIBUTES = HAVOK_CLOTH_KEYCODE;
extern const unsigned int HK_CLOTH_KEYVALUE  KEYCODE_ATTRIBUTES = hkGetKeyValue(HAVOK_CLOTH_KEYCODE);

//
// DESTRUCTION
//
extern const char         HK_DESTRUCTION_KEYCODE[] KEYCODE_ATTRIBUTES = HAVOK_DESTRUCTION_KEYCODE;
extern const unsigned int HK_DESTRUCTION_KEYVALUE  KEYCODE_ATTRIBUTES = hkGetKeyValue(HAVOK_DESTRUCTION_KEYCODE);



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
