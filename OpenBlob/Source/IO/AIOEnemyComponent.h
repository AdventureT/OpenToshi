#pragma once
#include "IO/AIOEntityComponent.h"

//-----------------------------------------------------------------------------
// Purpose: the enemy spawner of an IO entity, its component of type 1 (AIOEnemyComponent.cpp): the action 0x21 creates
// an enemy of the data's type at the entity (AEnemyManager).
//-----------------------------------------------------------------------------
class AIOEnemyComponent
    : public AIOEntityComponent
{
public:
	static TCONSTEXPR_VAR TINT TYPE = 1;

	static TCONSTEXPR_VAR TINT ACTION_Spawn = 0x21;

	// The IO data of the component (SetIOEntityData)
	struct Data
	{
		TINT iEnemyType;  // 0x00 AEnemyManager::EnemyType
		TINT iUnk04;      // 0x04 CreateEnemy's a_iUnk6
		TINT iPickupType; // 0x08
	};

	struct IOEntityData
	{
		TINT  iType; // 0x00 TYPE
		Data* pData; // 0x04
	};

public:
	AIOEnemyComponent( AIOEntity* a_pOwner );
	virtual ~AIOEnemyComponent() override; // +0x00

	// $deBlob: FUNCTION 007cc5c0
	virtual void Tick( TFLOAT a_fUnk ) override {} // +0x08

	// SetIOEntityData: a_pUnk1 is the component's IOEntityData
	virtual void SetIOEntityData( void* a_pData, Toshi::TTRB* a_pTRB ) override; // +0x0C

	// ExecuteAction: ACTION_Spawn
	virtual TBOOL ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther ) override; // +0x10

private:
	TINT m_iEnemyType;  // 0x14
	TINT m_iUnk18;      // 0x18
	TINT m_iPickupType; // 0x1C
};                      // 0x20
