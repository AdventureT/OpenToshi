#include "ToshiPCH.h"
#include "TBoxTree.h"
#include "TRenderContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TSTATICASSERT( sizeof( TBoxTree ) == 0x38 );
TSTATICASSERT( sizeof( TBoxTree::Node ) == 0x24 );

// The box containing a_rA and a_rB (TAABox)
// $deBlob: FUNCTION 0068bf10
static void BoxUnion( TBoxTree::Box& a_rOut, const TBoxTree::Box& a_rA, const TBoxTree::Box& a_rB )
{
	TVector3 vMin;
	TVector3 vMax;

	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fMinA = ( &a_rA.vCentre.x )[ i ] - ( &a_rA.vExtents.x )[ i ];
		TFLOAT fMinB = ( &a_rB.vCentre.x )[ i ] - ( &a_rB.vExtents.x )[ i ];
		TFLOAT fMaxA = ( &a_rA.vExtents.x )[ i ] + ( &a_rA.vCentre.x )[ i ];
		TFLOAT fMaxB = ( &a_rB.vCentre.x )[ i ] + ( &a_rB.vExtents.x )[ i ];

		( &vMin.x )[ i ] = ( fMinB <= fMinA ) ? fMinB : fMinA;
		( &vMax.x )[ i ] = ( fMaxA <= fMaxB ) ? fMaxB : fMaxA;
	}

	for ( TINT i = 0; i < 3; i++ )
	{
		( &a_rOut.vCentre.x )[ i ]  = ( ( &vMax.x )[ i ] + ( &vMin.x )[ i ] ) * 0.5f;
		( &a_rOut.vExtents.x )[ i ] = ( &vMax.x )[ i ] - ( &a_rOut.vCentre.x )[ i ];
	}
}

// TRUE if the box intersects the sphere (TAABox)
// $deBlob: FUNCTION 0068bb80
static TBOOL BoxIntersectsSphere( const TBoxTree::Box& a_rBox, const TSphere& a_rSphere )
{
	TFLOAT afDist[ 3 ];

	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fMin   = ( &a_rBox.vCentre.x )[ i ] - ( &a_rBox.vExtents.x )[ i ];
		TFLOAT fMax   = ( &a_rBox.vCentre.x )[ i ] + ( &a_rBox.vExtents.x )[ i ];
		TFLOAT fValue = ( &a_rSphere.GetOrigin().x )[ i ];

		if ( fMin < fValue )
			afDist[ i ] = ( fValue < fMax ) ? 0.0f : fValue - fMax;
		else
			afDist[ i ] = fValue - fMin;
	}

	return afDist[ 0 ] * afDist[ 0 ] + afDist[ 1 ] * afDist[ 1 ] + afDist[ 2 ] * afDist[ 2 ] <= a_rSphere.GetRadius() * a_rSphere.GetRadius();
}

// $deBlob: FUNCTION 006b2bd0
TBoxTree::TBoxTree()
{
	m_vViewPos          = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_pPlanes           = TNULL;
	m_pSphere           = TNULL;
	m_uiFlags           = 0;
	m_pUserData         = TNULL;
	m_pItems            = TNULL;
	m_pRenderFunction   = TNULL;
	m_pMakeAABBFunction = TNULL;
	m_pRoot             = TNULL;
	m_bBuilt            = TFALSE;
}

// $deBlob: FUNCTION 006b2c20
// $deBlob: FUNCTION 006251d0
TBoxTree::~TBoxTree()
{
	if ( m_bBuilt && m_pItems != TNULL )
	{
		Node* pRoot = m_pItems->pNode;

		if ( pRoot->pLeft != TNULL )
		{
			DeleteNode( pRoot->pLeft );
			DeleteNode( pRoot->pRight );
		}

		delete pRoot;
		delete m_pItems;
	}
}

// $deBlob: FUNCTION 006b2c90
void TBoxTree::AddListItem( ListItem* a_pItem )
{
	a_pItem->pPrev = TNULL;
	a_pItem->pNext = m_pItems;

	if ( m_pItems != TNULL )
		m_pItems->pPrev = a_pItem;

	m_pItems = a_pItem;
}

// $deBlob: FUNCTION 006b2cc0
void TBoxTree::AddItem( void* a_pItem )
{
	TASSERT( m_pMakeAABBFunction );

	ListItem* pListItem = new ListItem;
	Node*     pNode     = new Node;

	pNode->pItem = a_pItem;

	Box oBox;
	pNode->oBox   = *m_pMakeAABBFunction( &oBox, a_pItem );
	pNode->pLeft  = TNULL;
	pNode->pRight = TNULL;

	pListItem->pNode = pNode;
	AddListItem( pListItem );
}

// $deBlob: FUNCTION 006b2d90
void TBoxTree::RemoveListItem( ListItem* a_pItem )
{
	if ( a_pItem->pPrev == TNULL )
	{
		a_pItem->pNext->pPrev = TNULL;
		m_pItems              = a_pItem->pNext;
	}
	else if ( a_pItem->pNext == TNULL )
	{
		a_pItem->pPrev->pNext = TNULL;
	}
	else
	{
		a_pItem->pPrev->pNext = a_pItem->pNext;
		a_pItem->pNext->pPrev = a_pItem->pPrev;
	}

	a_pItem->pNext = TNULL;
	a_pItem->pPrev = TNULL;
	delete a_pItem;
}

// $deBlob: FUNCTION 006b2df0
void TBoxTree::Build()
{
	while ( m_pItems->pNext != TNULL )
	{
		TFLOAT    fBestVolume = TMath::MAXFLOAT;
		ListItem* pBestA      = TNULL;
		ListItem* pBestB      = TNULL;

		for ( ListItem* pA = m_pItems; pA != TNULL; pA = pA->pNext )
		{
			for ( ListItem* pB = pA->pNext; pB != TNULL; pB = pB->pNext )
			{
				Box oUnion;
				BoxUnion( oUnion, pA->pNode->oBox, pB->pNode->oBox );

				TFLOAT fVolume = oUnion.vExtents.x * oUnion.vExtents.y * oUnion.vExtents.z;

				if ( fVolume < fBestVolume )
				{
					pBestA      = pA;
					fBestVolume = fVolume;
					pBestB      = pB;
				}
			}
		}

		Node* pNode = new Node;
		BoxUnion( pNode->oBox, pBestA->pNode->oBox, pBestB->pNode->oBox );
		pNode->pLeft  = pBestA->pNode;
		pNode->pRight = pBestB->pNode;

		ListItem* pListItem = new ListItem;
		pListItem->pNode    = pNode;
		AddListItem( pListItem );

		RemoveListItem( pBestA );
		RemoveListItem( pBestB );
	}

	m_bBuilt = TTRUE;
	m_pRoot  = m_pItems->pNode;
}

// $deBlob: FUNCTION 006b2fb0
void TBoxTree::Render()
{
	if ( m_pRoot != TNULL )
	{
		TASSERT( m_pRenderFunction );

		if ( m_uiFlags & Flags_Sphere )
		{
			TASSERT( m_pSphere );
			RenderSphere( m_pRoot );
			return;
		}

		TASSERT( m_pPlanes );
		RenderPlanes( m_pRoot, 0x3F );
	}
}

// $deBlob: FUNCTION 006b3080
void TBoxTree::RenderPlanes( Node* a_pNode, TUINT a_uiClipFlags )
{
	while ( TTRUE )
	{
		TASSERT( m_pPlanes );
		TASSERT( m_pRenderFunction );

		TINT iClipFlags = 0;

		if ( a_uiClipFlags != 0 )
		{
			iClipFlags = TRenderContext::CullAABoxToFrustum( &a_pNode->oBox.vCentre, m_pPlanes, a_uiClipFlags, -1 );

			if ( iClipFlags == -1 )
				return;
		}

		Node* pLeft = a_pNode->pLeft;

		if ( pLeft == TNULL )
		{
			m_pRenderFunction( a_pNode->pItem, m_pUserData );
			return;
		}

		a_uiClipFlags = iClipFlags;

		if ( !( m_uiFlags & Flags_SortByDistance ) )
		{
			RenderPlanes( pLeft, iClipFlags );
			a_pNode = a_pNode->pRight;
		}
		else
		{
			Node* pRight = a_pNode->pRight;

			TFLOAT fLeftX  = m_vViewPos.x - pLeft->oBox.vCentre.x;
			TFLOAT fLeftY  = m_vViewPos.y - pLeft->oBox.vCentre.y;
			TFLOAT fLeftZ  = m_vViewPos.z - pLeft->oBox.vCentre.z;
			TFLOAT fRightY = m_vViewPos.y - pRight->oBox.vCentre.y;
			TFLOAT fRightX = m_vViewPos.x - pRight->oBox.vCentre.x;
			TFLOAT fRightZ = m_vViewPos.z - pRight->oBox.vCentre.z;

			if ( fRightY * fRightY + fRightX * fRightX + fRightZ * fRightZ <= fLeftY * fLeftY + fLeftX * fLeftX + fLeftZ * fLeftZ )
			{
				RenderPlanes( pRight, iClipFlags );
				a_pNode = a_pNode->pLeft;
			}
			else
			{
				RenderPlanes( pLeft, iClipFlags );
				a_pNode = a_pNode->pRight;
			}
		}
	}
}

// $deBlob: FUNCTION 006b31f0
void TBoxTree::RenderSphere( Node* a_pNode )
{
	while ( TTRUE )
	{
		TASSERT( m_pSphere );
		TASSERT( m_pRenderFunction );

		if ( !BoxIntersectsSphere( a_pNode->oBox, *m_pSphere ) )
			return;

		if ( a_pNode->pLeft == TNULL )
		{
			m_pRenderFunction( a_pNode->pItem, m_pUserData );
			return;
		}

		RenderSphere( a_pNode->pLeft );
		a_pNode = a_pNode->pRight;
	}
}

// $deBlob: FUNCTION 006b32a0
void TBoxTree::DeleteNode( Node* a_pNode )
{
	if ( a_pNode->pLeft != TNULL )
	{
		DeleteNode( a_pNode->pLeft );
		DeleteNode( a_pNode->pRight );
	}

	delete a_pNode;
}

// The side of the plane the box is on
// $deBlob: FUNCTION 0068bae0
TINT TBoxTree::Box::ClassifyPlane( const TPlane& a_rcPlane ) const
{
	const TVector3& rcNormal = a_rcPlane.GetNormal();
	TVector3        vAbsNormal;
	vAbsNormal.Abs( rcNormal );

	TFLOAT fDistance = rcNormal.y * vCentre.y + rcNormal.x * vCentre.x + rcNormal.z * vCentre.z - a_rcPlane.GetD();
	TFLOAT fRadius   = vExtents.y * vAbsNormal.y + vExtents.x * vAbsNormal.x + vExtents.z * vAbsNormal.z;

	if ( fRadius + fDistance >= 0.0f )
		return ( fDistance - fRadius > 0.0f ) ? PlaneSide_Front : PlaneSide_Crossing;

	return PlaneSide_Behind;
}

// $deBlob: FUNCTION 0068bc70
TBOOL TBoxTree::Box::IsPointInside( const TVector3& a_rcPoint ) const
{
	return a_rcPoint.x >= vCentre.x - vExtents.x && a_rcPoint.y >= vCentre.y - vExtents.y && a_rcPoint.z >= vCentre.z - vExtents.z &&
	    vExtents.x + vCentre.x >= a_rcPoint.x && vExtents.y + vCentre.y >= a_rcPoint.y && vExtents.z + vCentre.z >= a_rcPoint.z;
}

// The box around a_pBoxes
// $deBlob: FUNCTION 0068bcf0
void TBoxTree::Box::SetUnion( const Box* a_pBoxes, TUINT a_uiNumBoxes )
{
	if ( a_pBoxes == TNULL || a_uiNumBoxes == 0 )
		return;

	TFLOAT fMinX = a_pBoxes[ 0 ].vCentre.x - a_pBoxes[ 0 ].vExtents.x;
	TFLOAT fMinY = a_pBoxes[ 0 ].vCentre.y - a_pBoxes[ 0 ].vExtents.y;
	TFLOAT fMinZ = a_pBoxes[ 0 ].vCentre.z - a_pBoxes[ 0 ].vExtents.z;
	TFLOAT fMaxX = a_pBoxes[ 0 ].vExtents.x + a_pBoxes[ 0 ].vCentre.x;
	TFLOAT fMaxY = a_pBoxes[ 0 ].vExtents.y + a_pBoxes[ 0 ].vCentre.y;
	TFLOAT fMaxZ = a_pBoxes[ 0 ].vExtents.z + a_pBoxes[ 0 ].vCentre.z;

	for ( TUINT i = 1; i < a_uiNumBoxes; i++ )
	{
		const Box& rcBox = a_pBoxes[ i ];

		TFLOAT fBoxMinX = rcBox.vCentre.x - rcBox.vExtents.x;
		TFLOAT fBoxMinY = rcBox.vCentre.y - rcBox.vExtents.y;
		TFLOAT fBoxMinZ = rcBox.vCentre.z - rcBox.vExtents.z;
		TFLOAT fBoxMaxX = rcBox.vCentre.x + rcBox.vExtents.x;
		TFLOAT fBoxMaxY = rcBox.vCentre.y + rcBox.vExtents.y;
		TFLOAT fBoxMaxZ = rcBox.vCentre.z + rcBox.vExtents.z;

		if ( fBoxMinX <= fMinX ) fMinX = fBoxMinX;
		if ( fBoxMinY <= fMinY ) fMinY = fBoxMinY;
		if ( fBoxMinZ <= fMinZ ) fMinZ = fBoxMinZ;
		if ( fMaxX <= fBoxMaxX ) fMaxX = fBoxMaxX;
		if ( fMaxY <= fBoxMaxY ) fMaxY = fBoxMaxY;
		if ( fMaxZ <= fBoxMaxZ ) fMaxZ = fBoxMaxZ;
	}

	vCentre.x  = ( fMaxX + fMinX ) * 0.5f;
	vCentre.y  = ( fMaxY + fMinY ) * 0.5f;
	vCentre.z  = ( fMaxZ + fMinZ ) * 0.5f;
	vExtents.x = fMaxX - vCentre.x;
	vExtents.y = fMaxY - vCentre.y;
	vExtents.z = fMaxZ - vCentre.z;
}

// Where the box is after a_rcMatrix (the clip space test of its corners)
// $deBlob: FUNCTION 0068c0e0
TINT TBoxTree::Box::ClassifyClip( const TMatrix44& a_rcMatrix ) const
{
	TFLOAT afMin[ 3 ] = { vCentre.x - vExtents.x, vCentre.y - vExtents.y, vCentre.z - vExtents.z };
	TFLOAT afMax[ 3 ] = { vExtents.x + vCentre.x, vExtents.y + vCentre.y, vExtents.z + vCentre.z };

	TUINT uiOr  = 0;
	TUINT uiAnd = 0xFFFF;

	for ( TINT i = 0; i < 8; i++ )
	{
		TVector4 vCorner( ( i & 1 ) ? afMin[ 0 ] : afMax[ 0 ], ( i & 2 ) ? afMin[ 1 ] : afMax[ 1 ], ( i & 4 ) ? afMin[ 2 ] : afMax[ 2 ], 1.0f );
		TVector4 vClip;
		TMatrix44::TransformVector( vClip, a_rcMatrix, vCorner );

		TUINT uiCode = 0;

		if ( !( -vClip.w <= vClip.x ) ) uiCode = ClipCode_XMin;
		else if ( vClip.x > vClip.w ) uiCode = ClipCode_XMax;

		if ( !( -vClip.w <= vClip.y ) ) uiCode |= ClipCode_YMin;
		else if ( vClip.y > vClip.w ) uiCode |= ClipCode_YMax;

		if ( !( -vClip.w <= vClip.z ) ) uiCode |= ClipCode_ZMin;
		else if ( vClip.z > vClip.w ) uiCode |= ClipCode_ZMax;

		uiOr |= uiCode;
		uiAnd &= uiCode;
	}

	if ( uiOr == 0 )
		return ClipResult_Inside;

	if ( uiAnd != 0 )
		return ClipResult_Outside;

	return ClipResult_Crossing;
}

TOSHI_NAMESPACE_END
