#pragma once
#include <Physics/TPhysicsMotionBase.h>
#include <Math/TMatrix44.h>

class AVisibleEntity;

//-----------------------------------------------------------------------------
// Purpose: the custom physics mover of a visible entity (AEntityPhysicsData::m_pPhysicsMover): before each physics step
// it keyframes the entity's body to the entity's matrix (or to a bone of its model), unless the body is fixed or
// keyframed by the simulation itself.
//-----------------------------------------------------------------------------
class AEntityPhysicsMover
    : public Toshi::TPhysicsMotionBase
{
public:
	AEntityPhysicsMover( AVisibleEntity* a_pEntity );
	// $deBlob: FUNCTION 007c6740
	// $deBlob: FUNCTION 007c6730
	virtual ~AEntityPhysicsMover() override = default;

	virtual void OnRemoved() override;                                                                               // +0x04
	virtual void Update( TFLOAT a_fTimeStep, Toshi::TPhysicsManager* a_pManager ) override;                          // +0x08
	virtual void Unk0C( void* a_pUnk1, void* a_pUnk2 ) override;                                                     // +0x0c
	virtual void Unk10( void* a_pUnk1, void* a_pUnk2 ) override;                                                     // +0x10
	virtual void Unk14( void* a_pUnk1, void* a_pUnk2 ) override;                                                     // +0x14
	virtual void Unk18( void* a_pUnk1, void* a_pUnk2 ) override;                                                     // +0x18
	virtual void Unk1C( void* a_pUnk1, void* a_pUnk2 ) override;                                                     // +0x1c
	virtual void Unk20( void* a_pUnk1, void* a_pUnk2 ) override;                                                     // +0x20
	virtual void Unk24( void* a_pUnk1, void* a_pUnk2 ) override;                                                     // +0x24
	virtual void Unk28( void* a_pUnk1, void* a_pUnk2 ) override;                                                     // +0x28
	virtual void Unk2C( void* a_pUnk1, void* a_pUnk2 ) override;                                                     // +0x2c
	virtual void Unk30() override;                                                                                   // +0x30
	virtual void GetLinearVelocity( Toshi::TPhysicsManager* a_pManager, Toshi::TVector4& a_rvOutVelocity ) override; // +0x34

	// $deBlob: FUNCTION 007c66a0
	void SetBone( TINT a_iBone ) { m_iBone = a_iBone; }

	// Moves the entity's body to a_rcMatrix (offset by the body's m_oOffset) and stops it unless it's fixed (motion
	// type 0)
	void SetBodyTransform( const Toshi::TMatrix44& a_rcMatrix );

private:
	TINT            m_iUnk08;  // 0x08 -1
	AVisibleEntity* m_pEntity; // 0x0C
	TINT            m_iBone;   // 0x10 -1: the entity's matrix, else this bone of its model
};
