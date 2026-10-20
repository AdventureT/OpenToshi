#pragma once
#include "Entities/AVisibleEntity.h"

//-----------------------------------------------------------------------------
// Purpose: a decorative model that plays an idle animation (AIdleEntity\AIdleEntity.cpp): "Mesh", "DefaultAnimation"
// (from "DefaultAnimationStartTime", negative: from its end), hidden until MESSAGE_TRIGGER or 0x28 unless
// "StartVisible".
//-----------------------------------------------------------------------------
class AIdleEntity
    : public AVisibleEntity
{
public:
	TDECLARE_CLASS( AIdleEntity, AVisibleEntity );

	enum IDLEFLAG : TUINT8
	{
		IDLEFLAG_StartVisible = 0x1,
	};

public:
	AIdleEntity();
	virtual ~AIdleEntity() override; // +0x08

	virtual void Create() override;                                                                   // +0x1C
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8

	// $deBlob: FUNCTION 007b6520
	virtual const TCHAR* GetModelName() override { return TNULL; } // +0xEC

private:
	// Shows the entity if it starts visible, else hides it and stops its ticks
	void ApplyStartVisible();

private:
	TUINT8 m_uiIdleFlags; // 0x9C IDLEFLAG
};
