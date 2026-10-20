#pragma once
#include <Toshi/T2DList.h>
#include <Math/TVector4.h>

struct A2DHashRef;

//-----------------------------------------------------------------------------
// Purpose: an item stored in an A2DHash (the vtable: the destructor, GetPosition +0x04 and GetRadius +0x08).
// The item keeps the generation of the last query that visited it (+0x04), the head of its references (+0x08) and
// its own list node (+0x0C) for the lists of the owner.
//-----------------------------------------------------------------------------
class A2DHashItem
{
public:
	A2DHashItem();
	virtual ~A2DHashItem();

	virtual const Toshi::TVector4* GetPosition() = 0; // +0x04
	virtual TFLOAT                 GetRadius()   = 0; // +0x08

public:
	TUINT32                     m_uiStamp;  // 0x04 generation of the last query that visited the item
	A2DHashRef*                 m_pHashRef; // 0x08 the first reference of the item (chained through the references)
	Toshi::T2GenericDList::Node m_Node;     // 0x0C
};
