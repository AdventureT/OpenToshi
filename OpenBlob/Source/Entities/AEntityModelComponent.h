#pragma once
#include "Entities/AEntityComponent2.h"
#include <Math/TMatrix44.h>

class AModelInstance2;

//-----------------------------------------------------------------------------
// Purpose: an entity's model: an AModelInstance2 created by AModelFactory from the entity's mesh name, ticked and moved
// with the entity (AVisibleEntity::m_pModelComponent).
//-----------------------------------------------------------------------------
class AEntityModelComponent
    : public AEntityComponent2
{
public:
	AEntityModelComponent( ABaseEntity* a_pEntity, const TCHAR* a_szModelName );
	virtual ~AEntityModelComponent() override; // +0x00

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	void SetTransform( const Toshi::TMatrix44& a_rMatrix );

	AModelInstance2* GetModelInstance() const { return m_pModelInstance; }

private:
	AModelInstance2* m_pModelInstance; // 0x14
	TBOOL            m_bModelLoaded;   // 0x18 the model was loaded when the instance was created
};                                     // 0x1C
