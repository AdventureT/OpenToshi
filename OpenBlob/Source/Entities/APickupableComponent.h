#pragma once
#include "AEntityComponent2.h"
#include "Collision/ACollisionFunctor.h"

#include <Math/TVector4.h>
#include <Math/TQuaternion.h>

class ABaseEntity;

//-----------------------------------------------------------------------------
// Purpose: the collision functor of a thrown pickupable prop (8 bytes): the entity that threw it, and the entities flagged
// 0x10, are ignored by the throw's sweep.
//-----------------------------------------------------------------------------
struct HittableCollisionFunctor
    : public ACollisionFunctor
{
	// $deBlob: FUNCTION 007df1e0
	HittableCollisionFunctor( ABaseEntity* a_pThrower )
	    : m_pThrower( a_pThrower )
	{}

	// NOTE: the base's HitNonSolidObject returns TFALSE
	virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override { return TFALSE; } // +0x10
	virtual TBOOL HitIgnorableObject( ACollisionObject* a_pObject ) override;                  // +0x18

	ABaseEntity* m_pThrower; // 0x04
};

//-----------------------------------------------------------------------------
// Purpose: makes a prop pickupable and throwable (Props\APickupableComponent.cpp): an entity picks it up
// (MESSAGE_PLAYERINTERACT), throws or drops it (messages 0x1A/0x1B); thrown, it falls and spins until it hits something
// (damaging the entities around it) or 4 seconds passed.
// retail never creates one (no reference to its constructor or vftable outside of its own functions)
//-----------------------------------------------------------------------------
class APickupableComponent
    : public AEntityComponent2
{
public:
	enum STATE : TINT
	{
		STATE_Idle     = 0,
		STATE_PickedUp = 1,
		STATE_Thrown   = 2,
		STATE_Dropped  = 3,
	};

	// The global data "GlobalData_APickupableComponentData_EntityData"
	struct Data
	{
		TFLOAT fUnk00;      // 0x00
		TFLOAT fGravity;    // 0x04 (added to the vertical velocity per second)
		TFLOAT fThrowSpeed; // 0x08
		TFLOAT fThrowAngle; // 0x0C (the throw direction is pitched up by it)
		TFLOAT fSpinSpeed;  // 0x10
	};

	// The data of the throw messages (0x1A/0x1B)
	struct ThrowData
	{
		TBOOL           bThrow;     // 0x00 (TFALSE: dropped)
		Toshi::TVector4 vDirection; // 0x04
	};

	// The time a throw lasts at most
	static const TFLOAT scm_fMaxThrownTime;

public:
	APickupableComponent( ABaseEntity* a_pOwner, TUINT a_uiAttackDamage, TUINT a_uiAttackUnk2, TFLOAT a_fDamageRadius );
	virtual ~APickupableComponent() override; // +0x00

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x04
	virtual void ResetComponent() override;                                                           // +0x08
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x10

	static void LoadData();
	// Forgets the data and reads it again
	static void ReloadData();
	// Forgets the data (AEntityFramework::Shutdown)
	static void UnloadData();

private:
	void UpdatePosition( TFLOAT a_fDeltaTime );
	void UpdateOrientation( TFLOAT a_fDeltaTime );
	void DamageNearbyEntities( Toshi::TVector4 a_vDirection );

private:
	static Data* sm_pData;

private:
	STATE              m_eState;         // 0x14
	ABaseEntity*       m_pThrower;       // 0x18 (the entity that picked it up)
	Toshi::TVector4    m_vVelocity;      // 0x1C
	Toshi::TQuaternion m_qOrientation;   // 0x2C
	TFLOAT             m_fThrownTime;    // 0x3C
	TUINT              m_uiAttackDamage; // 0x40 (AAttackMessage::m_uiUnk14)
	TUINT              m_uiAttackUnk2;   // 0x44 (AAttackMessage::m_uiUnk18)
	TFLOAT             m_fDamageRadius;  // 0x48 (no damage when 0)
};                                       // 0x4C
