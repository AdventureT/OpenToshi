#pragma once
#include "Entities/AEntityComponent2.h"
#include "AAIWaypoint.h"

//-----------------------------------------------------------------------------
// Purpose: an AI path (PathFinding\AAIPath.cpp): the waypoints AAIPathFinder found, followed by AAINavigator (it owns
// two, the current one and the one being built).
//-----------------------------------------------------------------------------
class AAIPath
    : public AEntityComponent2
{
public:
	// How far LookAheadOptimise looks
	struct OptimiseParams
	{
		TFLOAT fShortcutDistance;  // 0x0 a point is only shortcut to when it's this close to the entity
		TFLOAT fLookAheadDistance; // 0x4 how far along the path to look (from the entity)
		TFLOAT fStep;              // 0x8 the step along a link
		TINT   iMaxSteps;          // 0xC
	};

public:
	AAIPath( ABaseEntity* a_pEntity );
	virtual ~AAIPath() override; // +0x00

	// Gives every waypoint back to the free list
	void ClearPath();

	void AddWaypoint( AAIWaypoint* a_pWaypoint );

	// Rounds the corner at the waypoint after a_pWaypoint when the path turns there by more than 30 degrees (XZ) and
	// both links are walked: the corner is replaced by 4 points of a Hermite curve from a_fRadius before it to
	// a_fRadius after it.
	void SmoothCorneringXZ( AAIWaypoint* a_pWaypoint, TFLOAT a_fRadius );

	// Gives the waypoints from a_pStart to a_pEnd (both included) back to the free list
	void RemoveWaypoints( AAIWaypoint* a_pStart, AAIWaypoint* a_pEnd );

	// Replaces the waypoints from a_pStart to a_pTarget by one at a_rvPosition (or by a_pTarget itself when it's
	// there) and returns it
	AAIWaypoint* ShortcutPath( AAIWaypoint* a_pStart, AAIWaypoint* a_pTarget, const Toshi::TVector4& a_rvPosition );

	// Walks the path from a_pWaypoint for a_rcParams.fLookAheadDistance looking for a point the entity can walk to
	// directly; on success a_pNewWaypoint is the shortcut's waypoint
	TBOOL LookAheadOptimise( const OptimiseParams& a_rcParams, AAIWaypoint* a_pWaypoint, AAIWaypoint*& a_pNewWaypoint );
	TBOOL LookAheadOptimiseRecursive( AAIWaypoint* a_pWaypoint, const Toshi::TVector4& a_rvPosition, const OptimiseParams& a_rcParams, TBOOL a_bCanShortcut, TFLOAT& a_rfDistanceLeft, TINT& a_riSteps, AAIWaypoint* a_pStartWaypoint, AAIWaypoint*& a_pNewWaypoint );

	// Whether the entity can walk straight to a_rvPosition
	TBOOL CanShortcut( const Toshi::TVector4& a_rvPosition );

	// Shortcuts the path from a_pStartwaypoint (AAINavigator::SetGoalWaypoint, ai_optimise_path) and returns the
	// waypoint to go to
	AAIWaypoint* OptimisePath( AAIWaypoint* a_pStartwaypoint );

	// Steps from a_rvFrom to a_rvTo by a_fStep: a_rvOutPoint is the last point reachable on the ground from a_rvStart,
	// returns its fraction of the way.
	TFLOAT GetClosestReachablePointToEnd( Toshi::TVector4& a_rvOutPoint, const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvFrom, const Toshi::TVector4& a_rvTo, TFLOAT a_fStep );

public:
	static OptimiseParams ms_oOptimiseParams; // { 12, 40, 4, 8 }

public:
	Toshi::T2DList<AAIWaypoint> m_Path; // 0x14
};
