/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_RESPONSE_MODIFIER_H
#define HK_DYNAMICS2_RESPONSE_MODIFIER_H

class hkpDynamicsContactMgr;
class hkpRigidBody;

/// This class contains a number of functions to allow altering the behavior of collision response
/// This class has a number of restrictions:
///    - Each modifier can only exist once. E.g. setting the mass scaling overrides previous mass scaling modifiers
///    - These modifiers change the constraint hierarchy, so you need access to the hkpConstraintOwner
///      You can get an hkpConstraintOwner in two ways:
///             -  by getting write access to the world (hkpWorld::lock()) and simply using the non fixed island of one of the entities
///					(a hkpSimulationIsland inherits from hkpConstraintOwner)
///				-  if called from a collision callback simply use event.m_output.m_constraintOwner or event.m_constraintOwner
///		- Modifiers will not be serialized
class hkpResponseModifier
{
	public:

			/// Scales the inverse mass of both bodies by the relevant scaling factors.
			/// \param factorA the modification of the inverse mass of body A. The first three components modify the body's
			/// inertia in its principal axes and the fourth modifies its mass.
			/// \param factorB the modification of the inverse mass of body B.
			/// Note: Passing in the zero vector as factor makes the entity "fixed" for the purpose of resolving this contact constraint.
			/// The constraint owner typically is the island of the involved objects. However if you call this function from a collision
			/// callback, you need to use event.m_collisionData.m_constraintOwner
		static void HK_CALL setInvMassScalingForContact( hkpDynamicsContactMgr* manager, hkpRigidBody* bodyA, hkpRigidBody* bodyB, hkpConstraintOwner& constraintOwner, const hkVector4& factorA, const hkVector4& factorB );

			/// Scales the inverse mass of one body by the relevant scaling factor, leaving the scaling factor for the other body untouched.
			/// \param factor the modification of the inverse mass. The first three components modify the body's inertia in its
			/// principal axes and the fourth modifies its mass.
			/// Note: Passing in the zero vector as factor makes the entity "fixed" for the purpose of resolving this contact constraint.
			/// The constraint owner typically is the island of the involved objects. However if you call this function from a collision
			/// callback, you need to use event.m_collisionData.m_constraintOwner
		static void HK_CALL setInvMassScalingForContact( hkpDynamicsContactMgr* manager, hkpRigidBody* body, hkpConstraintOwner& constraintOwner, const hkVector4& factor );

			/// Moves the center of mass of each object by a distance.
			///
			/// The constraint owner typically is the island of the involved objects. However if you call this function from a collision
			/// callback, you need to use event.m_collisionData.m_constraintOwner
		static void HK_CALL setCenterOfMassDisplacementForContact( hkpDynamicsContactMgr* manager, hkpRigidBody* bodyA, hkpRigidBody* bodyB, hkpConstraintOwner& constraintOwner, const hkVector4& displacementA, const hkVector4& displacementB );

			/// Scales and clips the force applied by a contact. 
			///
			/// usedImpulseFraction scales the forces, e.g. using values less than 0.2f yields noticeably softer contact response
			/// and allows object to penetrate. They do get a softer/springy behavior.
			/// The parameter maxAcceleration clips the maximum forced used. This parameter does not introduce springyness.
			/// Try different combinations of the parameters to get the desired effect.
			/// 
			/// The constraint owner typically is the island of the involved objects. However if you call this function from a collision
			/// callback, you need to use event.m_collisionOutput.m_constraintOwner
		static void HK_CALL setImpulseScalingForContact( hkpDynamicsContactMgr* manager, hkpRigidBody* bodyA, hkpRigidBody* bodyB, hkpConstraintOwner& constraintOwner, hkReal usedImpulseFraction, hkReal maxAcceleration );

			/// sets the surfaceVelocity
		static void HK_CALL setSurfaceVelocity( hkpDynamicsContactMgr* manager, hkpRigidBody* body, hkpConstraintOwner& constraintOwner, const hkVector4& velWorld );

			/// removes the surfaceVelocity
			/// NOTE: ONLY USE THIS FUNCTION AFTER CALLING setSurfaceVelocity
		static void HK_CALL clearSurfaceVelocity( hkpDynamicsContactMgr* manager, hkpConstraintOwner& constraintOwner, hkpRigidBody* body );

			/// sets the surface to have low viscosity. Effectively disables static friction.
		static void HK_CALL setLowSurfaceViscosity( hkpDynamicsContactMgr* manager, hkpConstraintOwner& constraintOwner );

			/// Marks the constraint to not go into the solver. 
			/// A disabled constraint is still in the world, and still ensures the linked bodies are in the same hkpSimulationIsland.
		static void HK_CALL disableConstraint(hkpConstraintInstance* instance, hkpConstraintOwner& constraintOwner);

			/// Marks the constraint to go into the solver.
		static void HK_CALL enableConstraint(hkpConstraintInstance* instance, hkpConstraintOwner& constraintOwner);

};

#endif		// HK_DYNAMICS2_RESPONSE_MODIFIER_H

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
