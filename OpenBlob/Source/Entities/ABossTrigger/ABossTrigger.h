#pragma once
#include "Entities/ABaseEntity.h"
#include "Entities/ATriggerUtil.h"

class ABoss;
class ABuildingInstance;

//-----------------------------------------------------------------------------
// Purpose: a target of the boss fight (ATriggerUtil as a second base): its targets are platforms (APropPlatform)
// carrying buildings, the boss, street spawners, container hazards and despawn triggers. In its round (BossStage) the
// platforms go up; once Blob painted every building of them in the required colour the trigger is complete, the
// platforms go down and the boss (ABoss::OnBossTriggerCompleted) is stunned when every trigger of the round is.
//-----------------------------------------------------------------------------
class ABossTrigger
    : public ABaseEntity
    , public ATriggerUtil
{
public:
	TDECLARE_CLASS( ABossTrigger, ABaseEntity );

	enum BTState : TUINT8
	{
		BTState_Up     = 0,
		BTState_Moving = 1, // the platforms move (BTFLAG_Raising: up)
		BTState_Down   = 2,
	};

	// m_uiBTFlags
	enum BTFLAG : TUINT8
	{
		BTFLAG_Completed   = 0x1,
		BTFLAG_Raising     = 0x2,
		BTFLAG_Projections = 0x4, // the buildings show the paint projection of the required colour
	};

public:
	ABossTrigger();
	virtual ~ABossTrigger() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                     // +0x0C
	virtual void PostCreate() override;                                    // +0x20 (also ATriggerUtil +0x08)
	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C (also ATriggerUtil +0x04)
	virtual void Reset( ABaseEntity* a_pEntity ) override;                 // +0xC8

	// Raises the platforms of the trigger (the boss's round starts)
	void Raise();
	// Raises the platforms again after the boss recovered: their buildings lose their colour
	void RaiseAgain();
	// Sends MESSAGE_TRIGGER to the street spawner targets
	void ActivateSpawners();
	// Sends 14 (a_bActive) or 15 to the container hazard targets
	void SetHazardsActive( TBOOL a_bActive );
	// Stops the street spawner targets (a_bDespawn: also their entities and triggers the despawn triggers)
	void DespawnTargets( TBOOL a_bDespawn );

	// $deBlob: FUNCTION 007a6130
	TUINT GetBTState() const { return m_eBTState; }
	// $deBlob: FUNCTION 007a6180
	void  SetBTState( TUINT8 a_eState ) { m_eBTState = a_eState; }
	TBOOL IsCompleted() const { return ( m_uiBTFlags & BTFLAG_Completed ) != 0; }
	// $deBlob: FUNCTION 007a6140
	TBOOL IsRaising() const { return ( m_uiBTFlags >> 1 ) & 1; }
	// $deBlob: FUNCTION 007a6150
	TBOOL IsProjecting() const { return ( m_uiBTFlags >> 2 ) & 1; }
	// $deBlob: FUNCTION 00772e80
	TUINT8 GetBossStage() const { return m_uiBossStage; }

private:
	void EnableProjections();
	void DisableProjections();
	void UpdateProjections( TFLOAT a_fDeltaTime );
	void RestoreBuildingsCollision();
	void SetBuildingsLocked( TBOOL a_bLocked );

	// Sends MESSAGE_TRIGGER to the platform targets
	void TriggerPlatforms();

private:
	TUINT8 m_eBTState;         // 0x90 BTState
	TUINT8 m_uiBTFlags;        // 0x91 BTFLAG
	TUINT8 m_uiRequiredColour; // 0x92 "Required Colour" (ABlobColourComponent::COLOUR)
	TUINT8 m_uiBossStage;      // 0x93 "BossStage", the round of the boss
	ABoss* m_pBoss;            // 0x94 the ABoss among the entity targets
};
