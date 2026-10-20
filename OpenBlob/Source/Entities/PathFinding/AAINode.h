#pragma once
#include <Math/TVector4.h>

class AAINode;
struct AStarNode;

//-----------------------------------------------------------------------------
// Purpose: a link between two nodes of the AI network (loaded by AAINetwork::LoadLinks). A link can be made stale for a
// while (SetStale) when an AI failed to follow it: the path finder then avoids it.
//-----------------------------------------------------------------------------
class AAILink
{
public:
	// m_uiFlags
	enum FLAG : TUINT8
	{
		FLAG_Stale = 0x1,
	};

	// m_uiMoveType (the motor's move type, see AAIWaypoint::FLAG)
	enum MOVETYPE : TUINT8
	{
		MOVETYPE_Walk,
		MOVETYPE_Jump,
		MOVETYPE_Fly,
		MOVETYPE_Climb,
		MOVETYPE_Count
	};

public:
	// $deBlob: FUNCTION 0078f460
	AAILink()
	{
		m_pNodeA     = TNULL;
		m_pNodeB     = TNULL;
		m_uiFlags    = 0;
		m_uiMoveType = 0;
	}

	// NOTE: empty
	// $deBlob: FUNCTION 0078f7a0
	~AAILink() {}

	// Stale for a_fTime seconds (a_fTime <= 0: not stale anymore)
	void SetStale( TFLOAT a_fTime );

	// Still stale (clears FLAG_Stale once the time is over)
	TBOOL IsStaleImp();

	TBOOL IsFlagged( TUINT8 a_uiFlag ) const { return ( m_uiFlags & a_uiFlag ) != 0; }

	// The node at the other end of the link
	AAINode* GetOtherNode( const AAINode* a_pNode ) const { return ( a_pNode == m_pNodeA ) ? m_pNodeB : m_pNodeA; }

public:
	TFLOAT   m_fStaleTime; // 0x00 the scheduler's total time when it stops being stale
	AAINode* m_pNodeA;     // 0x04
	AAINode* m_pNodeB;     // 0x08
	TUINT8   m_uiFlags;    // 0x0C FLAG
	TUINT8   m_uiMoveType; // 0x0D MOVETYPE
};

//-----------------------------------------------------------------------------
// Purpose: a node of the AI network (PathFinding\AAINode.cpp): a position, the links to the other nodes and the A*
// state of the search running in AAINetwork::FindPath.
//-----------------------------------------------------------------------------
class AAINode
{
public:
	// m_uiFlags (the low bits are the mask the vicinity searches filter with)
	enum FLAG : TUINT8
	{
		FLAG_Unk1         = 0x1, // set on every node
		FLAG_InClosedList = 0x2,
		FLAG_InOpenList   = 0x4,
	};

public:
	AAINode();
	~AAINode();

	// Resets the node and gives it room for a_uiLinksCount links
	void Initialise( TUINT8 a_uiLinksCount );

	void Create( TUINT8 a_uiLinksCount );

	// Puts a_pLink in the first free slot
	void AddLink( AAILink* a_pLink );

	// The link to a_pNode (TNULL if they aren't linked)
	AAILink* GetLinkTo( AAINode* a_pNode );

	// $deBlob: FUNCTION 0078faf0
	AAILink** GetLinks() const
	{
		TASSERT( TNULL != m_apLinks[ m_uiLinksCount - 1 ], "Getting links array before it is fully populated!" );
		return m_apLinks;
	}

	TBOOL IsFlagged( TUINT8 a_uiFlag ) const { return ( m_uiFlags & a_uiFlag ) != 0; }
	// $deBlob: FUNCTION 0078fae0
	TUINT GetFlagsRaw() const { return m_uiFlags; }

	const Toshi::TVector4& GetPosition() const { return m_vPosition; }
	// $deBlob: FUNCTION 0078fad0
	AStarNode* GetAStarNode() const { return m_pAStarNode; }
	// $deBlob: FUNCTION 0078fb40
	TUINT8 GetLinksCount() const { return m_uiLinksCount; }

public:
	Toshi::TVector4 m_vPosition;    // 0x00
	AAILink**       m_apLinks;      // 0x10
	AStarNode*      m_pAStarNode;   // 0x14 its entry in the open/closed list of AAINetwork::FindPath
	TUINT8          m_uiFlags;      // 0x18 FLAG
	TUINT8          m_uiSegmentID;  // 0x19 nodes of different segments aren't connected
	TUINT8          m_uiLinksCount; // 0x1A
};
