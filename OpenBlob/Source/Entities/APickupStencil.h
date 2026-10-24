#pragma once
#include "Entities/APickup.h"

//-----------------------------------------------------------------------------
// Purpose: a stencil pickup (APickupStencil\APickupStencil.cpp), pickup type 2: one of PT_STENCIL_COUNT stencils
// ("PickupType"); collecting it sets its bit in the collector's APlayerManager::BlobData.
//-----------------------------------------------------------------------------
class APickupStencil
    : public APickup
{
public:
	TDECLARE_CLASS( APickupStencil, APickup );

	static TCONSTEXPR_VAR TINT PT_STENCIL_COUNT = 10; // APickupTypes

public:
	APickupStencil();
	virtual ~APickupStencil() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C (also ATriggerUtil +0x04)
	virtual void OnCollected( ABaseEntity* a_pCollector ) override;        // +0xDC

	// $deBlob: FUNCTION 007b0800
	TINT GetStencilType() const
	{
		TASSERT( m_iStencilType >= 0 && m_iStencilType < PT_STENCIL_COUNT );
		return m_iStencilType;
	}

private:
	TINT8 m_iStencilType; // 0x15C -1
};
