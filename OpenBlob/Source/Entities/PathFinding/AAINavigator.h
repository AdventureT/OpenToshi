#pragma once
#include "Entities/AEntityComponent2.h"
#include "AIMoveGoal.h"

#include <Math/TVector4.h>

class AAIPath;
class AAIWaypoint;
class AAILink;
class AAIPathFinder;
class AGndMotorComponent;
class ABuildPathJob;

//-----------------------------------------------------------------------------
// Purpose: moves an AI character along its path (PathFinding\AAINavigator.cpp): a path build job (BuildPath, run by
// ABuildPathJobQueue) finds the path from the ground (BuildPathCheckGround), the local probes (BuildPathLocal) or the
// AI network (BuildPathNetwork); Tick then pushes the ground motor to each waypoint in turn
// (AGndMotorComponent::MoveToGoal) until the last one is reached or the move gets stuck.
//-----------------------------------------------------------------------------
class AAINavigator
    : public AEntityComponent2
{
public:
	// m_eState
	enum STATE
	{
		STATE_None     = 0,
		STATE_Moving   = 1,
		STATE_Finished = 2, // m_eResult
	};

	// m_eResult
	enum RESULT
	{
		RESULT_Reached = 0,
		RESULT_Failed  = 1,
		RESULT_None    = 2,
	};

	// How the ground under the goal is used (BuildPathCheckGround)
	enum eGroundGoal : TUINT8
	{
		GROUNDGOAL_Keep   = 0,
		GROUNDGOAL_Ground = 1, // moved onto the ground under it
	};

	// Which path is built: the current one or the other one (an aux path, ActivateAuxPath)
	enum eDoubleBuffer : TUINT8
	{
		DOUBLEBUFFER_Current = 0,
		DOUBLEBUFFER_Other   = 1,
	};

	// The start and goal of a path build job
	struct BuildPathState
	{
		Toshi::TVector4 vStart; // 0x00
		Toshi::TVector4 vGoal;  // 0x10
	};

public:
	AAINavigator( ABaseEntity* a_pEntity );
	virtual ~AAINavigator() override; // +0x00

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void ResetComponent() override;            // +0x08

	void  ClearGoal( TBOOL a_bStopMotor );
	TBOOL GetEndGoal( Toshi::TVector4& a_rvOutGoal );
	TBOOL FaceDirection( const Toshi::TVector4& a_rvFacing );
	TBOOL DestinationValid( const Toshi::TVector4& a_rvGoal, Toshi::TVector4* a_pOutNodePosition );
	// The waypoints from the current one to the end of the path (0 without one)
	TINT GetNumWaypointsLeft() const;
	// Not moving, or the end of the path is within a_fDistance of the entity
	TBOOL IsNearEndGoal( TFLOAT a_fDistance );

	ABuildPathJob* BuildPath( const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, TFLOAT a_fRadius, TBOOL a_bUnk, const Toshi::TVector4* a_pFacing, eGroundGoal a_eGroundGoal, eDoubleBuffer a_eDoubleBuffer );
	TBOOL          BuildPathCheckGround( BuildPathState& a_rState, const Toshi::TVector4& a_rvGoal, eGroundGoal a_eGroundGoal );
	TBOOL          BuildPathLocal( BuildPathState& a_rState, TFLOAT a_fRadius, const Toshi::TVector4* a_pFacing, eDoubleBuffer a_eDoubleBuffer );
	TBOOL          BuildPathNetwork( BuildPathState& a_rState, TFLOAT a_fRadius, const Toshi::TVector4* a_pFacing, eDoubleBuffer a_eDoubleBuffer );

	TINT GetState() const { return m_eState; }
	// $deBlob: FUNCTION 007e45c0
	TINT  GetResult() const { return m_eResult; }
	TBOOL IsOtherPathValid() const { return m_aiPathInvalid[ m_iCurrentPath == 0 ] == 0; }
	TBOOL HasValidAuxPath() const { return IsOtherPathValid(); }

	// Makes the other (aux) path the current one and follows it
	void ActivateAuxPath();

	// The paths optimised this frame (at most 4, SetGoalWaypoint), reset by AEnemyManager::Tick
	static TINT sm_iNumOptimisedPaths;

private:
	void  SetGoalWaypoint( AAIWaypoint* a_pWaypoint, AAIPath* a_pPath, const Toshi::TVector4* a_pFacing );
	void  OnReachedGoal();
	TBOOL AttemptSteer( TFLOAT a_fDeltaTime );
	void  CheckProgress();

	AAIPath* CurrentPath() const { return m_apPaths[ m_iCurrentPath ]; }

private:
	// The time the ground goal is predicted ahead with the motor's velocity (BuildPathCheckGround)
	static TFLOAT sm_fPredictTime;

private:
	TINT                m_eState;             // 0x14 STATE
	AAIWaypoint*        m_pCurrentWaypoint;   // 0x18
	AAILink*            m_pCurrentLink;       // 0x1C the AI network link to the current waypoint
	AIMoveGoal_t        m_CurrentGoal;        // 0x20
	AIMoveGoal_t        m_SteerGoal;          // 0x4C the goal before AttemptSteer
	TINT                m_eResult;            // 0x78 RESULT
	AAIPathFinder*      m_pPathFinder;        // 0x7C
	AGndMotorComponent* m_pMotor;             // 0x80
	TINT                m_iUnk84;             // 0x84
	TUINT8              m_aUnk88[ 0x18 ];     // 0x88
	TFLOAT              m_fSteerEndTime;      // 0xA0
	TUINT8              m_uiFlags;            // 0xA4 1: steering
	Toshi::TVector4     m_vLastPosition;      // 0xA8
	TFLOAT              m_fStuckTime;         // 0xB8
	AAIPath*            m_apPaths[ 2 ];       // 0xBC
	TINT                m_iCurrentPath;       // 0xC4
	TINT                m_aiPathInvalid[ 2 ]; // 0xC8 1: the path has to be built again
};
