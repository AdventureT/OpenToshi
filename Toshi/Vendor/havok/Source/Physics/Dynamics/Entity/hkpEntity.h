/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#ifndef HK_DYNAMICS2_ENTITY_H
#define HK_DYNAMICS2_ENTITY_H

#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/World/hkpWorldObject.h>
#include <Physics/Dynamics/Common/hkpMaterial.h>
#include <Physics/Dynamics/Constraint/hkpConstraintInstance.h>
#include <Physics/Dynamics/Motion/Rigid/hkpKeyframedRigidMotion.h>

class hkpEntityListener;
class hkpEntityActivationListener;
class hkpCollisionListener;
class hkpMotion;
class hkpSimulationIsland;
class hkpWorld;
class hkpConstraintInstance;
class hkpAction;
class hkpDynamicsContactMgr;
class hkSpuCollisionCallbackUtil;
class hkLocalFrame;
class hkpBreakableBody;
class hkdBreakableBody;

extern const hkClass hkpEntityClass;

/// This class represents the core "physical object" elements in the dynamics system, such
/// as rigid bodies.
class hkpEntity : public hkpWorldObject
{
	public:
		// +version(1)

		HK_DECLARE_REFLECTION();
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ENTITY);

		enum SpuCollisionCallbackEventFilter
		{
			SPU_SEND_NONE					= 0x00,
			SPU_SEND_CONTACT_POINT_ADDED	= 0x01,
			SPU_SEND_CONTACT_POINT_PROCESS	= 0x02,
			SPU_SEND_CONTACT_POINT_REMOVED	= 0x04,
			SPU_SEND_CONTACT_POINT_ADDED_OR_PROCESS = SPU_SEND_CONTACT_POINT_ADDED|SPU_SEND_CONTACT_POINT_PROCESS
		};

	public:

			// Destructor.
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [this,HK_ACCESS_RW] );
		virtual ~hkpEntity();

			//
			// Event Handling
			//

			/// Adds an entity listener to the entity.
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [this,HK_ACCESS_RW] );
		void addEntityListener( hkpEntityListener* el);

			/// Removes an entity listener from the entity.
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [this,HK_ACCESS_RW] );
		void removeEntityListener( hkpEntityListener* el);

			/// Adds an entity activation listener to the entity.
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [this,HK_ACCESS_RW] );
		void addEntityActivationListener( hkpEntityActivationListener* el);

			/// Removes an entity activation listener from the entity.
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [this,HK_ACCESS_RW] );
		void removeEntityActivationListener( hkpEntityActivationListener* el);

			/// Adds a collision listener to the entity.
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [this,HK_ACCESS_RW] );
		void addCollisionListener( hkpCollisionListener* cl );

			/// Removes a collision listener from the entity.
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [this,HK_ACCESS_RW] );
		void removeCollisionListener( hkpCollisionListener* cl);

			/// Get const access to the array of entity listeners.
		inline const hkSmallArray<hkpEntityListener*>& getEntityListeners() const;

			/// Get const access to the array of entity activation listeners.
		inline const hkSmallArray<hkpEntityActivationListener*>& getEntityActivationListeners() const;

			/// Get const access to the array of entity listeners.
		inline const hkSmallArray<hkpCollisionListener*>& getCollisionListeners() const;

			/// Simple thread safe check free function to see if any collision listener is attached
		inline bool areCollisionListenersAdded() const;


			/// Gets the process contact callback delay.
		inline hkUint16 getProcessContactCallbackDelay() const;

			/// Sets the process contact callback delay.
			/// This value is used to determine how often a callback is raised for the
			/// "process contact" collision event.
			/// A value of 0 means the callback is called every step, whereas a value of 4 means
			/// that a callback is raised every 5th step.
			/// Process contact callbacks can be used to change properties of contact points, such
			/// as the friction of a sliding contact.
		inline void setProcessContactCallbackDelay( hkUint16 delay );


			//
			// Utility functions
			//

			/// Gets the material used by this entity.
			/// If the entity has no collision detection representation,
			/// the material is not used.
		inline hkpMaterial& getMaterial();

			/// Gets the material used by this entity.
			/// If the entity has no collision detection representation,
			/// the material is not used.
		inline const hkpMaterial& getMaterial() const;

			/// A utility function to determine if the entity is fixed.
		inline hkBool isFixed() const;

			/// Checks whether the body's velocity cannot be influenced by physics directly.
			/// Uses a cached variable to avoid referencing hkpMotion object.
		inline hkBool isFixedOrKeyframed() const;

			/// Get's the entity's unique id. The uid is assigned in the entity's constructor and
			/// is also updated when your deserialize objects.
		inline hkUint32 getUid() const;

			/// Find the contact manager between 'this' and the supplied entity.
			///
			/// Returns HK_NULL if no contact manager exists between 'this' and the supplied entity.
		hkpDynamicsContactMgr* findContactMgrTo(const hkpEntity* entity) const;

			/// Returns a pointer to the attached hkdBreakableBody. Returns HK_NULL if none exists.
		hkdBreakableBody* getBreakableBody() const;


			//
			// Deactivation
			//

			/// Activates the specified entity and its island.
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [this,HK_ACCESS_RW] );
		void activate();

			/// Deactivates the specified entity and its island.
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_RW] [this,HK_ACCESS_RW] );
		void deactivate();

			/// Activates the specified entity and its island. Uses postponed operations queue if the world is locked for critical operations.
		void activateAsCriticalOperation();

			/// Deactivates the specified entity and its island. Uses postponed operations queue if the world is locked for critical operations.
		void deactivateAsCriticalOperation();

			/// Returns whether the entity is active. This method returns false if the entity
			/// has not yet been added to a hkpWorld object.
		hkBool isActive() const;


			//
			// Attached action and constraint accessors
			//

			/// Get the number of actions added to the world which reference this entity
		inline int getNumActions() const;

			/// Get the ith action added to the world which references this entity
		inline hkpAction* getAction(int i);

			///	Returns the number of constraints attached to this entity
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [m_simulationIsland,HK_ACCESS_RO] );
		int getNumConstraints() const;

			/// Returns the ith constraint attached to this entity
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [m_simulationIsland,HK_ACCESS_RW] );
		hkpConstraintInstance* getConstraint( int i );

			/// Returns all constraints of the body in a single array.
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [m_simulationIsland,HK_ACCESS_RO] );
		void getAllConstraints(hkArray<hkpConstraintInstance*>& constraints);

			/// Returns the ith constraint attached to this entity (const version)
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [m_simulationIsland,HK_ACCESS_RO] );
		const hkpConstraintInstance* getConstraint( int i ) const;

		/// Returns read only access to the internal constraint master list
		inline const hkSmallArray<struct hkConstraintInternal>&  getConstraintMasters() const;

		/// Returns read write access to the internal constraint master list
		inline hkSmallArray<struct hkConstraintInternal>&  getConstraintMastersRw();

		/// Returns read only access to the internal constraint master list
		inline const hkArray<class hkpConstraintInstance*>&  getConstraintSlaves() const;

			// Calculate the memory usage of this entity
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [this,HK_ACCESS_RO] );
		void calcContentStatistics( hkStatisticsCollector* collector, const hkClass* cls) const;

			/// Initialize cached AABB memory and SPU data (if available).
		void setCachedShapeData(const hkpWorld* world, const hkpShape* shape);

			/// Recalculate the cached AABB.
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [this,HK_ACCESS_RW] );
		void updateCachedAabb();


	protected:

			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [m_simulationIsland,HK_ACCESS_RO] );
		const	hkSmallArray<struct hkConstraintInternal>&	getConstraintMastersImpl() const;
					/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [m_simulationIsland,HK_ACCESS_RW] );
				hkSmallArray<struct hkConstraintInternal>&	getConstraintMastersRwImpl();

			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [m_simulationIsland,HK_ACCESS_RO] );
		const	hkArray<class hkpConstraintInstance*>&	getConstraintSlavesImpl() const;

		hkpEntity( const hkpShape* shape );

	public:

			//
			// INTERNAL FUNCTIONS
			//

		hkpEntity( class hkFinishLoadedObjectFlag flag );

			// Simulation units use this interface.
		inline hkpMotion* getMotion();

			// Get the simulation island, is HK_NULL for entities not in simulation.
		inline hkpSimulationIsland* getSimulationIsland() const;

			// Deallocates internal arrays if size 0.
			// Called internal by hkpWorld::removeEntity. Over
			/// ###ACCESS_CHECKS###( [m_world,HK_ACCESS_IGNORE] [this,HK_ACCESS_RW] );
		virtual void deallocateInternalArrays();

		virtual hkMotionState* getMotionState(){ return HK_NULL; }



		//
		// MEMBERS
		//

	protected:

			// The entity's material, only used if the collision detection is enabled.
		class hkpMaterial m_material;

	protected:

		friend class hkpWorldConstraintUtil;

	public:
		// this is just a quick workaround helper class for serialization

	public:
		class SmallArraySerializeOverrideType
		{
			public:
				HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkpEntity::SmallArraySerializeOverrideType );
				HK_DECLARE_REFLECTION();

				void* m_data;
				hkUint16 m_size;
				hkUint16 m_capacityAndFlags;
		};

    public:     void* m_limitContactImpulseUtil; //+serialized(false)

					/// A property used by Havok Destruction indicating how much damage an object will cause on another object
				hkReal m_damageMultiplier; //+default(1)

					
                hkpBreakableBody* m_breakableBody; //+nosave

		// the next three elements store constraint information (note: they are owned by the simulation island
		// offset into the accumulators
    public:     hkUint32                                    m_solverData;                   //+serialized(false)
	public: 	hkObjectIndex								m_storageIndex;					//+overridetype(hkUint16)
	protected:	hkUint16									m_processContactCallbackDelay;
    protected:  hkSmallArray<struct hkConstraintInternal>   m_constraintsMaster;            //+overridetype(class SmallArraySerializeOverrideType) +serialized(false)
    protected:  hkArray<hkpConstraintInstance*>             m_constraintsSlave;             //+serialized(false)

			// ------------------ 2nd CacheLine128 (rarely accessed data ) -------------------------

	protected:

        hkArray<hkUint8> m_constraintRuntime;   //+serialized(false)

		// The entity's simulation island.
        hkpSimulationIsland* m_simulationIsland; //+nosave

	public:

			/// See: hkpRigidBodyCinfo::m_autoRemoveLevel
		hkInt8  m_autoRemoveLevel;

			/// See: hkpRigidBodyCinfo::m_numUserDatasInContactPointProperties
		hkUint8 m_numUserDatasInContactPointProperties;

			// hkpWorld-unique Id
		hkUint32 m_uid; //+default(0xffffffff)

	public:

		struct SpuCollisionCallback
		{
		public:
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkpEntity::SpuCollisionCallback );
			HK_DECLARE_REFLECTION();
			SpuCollisionCallback():
				m_util(HK_NULL),
				m_capacity(0),
				m_eventFilter(SPU_SEND_CONTACT_POINT_ADDED_OR_PROCESS),
				m_userFilter(0x01)
			{
			}

				// only entities with a callback util will send events from spu to ppu; each event will only be fired once for each util
            hkSpuCollisionCallbackUtil* m_util; // +nosave

				// the maximum buffer size (counted in 16byte blocks) for events to be sent from spu to ppu; this value is set by setSpuCollisionCallbackUtil()
            hkUint16 m_capacity; //+serialized(false)

				// used to filter what events to send from spu to ppu
			hkUint8 m_eventFilter;

				// free to be set by the user; note that events will only be sent from spu to ppu for entities whose userFilter both have at least one matching bit set
			hkUint8	m_userFilter;

			SpuCollisionCallback(hkFinishLoadedObjectFlag flag) {}
		};

			// this class was created to keep the entity size <= 512
		struct ExtendedListeners
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_DYNAMICS, hkpEntity::ExtendedListeners );
			HK_DECLARE_REFLECTION();

			hkSmallArray<hkpEntityActivationListener*> m_activationListeners; //+overridetype(class SmallArraySerializeOverrideType) +serialized(false)
			hkSmallArray<hkpEntityListener*> m_entityListeners; //+overridetype(class SmallArraySerializeOverrideType) +serialized(false)
		};
	public:

		struct SpuCollisionCallback m_spuCollisionCallback;

    protected:  mutable ExtendedListeners* m_extendedListeners; //+serialized(false)

			// The motion of the object
	public:		class hkpMaxSizeMotion m_motion;//+hk.DataObjectType("hkpMotion")

	protected:

			//
			//	Rarely used members
			//
		friend class hkpEntityCallbackUtil;
		friend class hkpWorldCallbackUtil;
		friend class hkpWorld;
		friend class hkpSimulationIsland;
		friend class hkpWorldOperationUtil;

        hkSmallArray<hkpCollisionListener*>         m_collisionListeners;   //+overridetype(class SmallArraySerializeOverrideType) +serialized(false)


	protected:

        hkSmallArray<hkpAction*>                        m_actions;              //+overridetype(class SmallArraySerializeOverrideType) +serialized(false)

	public:
	
			/// A hierarchy of local frames attached to the entity.
		hkRefPtr<hkLocalFrame> m_localFrame;

};

#include <Physics/Dynamics/Entity/hkpEntity.inl>

#endif // HK_DYNAMICS2_ENTITY_H

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
