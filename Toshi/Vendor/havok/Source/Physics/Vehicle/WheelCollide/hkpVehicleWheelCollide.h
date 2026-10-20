/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKVEHICLE_COLLISIONDETECTION_hkVehicleCOLLISIONDETECTION_XML_H
#define HKVEHICLE_COLLISIONDETECTION_hkVehicleCOLLISIONDETECTION_XML_H

#include <Common/Base/hkBase.h>

class hkpVehicleInstance;
class hkpRigidBody;
	
/// This component manages the collision detection between the wheels and the
/// ground.
class hkpVehicleWheelCollide : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VEHICLE);
		HK_DECLARE_REFLECTION();
	
			/// Container for data output by the collision calculations.  
		struct CollisionDetectionWheelOutput
		{
			public:
					/// The maximum number of shape keys which is reported (see also hkpShapeRayCastOutput::MAX_HIERARCHY_DEPTH)
				enum { MAX_NUM_SHAPE_KEYS = 8  };	

				HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_VEHICLE, hkpVehicleWheelCollide::CollisionDetectionWheelOutput );
				/// The point of contact of the wheel with the ground.
			class hkContactPoint m_contactPoint;
			
				/// The friction coefficient at the point of contact.
			hkReal m_contactFriction;
			
				/// The ground body the vehicle is in contact.  This value is HK_NULL
				/// if none of the wheels are in contact with the ground.
			hkpRigidBody* m_contactBody;

				/// The shapeKey of the object at the point of contact.
			hkpShapeKey m_contactShapeKey[MAX_NUM_SHAPE_KEYS];

				/// The length of the suspension due to the wheel being in contact at
				/// the given point.
			hkReal m_currentSuspensionLength;

				/// The velocity of the suspension.
			hkReal m_suspensionRelativeVelocity;
				
				/// Scaling factor used to handle curb interaction.
				/// Forces along the contact normal are scaled by this factor.
				/// This ensures that the suspension force component is unscaled.
				/// Clipping is affected by hkpVehicleData::m_normalClippingAngle.
			hkReal m_clippedInvContactDotSuspension;
		};
		
		//
		// Methods
		//
		
			/// The caller of this method pre-allocates cdInfoOut with a buffer the size of the
			/// number of wheels in the vehicle
		virtual void collideWheels(const hkReal deltaTime, hkpVehicleInstance* vehicle, CollisionDetectionWheelOutput* cdInfoOut) = 0;

		virtual void init( const hkpVehicleInstance* vehicle ) = 0;

		virtual void getPhantoms( hkArray<hkpPhantom*>& phantomsOut ) = 0;

			/// As all other parts of the vehicle can usually be shared, except for the wheel collide.
		virtual hkpVehicleWheelCollide* clone( const hkArray<hkpPhantom*>& newPhantoms ) const = 0;

		hkpVehicleWheelCollide(hkFinishLoadedObjectFlag f) : hkReferencedObject(f) {}

	protected:

		hkpVehicleWheelCollide() : m_alreadyUsed(false) {}

	public:
		    /// This component cannot be shared between vehicle instances - this variable
		    /// indicates if a vehicle already owns it.
		hkBool m_alreadyUsed;
};

#endif // HKVEHICLE_COLLISIONDETECTION_hkVehicleCOLLISIONDETECTION_XML_H

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
