#pragma once
#include "Entities/AEntityComponent2.h"

#include "AAIMoveSolver.h"
#include "AIMoveGoal.h"
#include "AStack.h"

class AAIWaypoint;

//-----------------------------------------------------------------------------
// Purpose: steers an AI character around what blocks its move (PathFinding\AAISteer.cpp, AAINavigator::AttemptSteer).
// Free directions (suggestions) and blocked ones (constraints) around the wanted facing are probed and handed to an
// AAIMoveSolver; the solution becomes a temporary waypoint.
//-----------------------------------------------------------------------------
class AAISteer
    : public AEntityComponent2
{
public:
	AAISteer( ABaseEntity* a_pEntity );
	virtual ~AAISteer() override; // +0x00

	virtual void ResetComponent() override; // +0x08

	AAIWaypoint* AttemptSteer( TFLOAT a_fDeltaTime, const AIMoveGoal_t& a_rcGoal );
	// $deBlob: FUNCTION 007e8ac0
	void RenderConstraints() {} // the debug render is compiled out

private:
	TBOOL IsSolveable( ABaseEntity* a_pBlocker, const AIMoveGoal_t& a_rcGoal );
	void  AdjustSuggestions();
	TBOOL ProbeYaw( TFLOAT a_fYaw, TFLOAT a_fSpan, const Toshi::TVector4& a_rcPosition, TFLOAT a_fDistance, TFLOAT a_fWeight );
	void  AddMoveConstraints( TFLOAT a_fDistance );
	void  AddWorldConstraint( TFLOAT a_fYaw, TFLOAT a_fSpan, TFLOAT a_fDistance );
	void  AddNPCConstraint( TFLOAT a_fYaw, TFLOAT a_fSpan, TFLOAT a_fDistance );

private:
	AAIMoveSolver                 m_oSolver;         // 0x14
	AAIMoveSolver::ConstraintList m_lSuggestions;    // 0x3C
	Toshi::TVector4               m_vToGoal;         // 0x50
	TUINT8                        m_uiBlockerSide;   // 0x60 AAIMoveSolver::SIDE of m_pBlocker
	ABaseEntity*                  m_pBlocker;        // 0x64 the entity that blocks the move
	ABaseEntity*                  m_pLastBlocker;    // 0x68 m_pBlocker of the last solution
	TFLOAT                        m_fLastSolution;   // 0x6C
	AStack<ABaseEntity*, 20>      m_lMovingBlockers; // 0x70
};
