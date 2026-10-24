#pragma once
#include "Entities/APropTrigger.h"

//-----------------------------------------------------------------------------
// Purpose: a voice-over trigger volume: triggered by a character entity with flag 0x10 (a blob); its Trigger does
// nothing and "VO_ID" is read but not kept.
// NOTE: names from the entity params ("VO_ID")
//-----------------------------------------------------------------------------
class APropTriggerVO
    : public APropTrigger
{
public:
	TDECLARE_CLASS( APropTriggerVO, APropTrigger );

public:
	APropTriggerVO();
	virtual ~APropTriggerVO() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C (also ATriggerUtil +0x04)
	virtual void PostCreate() override;                                    // +0x20 (also ATriggerUtil +0x08)

	virtual TBOOL CanObjectTrigger( Toshi::TObject* a_pObject ) override; // +0xE0
	// $deBlob: FUNCTION 0079c170
	virtual void Trigger() override {} // +0xE4

private:
	TUINT32 m_uiUnkC0; // 0xC0 not initialised nor used
};
