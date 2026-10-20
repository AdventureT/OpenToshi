#pragma once
#include "Entities/AInvisibleEntity.h"
#include "FXComposition/AFXCompositionInstance.h"

//-----------------------------------------------------------------------------
// Purpose: plays an FX composition at its position, toggled when triggered.
// NOTE: names from the entity params ("StopAfterTriggered", "On", "EffectName")
//-----------------------------------------------------------------------------
class APropFX
    : public AInvisibleEntity
{
public:
	TDECLARE_CLASS( APropFX, AInvisibleEntity );

	enum FXFLAG : TUINT32
	{
		FXFLAG_Triggered          = 0x1,
		FXFLAG_On                 = 0x2,
		FXFLAG_StopAfterTriggered = 0x4,
	};

public:
	APropFX();
	virtual ~APropFX() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8

private:
	TUINT32                m_uiFXFlags; // 0x84 FXFLAG
	AFXCompositionInstance m_oFX;       // 0x88
	TBOOL                  m_bOn;       // 0x170 "On"
};
