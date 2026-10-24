#pragma once
#include "Entities/AVisibleEntity.h"
#include <Render/TModel.h>

class AInkJet;
class ACollisionObject;
namespace Toshi
{
class TMaterial;
}

//-----------------------------------------------------------------------------
// Purpose: the spawner of a jetbike (AJetbikeSpawner\AJetbikeSpawner.cpp, parent AVisibleEntity): it finds its AInkJet
// by the ID of its parameters and launches it again once the jetbike is gone (IsDead), when the first blob is in
// its cone ("Trigger Distance", PI / 8). States: 0 waits, 1 opens (the jet is prepared after 1.4 s), 2 launches the jet
// at half a second, 3 closes. Deactivated (MESSAGE_TOGGLE) it renders with the spawner colour material
// (RenderCallback), like AStreetSpawner.
// The layout is checked by TSTATICASSERT (0x9C is AVisibleEntity's size).
//-----------------------------------------------------------------------------
class AJetbikeSpawner
    : public AVisibleEntity
{
public:
	TDECLARE_CLASS( AJetbikeSpawner, AVisibleEntity );

	using JSFLAG = TUINT8;
	enum JSFLAG_ : JSFLAG
	{
		JSFLAG_HasTriggerDistance = 0x01, // "Trigger Distance" was set: waits for the blob in the cone
		JSFLAG_Deactivated        = 0x02, // MESSAGE_TOGGLE: never launches again, rendered with sm_pMaterial
		JSFLAG_Reset              = 0x04, // reset by a blob this frame (IsDead), cleared by Tick
		JSFLAG_JetPrepared        = 0x08, // the jet was prepared (AInkJet::Activate) and not launched yet
		JSFLAG_Enabled            = 0x10,
	};

public:
	AJetbikeSpawner();
	virtual ~AJetbikeSpawner() override; // +0x08

	virtual void              Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void              OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void              Create() override;                                                                   // +0x1C
	virtual void              Destroy() override;                                                                  // +0x24
	virtual void              ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual TBOOL             IsDead() const override;                                                             // +0x3C
	virtual void              HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual ACollisionObject* GetCollisionObject() override;                                                       // +0x70
	virtual TBOOL             IsTargetable() override;                                                             // +0xA0
	virtual void              Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual void              Despawn( TBOOL a_bPlayEffect ) override;                                             // +0xE4
	virtual const TCHAR*      GetModelName() override;                                                             // +0xEC

	// $deBlob: FUNCTION 007adfb0
	AInkJet* GetInkJet() const { return m_pInkJet; }

	// Starts the spawner
	void Enable();

	// Restarts the timer
	void RestartTimer();

	// The collision object and the entity flags of Create
	void CreateCollisionObject();

private:
	// The model's render callback: the deactivated spawner renders with sm_pMaterial
	static void __stdcall RenderCallback( Toshi::TModelInstance* a_pModelInstance, void* a_pSpawner );

	// Launches the jet unless it's already out (messages 1 to the jet)
	void LaunchJet();

private:
	static Toshi::TMaterial* sm_pMaterial;   // the spawner colour material
	static TFLOAT            ms_fTriggerFOV; // (initialised at startup)

	Toshi::TModelInstance::t_CustomRenderCB m_fnOldRenderCallback; // 0x9C the model's render callback before Create
	void*                                   m_pOldRenderUserData;  // 0xA0
	JSFLAG                                  m_uiJSFlags;           // 0xA4 JSFLAG
	AInkJet*                                m_pInkJet;             // 0xA8
	ACollisionObject*                       m_pCollisionObject;    // 0xAC
	TFLOAT                                  m_fTriggerDistance;    // 0xB0 "Trigger Distance"
	TFLOAT                                  m_fTimer;              // 0xB4
};
