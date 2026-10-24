#pragma once

TOSHI_NAMESPACE_START

class TPhysicsManager;
class TVector4;

// A custom mover of TPhysicsManager (m_pCustomMovers): updated before each step of its world
// NOTE: only the slots TPhysicsManager calls are known; the others keep the slot count with unknown signatures
class TPhysicsMotionBase
{
public:
	TPhysicsMotionBase()
	    : m_iWorld( 0 )
	{}

	// $deBlob: FUNCTION 005a7f60
	// $deBlob: FUNCTION 005a7f20
	virtual ~TPhysicsMotionBase() = default;

	virtual void OnRemoved()                                                                 = 0; // +0x04 (TPhysicsManager::Unitialise)
	virtual void Update( TFLOAT a_fTimeStep, TPhysicsManager* a_pManager )                   = 0; // +0x08
	virtual void Unk0C( void* a_pUnk1, void* a_pUnk2 )                                       = 0; // +0x0c
	virtual void Unk10( void* a_pUnk1, void* a_pUnk2 )                                       = 0; // +0x10
	virtual void Unk14( void* a_pUnk1, void* a_pUnk2 )                                       = 0; // +0x14
	virtual void Unk18( void* a_pUnk1, void* a_pUnk2 )                                       = 0; // +0x18
	virtual void Unk1C( void* a_pUnk1, void* a_pUnk2 )                                       = 0; // +0x1c
	virtual void Unk20( void* a_pUnk1, void* a_pUnk2 )                                       = 0; // +0x20
	virtual void Unk24( void* a_pUnk1, void* a_pUnk2 )                                       = 0; // +0x24
	virtual void Unk28( void* a_pUnk1, void* a_pUnk2 )                                       = 0; // +0x28
	virtual void Unk2C( void* a_pUnk1, void* a_pUnk2 )                                       = 0; // +0x2c
	virtual void Unk30()                                                                     = 0; // +0x30
	virtual void GetLinearVelocity( TPhysicsManager* a_pManager, TVector4& a_rvOutVelocity ) = 0; // +0x34

public:
	TINT m_iWorld; // 0x4 (set by TPhysicsManager::AddCustomMover)
};

TOSHI_NAMESPACE_END
