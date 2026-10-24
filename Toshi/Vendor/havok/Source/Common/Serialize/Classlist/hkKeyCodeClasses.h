/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// Build up a classlist based on the Havok products in use.

#include <Common/Base/KeyCode.h>

#ifdef USING_HAVOK_PHYSICS
#	define INCLUDE_HAVOK_PHYSICS_CLASSES
#endif

#ifdef USING_HAVOK_ANIMATION
#	define INCLUDE_HAVOK_ANIMATION_CLASSES
#endif

#ifdef USING_HAVOK_BEHAVIOR
#	define INCLUDE_HAVOK_BEHAVIOR_CLASSES
#endif

#ifdef USING_HAVOK_CLOTH
#	define INCLUDE_HAVOK_CLOTH_CLASSES
#endif

#ifdef USING_HAVOK_DESTRUCTION
#	define INCLUDE_HAVOK_DESTRUCTION_CLASSES
#endif

#include <Common/Serialize/Classlist/hkClasses.h>

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
