#pragma once
#include "Entities/AVisibleEntity.h"

class ACollisionObject;
class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: a small creature that is squashed when Blob rolls over it: a model ("Mesh") coloured with a random enemy
// colour and a collision object.
//-----------------------------------------------------------------------------
class ACritter
    : public AVisibleEntity
{
public:
	TDECLARE_CLASS( ACritter, AVisibleEntity );

	enum CRITTERFLAG : TUINT8
	{
		CRITTERFLAG_TypeMask       = 0x3,
		CRITTERFLAG_StartInvisible = 0x4,
	};

	static TCONSTEXPR_VAR TINT Max = 3;

public:
	ACritter();
	virtual ~ACritter() override; // +0x08

	virtual void              Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void              PostCreate() override;                                                               // +0x20
	virtual void              Destroy() override;                                                                  // +0x24
	virtual void              ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void              HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual ACollisionObject* GetCollisionObject() override;                                                       // +0x70
	virtual void              Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual const TCHAR*      GetModelName() override;                                                             // +0xEC

	static void SquashCritters( ABlobEntity* a_pBlob );

private:
	// $deBlob: FUNCTION 007ac040
	TINT GetType() const { return m_uiCritterFlags & CRITTERFLAG_TypeMask; }

	void CreateCollision();
	void SetupModel();

private:
	static const TINT   ms_aNormalStates[ Max ];
	static const TINT   ms_aSquashedStates[ Max ];
	static const TFLOAT ms_fScale;

private:
	ACollisionObject* m_pCollision;     // 0x9C
	TINT8             m_uiCritterFlags; // 0xA0 CRITTERFLAG
};
