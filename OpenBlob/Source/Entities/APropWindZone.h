#pragma once
#include "Entities/APropTrigger.h"

#include <Toshi/T2Vector.h>

TOSHI_NAMESPACE_START
class TParticleEffect;
class TParticleEffectInstance;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: a trigger volume that pushes entities with wind, optionally towards a target position, switching on and off
// (APropWindZone\APropWindZone.cpp, plus ATriggerUtil's 6 at 0x84).
//-----------------------------------------------------------------------------
class APropWindZone
    : public APropTrigger
{
public:
	TDECLARE_CLASS( APropWindZone, APropTrigger );

	enum WINDFLAG : TUINT8
	{
		WINDFLAG_On          = 0x1,
		WINDFLAG_HasEntities = 0x2, // m_vecEntities isn't empty
		WINDFLAG_Blowing     = 0x4,
		WINDFLAG_AlwaysOn    = 0x8,
	};

	static TCONSTEXPR_VAR TINT TYPE_Target  = 3;
	static TCONSTEXPR_VAR TINT MAX_ENTITIES = 4;

public:
	APropWindZone();
	virtual ~APropWindZone() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;  // +0x0C
	virtual void OnLODChanging( TINT a_iLOD ) override; // +0x10
	// $deBlob: FUNCTION 007a2920
	// $deBlob: FUNCTION 007a36d7
	virtual void PostCreate() override { APropTrigger::PostCreate(); }                                // +0x20
	virtual void Destroy() override;                                                                  // +0x24
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C (also ATriggerUtil +0x04)
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8

private:
	static void OnParticlesFinished( Toshi::TParticleEffectInstance* a_pInstance, void* a_pUserData );

	void                                                  SetWindOn( TBOOL a_bOn );
	void                                                  UpdateEntity( ABaseEntity* a_pEntity, TBOOL a_bEntered );
	Toshi::T2Vector<ABaseEntity*, MAX_ENTITIES>::Iterator FindEntity( ABaseEntity* a_pEntity );

	void StartParticles();
	void StopParticles();

private:
	TUINT8                                      m_uiWindFlags;        // 0xC0 WINDFLAG
	TUINT8                                      m_uiType;             // 0xC1 "Type"
	TBOOL                                       m_bWindOn;            // 0xC2 "WindOn"
	TBOOL                                       m_bTargetOnExitFront; // 0xC3 "TargetOnExitFront"
	TFLOAT                                      m_fOnDuration;        // 0xC4 "OnDuration"
	TFLOAT                                      m_fOffDuration;       // 0xC8 "OffDuration"
	TFLOAT                                      m_fTimer;             // 0xCC
	Toshi::TParticleEffectInstance*             m_pParticleInstance;  // 0xD0
	Toshi::TParticleEffect*                     m_pParticleEffect;    // 0xD4 "Particle"
	Toshi::T2Vector<ABaseEntity*, MAX_ENTITIES> m_vecEntities;        // 0xD8 the entities in the zone
	TINT                                        m_iRequiredSize;      // 0xEC "RequiredSize"
	Toshi::TVector4                             m_vTargetBase;        // 0xF0
	Toshi::TVector4                             m_vTargetPos;         // 0x100 "TargetPos"
};
