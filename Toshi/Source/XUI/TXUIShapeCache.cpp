#include "ToshiPCH.h"
#include "TXUIShapeCache.h"
#include "TXUIFigure.h"
#include "TXUI.h"
#include "GUI/T2GUITransform.h"

#include <malloc.h>
#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TINT TXUIShape::ms_iSplineSteps = 5;

// Size of the index buffer TesselateShape builds the triangle list in
static TCONSTEXPR_VAR TUINT32 uiIndicesBufSize = 1024;

//-----------------------------------------------------------------------------
// TXUIShape
//-----------------------------------------------------------------------------

// Returns TTRUE if the vertex a_uiCurrVert of the polygon is an ear: convex and no other vertex or edge in the
// triangle a_uiPrevVert, a_uiCurrVert, a_uiNextVert
// $deBlob: FUNCTION 006db900
TBOOL TXUIShape::IsEar( TUINT32 a_uiPrevVert, TUINT32 a_uiCurrVert, TUINT32 a_uiNextVert )
{
	TASSERT( a_uiPrevVert < m_uiNumVerts );
	TASSERT( a_uiCurrVert < m_uiNumVerts );
	TASSERT( a_uiNextVert < m_uiNumVerts );

	TVector2 vPrev = m_pVerts[ a_uiPrevVert ].Get();
	TVector2 vCurr = m_pVerts[ a_uiCurrVert ].Get();
	TVector2 vNext = m_pVerts[ a_uiNextVert ].Get();

	TFLOAT fCross = ( vNext.y - vCurr.y ) * ( vCurr.x - vPrev.x ) - ( vNext.x - vCurr.x ) * ( vCurr.y - vPrev.y );

	if ( m_bIsClockWise )
	{
		if ( 0.0f < fCross )
			return TFALSE;
	}
	else if ( !( 0.0f <= fCross ) ) // NOTE: as retail tests it (a NaN isn't an ear)
	{
		return TFALSE;
	}

	if ( TMath::Abs( fCross ) >= 1e-06f )
	{
		for ( TINT i = 0; i < TINT( m_uiNumVerts ); i++ )
		{
			TINT iNext = i + 1;

			if ( iNext >= TINT( m_uiNumVerts ) )
				iNext -= m_uiNumVerts;

			if ( TUINT32( i ) != a_uiPrevVert )
			{
				if ( TUINT32( i ) != a_uiNextVert && TUINT32( iNext ) != a_uiPrevVert && TUINT32( iNext ) != a_uiNextVert )
				{
					TVector2 vEdgeStart = m_pVerts[ i ].Get();
					TVector2 vEdgeEnd   = m_pVerts[ iNext ].Get();

					if ( TMath::LineSegmentVsLineSegment2D( vPrev, vNext, vEdgeStart, vEdgeEnd ) == TTRUE )
						return TFALSE;
				}

				if ( TUINT32( i ) != a_uiCurrVert && TUINT32( i ) != a_uiNextVert )
				{
					TVector2 vPoint = m_pVerts[ i ].Get();

					if ( TMath::IsPointInTriangle2D( vPrev, vCurr, vNext, vPoint ) )
						return TFALSE;
				}
			}
		}
	}

	return TTRUE;
}

// NOTE: TT == 1 returns the 4th point whatever a_iN is
// $deBlob: FUNCTION 006dc990
TVector2 Bezier( const TVector2* a_pPoints, TINT a_iN, TFLOAT a_fT )
{
	if ( a_fT == 0.0f )
		return a_pPoints[ 0 ];

	if ( a_fT == 1.0f )
		return a_pPoints[ 3 ];

	TVector2 vResult( 0.0f, 0.0f );
	TFLOAT   fOneMinusT    = 1.0f - a_fT;
	TFLOAT   fPowOneMinusT = TFLOAT( pow( fOneMinusT, a_iN ) );
	TFLOAT   fPowT         = 1.0f;
	TINT     iNMinusI      = a_iN;

	for ( TINT i = 0; i <= a_iN; i++ )
	{
		TFLOAT fCoeff = fPowOneMinusT * fPowT;
		fPowT         = fPowT * a_fT;
		fPowOneMinusT = fPowOneMinusT * ( 1.0f / fOneMinusT );

		// Binomial coefficient a_iN! / ( i! * ( a_iN - i )! )
		TINT iFactorial   = a_iN;
		TINT iIFactorial  = i;
		TINT iNIFactorial = iNMinusI;

		while ( iFactorial > 0 )
		{
			fCoeff = fCoeff * TFLOAT( iFactorial-- );

			if ( iIFactorial > 1 )
				fCoeff = fCoeff / TFLOAT( iIFactorial-- );

			if ( iNIFactorial > 1 )
				fCoeff = fCoeff / TFLOAT( iNIFactorial-- );
		}

		vResult.x = a_pPoints[ i ].x * fCoeff + vResult.x;
		vResult.y = a_pPoints[ i ].y * fCoeff + vResult.y;
		iNMinusI--;
	}

	return vResult;
}

//-----------------------------------------------------------------------------
// TXUIShapeCache
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006dc6b0
TXUIShapeCache::TXUIShapeCache()
    : m_oAllocator( TXUI::ms_pXUIMemoryBlock )
    , m_oFigureToShapeMap( &m_oAllocator )
{
}

// $deBlob: FUNCTION 006dc710
// $deBlob: FUNCTION 006be680
TXUIShapeCache::~TXUIShapeCache()
{
	TASSERT( m_oFigureToShapeMap.IsEmpty() );
}

// $deBlob: FUNCTION 006dc790
TXUIShape* TXUIShapeCache::CreateShape( TXUIShapeParams* a_pParams )
{
	ShapeMap::Iterator it = m_oFigureToShapeMap.Find( a_pParams );

	if ( it != m_oFigureToShapeMap.End() )
	{
		TXUIShape* pShape = it->second;
		TASSERT( pShape->m_uiRefCount < ( 1 << 16 ) - 1 );
		pShape->m_uiRefCount++;

		delete a_pParams;
		return pShape;
	}

	TXUIShape* pShape = new ( TXUI::ms_pXUIMemoryBlock ) TXUIShape();

	TBOOL bRes = pShape->CreateControlPoints( a_pParams->m_pPoints, a_pParams->m_uiNumPoints );
	TASSERT( TTRUE == bRes );

	pShape->TesselateShape( a_pParams->m_pPoints, a_pParams->m_uiNumPoints );
	pShape->CalcScale();

	m_oFigureToShapeMap.Insert( a_pParams, pShape );
	pShape->m_uiRefCount++;

	return pShape;
}

// $deBlob: FUNCTION 006dc8b0
void TXUIShapeCache::DestroyShape( TXUIShape* a_pShape )
{
	TASSERT( a_pShape->m_uiRefCount > 0 );

	if ( --a_pShape->m_uiRefCount == 0 )
	{
		for ( ShapeMap::Iterator it = m_oFigureToShapeMap.Begin(); it != m_oFigureToShapeMap.End(); it++ )
		{
			if ( it->second == a_pShape )
			{
				TXUIShapeParams* pParams = it->first;
				m_oFigureToShapeMap.Remove( it );

				if ( a_pShape )
					delete a_pShape;

				delete pParams;
				return;
			}
		}
	}
}

// Orders the shapes by number of points, then by the bytes of the points that aren't equal
// $deBlob: FUNCTION 006dd070
TINT TXUIShapeCache::Comparator::operator()( TXUIShapeParams* a_pParams1, TXUIShapeParams* a_pParams2 ) const
{
	if ( a_pParams1->m_uiNumPoints < a_pParams2->m_uiNumPoints )
		return -1;

	if ( a_pParams2->m_uiNumPoints < a_pParams1->m_uiNumPoints )
		return 1;

	for ( TUINT32 i = 0; i < a_pParams1->m_uiNumPoints; i++ )
	{
		if ( !( a_pParams1->m_pPoints[ i ] == a_pParams2->m_pPoints[ i ] ) )
		{
			TINT iResult = TUtil::MemCompare( a_pParams1->m_pPoints, a_pParams2->m_pPoints, a_pParams1->m_uiNumPoints * sizeof( XUIFigurePoint ) );

			if ( iResult < 0 )
				return -1;

			if ( iResult > 0 )
				return 1;
		}
	}

	return 0;
}

// NOTE: retail's T2MemoryAllocator calls the heap functions directly
void* TXUIShapeCache::HeapAllocator::Malloc( size_t a_uiSize )
{
	return TMemoryHeap_Malloc( m_pHeap, a_uiSize );
}

void* TXUIShapeCache::HeapAllocator::Malloc( size_t a_uiSize, size_t a_uiAlignment )
{
	return TMemoryHeap_Memalign( m_pHeap, a_uiAlignment, a_uiSize );
}

void TXUIShapeCache::HeapAllocator::Free( void* a_pMem )
{
	operator delete( a_pMem );
}

TBOOL TXUIShapeCache::HeapAllocator::CanAllocate( size_t a_uiSize ) const
{
	return TTRUE;
}

void* TXUIShapeCache::HeapAllocator::TryMalloc( size_t a_uiSize )
{
	return Malloc( a_uiSize );
}

void* TXUIShapeCache::HeapAllocator::TryMalloc( size_t a_uiSize, size_t a_uiAlignment )
{
	return Malloc( a_uiSize, a_uiAlignment );
}

size_t TXUIShapeCache::HeapAllocator::GetUsedSize() const
{
	return 0;
}

size_t TXUIShapeCache::HeapAllocator::GetCapacity() const
{
	return 0;
}

//-----------------------------------------------------------------------------
// TXUIShape
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006da060
TXUIShape::TXUIShape()
    : m_vDims( TVector2::VEC_ZERO )
{
	m_uiRefCount         = 0;
	m_uiNumControlPoints = 0;
	m_uiNumVerts         = 0;
	m_uiNumIndices       = 0;
	m_uiNumTriangles     = 0;
	m_pVerts             = TNULL;
	m_pControlPoints     = TNULL;
	m_pIndices           = TNULL;
	m_pColours           = TNULL;
	m_pUVs               = TNULL;
	m_bIsClockWise       = TTRUE;
}

// $deBlob: FUNCTION 006dc5a0
// $deBlob: FUNCTION 006c9860
TXUIShape::~TXUIShape()
{
	operator delete( m_pVerts );
	operator delete( m_pControlPoints );
	operator delete( m_pIndices );
	operator delete( m_pColours );
	operator delete( m_pUVs );
}

// Converts the points of a figure to control points, 3 per point (the point and its two Bezier controls)
// $deBlob: FUNCTION 006da0d0
TBOOL TXUIShape::CreateControlPoints( const XUIFigurePoint* a_pPoints, TUINT32 a_uiNumPoints )
{
	TASSERT( 3 * a_uiNumPoints < ( 1 << 16 ) );
	m_uiNumControlPoints = TUINT16( a_uiNumPoints * 3 );
	m_pControlPoints     = new ( TXUI::ms_pXUIMemoryBlock ) TXUIFixedVector2<6>[ m_uiNumControlPoints ];

	for ( TUINT32 i = 0; i < a_uiNumPoints; i++ )
	{
		m_pControlPoints[ i * 3 + 0 ].Set( a_pPoints[ i ].m_vPoint.GetX(), a_pPoints[ i ].m_vPoint.GetY() );
		m_pControlPoints[ i * 3 + 1 ].Set( a_pPoints[ i ].m_vCtrl1.GetX(), a_pPoints[ i ].m_vCtrl1.GetY() );
		m_pControlPoints[ i * 3 + 2 ].Set( a_pPoints[ i ].m_vCtrl2.GetX(), a_pPoints[ i ].m_vCtrl2.GetY() );
	}

	return TTRUE;
}

// Computes the size of the vertices' bounding box (from 0) and centres the vertices and control points on it
// $deBlob: FUNCTION 006da3f0
void TXUIShape::CalcScale()
{
	m_vDims.m_iX = 0;
	m_vDims.m_iY = 0;

	for ( TINT i = 0; i < m_uiNumVerts; i++ )
	{
		m_vDims.SetX( TMath::Max( m_pVerts[ i ].GetX(), m_vDims.GetX() ) );
		m_vDims.SetY( TMath::Max( m_pVerts[ i ].GetY(), m_vDims.GetY() ) );
	}

	TINT16 iOffsetX = TINT16( TINT( m_vDims.GetX() * -0.5f * 6.0f ) );
	TINT16 iOffsetY = TINT16( TINT( m_vDims.GetY() * -0.5f * 6.0f ) );

	for ( TINT i = 0; i < m_uiNumControlPoints; i++ )
	{
		m_pControlPoints[ i ].m_iX += iOffsetX;
		m_pControlPoints[ i ].m_iY += iOffsetY;
	}

	for ( TINT i = 0; i < m_uiNumVerts; i++ )
	{
		m_pVerts[ i ].m_iX += iOffsetX;
		m_pVerts[ i ].m_iY += iOffsetY;
	}
}

// Makes the vertices of the path (one per straight segment, CalculateSplineSteps per curve) and the triangle
// list of its fill (ear clipping)
// $deBlob: FUNCTION 006da540
void TXUIShape::TesselateShape( const XUIFigurePoint* a_pPoints, TUINT32 a_uiNumPoints )
{
	TASSERT( m_uiNumControlPoints > 0 );
	m_uiNumVerts = 0;

	TVector2 aControlPts[ 4 ];

	// Count the vertices
	for ( TUINT32 i = 0, uiControlPoint = 0; i < a_uiNumPoints; i++, uiControlPoint += 3 )
	{
		for ( TINT k = 0; k < 4; k++ )
		{
			TINT iIndex = uiControlPoint + k;

			if ( iIndex >= m_uiNumControlPoints )
				iIndex = 0;

			aControlPts[ k ] = m_pControlPoints[ iIndex ].Get();
		}

		if ( !IsDegenerateSpline( aControlPts ) )
		{
			if ( aControlPts[ 0 ].IsEqual( aControlPts[ 1 ] ) && aControlPts[ 2 ].IsEqual( aControlPts[ 3 ] ) )
			{
				// Straight segment
				TASSERT( m_uiNumVerts + 1 < ( 1 << 16 ) );
				m_uiNumVerts++;
			}
			else
			{
				TASSERT( m_uiNumVerts + CalculateSplineSteps( aControlPts ) < ( 1 << 16 ) );
				m_uiNumVerts += CalculateSplineSteps( aControlPts );
			}
		}
	}

	m_pVerts = new ( TXUI::ms_pXUIMemoryBlock ) TXUIFixedVector2<6>[ m_uiNumVerts ];

	// Make the vertices
	TINT iVert = 0;

	for ( TUINT32 i = 0, uiControlPoint = 0; i < a_uiNumPoints; i++, uiControlPoint += 3 )
	{
		for ( TINT k = 0; k < 4; k++ )
		{
			TINT iIndex = uiControlPoint + k;

			if ( iIndex >= m_uiNumControlPoints )
				iIndex = 0;

			aControlPts[ k ] = m_pControlPoints[ iIndex ].Get();
		}

		if ( !IsDegenerateSpline( aControlPts ) )
		{
			if ( aControlPts[ 0 ].IsEqual( aControlPts[ 1 ] ) && aControlPts[ 2 ].IsEqual( aControlPts[ 3 ] ) )
			{
				m_pVerts[ iVert++ ] = aControlPts[ 0 ];
			}
			else
			{
				TINT   iNumSteps = CalculateSplineSteps( aControlPts );
				TFLOAT fStep     = 1.0f / TFLOAT( iNumSteps );
				TFLOAT fT        = 0.0f;

				for ( ; iNumSteps != 0; iNumSteps-- )
				{
					TVector2 vPoint = Bezier( aControlPts, 3, fT );
					fT += fStep;
					m_pVerts[ iVert++ ] = vPoint;
				}
			}
		}
	}

	// Remove the duplicated vertices
	for ( TINT i = 1; i < m_uiNumVerts - 1; i++ )
	{
		if ( m_pVerts[ i - 1 ].m_iX == m_pVerts[ i ].m_iX && m_pVerts[ i - 1 ].m_iY == m_pVerts[ i ].m_iY )
		{
			for ( TINT j = i - 1; j < m_uiNumVerts - 1; j++ )
				m_pVerts[ j ] = m_pVerts[ j + 1 ];

			m_uiNumVerts--;
			i--;
		}
		else if ( m_pVerts[ i ].m_iX == m_pVerts[ i + 1 ].m_iX && m_pVerts[ i ].m_iY == m_pVerts[ i + 1 ].m_iY )
		{
			for ( TINT j = i + 1; j < m_uiNumVerts - 1; j++ )
				m_pVerts[ j ] = m_pVerts[ j + 1 ];

			m_uiNumVerts--;
			i--;
		}
	}

	if ( !CanTesselate() )
		return;

	// NOTE: inlined by retail
	ComputeIsClockWise();

	// Ear clipping
	VertNode*         aVertNodes = new ( TXUI::ms_pXUIMemoryBlock ) VertNode[ m_uiNumVerts ];
	T2DList<VertNode> oVertList;
	TINT              vertListSize = 0;

	for ( TUINT32 i = 0; i < m_uiNumVerts; i++ )
	{
		aVertNodes[ i ].m_uiIndex = TUINT16( i );
		oVertList.PushBack( &aVertNodes[ i ] );
		vertListSize++;
	}

	TUINT16 aIndices[ uiIndicesBufSize ];
	TUINT32 uiCurrIndex = 0;

	T2DList<VertNode>::Iterator iter = oVertList.Begin().Next();

	while ( vertListSize > 0 )
	{
		TASSERT( oVertList.Size() == vertListSize );

		T2DList<VertNode>::Iterator iterPrev = iter.Prev();

		if ( iterPrev == oVertList.End() )
			iterPrev = iterPrev.Prev();

		T2DList<VertNode>::Iterator iterNext = iter.Next();

		if ( iterNext == oVertList.End() )
			iterNext = oVertList.Begin();

		TASSERT( iterPrev != oVertList.End() );
		TASSERT( iterNext != oVertList.End() );

		TUINT16 uiPrevVert = iterPrev->m_uiIndex;
		TUINT16 uiCurrVert = iter.Get()->m_uiIndex;
		TUINT16 uiNextVert = iterNext->m_uiIndex;

		if ( IsEar( uiPrevVert, uiCurrVert, uiNextVert ) != TTRUE && vertListSize != 3 )
		{
			iter = iter.Next();

			if ( iter == oVertList.End() )
				iter = oVertList.Begin();

			continue;
		}

		aIndices[ uiCurrIndex++ ] = uiPrevVert;
		TASSERT( uiCurrIndex < uiIndicesBufSize );
		aIndices[ uiCurrIndex++ ] = uiCurrVert;
		TASSERT( uiCurrIndex < uiIndicesBufSize );
		aIndices[ uiCurrIndex++ ] = uiNextVert;
		TASSERT( uiCurrIndex < uiIndicesBufSize );

		TASSERT( vertListSize >= 3 );

		if ( vertListSize == 3 )
		{
			oVertList.Clear();
			break;
		}

		T2DList<VertNode>::Iterator iterAfter = iter.Next();
		iter->Remove();
		vertListSize--;
		TASSERT( oVertList.Size() == vertListSize );

		iter = iterAfter;

		if ( iter == oVertList.End() )
			iter = oVertList.Begin();
	}

	TASSERT( oVertList.IsEmpty() );
	delete[] aVertNodes;

	TASSERT( uiCurrIndex < ( 1 << 16 ) );
	m_uiNumIndices = TUINT16( uiCurrIndex );
	TASSERT( m_uiNumIndices / 3 < ( 1 << 16 ) );
	m_uiNumTriangles = m_uiNumIndices / 3;

	m_pIndices = TSTATICCAST( TUINT16, TMalloc( m_uiNumIndices * sizeof( TUINT16 ), TXUI::ms_pXUIMemoryBlock ) );
	TUtil::MemCopy( m_pIndices, aIndices, m_uiNumIndices * sizeof( TUINT16 ) );
}

// Returns TFALSE if two edges of the polygon (that don't share a vertex) intersect
// $deBlob: FUNCTION 006da250
TBOOL TXUIShape::CanTesselate()
{
	TVector2* aVerts = TSTATICCAST( TVector2, _alloca( m_uiNumVerts * sizeof( TVector2 ) ) );

	for ( TINT i = 0; i < m_uiNumVerts; i++ )
		m_pVerts[ i ].Get( aVerts[ i ] );

	for ( TINT i = 0; i < m_uiNumVerts; i++ )
	{
		TINT iPrev = i - 1;

		if ( iPrev < 0 )
			iPrev += m_uiNumVerts;

		TINT iNext = i + 1;

		if ( iNext > m_uiNumVerts - 1 )
			iNext -= m_uiNumVerts;

		for ( TINT j = 0; j < m_uiNumVerts; j++ )
		{
			if ( j != i && j != iPrev && j != iNext )
			{
				TINT jNext = j + 1;

				if ( jNext >= m_uiNumVerts )
					jNext -= m_uiNumVerts;

				if ( !aVerts[ iNext ].IsEqual( aVerts[ j ] ) && !aVerts[ i ].IsEqual( aVerts[ jNext ] ) )
				{
					TVector2 vIntersection;

					if ( TMath::LineSegmentVsLineSegment2D( aVerts[ i ], aVerts[ iNext ], aVerts[ j ], aVerts[ jNext ], vIntersection ) == TTRUE )
						return TFALSE;
				}
			}
		}
	}

	return TTRUE;
}

// Makes the tri strip of the stroke: a quad per edge (the edge and the edge moved a_fWidth along its normal), then
// the first vertex again
// $deBlob: FUNCTION 006db630
TBOOL TXUIShape::TesselateStroke( TFLOAT a_fWidth, TVector2*& a_rpVerts, TVector2 a_vScale )
{
	TINT      iVert  = 0;
	TVector2* aVerts = TSTATICCAST( TVector2, _alloca( m_uiNumVerts * sizeof( TVector2 ) ) );

	for ( TINT i = 0; i < m_uiNumVerts; i++ )
		m_pVerts[ i ].Get( aVerts[ i ] );

	TVector2 vFirstNormal;

	for ( TINT i = 0; i < m_uiNumVerts; i++ )
	{
		TINT iNext = i + 1;

		if ( iNext < 0 )
			iNext = m_uiNumVerts + 1 + i;
		else if ( iNext > m_uiNumVerts - 1 )
			iNext = i - m_uiNumVerts + 1;

		TVector2 vNormal(
		    aVerts[ iNext ].y * a_vScale.y - aVerts[ i ].y * a_vScale.y,
		    -( aVerts[ iNext ].x * a_vScale.x - aVerts[ i ].x * a_vScale.x )
		);
		vNormal.Normalize();

		if ( i == 0 )
			vFirstNormal = vNormal;

		a_rpVerts[ iVert ].x     = aVerts[ i ].x * a_vScale.x;
		a_rpVerts[ iVert ].y     = aVerts[ i ].y * a_vScale.y;
		a_rpVerts[ iVert + 1 ].x = aVerts[ i ].x * a_vScale.x + vNormal.x * a_fWidth;
		a_rpVerts[ iVert + 1 ].y = aVerts[ i ].y * a_vScale.y + vNormal.y * a_fWidth;
		a_rpVerts[ iVert + 2 ].x = a_vScale.x * aVerts[ iNext ].x;
		a_rpVerts[ iVert + 2 ].y = aVerts[ iNext ].y * a_vScale.y;
		a_rpVerts[ iVert + 3 ].x = aVerts[ iNext ].x * a_vScale.x + vNormal.x * a_fWidth;
		a_rpVerts[ iVert + 3 ].y = aVerts[ iNext ].y * a_vScale.y + vNormal.y * a_fWidth;
		iVert += 4;
	}

	a_rpVerts[ iVert ].x     = aVerts[ 0 ].x * a_vScale.x;
	a_rpVerts[ iVert ].y     = aVerts[ 0 ].y * a_vScale.y;
	a_rpVerts[ iVert + 1 ].x = aVerts[ 0 ].x * a_vScale.x + vFirstNormal.x * a_fWidth;
	a_rpVerts[ iVert + 1 ].y = aVerts[ 0 ].y * a_vScale.y + vFirstNormal.y * a_fWidth;

	return TTRUE;
}

// Makes the anti-aliasing edge of the shape: a tri strip around the outline whose outer vertices are transparent
// $deBlob: FUNCTION 006dbbe0
TXUIShape* TXUIShape::CloneAndMakeAaEdge( TVector2 a_vScale, TUINT32 a_uiColour )
{
	ComputeIsClockWise();

	TXUIShape* pShape = new ( TXUI::ms_pXUIMemoryBlock ) TXUIShape();

	TASSERT( m_uiNumVerts * 2 < ( 1 << 16 ) );
	pShape->m_uiNumVerts     = m_uiNumVerts * 2;
	pShape->m_pVerts         = new ( TXUI::ms_pXUIMemoryBlock ) TXUIFixedVector2<6>[ pShape->m_uiNumVerts ];
	pShape->m_uiNumTriangles = pShape->m_uiNumVerts;

	TASSERT( pShape->m_uiNumVerts + 2 < ( 1 << 16 ) );
	pShape->m_uiNumIndices = pShape->m_uiNumVerts + 2;
	pShape->m_pIndices     = TSTATICCAST( TUINT16, TMalloc( pShape->m_uiNumIndices * sizeof( TUINT16 ), TXUI::ms_pXUIMemoryBlock ) );
	pShape->m_pColours     = TSTATICCAST( TUINT32, TMalloc( pShape->m_uiNumVerts * sizeof( TUINT32 ), TXUI::ms_pXUIMemoryBlock ) );

	MakeAaEdgeVerts( pShape, a_vScale );

	if ( pShape->m_pColours )
	{
		for ( TINT i = 0; i < pShape->m_uiNumVerts; i += 2 )
		{
			pShape->m_pColours[ i ]     = a_uiColour;
			pShape->m_pColours[ i + 1 ] = a_uiColour & 0x00FFFFFF;
		}
	}

	for ( TINT i = 0; i < pShape->m_uiNumIndices; i++ )
		pShape->m_pIndices[ i ] = TUINT16( i );

	// Close the strip
	pShape->m_pIndices[ pShape->m_uiNumIndices - 2 ] = 0;
	pShape->m_pIndices[ pShape->m_uiNumIndices - 1 ] = 1;

	return pShape;
}

// Puts the vertices of the anti-aliasing edge a_pShape: each vertex of this shape, and the vertex moved out by 1.5
// pixels along the bisector of its edges' normals
// $deBlob: FUNCTION 006dbe50
void TXUIShape::MakeAaEdgeVerts( TXUIShape* a_pShape, TVector2 a_vScale )
{
	TASSERT( a_pShape->m_uiNumVerts == m_uiNumVerts * 2 );
	TASSERT( a_pShape->m_uiNumTriangles == a_pShape->m_uiNumVerts );
	TASSERT( a_pShape->m_uiNumIndices == a_pShape->m_uiNumVerts + 2 );

	TVector2* aVerts = TSTATICCAST( TVector2, _alloca( m_uiNumVerts * sizeof( TVector2 ) ) );

	for ( TINT i = 0; i < m_uiNumVerts; i++ )
		m_pVerts[ i ].Get( aVerts[ i ] );

	TINT iNewVertexIndex = 0;

	for ( TINT i = 0; i < m_uiNumVerts; i++ )
	{
		TINT iPrev = i - 1;

		if ( iPrev < 0 )
			iPrev = m_uiNumVerts - 1;

		TINT iNext = ( i + 1 ) % m_uiNumVerts;

		const TVector2& vCurr = aVerts[ i ];
		const TVector2& vPrev = aVerts[ iPrev ];
		const TVector2& vNext = aVerts[ iNext ];

		TVector2 vNormal1(
		    vCurr.y * a_vScale.y - vPrev.y * a_vScale.y,
		    -( vCurr.x * a_vScale.x - vPrev.x * a_vScale.x )
		);
		vNormal1.Normalize();

		TVector2 vNormal2(
		    vNext.y * a_vScale.y - vCurr.y * a_vScale.y,
		    -( a_vScale.x * vNext.x - vCurr.x * a_vScale.x )
		);
		vNormal2.Normalize();

		TVector2 vBisector( vNormal2.x + vNormal1.x, vNormal2.y + vNormal1.y );
		vBisector.Normalize();

		TFLOAT fLength = 1.5f / ( vBisector.y * vNormal1.y + vBisector.x * vNormal1.x );

		if ( m_bIsClockWise )
			fLength *= -1.0f;

		TASSERT( iNewVertexIndex < a_pShape->m_uiNumVerts );
		a_pShape->m_pVerts[ iNewVertexIndex ].Set( vCurr.x * a_vScale.x, vCurr.y * a_vScale.y );
		iNewVertexIndex++;

		TASSERT( iNewVertexIndex < a_pShape->m_uiNumVerts );
		a_pShape->m_pVerts[ iNewVertexIndex ].Set( vCurr.x * a_vScale.x + vBisector.x * fLength, vCurr.y * a_vScale.y + vBisector.y * fLength );
		iNewVertexIndex++;
	}
}

// $deBlob: FUNCTION 006dc200
void TXUIShape::SetColorAaEdge( TUINT32 a_uiColour )
{
	if ( m_pColours )
	{
		for ( TINT i = 0; i < m_uiNumVerts; i += 2 )
		{
			m_pColours[ i ]     = a_uiColour;
			m_pColours[ i + 1 ] = a_uiColour & 0x00FFFFFF;
		}
	}
}

// Same as TXUIFigure::SetupUVs for the anti-aliasing edge
// $deBlob: FUNCTION 006dc240
void TXUIShape::MakeUVsAaEdge( T2GUITransform* a_pTransform, TVector2 a_vScale, TVector2 a_vSize, TFLOAT a_fMaxU, TFLOAT a_fMaxV )
{
	TFLOAT fWidth  = a_vScale.x * a_vSize.x;
	TFLOAT fHeight = a_vScale.y * a_vSize.y;

	if ( m_pUVs == TNULL )
		m_pUVs = new ( TXUI::ms_pXUIMemoryBlock ) TXUIFixedVector2<128>[ m_uiNumVerts ];

	if ( fWidth != 0.0f && fHeight != 0.0f )
	{
		TFLOAT fOffsetX = fWidth * -0.5f;
		TFLOAT fOffsetY = fHeight * -0.5f;

		static TVector2 s_vCentre( 0.5f, 0.5f );

		for ( TUINT32 i = 0; i < m_uiNumVerts; i++ )
		{
			TFLOAT fV = ( ( m_pVerts[ i ].GetY() * a_vScale.y - fOffsetY ) / fHeight ) * a_fMaxV;
			TFLOAT fU = ( ( m_pVerts[ i ].GetX() * a_vScale.x - fOffsetX ) / fWidth ) * a_fMaxU;
			TMath::Clip( fU, 0.0f, a_fMaxU );
			TMath::Clip( fV, 0.0f, a_fMaxV );

			TFLOAT fY = a_pTransform->m_Rot[ 0 ].GetY() * ( fU - s_vCentre.x ) + a_pTransform->m_Rot[ 1 ].GetY() * ( fV - s_vCentre.y ) + a_pTransform->m_Pos.y + s_vCentre.y;
			TFLOAT fX = a_pTransform->m_Rot[ 0 ].GetX() * ( fU - s_vCentre.x ) + a_pTransform->m_Rot[ 1 ].GetX() * ( fV - s_vCentre.y ) + a_pTransform->m_Pos.x + s_vCentre.x;
			m_pUVs[ i ].Set( fX, fY );
		}
	}
}

// $deBlob: FUNCTION 006dc4e0
void TXUIShape::ComputeIsClockWise()
{
	TFLOAT fArea = 0.0f;

	if ( m_uiNumVerts > 1 )
	{
		TFLOAT fPrevX = m_pVerts[ 0 ].GetX();
		TFLOAT fPrevY = m_pVerts[ 0 ].GetY();

		for ( TINT i = 1; i < m_uiNumVerts; i++ )
		{
			TFLOAT fX = m_pVerts[ i ].GetX();
			TFLOAT fY = m_pVerts[ i ].GetY();
			fArea += fPrevX * fY - fPrevY * fX;
			fPrevX = fX;
			fPrevY = fY;
		}
	}

	m_bIsClockWise = fArea < 0.0f;
}

// Returns TTRUE if the 4 control points of the segment are all the same
// $deBlob: FUNCTION 006dc5d0
TBOOL TXUIShape::IsDegenerateSpline( TVector2* a_pControlPts )
{
	for ( TINT i = 0; i < 4; i++ )
	{
		for ( TINT j = 0; j < 4; j++ )
		{
			if ( j != i && !a_pControlPts[ i ].IsEqual( a_pControlPts[ j ] ) )
				return TFALSE;
		}
	}

	return TTRUE;
}

// Number of vertices of a curved segment: up to ms_iSplineSteps, by its length (a segment of 10 pixels or more
// gets all of them), at least 2
// $deBlob: FUNCTION 006dc630
TINT TXUIShape::CalculateSplineSteps( TVector2* a_pControlPts )
{
	TFLOAT fFrac = TVector2::DistanceSq( a_pControlPts[ 0 ], a_pControlPts[ 3 ] ) / 100.0f;
	TMath::Clip( fFrac, 0.0f, 1.0f );

	TINT iNumSteps = TINT( TFLOAT( ms_iSplineSteps ) * fFrac );
	return ( 1 < iNumSteps ) ? iNumSteps : 2;
}

TOSHI_NAMESPACE_END
