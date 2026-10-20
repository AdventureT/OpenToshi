#pragma once
#include "Entities/AEntityComponent2.h"

class ABlobEntity;

namespace FMOD
{
class Event;
}

//-----------------------------------------------------------------------------
// Purpose: the sounds of Blob: the rolling and swimming loops (pitched by the speed, muted when slow), the one shot
// effects of the state changes.
//-----------------------------------------------------------------------------
class ABlobSoundComponent
    : public AEntityComponent2
{
public:
	// m_uiFlags
	enum FLAGS : TUINT16
	{
		FLAGS_ROLL_MUTED   = 1 << 0,
		FLAGS_SWIM_MUTED   = 1 << 1,
		FLAGS_AIR_TIMER    = 1 << 3,
		FLAGS_ZJUMP_FLIGHT = 1 << 4,
	};

public:
	ABlobSoundComponent( ABlobEntity* a_pBlob );
	~ABlobSoundComponent();

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	// The splash of the jump out of the water or ink
	void PlayLiquidJump();
	// The splash of the Z-jump launch: a flight longer than 1.25 s also starts the air timer (FLAGS_AIR_TIMER) for the
	// rest of the flight
	void PlayZJumpLaunch( TFLOAT a_fFlightTime );
	// Stops m_pZJumpEvent and plays the Z-jump landing (ABlobZJumpComponent)
	void PlayZJumpLand();

	// The Z-jump lock sounds (ABlobZJumpComponent)
	// Stops m_pZJumpEvent and starts the loop of a Z-jump point (event 0x3B) in it
	void StartZJumpPointLoop();
	// The lock on a target (event 0x2F)
	void PlayLockOn();
	// The aim of state 4 (event 0x30)
	void PlayLockAim();
	// The lock went to another target (event 0x30)
	void PlayLockChange();

	// The blob touches a locked building: plays event 1/0x1D in m_pLockedBuildingEvent (if none plays) and restarts m_fTime;
	// a_pUnused isn't read (ABuildingInstance passes the blob, ABlobEntity's collision the landmark)
	void PlayLockedBuilding( void* a_pUnused );
	// Called with the Z-jump point lock (does nothing)
	void OnZJumpPoint();
	// Stops m_pZJumpEvent and plays the end of the Z-jump set (event 0x3A)
	void PlayZJumpEnd();
	// The landing on a Z-jump node: events 0x33-0x39, one higher per node (m_uiZJumpNodeCount, at most 7)
	void PlayZJumpNode();

	// The "pattern_pickup" music sound at Blob's body position (APickupPattern::OnCollected)
	void PlayPatternPickup();

	// The life and energy pickups' sounds at Blob's body position
	void PlayLifePickup();
	void PlayEnergyPickup();

	// The bonus pickup's sound (APickupBonus::OnCollected): does nothing
	// $deBlob: FUNCTION 0075e110
	void PlayBonusPickup() {}

	// The music follows the colour Blob is about to take (ink or not)
	void OnColourChanging( TUINT32 a_uiColour );

	// A paint point lost to the water
	void PlayPaintLost();

	// The splash of Blob falling into the water or ink (by Blob's size)
	void PlayLiquidEnter();

	// Stops m_pHurtLoopA and m_pHurtLoopB
	void StopHurtLoops();

	// A hurting surface (ABlobEntity::UpdateHazards): starts m_pHurtLoopB (0x61) and m_pHurtLoopA (0x60)
	void StartHurtLoops();
	// A hurting surface hit hard or again (event 0x65) or not (0x62)
	void PlayHurtHard();
	void PlayHurt();
	// Stops m_pZJumpEvent, the "shaker_loop" music sound and the m_pHurtLoopA/34 events (ABlobEntity::Reset)
	void StopLoops();

	// Mutes (or unmutes) the sound breakpoints of the character model
	void SetSoundsEnabled( TBOOL a_bEnabled );
	// Stops m_pZJumpEvent and clears FLAGS_ZJUMP_FLIGHT
	void StopZJumpEvent();
	void PlaySizeUp();
	void PlaySizeDown();

	// The electrocution loop (m_pElectrocuteEvent) and the "electrocution" music snapshot
	void StartElectrocution();
	// The shot of the wind cannon
	void PlayCannonFire();
	void StopElectrocution();

	// On a hot plate (ABlobEntity::STATE_HOTPLATE): its sounds (0x5A, the loop 0x5B) and the music snapshot
	void StartHotPlate();
	void StopHotPlate();

	// Blob SFX event 38 at Blob's body position
	void PlayEvent38();
	// DOES_NOTHING
	void NoEvent38();
	// The "shaker_single" music SFX at Blob's body position
	void PlayShakerSingle();
	// The "electrocution" music snapshot in single player (when not inked), and its release
	void PlayElectrocutionSnapshot();
	void ReleaseElectrocutionSnapshot();

	// Shaking a landmark or the boss (ABlobEntity::STATE_LMARK/STATE_BOSS): LM_COMMON 211 and the "shaker_loop" music
	// sound
	void StartShaker();
	void StopShaker();
	// The "shaker_loop" PLoop parameter: 1 while shaking, 0 once the shaking stopped
	void SetShakerOn();
	void SetShakerOff();

	// Mutes the roll and swim loops the state a_pBlob is in doesn't use (ABlobEntity::SetState)
	void MuteLoopsForState();

	void PlayShieldHit();

	// The music when Blob respawns: the snapshot is released and a kept pitch change reset
	void        OnRespawn();

	// Blob pops (ABlobEntity::PopDeath): stops m_pZJumpEvent, plays BLOB_SFX 5 and slows the music down (single player, not
	// in the last 2 seconds of a sprint level)
	void PlayDeath();

	// The jelly squashes against a contact (ABlobEntity::UpdateModelTransforms): the bounce (BLOB_SFX 0 and the
	// "blob_bounce" music SFX) once the squash goes below 0.7, again after it went back over 0.8
	void UpdateSquashBounce();


	// The paint squeeze starts (ABlobEntity::Tick, multiplayer): event 230 of group 16
	void PlaySqueeze();

	// The start of the rainbow paint (ABlobEntity::StartRainbow): COMMON event 42 and the "rainbow" music snapshot
	void PlayRainbow();

	// The end of the rainbow paint (ABlobEntity::EndRainbow): releases the "rainbow" music snapshot if it plays
	static void ReleaseRainbowSnapshot();

private:
	ABlobEntity* m_pBlob;                // 0x14
	TBOOL        m_bBouncePlayed;        // 0x18
	FMOD::Event* m_pRollEvent;           // 0x1C BLOB_SFX 6, looping
	FMOD::Event* m_pZJumpEvent;          // 0x20
	FMOD::Event* m_pSwimEvent;           // 0x24 BLOB_SFX 7, looping
	FMOD::Event* m_pHotPlateEvent;       // 0x28 INTERACTABLES 0x5B, looping
	FMOD::Event* m_pElectrocuteEvent;    // 0x2C INTERACTABLES 87, looping
	FMOD::Event* m_pHurtLoopA;           // 0x30 follows the ball outside the water
	FMOD::Event* m_pHurtLoopB;           // 0x34 follows the ball outside the water
	FMOD::Event* m_pLockedBuildingEvent; // 0x38 stopped once m_fTime passes 0.25 s
	TUINT32      m_uiZJumpNodeCount;     // 0x3C
	TFLOAT       m_fAirTimer;            // 0x40 FLAGS_AIR_TIMER is cleared when it runs out (STATE_BALLISTIC)
	TFLOAT       m_fTime;                // 0x44
	TUINT16      m_uiFlags;              // 0x48 FLAGS
};                                       // 0x4C
