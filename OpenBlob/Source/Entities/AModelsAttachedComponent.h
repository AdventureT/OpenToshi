#pragma once
#include "Entities/AEntityComponent2.h"

class AModelInstance2;

//-----------------------------------------------------------------------------
// Purpose: models attached to bones of an entity's model: the chopper's parts, the paint tanker's tanks (ACar).
//-----------------------------------------------------------------------------
class AModelsAttachedComponent
    : public AEntityComponent2
{
public:
	static TCONSTEXPR_VAR TINT MAXMODELSATTACHED = 10;

	struct AttachedModel
	{
		AModelInstance2* pInstance; // 0x0
		TINT16           iBone;     // 0x4 the bone of the entity's model
		TBOOL            bVisible;  // 0x6
		TBOOL            bAnimated; // 0x7 the instance is ticked
	};

public:
	AModelsAttachedComponent( ABaseEntity* a_pEntity );
	virtual ~AModelsAttachedComponent() override; // +0x00

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void ResetComponent() override;            // +0x08

	// Attaches a_szModel to the bone a_szBone of the entity's model
	void AttachModel( const TCHAR* a_szModel, const TCHAR* a_szBone, TBOOL a_bVisible, TBOOL a_bAnimated, TBOOL a_bSoundBreakpoints );

	void             SetBoneModelVisible( const TCHAR* a_szBone, TBOOL a_bVisible );
	void             SetModelVisible( TINT a_iNum, TBOOL a_bVisible );
	void             DestroyModels();
	void             ShowAllModels();
	AModelInstance2* GetModelInstance( TINT a_iNum );
	void             SetVisible( TBOOL a_bVisible );

	TINT GetNumModelsAttached() const { return m_iNumModelsAttached; }

private:
	TINT          m_iNumModelsAttached;           // 0x14
	AttachedModel m_aModels[ MAXMODELSATTACHED ]; // 0x18
};
