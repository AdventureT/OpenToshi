#pragma once
#include "IO/IIOMessageUpdater.h"

class ABaseEntity;

//-----------------------------------------------------------------------------
// Purpose: the IO action 0x27, shakes the entity (and the entities of the action's target list) once: it tilts away
// from the shaking entity and back three times with less of the angle each time, and bounces up and down three times.
//-----------------------------------------------------------------------------
class IOShakeUpdater
    : public IIOMessageUpdater
{
public:
	static TCONSTEXPR_VAR TINT NUM_STAGES = 3;

	// The parameters of the action
	struct Params
	{
		TINT   iUnk00;        // 0x00
		TFLOAT fTiltSpeed;    // 0x04
		TFLOAT fTiltAngle;    // 0x08 degrees
		TFLOAT fDamping;      // 0x0C the part of the last tilt the next one has (the other way)
		TFLOAT fBounceSpeed;  // 0x10
		TFLOAT fBounceHeight; // 0x14
	};

	// A tilt (8 bytes)
	struct Tilt
	{
		TINT   iDirection; // 0x00 0: +X, 1: -X, 2: -Z, 3: +Z (the direction of the shaking entity)
		TFLOAT fAngle;     // 0x04 radians
	};

public:
	IOShakeUpdater();

	// $deBlob: FUNCTION 00786b70
	// $deBlob: FUNCTION 00786940
	virtual ~IOShakeUpdater() override {} // +0x00

	virtual void Update( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void Activate() override;                    // +0x08

	void SetShaker( ABaseEntity* a_pShaker ) { m_pShaker = a_pShaker; }

private:
	ABaseEntity* m_pShaker;              // 0x1C the entity that shakes this one (TNULL: from +X)
	TINT         m_iTilt;                // 0x20 the current tilt, -1: none
	TFLOAT       m_fTiltAngle;           // 0x24
	TBOOL        m_bTiltBack;            // 0x28
	Tilt         m_aTilts[ NUM_STAGES ]; // 0x2C
	TFLOAT       m_fBounce;              // 0x44 0..1
	TFLOAT       m_fBounceHeight;        // 0x48 0.1f
	TFLOAT       m_fBaseHeight;          // 0x4C
	TINT         m_iBounce;              // 0x50 the current bounce, -1: none
	TBOOL        m_bBounceDown;          // 0x54
};                                       // 0x58
