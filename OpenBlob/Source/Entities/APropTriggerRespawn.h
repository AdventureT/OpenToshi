#pragma once
#include "Entities/APropTrigger.h"

//-----------------------------------------------------------------------------
// Purpose: a trigger that sets the blob's respawn point (APropTriggerRespawn\APropTriggerRespawn.cpp, plus
// ATriggerUtil's 6 at 0x84).
//-----------------------------------------------------------------------------
class APropTriggerRespawn
    : public APropTrigger
{
public:
	TDECLARE_CLASS( APropTriggerRespawn, APropTrigger );

	enum RESPAWNFLAG : TUINT8
	{
		RESPAWNFLAG_HasPoint = 0x1,
		RESPAWNFLAG_PlaySeq  = 0x2,
		RESPAWNFLAG_TransSeq = 0x4,
	};

public:
	APropTriggerRespawn();
	virtual ~APropTriggerRespawn() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C (also ATriggerUtil +0x04)
	virtual void Trigger() override;                                       // +0xE4

private:
	Toshi::TVector4 m_vRespawnPosition; // 0xC0
	Toshi::TVector4 m_vRespawnRotation; // 0xD0
	TFLOAT          m_fSeqStartRatio;   // 0xE0
	TFLOAT          m_fSeqEndRatio;     // 0xE4
	TUINT8          m_uiRespawnFlags;   // 0xE8 RESPAWNFLAG
};
