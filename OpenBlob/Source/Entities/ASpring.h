#pragma once

//-----------------------------------------------------------------------------
// Purpose: a damped mass spring along one axis, used by the "Bounce" of APropPlatform.
//-----------------------------------------------------------------------------
class ASpring
{
public:
	ASpring( TFLOAT a_fPosition, TFLOAT a_fStiffness, TFLOAT a_fDamping );
	~ASpring();

	// Integrates the spring over a_fDeltaTime
	void Tick( TFLOAT a_fDeltaTime );

	// The same for an angle (the offset and the position wrapped to -PI..PI)
	void TickAngle( TFLOAT a_fDeltaTime );

	// a_fX wrapped to -PI..PI (|a_fX| below 100 turns)
	static TFLOAT WrapAngle( TFLOAT a_fX );

	// Adds to the velocity (the touch of a bouncing platform)
	// $deBlob: FUNCTION 005ea550
	void AddVelocity( TFLOAT a_fVelocity ) { m_fVelocity += a_fVelocity; }

	// Sets the damping as a ratio of the critical damping
	void SetDampingRatio( TFLOAT a_fRatio );

	// Sets the damping to the critical damping
	void SetCriticalDamping();

	// Sets the mass and its inverse
	void SetMass( TFLOAT a_fMass );

public:
	TFLOAT m_fPosition;    // 0x00
	TFLOAT m_fRest;        // 0x04
	TFLOAT m_fVelocity;    // 0x08
	TFLOAT m_fStiffness;   // 0x0C
	TFLOAT m_fDamping;     // 0x10
	TFLOAT m_fMass;        // 0x14 1.0f
	TFLOAT m_fInverseMass; // 0x18 1.0f
};
