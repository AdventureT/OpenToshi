#pragma once
#include "AAINode.h"
#include "AAINodeHash.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Toshi/T2Vector.h>
#include <Toshi/T2SortedVector.h>
#include <Toshi/T2SortedList.h>

class ABaseEntity;

// An entry of the open/closed lists of AAINetwork::FindPath (0x1C bytes, pooled by AAINetwork)
struct AStarNode
    : public Toshi::T2DList<AStarNode>::Node
{
	AAINode*   node;    // 0x08
	AStarNode* pParent; // 0x0C
	TFLOAT     fCost;   // 0x10 from the start
	TFLOAT     fGuess;  // 0x14 heuristic to the goal
	TFLOAT     fTotal;  // 0x18 fCost + fGuess, the open list is sorted by it
};

//-----------------------------------------------------------------------------
// Purpose: the level's AI network (PathFinding\AAINetwork.cpp): the nodes and links loaded from the level's network
// data block (in steps, Load is called once per frame until it's done), their hash and the A* path search between two
// nodes.
//-----------------------------------------------------------------------------
class AAINetwork
    : public Toshi::TSingleton<AAINetwork>
{
public:
	static TCONSTEXPR_VAR TINT MAX_NODES     = 500;
	static TCONSTEXPR_VAR TINT MAX_PATH_SIZE = 128;

	using SortedNodeRef = AAINodeHash::NodeRef;

	struct SortNodeByDist
	{
		TINT operator()( const SortedNodeRef& a_rcVal1, const SortedNodeRef& a_rcVal2 ) const
		{
			TFLOAT fDiff = a_rcVal1.fDistSq - a_rcVal2.fDistSq;

			if ( 0.0f < fDiff )
				return 1;

			if ( fDiff < 0.0f )
				return -1;

			return 0;
		}
	};

	using ClosestNodes = Toshi::T2SortedVector<SortedNodeRef, Toshi::T2Vector<SortedNodeRef, MAX_NODES>, SortNodeByDist>;

	// The "Header" symbol of the network data block
	struct NodeData
	{
		TUINT32         uiID;         // 0x00
		TUINT32         uiUnk4;       // 0x04
		Toshi::TVector4 vPosition;    // 0x08
		TUINT8          uiSegmentID;  // 0x18
		TUINT8          uiLinksCount; // 0x19
	};

	struct LinkData
	{
		TINT    iNodeA;     // 0x00
		TINT    iNodeB;     // 0x04
		TUINT32 uiMoveType; // 0x08
		TUINT32 uiUnkC;     // 0x0C
	};

	struct Header
	{
		TUINT32   uiVersion;  // 0x00 low byte: flags, high word: version
		TINT      iNodeCount; // 0x04
		TINT      iLinkCount; // 0x08
		LinkData* pLinks;     // 0x0C
		NodeData* pNodes;     // 0x10
	};

	// m_iLoadState
	enum LOADSTATE
	{
		LOADSTATE_Alloc,
		LOADSTATE_Nodes,
		LOADSTATE_Links,
		LOADSTATE_CreateHash,
		LOADSTATE_HashNodes,
		LOADSTATE_Done,
	};

public:
	AAINetwork();
	~AAINetwork();

	// The closest node (with any flag) within a_fRange that isn't one of a_ppIgnore
	AAINode* GetClosestNode( const Toshi::TVector4& a_rvPosition, TFLOAT a_fRange, AAINode** a_ppIgnore, TINT a_iNumIgnore );

	// Adds the nodes within a_fRange to a_rNodes, sorted by distance (at most a_iMaxNodes, the farther ones dropped)
	void GetClosestNodes( const Toshi::TVector4& a_rvPosition, ClosestNodes& a_rNodes, TINT a_iMaxNodes, TFLOAT a_fRange, TUINT8 a_uiNodeMaskFlags );

	// GetClosestNodes into m_oClosestNodes (cleared first)
	ClosestNodes& GetClosestNodes2( const Toshi::TVector4& a_rvPosition, TINT a_iMaxNodes, TFLOAT a_fRange, TUINT8 a_uiNodeMaskFlags );

	// The A* path from a_pStart to a_pGoal: m_apPath (from the goal back to the start, a_rPathLength nodes) or TNULL
	AAINode** FindPath( AAINode* a_pStart, AAINode* a_pGoal, ABaseEntity* a_pEntity, TINT& a_rPathLength, TFLOAT* a_pCost );

	static TFLOAT AStarHeuristic( AAINode* a_pNode, AAINode* a_pGoal );
	static TFLOAT MoveCost( AAINode* a_pFrom, AAINode* a_pTo, ABaseEntity* a_pEntity );

	// The entry of a_pNode in a_rList
	static AStarNode* FindNodeInList( AAINode* a_pNode, Toshi::T2DList<AStarNode>& a_rList );

	void InitLoad();

	// One step of the loading (spends ~15 ms at most); TTRUE once it's done
	TBOOL Load();

	// The x/z bounds of the nodes
	void GetBounds( TFLOAT& a_rfMinX, TFLOAT& a_rfMinZ, TFLOAT& a_rfMaxX, TFLOAT& a_rfMaxZ );

	TUINT16  GetNodeCount() const { return m_uiNodeCount; }
	AAINode* GetNode( TINT a_iIndex ) const { return &m_pNodeBlock[ a_iIndex ]; }

private:
	void LoadLinks( LinkData* a_pDataSection, TUINT32 a_uiVersion, TINT a_iStart, TINT a_iEnd );
	void LoadNodes( NodeData* a_pDataSection, TINT a_iCount, TUINT32 a_uiVersion, TINT a_iStart, TINT a_iEnd );

	// Adds 256 entries to m_oAStarPool
	void FillAStarPool();

private:
	TUINT16                   m_uiNodeCount;             // 0x0000
	TUINT16                   m_uiLinkCount;             // 0x0002
	Toshi::T2DList<AStarNode> m_oAStarPool;              // 0x0004
	AAINodeHash*              m_pNodeHash;               // 0x000C
	ClosestNodes              m_oClosestNodes;           // 0x0010
	TINT                      m_iLoadState;              // 0x0FB4 LOADSTATE
	TINT                      m_iLoadIndex;              // 0x0FB8
	AAINode*                  m_pNodeBlock;              // 0x0FBC
	AAILink*                  m_pLinkBlock;              // 0x0FC0
	AAINode*                  m_apPath[ MAX_PATH_SIZE ]; // 0x0FC4
};
