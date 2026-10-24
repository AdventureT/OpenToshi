/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_CONSTRAINT_LISTENER_H
#define HK_DYNAMICS2_CONSTRAINT_LISTENER_H

class hkpConstraintInstance;
class hkpWorld;

struct hkpConstraintBrokenEvent
{
	enum EventSource
	{
		EVENT_SOURCE_UNKNOWN,
		EVENT_SOURCE_BREAKABLE_CONSTRAINT,
		EVENT_SOURCE_FLEXIBLE_JOINT,
	};

	hkpConstraintBrokenEvent(hkpWorld* world, hkpConstraintInstance* i, EventSource es)
	:	m_world(world)
	,	m_constraintInstance(i)
	,	m_eventSource(es)
	,	m_eventSourceDetails(0)
	,	m_constraintBroken(true)
	,	m_actualImpulse(0.0f)
	,	m_impulseLimit(0.0f )
	{
	}

	hkpWorld*			m_world;
	hkpConstraintInstance* m_constraintInstance;

	hkEnum<EventSource,hkUint8> m_eventSource;	///
	hkUint8 m_eventSourceDetails;				///
	hkBool m_constraintBroken;					///
	hkReal m_actualImpulse;						///
	hkReal m_impulseLimit;						///
};


	/// Any class that is interested in events from a constraint inherits from this class
class hkpConstraintListener
{
	public:

		virtual ~hkpConstraintListener() {}

			/// Called when a constraint is added to the world.
		virtual void constraintAddedCallback( hkpConstraintInstance* constraint ) {}

			/// Called when a constraint is removed from the world.
		virtual void constraintRemovedCallback( hkpConstraintInstance* constraint ) {}

			/// Called when a constraint is deleted, only called to listeners which are directly attached to hkpConstraintInstance
		virtual void constraintDeletedCallback( hkpConstraintInstance* constraint ) {}

			/// Called when a constraint gets broken or repaired (e.g. hkpBreakableConstraintData fires this event)
		virtual void constraintBreakingCallback( const hkpConstraintBrokenEvent& event ){}
};


#endif // HK_DYNAMICS2_CONSTRAINT_LISTENER_H

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
