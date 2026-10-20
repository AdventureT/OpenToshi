/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// Build up a Havok classlist from the specified products.

#include <Common/Serialize/Classlist/hkCommonClasses.h>

#ifdef INCLUDE_HAVOK_PHYSICS_CLASSES
#  include <Common/Serialize/Classlist/hkPhysicsClasses.h>
#endif

#ifdef INCLUDE_HAVOK_ANIMATION_CLASSES
#  include <Common/Serialize/Classlist/hkAnimationClasses.h>
#endif

#ifdef INCLUDE_HAVOK_BEHAVIOR_CLASSES
#  include <Common/Serialize/Classlist/hkBehaviorClasses.h>
#endif

#ifdef INCLUDE_HAVOK_CLOTH_CLASSES
#  include <Common/Serialize/Classlist/hkClothClasses.h>
#endif

#ifdef INCLUDE_HAVOK_DESTRUCTION_CLASSES
#  include <Common/Serialize/Classlist/hkDestructionClasses.h>
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
