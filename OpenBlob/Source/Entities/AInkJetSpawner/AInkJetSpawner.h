#pragma once
#include "Entities/AVisibleEntity.h"

class AInkJet;

//-----------------------------------------------------------------------------
// Purpose: the spawner of an ink jet (Entities\AInkJetSpawner\AInkJetSpawner.cpp, parent AVisibleEntity): it finds its
// AInkJet by the ID of its parameters (ProcessParams, FindIDEntity) and runs the cycle of the jet (states 0 to 4: wait
// for the trigger, 5 s, a HandleEntityIO to the jet, 0.8 s, back to 0).
// The layout is checked by TSTATICASSERT (0x9C is AVisibleEntity's size).
//-----------------------------------------------------------------------------
class AInkJetSpawner
    : public AVisibleEntity
{
public:
	TDECLARE_CLASS( AInkJetSpawner, AVisibleEntity );

public:
	AInkJetSpawner();
	virtual ~AInkJetSpawner() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x0C
	virtual void Create() override;                    // +0x1C
	// $deBlob: FUNCTION 007ae210
	virtual void         Destroy() override { ABaseEntity::Destroy(); }                                       // +0x24
	virtual void         ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void         HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void         Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual const TCHAR* GetModelName() override;                                                             // +0xEC

protected:
	AInkJet* m_pInkJet;    // 0x9C
	TUINT8   m_bTriggered; // 0xA0 (set by HandleEntityIO and cleared by Reset)
};
