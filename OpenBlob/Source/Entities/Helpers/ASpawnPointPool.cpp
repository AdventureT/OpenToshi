#include "pch.h"
#include "ASpawnPointPool.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ASpawnPointPool::SpawnPoint ) == 0x1C );
TSTATICASSERT( sizeof( ASpawnPointPool ) == 0x18 );

// $deBlob: FUNCTION 007c0020
ASpawnPointPool::ASpawnPointPool( TVector4* const& a_rpPoints, TUINT8 a_uiNumPoints )
{
	m_pPoints     = new SpawnPoint[ a_uiNumPoints ];
	m_iNumPoints  = a_uiNumPoints;
	m_iFreePoints = a_uiNumPoints;

	for ( TUINT i = 0; i < a_uiNumPoints; i++ )
	{
		m_pPoints[ i ].m_vPosition = a_rpPoints[ i ];
		m_pPoints[ i ].m_pOwner    = this;
		m_lFreePoints.PushBack( &m_pPoints[ i ] );
	}
}

// $deBlob: FUNCTION 007b5b00
// $deBlob: FUNCTION 007c0120
ASpawnPointPool::~ASpawnPointPool()
{
	if ( m_pPoints != TNULL )
	{
		delete[] m_pPoints;
		m_pPoints = TNULL;
	}
}

// $deBlob: FUNCTION 007c01b0
void ASpawnPointPool::ReleaseAll()
{
	if ( !m_lUsedPoints.IsEmpty() )
		m_lFreePoints.Splice( m_lFreePoints.End(), m_lUsedPoints );

	m_iFreePoints = m_iNumPoints;
}

// $deBlob: FUNCTION 007c01f0
ASpawnPointPool::SpawnPoint* ASpawnPointPool::GetSpawnPoint()
{
	if ( m_lFreePoints.IsEmpty() )
		return TNULL;

	SpawnPoint* pPoint = m_lFreePoints.Head();
	pPoint->Remove();
	m_iFreePoints--;
	m_lUsedPoints.PushBack( pPoint );

	return pPoint;
}

// $deBlob: FUNCTION 007c0250
ASpawnPointPool::SpawnPoint* ASpawnPointPool::GetSpawnPoint( const TVector4& a_rPosition, const TVector4& a_vDirection )
{
	if ( m_lFreePoints.IsEmpty() )
		return TNULL;

	TASSERT( a_vDirection.IsNormalised() );

	auto        it     = m_lFreePoints.Begin();
	SpawnPoint* pBest  = it;

	TVector4 vToPoint;
	vToPoint.x = pBest->m_vPosition.x - a_rPosition.x;
	vToPoint.y = pBest->m_vPosition.y - a_rPosition.y;
	vToPoint.z = pBest->m_vPosition.z - a_rPosition.z;
	vToPoint.w = pBest->m_vPosition.w;
	vToPoint.Normalise();

	TFLOAT fBestDot = a_vDirection.y * vToPoint.y + a_vDirection.x * vToPoint.x + a_vDirection.z * vToPoint.z;

	for ( it = it.Next(); it != m_lFreePoints.End(); it = it.Next() )
	{
		vToPoint.x = it->m_vPosition.x - a_rPosition.x;
		vToPoint.y = it->m_vPosition.y - a_rPosition.y;
		vToPoint.z = it->m_vPosition.z - a_rPosition.z;
		vToPoint.w = it->m_vPosition.w;
		vToPoint.Normalise();

		TFLOAT fDot = a_vDirection.y * vToPoint.y + a_vDirection.x * vToPoint.x + a_vDirection.z * vToPoint.z;

		if ( fBestDot < fDot )
		{
			pBest    = it;
			fBestDot = fDot;
		}
	}

	pBest->Remove();
	m_iFreePoints--;
	m_lUsedPoints.PushBack( pBest );

	return pBest;
}

// $deBlob: FUNCTION 007c04b0
void ASpawnPointPool::ReleaseSpawnPoint( SpawnPoint* a_pSpawnPoint )
{
	TASSERT( a_pSpawnPoint->m_pOwner == this && "Trying to release a spawn point that this pool does not own." );
	TASSERT( m_lUsedPoints.IsInList( a_pSpawnPoint ) && "Trying to release a spawn point that is not in use." );

	a_pSpawnPoint->Remove();
	m_lFreePoints.PushBack( a_pSpawnPoint );
	m_iFreePoints++;

	TASSERT( m_iFreePoints <= m_iNumPoints );
}

// $deBlob: FUNCTION 007c0580
void ASpawnPointPool::ReleaseSpawnPointCallback( APickup* a_pPickup, void* a_pSpawnPoint )
{
	SpawnPoint* pSpawnPoint = TSTATICCAST( SpawnPoint, a_pSpawnPoint );
	pSpawnPoint->m_pOwner->ReleaseSpawnPoint( pSpawnPoint );
}
