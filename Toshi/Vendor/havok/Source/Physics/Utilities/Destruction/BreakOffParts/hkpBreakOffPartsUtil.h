/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_UTILITIES_BREAK_OFF_PARTS_UTIL_H
#define INC_UTILITIES_BREAK_OFF_PARTS_UTIL_H

#include <Common/Base/hkBase.h>
#include <Common/Base/Container/PointerMap/hkPointerMap.h>

#include <Physics/Dynamics/World/Listener/hkpWorldDeletionListener.h>
#include <Physics/Dynamics/Constraint/Contact/hkpContactImpulseLimitBreachedListener.h>
#include <Physics/Dynamics/Entity/hkpEntityListener.h>
#include <Physics/Dynamics/Collide/hkpCollisionListener.h>
#include <Physics/Dynamics/Constraint/hkpConstraintListener.h>

class hkpPhysicsSystem;
class hkpRigidBody;


	/// This is an interface to the game/graphics engine and implements the user part of the hkpBreakOffPartsUtil
class hkpBreakOffPartsListener
{
public:	
	virtual ~hkpBreakOffPartsListener(){}

		/// Event structure
	struct ContactImpulseLimitBreachedEvent
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_UTILITIES, hkpBreakOffPartsListener::ContactImpulseLimitBreachedEvent );
		//ContactImpulseLimitBreachedEvent()  { }

		struct PointInfo
		{
			hkpRigidBody* m_collidingBody;
			hkpShapeKey   m_brokenShapeKey;
			hkBool        m_isContact;
			hkReal		  m_contactPointDirection; // either +1 or -1
			const hkContactPoint* m_contactPoint;
			const hkpContactPointProperties* m_properties;
			const hkpSimpleConstraintContactMgr* m_internalContactMgr;
		};

		hkpRigidBody*						 m_breakingBody;
		hkInplaceArray<PointInfo,4> m_points;
	};


		/// breaks off a subpart and returns the newly created objects.
		/// The implementation can decide to break off several elements, it just has to return
		/// the removed shapekeys and the created bodies and constraints.
		/// This is called from deep within the engine, so the implementation has to be THREADSAFE! 
		/// However there are never two calls to this listener in parallel thanks to a critical section in hkpBreakOffPartsUtil
	virtual hkResult breakOffSubPart( const ContactImpulseLimitBreachedEvent& event, hkArray<hkpShapeKey>& keysBrokenOffOut, hkpPhysicsSystem& bodiesOut ) = 0;
};



	/// This is a helper class, breaking off pieces from a compound body. 
	/// The basic idea is that this class properly monitors and clips the impulse acting on a compound sub part.
	/// So it basically can be used instead of the usual hkContactPointAdded callback.
	///   - There should only be one instance of this class per hkpWorld.
	///   - This only works if you can identify a sub piece using a single hkpShapeKey, that means
	///     only one hkMoppShape is allowed per rigid body (e.g. hkpExtendedMeshShape)
	///   - If you use this class on the PLAYSTATION(R)3 SPU, the default implementation on the SPU will put the shape key into the materials userData
class hkpBreakOffPartsUtil: public hkReferencedObject, public hkpContactImpulseLimitBreachedListener, hkpEntityListener, 
								hkpWorldDeletionListener, hkpConstraintListener
{
	public:
		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_WORLD );

			/// Creates an instance of this class and add itself to the world. You only need one per world. 
			/// It automatically calls hkpWorld::addContactImpulseLimitBreachedListener(this) and adds a reference
			/// to itself as long as the world is valid.
			/// The hkpBreakOffPartsListener is used to decide on the destruction details and needs
			/// to be implemented by the user
		hkpBreakOffPartsUtil( hkpWorld* world, hkpBreakOffPartsListener* listenerInterface );

			/// Flag a subpart of an object to break off above a given impulse limit
		void markPieceBreakable( hkpEntity* entity, hkpShapeKey key, hkReal maxImpulse );

			/// Flag a whole entity to be breakable
		void markEntityBreakable( hkpEntity* entity, hkReal maxImpulse );

			/// Set factor used to multiply maximum impulse for attached constraints.
			/// This function must be used when setting max impulses for individual hkpShapeKeys, 
			/// and not for the entire entity with markEntityBreakable().
		void setMaxConstraintImpulse( hkpEntity* entity, hkReal maxConstraintImpulse );

			/// Safely removes a piece from a listshape, or a listshape wrapped in a moppshape
			/// Both the mopp and listshapes are updated
		static void HK_CALL removeKeysFromListShape( hkpEntity* entity, hkpShapeKey* keysToRemove, int numKeys);


		//
		//	internal section
		//
	protected:
		~hkpBreakOffPartsUtil();

			// this removes pieces which are to be broken off
		virtual void contactImpulseLimitBreachedCallback( const hkpContactImpulseLimitBreachedListenerInfo* breachedContacts, int numBreachedContacts );

		        void contactImpulseLimitBreachedCallback_forToi( const hkpContactImpulseLimitBreachedListenerInfo* breachedContact );

			// hkWorldListenerImplementation
		virtual void worldDeletedCallback( hkpWorld* world);

			// hkEntityListenerImplementation
		virtual void entityRemovedCallback( hkpEntity* entity );

			// hkpConstraintListener interface implementation
		virtual void constraintAddedCallback( hkpConstraintInstance* constraint );
		virtual void constraintRemovedCallback( hkpConstraintInstance* constraint ) {}


	public:

			/// A small helper class, which gets attached to an hkpEntity 
		class LimitContactImpulseUtil: public hkReferencedObject, public hkpCollisionListener
		{
			public:
				LimitContactImpulseUtil( hkpBreakOffPartsUtil* breakUtil, hkpEntity* entity );
			protected:
					// hkpCollisionListener interface implementation
				virtual void contactPointAddedCallback(	hkpContactPointAddedEvent& event);
				virtual void contactPointConfirmedCallback( hkpContactPointConfirmedEvent& event){}
				virtual void contactPointRemovedCallback( hkpContactPointRemovedEvent& event ){}


			public:

				/// Associate the given max impulse with the given key.
				inline void setMaxImpulseForShapeKey ( const hkpShapeKey key, hkUint8 max_impulse );

				/// Get the max impulse associated with the given key.
				/// If no impulse has been associated with this key, zero is returned.
				inline hkUint8 getMaxImpulseForKey ( const hkpShapeKey key ) const; 

				/// Stop storing the associated impulse value for this key.
				inline void removeKey ( const hkpShapeKey key );

				/// Checks if there are any hkpShapeKey-specific max impulse values defined.
				inline hkBool32 hasShapeKeySpecificMaxImpulses() { return m_shapeKeyToMaxImpulse.getSize(); }

				/// Return the weakest point on the object. 
				void findWeakestPoint( hkpShapeKey& keyOut, hkReal& weakestImpulseOut );

			public:
				HK_ALIGN16(hkpEntity*       m_entity);

				hkUFloat8                  m_maxImpulse;		// set to 255 = (hkUFloat8::MAX_VALUE-1) if not breakable

				hkReal						m_maxConstraintImpulse;

			private:

				/// Stores a the max impulse for given shape key.
				/// The first value is a shape key
				/// The second value is the int value field of hkUfloat8.
				hkPointerMap<hkpShapeKey, hkUint8> m_shapeKeyToMaxImpulse;
		};
	private:

			// return the entity util if one exists or create one 
		HK_FORCE_INLINE LimitContactImpulseUtil* getOrCreateUtil( hkpEntity* entity );

	public:
		hkpWorld* m_world;

			// an optional critical section to serialize access to this utility
		hkCriticalSection* m_criticalSection;

		hkpBreakOffPartsListener* m_breakOffPartsListener;

};

#include <Physics/Utilities/Destruction/BreakOffParts/hkpBreakOffPartsUtil.inl>

#endif //INC_UTILITIES_BREAK_OFF_PARTS_UTIL_H



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
