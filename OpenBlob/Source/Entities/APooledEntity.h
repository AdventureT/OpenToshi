#pragma once
#include <Toshi/T2DList.h>

class ABaseEntity;

//-----------------------------------------------------------------------------
// Purpose: an entry of the free and used lists of APooledEntityManager (0x791600 is its deleting wrapper): the pooled
// entity (0x0C), its pool type (0x10, -1 until the pool is created) and the list node (0x04). Pool entries are
// allocated per type as one array.
//-----------------------------------------------------------------------------
class APooledEntity
    : public Toshi::T2GenericDList::Node
{
public:
	APooledEntity();
	virtual ~APooledEntity();

public:
	ABaseEntity* m_pEntity; // 0x0C
	TINT         m_eType;   // 0x10
};
