#pragma once
#include "Entities/AEntityComponent2.h"
#include "Collision/ACollisionFunctor.h"

#include <Math/TVector4.h>
#include <Math/TMathInline.h>

class AGndMotorComponent;

namespace Toshi
{
class TPhysicsPhantomContainer;
}

//-----------------------------------------------------------------------------
// Purpose: tests whether an AI character can make a move (PathFinding\AAIMoveProbe.cpp): walking (the box is swept
// along the ground step by step, SteppedGroundTest), jumping, flying and climbing; what blocks the move goes to the
// pathfinding's obstruction. The probe results are PROBERESULT.
//-----------------------------------------------------------------------------
class AAIMoveProbe
    : public AEntityComponent2
    , public ACollisionFunctor
{
public:
	// The result of a probe (AAIPathFinder: 0-4 block the move, 5 is clear)
	enum PROBERESULT
	{
		PROBERESULT_PushableObject = 1, // the obstruction is flagged 0x20
		PROBERESULT_Object         = 2, // the obstruction is flagged 0x10
		PROBERESULT_Wall           = 3,
		PROBERESULT_NoGround       = 4,
		PROBERESULT_Clear          = 5,
	};

	// The polygons of the ground under the box (SteppedGroundTest): only the walkable ones, the closest is kept
	class GroundPolyFunctor
	    : public ACollisionFunctor
	{
	public:
		GroundPolyFunctor()
		{
			Reset();
		}

		void Reset()
		{
			m_bHasTriangle = TFALSE;
			m_bNonMesh     = TFALSE;
			m_fDistance    = Toshi::TMath::MAXFLOAT;
		}

		virtual TBOOL PreFilterSweptAABBPoly( const Toshi::TVector4& a_rNormal, const Toshi::TVector4* a_pTriangle ) override; // +0x00
		// NOTE: the base's HitNonSolidObject returns TFALSE
		virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override { return TFALSE; }         // +0x10
		virtual void  AABBSweptHitPoly( const Toshi::TVector4* a_pTriangle, TFLOAT a_fDistance ) override; // +0x1C

	public:
		Toshi::TVector4 m_avTriangle[ 3 ]; // 0x04
		TBOOL           m_bHasTriangle;    // 0x34
		TFLOAT          m_fDistance;       // 0x38
		TBOOL           m_bNonMesh;        // 0x3C
	};

public:
	AAIMoveProbe( ABaseEntity* a_pEntity );
	virtual ~AAIMoveProbe() override; // +0x00

	// NOTE: both empty in de Blob (AEntityFramework::Startup / Shutdown)
	static void Init();
	static void Destroy();

	virtual void ResetComponent() override; // +0x08

	// NOTE: the base's HitNonSolidObject returns TFALSE
	virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override { return TFALSE; } // +0x10 (ACollisionFunctor)

	// Adds the probe's phantom (created with the owner's box) to the simulation or removes it
	void SetPhantomActive( TBOOL a_bActive );

	TINT CheckGroundMove( const Toshi::TVector4& a_rvStart, Toshi::TVector4& a_rvGoal, TUINT32 a_uiFlags );

	// Climbing: always PROBERESULT_NoGround in retail (the obstruction is reset before it's read)
	TINT CheckClimbMove( const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, TUINT32 a_uiFlags );

	// A jump along AAIJumpTrajectory from a_rvStart to a_rvGoal
	TINT CheckJumpMove( const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, TUINT32 a_uiFlags );

	// The owner's box swept straight from a_rvStart to a_rvGoal; 0: it hits the terrain
	TINT CheckFlyMove( const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, TUINT32 a_uiFlags );

	// A box (the owner's footprint, 0.2 high) swept a_fProbeDist down onto the ground; TTRUE if it lands on a ground
	// triangle and the owner's position is inside the triangle, at least 0.2 from its edges (XZ)
	// NOTE: retail sweeps from a zero vector, not from a_rvPosition (unused, like a_iUnk and a_fUnk)
	TBOOL IsStandablePosition( const Toshi::TVector4& a_rvPosition, TFLOAT a_fProbeDist, TINT a_iUnk, TFLOAT a_fUnk );

	// One step of a walk from a_rvStart to a_rvPosition (moved onto the ground or up the slope); 1: fine, -1: no
	// ground, -2: the step can't be climbed, -3: blocked (the obstruction is set). a_ruiFlags (SGT_FLAGS): 1 no layer 4,
	// 2 skip the ground probe (cleared once on the ground), 8/0x10/0x20 retest.
	TINT TestGroundStep( const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvDirection, Toshi::TVector4& a_rvPosition, TFLOAT a_fUnk, TFLOAT a_fStepHeight, TUINT32& a_ruiFlags, void* a_pCollisionCache );

	// The PROBERESULT of a collision object: 0 terrain, PROBERESULT_PushableObject or PROBERESULT_Object
	TINT ProbeResult( ACollisionObject* a_pObject );

	// Sweeps a_rvHalfExtents a_rfProbeDst down from a_rvPosition (a_rfProbeDst: the distance to the ground); TFALSE
	// without a collision cache (ACollisionCommon::GatheredCollisionList, a_bUnk is unused)
	TBOOL ProbeGround( TFLOAT& a_rfProbeDst, const Toshi::TVector4& a_rvPosition, const Toshi::TVector4& a_rvHalfExtents, void* a_pCollisionCache, TBOOL a_bUnk );

private:
	TINT SteppedGroundTest( const Toshi::TVector4& a_rvStart, Toshi::TVector4& a_rvGoal, TFLOAT a_fUnk, TFLOAT a_fStepHeight, TFLOAT a_fUnk2, TUINT32 a_uiFlags );

private:
	AGndMotorComponent*              m_pMotor;   // 0x18
	Toshi::TPhysicsPhantomContainer* m_pPhantom; // 0x1C
};
