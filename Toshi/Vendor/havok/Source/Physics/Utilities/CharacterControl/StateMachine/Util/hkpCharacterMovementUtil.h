/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_CHARACTER_MOVEMENT_UTIL_H
#define HK_CHARACTER_MOVEMENT_UTIL_H

#include <Common/Base/hkBase.h>

/// This utility class calculates an output velocity in world space given a moving surface frame
/// and a desired velocity in this frame. This code is used in many of the states to control movement
class hkpCharacterMovementUtil
{
	public:

		/// The struct passed as input to calculate the new character velocity. 
		struct hkpMovementUtilInput
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_UTILITIES, hkpCharacterMovementUtil::hkpMovementUtilInput );

			/// Gain for the character controller.
			/// This variable controls the acceleration of the character. It should be
			/// scaled by the current timestep to ensure that the characters acceleration
			/// is not timestep dependent.
			hkReal    m_gain;

			/// Forward direction in world space
			hkVector4 m_forward;

			/// Up direction in world space
			hkVector4 m_up;

			/// Normal of the surface we're standing on in world space
			hkVector4 m_surfaceNormal;

			/// Our current velocity in world space
			hkVector4 m_currentVelocity;

			/// Our desired velocity in the surface frame
			hkVector4 m_desiredVelocity;

			/// Limit the maximum acceleration of the character
			hkReal	  m_maxVelocityDelta;

			/// Velocity of the surface we're standing on in world space
			hkVector4 m_surfaceVelocity;
		};

		/// Calculate a new output velocity based on the input
		static void HK_CALL calculateMovement(const hkpMovementUtilInput& input, hkVector4& velocityOut);

};

#endif // HK_CHARACTER_MOVEMENT_UTIL_H

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
