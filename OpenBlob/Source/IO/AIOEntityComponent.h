#pragma once
#include "Entities/AEntityIOPacket.h"

namespace Toshi
{
class TTRB;
} // namespace Toshi

class AIOEntity;
class ABaseEntity;

//-----------------------------------------------------------------------------
// Purpose: the base of the IO components an AIOEntity owns. It is its own class, it does not derive from
// AEntityComponent2.
//-----------------------------------------------------------------------------
class AIOEntityComponent
{
public:
	AIOEntityComponent( AIOEntity* a_pOwner, TINT a_iUnk );
	virtual ~AIOEntityComponent(); // +0x00

	// DOES_NOTHING (shared stub, 0 args)
	// $deBlob: FUNCTION 007cc470
	virtual void Unk04() {} // +0x04

	// DOES_NOTHING (shared stub, 1 arg)
	// $deBlob: FUNCTION 007cc4a0
	virtual void Tick( TFLOAT a_fDeltaTime ) {} // +0x08

	// DOES_NOTHING (shared stub, 2 args)
	// $deBlob: FUNCTION 007e55f0
	virtual void SetIOEntityData( void* a_pData, Toshi::TTRB* a_pTRB ) {} // +0x0C

	// Returns false, the base does not handle the action. The first argument is the message pointer
	// (AIOEntity::HandleInputMessage pushes it)
	virtual TBOOL ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther ); // +0x10

	// DOES_NOTHING (shared stub, 2 args)
	// $deBlob: FUNCTION 007cc450
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) {} // +0x14

	AIOEntity* GetOwner() const { return m_pOwner; }

	// The components of an AIOEntity form a list (AIOEntity+0xFC) through this pointer; the type is +0x04
	AIOEntityComponent* GetNext() const { return m_pNext; }
	void                SetNext( AIOEntityComponent* a_pNext ) { m_pNext = a_pNext; }
	TINT                GetType() const { return m_iListType; }
	TUINT32             GetFlags() const { return m_uiFlags; }

protected:
	TINT                m_iListType; // 0x04 the second ctor argument, the list type (4 is the one OnLODChanging drives)
	AIOEntity*          m_pOwner;    // 0x08 the first ctor argument
	AIOEntityComponent* m_pNext;     // 0x0C next component in the owner's list
	TUINT32             m_uiFlags;   // 0x10 bit 0x1 is set by the subclasses (ASoundComponent ctor)
}; // 0x14
