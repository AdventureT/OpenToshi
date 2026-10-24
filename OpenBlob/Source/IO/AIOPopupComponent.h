#pragma once
#include "IO/AIOEntityComponent.h"

//-----------------------------------------------------------------------------
// Purpose: a popup of an IO entity, its component of type 2 (AIOPopupComponent.cpp): it keeps the data's value and only
// accepts the action 0x28 (it shows nothing itself).
//-----------------------------------------------------------------------------
class AIOPopupComponent
    : public AIOEntityComponent
{
public:
	static TCONSTEXPR_VAR TINT TYPE = 2;

	static TCONSTEXPR_VAR TINT ACTION_Popup = 0x28;

	struct IOEntityData
	{
		TINT  iType; // 0x00 TYPE
		TINT* pData; // 0x04
	};

public:
	AIOPopupComponent( AIOEntity* a_pOwner );
	virtual ~AIOPopupComponent() override; // +0x00

	// $deBlob: FUNCTION 007cc6a0
	virtual void Tick( TFLOAT a_fUnk ) override {} // +0x08

	// SetIOEntityData: a_pUnk1 is the component's IOEntityData
	virtual void SetIOEntityData( void* a_pData, Toshi::TTRB* a_pTRB ) override; // +0x0C

	// ExecuteAction: accepts ACTION_Popup (does nothing else)
	virtual TBOOL ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther ) override; // +0x10

private:
	TINT m_iValue; // 0x14 the data's value
}; // 0x18
