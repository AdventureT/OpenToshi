#pragma once
#include "Entities/AProp.h"
#include <Toshi/T2DList.h>

TOSHI_NAMESPACE_START
class TParticleEffect;
class TParticleEffectInstance;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: a grass bomb, collected with an attack slam, that plays a scripted sequence (APropGrassBomb\
// APropGrassBomb.cpp, plus ATriggerUtil's 6 at 0x9C).
//-----------------------------------------------------------------------------
class APropGrassBomb
    : public AProp
{
public:
	TDECLARE_CLASS( APropGrassBomb, AProp );

	friend class ABoss; // the defeated boss activates the boss grass bomb (TryActivate)

	enum BOMBFLAG : TUINT8
	{
		BOMBFLAG_SoundPlaying         = 0x1,
		BOMBFLAG_Active               = 0x2,
		BOMBFLAG_BossCompletion       = 0x4,
		BOMBFLAG_BottomDisplayStarted = 0x8,  // the letterbox's bottom display was started
		BOMBFLAG_WaitsForSequence     = 0x10, // waits for its scripted sequence (TryActivate)
	};

public:
	APropGrassBomb();
	virtual ~APropGrassBomb() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                     // +0x0C
	virtual void OnLODChanging( TINT a_iLOD ) override;                    // +0x10
	virtual void Create() override;                                        // +0x1C
	virtual void PostCreate() override;                                    // +0x20
	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C
	// $deBlob: FUNCTION 007882f0
	virtual TBOOL IsDead() const override { return !IsVisible(); }                                     // +0x3C
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual TBOOL IsTargetable() override;                                                             // +0xA0
	// $deBlob: FUNCTION 00788280
	virtual TBOOL CanBeTargetedFromBelow() override { return TTRUE; } // +0xA4
	virtual void  Reset( ABaseEntity* a_pEntity ) override;           // +0xC8

public:
	// A grass bomb in a list of the level (AGameStatus's m_oList794: the ones waiting for their camera, sorted by their
	// score; m_oList78C: the active ones); 0xC bytes.
	struct BombNode
	    : public Toshi::T2GenericDList::Node
	{
		APropGrassBomb* m_pOwner; // 0x08
	};

	using BombList = Toshi::T2DList<BombNode>;

	// $deBlob: FUNCTION 005c0940
	TINT GetRequiredScore() const { return m_iRequiredScore; }

	// Activates the bomb or queues its camera (a scripted sequence needs it; AGameStatus::LevelData::AddPoints)
	void TryActivate();

private:
	// $deBlob: FUNCTION 005bc3d0
	static TINT CompareRequiredScore( const BombNode* a_pNode, const BombNode* a_pOther )
	{
		return a_pNode->m_pOwner->m_iRequiredScore - a_pOther->m_pOwner->m_iRequiredScore;
	}

	// Activates the bomb: visible, in the active list, with its idle particles
	void Activate();

	// Puts the bomb into the list of the waiting ones, sorted by the score it needs
	void AddToWaitingList();

	// Fade of the bomb's model (1.0: Activate, 0.25: the rest)
	void SetModelFade( TFLOAT a_fFade );

private:
	static Toshi::TParticleEffect* sm_pIdleEffect;   // "Blob_Grassbomb_Idle"
	static Toshi::TParticleEffect* sm_pPickupEffect; // "Blob_Pickup_GrassBomb"

private:
	TUINT8                          m_uiBombFlags;          // 0xB4 BOMBFLAG
	TINT                            m_iRequiredScore;       // 0xB8 "Required Score"
	TINT                            m_iCameraRegion;        // 0xBC "Camera Region"
	TINT                            m_iScriptedSequence;    // 0xC0 "Scripted Sequence"
	TFLOAT                          m_fTimer;               // 0xC4
	BombNode                        m_oNode;                // 0xC8
	Toshi::TParticleEffectInstance* m_pIdleParticles;       // 0xD4
	TFLOAT                          m_fStopPlayerTime;      // 0xD8 "StopPlayerTime"
	TFLOAT                          m_fStartTransitionTime; // 0xDC "StartTransitionTime"
	TFLOAT                          m_fTriggerTime;         // 0xE0 "TriggerTime"
	TFLOAT                          m_fEndTransitionTime;   // 0xE4 "EndTransitionTime"
	TFLOAT                          m_fReturnToGameTime;    // 0xE8 "ReturnToGameTime"
	TUINT8                          m_uiState;              // 0xEC
	TFLOAT                          m_fLetterboxDelay;      // 0xF0
};
