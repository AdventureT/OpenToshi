#include "pch.h"
#include "APointDataFunction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APointDataFunction::Point ) == 0x30 );

// $deBlob: FUNCTION 005c0c40
APointDataFunction::APointDataFunction()
{
	m_pHeader = TNULL;
}

// $deBlob: FUNCTION 005c0c50
APointDataFunction::~APointDataFunction()
{
	m_pHeader = TNULL;
}

// $deBlob: FUNCTION 005c0c60
TBOOL APointDataFunction::Load( TTRB* a_pTRB )
{
	m_pHeader = TNULL;
	m_pHeader = TSTATICCAST( PointDataHeader, a_pTRB->GetSymbolAddress( "PointDataHeader" ) );
	return TTRUE;
}

// $deBlob: FUNCTION 005c0ca0
APointDataFunction::Point* APointDataFunction::GetPoint( TINT a_itype )
{
	TASSERT( a_itype >= POINT_DEFAULT && a_itype < POINT_MAX );
	return GetNthPoint( a_itype, 1 );
}

// $deBlob: FUNCTION 005c0cf0
APointDataFunction::Point* APointDataFunction::GetLastPoint( TINT a_itype )
{
	TASSERT( a_itype >= POINT_DEFAULT && a_itype < POINT_MAX );

	if ( m_pHeader != TNULL )
	{
		for ( TINT i = m_pHeader->m_iNumPoints; i != 0; )
		{
			i--;

			if ( m_pHeader->m_pPoints[ i ].m_iType == a_itype )
				return &m_pHeader->m_pPoints[ i ];
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 005c0d80
APointDataFunction::Point* APointDataFunction::GetNthPoint( TINT a_itype, TINT a_iNth )
{
	TASSERT( a_itype >= POINT_DEFAULT && a_itype < POINT_MAX );
	TASSERT( a_iNth > 0 );

	if ( m_pHeader )
	{
		for ( TINT i = 0; i < m_pHeader->m_iNumPoints; i++ )
		{
			if ( m_pHeader->m_pPoints[ i ].m_iType == a_itype && --a_iNth < 1 )
				return &m_pHeader->m_pPoints[ i ];
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 005c0e40
APointDataFunction::Point* APointDataFunction::GetPointWithUnk2C( TINT a_itype, TINT a_iUnk2C )
{
	TASSERT( a_itype >= POINT_DEFAULT && a_itype < POINT_MAX );
	return GetNthPointWithUnk2C( a_itype, a_iUnk2C, 1 );
}

// $deBlob: FUNCTION 005c0ea0
APointDataFunction::Point* APointDataFunction::GetLastPointWithUnk2C( TINT a_itype, TINT a_iUnk2C )
{
	TASSERT( a_itype >= POINT_DEFAULT && a_itype < POINT_MAX );

	if ( m_pHeader != TNULL )
	{
		for ( TINT i = m_pHeader->m_iNumPoints; i != 0; )
		{
			i--;
			Point& rPoint = m_pHeader->m_pPoints[ i ];

			if ( rPoint.m_iType == a_itype && rPoint.m_iUnk2C == a_iUnk2C )
				return &rPoint;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 005c0f30
APointDataFunction::Point* APointDataFunction::GetNthPointWithUnk2C( TINT a_itype, TINT a_iUnk2C, TINT a_iNth )
{
	TASSERT( a_itype >= POINT_DEFAULT && a_itype < POINT_MAX );
	TASSERT( a_iNth > 0 );

	if ( m_pHeader != TNULL )
	{
		for ( TINT i = 0; i < m_pHeader->m_iNumPoints; i++ )
		{
			Point& rPoint = m_pHeader->m_pPoints[ i ];

			if ( rPoint.m_iType == a_itype && rPoint.m_iUnk2C == a_iUnk2C && --a_iNth < 1 )
				return &rPoint;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 005c1000
APointDataFunction::Point* APointDataFunction::GetNearestPoint( TINT a_itype, const TVector4& a_rPosition )
{
	TASSERT( a_itype >= POINT_DEFAULT && a_itype < POINT_MAX );

	Point* pNearest     = TNULL;
	TFLOAT fNearestDist = TMath::MAXFLOAT;

	if ( m_pHeader != TNULL )
	{
		for ( TINT i = 0; i < m_pHeader->m_iNumPoints; i++ )
		{
			Point& rPoint = m_pHeader->m_pPoints[ i ];

			if ( rPoint.m_iType == a_itype )
			{
				TFLOAT fDist = TVector4::DistanceSq( rPoint.m_vPosition, a_rPosition );

				if ( fDist < fNearestDist )
				{
					pNearest     = &rPoint;
					fNearestDist = fDist;
				}
			}
		}
	}

	return pNearest;
}

// $deBlob: FUNCTION 005c10d0
APointDataFunction::Point* APointDataFunction::GetFarthestPoint( TINT a_itype, const TVector4& a_rPosition )
{
	TASSERT( a_itype >= POINT_DEFAULT && a_itype < POINT_MAX );

	Point* pFarthest     = TNULL;
	TFLOAT fFarthestDist = 0.0f;

	if ( m_pHeader != TNULL )
	{
		for ( TINT i = 0; i < m_pHeader->m_iNumPoints; i++ )
		{
			Point& rPoint = m_pHeader->m_pPoints[ i ];

			if ( rPoint.m_iType == a_itype )
			{
				TFLOAT fDist = TVector4::DistanceSq( rPoint.m_vPosition, a_rPosition );

				if ( fFarthestDist < fDist )
				{
					pFarthest     = &rPoint;
					fFarthestDist = fDist;
				}
			}
		}
	}

	return pFarthest;
}

// $deBlob: FUNCTION 005c1190
APointDataFunction::Point* APointDataFunction::GetNearestRespawnPoint( TINT a_itype, const TVector4& a_rPosition )
{
	TASSERT( a_itype >= POINT_PLAYER_1_RESPAWN && a_itype <= POINT_PLAYER_4_RESPAWN );

	Point* pNearest     = TNULL;
	TFLOAT fNearestDist = TMath::MAXFLOAT;

	if ( m_pHeader != TNULL )
	{
		for ( TINT i = m_pHeader->m_iNumPoints; i != 0; )
		{
			i--;
			Point& rPoint = m_pHeader->m_pPoints[ i ];

			if ( 0.0f < rPoint.m_aUnk20[ 0 ] && ( rPoint.m_iType == a_itype || rPoint.m_iType == POINT_PLAYER_RESPAWN ) )
			{
				TFLOAT fDist = TVector4::DistanceSq( rPoint.m_vPosition, a_rPosition );

				if ( fDist < fNearestDist )
				{
					pNearest     = &rPoint;
					fNearestDist = fDist;
				}
			}
		}
	}

	return pNearest;
}

// $deBlob: FUNCTION 005c1270
APointDataFunction::Point* APointDataFunction::GetFarthestRespawnPoint( TINT a_itype, const Toshi::TVector4& a_rPosition )
{
	TASSERT( a_itype >= POINT_PLAYER_1_RESPAWN && a_itype <= POINT_PLAYER_4_RESPAWN );

	Point* pFarthest     = TNULL;
	TFLOAT fFarthestDist = 0.0f;

	if ( m_pHeader != TNULL )
	{
		for ( TINT i = m_pHeader->m_iNumPoints; i != 0; )
		{
			i--;
			Point& rPoint = m_pHeader->m_pPoints[ i ];

			if ( 0.0f < rPoint.m_aUnk20[ 0 ] && ( rPoint.m_iType == a_itype || rPoint.m_iType == POINT_PLAYER_RESPAWN ) )
			{
				TFLOAT fDist = Toshi::TVector4::DistanceSq( rPoint.m_vPosition, a_rPosition );

				if ( fFarthestDist < fDist )
				{
					pFarthest     = &rPoint;
					fFarthestDist = fDist;
				}
			}
		}
	}

	return pFarthest;
}

// The point types 9-13; NOTE: names not resolved
static TBOOL IsRespawnPointType( TINT16 a_iType )
{
	return a_iType == 13 || a_iType == 9 || a_iType == 10 || a_iType == 11 || a_iType == 12;
}

// $deBlob: FUNCTION 005c1350
APointDataFunction::Point* APointDataFunction::GetPointByIndex( TINT a_iIndex, TINT a_itype )
{
	if ( m_pHeader == TNULL )
		return TNULL;

	TINT iIndex = 0;

	for ( TINT i = 0; i < m_pHeader->m_iNumPoints; i++ )
	{
		if ( m_pHeader->m_pPoints[ i ].m_iType == a_itype )
		{
			if ( iIndex == a_iIndex )
				return &m_pHeader->m_pPoints[ i ];

			iIndex++;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 005c13f0
TINT APointDataFunction::GetPointIndex( Point* a_pPoint, TINT a_itype )
{
	TINT iIndex = 0;

	for ( TINT i = 0; i < m_pHeader->m_iNumPoints; i++ )
	{
		if ( m_pHeader->m_pPoints[ i ].m_iType == a_itype )
		{
			if ( &m_pHeader->m_pPoints[ i ] == a_pPoint )
				return iIndex;

			iIndex++;
		}
	}

	return -1;
}

// $deBlob: FUNCTION 005c14e0
void APointDataFunction::ResetUnk20()
{
	if ( m_pHeader == TNULL )
		return;

	for ( TINT i = m_pHeader->m_iNumPoints; i != 0; )
	{
		i--;
		Point& rPoint = m_pHeader->m_pPoints[ i ];

		if ( IsRespawnPointType( rPoint.m_iType ) )
			rPoint.m_aUnk20[ 0 ] = ( rPoint.m_iUnk2C == -1 ) ? 1.0f : -1.0f;
	}
}

// $deBlob: FUNCTION 005c1450
void APointDataFunction::SetUnk20( TINT a_iUnk2C, TBOOL a_bSet )
{
	if ( m_pHeader == TNULL )
		return;

	for ( TINT i = m_pHeader->m_iNumPoints; i != 0; )
	{
		i--;
		Point& rPoint = m_pHeader->m_pPoints[ i ];

		if ( rPoint.m_iUnk2C == a_iUnk2C && IsRespawnPointType( rPoint.m_iType ) )
			rPoint.m_aUnk20[ 0 ] = a_bSet ? 1.0f : -1.0f;
	}
}
