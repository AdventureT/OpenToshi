#pragma once
#include "Entities/AEntityComponent2.h"

#include <Math/TVector4.h>
#include <Toshi/T2Array.h>

class AAIPath;
class AAINode;

//-----------------------------------------------------------------------------
// Purpose: the AI network nodes closest to a position (AAINodeCache<1>, Projects\Entities\PathFinding\AAINodeCache.h of
// the original tree), refreshed by AAIPathFinder::GetClosestReachableNode when the position moves.
//-----------------------------------------------------------------------------
struct AAINodeCache
{
	static TCONSTEXPR_VAR TINT CACHE_SIZE = 3;

	AAINode* GetNode( TINT a_iIndex ) { return m_apNodes[ a_iIndex ]; }

	// $deBlob: FUNCTION 007e8460
	void SetNode( AAINode* a_pNode, TINT a_iIndex ) { m_apNodes[ a_iIndex ] = a_pNode; }

	// NOTE: retail checks the previous m_iFilledSize, before it's set
	// $deBlob: FUNCTION 007e8630
	void SetFilledSize( TINT a_iFilledSize )
	{
		TASSERT( m_iFilledSize <= CACHE_SIZE );
		m_iFilledSize = a_iFilledSize;
	}

	Toshi::T2Array<AAINode*, CACHE_SIZE> m_apNodes;     // 0x00
	Toshi::TVector4                      m_vPosition;   // 0x0C the position the nodes were found for
	TINT                                 m_iFilledSize; // 0x1C
};

//-----------------------------------------------------------------------------
// Purpose: finds the path of an AI character (PathFinding\AAIPathFinder.cpp): a direct move the probes allow (walk,
// jump, fly; around the obstruction with Triangulate) or a route through the AI network.
//-----------------------------------------------------------------------------
class AAIPathFinder
    : public AEntityComponent2
{
public:
	// The moves of PathFindLocal_Imp
	enum MOVEKIND
	{
		MOVEKIND_Ground = 0,
		MOVEKIND_Jump   = 1,
		MOVEKIND_Fly    = 2,
		MOVEKIND_Climb  = 3,
	};

public:
	AAIPathFinder( ABaseEntity* a_pEntity );
	virtual ~AAIPathFinder() override; // +0x00

	virtual void ResetComponent() override; // +0x08

	// 1: a_pPath leads to the goal, 2: already there, else no path (-2: blocked by an object)
	TINT PathFindLocal( const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, AAIPath* a_pPath, TFLOAT a_fRadius );

	// 1: a_pPath leads through the AI network to the goal
	TINT PathFindNetwork( const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, AAIPath* a_pPath, TFLOAT a_fRadius );

	// The goal can be reached from a node of the AI network (its position in a_pOutNodePosition if not TNULL)
	TBOOL DestinationValid( const Toshi::TVector4& a_rvGoal, Toshi::TVector4* a_pOutNodePosition );

	// Does a probe result block the move?
	static TBOOL IsBlocked( TINT a_eProbeResult );

	// The debug command's callback (ADebugCommand*, not ported): toggles sm_bDebug
	// NOTE: retail pops its argument (RET 4) and doesn't use ECX
	static TBOOL ToggleDebug( void* a_pCommand );

	static TBOOL sm_bDebug;

private:
	TINT PathFindLocal_Imp( const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, AAIPath* a_pPath, TFLOAT a_fRadius, MOVEKIND a_eMoveKind );

	// The route between two nodes of the same segment (AAINetwork::FindPath) as waypoints of a_pPath
	TINT PathFindNetwork( AAINode* a_pStart, AAINode* a_pEnd, AAIPath* a_pPath );

	// A point beside the line to a_rvGoal (four box widths to either side of the point a_fDistance along it) the
	// ground moves reach both ends from; only for MOVEKIND_Ground
	TBOOL Triangulate( const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, Toshi::TVector4& a_rvOutApex, TFLOAT a_fDistance, TINT a_eMoveKind );

	// The first node of a_rCache (refreshed for a_rvPosition) a move reaches (walk, jump, fly in that order; to the node,
	// or from it if a_bFromNode), its MOVEKIND in a_rMoveKind; TNULL if none
	AAINode* GetClosestReachableNode( const Toshi::TVector4& a_rvPosition, AAINodeCache& a_rCache, TINT& a_rMoveKind, TBOOL a_bFromNode, TUINT32 a_uiProbeFlags );

private:
	AAINodeCache m_oStartCache; // 0x14
	AAINodeCache m_oEndCache;   // 0x34
};
