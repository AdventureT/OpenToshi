/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_SIMPLE_CONSTRAINT_CONTACT_MGR_H
#define HK_DYNAMICS2_SIMPLE_CONSTRAINT_CONTACT_MGR_H

#include <Physics/Collide/Dispatch/ContactMgr/hkpContactMgrFactory.h>
#include <Physics/Dynamics/Collide/hkpDynamicsContactMgr.h>
#include <Physics/Dynamics/Constraint/Contact/hkpSimpleContactConstraintData.h>

class hkpWorld;

	/// This class acts as a bridge between hkCollide and hkDynamics constraint system.
	/// It collects all contact point information from the collision detector through the hkpContactMgr interface
	/// and copies this data to an internal hkContactConstraint
class hkpSimpleConstraintContactMgr: public hkpDynamicsContactMgr
{
	public:

		hkpSimpleConstraintContactMgr( hkpWorld* world, hkpRigidBody *bodyA, hkpRigidBody *bodyB );

		~hkpSimpleConstraintContactMgr();

		inline hkpSimpleConstraintContactMgr( hkFinishLoadedObjectFlag f ) : hkpDynamicsContactMgr( hkpContactMgr::TYPE_SIMPLE_CONSTRAINT_CONTACT_MGR), m_contactConstraintData(f), m_constraint(f) {}
			/// hkpDynamicsContactMgr interface implementation.
		hkContactPoint* getContactPoint( hkContactPointId id );

			/// hkpDynamicsContactMgr interface implementation.
		hkpContactPointProperties* getContactPointProperties( hkContactPointId id );


			/// hkpContactMgr interface implementation.
		hkContactPointId addContactPointImpl( const hkpCdBody& a, const hkpCdBody& b, const hkpProcessCollisionInput& input, hkpProcessCollisionOutput& output, const hkpGskCache* contactCache, hkContactPoint& cp );

			/// hkpContactMgr interface implementation.
		hkResult reserveContactPointsImpl( int numPoints );
	
			/// hkpContactMgr interface implementation.
		void removeContactPointImpl( hkContactPointId cpId, hkCollisionConstraintOwner& constraintOwner );

			/// hkpContactMgr interface implementation.
		void processContactImpl( const hkpCollidable& a, const hkpCollidable& b, const hkpProcessCollisionInput& input, hkpProcessCollisionData& collisionData );

			/// hkpContactMgr interface implementation.
		ToiAccept addToiImpl( const hkpCdBody& a, const hkpCdBody& b, const hkpProcessCollisionInput& input, hkpProcessCollisionOutput& output, hkTime toi, hkContactPoint& cp, const hkpGskCache* gskCache, hkReal& projectedVelocity, hkpContactPointProperties& properties );

			/// hkpContactMgr interface implementation.
		void removeToiImpl( class hkCollisionConstraintOwner& constraintOwner, hkpContactPointProperties& properties );

			/// hkpContactMgr interface implementation; apply custom toi-collision handling before localized solving
		HK_NOSPU_VIRTUAL void confirmToi( struct hkpToiEvent& event, hkReal rotateNormal, class hkArray<class hkpEntity*>& outToBeActivated );

		HK_NOSPU_VIRTUAL void cleanup(){ delete this; }

		HK_NOSPU_VIRTUAL Type getType() const { return TYPE_SIMPLE_CONSTRAINT_CONTACT_MGR; }

			/// Get the ids of all contact points in this collision.
		HK_NOSPU_VIRTUAL void getAllContactPointIds( hkArray<hkContactPointId>& contactPointIds ) const;

		HK_FORCE_INLINE hkpSimpleContactConstraintAtom* getAtom() { HK_ASSERT2(0xad76aba3, m_contactConstraintData.m_atom, "Atom not created yet."); return m_contactConstraintData.m_atom; }

		HK_FORCE_INLINE const hkpSimpleContactConstraintAtom* getAtom() const { HK_ASSERT2(0xad76aba3, m_contactConstraintData.m_atom, "Atom not created yet."); return m_contactConstraintData.m_atom; }

			/// Get the constraint instance
		virtual hkpConstraintInstance* getConstraintInstance();

		virtual void toiCollisionResponseBeginCallback( const hkContactPoint& cp, struct hkpSimpleConstraintInfoInitInput& inA, struct hkpBodyVelocity& velA, hkpSimpleConstraintInfoInitInput& inB, hkpBodyVelocity& velB);

		virtual void toiCollisionResponseEndCallback( const hkContactPoint& cp, hkReal impulseApplied, struct hkpSimpleConstraintInfoInitInput& inA, struct hkpBodyVelocity& velA, hkpSimpleConstraintInfoInitInput& inB, hkpBodyVelocity& velB);


	protected:

		inline
		hkpRigidBody* setContactPointProperties( const hkpCdBody& a, const hkpCdBody& b, int maxNumExtraDataInEvent, hkpContactPointProperties* cpi );

			/// produce some memory statistics
		void calcContentStatistics( hkStatisticsCollector* collector, const hkClass* cls ) const;

	public:

			/// Class that creates instances of hkpSimpleConstraintContactMgr.
		class Factory: public hkpContactMgrFactory
		{
			//+vtable(true)
			public:
				Factory(hkpWorld* mgr);

				hkpContactMgr*	createContactMgr( const hkpCollidable& a, const hkpCollidable& b, const hkpCollisionInput& input );

			protected:
				hkpWorld* m_world;
		};

		//
		//	For internal use only
		//
	public:

		hkUint16					m_reservedContactPoints;
		hkUint16					m_skipNextNprocessCallbacks;

		hkpSimpleContactConstraintData	m_contactConstraintData;
		hkpConstraintInstance			m_constraint;

		hkUint32					m_pad[2];
};


#endif // HK_DYNAMICS2_SIMPLE_CONSTRAINT_CONTACT_MGR_H

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
