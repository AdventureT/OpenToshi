#pragma once
#include "IO/IIOMessageUpdater.h"

//-----------------------------------------------------------------------------
// Purpose: the IO action 0x3B, rotates the entity around one axis over time: by its angle in a duration, or (duration
// -1) to an angle with an acceleration and a maximum speed.
//-----------------------------------------------------------------------------
class IORotateUpdater
    : public IIOMessageUpdater
{
public:
	// The parameters of the action
	struct Params
	{
		TFLOAT fDuration;        // 0x00 -1: the speed is used
		TFLOAT fAngle;           // 0x04 degrees
		TINT   iLoop;            // 0x08
		TINT   iAxis;            // 0x0C 0: X, 1: Z, 2: Y
		TINT   iAbsolute;        // 0x10 0: rotates by fAngle, else to fAngle
		TFLOAT fMaxSpeed;        // 0x14
		TFLOAT fAcceleration;    // 0x18
		TINT   iDirection;       // 0x1C 1: forwards, 2: backwards, else the shorter way
		TINT   iUnk20;           // 0x20
		TINT   iUnk24;           // 0x24
		TINT   iFinishedMessage; // 0x28 SendFinishedMessage
	};

public:
	IORotateUpdater() {}

	// $deBlob: FUNCTION 00786b10
	// $deBlob: FUNCTION 00786920
	virtual ~IORotateUpdater() override {} // +0x00

	virtual void Update( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void Activate() override;                    // +0x08

private:
	// NOTE: named by what they do
	void RotateUsingDuration( TFLOAT a_fDeltaTime, const Params* a_pParams );
	void RotateUsingVelocity( TFLOAT a_fDeltaTime, const Params* a_pParams );

	// m_fStartAngle: the entity's angle around the axis in degrees (unchanged for an unknown axis)
	void ReadStartAngle( const Params* a_pParams );

private:
	TBOOL  m_bReverse;     // 0x1C
	TFLOAT m_fSpeed;       // 0x20
	TFLOAT m_fStartAngle;  // 0x24
	TFLOAT m_fTotalAngle;  // 0x28
	TFLOAT m_fRotated;     // 0x2C
	TBOOL  m_bMessageSent; // 0x30
	TINT   m_iUnk34;       // 0x34 not used
};                         // 0x38
