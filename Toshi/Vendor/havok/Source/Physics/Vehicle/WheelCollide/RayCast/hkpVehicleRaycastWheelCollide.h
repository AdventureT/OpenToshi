/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKVEHICLE_COLLISIONDETECTION_DEFAULT_hkVehicleRaycastWheelCollide_XML_H
#define HKVEHICLE_COLLISIONDETECTION_DEFAULT_hkVehicleRaycastWheelCollide_XML_H

#include <Physics/Vehicle/hkpVehicleInstance.h>
#include <Physics/Vehicle/WheelCollide/hkpVehicleWheelCollide.h>
#include <Physics/Dynamics/Phantom/hkpPhantomOverlapListener.h>

class hkpAabbPhantom;
struct hkpWorldRayCastOutput;
class hkAabb;

/// An hkpPhantomOverlapListener used to ignore the chassis when doing collision detection.
/// (This inherits from base object so that it can be serialize without
/// the listener interface declared as reflected.)
class hkpRejectRayChassisListener : public hkReferencedObject, public hkpPhantomOverlapListener
{
	public:
	
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VEHICLE);
		HK_DECLARE_REFLECTION();

			/// Default constructor
		hkpRejectRayChassisListener();

		virtual ~hkpRejectRayChassisListener();
		
		//
		// Methods
		//
		
			/// 
		void collidableAddedCallback(const hkpCollidableAddedEvent& event);
		
			/// 
		void collidableRemovedCallback(const hkpCollidableRemovedEvent& event);
		
		//
		// Members
		//
	public:
		
			/// 
		const hkpCollidable* m_chassis; //+nosave

	public:
		hkpRejectRayChassisListener(hkFinishLoadedObjectFlag f) : hkReferencedObject(f) { }
};


/// Default implementation of hkpVehicleWheelCollide - performs a raycast
/// from the wheels to the ground.  This component cannot be shared between vehicles.
class hkpVehicleRaycastWheelCollide : public hkpVehicleWheelCollide
{
	public:

		HK_DECLARE_REFLECTION();

			/// Default constructor
		hkpVehicleRaycastWheelCollide();

		~hkpVehicleRaycastWheelCollide();
		
		//
		// Methods
		//
		
			///
		virtual void init( const hkpVehicleInstance* vehicle );

			/// Calculates information about the effects of colliding the wheels with the ground, on the vehicle.
		virtual void collideWheels(const hkReal deltaTime, hkpVehicleInstance* vehicle, CollisionDetectionWheelOutput* cdInfoOut);
		
			/// Passes back an AABB that encompasses the raycasts. It uses the hardpoints and
			/// suspension length to determine minimum and maximum extents.
		virtual void calcWheelsAABB( const hkpVehicleInstance* vehicle, hkAabb& aabbOut);
		
			/// Use this method to override the default friction value set by the raycast
			/// vehicle updateBodies loop. The default value is the friction of the rigidbody
			/// that the wheel raycast hits Use the hkpWorldRayCastOutput to obtain access to the
			/// shape hit by the raycast
		virtual void calcSingleWheelGroundFriction(hkpVehicleInstance* vehicle, hkInt8 wheelInfoNum, const hkpWorldRayCastOutput& worldRayCastOutput, hkReal& frictionOut) const;
		
			/// Perform raycast for a single wheel. This implementation performs a castRay call
			/// on the hkpAabbPhantom.
		virtual void castSingleWheel(const hkpVehicleInstance::WheelInfo& wheelInfo, hkpVehicleInstance* vehicle, hkpWorldRayCastOutput& output);
		
			/// 
		virtual void updatePhantom(hkpVehicleInstance* vehicle);
		
			///
		virtual hkpVehicleWheelCollide* clone( const hkArray<hkpPhantom*>& newPhantoms ) const;

			///
		virtual void getPhantoms( hkArray<hkpPhantom*>& phantomsOut );

		//
		// Members
		//
	public:
		
		/// Use to disable collision detection with the chassis.
		hkUint32 m_wheelCollisionFilterInfo;
		
		/// The phantom must be added to the world by the user.
		hkpAabbPhantom* m_phantom;
		
		/// This hkpPhantomOverlapListener is added to the phantom to ignore the chassis.
		class hkpRejectRayChassisListener m_rejectRayChassisListener;

	public: 

		hkpVehicleRaycastWheelCollide(hkFinishLoadedObjectFlag f) : hkpVehicleWheelCollide(f), m_rejectRayChassisListener(f) { }
};

#endif // HKVEHICLE_COLLISIONDETECTION_DEFAULT_hkVehicleRaycastWheelCollide_XML_H

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
