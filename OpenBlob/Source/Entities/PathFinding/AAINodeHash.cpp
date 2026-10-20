#include "pch.h"
#include "AAINodeHash.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AAINodeHash ) == 0x4FBC );

// $deBlob: FUNCTION 007e0200
AAINodeHash::AAINodeHash( TFLOAT a_fMinX, TFLOAT a_fMinY, TFLOAT a_fMaxX, TFLOAT a_fMaxY, TUINT a_uiDivX, TUINT a_uiDivY )
{
	m_uiDivX   = a_uiDivX;
	m_fOffsetX = 0.0f;
	m_fOffsetY = 0.0f;
	m_uiDivY   = a_uiDivY;

	if ( a_fMinX < 0.0f )
		m_fOffsetX = -a_fMinX;

	if ( a_fMinY < 0.0f )
		m_fOffsetY = -a_fMinY;

	TFLOAT fRangeX = a_fMaxX - a_fMinX;
	TFLOAT fRangeY = a_fMaxY - a_fMinY;
	TASSERT( fRangeX >= 0 );
	TASSERT( fRangeY >= 0 );

	m_fInvCellSizeX = 1.0f / ( fRangeX / TFLOAT( m_uiDivX ) );
	m_fInvCellSizeY = 1.0f / ( fRangeY / TFLOAT( m_uiDivY ) );

	AddFreeRefs();
}

// $deBlob: FUNCTION 007e0430
// $deBlob: FUNCTION 0078f850
AAINodeHash::~AAINodeHash()
{
	for ( TINT i = 0; i < BUCKETS_DIV * BUCKETS_DIV; i++ )
	{
		while ( !m_aBuckets[ i ].IsEmpty() )
			delete m_aBuckets[ i ].PopFront().Get();
	}

	while ( !m_oFreeRefs.IsEmpty() )
		delete m_oFreeRefs.PopFront().Get();
}

void AAINodeHash::AddFreeRefs()
{
	for ( TINT i = REFS_PER_ADD; i != 0; i-- )
	{
		HashRef* pRef = new HashRef;
		pRef->m_pNode = TNULL;
		m_oFreeRefs.PushBack( pRef );
	}
}

// $deBlob: FUNCTION 007e0530
void AAINodeHash::Hash( AAINode* a_pNode )
{
	TFLOAT fXL = m_fOffsetX + a_pNode->m_vPosition.x;
	TFLOAT fYL = m_fOffsetY + a_pNode->m_vPosition.z;
	TASSERT( fXL >= 0 && fYL >= 0, "Entity is outside the range of the entity hash, increase hash min/max ranges" );

	TINT iX = GetX( fXL );
	TINT iY = GetY( fYL );

	if ( m_oFreeRefs.IsEmpty() )
		AddFreeRefs();

	HashRef* pRef = m_oFreeRefs.PopBack().Get();
	pRef->m_pNode = a_pNode;
	GetBucket( iX, iY ).PushBack( pRef );
}

// $deBlob: FUNCTION 007e06c0
void AAINodeHash::Release( AAINode* a_pNode )
{
	HashBucket* pBucket = GetBucket( a_pNode->m_vPosition.x, a_pNode->m_vPosition.z );

	for ( auto it = pBucket->Begin(); it != pBucket->End(); it++ )
	{
		if ( it->m_pNode == a_pNode )
		{
			HashRef* pRef = it.Get();
			pBucket->Erase( it );
			m_oFreeRefs.PushBack( pRef );
			return;
		}
	}
}

// $deBlob: FUNCTION 007e0780
AAINodeHash::NodeRef* AAINodeHash::VicinityHashCompare( const TVector4& a_rvPosition, TFLOAT a_fRange, TUINT& a_ruiNumNodes, TUINT a_uiMaxNodes, TUINT8 a_uiNodeMaskFlags )
{
	a_ruiNumNodes = 0;

	TFLOAT fXL = ( a_rvPosition.x - a_fRange ) + m_fOffsetX;
	TFLOAT fXU = ( a_rvPosition.x + a_fRange ) + m_fOffsetX;
	TFLOAT fYL = ( a_rvPosition.z - a_fRange ) + m_fOffsetY;
	TFLOAT fYU = ( a_rvPosition.z + a_fRange ) + m_fOffsetY;

	if ( fXL < 0.0f ) fXL = 0.0f;
	if ( fXU < 0.0f ) fXU = 0.0f;
	if ( fYL < 0.0f ) fYL = 0.0f;
	if ( fYU < 0.0f ) fYU = 0.0f;

	TUINT iXL = GetX( fXL );
	TUINT iXU = GetX( fXU ) + 1;
	TUINT iYL = GetY( fYL );
	TUINT iYU = GetY( fYU ) + 1;

	// NOTE: the upper bounds are clamped to the last cell, which is never searched (retail)
	iYU = TMath::Min( iYU, m_uiDivY - 1 );
	iXU = TMath::Min( iXU, m_uiDivX - 1 );
	iYL = TMath::Min( iYL, m_uiDivY - 1 );
	iXL = TMath::Min( iXL, m_uiDivX - 1 );

	TASSERT( iXL < m_uiDivX );
	TASSERT( iXU < m_uiDivX );
	TASSERT( iYL < m_uiDivY );
	TASSERT( iYU < m_uiDivY );

	TFLOAT fRangeSq = a_fRange * a_fRange;

	for ( TUINT iY = iYL; iY < iYU; iY++ )
	{
		for ( TUINT iX = iXL; iX < iXU; iX++ )
		{
			HashBucket& rBucket = GetBucket( TINT( iX ), TINT( iY ) );

			for ( auto it = rBucket.Begin(); it != rBucket.End(); it++ )
			{
				AAINode* pNode = it->m_pNode;

				if ( pNode->m_uiFlags & a_uiNodeMaskFlags )
				{
					TFLOAT fDistSq = TVector4::DistanceSq( pNode->m_vPosition, a_rvPosition );

					if ( fDistSq <= fRangeSq )
					{
						m_aResults[ a_ruiNumNodes ].pNode   = pNode;
						m_aResults[ a_ruiNumNodes ].fDistSq = fDistSq;
						a_ruiNumNodes++;

						if ( a_ruiNumNodes >= a_uiMaxNodes )
							return m_aResults;
					}
				}
			}
		}
	}

	return m_aResults;
}

// $deBlob: FUNCTION 007e0c30
TFLOAT AAINodeHash::GetAverageBucketSize()
{
	TFLOAT fTotal      = 0.0f;
	TINT   iNumBuckets = 0;

	for ( TINT y = 0; y < BUCKETS_DIV; y++ )
	{
		for ( TINT x = 0; x < BUCKETS_DIV; x++ )
		{
			TINT iSize = GetBucket( x, y ).Size();

			if ( iSize > 0 )
			{
				iNumBuckets++;
				fTotal += TFLOAT( iSize );
			}
		}
	}

	return fTotal / TFLOAT( iNumBuckets );
}

// $deBlob: FUNCTION 007e0cb0
AAINodeHash::HashBucket* AAINodeHash::GetBucket( TFLOAT a_fX, TFLOAT a_fY )
{
	TFLOAT fXL = m_fOffsetX + a_fX;
	TFLOAT fYL = m_fOffsetY + a_fY;
	TASSERT( fXL >= 0 && fYL >= 0, "Entity is outside the range of the entity hash, increase hash min/max ranges" );

	return &GetBucket( GetX( fXL ), GetY( fYL ) );
}
