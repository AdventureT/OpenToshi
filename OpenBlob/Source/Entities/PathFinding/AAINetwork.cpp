#include "pch.h"
#include "AAINetwork.h"
#include "APathfindingComponent.h"
#include "ACurrentLevelInformation.h"
#include "Entities/ABaseEntity.h"

#include <File/TTRB.h>
#include <Toshi/THPTimer.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AStarNode ) == 0x1C );
TSTATICASSERT( sizeof( AAINetwork ) == 0x11C4 );

// How much longer than the straight line a path may be
static TCONSTEXPR_VAR TFLOAT s_fMaxPathCostFactor = 3.0f;

// The cost factor of a jump link for an entity that can jump
static TCONSTEXPR_VAR TFLOAT s_fJumpCostFactor = 1.414f;

// The extra cost of a jump link for an entity that can't jump, and of a stale link
static TCONSTEXPR_VAR TFLOAT s_fBlockedCost = 10000.0f;

// The time slice of one Load step
static TCONSTEXPR_VAR TFLOAT s_fLoadTimeSlice = 0.015f;

// The node and link counts of the network data block before the "per node links count" version
static TCONSTEXPR_VAR TUINT8 s_uiDefaultLinksCount = 16;

// $deBlob: FUNCTION 0078dc90
AAINetwork::AAINetwork()
{
	m_uiNodeCount = 0;
	m_uiLinkCount = 0;
	m_pNodeHash   = TNULL;
	m_iLoadState  = -1;
	m_pNodeBlock  = TNULL;
	m_pLinkBlock  = TNULL;

	FillAStarPool();
}

// $deBlob: FUNCTION 0078dd80
// $deBlob: FUNCTION 0078d9d0
AAINetwork::~AAINetwork()
{
	m_oAStarPool.DeleteAll();

	if ( m_pNodeBlock )
		delete[] m_pNodeBlock;

	if ( m_pLinkBlock )
		delete[] m_pLinkBlock;

	if ( m_pNodeHash )
		delete m_pNodeHash;
}

void AAINetwork::FillAStarPool()
{
	for ( TINT i = 256; i != 0; i-- )
		m_oAStarPool.PushBack( new AStarNode );
}

// $deBlob: FUNCTION 0078de90
AAINode* AAINetwork::GetClosestNode( const TVector4& a_rvPosition, TFLOAT a_fRange, AAINode** a_ppIgnore, TINT a_iNumIgnore )
{
	AAINode*       pClosest   = TNULL;
	TFLOAT         fClosestSq = a_fRange * a_fRange;
	TUINT          uiNumNodes = 0;
	SortedNodeRef* pRefs      = m_pNodeHash->VicinityHashCompare( a_rvPosition, a_fRange, uiNumNodes, MAX_NODES, 0xFF );

	for ( TUINT i = 0; i < uiNumNodes; i++ )
	{
		TBOOL bIgnored = TFALSE;

		for ( TINT j = 0; j < a_iNumIgnore; j++ )
		{
			if ( pRefs[ i ].pNode == a_ppIgnore[ j ] )
			{
				bIgnored = TTRUE;
				break;
			}
		}

		if ( !bIgnored && fClosestSq > pRefs[ i ].fDistSq )
		{
			pClosest   = pRefs[ i ].pNode;
			fClosestSq = pRefs[ i ].fDistSq;
		}
	}

	return pClosest;
}

// $deBlob: FUNCTION 0078df40
void AAINetwork::GetClosestNodes( const TVector4& a_rvPosition, ClosestNodes& a_rNodes, TINT a_iMaxNodes, TFLOAT a_fRange, TUINT8 a_uiNodeMaskFlags )
{
	TASSERT( a_iMaxNodes <= MAX_NODES );

	TUINT          uiNumNodes = 0;
	TFLOAT         fRangeSq   = a_fRange * a_fRange;
	SortedNodeRef* pNodes     = m_pNodeHash->VicinityHashCompare( a_rvPosition, a_fRange, uiNumNodes, MAX_NODES, a_uiNodeMaskFlags );

	for ( TUINT uiNode = 0; uiNode < uiNumNodes; uiNode++ )
	{
		TASSERT( 0 != ( pNodes[ uiNode ].pNode->GetFlagsRaw() & a_uiNodeMaskFlags ) );
		SortedNodeRef oRef = pNodes[ uiNode ];

		if ( oRef.fDistSq > fRangeSq )
			continue;

		if ( a_rNodes.Size() == a_iMaxNodes )
		{
			// Full: drops the farthest one if this one is closer
			TBOOL bNotUnique;
			if ( a_rNodes.FindInsertionPoint( oRef, bNotUnique ) == a_rNodes.End() )
				continue;

			a_rNodes.PopBack();
		}

		a_rNodes.Push( oRef );
	}
}

// $deBlob: FUNCTION 0078e110
AAINetwork::ClosestNodes& AAINetwork::GetClosestNodes2( const TVector4& a_rvPosition, TINT a_iMaxNodes, TFLOAT a_fRange, TUINT8 a_uiNodeMaskFlags )
{
	m_oClosestNodes.Clear();
	GetClosestNodes( a_rvPosition, m_oClosestNodes, a_iMaxNodes, a_fRange, a_uiNodeMaskFlags );

	return m_oClosestNodes;
}

// Sorts the open list of FindPath by the total cost (inserted before the first entry with a higher one)
struct AStarNodeSortResults
{
	TINT operator()( const AStarNode& a_rcVal1, const AStarNode& a_rcVal2 ) const
	{
		if ( a_rcVal1.fTotal <= a_rcVal2.fTotal && a_rcVal2.fTotal != a_rcVal1.fTotal )
			return -1;

		return 1;
	}
};

using AStarSortedList = T2SortedList<AStarNode, T2DList<AStarNode>, AStarNodeSortResults>;

// $deBlob: FUNCTION 0078e170
AAINode** AAINetwork::FindPath( AAINode* a_pStart, AAINode* a_pGoal, ABaseEntity* a_pEntity, TINT& a_rPathLength, TFLOAT* a_pCost )
{
	a_rPathLength = 0;

	if ( a_pStart->m_uiSegmentID != a_pGoal->m_uiSegmentID )
		return TNULL;

	if ( a_pStart == a_pGoal )
	{
		m_apPath[ 0 ] = a_pStart;
		a_rPathLength++;

		if ( a_pCost )
			*a_pCost = 0.0f;

		return m_apPath;
	}

	TFLOAT             fMaxCost = TVector4::Distance( a_pStart->m_vPosition, a_pGoal->m_vPosition ) * s_fMaxPathCostFactor;
	T2DList<AStarNode> oClosedList;
	AStarSortedList    oOpenList;

	if ( m_oAStarPool.IsEmpty() )
		FillAStarPool();

	TASSERT( TFALSE == a_pStart->IsFlagged( AAINode::FLAG_InClosedList ) );
	TASSERT( TFALSE == a_pStart->IsFlagged( AAINode::FLAG_InOpenList ) );

	AStarNode* pStartNode = m_oAStarPool.PopBack();
	pStartNode->node      = a_pStart;
	pStartNode->pParent   = TNULL;
	pStartNode->fCost     = 0.0f;
	pStartNode->fGuess    = 0.0f;
	pStartNode->fTotal    = 0.0f;
	a_pStart->m_uiFlags |= AAINode::FLAG_InOpenList;
	pStartNode->fGuess = AStarHeuristic( pStartNode->node, a_pGoal );
	pStartNode->fTotal = pStartNode->fCost + pStartNode->fGuess;
	oOpenList.Push( pStartNode );
	pStartNode->node->m_pAStarNode = pStartNode;

	AAINode** ppPath = TNULL;

	while ( !oOpenList.IsEmpty() )
	{
		AStarNode* pCurNode = oOpenList.PopFront();
		TASSERT( TTRUE == pCurNode->node->IsFlagged( AAINode::FLAG_InOpenList ) );
		pCurNode->node->m_uiFlags &= ~AAINode::FLAG_InOpenList;

		// NOTE: a goal without links is never reached (retail checks the links count first)
		if ( pCurNode->node->m_uiLinksCount != 0 )
		{
			if ( pCurNode->node == a_pGoal )
			{
				// Found: the path from the goal back to the start
				if ( a_pCost )
					*a_pCost = pCurNode->pParent->fTotal;

				for ( AStarNode* pNode = pCurNode; pNode != TNULL; pNode = pNode->pParent )
				{
					pNode->node->m_uiFlags &= ~( AAINode::FLAG_InClosedList | AAINode::FLAG_InOpenList );
					TASSERT( a_rPathLength < MAX_PATH_SIZE );
					m_apPath[ a_rPathLength++ ] = pNode->node;
				}

				m_oAStarPool.PushBack( pCurNode );
				pCurNode->node->m_uiFlags &= ~( AAINode::FLAG_InClosedList | AAINode::FLAG_InOpenList );
				ppPath = m_apPath;
				break;
			}

			AAILink** ppLinks    = pCurNode->node->GetLinks();
			TUINT     uiNumLinks = pCurNode->node->m_uiLinksCount;

			for ( TUINT i = 0; i < uiNumLinks; i++ )
			{
				AAINode* pOtherNode = ppLinks[ i ]->GetOtherNode( pCurNode->node );

				// MoveCost
				TFLOAT   fCost = TVector4::Distance( pCurNode->node->m_vPosition, pOtherNode->m_vPosition );
				AAILink* pLink = pCurNode->node->GetLinkTo( pOtherNode );

				if ( a_pEntity && pLink->m_uiMoveType == AAILink::MOVETYPE_Jump )
				{
					if ( TSTATICCAST( APathfindingComponent, a_pEntity->GetPathfinding() )->GetFlags() & APathfindingComponent::FLAG_Jump )
						fCost *= s_fJumpCostFactor;
					else
						fCost += s_fBlockedCost;
				}

				if ( pLink->IsFlagged( AAILink::FLAG_Stale ) && pLink->IsStaleImp() )
					fCost += s_fBlockedCost;

				fCost += pCurNode->fCost;

				if ( fCost > fMaxCost )
					continue;

				AStarNode* pOpenNode = TNULL;

				if ( pOtherNode->IsFlagged( AAINode::FLAG_InOpenList ) )
				{
					TASSERT( TFALSE == pOtherNode->IsFlagged( AAINode::FLAG_InClosedList ) );
					pOpenNode = pOtherNode->m_pAStarNode;

					if ( pOpenNode && pOpenNode->fCost <= fCost )
						continue;
				}

				AStarNode* pClosedNode = TNULL;

				if ( pOtherNode->IsFlagged( AAINode::FLAG_InClosedList ) )
				{
					TASSERT( TFALSE == pOtherNode->IsFlagged( AAINode::FLAG_InOpenList ) );
					pClosedNode = pOtherNode->m_pAStarNode;

					if ( pClosedNode && pClosedNode->fCost <= fCost )
						continue;
				}

				AStarNode* pComparisonNode;

				if ( pOpenNode == TNULL )
				{
					if ( pClosedNode == TNULL )
					{
						if ( m_oAStarPool.IsEmpty() )
							FillAStarPool();

						pComparisonNode          = m_oAStarPool.PopBack();
						pComparisonNode->node    = ppLinks[ i ]->GetOtherNode( pCurNode->node );
						pComparisonNode->pParent = pCurNode;
						pComparisonNode->fCost   = 0.0f;
						pComparisonNode->fGuess  = 0.0f;
						pComparisonNode->fTotal  = 0.0f;
						TASSERT( TFALSE == pComparisonNode->node->IsFlagged( AAINode::FLAG_InOpenList ) );
						TASSERT( TFALSE == pComparisonNode->node->IsFlagged( AAINode::FLAG_InClosedList ) );
					}
					else
					{
						// Reopened
						pClosedNode->pParent = pCurNode;
						pClosedNode->Remove();
						pClosedNode->node->m_uiFlags &= ~AAINode::FLAG_InClosedList;
						pComparisonNode = pClosedNode;
					}
				}
				else
				{
					// NOTE: a better open entry keeps its place in the open list (retail doesn't sort it again)
					pOpenNode->pParent = pCurNode;
					pComparisonNode    = pOpenNode;
				}

				pComparisonNode->fCost  = fCost;
				pComparisonNode->fGuess = AStarHeuristic( pComparisonNode->node, a_pGoal );
				pComparisonNode->fTotal = pComparisonNode->fGuess + pComparisonNode->fCost;

				if ( pOpenNode == TNULL )
				{
					TASSERT( TFALSE == pComparisonNode->node->IsFlagged( AAINode::FLAG_InOpenList ) );
					TASSERT( TFALSE == pComparisonNode->node->IsFlagged( AAINode::FLAG_InClosedList ) );
					oOpenList.Push( pComparisonNode );
					pComparisonNode->node->m_uiFlags |= AAINode::FLAG_InOpenList;
					pComparisonNode->node->m_pAStarNode = pComparisonNode;
				}
			}
		}

		oClosedList.PushBack( pCurNode );
		pCurNode->node->m_uiFlags |= AAINode::FLAG_InClosedList;
		pCurNode->node->m_pAStarNode = pCurNode;
	}

	// The entries go back to the pool
	while ( !oClosedList.IsEmpty() )
	{
		AStarNode* pNode = oClosedList.PopBack();
		pNode->node->m_uiFlags &= ~( AAINode::FLAG_InClosedList | AAINode::FLAG_InOpenList );
		m_oAStarPool.PushBack( pNode );
	}

	while ( !oOpenList.IsEmpty() )
	{
		AStarNode* pNode = oOpenList.PopBack();
		pNode->node->m_uiFlags &= ~( AAINode::FLAG_InClosedList | AAINode::FLAG_InOpenList );
		m_oAStarPool.PushBack( pNode );
	}

	return ppPath;
}

// $deBlob: FUNCTION 0078eb10
void AAINetwork::InitLoad()
{
	m_iLoadState = LOADSTATE_Alloc;
	TASSERT( TNULL == m_pNodeHash );
	m_iLoadIndex = 0;
}

// $deBlob: FUNCTION 0078eb60
TBOOL AAINetwork::Load()
{
	TTRB*   pTRB      = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_Network );
	Header* pHeader   = TSTATICCAST( Header, pTRB->GetSymbolAddress( "Header" ) );
	TUINT32 uiVersion = pHeader->uiVersion;

	THPTimer oTimer;
	oTimer.Reset();
	TFLOAT fElapsed = 0.0f;

	switch ( m_iLoadState )
	{
		case LOADSTATE_Alloc:
			TASSERT( !m_pNodeBlock );
			TASSERT( !m_pLinkBlock );

			m_uiNodeCount = TUINT16( pHeader->iNodeCount );
			m_pNodeBlock  = new AAINode[ m_uiNodeCount ];
			m_uiLinkCount = TUINT16( pHeader->iLinkCount );
			m_pLinkBlock  = new AAILink[ m_uiLinkCount ];
			m_iLoadState  = LOADSTATE_Nodes;
			return TFALSE;
		case LOADSTATE_Nodes:
			do
			{
				LoadNodes( pHeader->pNodes, pHeader->iNodeCount, uiVersion, m_iLoadIndex, TMath::Min( m_iLoadIndex + 1, pHeader->iNodeCount ) );

				if ( ++m_iLoadIndex >= pHeader->iNodeCount )
				{
					m_iLoadState = LOADSTATE_Links;
					m_iLoadIndex = 0;
					return TFALSE;
				}

				oTimer.Update();
				fElapsed += oTimer.GetDelta();
			} while ( oTimer.GetDelta() + fElapsed < s_fLoadTimeSlice );

			return TFALSE;
		case LOADSTATE_Links:
			do
			{
				LoadLinks( pHeader->pLinks, uiVersion, m_iLoadIndex, TMath::Min( m_iLoadIndex + 1, pHeader->iLinkCount ) );

				if ( ++m_iLoadIndex >= pHeader->iLinkCount )
				{
					m_iLoadState = LOADSTATE_CreateHash;
					return TFALSE;
				}

				oTimer.Update();
				fElapsed += oTimer.GetDelta();
			} while ( oTimer.GetDelta() + fElapsed < s_fLoadTimeSlice );

			return TFALSE;
		case LOADSTATE_CreateHash:
			TASSERT( TNULL == m_pNodeHash );
			m_pNodeHash  = new AAINodeHash( -2000.0f, -2000.0f, 2000.0f, 2000.0f, 32, 32 );
			m_iLoadIndex = 0;
			m_iLoadState = LOADSTATE_HashNodes;
			return TFALSE;
		case LOADSTATE_HashNodes:
			while ( m_iLoadIndex < TINT( m_uiNodeCount ) )
			{
				m_pNodeHash->Hash( &m_pNodeBlock[ m_iLoadIndex ] );
				oTimer.Update();
				m_iLoadIndex++;
				fElapsed += oTimer.GetDelta();

				if ( s_fLoadTimeSlice <= oTimer.GetDelta() + fElapsed )
					return TFALSE;
			}

			m_iLoadState = LOADSTATE_Done;
			return TFALSE;
		default:
			return TTRUE;
	}
}

// $deBlob: FUNCTION 0078ef80
void AAINetwork::LoadLinks( LinkData* a_pDataSection, TUINT32 a_uiVersion, TINT a_iStart, TINT a_iEnd )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TASSERT( a_pDataSection[ i ].uiMoveType < AAILink::MOVETYPE_Count );
		TASSERT( a_pDataSection[ i ].iNodeA >= 0 && a_pDataSection[ i ].iNodeA <= m_uiNodeCount );
		TASSERT( a_pDataSection[ i ].iNodeB >= 0 && a_pDataSection[ i ].iNodeB <= m_uiNodeCount );

		AAILink* pLink      = &m_pLinkBlock[ i ];
		pLink->m_pNodeA     = &m_pNodeBlock[ a_pDataSection[ i ].iNodeA ];
		pLink->m_pNodeB     = &m_pNodeBlock[ a_pDataSection[ i ].iNodeB ];
		pLink->m_uiFlags    = 0;
		pLink->m_uiMoveType = TUINT8( a_pDataSection[ i ].uiMoveType );
		pLink->m_pNodeA->AddLink( pLink );
		pLink->m_pNodeB->AddLink( pLink );
	}
}

// $deBlob: FUNCTION 0078f0e0
void AAINetwork::LoadNodes( NodeData* a_pDataSection, TINT a_iCount, TUINT32 a_uiVersion, TINT a_iStart, TINT a_iEnd )
{
	TUINT16 uiVersion = TUINT16( a_uiVersion >> 16 );
	TUINT8  uiFlags   = TUINT8( a_uiVersion );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TUINT32 iID          = a_pDataSection[ i ].uiID;
		TUINT8  uiLinksCount = s_uiDefaultLinksCount;

		// Version 1 with the flag and later versions give each node its links count
		if ( ( uiVersion == 1 && uiFlags != 0 ) || uiVersion > 1 )
			uiLinksCount = a_pDataSection[ i ].uiLinksCount;

		TASSERT( iID < TUINT32( a_iCount ) );
		AAINode* pNode = &m_pNodeBlock[ iID ];
		pNode->Initialise( uiLinksCount );
		pNode->m_vPosition   = a_pDataSection[ i ].vPosition;
		pNode->m_uiSegmentID = a_pDataSection[ i ].uiSegmentID;
	}
}

// $deBlob: FUNCTION 0078f1e0
AStarNode* AAINetwork::FindNodeInList( AAINode* a_pNode, T2DList<AStarNode>& a_rList )
{
	for ( auto it = a_rList.Begin(); it != a_rList.End(); it++ )
	{
		if ( it->node == a_pNode )
			return it.Get();
	}

	return TNULL;
}

// $deBlob: FUNCTION 0078f260
TFLOAT AAINetwork::MoveCost( AAINode* a_pFrom, AAINode* a_pTo, ABaseEntity* a_pEntity )
{
	TFLOAT   fCost = TVector4::Distance( a_pFrom->m_vPosition, a_pTo->m_vPosition );
	AAILink* pLink = a_pFrom->GetLinkTo( a_pTo );

	if ( a_pEntity && pLink->m_uiMoveType == AAILink::MOVETYPE_Jump )
	{
		if ( TSTATICCAST( APathfindingComponent, a_pEntity->GetPathfinding() )->GetFlags() & APathfindingComponent::FLAG_Jump )
			fCost *= s_fJumpCostFactor;
		else
			fCost += s_fBlockedCost;
	}

	if ( pLink->IsFlagged( AAILink::FLAG_Stale ) && pLink->IsStaleImp() )
		return fCost + s_fBlockedCost;

	return fCost;
}

// $deBlob: FUNCTION 0078f300
TFLOAT AAINetwork::AStarHeuristic( AAINode* a_pNode, AAINode* a_pGoal )
{
	return TVector4::Distance( a_pNode->m_vPosition, a_pGoal->m_vPosition );
}

// $deBlob: FUNCTION 0078f310
void AAINetwork::GetBounds( TFLOAT& a_rfMinX, TFLOAT& a_rfMinZ, TFLOAT& a_rfMaxX, TFLOAT& a_rfMaxZ )
{
	a_rfMaxX = -100000.0f;
	a_rfMaxZ = -100000.0f;
	a_rfMinX = TMath::MAXFLOAT;
	a_rfMinZ = TMath::MAXFLOAT;

	for ( TINT i = 0; i < m_uiNodeCount; i++ )
	{
		const TVector4& rvPosition = m_pNodeBlock[ i ].m_vPosition;
		a_rfMinX                   = TMath::Min( a_rfMinX, rvPosition.x );
		a_rfMinZ                   = TMath::Min( a_rfMinZ, rvPosition.z );
		a_rfMaxX                   = TMath::Max( a_rfMaxX, rvPosition.x );
		a_rfMaxZ                   = TMath::Max( a_rfMaxZ, rvPosition.z );
	}
}
