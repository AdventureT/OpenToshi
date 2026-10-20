#pragma once
#include "Entities/AProp.h"

TOSHI_NAMESPACE_START
class TParticleEffect;
class TParticleEffectInstance;
TOSHI_NAMESPACE_END

class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: the wind cannon (APropWindCannon\APropWindCannon.cpp, plus ATriggerUtil's 6 at 0x9C): an AProp whose only
// target is the cannon's moving part (an AProp, its animations "Open", "Open_Loop", "Close", "Close_Loop"). Switched
// on, it opens and seeks: a blob within 5 of the target's position is offered the cannon (ABlobEntity 0xB4C, the action
// button), pulled in (STATE_CANNON), loaded and shot ("shoot", the bone "blob_spawn") towards "TargetPos".
//-----------------------------------------------------------------------------
class APropWindCannon
    : public AProp
{
public:
	TDECLARE_CLASS( APropWindCannon, AProp );

	static TCONSTEXPR_VAR TINT MESH_COUNT = 2;

	enum STATE : TINT
	{
		STATE_CLOSED   = 0,
		STATE_OPENING  = 1,
		STATE_CLOSING  = 2,
		STATE_SEEK     = 3, // open, waiting for a blob
		STATE_OFFERED  = 4, // a blob is near (it may press the action button)
		STATE_ENTERING = 5, // the blob is in (ABlobEntity::SetState( STATE_CANNON ) sent 0x12)
		STATE_LOADED   = 6,
		STATE_FIRING   = 7,
	};

	enum CANNONFLAG : TUINT32
	{
		CANNONFLAG_StartOn    = 0x1,
		CANNONFLAG_On         = 0x2,
		CANNONFLAG_Sound      = 0x4, // the loop of the target is registered
		CANNONFLAG_EntryEnded = 0x8, // the blob is being pulled in: no entry effect
	};

	// The animations of all the cannons (ms_oAnims)
	struct Anims
	{
		TBOOL  m_bLoaded;      // 0x00 never set: they're looked up by every PostCreate
		TINT16 m_iOpen;        // 0x02 "Open" (of the target)
		TINT16 m_iOpenLoop;    // 0x04 "Open_Loop"
		TINT16 m_iClose;       // 0x06 "Close"
		TINT16 m_iCloseLoop;   // 0x08 "Close_Loop"
		TINT16 m_iShoot;       // 0x0A "shoot" (of the cannon)
		TINT16 m_iSpawnBoneID; // 0x0C "blob_spawn"
	};

public:
	APropWindCannon();
	virtual ~APropWindCannon() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void PostCreate() override;                                                               // +0x20
	virtual void Destroy() override;                                                                  // +0x24
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual void SetActive( TBOOL a_bActive ) override;                                               // +0xCC

	void SetState( STATE a_eState );

	// The blob is pulled in (the entry effect and the loop stop) / let go again (ABlobEntity::UpdateInput)
	void OnBlobEntering();
	void OnBlobLeaving();

	// ABlobEntity::SetPhysicsPosition: a_pBlob moved; it's no longer offered the cannon once it's away
	void OnBlobMoved( ABlobEntity* a_pBlob );

	// $deBlob: FUNCTION 00751a60
	STATE        GetState() const { return m_eState; }
	ABaseEntity* GetTarget() const { return m_pTarget; }

private:
	void StartFireEffect();
	void StartLoopSound();
	void StopLoopSound();
	void StartEntry(); // the loop, the entry effects
	void StopEntryEffects( TBOOL a_bInstant );
	void StartTargetEffect();
	void StopFireEffect( TBOOL a_bInstant );
	void StopTargetEffect( TBOOL a_bInstant );

	// The finished callback of the effect instances: clears the slot of a_pInstance
	static void OnEffectFinished( Toshi::TParticleEffectInstance* a_pInstance, void* a_pCannon );

	// Stops and forgets an effect instance
	static void StopEffect( Toshi::TParticleEffectInstance*& a_rpInstance, TBOOL a_bInstant );

	// The effects' matrix: the cannon's rotation at the end of the barrel
	void GetEffectTransform( Toshi::TMatrix44& a_rOutMatrix ) const;

private:
	static Anims ms_oAnims;

private:
	STATE                           m_eState;          // 0xB4
	TUINT32                         m_uiCannonFlags;   // 0xB8 CANNONFLAG
	ABaseEntity*                    m_pTarget;         // 0xBC the moving part (an AProp)
	ABlobEntity*                    m_pBlob;           // 0xC0
	Toshi::TParticleEffectInstance* m_pEntryInstance;  // 0xC4
	Toshi::TParticleEffect*         m_pEntryEffect;    // 0xC8 "Blob_WindLauncher_Suck2"
	Toshi::TParticleEffectInstance* m_pFireInstance;   // 0xCC
	Toshi::TParticleEffect*         m_pFireEffect;     // 0xD0 "Blob_WindLauncher_Fire"
	Toshi::TParticleEffectInstance* m_pTargetInstance; // 0xD4 at the target
	Toshi::TParticleEffect*         m_pTargetEffect;   // 0xD8 never set
	TFLOAT                          m_fEnterTime;      // 0xDC
	TFLOAT                          m_fEnterWait;      // 0xE0 "EnterWait" (0.5)
	TFLOAT                          m_fLoadTime;       // 0xE4
	TFLOAT                          m_fLoadWait;       // 0xE8 "LoadWait" (1.0)
	TFLOAT                          m_fFireTime;       // 0xEC
	Toshi::TVector4                 m_vSpawnEnd;       // 0xF0 the bone "blob_spawnend"
	Toshi::TVector4                 m_vTargetPos;      // 0x100 "TargetPos"
	Toshi::TVector4                 m_vDirection;      // 0x110 down the barrel (-Y of the matrix)
};
