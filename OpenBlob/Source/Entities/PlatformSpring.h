#pragma once

//-----------------------------------------------------------------------------
// Purpose: the vertical offset of a platform: a drop over "Distance" (accelerating with "AccelDown"/"AccelUp", capped
// at 9.8/4.0) or a sine oscillation ("Oscillation", "BobOnEnd").
//-----------------------------------------------------------------------------
class PlatformSpring
{
public:
	enum FLAG_ : TUINT8
	{
		FLAG_Off         = 0x1, // the distance is too short
		FLAG_Rising      = 0x2, // moving back up
		FLAG_Unk4        = 0x4,
		FLAG_Oscillating = 0x8,  // a sine oscillation
		FLAG_BobOnEnd    = 0x10, // starts to oscillate (bob) once it's back up
		FLAG_Unk20       = 0x20,
		FLAG_Fading      = 0x40, // the oscillation fades out (m_fFadeSpeed)
	};

public:
	PlatformSpring();

	// Starts a new drop/rise cycle with these parameters (the touch of the platform)
	void Init( TFLOAT a_fAccelDown, TFLOAT a_fAccelUp, TFLOAT a_fDistance, TFLOAT a_fFadeSpeed );

	// Returns the offset after a_fDeltaTime
	TFLOAT Tick( TFLOAT a_fDeltaTime );

public:
	TFLOAT m_fAccelDown; // 0x00 (the oscillation speed)
	TFLOAT m_fAccelUp;   // 0x04
	TFLOAT m_fDistance;  // 0x08
	TFLOAT m_fAmplitude; // 0x0C 1.0f
	TFLOAT m_fFade;      // 0x10 1.0f
	TFLOAT m_fFadeSpeed; // 0x14
	TFLOAT m_fVelocity;  // 0x18
	TFLOAT m_fOffset;    // 0x1C
	TFLOAT m_fPhase;     // 0x20 random [0, 2 PI)
	TUINT8 m_uiFlags;    // 0x24 FLAG_
	TFLOAT m_fWait;      // 0x28
};
