#pragma once
#include <Math/TVector4.h>

class ACamera;

//-----------------------------------------------------------------------------
// Purpose: the camera shakes: up to 10 running shakes started from the level's "CommonCameraShake" presets
// (SHAKE_CUSTOM_TOTAL of them).
//-----------------------------------------------------------------------------
class ACameraShakeFunction
{
public:
	static TCONSTEXPR_VAR TINT SHAKE_CUSTOM_TOTAL = 12;
	static TCONSTEXPR_VAR TINT MAX_SHAKES         = 10;

	enum StatusFlag : TUINT32
	{
		SF_LOOPING_EFFECT_PULSE    = 0x02,
		SF_LOOPING_EFFECT_CONSTANT = 0x04,
		SF_SQUARE_START            = 0x08, // the ramp up is squared
		SF_SQUARE_END              = 0x10, // the ramp down is squared
		SF_ENABLED                 = 0x20,
		SF_UNK40                   = 0x40, // set with SF_ENABLED for the looping effects
		SF_FROM_BLOB               = 0x80, // the distance is from the camera's blob (not for camera types 2 and 4)
	};

	// A shake (a preset of CommonCameraShake, or a running one)
	struct ShakeData
	{
		Toshi::TVector4 m_vPosition;        // 0x00
		TINT            m_iID;              // 0x10
		TFLOAT          m_fShake_Strength;  // 0x14
		TFLOAT          m_fRadius;          // 0x18 full strength within it, -1: everywhere
		TFLOAT          m_fFallOffDistance; // 0x1C beyond m_fRadius
		TFLOAT          m_fDelay;           // 0x20 before it starts
		TFLOAT          m_fStart_Time;      // 0x24 the ramp up
		TFLOAT          m_fHold_Time;       // 0x28 at full strength
		TFLOAT          m_fEnd_Time;        // 0x2C the ramp down
		TFLOAT          m_fUnk30;           // 0x30 after the ramp down (part of the length)
		TFLOAT          m_fLookAtDirEffect; // 0x34 0..1: how much looking away from it weakens it
		TFLOAT          m_fElapsedTime;     // 0x38 the time since it was added
		TFLOAT          m_fCurrentStrength; // 0x3C
		TUINT32         m_iStatusFlag;      // 0x40 SF_*
		TINT            m_iPlayerEffected;  // 0x44 -1: all
		TUINT32         m_auiUnk48[ 2 ];    // 0x48
	};

	TSTATICASSERT( sizeof( ShakeData ) == 0x50 );

	// More shakes updated with the running ones (0x328)
	struct ShakeArray
	{
		ShakeData* m_pShakes;    // 0x0
		TINT       m_iNumShakes; // 0x4
	};

public:
	// $deBlob: FUNCTION 0058f4c0
	ACameraShakeFunction()
	    : m_iNumShakes( 0 ), m_iNextID( 0 ), m_pExtraShakes( TNULL ), m_bShaking( TFALSE )
	{}

	// $deBlob: FUNCTION 0058f4f0
	~ACameraShakeFunction() { m_pExtraShakes = TNULL; }

	// $deBlob: FUNCTION 0058f5c0
	// $deBlob: FUNCTION 0058e6b0
	void Clear() { m_pExtraShakes = TNULL; }

	// Starts a shake of the preset a_eType at a_rPosition, its strength scaled by a_fStrength, for a_iPlayer (-1: all);
	// returns its ID, -1 if MAX_SHAKES are running
	TINT AddShake( TINT a_eType, const Toshi::TVector4& a_rPosition, TFLOAT a_fStrength, TINT a_iPlayer );

	// Ends the looping of the running shake a_iID (clears SF_UNK40)
	void StopShake( TINT a_iID );

	// The placed shake (m_pExtraShakes) with a_iID, TNULL if there's none
	ShakeData* FindPlacedShake( TINT a_iID );
	// Starts a copy of a_rShake with a new ID; returns it, -1 if MAX_SHAKES are running
	TINT AddShake( const ShakeData& a_rShake );
	// Disables the running shake a_iID (clears SF_ENABLED)
	void DisableShake( TINT a_iID );

	// Reads/forgets the presets of the level (CommonCameraShake of the CameraShakeCommon data block)
	// Reads the shakes placed in the level (CameraShakeHeader of the CameraShake data block) and resets them and the
	// running ones
	void LoadLevelShakes();

	// Stops the running shakes and resets the placed ones (m_pExtraShakes) to their start state
	void ResetShakes();

	static void LoadShakeTypes();
	static void UnloadShakeTypes();

	static ShakeData* GetShakeType( TINT a_eType );

	// Drops the finished shakes and updates the strengths of the others (and of m_pExtraShakes)
	void Update( TFLOAT a_fDeltaTime );

	// Advances a_ShakeData by a_fDeltaTime: its m_fCurrentStrength from the delay/start/hold/end times; clears
	// SF_ENABLED when it's over (unless looping)
	void UpdateShake( TFLOAT a_fDeltaTime, ShakeData& a_ShakeData );

	// Starts the placed shake (m_pExtraShakes) with a_iID unless it runs
	void StartShake( TINT a_iID );

	// Stops the looping placed shake with a_iID
	void StopLoopingShake( TINT a_iID );
	// Disables the extra shake a_iID (any kind)
	void StopExtraShake( TINT a_iID );

	// The strength of a_ShakeData for a_pCamera (its distance, direction and player), 0 if it's not above a_fMax
	static TFLOAT GetShakeStrength( const ShakeData& a_ShakeData, ACamera* a_pCamera, TFLOAT a_fMax );

	// Shakes a_pCamera (its shake matrix) by the strongest shake
	void ApplyShakes( ACamera* a_pCamera );

private:
	static ShakeData* ms_pShakeTypes;

private:
	ShakeData   m_aShakes[ MAX_SHAKES ]; // 0x000
	TINT        m_iNumShakes;            // 0x320
	TINT        m_iNextID;               // 0x324
	ShakeArray* m_pExtraShakes;          // 0x328
	TBOOL       m_bShaking;              // 0x32C a shake has some strength (Update)
};

TSTATICASSERT( sizeof( ACameraShakeFunction ) == 0x330 );
