#pragma once
#include "Entities/AEntityComponent2.h"

class AEnemy;

//-----------------------------------------------------------------------------
// Purpose: the components of one AEnemy::ComponentType of every pooled enemy (Enemies\AEnemyComponentManager.cpp, one
// per type in AEnemyManager): the components in the update list are ticked by AEnemyManager::Tick (when their enemy is
// active), the others aren't.
//-----------------------------------------------------------------------------
class AEnemyComponentManager
{
public:
	// The list a component is in (AEnemy::UpdateType)
	enum UpdateType
	{
		UpdateType_Update   = 0, // ticked
		UpdateType_NoUpdate = 1,
		UpdateType_Count    = 2,
	};

	using ComponentList = Toshi::T2DList<AEntityComponent2>;

public:
	AEnemyComponentManager();
	~AEnemyComponentManager();

	// Moves a_pComponent to the update list (a_bActive) or the other one
	void Activate( AEntityComponent2* a_pComponent, TBOOL a_bActive );

	// Deletes every component
	void Destroy();

	void Tick( TFLOAT a_fDeltaTime, TINT a_eComponentType );

	// Moves the a_eComponentType component of a_pEnemy to the front of the a_eUpdateType list
	void SetUpdateList( AEnemy* a_pEnemy, UpdateType a_eUpdateType, TINT a_eComponentType );

	static void UpdateList( TFLOAT a_fDeltaTime, ComponentList& a_rList, UpdateType a_eUpdateType, TINT a_eComponentType );

private:
	ComponentList m_aLists[ UpdateType_Count ]; // 0x00
};
