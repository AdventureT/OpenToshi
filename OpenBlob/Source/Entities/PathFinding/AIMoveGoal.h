#pragma once
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: where AAINavigator moves its owner to: the goal (the current waypoint), the facing to end with, how to move
// there (the ground motor's move type) and how close is close enough.
//-----------------------------------------------------------------------------
struct AIMoveGoal_t
{
	// The goal of a reset move goal
	static const Toshi::TVector4 scm_vInvalidGoal;

	// $deBlob: FUNCTION 007e5e50
	const Toshi::TVector4& GetGoal() const { return vGoal; }

	// $deBlob: FUNCTION 007e5e60
	void Reset()
	{
		vGoal     = scm_vInvalidGoal;
		iUnk20    = 1;
		eMoveType = 4;
		vFacing   = Toshi::TVector4::VEC_ZERO;
		fRadius   = 0.0f;
	}

	Toshi::TVector4 vGoal;     // 0x00
	Toshi::TVector4 vFacing;   // 0x10
	TINT            iUnk20;    // 0x20
	TINT            eMoveType; // 0x24 AGndMotorComponent::MoveToGoal (0: walk, 1: jump, 2: fly, 3: climb)
	TFLOAT          fRadius;   // 0x28
};
