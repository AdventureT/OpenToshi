/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_CHARACTER_CONTROLLER_H
#define HK_CHARACTER_CONTROLLER_H

#include <Physics/Dynamics/Motion/hkpMotion.h>

/// Surface information returned from user queries with hkRigidBodyCharacterProxy::checkSupport.
struct hkpSurfaceInfo
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_UTILITIES, hkpSurfaceInfo );

	/// The supported state of the character
	enum SupportedState
	{
		/// This state implies there are no surfaces underneath the character.
		UNSUPPORTED = 0,

		/// This state means that there are surfaces underneath the character, but they are too
		/// steep to prevent the character sliding downwards.
		SLIDING = 1,

		/// This state means the character is supported, and will not slide.
		SUPPORTED = 2
	};

	/// The supported state of the character.
	SupportedState m_supportedState;

	/// The average surface normal in this given direction
	hkVector4	m_surfaceNormal;	

	/// The average surface velocity
	hkVector4	m_surfaceVelocity;

	/// The average surface distance
	hkReal	m_surfaceDistance;

	/// The type of surface (motion type of contact object).
	hkpMotion::MotionType	m_surfaceMotionType; 

	/// Constructors
	hkpSurfaceInfo()
	{
		m_supportedState = SUPPORTED; 
		m_surfaceNormal.set(0,0,1,0);
		m_surfaceVelocity.set(0,0,0,0);
		m_surfaceDistance = 0;
		m_surfaceMotionType = hkpMotion::MOTION_FIXED;
	}	

	hkpSurfaceInfo(const hkVector4& up, const hkVector4& velocity = hkVector4::getZero(), const SupportedState state = SUPPORTED, const hkpMotion::MotionType type = hkpMotion::MOTION_FIXED)
	{
		m_supportedState = state; 
		m_surfaceNormal = up;
		m_surfaceVelocity = velocity;
		m_surfaceDistance = 0;
		m_surfaceMotionType = type;
	}	

	/// Set structure from other 
	inline void set(const hkpSurfaceInfo& other)
	{
		m_supportedState = other.m_supportedState;
		m_surfaceNormal = other.m_surfaceNormal;
		m_surfaceVelocity = other.m_surfaceVelocity;
		m_surfaceDistance = other.m_surfaceDistance;
		m_surfaceMotionType = other.m_surfaceMotionType;

	}
};

/// Surface information returned from user queries with hkpCharacterProxy::checkSupportDeprecated
struct hkpSurfaceInfoDeprecated
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_UTILITIES, hkpSurfaceInfoDeprecated );

	/// Am I supported by the surface. i.e. is there any surface of any slope in this direction
	hkBool		m_isSupported;

	/// Am I sliding
	hkBool		m_isSliding;		

	/// The average surface normal in this given direction
	hkVector4	m_surfaceNormal;	

	/// The average surface velocity
	hkVector4	m_surfaceVelocity;	
};

#endif //HK_CHARACTER_CONTROLLER_H

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
