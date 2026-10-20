/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

//
// PHYSICS
//
#if defined( HAVOK_PHYSICS_KEYCODE )
#define USING_HAVOK_PHYSICS
#else
#define HAVOK_PHYSICS_KEYCODE ""
#endif

//
// ANIMATION
//
#if defined( HAVOK_ANIMATION_KEYCODE )
#define USING_HAVOK_ANIMATION
#else
#define HAVOK_ANIMATION_KEYCODE ""
#endif

//
// BEHAVIOR
//
#if defined( HAVOK_BEHAVIOR_KEYCODE )
#define USING_HAVOK_BEHAVIOR
#else
#define HAVOK_BEHAVIOR_KEYCODE ""
#endif

//
// CLOTH
//
#if defined( HAVOK_CLOTH_KEYCODE )
#define USING_HAVOK_CLOTH
#else
#define HAVOK_CLOTH_KEYCODE ""
#endif

//
// DESTRUCTION
//
#if defined( HAVOK_DESTRUCTION_KEYCODE )
#define USING_HAVOK_DESTRUCTION
#else
#define HAVOK_DESTRUCTION_KEYCODE ""
#endif

 
 
 
 
 
 
 
 

//
// Raise an error if no keycodes are set.
//
#if !defined(USING_HAVOK_PHYSICS) && !defined(USING_HAVOK_ANIMATION) && !defined(USING_HAVOK_BEHAVIOR) && !defined(USING_HAVOK_CLOTH) && !defined(USING_HAVOK_DESTRUCTION) 
#error No keycodes found. Please enter your keycode(s) in <Common\Base\KeyCode.h>
#endif

//
// Product dependency checks:
//
#if defined(USING_HAVOK_BEHAVIOR) && !defined(USING_HAVOK_PHYSICS)
#error "Can't use BEHAVIOR without PHYSICS. Please add your PHYSICS keycode to <Common\Base\KeyCode.h>"
#endif

#if defined(USING_HAVOK_BEHAVIOR) && !defined(USING_HAVOK_ANIMATION)
#error "Can't use BEHAVIOR without ANIMATION. Please add your ANIMATION keycode to <Common\Base\KeyCode.h>"
#endif

#if defined(USING_HAVOK_DESTRUCTION) && !defined(USING_HAVOK_PHYSICS)
#error "Can't use DESTRUCTION without PHYSICS. Please add your PHYSICS keycode to <Common\Base\KeyCode.h>"
#endif


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
