#include "pch.h"
#include "A2DHash.h"
#include "Terrain2/ATerrain2.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUIRenderer.h>

#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The number of buckets the cells are hashed into (the index is masked with 0x3FF)
static TCONSTEXPR_VAR TUINT32 A2DHASH_BUCKET_COUNT = 0x400;

TSTATICASSERT( sizeof( A2DHashRef ) == 0x14 );
TSTATICASSERT( sizeof( A2DHashBucket ) == 0x18 );
TSTATICASSERT( sizeof( A2DHash ) == 0x5C );

TUINT32 A2DHash::ms_uiGeneration = 0;

//-----------------------------------------------------------------------------
// Purpose: whether the reference is in a list
//-----------------------------------------------------------------------------
// $deBlob: FUNCTION 0056a8a0
static TBOOL IsRefLinked( const A2DHashRef* a_pRef )
{
	return ( a_pRef->m_pNext != a_pRef ) && ( a_pRef->m_pPrev != a_pRef );
}

//-----------------------------------------------------------------------------
// Purpose: links the reference after a_pRoot
//-----------------------------------------------------------------------------
// $deBlob: FUNCTION 0056a840
static void LinkRefAfter( A2DHashRef* a_pRef, A2DHashRef* a_pRoot )
{
	TASSERT( !IsRefLinked( a_pRef ) );
	a_pRef->m_pPrev          = a_pRoot;
	a_pRef->m_pNext          = a_pRoot->m_pNext;
	a_pRef->m_pNext->m_pPrev = a_pRef;
	a_pRef->m_pPrev->m_pNext = a_pRef;
}

//-----------------------------------------------------------------------------
// Purpose: takes the reference out of its list
//-----------------------------------------------------------------------------
// $deBlob: FUNCTION 0056a930
static void UnlinkRef( A2DHashRef* a_pRef )
{
	TASSERT( IsRefLinked( a_pRef ) );
	a_pRef->m_pNext->m_pPrev = a_pRef->m_pPrev;
	a_pRef->m_pPrev->m_pNext = a_pRef->m_pNext;
	a_pRef->m_pPrev          = a_pRef;
	a_pRef->m_pNext          = a_pRef;
}

// $deBlob: FUNCTION 00569770
A2DHash::A2DHash()
{
	m_pfnCallback    = TNULL;
	m_uiMaxResults   = 0;
	m_uiNumRefs      = 0;
	m_uiNumBuckets   = 0;
	m_fXMax          = 0.0f;
	m_fYMax          = 0.0f;
	m_iXDiv          = 0;
	m_iYDiv          = 0;
	m_fXOffset       = 0.0f;
	m_fYOffset       = 0.0f;
	m_fCellWidth     = 0.0f;
	m_fCellHeight    = 0.0f;
	m_fInvCellWidth  = 0.0f;
	m_fInvCellHeight = 0.0f;
	m_paBuckets      = TNULL;
	m_paResults      = TNULL;
	m_paRefs         = TNULL;

	// The free list is empty: its sentinel points to itself
	m_FreeListRoot.m_pItem   = TNULL;
	m_FreeListRoot.m_pChain  = TNULL;
	m_FreeListRoot.m_pNext   = &m_FreeListRoot;
	m_FreeListRoot.m_pPrev   = &m_FreeListRoot;
	m_FreeListRoot.m_pBucket = TNULL;
}

// $deBlob: FUNCTION 0056a660
// $deBlob: FUNCTION 00569800
A2DHash::~A2DHash()
{
	TFree( m_paBuckets );
	TFree( m_paResults );
	TFree( m_paRefs );
}

// $deBlob: FUNCTION 00569840
void A2DHash::Init( TMemoryHeap* a_pHeap, Callback a_pfnCallback, TFLOAT a_fMinX, TFLOAT a_fMinY, TFLOAT a_fMaxX, TFLOAT a_fMaxY, TINT a_iXDiv, TINT a_iYDiv, TUINT32 a_uiNumResults, TUINT32 a_uiNumRefs, TUINT32 a_uiNumBuckets )
{
	m_uiNumBuckets = a_uiNumBuckets;
	m_pfnCallback  = a_pfnCallback;
	m_uiNumRefs    = a_uiNumRefs;
	m_uiMaxResults = a_uiNumResults;

	m_paResults = TREINTERPRETCAST( Result*, TMalloc( a_uiNumResults * sizeof( Result ), a_pHeap ) );

	// The refs of the pool, all unlinked
	m_paRefs = TREINTERPRETCAST( A2DHashRef*, TMalloc( m_uiNumRefs * sizeof( A2DHashRef ), a_pHeap ) );

	for ( TUINT32 i = 0; i < m_uiNumRefs; i++ )
	{
		A2DHashRef* pRef = &m_paRefs[ i ];

		pRef->m_pItem   = TNULL;
		pRef->m_pChain  = TNULL;
		pRef->m_pNext   = pRef;
		pRef->m_pPrev   = pRef;
		pRef->m_pBucket = TNULL;
	}

	// The buckets, all empty (each is the sentinel of its own list)
	m_paBuckets = TREINTERPRETCAST( A2DHashBucket*, TMalloc( m_uiNumBuckets * sizeof( A2DHashBucket ), a_pHeap ) );

	for ( TUINT32 i = 0; i < m_uiNumBuckets; i++ )
	{
		A2DHashBucket* pBucket = &m_paBuckets[ i ];

		pBucket->m_uiUnk00 = 0;
		pBucket->m_uiUnk04 = 0;
		pBucket->m_pNext   = TREINTERPRETCAST( A2DHashRef*, pBucket );
		pBucket->m_pPrev   = TREINTERPRETCAST( A2DHashRef*, pBucket );
		pBucket->m_uiUnk10 = 0;
		pBucket->m_uiStamp = 0;
	}

	// Every ref goes to the free list
	for ( TUINT32 i = 0; i < m_uiNumRefs; i++ )
	{
		A2DHashRef* pRef = &m_paRefs[ i ];

		LinkRefAfter( pRef, &m_FreeListRoot );
	}

	m_fXMax = a_fMaxX;
	m_fYMax = a_fMaxY;
	m_iXDiv = a_iXDiv;
	m_iYDiv = a_iYDiv;

	TASSERT( m_fXMax != 0.0f );
	TASSERT( m_fYMax != 0.0f );
	TASSERT( m_iXDiv != 0 );
	TASSERT( m_iYDiv != 0 );
	TASSERT( a_fMaxX - a_fMinX >= 0.0f );
	TASSERT( a_fMaxY - a_fMinY >= 0.0f );

	m_fCellWidth     = ( a_fMaxX - a_fMinX ) / TFLOAT( m_iXDiv );
	m_fCellHeight    = ( a_fMaxY - a_fMinY ) / TFLOAT( m_iYDiv );
	m_fInvCellWidth  = 1.0f / m_fCellWidth;
	m_fInvCellHeight = 1.0f / m_fCellHeight;

	if ( a_fMinX < 0.0f )
		m_fXOffset = -a_fMinX;

	if ( a_fMinY < 0.0f )
		m_fYOffset = -a_fMinY;
}

// $deBlob: FUNCTION 00569cc0
A2DHashRef* A2DHash::Insert( A2DHashItem* a_pItem )
{
	Remove( a_pItem );

	const Toshi::TVector4* pPos    = a_pItem->GetPosition();
	TFLOAT                 fRadius = a_pItem->GetRadius();

	TINT iX0 = GetCell( ( pPos->x - fRadius + m_fXOffset ) * m_fInvCellWidth );
	TINT iX1 = GetCell( ( pPos->x + fRadius + m_fXOffset ) * m_fInvCellWidth );
	TINT iY0 = GetCell( ( pPos->z - fRadius + m_fYOffset ) * m_fInvCellHeight );
	TINT iY1 = GetCell( ( pPos->z + fRadius + m_fYOffset ) * m_fInvCellHeight );

	A2DHashRef* pHead = TNULL;

	for ( TINT iY = iY0; iY <= iY1; iY++ )
	{
		for ( TINT iX = iX0; iX <= iX1; iX++ )
		{
			TUINT32 uiIndex = TUINT32( iY * 0x41 + iX * -0xbd ) & 0x3FF;
			TASSERT( uiIndex < A2DHASH_BUCKET_COUNT );

			A2DHashBucket* pBucket = &m_paBuckets[ uiIndex ];

			// The cell is only added once per item
			TBOOL bDuplicate = TFALSE;

			for ( A2DHashRef* pRef = pHead; pRef != TNULL; pRef = pRef->m_pChain )
			{
				if ( pRef->m_pBucket == pBucket )
				{
					bDuplicate = TTRUE;
					break;
				}
			}

			if ( bDuplicate )
				continue;

			TASSERT( m_FreeListRoot.m_pNext != &m_FreeListRoot, "2D hash ref pool empty" );

			A2DHashRef* pRef = m_FreeListRoot.m_pNext;

			// Takes the ref off the free list
			UnlinkRef( pRef );

			// Puts it in the bucket, after the sentinel
			LinkRefAfter( pRef, TREINTERPRETCAST( A2DHashRef*, pBucket ) );

			pRef->m_pChain  = pHead;
			pRef->m_pItem   = a_pItem;
			pRef->m_pBucket = pBucket;
			pHead           = pRef;
		}
	}

	TASSERT( a_pItem->m_pHashRef == TNULL );

	a_pItem->m_pHashRef = pHead;
	return pHead;
}

// $deBlob: FUNCTION 0056a000
void A2DHash::Remove( A2DHashItem* a_pItem )
{
	A2DHashRef* pRef = a_pItem->m_pHashRef;

	while ( pRef != TNULL )
	{
		A2DHashRef* pNextRef = pRef->m_pChain;

		// Takes the ref out of its bucket
		UnlinkRef( pRef );

		// Puts it at the front of the free list
		LinkRefAfter( pRef, &m_FreeListRoot );

		a_pItem->m_pHashRef = TNULL;
		pRef                = pNextRef;
	}
}

// $deBlob: FUNCTION 0056a100
A2DHash::Result* A2DHash::Query( const Toshi::TVector4& a_rCenter, const Toshi::TVector4& a_rDirection, TFLOAT a_fRadius, TINT* a_piCount, TINT a_iMax )
{
	TASSERT( a_fRadius > 0.0f && a_fRadius <= 1000.0f, "Incorrect radius!" );

	// A new generation: the stamps of the items and buckets of the previous query are stale. The callback runs on wrap
	TUINT32 uiNextGeneration = ms_uiGeneration + 1;
	TBOOL   bWrapped         = uiNextGeneration < ms_uiGeneration;

	ms_uiGeneration = uiNextGeneration;

	if ( bWrapped )
	{
		ms_uiGeneration = 0;
		m_pfnCallback( 0 );
	}

	*a_piCount = 0;

	TINT iX0 = GetCell( ( a_rCenter.x - a_fRadius + m_fXOffset ) * m_fInvCellWidth );
	TINT iX1 = GetCell( ( a_rCenter.x + a_fRadius + m_fXOffset ) * m_fInvCellWidth );
	TINT iY0 = GetCell( ( a_rCenter.z - a_fRadius + m_fYOffset ) * m_fInvCellHeight );
	TINT iY1 = GetCell( ( a_rCenter.z + a_fRadius + m_fYOffset ) * m_fInvCellHeight );

	for ( TINT iY = iY0; iY <= iY1; iY++ )
	{
		for ( TINT iX = iX0; iX <= iX1; iX++ )
		{
			TUINT32        uiIndex = TUINT32( iY * 0x41 + iX * -0xbd ) & 0x3FF;
			A2DHashBucket* pBucket = &m_paBuckets[ uiIndex ];

			if ( pBucket->m_uiStamp == ms_uiGeneration )
				continue;

			pBucket->m_uiStamp = ms_uiGeneration;

			// Only the buckets with items
			if ( pBucket->m_pNext == TREINTERPRETCAST( A2DHashRef*, pBucket ) || pBucket->m_pPrev == TREINTERPRETCAST( A2DHashRef*, pBucket ) )
				continue;

			if ( !QueryBucket( pBucket, a_rCenter, a_rDirection, a_fRadius, a_piCount, a_iMax ) )
				return m_paResults;
		}
	}

	return m_paResults;
}

// $deBlob: FUNCTION 0056a440
TBOOL A2DHash::QueryBucket( A2DHashBucket* a_pBucket, const Toshi::TVector4& a_rCenter, const Toshi::TVector4& a_rDirection, TFLOAT a_fRadius, TINT* a_piCount, TINT a_iMax )
{
	A2DHashRef* pSentinel = TREINTERPRETCAST( A2DHashRef*, a_pBucket );

	for ( A2DHashRef* pRef = a_pBucket->m_pNext; pRef != pSentinel; pRef = pRef->m_pNext )
	{
		A2DHashItem* pItem = pRef->m_pItem;

		if ( pItem->m_uiStamp == ms_uiGeneration )
			continue;

		pItem->m_uiStamp = ms_uiGeneration;

		const Toshi::TVector4* pPos    = pItem->GetPosition();
		TFLOAT                 fRadius = pItem->GetRadius();

		TFLOAT fMaxDistance = ( fRadius + a_fRadius ) * ( fRadius + a_fRadius );
		TFLOAT fDistanceSq  = Toshi::TVector4::DistanceSq( *pPos, a_rCenter );

		// Only the items in front of the direction
		TFLOAT fDot = a_rDirection.y * ( pPos->y - a_rCenter.y ) + a_rDirection.x * ( pPos->x - a_rCenter.x ) + a_rDirection.z * ( pPos->z - a_rCenter.z );

		if ( fDistanceSq < fMaxDistance && 0.0f <= fDot )
		{
			m_paResults[ *a_piCount ].m_pItem     = pItem;
			m_paResults[ *a_piCount ].m_fDistance = fMaxDistance - fRadius * fRadius;

			( *a_piCount )++;

			if ( *a_piCount == a_iMax )
				return TFALSE;
		}
	}

	return TTRUE;
}

// Purpose: the cell of a scaled coordinate (negative values are rounded down)
TINT A2DHash::GetCell( TFLOAT a_fValue )
{
	if ( a_fValue < 0.0f )
		a_fValue -= 1.0f;

	return TINT( std::lrintf( a_fValue ) );
}

// $deBlob: FUNCTION 0056a7a0
TINT A2DHash::GetCellX( TFLOAT a_fX ) const
{
	return GetCell( m_fInvCellWidth * a_fX );
}

// $deBlob: FUNCTION 0056a7f0
TINT A2DHash::GetCellY( TFLOAT a_fY ) const
{
	return GetCell( m_fInvCellHeight * a_fY );
}

// An ARGB colour from 0-1 components
// $deBlob: FUNCTION 0056a570
static TUINT32 MakeColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha )
{
	return TUINT32( TINT( a_fBlue * 255.0f ) ) |
	    ( ( TUINT32( TINT( a_fGreen * 255.0f ) ) | ( ( ( TUINT32( TINT( a_fAlpha * 255.0f ) ) << 8 ) | TUINT32( TINT( a_fRed * 255.0f ) ) ) << 8 ) ) << 8 );
}

// $deBlob: FUNCTION 0056a3a0
void A2DHash::DebugRender()
{
	T2GUIRenderer* pRenderer = T2GUI::GetSingleton()->GetRenderer();
	TINT           iX        = -512;

	for ( TUINT32 i = 0; i < A2DHASH_BUCKET_COUNT; i++ )
	{
		A2DHashBucket* pBucket = &m_paBuckets[ i ];

		TINT iNumRefs = 0;
		for ( A2DHashRef* pRef = pBucket->m_pNext; pRef != TREINTERPRETCAST( A2DHashRef*, pBucket ); pRef = pRef->m_pNext )
			iNumRefs++;

		pRenderer->SetColour( 0x7FFF0000 );
		pRenderer->RenderLine( TFLOAT( iX ), 0.0f, TFLOAT( iX ), TFLOAT( -iNumRefs ) );
		iX++;
	}
}

//-----------------------------------------------------------------------------
// Purpose: the bounds of an A2DHash over the level (ASpawnedCivManager, AEnvParticles): the region 0 of the terrain (its first corner, 150 units per cell, with one
// extra cell on each side) and its number of cells of a_fCellSize.
//-----------------------------------------------------------------------------
// $deBlob: FUNCTION 00569bc0
void A2DHash::GetLevelBounds( TFLOAT a_fCellSize, TFLOAT* a_pfMinX, TFLOAT* a_pfMinY, TFLOAT* a_pfMaxX, TFLOAT* a_pfMaxY, TINT* a_piXDiv, TINT* a_piYDiv )
{
	TerrainTypes::RegionInfo* pInfo = ATerrain2::GetInstance()->GetRegion( 0 )->m_pInfo;

	*a_pfMinX = pInfo->m_fX - 150.0f;
	*a_pfMinY = pInfo->m_fZ - 150.0f;
	*a_pfMaxX = TFLOAT( pInfo->m_uiSizeX + 2 ) * 150.0f + *a_pfMinX;
	*a_pfMaxY = TFLOAT( pInfo->m_uiSizeZ + 2 ) * 150.0f + *a_pfMinY;

	*a_piXDiv = TINT( std::lrintf( ( *a_pfMaxX - *a_pfMinX ) / a_fCellSize ) );
	*a_piYDiv = TINT( std::lrintf( ( *a_pfMaxY - *a_pfMinY ) / a_fCellSize ) );
}
