#pragma once
#include "Entities/Enemies/AEnemyMelee.h"

//-----------------------------------------------------------------------------
// Purpose: the Inky soldier with the pen (Entities\AInky_Pen\AInky_Pen.cpp): an AEnemyMelee with the
// "spheremap_inky.tga" skin and the "Blob_InkySplat" particle effect (sm_pInkySplatFX, shared by all the Inkies).
// It is hit by the slam of the player's blob (HandleEntityIO, attack messages 3 and 4).
// The AI task 0x25 sprays ink: state 0x10 faces the enemy, state 0x11 sprays a cone from a bone of the model (type data
// 0xF4-0xFC) at the blobs (MESSAGE_ENEMYATTACK).
//-----------------------------------------------------------------------------
class AInky_Pen
    : public AEnemyMelee
{
public:
	TDECLARE_CLASS( AInky_Pen, AEnemyMelee );

	// The type data of the pen
	struct PenData
	    : public AEnemyMelee::MeleeData
	{
		TFLOAT m_fSprayTime;   // 0xF0 the time of a spray (state 0x11)
		TFLOAT m_fSprayLength; // 0xF4 the length of the cone (once the spray has run for 0.5 s)
		TFLOAT m_fSprayFOV;    // 0xF8 the angle of the cone
		TINT   m_iSprayBone;   // 0xFC the bone of the model the cone starts from (along -y)
	};

public:
	AInky_Pen();
	virtual ~AInky_Pen() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x0C
	virtual void Create() override;                    // +0x1C
	virtual void PostCreate() override;                // +0x20
	// $deBlob: FUNCTION 007aa180
	virtual void         Destroy() override { ABaseEntity::Destroy(); }                                       // +0x24
	virtual void         HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void         OnLeaveState( TINT a_iState ) override;                                              // +0xD4
	virtual void         OnEnterState( TINT a_iState ) override;                                              // +0xD8
	virtual const TCHAR* GetModelName() override;                                                             // +0xEC
	virtual void         StartCurrentStateTask() override;                                                    // +0x154
	virtual TBOOL        UpdateCurrentStateTask( TFLOAT a_fDeltaTime ) override;                              // +0x158
	virtual TINT         GetTargettingPriority() override;                                                    // +0xA8
	virtual TINT         GetMaxLife() override;                                                               // +0xB8
	virtual TBOOL        ShouldFlee( TINT a_iEnemy ) override;                                                // +0x1B8
	virtual TFLOAT       GetAttackedTimeOut( TINT a_iSchedule, TINT a_iTask ) override;                       // +0x1CC
	virtual TINT         GetAttackSchedule() override;                                                        // +0x1D4
	virtual TINT         GetKillScore() override;                                                             // +0x1D8
	virtual void*        GetSplatEffect() override;                                                           // +0x224

	// The helpers of the states (the addresses are the marker)
	void Die( ABaseEntity* a_pKiller );
	void StartSpray();
	void EndSpray();
	void UpdateSpray();

protected:
	static void* sm_pInkySplatFX; // "Blob_InkySplat" (also the FX of the other Inkies)

	static TBOOL sm_bSkinDone; // the sphere map of the skin is set

	TFLOAT m_fSprayTimer; // 0xEC the time of the spray (state 0x11, AI task 0x25)
};
