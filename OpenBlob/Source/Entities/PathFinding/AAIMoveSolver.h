#pragma once
#include <Math/TVector4.h>
#include <ToshiTools/T2DynamicVector.h>

//-----------------------------------------------------------------------------
// Purpose: picks the yaw an AI character steers to (PathFinding\AAIMoveSolver.cpp). The constraints (arcs of yaw with a
// weight) are spread over 60 buckets of 6 degrees, the heaviest bucket wins.
//-----------------------------------------------------------------------------
class AAIMoveSolver
{
public:
	enum CONSTRAINTTYPE : TINT
	{
		CONSTRAINTTYPE_Suggestion, // a free direction (AAISteer::ProbeYaw)
		CONSTRAINTTYPE_NPC,        // a direction blocked by an entity (AAISteer::AddNPCConstraint)
		CONSTRAINTTYPE_World,      // a direction blocked by the world (AAISteer::AddWorldConstraint)
		CONSTRAINTTYPE_NUMOF
	};

	enum SIDE : TUINT8
	{
		SIDE_Left  = 0x1, // what blocks is on the left of the goal direction (cross product y > 0)
		SIDE_Right = 0x2, // ... on the right (cross product y < 0)
	};

	struct Arc
	{
		TFLOAT m_fCenter; // 0x0 degrees, [0, 360)
		TFLOAT m_fSpan;   // 0x4 degrees, (0, 360)
	};

	struct Constraint
	{
		TFLOAT         m_fWeight;   // 0x00
		CONSTRAINTTYPE m_eType;     // 0x04
		Arc            m_oArc;      // 0x08
		TUINT8         m_uiSide;    // 0x10 SIDE
		TFLOAT         m_fDistance; // 0x14 how far was probed
	};

	using ConstraintList = Toshi::T2DynamicVector<Constraint>;

public:
	AAIMoveSolver();
	~AAIMoveSolver();

	void  AddConstraint( const Constraint& a_rcConstraint );
	void  Reset();
	TBOOL Solve( const ConstraintList& a_rcSuggestions, TFLOAT& a_rfSolution );

	static void            NormaliseContraints( ConstraintList& a_rConstraints );
	static void            AssertYawConversion( TINT a_iYaw, Toshi::TVector4 a_vOriginal );
	static TINT            DirectionToYaw( const Toshi::TVector4& a_rcDirection );
	static Toshi::TVector4 YawToDirection( TINT a_iYaw );
	static Toshi::TVector4 YawToDirection( TFLOAT a_fYaw );
	static TFLOAT          YawDelta( TFLOAT a_fYaw1, TFLOAT a_fYaw2 );

	// $deBlob: FUNCTION 007eb780
	static void WrapYaw( TINT& a_riYaw )
	{
		if ( a_riYaw < 0 )
			a_riYaw += 360;

		// NOTE: retail lets 360 through (the assert below catches it)
		if ( a_riYaw > 360 )
			a_riYaw -= 360;

		TASSERT( a_riYaw >= 0 && a_riYaw < 360 );
	}

private:
	ConstraintList m_lConstraints;     // 0x00
	ConstraintList m_lTempConstraints; // 0x14
};
