/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_CHARACTER_RIGID_BODY_CINFO__H
#define HK_CHARACTER_RIGID_BODY_CINFO__H

/// Character controller cinfo
struct hkpCharacterRigidBodyCinfo
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CHARACTER, hkpCharacterRigidBodyCinfo );

	//
	// Values used to set up rigid body
	//

		/// The collision filter info
		/// See hkRigidBodyCinfo for details
	hkUint32 m_collisionFilterInfo;

		/// The shape
		/// See hkRigidBodyCinfo for details
	hkpShape* m_shape;	

		/// Initial position
		/// See hkRigidBodyCinfo for details
	hkVector4 m_position;

		/// Initial rotation
		/// See hkRigidBodyCinfo for details
	hkQuaternion m_rotation;

		///	The mass of character
		/// See hkRigidBodyCinfo for details
	hkReal m_mass;	

		/// Set friction of character
		/// See hkRigidBodyCinfo for details
	hkReal m_friction;

		/// Set maximal linear velocity
		/// See hkRigidBodyCinfo for details
	hkReal m_maxLinearVelocity;

		/// Set maximal allowed penetration depth
		/// See hkRigidBodyCinfo for details
	hkReal m_allowedPenetrationDepth;


	//
	// Character controller specific values
	//

		/// Set up direction 
	hkVector4 m_up;	

		/// Set maximal slope
	hkReal m_maxSlope;

		/// Set maximal force of character
	hkReal m_maxForce;


	//
	// Parameters used by checkSupport
	//

		/// Set maximal speed for simplex solver
	hkReal m_maxSpeedForSimplexSolver;

		/// A character is considered supported if it is less than this distance above its supporting planes.
	hkReal m_supportDistance;

		/// A character should keep falling until it is this distance or less from its supporting planes.
	hkReal m_hardSupportDistance;

		/// Set color of character for the visual debugger
	hkInt32	m_vdbColor;

		/// Constructor. Sets some defaults.
	hkpCharacterRigidBodyCinfo()
	{
		m_mass = 100.0f;
		m_maxForce = 1000.0f;
		m_friction = 0.0f;
		m_maxSlope = HK_REAL_PI / 3.0f;
		m_up.set( 0,1,0 );
		m_maxLinearVelocity = 20.0f;
		m_allowedPenetrationDepth = -0.1f;
		m_maxSpeedForSimplexSolver = 10.0f;
		m_collisionFilterInfo = 0;
		m_position.setZero4();
		m_rotation.setIdentity();
		m_supportDistance = 0.1f;
		m_hardSupportDistance = 0.0f;
		
		m_vdbColor = 0xA0FF0000;
	}
};

#endif // HK_CHARACTER_RIGID_BODY_CINFO__H

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090704)
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
