#pragma once
#include "AEnemy.h"

//-----------------------------------------------------------------------------
// Purpose: the melee enemies (Entities\Enemies\AEnemyMelee.cpp): the base of the Inky soldiers (AInky_Pen, AInky_Baton,
// AInky_Baton_Elite). Adds the attack state (+0x228 DoAttack), the attack state entry (AEnemy state 0x0E) and the
// recover handling (task 0x0A goes to state 0x0D).
//-----------------------------------------------------------------------------
class AEnemyMelee
    : public AEnemy
{
public:
	TDECLARE_CLASS( AEnemyMelee, AEnemy );

	// The type data of a melee enemy
	struct MeleeData
	    : public AEnemy::EnemyData
	{
		TINT   m_iAttackDamage; // 0xDC AAttackMessage::m_uiDamage of the hit
		TINT   m_iAttackUnk18;  // 0xE0 AAttackMessage::m_uiUnk18 of the hit
		TFLOAT m_fAttackMinDot; // 0xE4 CheckMeleeCollision's a_fMinDot
		TFLOAT m_fLungeSpeed;   // 0xE8 the speed forward while attacking (0: none)
		TFLOAT m_fLungeTime;    // 0xEC
	};

public:
	AEnemyMelee();
	virtual ~AEnemyMelee() override; // +0x08

	virtual void  OnEnterState( TINT a_iState ) override;                 // +0xD8
	virtual void  StartCurrentStateTask() override;                       // +0x154
	virtual TBOOL UpdateCurrentStateTask( TFLOAT a_fDeltaTime ) override; // +0x158

	// The attack of the enemy (a_iUnk: the attack type, 0 from OnEnterState)
	virtual void DoAttack( TINT a_iAttack ); // +0x228
};
