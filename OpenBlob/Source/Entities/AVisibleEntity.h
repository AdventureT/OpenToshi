#pragma once
#include "Entities/ABaseEntity.h"
#include "Entities/AEntityFactory.h"
#include "Entities/AEntityParams.h"

class AEntityComponent2;
class AShadow;
class AEntityModelComponent;
class AEntityPoseController;
class AEntityStateComponent;

namespace Toshi
{
class TPhysicsObjectContainer;
class TPhysicsMotionBase;
class TPhysicsResponder;
class TShapeBaseClass;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: the physics body of a visible entity (AVisibleEntity::m_pPhysicsData, freed with TFree, no destructor).
//-----------------------------------------------------------------------------
struct AEntityPhysicsData
{
	Toshi::TMatrix44                m_oMatrix;                // 0x00 the body's matrix (offset by m_oOffset)
	Toshi::TMatrix44                m_oOffset;                // 0x40 the shape's offset from the entity
	TINT                            m_eOwnerType;             // 0x80 the body's user data (ACollisionInstance2::OwnerInfo { 2, entity })
	void*                           m_pOwner;                 // 0x84
	Toshi::TPhysicsObjectContainer* m_pBody;                  // 0x88
	Toshi::TPhysicsResponder*       m_pCollisionEventHandler; // 0x8C deleted through its virtual destructor
	Toshi::TPhysicsMotionBase*      m_pPhysicsMover;          // 0x90 a custom mover of TPhysicsManager (AEntityPhysicsMover)
};

//-----------------------------------------------------------------------------
// Purpose: an entity with a model (abstract: slot +0xEC is pure): owns the model, pose, state and FX breakpoint
// components.
//-----------------------------------------------------------------------------
class AVisibleEntity
    : public ABaseEntity
{
public:
	TDECLARE_CLASS( AVisibleEntity, ABaseEntity );

	friend class AEntityPhysicsMover;

	static TCONSTEXPR_VAR TUINT32 FLAG_NoComponentDelete = 0x80000;   // the dtor doesn't delete the components
	static TCONSTEXPR_VAR TUINT32 FLAG_VisibleDirty      = 0x8000000; // SetVisible before the model existed
	static TCONSTEXPR_VAR TUINT32 FLAG_ModelAlwaysLOD    = 0x4000000; // the model is shown at every LOD (else only at FLAG_HighLOD)

public:
	AVisibleEntity();
	virtual ~AVisibleEntity() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                     // +0x0C
	virtual void OnLODChanging( TINT a_iLOD ) override;                    // +0x10
	virtual void SyncBodyTransform() override;                             // +0x14
	virtual void Create() override;                                        // +0x1C
	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C
	virtual void SetVisible( TBOOL a_bVisible ) override;                  // +0x38
	// $deBlob: FUNCTION 007614d0
	virtual void* GetModelComponent() override { return m_pModelComponent; } // +0x4C
	// $deBlob: FUNCTION 007614f0
	virtual void* GetPoseController() override { return m_pPoseController; } // +0x50
	// $deBlob: FUNCTION 00761510
	virtual void* GetStateComponent() override { return m_pStateComponent; }                           // +0x58
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void  PreRenderUpdate() override;                                                          // +0x6C
	virtual void  AddToPhysics() override;                                                             // +0x88
	virtual void  RemoveFromPhysics() override;                                                        // +0x8C
	virtual void  SetPhysicsEnabled( TBOOL a_bEnable ) override;                                       // +0x90
	virtual void  Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	// $deBlob: FUNCTION 007614c0
	virtual void* GetFXBreakpointComponent() { return m_pFXBreakpointComponent; } // +0xDC
	// $deBlob: FUNCTION 00761440
	virtual void* GetHitEffect() { return TNULL; } // +0xE0
	// Deactivates the entity (hidden, not ticking, AEntityFactory::DeactivateEntity)
	virtual void Despawn( TBOOL a_bPlayEffect ); // +0xE4
	// $deBlob: FUNCTION 007614a0
	virtual void* GetDespawnEffect() { return TNULL; } // +0xE8
	// The model name (also passed to the FX breakpoint component)
	virtual const TCHAR* GetModelName() = 0; // +0xEC

	// $deBlob: FUNCTION 007614e0
	AEntityPhysicsData* GetPhysicsData() const { return m_pPhysicsData; }

protected:
	void CreatePoseController();
	void CreateModelComponent( const TCHAR* a_szModelName );
	void CreateStateComponent();
	void CreateFXBreakpointComponent( const TCHAR* a_szName );
	// Creates m_pPhysicsData (or uses a_pPhysicsData) and its body from the type's physics data and a_iNumObjects
	// shapes (a_pMatrices: one offset each); a_bFixed: a fixed body (TPhysicsManager::CreateFixedBody)
	void CreatePhysics( const AEntityTypeData::TRBData* a_pTRBData, Toshi::TShapeBaseClass** a_ppShapes, const Toshi::TMatrix44* a_pMatrices, TINT a_iNumObjects, AEntityPhysicsData* a_pPhysicsData, TBOOL a_bFixed );
	// Creates the physics (CreatePhysics above) from a collision object of the params (AEntityParams::GetCollision)
	// with the type's physics data, its groups and flags replaced by the collision object's
	void CreatePhysics( const AEntityTypeData::TRBData* a_pTRBData, const AEntityParam::Collision* a_pCollision, AEntityPhysicsData* a_pPhysicsData, TBOOL a_bFixed );
	void DestroyPhysicsData();
	void SetPhysicsMover( Toshi::TPhysicsMotionBase* a_pPhysicsMover );
	void SetCollisionEventHandler( Toshi::TPhysicsResponder* a_pHandler );
	// Creates the shadow (an AShadow of type 0)
	void CreateShadow( TFLOAT a_fSize, TFLOAT a_fAlpha );

	void UpdateModelVisibility( TBOOL a_bVisible );

protected:
	AEntityModelComponent* m_pModelComponent;        // 0x80
	AShadow*               m_pShadow;                // 0x84 (CreateShadow)
	TFLOAT                 m_fAmbientScale;          // 0x88 "Ambient Scale"
	AEntityPoseController* m_pPoseController;        // 0x8C
	AEntityStateComponent* m_pStateComponent;        // 0x90
	AEntityComponent2*     m_pFXBreakpointComponent; // 0x94 AFXBreakpointComponent
	AEntityPhysicsData*    m_pPhysicsData;           // 0x98
};
