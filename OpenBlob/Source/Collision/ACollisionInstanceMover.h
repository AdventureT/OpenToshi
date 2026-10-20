#pragma once
#include <Math/TMatrix44.h>
#include <Physics/TPhysicsMotionBase.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>

class ACollisionInstance2;

//-----------------------------------------------------------------------------
// Purpose: moves a body of an ACollisionInstance2 that is attached to a bone of its model (or released as a dynamic
// body): a custom mover of the physics manager.
//-----------------------------------------------------------------------------
class ACollisionInstanceMover
    : public Toshi::TPhysicsMotionBase
{
public:
	enum Flags : TUINT16
	{
		Flags_SinkAndHide           = 0x0001, // sinks the model (by its size) and hides it once it's off screen
		Flags_FallPending           = 0x0002,
		Flags_SinkAndRemove         = 0x0004, // sinks the model and removes it from the world
		Flags_NoBone                = 0x0008, // the body follows the instance's matrix only
		Flags_ReturnToStatic        = 0x0010, // a resting dynamic body goes back to static
		Flags_Unk20                 = 0x0020,
		Flags_MotionStateStatic     = 0x0040,
		Flags_MotionStateKeyframe   = 0x0080, // keyframed to its bone
		Flags_MotionStateDynamic    = 0x0100,
		Flags_MotionStateMask       = Flags_MotionStateStatic | Flags_MotionStateKeyframe | Flags_MotionStateDynamic,
		Flags_StateInvisible        = 0x0200,
		Flags_StateCollisionDisable = 0x0400,
		Flags_StateRemoved          = 0x0800,
		Flags_StateFadingOut        = 0x1000,
		Flags_StateMask             = Flags_StateInvisible | Flags_StateCollisionDisable | Flags_StateRemoved | Flags_StateFadingOut,
	};

public:
	ACollisionInstanceMover( Toshi::TPhysicsObjectContainer* a_pBody, ACollisionInstance2* a_pInstance, TINT16 a_iIndex );
	// Without a body or an instance (SetBodyAndInstance sets them)
	ACollisionInstanceMover( TINT16 a_iIndex );

	// Sets the body and the instance and copies the instance's matrix
	void SetBodyAndInstance( Toshi::TPhysicsObjectContainer* a_pBody, ACollisionInstance2* a_pInstance );

	// $deBlob: FUNCTION 005a7f30
	// $deBlob: FUNCTION 005a7f10
	virtual ~ACollisionInstanceMover() override = default;

	// $deBlob: FUNCTION 005a7f90
	virtual void OnRemoved() override {}
	virtual void Update( TFLOAT a_fTimeStep, Toshi::TPhysicsManager* a_pManager ) override;
	// $deBlob: FUNCTION 005a8050
	virtual void Unk0C( void* a_pUnk1, void* a_pUnk2 ) override {}
	// $deBlob: FUNCTION 005a8040
	virtual void Unk10( void* a_pUnk1, void* a_pUnk2 ) override {}
	// $deBlob: FUNCTION 005a8090
	virtual void Unk14( void* a_pUnk1, void* a_pUnk2 ) override {}
	// $deBlob: FUNCTION 005a8080
	virtual void Unk18( void* a_pUnk1, void* a_pUnk2 ) override {}
	// $deBlob: FUNCTION 005a8030
	virtual void Unk1C( void* a_pUnk1, void* a_pUnk2 ) override {}
	// $deBlob: FUNCTION 005a8020
	virtual void Unk20( void* a_pUnk1, void* a_pUnk2 ) override {}
	// $deBlob: FUNCTION 005a8070
	virtual void Unk24( void* a_pUnk1, void* a_pUnk2 ) override {}
	// $deBlob: FUNCTION 005a8060
	virtual void Unk28( void* a_pUnk1, void* a_pUnk2 ) override {}
	// $deBlob: FUNCTION 005a8010
	virtual void Unk2C( void* a_pUnk1, void* a_pUnk2 ) override {}
	// $deBlob: FUNCTION 005a7fa0
	virtual void Unk30() override {}
	virtual void GetLinearVelocity( Toshi::TPhysicsManager* a_pManager, Toshi::TVector4& a_rvOutVelocity ) override;

	// The player hit the body (ACollisionInstanceResponderMover); a static body turns dynamic.
	virtual void OnHit(); // +0x38

	// Sets the motion state (one of Flags_MotionState*)
	void SetMotionState( TUINT16 a_uiFlag );

	// Sets the state (one of Flags_State*)
	void SetState( TUINT16 a_uiFlag );

	// Sets or clears Flags_SinkAndHide with its parameter (m_fHideDelay)
	void SetSinkAndHide( TBOOL a_bSet, TFLOAT a_fParam );

	// Sets or clears Flags_SinkAndRemove with its parameter (m_fRemoveDelay)
	void SetSinkAndRemove( TBOOL a_bSet, TFLOAT a_fParam );

	// Stores the inverse of a_rMatrix in m_mInstanceInverse (the instance's matrix is inverted once)
	void SetInverseMatrix( const Toshi::TMatrix44& a_rMatrix );

	// Snaps the keyframed body to its bone at once (ACollisionInstance2::UpdateBodyTransforms)
	void SnapToBone();

	// Whether the model's bounding sphere is in the view of the current camera
	TBOOL IsOnScreen();

	// A dynamic body goes back to the instance's matrix, static and visible (the level restart, ATerrain2Interface);
	// NOTE: unlike Update's return to static, retail doesn't make the Havok body static here
	void ResetToInstance();

	TUINT16           GetFlags() const { return m_uiFlags; }
	void              AddFlags( TUINT16 a_uiFlags ) { m_uiFlags |= a_uiFlags; }
	Toshi::TMatrix44& GetInstanceMatrix() { return m_mInstance; }

private:
	friend class ACollisionInstance2;

	TINT                            m_iUnk08;            // 0x08 -1
	Toshi::TPhysicsObjectContainer* m_pBody;             // 0x0c
	ACollisionInstance2*            m_pInstance;         // 0x10
	Toshi::TMatrix44                m_mInstance;         // 0x14 the instance's matrix (ACollisionInstance2::SetMatrix)
	Toshi::TMatrix44                m_mInstanceInverse;  // 0x54 inverted by Update
	TFLOAT                          m_fTime;             // 0x94
	TFLOAT                          m_fDynamicTime;      // 0x98
	TFLOAT                          m_fHideDelay;        // 0x9c
	TFLOAT                          m_fFallDelay;        // 0xa0
	TFLOAT                          m_fRemoveDelay;      // 0xa4
	TINT16                          m_iIndex;            // 0xa8 the collision mesh of the model
	TUINT16                         m_uiFlags;           // 0xaa Flags
	TUINT32                         m_uiCollisionFilter; // 0xac the body's group (low 12 bits) and layer (bits 16-27)
};
