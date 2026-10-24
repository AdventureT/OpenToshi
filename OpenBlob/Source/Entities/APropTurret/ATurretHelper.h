#pragma once
#include <Math/TVector4.h>

class ABaseEntity;
class ABlobEntity;
class APropTurret;
class AProjectile;

//-----------------------------------------------------------------------------
// Purpose: the aiming helper of a turret (the tank's turret in ACar, APropTurret). It owns the turret's facing and
// its aim at the nearest blob, and plays the looping turret sounds of its owner.
// APropTurret\ATurretHelper.cpp, 0xA8 bytes, not a TClass.
//-----------------------------------------------------------------------------
class ATurretHelper
{
public:
	friend class APropTurret; // the turret animations read the yaw and the turning (0x5C, 0x64) and set the bone origin
	friend class ACar;        // the tank does the same with its turret bones

public:
	enum TURRETFLAG : TUINT8
	{
		TURRETFLAG_Unk1         = 0x1,  // 0x5b bit 0, toggled by Tick
		TURRETFLAG_Turning      = 0x2,  // 0x5b bit 1, set while the turret is still turning
		TURRETFLAG_SoundLoop140 = 0x4,  // looping event 0x140 of the owner
		TURRETFLAG_SoundLoop148 = 0x8,  // looping event 0x148 of the owner
		TURRETFLAG_SoundLoop144 = 0x10, // looping event 0x144 of the owner
		TURRETFLAG_SoundLoop145 = 0x20, // looping event 0x145 of the owner
	};

public:
	ATurretHelper( ABaseEntity* a_pOwner );
	~ATurretHelper();

	TBOOL            Tick( TFLOAT a_fDeltaTime );
	void             Reset();
	void             Initialise( TFLOAT a_fSearchAngle, TFLOAT a_fFireDelay, TFLOAT a_fGravity, TFLOAT a_fProjectileSpeed, TFLOAT a_fRotateSpeed, TFLOAT a_fVisualRangeA, TFLOAT a_fVisualRangeB, TFLOAT a_fMaxVisualAngleA, TFLOAT a_fMaxVisualAngleB, TINT a_iMaxBurst, TFLOAT a_fBurstDelay );
	void             SetFacing( const Toshi::TVector4& a_rvFacing );
	void             SetState( TINT a_eState );
	void             ReleaseTarget();
	TFLOAT           GetCurrentTimer() const;
	void             OnFire( AProjectile* a_pProjectile );
	TBOOL            UpdateTargeting();
	TBOOL            UpdateAnimation( TFLOAT a_fDeltaTime );
	void             UpdateSounds( TFLOAT a_fPitch, TFLOAT a_fYaw );
	Toshi::TVector4* GetAimDirection( Toshi::TVector4* a_pOut ) const;
	void             PlayPositionalSound();
	void             SetFlagUnk1( TBOOL a_bSet );
	void             SetTurning( TBOOL a_bSet );
	void             StopEffects();

	// $deBlob: FUNCTION 007c9930
	const Toshi::TVector4& GetOrigin() const { return m_vOrigin; }
	// $deBlob: FUNCTION 007c9940
	TBOOL IsFlagUnk1() const { return m_uiFlags & TURRETFLAG_Unk1; }
	// $deBlob: FUNCTION 007c9950
	const Toshi::TVector4& GetFacing() const { return m_vFacing; }
	// $deBlob: FUNCTION 007c9960
	const Toshi::TVector4& GetLookAt() const { return m_vLookAt; }

public:
	static TBOOL        IsInCone( const Toshi::TVector4& a_rvOrigin, const Toshi::TVector4& a_rvConeDirection, TFLOAT a_fConeFOV, TFLOAT a_fConeRange, const Toshi::TVector4& a_rvTarget );
	static const TCHAR* GetProjectileDataName( TINT a_iIndex );

private:
	Toshi::TVector4 m_vRight;           // 0x00 (cross of the facing and -Y)
	Toshi::TVector4 m_vOrigin;          // 0x10 the turret position (the ray origin)
	Toshi::TVector4 m_vAim;             // 0x20 ballistic velocity to the blob (UpdateTargeting, not normalised in memory)
	Toshi::TVector4 m_vLookAt;          // 0x30 the point the aim is lerped to
	Toshi::TVector4 m_vFacing;          // 0x40 GetFacing()
	ABaseEntity*    m_pOwner;           // 0x50 the car, owner of the sounds and the target check
	ABlobEntity*    m_pTarget;          // 0x54 the blob targeted (counted in its m_uiTargettingAndShakes)
	TUINT8          m_eState;           // 0x58 state of the Tick state machine (0..3)
	TINT8           m_iCounter;         // 0x59 shots of the current burst
	TINT8           m_iMaxBurst;        // 0x5A "a_iMaxBurst" (default 3)
	TUINT8          m_uiFlags;          // 0x5B TURRETFLAG
	TFLOAT          m_fAngleFrac;       // 0x5C current yaw as a fraction of a turn (GetAimDirection)
	TFLOAT          m_fTimer60;         // 0x60 idle timer
	TFLOAT          m_fPitchFrac;       // 0x64 turning amount (UpdateAnimation)
	TFLOAT          m_fTargetPitchFrac; // 0x68
	TFLOAT          m_fFireTimer;       // 0x6C
	TFLOAT          m_fBurstTimer;      // 0x70
	TFLOAT          m_fIdleTime;        // 0x74 the idle time of Tick (1.0 - it or it)
	TFLOAT          m_fGravity;         // 0x78
	TFLOAT          m_fProjectileSpeed; // 0x7C
	TFLOAT          m_fRotateSpeed;     // 0x80 turn speed, radians per second ("fLerpAmount")
	TFLOAT          m_fMaxVisualAngleA; // 0x84 min( a_fMaxVisualAngleA, PI )
	TFLOAT          m_fMaxVisualAngleB; // 0x88 min( a_fMaxVisualAngleB, PI )
	TFLOAT          m_fVisualRangeA;    // 0x8C
	TFLOAT          m_fVisualRangeB;    // 0x90
	TFLOAT          m_fFireDelay;       // 0x94
	TFLOAT          m_fBurstDelay;      // 0x98
	TFLOAT          m_fPitchMin;        // 0x9C
	TFLOAT          m_fPitchMax;        // 0xA0 upper bound of m_fPitchFrac
	TFLOAT          m_fLOSTimer;        // 0xA4
};

TSTATICASSERT( sizeof( ATurretHelper ) == 0xA8 );
