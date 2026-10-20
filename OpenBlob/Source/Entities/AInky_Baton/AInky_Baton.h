#pragma once
#include "Entities/Enemies/AEnemyMelee.h"

//-----------------------------------------------------------------------------
// Purpose: the Inky soldier with the baton (Entities\AInky_Baton\AInky_Baton.cpp): an AEnemyMelee with the "ss_inky" /
// "ss_elite_inky" skins, the "Blob_InkySplat" effect (sm_pInkySplatFX) and the "Blob_InkyBaton_Strike" effect
// (sm_pStrikeFX), both shared by all the batons. m_uiType (0xEC) is the baton type (ABlobColourComponent colour, white
// = 0), read by AInky_Baton_Elite. SetCustomData also sets the byte at 0x63 (high byte of m_iCollisionGroupAndType).
// A slam of a blob costs the blob the baton's health in paint points (HandleEntityIO, a coloured baton only takes its
// colour or the rainbow); it hits back with AEnemyMelee::DoAttack (CheckMeleeCollision, MESSAGE_ENEMYATTACK).
//-----------------------------------------------------------------------------
class AInky_Baton
    : public AEnemyMelee
{
public:
	TDECLARE_CLASS( AInky_Baton, AEnemyMelee );

public:
	AInky_Baton();
	virtual ~AInky_Baton() override; // +0x08

	// The baton type (the colour, ABlobZJumpComponent's lock element)
	// $deBlob: FUNCTION 0076a840
	TUINT8 GetBatonType() const { return m_uiType; }

	virtual void  Create() override;                                                                   // +0x1C
	virtual void  PostCreate() override;                                                               // +0x20
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual TINT  GetTargetColour() override;                                                          // +0xB4
	virtual TINT  GetMaxLife() override;                                                               // +0xB8
	virtual void* GetHitEffect() override;                                                             // +0xE0
	virtual void* GetDespawnEffect() override;                                                         // +0xE8
	virtual void  SetCustomData( TINT a_iCustomData ) override;                                        // +0x1A0
	virtual TBOOL ShouldFlee( TINT a_iEnemy ) override;                                                // +0x1B8
	virtual TINT  GetKillScore() override;                                                             // +0x1D8
	virtual void* GetSplatEffect() override;                                                           // +0x224
	virtual void  PlayDeathSound();                                                                    // +0x22C (new slot, after DoAttack +0x228)

	// The helper of the attack (the address is the marker): the decal, PlayDeathSound and OnDeath( 6, pKiller)
	void Die( ABaseEntity* a_pKiller );

protected:
	static void*  sm_pInkySplatFX; // "Blob_InkySplat"
	static void*  sm_pStrikeFX;    // "Blob_InkyBaton_Strike"
	static TUINT8 sm_uiSkinFlags;  // skins done: 1 ss_inky, 2 ss_elite_inky, 3 all

	TUINT8 m_uiType; // 0xEC the baton type (ABlobColourComponent colour)
};
