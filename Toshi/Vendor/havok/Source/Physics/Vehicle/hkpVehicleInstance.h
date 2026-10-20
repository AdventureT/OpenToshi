/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKVEHICLE_HKVEHICLEINSTANCE_H
#define HKVEHICLE_HKVEHICLEINSTANCE_H

#include <Physics/Dynamics/Action/hkpUnaryAction.h>

#include <Common/Base/hkBase.h>

class hkpVehicleAerodynamics;
class hkpVehicleDriverInput;
class hkpVehicleBrake;
class hkpVehicleEngine;
class hkpVehicleSteering;
class hkpVehicleSuspension;
class hkpVehicleTransmission;
class hkpVehicleWheelCollide;

#include <Physics/Vehicle/hkpVehicleData.h>

class hkpVelocityAccumulator;
class hkStepInfo;


/// This is the main class for a vehicle - it is a container for all the
/// runtime data the vehicle needs, and also contains pointers to all the
/// components that can be shared between different vehicles.
/// This class cannot be shared between different vehicles.
/// 
/// Because this class is an hkpUnaryAction, it can be added to the world 
/// as an action.  See the VehicleApi demos for examples of usage.
class hkpVehicleInstance : public hkpUnaryAction
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VEHICLE);
		HK_DECLARE_REFLECTION();
	
			/// This structure stores all data that is useful to the user and is shared
			/// among vehicle components.
		struct WheelInfo
		{
			//+version(1)
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_VEHICLE, hkpVehicleInstance::WheelInfo );
			HK_DECLARE_REFLECTION();

				/// The maximum number of shape keys which is reported by vehicle wheel collider (see also hkpShapeRayCastOutput::MAX_HIERARCHY_DEPTH)
			enum { MAX_NUM_SHAPE_KEYS = 8  };	

				/// The point of contact of the wheel with the ground (if the wheel is in contact with the ground).
			class hkContactPoint m_contactPoint;

				/// The friction coefficient at the point of contact.
			hkReal m_contactFriction;

				/// The ground body the vehicle is in contact.  This value is HK_NULL
				/// if none of the wheels are in contact with the ground.
			hkpRigidBody* m_contactBody; //+nosave

				/// The shapeKey hierarchy of the object at the point of contact.
			hkpShapeKey m_contactShapeKey[hkpVehicleInstance::WheelInfo::MAX_NUM_SHAPE_KEYS];

				/// Location of the wheel hardPoint in world space
			hkVector4 m_hardPointWs;

				/// Farthest point on the wheel from the vehicle in the direction of the suspension.
			hkVector4 m_rayEndPointWs;

				/// Current length of the suspension.
			hkReal m_currentSuspensionLength;

				/// Current direction of the suspension (in world space).
			hkVector4 m_suspensionDirectionWs;

				/// Axis relative to the chassis that the wheel is spinning around.
			hkVector4 m_spinAxisChassisSpace;
			
				/// Axis in world space that the wheel is spinning around.
			hkVector4 m_spinAxisWs;

				/// The current rotation of the steering wheel.
			hkQuaternion m_steeringOrientationChassisSpace;
			
				/// The current spin velocity of the wheel in rad/s.
			hkReal m_spinVelocity;
			
				/// The current spin angle of the wheel in rads.
			hkReal m_spinAngle;
						
				/// The energy density lost when skidding (useful to implement tyre marks or skid sounds).
			hkReal m_skidEnergyDensity;
			
				/// The side force at the particular wheel.
			hkReal m_sideForce;
			
				/// The forward velocity lost by this particular wheel when slipping. 
				/// This is the difference between the linear velocity of the wheels 
				/// (angular velocity projected) and the actual velocity of the vehicle
			hkReal m_forwardSlipVelocity;
			
				/// The slip velocity in the side direction for the particular wheel.
			hkReal m_sideSlipVelocity;

			void init();
		};


			/// Default constructor, the chassis rigid body must be constructed already.
		hkpVehicleInstance( hkpRigidBody* chassis );

		~hkpVehicleInstance();
		
		//
		// Methods
		//
		
			/// Call to initialize any data that is derived from the initially setup data, such
			/// as the number of wheels on each axle.
		virtual void init();

			/// Applies the vehicle controller at a given time / timestep. This function
			/// updates the physical state of the vehicle.
		virtual void applyAction(const class hkStepInfo& stepInfo);

			/// Clone functionality from hkpAction. Will make a new vehicle instance
			/// sharing as much data as it can.
		virtual hkpAction* clone( const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms ) const;

			/// This forwards to hkpVehicleWheelCollide::getPhantoms
		virtual void getPhantoms( hkArray<hkpPhantom*>& phantomsOut );


		//
		// Functions to calculate useful information.
		//
		
			/// Calculate the current position and rotation of a wheel for the graphics engine.
		virtual void calcCurrentPositionAndRotation( const hkpRigidBody* chassis, const hkpVehicleSuspension* suspension, int wheelNo, hkVector4& posOut, hkQuaternion& rotOut );

			/// Retrieves the current RPM of the vehicle
		virtual hkReal calcRPM() const;
		
			/// Retrieves the current speed of the vehicle in KM/H
		virtual hkReal calcKMPH() const;
		
			/// Retrieves the current speed of the vehicle in MP/H
		virtual hkReal calcMPH() const;
		
			/// Retrieve the velocity for a fixed ground object.
			/// You can override this function if you want to assign velocity to ground objects
			/// like speed pads
		virtual void handleFixedGroundAccum( hkpRigidBody* ground, hkpVelocityAccumulator& accum );
		
		//
		// Internal functions
		//
	protected:
		void updateWheelsHardPoints(const hkTransform &car_transform);

		//
		// Members
		//
	public:

			/// Contains data about the vehicle that can be shared across several vehicles.
		class hkpVehicleData* m_data;
		
			/// The DriverInput  for the vehicle.
		class hkpVehicleDriverInput* m_driverInput;
		
			/// The Steering  for the vehicle.
		class hkpVehicleSteering* m_steering;
		
			/// The Engine  for the vehicle.
		class hkpVehicleEngine* m_engine;
		
			/// The Transmission  for the vehicle.
		class hkpVehicleTransmission* m_transmission;
		
			/// The Brake  for the vehicle.
		class hkpVehicleBrake* m_brake;
		
			/// The Suspension  for the vehicle.
		class hkpVehicleSuspension* m_suspension;
		
			/// The Aerodynamics  for the vehicle.
		class hkpVehicleAerodynamics* m_aerodynamics;
		
			/// The Collision  for the vehicle.
		class hkpVehicleWheelCollide* m_wheelCollide;
		
			/// The Tyremarks controller  for the vehicle.
		class hkpTyremarksInfo* m_tyreMarks;

			/// The list of external vehicle controllers.
		class hkpVehicleVelocityDamper* m_velocityDamper;
	
			/// The WheelInfo class holds all wheel information generated externally (from the
			/// physics engine) such as each wheel's ground contact, sliding state, forces,
			/// contact friction etc. Since all information is shared between several
			/// s, none of it does not belong in any one. 	
		hkArray<struct WheelInfo> m_wheelsInfo;

		struct hkpVehicleFrictionStatus m_frictionStatus;

		
		//
		// Variables used by the components to cache data.
		// This is slightly ugly, but necessary until it is possible until a 
		// decent runtime cache manager can be constructed.
		//
		

		///	Current controller input state.
		class hkpVehicleDriverInputStatus* m_deviceStatus; 

		//		from brake
		hkArray<hkBool> m_isFixed;
		hkReal m_wheelsTimeSinceMaxPedalInput;

		//		from driver input
		hkBool m_tryingToReverse;

		//		from engine
		hkReal m_torque;
		hkReal m_rpm;

		//		from steering
		hkReal m_mainSteeringAngle;
		hkArray<hkReal> m_wheelsSteeringAngle;

		//		from transmission
		hkBool m_isReversing;
		hkInt8 m_currentGear;
		hkBool m_delayed;
		hkReal m_clutchDelayCountdown;

	public:

		/// Get a pointer the vehicle's chassis.
		inline class hkpRigidBody* getChassis ( void ) const;

		/// Set the chassis of the vehicle.
		/// This should only be called before the vehicle has been added to the
		/// world.
		void setChassis ( class hkpRigidBody* chassis );
	
		hkpVehicleInstance( hkFinishLoadedObjectFlag f ) : hkpUnaryAction(f), m_wheelsInfo(f), m_frictionStatus(f), m_isFixed(f), m_wheelsSteeringAngle(f) { }

};

#include <Physics/Vehicle/hkpVehicleInstance.inl>

#endif // HKVEHICLE_HKVEHICLEINSTANCE_H

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
