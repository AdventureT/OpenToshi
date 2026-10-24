/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_AGENT3_1N_MACHINE_H
#define HK_COLLIDE2_AGENT3_1N_MACHINE_H

#include <Physics/Internal/Collide/Agent3/hkpAgent3.h>


struct hkpAgent1nTrack;
struct hkpAgent3Input;
class hkpShapeCollection;
#ifndef hkCollisionConstraintOwner
class hkpConstraintOwner;
#	define hkCollisionConstraintOwner hkpConstraintOwner
#endif


// Make sure if you use this header that you pad to get a 16 byte alignment
struct hkpAgent1nMachineEntry: hkpAgentEntry
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CDINFO, hkpAgent1nMachineEntry );

	hkShapeKeyPair m_shapeKeyPair;
};

struct hkpAgent1nMachinePaddedEntry: hkpAgent1nMachineEntry
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CDINFO, hkpAgent1nMachinePaddedEntry );

	hkUint32 m_padUpTo16Bytes;
};

struct hkpAgent1nMachineTimEntry: hkpAgent1nMachineEntry
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CDINFO, hkpAgent1nMachineTimEntry );

	hkTime     m_timeOfSeparatingNormal;		// only used if tims are enabled
	hkVector4  m_separatingNormal;
};


struct hkpAgent1nMachine_VisitorInput
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CDINFO, hkpAgent1nMachine_VisitorInput );

		/// BodyA
	const hkpCdBody* m_bodyA;

		/// BvTree or shape collection (if there's no bvTree in the hierarchy) BodyB or 
	const hkpCdBody* m_collectionBodyB;

		/// The shape container, which is the shape of bodyA.
		/// Note that this is only used for Nm Machine, and can be set to HK_NULL otherwise.
		/// Note that we do not store the shape key needed to extract the child body of interest from m_bodyA. 
		/// The shape key is taken from the agent entry.
	const hkpShapeContainer* m_containerShapeA;

		/// The shape container, which is the shape of bodyB
		/// Note that we do not store the shape key needed to extract the child body of interest from m_collectionBodyB. 
		/// The shape key is taken from the agent entry.
	const hkpShapeContainer* m_containerShapeB;

		/// Pointer to hkpProcessCollisionInput
	const hkpCollisionInput* m_input;

		/// Pointer to hkpContactMgr
	hkpContactMgr*	m_contactMgr;

	hkCollisionConstraintOwner* m_constraintOwner;

	void* m_clientData;
};


typedef hkpAgentData* (*hkAgent1nMachine_VisitorCallback)( hkpAgent1nMachine_VisitorInput& vin, hkpAgent1nMachineEntry* entry, hkpAgentData* agentData );   


extern "C"
{
		/// Initialize the agentTrack
	void HK_CALL hkAgent1nMachine_Create( hkpAgent1nTrack& agentTrack );

		/// call process collision on all agents
	void HK_CALL hkAgent1nMachine_Process( hkpAgent1nTrack& agentTrack, hkpAgent1nTrack* agentTrackOnPpu, const hkpAgent3ProcessInput& input, const HK_SHAPE_CONTAINER* shapeContainterB, const hkpShapeKey* hitList, hkpProcessCollisionOutput& output  );
	void HK_CALL hkAgentNmMachine_Process( hkpAgent1nTrack& agentTrack, hkpAgent1nTrack* agentTrackOnPpu, const hkpAgent3ProcessInput& input, const HK_SHAPE_CONTAINER* shapeContainerA, const HK_SHAPE_CONTAINER* shapeCollectionB, const hkShapeKeyPair* hitList, hkpProcessCollisionOutput& output  );

		/// Update all filters
	void HK_CALL hkAgent1nMachine_UpdateShapeCollectionFilter( hkpAgent1nTrack& agentTrack, hkpAgent1nMachine_VisitorInput& vin );
	void HK_CALL hkAgentNmMachine_UpdateShapeCollectionFilter( hkpAgent1nTrack& agentTrack, hkpAgent1nMachine_VisitorInput& vin );

		/// revisits all agents, the visitor should return a pointer just after the end of the agent.
		/// visitor can change size of agent. If visitor returns the start of the agent, the agent will be removed 
	void HK_CALL hkAgent1nMachine_VisitAllAgents( hkpAgent1nTrack& agentTrack, hkpAgent1nMachine_VisitorInput& vin, hkpContactMgr* mgr, hkAgent1nMachine_VisitorCallback visitor );


		/// destroy all agents
	void HK_CALL hkAgent1nMachine_Destroy( hkpAgent1nTrack& agentTrack, hkpCollisionDispatcher* dispatch, hkpContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner );

		/// destroy all cache information
	void HK_CALL hkAgent1nMachine_InvalidateTim( hkpAgent1nTrack& track, const hkpCollisionInput& input );
		
	void HK_CALL hkAgent1nMachine_WarpTime( hkpAgent1nTrack& track, hkTime oldTime, hkTime newTime, const hkpCollisionInput& input );
	
	//
	// internal functions
	//
	
		/// Input:
		///   - List of potential points, all potential points have a reserved contact point
		///   - List of reference contact points. This list also includes potential points
	void HK_CALL hkAgent1nMachine_Weld( hkpAgent3Input& input, const HK_SHAPE_CONTAINER* shapeCollection, hkpProcessCollisionOutput& output );

	hkpAgent1nMachineEntry* hkAgent1nMachine_FindAgent( hkpAgent1nTrack& agentTrack, hkShapeKeyPair keyPair, hkpAgentData** agentDataOut);
}



#endif // HK_COLLIDE2_AGENT3_1N_MACHINE_H

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
