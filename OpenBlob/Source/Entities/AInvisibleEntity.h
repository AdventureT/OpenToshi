#pragma once
#include "Entities/ABaseEntity.h"

//-----------------------------------------------------------------------------
// Purpose: an entity without a model: only a collision object (ACollisionObject, owner type 1) that follows its matrix
// and bounding box.
//-----------------------------------------------------------------------------
class AInvisibleEntity
    : public ABaseEntity
{
public:
	TDECLARE_CLASS( AInvisibleEntity, ABaseEntity );

public:
	AInvisibleEntity();
	virtual ~AInvisibleEntity() override; // +0x08

	virtual void SetPosition( const Toshi::TVector4& a_rPosition ) override;                          // +0x40
	virtual void SetMatrix( const Toshi::TMatrix44& a_rMatrix ) override;                             // +0x48
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	// $deBlob: FUNCTION 00762720
	virtual ACollisionObject* GetCollisionObject() override { return m_pCollisionObject; } // +0x70
	virtual void              SetBBox( Toshi::TVector4 a_vBBox ) override;                 // +0x7C
	virtual void              Reset( ABaseEntity* a_pEntity ) override;                    // +0xC8
	// Deactivates the entity (hidden, not ticking, AEntityFactory::DeactivateEntity)
	virtual void Despawn(); // +0xDC

protected:
	ACollisionObject* m_pCollisionObject; // 0x80
};
