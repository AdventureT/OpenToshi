#pragma once
#include "AEntityComponent2.h"

#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: shakes its entity when it's hit: three rocking swings around the axis facing away from the hitter, each one
// damped, and a bounce up and down. The tuning comes from the global entity data
// "GlobalData_AShakerComponentData_EntityData".
// NOTE: retail never creates the component, only its data is loaded and unloaded by AEntityFramework.
//-----------------------------------------------------------------------------
class AShakerComponent
    : public AEntityComponent2
{
public:
	// The global tuning (the entity data symbol)
	struct Data
	{
		TUINT32 m_uiUnk00;      // 0x00
		TFLOAT  m_fSwingSpeed;  // 0x04 radians per second
		TFLOAT  m_fSwingAngle;  // 0x08 degrees of the first swing
		TFLOAT  m_fSwingDamp;   // 0x0C the factor of each next swing
		TFLOAT  m_fBounceSize;  // 0x10 the height of the bounce
		TFLOAT  m_fBounceSpeed; // 0x14 per second
	};

	static TCONSTEXPR_VAR TINT NUM_SWINGS = 3;

	// One swing: the axis (the direction index of GetDirection, 0/1 around Z, 2/3 around X) and the angle
	struct Swing
	{
		TINT   m_iDirection; // 0x00
		TFLOAT m_fAngle;     // 0x04
	};

public:
	AShakerComponent( ABaseEntity* a_pEntity );
	virtual ~AShakerComponent() override; // +0x00

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x04
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x10

	// Starts the shake away from a_pHitter (TNULL: along +X) unless it's shaking
	void Start( ABaseEntity* a_pHitter );

	// Ends the swing and the bounce
	void Stop();

	// Reads sm_pData from the entity data (AEntityFramework::Startup)
	static void LoadData();
	// Forgets sm_pData and reads it again
	static void ReloadData();
	// Forgets sm_pData (AEntityFramework::Shutdown)
	static void UnloadData();

private:
	// The index of the direction (+X, -X, -Z, +Z) closest to a_vDirection
	TINT GetDirection( Toshi::TVector4 a_vDirection );

	// Sets up the swings and the bounce for a hit from a_rvDirection
	void SetupShake( Toshi::TVector4& a_rvDirection );

private:
	TINT   m_iSwing;                // 0x14 the current swing, -1: none
	TFLOAT m_fAngle;                // 0x18 the angle of the current half swing
	TBOOL  m_bSwingBack;            // 0x1C the second half of the swing
	Swing  m_aSwings[ NUM_SWINGS ]; // 0x20
	TFLOAT m_fBounce;               // 0x38 0..1
	TFLOAT m_fBounceSize;           // 0x3C
	TFLOAT m_fBounceBaseY;          // 0x40 the entity's Y when the shake started
	TINT   m_iBounce;               // 0x44 the current bounce, -1: none
	TBOOL  m_bBounceDown;           // 0x48

	static Data* sm_pData;
};
