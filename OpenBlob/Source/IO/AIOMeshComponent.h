#pragma once
#include "IO/AIOEntityComponent.h"

#include <Math/TMatrix44.h>
#include <Math/TVector4.h>

class AEntityModelComponent;

//-----------------------------------------------------------------------------
// Purpose: the model of an IO entity, its component of type 4 (AIOMeshComponent.cpp): an AEntityModelComponent created
// from the IO data's model name, placed at the data's position, rotation and scale relative to the entity.
//-----------------------------------------------------------------------------
class AIOMeshComponent
    : public AIOEntityComponent
{
public:
	static TCONSTEXPR_VAR TINT TYPE = 4;

	// The IO data of the component (SetIOEntityData)
	struct Data
	{
		const TCHAR*           szModelName; // 0x00 without the extension, TNULL for none
		const Toshi::TVector4* pPosition;   // 0x04 Z up
		const Toshi::TVector4* pRotation;   // 0x08 a quaternion
		const Toshi::TVector4* pScale;      // 0x0C Z up
	};

	struct IOEntityData
	{
		TINT  iType; // 0x00 TYPE
		Data* pData; // 0x04
	};

public:
	AIOMeshComponent( AIOEntity* a_pOwner );
	virtual ~AIOMeshComponent() override; // +0x00

	// Tick: shows the model with the entity (while the entity's IO flag 0x100 is set) and ticks it
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x08

	// SetIOEntityData: a_pUnk1 is the component's IOEntityData
	virtual void SetIOEntityData( void* a_pData, Toshi::TTRB* a_pTRB ) override; // +0x0C

	// ExecuteAction: the action 0x19 shows (its params' +4 == 1) or hides the model
	virtual TBOOL ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther ) override; // +0x10

	void UpdateWorldTransform( const Toshi::TMatrix44& a_rEntityMatrix );
	void GetMatrix( Toshi::TMatrix44& a_rOutMatrix );
	void SetVisible( TBOOL a_bVisible );

	AEntityModelComponent* GetModelComponent() const { return m_pModelComponent; }

private:
	const TCHAR*           m_szModelName;     // 0x14
	const Toshi::TVector4* m_pPosition;       // 0x18
	const Toshi::TVector4* m_pRotation;       // 0x1C
	const Toshi::TVector4* m_pScale;          // 0x20
	Toshi::TMatrix44       m_oLocalMatrix;    // 0x24
	AEntityModelComponent* m_pModelComponent; // 0x64
};                                            // 0x68
