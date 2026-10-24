#pragma once
#include "Entities/APickup.h"

//-----------------------------------------------------------------------------
// Purpose: the base of the pickups that go away after a while (Pickups\APickupSolid.cpp, can't be created at runtime):
// StartDying (AEnemy::SpawnPickups) waits GetDyingTime, then the pickup flashes GetNumFlashes times (SetAlpha 0.1/1
// every 0.1 s) and is hidden and unhashed.
//-----------------------------------------------------------------------------
class APickupSolid
    : public APickup
{
public:
	TDECLARE_CLASS( APickupSolid, APickup );

	enum SOLIDFLAG : TUINT8
	{
		SOLIDFLAG_Dying  = 0x02, // m_uiPickupFlags: StartDying was called
		SOLIDFLAG_Opaque = 0x04, // m_uiPickupFlags: the flash is at full alpha
	};

public:
	APickupSolid( TUINT8 a_uiPickupType, TINT a_iUnk2, TFLOAT a_fUnk3 );
	virtual ~APickupSolid() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x0C
	virtual void Show() override;                      // +0xE4

	// $deBlob: FUNCTION 007e4bf0
	virtual TINT UnkF4() { return 0; } // +0xF4
	// $deBlob: FUNCTION 007e4bd0
	virtual TFLOAT GetDyingTime() { return 0.0f; } // +0xF8
	// $deBlob: FUNCTION 007e4be0
	virtual TINT GetNumFlashes() { return 0; } // +0xFC
	virtual void SetAlpha( TFLOAT a_fAlpha );  // +0x100

	// Starts the countdown to the flashes (GetDyingTime, then GetNumFlashes flashes)
	void StartDying();

	// $deBlob: FUNCTION 007e4bc0
	TBOOL IsOpaque() const { return ( m_uiPickupFlags & SOLIDFLAG_Opaque ) != 0; }

	// $deBlob: FUNCTION 007e4c00
	TBOOL IsDying() const { return ( m_uiPickupFlags & SOLIDFLAG_Dying ) != 0; }

	// A random velocity: x from a_fSpeed / a_iMaxDivisor to a_fSpeed / a_iMinDivisor times a_fScale, y from -a_fUp to
	// -a_fUp / a_iMinDivisor, z a_fZ; a_iUnused1/a_iUnused2 aren't used
	static Toshi::TVector4 GetRandomVelocity( TFLOAT a_fScale, TINT a_iUnused1, TFLOAT a_fZ, TINT a_iUnused2, TFLOAT a_fSpeed, TFLOAT a_fUp, TINT a_iMinDivisor, TINT a_iMaxDivisor );

private:
	// The flashes once the dying time is over
	void UpdateFlashes( TFLOAT a_fDeltaTime );

private:
	TFLOAT m_fDyingTime;  // 0x15C 3
	TFLOAT m_fDyingTimer; // 0x160
	TFLOAT m_fFlashTimer; // 0x164
	TUINT8 m_uiFlashes;   // 0x168 5
};
