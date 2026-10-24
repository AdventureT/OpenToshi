/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_SPATIAL_RIGID_BODY_DEACTIVATOR_H
#define HK_DYNAMICS2_SPATIAL_RIGID_BODY_DEACTIVATOR_H

#include <Physics/Dynamics/Entity/hkpRigidBodyDeactivator.h>

class hkpRigidBody;

extern const hkClass hkpSpatialRigidBodyDeactivatorClass;

	/// The default deactivator. You do not need to create one of these objects
	/// by hand. One of these objects is automatically created on construction of each hkpRigidBody.
class hkpSpatialRigidBodyDeactivator : public hkpRigidBodyDeactivator
{
	public:
		HK_DECLARE_REFLECTION();		

			/// Constructor takes the radius, and initializes all other values to the specified defaults.
		hkpSpatialRigidBodyDeactivator();

		virtual enum DeactivatorType getRigidBodyDeactivatorType() const;

		virtual hkBool shouldDeactivateHighFrequency( const hkpEntity* entity );

		virtual hkBool shouldDeactivateLowFrequency( const hkpEntity* entity );

	public:

		struct Sample
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkpSpatialRigidBodyDeactivator::Sample );
			HK_DECLARE_REFLECTION();
			
			hkVector4    m_refPosition;
			hkQuaternion m_refRotation;
		};

	protected:

		struct Sample m_highFrequencySample;
		struct Sample m_lowFrequencySample;

	
	public:

			/// The squared radius of the object that this deactivator is being used on. Can be an approximation.
			/// This is set automatically by the owner rigid body upon construction.
			/// Kept for hk30 serialization compatibility. Member not used.
		hkReal m_radiusSqrd; 

			/// High frequency translational deactivation metric. Best left at default value of
			/// 0.01f, and should be higher than 0. Entity will never be deactivated if value is 0.
			/// Higher value makes the deactivator more aggressive.
			/// This is set automatically by the owner rigid body upon construction.
		hkReal m_minHighFrequencyTranslation;

			/// High frequency rotation deactivation metric. Best left at default value of 0.005f,
			/// and should be higher than 0. Entity will never be deactivated if value is 0.
			/// Higher value makes the deactivator more aggressive.
			/// This is set automatically by the owner rigid body upon construction.
		hkReal m_minHighFrequencyRotation;

			/// Low frequency translational deactivation metric. Best left at default value of 0.1f,
			/// and should be higher than 0. Entity will never be deactivated if value is 0.
			/// Higher value makes the deactivator more aggressive, the value should be higher then
			/// the value for the high frequency translation.
			/// Low frequency deactivation is seldom used in practice - in most situations high
			/// frequency deactivation is actually what deactivates rigid bodies.  Low frequency
			/// deactivation is used if for some reason a rigid body does not settle fully, and has
			/// a residual "jitter" that prevents the high frequency deactivation from deactivating the
			/// object.
			/// This is set automatically by the owner rigid body upon construction.
		hkReal m_minLowFrequencyTranslation;

			/// Low frequency rotation deactivation metric. Best left at default value of 0.2f,
			/// and should be higher than 0. Entity will never be deactivated if value is 0.
			/// Higher value makes the deactivator more aggressive, the value should be higher then
			/// the value for the high frequency rotation.
			/// Low frequency deactivation is seldom used in practice - in most situations high
			/// frequency deactivation is actually what deactivates rigid bodies.  Low frequency
			/// deactivation is used if for some reason a rigid body does not settle fully, and has
			/// a residual "jitter" that prevents the high frequency deactivation from deactivating the
			/// object.
			/// This is set automatically by the owner rigid body upon construction.
		hkReal m_minLowFrequencyRotation;

	protected:

		void initFrequencies();
		void initSamples();

	public:

		hkpSpatialRigidBodyDeactivator( class hkFinishLoadedObjectFlag flag ) :
			hkpRigidBodyDeactivator(flag) {}
};



#endif // HK_DYNAMICS2_SPATIAL_RIGID_BODY_DEACTIVATOR_H

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
