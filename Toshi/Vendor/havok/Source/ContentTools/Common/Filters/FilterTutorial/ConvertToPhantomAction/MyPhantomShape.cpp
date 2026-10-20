/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <ContentTools/Common/Filters/FilterTutorial/hctFilterTutorial.h>
#include <ContentTools/Common/Filters/FilterTutorial/ConvertToPhantomAction/MyPhantomShape.h>

#include <Physics/Dynamics/Action/hkpBinaryAction.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>


//
// Actions
//

// Apply a constant force to the rigid body
// (Use a binary action so that it maintains a ref to the phantom)
class MyWindAction : public hkpBinaryAction
{
public:

	MyWindAction( hkpRigidBody* phantom, hkpRigidBody* rb, const hkVector4& force ) : hkpBinaryAction(phantom,rb), m_force(force) {}
	MyWindAction( hkFinishLoadedObjectFlag flag ) : hkpBinaryAction(flag) {}
	
	/*virtual*/ void applyAction( const hkStepInfo& stepInfo )
	{
		hkpRigidBody* rb = (hkpRigidBody*)getEntityB();
		rb->applyForce( stepInfo.m_deltaTime, m_force );
	}
	
	/*virtual*/ MyWindAction* clone( const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms ) const
	{
		return HK_NULL;
	}
	
	hkVector4 m_force;
};

// Apply a 'point' force to the rigid body
class MyAttractAction : public hkpBinaryAction
{
public:

	MyAttractAction( hkpRigidBody* phantom, hkpRigidBody* rb, float strength ) : hkpBinaryAction(phantom,rb), m_strength(strength) {}
	MyAttractAction( hkFinishLoadedObjectFlag flag ) : hkpBinaryAction(flag) {}
	
	/*virtual*/ void applyAction( const hkStepInfo& stepInfo )
	{
		hkVector4 force;
		force.setSub4( getEntityA()->getMotion()->getPosition(), getEntityB()->getMotion()->getPosition() );
		float distance = force.length3();
		force.mul4( m_strength / ( ( distance * distance ) + HK_REAL_EPSILON ) );
		
		hkpRigidBody* rb = (hkpRigidBody*)getEntityB();
		rb->applyForce( stepInfo.m_deltaTime, force );
	}
	
	/*virtual*/ MyAttractAction* clone( const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms ) const
	{
		return HK_NULL;
	}
	
	float m_strength;
};



//
// hkpPhantom interface implementation
//

MyPhantomShape::MyPhantomShape()
{
	m_actionType = ACTION_WIND;
	m_direction.setZero4();
	m_strength = 0.0f;
}

/*virtual*/ void MyPhantomShape::phantomEnterEvent( const hkpCollidable* phantomColl, const hkpCollidable* otherColl, const hkpCollisionInput& env )
{
	hkpRigidBody* phantom = hkGetRigidBody( phantomColl );
	hkpRigidBody* rb = hkGetRigidBody( otherColl );
	if( !phantom || !rb )
	{
		return;
	}
	
	// Create the appropriate action
	hkpAction* action = HK_NULL;
	switch( m_actionType )
	{
		case ACTION_WIND:
			{
				hkVector4 force;
				force.setMul4( m_strength, m_direction );
				action = new MyWindAction( phantom, rb, force );
			}
			break;
		case ACTION_ATTRACT:
			{
				action = new MyAttractAction( phantom, rb, m_strength );
			}
			break;
		case ACTION_DEFLECT:
			{
				// use the attract action with a negative strength
				action = new MyAttractAction( phantom, rb, -m_strength );
			}
			break;
	}
	
	// Add it to the world
	if( action )
	{
		rb->getWorld()->addAction( action );
		action->removeReference();
	}
}

/*virtual*/ void MyPhantomShape::phantomLeaveEvent( const hkpCollidable* phantomColl, const hkpCollidable* otherColl )
{
	hkpRigidBody* phantom = hkGetRigidBody( phantomColl );
	hkpRigidBody* rb = hkGetRigidBody( otherColl );
	if( !phantom || !rb )
	{
		return;
	}
	
	// Find and remove the appropriate action
	int numActions = phantom->getNumActions();
	for( int i=numActions-1; i>=0; --i )
	{
		hkpAction* action = phantom->getAction(i);
		
		if (action)
		{
			hkArray<hkpEntity*> entities;
			action->getEntities( entities );		
			for( int j=0; j<entities.getSize(); ++j )
			{
				if( entities[j] == rb )
				{
					action->getWorld()->removeAction( action );
				}
			}
		}
	}
}

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
