#pragma once
#include "APathfindingComponent.h"

#include <Math/TVector4.h>

class ABaseEntity;
class AConsoleVar;

//-----------------------------------------------------------------------------
// Purpose: the parabola of an AI character's jump (PathFinding\AAIJumpTrajectory.cpp, no vtable): position( t) =
// m_vStart + m_vVelocity * t + m_vAcceleration * t * t, optionally with a second jump (double jump) or a horizontal
// move that starts or stops part way. AAIMoveProbe checks jumps with it, AGndMotorComponent follows one
// (m_JumpParabola).
//-----------------------------------------------------------------------------
class AAIJumpTrajectory
{
public:
	enum JumpType : TINT
	{
		JumpType_Auto         = 0,  // CheckValidJump picks JumpType_MoveThenJump or JumpType_JumpThenMove
		JumpType_MoveThenStop = 1,  // the horizontal move stops at m_fHorizontalTime
		JumpType_MoveThenJump = 2,  // the horizontal move lasts the whole jump
		JumpType_JumpThenMove = 3,  // the horizontal move (m_vHorizontalVelocity) starts at m_fHorizontalTime
		JumpType_None         = -1, // NOTE: set by CheckValidJump before the type is chosen
	};

	// The jump that is followed (0x54 bytes)
	struct Trajectory
	{
		// $deBlob: FUNCTION 007d9620
		Trajectory()
		{
			m_vAcceleration       = Toshi::TVector4::VEC_ZERO;
			m_vVelocity           = Toshi::TVector4::VEC_ZERO;
			m_vStart              = Toshi::TVector4::VEC_ZERO;
			m_fTotalTime          = 0.0f;
			m_fSecondJumpSpeed    = 0.0f;
			m_fSecondJumpTime     = -1.0f;
			m_fHorizontalTime     = -1.0f;
			m_vHorizontalVelocity = Toshi::TVector4::VEC_ZERO;
			m_eJumpType           = JumpType_Auto;
		}

		// $deBlob: FUNCTION 007d95d0
		Trajectory& operator=( const Trajectory& a_rcOther )
		{
			m_vAcceleration       = a_rcOther.m_vAcceleration;
			m_vVelocity           = a_rcOther.m_vVelocity;
			m_vStart              = a_rcOther.m_vStart;
			m_fTotalTime          = a_rcOther.m_fTotalTime;
			m_fSecondJumpSpeed    = a_rcOther.m_fSecondJumpSpeed;
			m_fSecondJumpTime     = a_rcOther.m_fSecondJumpTime;
			m_fHorizontalTime     = a_rcOther.m_fHorizontalTime;
			m_vHorizontalVelocity = a_rcOther.m_vHorizontalVelocity;
			m_eJumpType           = a_rcOther.m_eJumpType;
			return *this;
		}

		Toshi::TVector4 m_vAcceleration;       // 0x00
		Toshi::TVector4 m_vVelocity;           // 0x10
		Toshi::TVector4 m_vStart;              // 0x20
		TFLOAT          m_fTotalTime;          // 0x30
		TFLOAT          m_fSecondJumpSpeed;    // 0x34
		TFLOAT          m_fSecondJumpTime;     // 0x38 -1: no second jump
		TFLOAT          m_fHorizontalTime;     // 0x3C -1: none
		Toshi::TVector4 m_vHorizontalVelocity; // 0x40
		JumpType        m_eJumpType;           // 0x50
	};

public:
	AAIJumpTrajectory();
	AAIJumpTrajectory( const Toshi::TVector4& a_rvA, const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvEnd, TFLOAT a_fHeight );
	~AAIJumpTrajectory();

	static void Init();
	static void Destroy();

	void Create( const Toshi::TVector4& a_rvA, const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvEnd, TFLOAT a_fHeight );

	TBOOL CheckValidJump( ABaseEntity* a_pOwner, Toshi::TVector4 a_vStart, Toshi::TVector4 a_vGoal, TUINT32 a_uiFlags, APathfindingComponent::Obstruction* a_pObstruction, JumpType a_eJumpType, TBOOL a_bCheckGoal, TBOOL a_bForce );
	TBOOL CheckSingleJump( const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, TFLOAT a_fGravity, TFLOAT a_fJumpSpeed, TFLOAT a_fDistance, TFLOAT a_fSpeed, TUINT32 a_uiFlags, APathfindingComponent::Obstruction* a_pObstruction, void* a_pCollisionCache );
	TBOOL CheckDoubleJump( const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, TFLOAT a_fGravity, TFLOAT a_fJumpSpeed, TFLOAT a_fDistance, TFLOAT a_fSpeed, TUINT32 a_uiFlags, APathfindingComponent::Obstruction* a_pObstruction, void* a_pCollisionCache );

	// Advances the jump; TFALSE once it's over
	TBOOL Update( TFLOAT a_fDeltaTime );

	// The position a_fTime after the current time (clamped to the end of the jump)
	Toshi::TVector4& GetPosition( Toshi::TVector4& a_rvPosition, TFLOAT a_fTime );
	Toshi::TVector4& GetVelocity( Toshi::TVector4& a_rvVelocity );

	// Drops a_rvPosition onto standable ground (a ray up from a_fHeight / 2 below it); TFALSE if there's none
	TBOOL IsValidPosition( Toshi::TVector4& a_rvPosition, TFLOAT a_fHeight );

	TBOOL CheckJumpCollision( ABaseEntity* a_pOwner, TFLOAT a_fTime );
	TBOOL CheckJumpCollision( TFLOAT a_fTime );
	TBOOL CheckFullJumpCollision( TUINT32 a_uiFlags, APathfindingComponent::Obstruction* a_pObstruction, void* a_pCollisionCache );

	TBOOL IsActive() const { return m_bActive; }
	void  SetActive( TBOOL a_bActive ) { m_bActive = a_bActive; }
	TBOOL HasValidTrajectory() const { return m_bActive; }
	// $deBlob: FUNCTION 007d9690
	TFLOAT GetTime() const { return m_fTime; }
	// $deBlob: FUNCTION 007d96c0
	TFLOAT GetTotalTime() const { return m_oTrajectory.m_fTotalTime; }

public:
	static AConsoleVar sm_cvShowTrajectory;

private:
	ABaseEntity* m_pOwner; // 0x00
	// NOTE: only Create writes these (a parabola of their own, its check reads the members below)
	Toshi::TVector4 m_vCreateAcceleration; // 0x04
	Toshi::TVector4 m_vCreateVelocity;     // 0x14
	Toshi::TVector4 m_vCreateStart;        // 0x24
	TFLOAT          m_fCreateTotalTime;    // 0x34
	TFLOAT          m_fTime;               // 0x38
	TFLOAT          m_fTimeScale;          // 0x3C
	TBOOL           m_bActive;             // 0x40
	TBOOL           m_bUnk41;              // 0x41
	Trajectory      m_oTrajectory;         // 0x44
};
