#pragma once
#include <TParticleSystem/TParticleEffect.h>
#include "Entities/AProp.h"

TOSHI_NAMESPACE_START
class TParticleEffectInstance;
TOSHI_NAMESPACE_END

class AEntityModelComponent;
class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: a water hydrant that the blob uses to wash off its colour (APropWaterHydrant\APropWaterHydrant.cpp, plus
// ATriggerUtil's 6 at 0x9C).
//-----------------------------------------------------------------------------
class APropWaterHydrant
    : public AProp
{
public:
	TDECLARE_CLASS( APropWaterHydrant, AProp );

public:
	APropWaterHydrant();
	virtual ~APropWaterHydrant() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;  // +0x0C
	virtual void OnLODChanging( TINT a_iLOD ) override; // +0x10
	// $deBlob: FUNCTION 0077dc40
	virtual void Create() override { AProp::Create(); }                    // +0x1C
	virtual void PostCreate() override;                                    // +0x20
	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C
	// $deBlob: FUNCTION 0077deb0
	virtual void SetVisible( TBOOL a_bVisible ) override { AVisibleEntity::SetVisible( a_bVisible ); } // +0x38
	// $deBlob: FUNCTION 0077e120
	virtual TBOOL IsDead() const override { return m_bUsed; }                                          // +0x3C
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	// $deBlob: FUNCTION 0077dce0
	virtual TBOOL IsTargetable() override { return TTRUE; } // +0xA0
	virtual void  Reset( ABaseEntity* a_pEntity ) override; // +0xC8

	// Switches to the used mesh and plays the water while a_bUsed
	void SetUsed( TBOOL a_bUsed );

	// Finds the level's effects
	static void LoadEffects();
	static void ClearEffects();

private:
	// The water of the hydrant on a blob it reaches: washes the colour off and drains (or fills) the paint points
	// towards the common properties' m_uiWaterPoints (like the water in ABlobEntity's update, with the timers at
	// 0x5B0/0x5B4)
	static void WashBlob( ABlobEntity* a_pBlob, TFLOAT a_fDeltaTime );

	void StartLoopSound();
	void StopLoopSound();

private:
	static Toshi::TParticleEffect* ms_pWaterHydrantEffect;    // "Blob_env_WaterHydrant"
	static Toshi::TParticleEffect* ms_pLeechBotDestroyEffect; // a copy of "Blob_LeechBotDestroy"

	TBOOL                           m_bUsed;         // 0xB4
	TBOOL                           m_bSoundPlaying; // 0xB5
	AEntityModelComponent*          m_pMesh2;        // 0xB8 "Mesh2"
	Toshi::TParticleEffectInstance* m_pParticles;    // 0xBC
};
