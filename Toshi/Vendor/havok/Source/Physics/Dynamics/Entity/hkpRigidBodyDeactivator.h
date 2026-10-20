/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_RIGIDBODY_DEACTIVATOR_H
#define HK_DYNAMICS2_RIGIDBODY_DEACTIVATOR_H

#include <Physics/Dynamics/Entity/hkpEntityDeactivator.h>

extern const hkClass hkpRigidBodyDeactivatorClass;

	/// An empty class, purely for type safety
class hkpRigidBodyDeactivator : public hkpEntityDeactivator
{
	public:

		HK_DECLARE_REFLECTION();


			/// A list of all rigid body deactivator types. The deactivator type of a
			/// hkpRigidBody determines which mechanism Havok will use to classify the body as
			/// deactivated. This deactivator actually works on a per island basis. That means
			/// if all deactivators try to deactivate all entities in a given island, the
			/// island will be turned off. 	
		enum DeactivatorType
		{
				/// 
			DEACTIVATOR_INVALID,
				/// This will force the rigid body to never deactivate.
			DEACTIVATOR_NEVER,

				/// Tells Havok to use a spatial deactivation scheme. This makes use of high and low
				/// frequencies of positional motion to determine when deactivation should occur.
			DEACTIVATOR_SPATIAL,
				/// 
			DEACTIVATOR_MAX_ID
		};

			/// Get the type of this deactivator.
		virtual enum DeactivatorType getRigidBodyDeactivatorType() const = 0;

		hkpRigidBodyDeactivator(hkFinishLoadedObjectFlag flag) : hkpEntityDeactivator(flag) {}

	protected:

		hkpRigidBodyDeactivator() {}
};

#endif // HK_DYNAMICS2_RIGID_BODYDEACTIVATOR_H

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
