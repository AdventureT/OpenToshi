#pragma once
#include "Entities/ABaseEntity.h"

TOSHI_NAMESPACE_START
class TModelInstance;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: a god ray model rendered through an AEntityFactory callback, toggled visible when triggered.
// NOTE: names from the entity params ("Mesh", "Start Invisible", "Toggle Vis When Triggered")
//-----------------------------------------------------------------------------
class AGodRay
    : public ABaseEntity
{
public:
	TDECLARE_CLASS( AGodRay, ABaseEntity );

	enum GODRAYFLAG : TUINT32
	{
		GODRAYFLAG_StartInvisible = 0x1,
		GODRAYFLAG_ToggleVis      = 0x2,
		GODRAYFLAG_Visible        = 0x4,
	};

public:
	AGodRay();
	virtual ~AGodRay() override; // +0x08

	virtual void Destroy() override;                                                                  // +0x24
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8

private:
	static void RenderCallback( void* a_pUserData );

private:
	Toshi::TModelInstance* m_pModelInstance; // 0x80
	TUINT32                m_uiGodRayFlags;  // 0x84 GODRAYFLAG
};
