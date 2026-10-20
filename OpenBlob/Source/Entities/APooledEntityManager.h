#pragma once
#include "APooledEntity.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>

class ABaseEntity;

//-----------------------------------------------------------------------------
// Purpose: the pools of the pooled entities (APooledEntityManager.cpp): per pool type, a free list (the entities are
// removed from AEntityFactory and hidden), a used list (the entities handed out), the pool array and the number of
// entities it holds. The pool types: 0 ACiv, 1-3 APropTriggerTimeBonus, 4 APickupLife.
//-----------------------------------------------------------------------------
class APooledEntityManager
    : public Toshi::TSingleton<APooledEntityManager>
{
public:
	static TCONSTEXPR_VAR TINT NUM_POOL_TYPES = 5;

public:
	APooledEntityManager();
	~APooledEntityManager();

	// Raises the number of entities of the type to at least a_iCount; returns the new number
	TINT EnsurePoolCount( TINT a_eType, TINT a_iCount );
	// Adds a_iCount entities to the number of the type; returns a_iCount
	TINT AddPoolCount( TINT a_eType, TINT a_iCount );

	// Creates the pools of every type that has entities to hold
	void CreatePools();
	// Creates the pool of one type: a_iCount entities, all in the free list
	void CreatePool( TINT a_eType, TINT a_iCount );

	// Puts the free entities into the state of a pooled entity (PostCreate, hidden, inactive)
	void DeactivateFreeEntities();
	// Deletes the free entities and the pools (the used list is only cleared)
	void DestroyPools();

	// Takes the last free entity of the type, moves it to the used list, activates it at a_rPosition facing a_rFacing
	// (a_iParam is its +0xC0 value) and returns it; TNULL if the free list is empty
	APooledEntity* Acquire( TINT a_eType, const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rFacing, TINT a_iParam );

	// Gives a pooled entity back to its free list
	void Release( APooledEntity* a_pPooled );

	// TRUE when the free list of the type has an entity (the first dword of the manager is the list of type 0)
	TBOOL HasFree( TINT a_eType ) const { return !m_FreeList[ a_eType ].IsEmpty(); }
	// Gives every used pooled entity back to its free list
	void ReleaseAll();

	// The used entry of the type whose entity is a_pEntity (TNULL if none)
	APooledEntity* FindInUse( ABaseEntity* a_pEntity, TINT a_eType );

private:
	// The class name of a pool type, TNULL for an unknown type
	static const TCHAR* GetEntityName( TINT a_eType );

private:
	Toshi::T2DList<APooledEntity> m_FreeList[ NUM_POOL_TYPES ];             // 0x00
	Toshi::T2DList<APooledEntity> m_UsedList[ NUM_POOL_TYPES ];             // 0x28
	APooledEntity*                m_EntityPoolAllocArray[ NUM_POOL_TYPES ]; // 0x50
	TINT                          m_aiEntityPoolCount[ NUM_POOL_TYPES ];    // 0x64
};
