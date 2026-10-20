#pragma once
#include "Entities/APropPlatform.h"

TOSHI_NAMESPACE_START
class TParticleEffect;
class TPhysicsPhantomContainer;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: the Inky jetbike (Entities\AInkJet\AInkJet.cpp, parent APropPlatform, vtable 0x918428, the second base at
// +0x9C is ATriggerUtil): a platform that drives along its waypoint spline, steering towards the closest blob
// (GetNextNode, AdjustNodePosition with "Randomness"). It spots a blob in its cone (state 2), drops three leechbots in front of it
// (state 3), and is hit by the blob's Z attacks (state 4: the blob loses paint points, or the jetbike explodes when it
// has none left). Spawned by AJetbikeSpawner / AInkJetSpawner (IJFLAG_Spawned): such a jetbike waits off
// (IJFLAG_Waiting) until the spawner launches it.
//-----------------------------------------------------------------------------
class AInkJet
    : public APropPlatform
{
public:
	TDECLARE_CLASS( AInkJet, APropPlatform );

	// m_uiInkJetFlags
	using IJFLAG = TUINT8;
	enum IJFLAG_ : IJFLAG
	{
		IJFLAG_Waiting        = 0x01, // a spawned jetbike parked off: its query helper is out of the simulation
		IJFLAG_CanDropLeeches = 0x02, // set when it spots a blob (state 2), cleared when the blob hits it
		IJFLAG_Spawned        = 0x04, // owned by a spawner (AJetbikeSpawner / AInkJetSpawner ProcessParams)
		IJFLAG_HitPending     = 0x08, // the blob hit it with a Z attack (MESSAGE_SLAM) and still has paint points
		IJFLAG_LeechesDropped = 0x10, // the three leechbots of state 3 were dropped
	};

	// The type data of AInkJet (the TRB data of its entity type)
	struct InkJetData
	    : public AEntityTypeData::TRBData
	{
		TFLOAT m_fSpotFOV;          // 0x54 the cone it spots the blob in
		TFLOAT m_fSpotDistance;     // 0x58
		TFLOAT m_fLoseTime;         // 0x5C state 3 goes back to 1 after this long without seeing the blob
		TFLOAT m_fCruiseSpeed;      // 0x60 the speed of state 1
		TFLOAT m_fChaseSpeed;       // 0x64 the speed of state 3
		TFLOAT m_fLeechCheckRadius; // 0x68 no leechbots are dropped while a hungry one is this close
	};

public:
	AInkJet();
	virtual ~AInkJet() override; // +0x08

	virtual void                 Tick( TFLOAT a_fDeltaTime ) override;                                                                                                             // +0x0C
	virtual void                 OnLODChanging( TINT a_iLOD ) override;                                                                                                            // +0x10
	virtual void                 OnOutOfWorld() override;                                                                                                                          // +0x18
	virtual void                 Create() override;                                                                                                                                // +0x1C
	virtual void                 PostCreate() override;                                                                                                                            // +0x20
	virtual void                 ProcessParams( const AEntityParams& a_rParams ) override;                                                                                         // +0x2C
	virtual void                 HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override;                                                              // +0x5C
	virtual TBOOL                IsTargetable() override;                                                                                                                          // +0xA0
	virtual TINT                 GetTargettingPriority() override;                                                                                                                 // +0xA8
	virtual TINT                 GetMaxLife() override;                                                                                                                            // +0xB8
	virtual void                 Reset( ABaseEntity* a_pEntity ) override;                                                                                                         // +0xC8
	virtual void                 OnEnterState( TINT a_iState ) override;                                                                                                           // +0xD8
	virtual const TCHAR*         GetModelName() override;                                                                                                                          // +0xEC
	virtual TBOOL                TicksAtLowLOD() override;                                                                                                                         // +0xF4
	virtual void                 StartFromCurrentNode() override;                                                                                                                  // +0xFC
	virtual void                 OnStartMoving() override;                                                                                                                         // +0x100
	virtual void                 StopMoving() override;                                                                                                                            // +0x104
	virtual AWaypointData::Node* GetNextNode( TINT16 a_iNode, TBOOL a_bReverse, TBOOL a_bToggled, TINT* a_pOutNode ) override;                                                     // +0x114
	virtual void                 AdjustNodePosition( Toshi::TVector4* a_pOut, const Toshi::TVector4* a_pFrom, const Toshi::TVector4* a_pNodePos, TBOOL a_bNoRandomness ) override; // +0x118

	// Parked by its spawner (AInkJetSpawner waits for this)
	TBOOL IsWaiting() const { return ( m_uiInkJetFlags & IJFLAG_Waiting ) != 0; }
	// At the end of its route (AInkJetSpawner, assert m_pInkJet->IsWaitingForTrigger())
	TBOOL IsWaitingForTrigger() const { return ( m_uiPlatformFlags & PLATFORMFLAG_Stopped ) != 0; }
	void  SetSpawned() { m_uiInkJetFlags |= IJFLAG_Spawned; }

	// Turns it on: visible and ticking, its life back (AJetbikeSpawner, MESSAGE_TRIGGER)
	void Activate();
	// Turns it off: hidden, FLAG_Dead (a parked or exploded jetbike)
	void Deactivate();
	// Blows it up (the explosion effect, sounds, a camera shake) and restarts its route if spawned or reset, else turns
	// it off
	void Explode();

private:
	const InkJetData* GetInkJetData() const { return TSTATICCAST( const InkJetData, m_pTypeData->m_pTRBData ); }

private:
	static Toshi::TParticleEffect* ms_pExplosionEffect; // "Blob_Explosion"
	static TBOOL                   ms_bMaterialsSetUp;  // the sphere map of the model

	TINT8                            m_iTargetPlayer;         // 0x1F0 the closest player, -1: none
	IJFLAG                           m_uiInkJetFlags;         // 0x1F1 IJFLAG
	TFLOAT                           m_fRandomness;           // 0x1F4 "Randomness" 3.0: how far off the spline it steers
	TFLOAT                           m_fArmTime;              // 0x1F8 1.3 s after a launch its query helper tests layer 0x42, then 0xC3 (-1)
	TFLOAT                           m_fTimeSinceSeen;        // 0x1FC since it last saw the blob in its cone (100 at most)
	TFLOAT                           m_fHitCooldown;          // 0x200 between the hits of the blob (-1)
	Toshi::TPhysicsPhantomContainer* m_pCollisionQueryHelper; // 0x204 the capsule shape phantom of its collision queries
};
