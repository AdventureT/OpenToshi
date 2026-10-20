#include "pch.h"
#include "AEntityHash.h"
#include "AEntityHashFilter.h"
#include "ABaseEntity.h"
#include "AEntityFactory.h"

#include <File/TFile.h>
#include <GUI/T2GUI.h>
#include <GUI/T2GUIRenderer.h>

#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TUINT32 AEntityHash::ms_uiTimeStamp = 1;

// NOTE: retail truncates and subtracts one for negative values (an exact negative integer goes one cell too far)
// NOTE: retail converts with fistp (the FPU's rounding to nearest), not a truncation
static TINT AEntityHash_CellIndex( TFLOAT a_fValue )
{
	return ( a_fValue >= 0.0f ) ? TINT( std::lrintf( a_fValue ) ) : TINT( std::lrintf( a_fValue - 1.0f ) );
}

// An ARGB colour from 0-1 components
// $deBlob: FUNCTION 00773fb0
static TUINT32 MakeColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha )
{
	return TUINT32( TINT( a_fBlue * 255.0f ) ) |
	    ( ( TUINT32( TINT( a_fGreen * 255.0f ) ) | ( ( ( TUINT32( TINT( a_fAlpha * 255.0f ) ) << 8 ) | TUINT32( TINT( a_fRed * 255.0f ) ) ) << 8 ) ) << 8 );
}

// $deBlob: FUNCTION 007741e0
TINT AEntityHash::GetCellX( TFLOAT a_fX ) const
{
	return AEntityHash_CellIndex( m_fInvCellWidth * a_fX );
}

// $deBlob: FUNCTION 00774230
TINT AEntityHash::GetCellY( TFLOAT a_fY ) const
{
	return AEntityHash_CellIndex( m_fInvCellHeight * a_fY );
}

TSTATICASSERT( sizeof( AEntityHash ) == 0x29014 );

// $deBlob: FUNCTION 00773070
AEntityHash::AEntityHash()
{
	m_pFactory = TNULL;
	m_fXMax    = 0.0f;
	m_fYMax    = 0.0f;
	m_iXDiv    = 0;
	m_iYDiv    = 0;
	m_fXOffset = 0.0f;
	m_fYOffset = 0.0f;

	for ( TINT i = 0; i < AUNITHASH_BUCKET_COUNT; i++ )
		m_aBuckets[ i ].m_iTimeStamp = 0;

	for ( TINT i = 0; i < MAX_REFS; i++ )
		m_aRefs[ i ].LinkAfter( &m_FreeListRoot );
}

// $deBlob: FUNCTION 00773260
void AEntityHash::Init( AEntityFactory* a_pFactory, TFLOAT a_fXMin, TFLOAT a_fYMin, TFLOAT a_fXMax, TFLOAT a_fYMax, TINT a_iXDiv, TINT a_iYDiv )
{
	m_pFactory = a_pFactory;
	m_iXDiv    = a_iXDiv;
	m_fYMax    = a_fYMax;
	m_fXMax    = a_fXMax;
	m_iYDiv    = a_iYDiv;

	TASSERT( m_fXMax != 0 );
	TASSERT( m_fYMax != 0 );
	TASSERT( m_iXDiv != 0 );
	TASSERT( m_iYDiv != 0 );

	TFLOAT fRangeX = a_fXMax - a_fXMin;
	TFLOAT fRangeY = a_fYMax - a_fYMin;
	TASSERT( fRangeX >= 0 );
	TASSERT( fRangeY >= 0 );

	m_fCellWidth     = fRangeX / TFLOAT( m_iXDiv );
	m_fInvCellWidth  = 1.0f / m_fCellWidth;
	m_fInvCellHeight = 1.0f / ( fRangeY / TFLOAT( m_iYDiv ) );
	m_fCellHeight    = fRangeY / TFLOAT( m_iYDiv );

	if ( a_fXMin < 0.0f )
		m_fXOffset = -a_fXMin;

	if ( a_fYMin < 0.0f )
		m_fYOffset = -a_fYMin;
}

// $deBlob: FUNCTION 00773470
AEntityHashRef* AEntityHash::Add( ABaseEntity& a_roUnit, TBOOL a_bUnk )
{
	if ( !( a_roUnit.GetFlags() & ABaseEntity::FLAG_Hashed ) )
		return TNULL;

	Remove( a_roUnit );

	TVector4 vPos    = a_roUnit.GetBasePosition();
	TFLOAT   fRadius = a_roUnit.GetBoundingSphereRadius();

	TINT iX0 = AEntityHash_CellIndex( ( vPos.x - fRadius + m_fXOffset ) * m_fInvCellWidth );
	TINT iX1 = AEntityHash_CellIndex( m_fInvCellWidth * ( vPos.x + fRadius + m_fXOffset ) ) + 1;
	TINT iY0 = AEntityHash_CellIndex( m_fInvCellHeight * ( vPos.z - fRadius + m_fYOffset ) );
	TINT iY1 = AEntityHash_CellIndex( m_fInvCellHeight * ( vPos.z + fRadius + m_fYOffset ) ) + 1;

	TBOOL           bPickup = ( a_roUnit.GetFlags() & ABaseEntity::FLAG_Pickup ) != 0;
	AEntityHashRef* pFirst  = TNULL;

	for ( TINT iY = iY0; iY < iY1; iY++ )
	{
		for ( TINT iX = iX0; iX < iX1; iX++ )
		{
			TINT bucketIndex = ( iY * 65 - iX * 189 ) & ( AUNITHASH_BUCKET_COUNT - 1 );
			TASSERT( bucketIndex >= 0 && bucketIndex < AUNITHASH_BUCKET_COUNT );
			Bucket* pBucket = &m_aBuckets[ bucketIndex ];

			// Already in this bucket (the hash wraps around)?
			AEntityHashRef* pRef = pFirst;
			while ( pRef != TNULL && pRef->m_pBucket != pBucket )
				pRef = pRef->m_pNextRef;

			if ( pRef != TNULL )
				continue;

			AEntityHashRef* pRoot = bPickup ? &pBucket->m_oPickups : &pBucket->m_oEntities;

			TASSERT( m_FreeListRoot.m_Next != &m_FreeListRoot, "Entity hash ref pool empty" );
			AEntityHashRef* pNewRef = m_FreeListRoot.m_Next;
			pNewRef->Unlink();
			pNewRef->LinkAfter( pRoot );
			pNewRef->m_pNextRef = pFirst;
			pNewRef->m_pEntity  = &a_roUnit;
			pNewRef->m_pBucket  = pBucket;
			pFirst              = pNewRef;
		}
	}

	TASSERT( a_roUnit.GetHashRef() == TNULL );
	a_roUnit.SetHashRef( pFirst );
	return pFirst;
}

// $deBlob: FUNCTION 007737f0
void AEntityHash::Remove( ABaseEntity& a_roUnit )
{
	AEntityHashRef* pRef = a_roUnit.GetHashRef();

	while ( pRef != TNULL )
	{
		AEntityHashRef* pCurRef = pRef;
		pCurRef->Unlink();
		pRef = pRef->m_pNextRef;
		pCurRef->LinkAfter( &m_FreeListRoot );
		a_roUnit.SetHashRef( TNULL );
	}
}

// $deBlob: FUNCTION 007738d0
TUINT32 AEntityHash::NextTimeStamp()
{
	TUINT32 uiOld = ms_uiTimeStamp++;

	if ( uiOld > ms_uiTimeStamp )
	{
		ms_uiTimeStamp = 1;

		if ( m_pFactory )
			m_pFactory->SetAllUnitTimeStamps( 0 );
	}

	return ms_uiTimeStamp;
}

// $deBlob: FUNCTION 00773c80
void AEntityHash::DumpBucketCounts( const TCHAR* a_szFileName )
{
	TFile* pFile = TFile::Create( a_szFileName, TFILEMODE_CREATENEW );

	if ( pFile != TNULL )
	{
		for ( TINT i = 0; i < AUNITHASH_BUCKET_COUNT; i++ )
		{
			Bucket& rBucket = m_aBuckets[ i ];

			TINT iNumEntities = 0;
			for ( AEntityHashRef* pRef = rBucket.m_oEntities.m_Next; pRef != &rBucket.m_oEntities; pRef = pRef->m_Next )
				iNumEntities++;

			TINT iNumPickups = 0;
			for ( AEntityHashRef* pRef = rBucket.m_oPickups.m_Next; pRef != &rBucket.m_oPickups; pRef = pRef->m_Next )
				iNumPickups++;

			pFile->CPrintf( "%i,%i,%i\n", iNumPickups + iNumEntities, iNumEntities, iNumPickups );
		}

		pFile->Destroy();
	}
}

// $deBlob: FUNCTION 00773d40
void AEntityHash::DebugRenderBucketCounts()
{
	T2GUIRenderer* pRenderer = T2GUI::GetSingleton()->GetRenderer();
	TINT           iX        = -256;

	for ( TINT i = 0; i < AUNITHASH_BUCKET_COUNT; i++ )
	{
		Bucket& rBucket = m_aBuckets[ i ];

		TINT iNumEntities = 0;
		for ( AEntityHashRef* pRef = rBucket.m_oEntities.m_Next; pRef != &rBucket.m_oEntities; pRef = pRef->m_Next )
			iNumEntities++;

		TINT iNumPickups = 0;
		for ( AEntityHashRef* pRef = rBucket.m_oPickups.m_Next; pRef != &rBucket.m_oPickups; pRef = pRef->m_Next )
			iNumPickups++;

		pRenderer->SetColour( 0x7FFFFFFF );
		pRenderer->RenderLine( TFLOAT( iX ), -100.0f, TFLOAT( iX ), -100.0f - TFLOAT( iNumPickups + iNumEntities ) );
		pRenderer->SetColour( 0x7FFF0000 );
		pRenderer->RenderLine( TFLOAT( iX ), 0.0f, TFLOAT( iX ), TFLOAT( -iNumEntities ) );
		pRenderer->SetColour( 0x7F00FF00 );
		pRenderer->RenderLine( TFLOAT( iX ), 100.0f, TFLOAT( iX ), 100.0f - TFLOAT( iNumPickups ) );

		iX++;
	}
}

// $deBlob: FUNCTION 00773ea0
TBOOL AEntityHash::QueryList( AEntityHashRef* a_pRoot, const TVector4& a_rvPos, TFLOAT a_fRadius, TBOOL a_bUse3D, TINT* a_piCount, TINT a_iMax, AEntityHashFilter* a_pFilter )
{
	TUINT32 uiStamp = ms_uiTimeStamp;

	for ( AEntityHashRef* pRef = a_pRoot->m_Next; pRef != a_pRoot; pRef = pRef->m_Next )
	{
		ABaseEntity* pEntity = pRef->m_pEntity;

		// Each entity once per query
		if ( pEntity->GetHashTimeStamp() == uiStamp )
			continue;

		pEntity->SetHashTimeStamp( uiStamp );

		if ( pEntity->GetFlags() & ABaseEntity::FLAG_Despawned )
			continue;

		if ( a_pFilter != TNULL && !( *a_pFilter )( pEntity ) )
			continue;

		TVector4 vOffset = pEntity->GetBasePosition();
		vOffset.x -= a_rvPos.x;
		vOffset.y -= a_rvPos.y;
		vOffset.z -= a_rvPos.z;

		TFLOAT fDistanceSq = a_bUse3D ? vOffset.MagnitudeSq() : vOffset.MagnitudeSqXZ();
		TFLOAT fReach      = pEntity->GetBoundingSphereRadius() + a_fRadius;

		if ( fReach * fReach > fDistanceSq )
		{
			m_apQueryResults[ ( *a_piCount )++ ] = pEntity;

			if ( *a_piCount == a_iMax )
				return TFALSE;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00773910
ABaseEntity** AEntityHash::Query( const TVector4& a_rvPos, TFLOAT a_fRadius, TINT* a_piCount, AEntityHashFilter* a_pFilter, TBOOL a_bUse3D, TINT a_iMax, TBOOL a_bBruteForce, TBOOL a_bIgnorePickups, TBOOL a_bIgnoreEverythingElse )
{
	// The asserts are once per process (retail's dword flags)
	static TBOOL s_bAssertedIgnoreBoth = TFALSE;
	static TBOOL s_bAssertedBruteForce = TFALSE;
	static TBOOL s_bAssertedRadius     = TFALSE;

	if ( !s_bAssertedIgnoreBoth && a_bIgnorePickups && a_bIgnoreEverythingElse )
	{
		TASSERT( TFALSE && "Cant Ignore Everything..NO Point IN calling this function !" );
		s_bAssertedIgnoreBoth = TTRUE;
	}

	if ( !s_bAssertedBruteForce && a_bBruteForce )
	{
		TASSERT( TFALSE && "Brute force depricated" );
		s_bAssertedBruteForce = TTRUE;
	}

	if ( !s_bAssertedRadius && ( a_fRadius <= 0.0f || a_fRadius > 1000.0f ) )
	{
		TASSERT( TFALSE && "Incorrect entity radius! This MUST fit the entity correctly" );
		s_bAssertedRadius = TTRUE;
	}

	TUINT32 uiStamp = NextTimeStamp();

	*a_piCount = 0;

	TINT iX0 = AEntityHash_CellIndex( ( a_rvPos.x - a_fRadius + m_fXOffset ) * m_fInvCellWidth );
	TINT iX1 = AEntityHash_CellIndex( m_fInvCellWidth * ( a_rvPos.x + a_fRadius + m_fXOffset ) ) + 1;
	TINT iY0 = AEntityHash_CellIndex( m_fInvCellHeight * ( a_rvPos.z - a_fRadius + m_fYOffset ) );
	TINT iY1 = AEntityHash_CellIndex( m_fInvCellHeight * ( a_rvPos.z + a_fRadius + m_fYOffset ) ) + 1;

	for ( TINT iY = iY0; iY < iY1; iY++ )
	{
		for ( TINT iX = iX0; iX < iX1; iX++ )
		{
			TINT    iBucket = ( iY * 65 - iX * 189 ) & ( AUNITHASH_BUCKET_COUNT - 1 );
			Bucket* pBucket = &m_aBuckets[ iBucket ];

			// A bucket is searched once per query
			if ( pBucket->m_iTimeStamp == TINT( uiStamp ) )
				continue;

			pBucket->m_iTimeStamp = TINT( uiStamp );

			if ( !a_bIgnorePickups && pBucket->m_oPickups.IsLinked() &&
			     !QueryList( &pBucket->m_oPickups, a_rvPos, a_fRadius, a_bUse3D, a_piCount, a_iMax, a_pFilter ) )
			{
				return m_apQueryResults;
			}

			if ( !a_bIgnoreEverythingElse && pBucket->m_oEntities.IsLinked() &&
			     !QueryList( &pBucket->m_oEntities, a_rvPos, a_fRadius, a_bUse3D, a_piCount, a_iMax, a_pFilter ) )
			{
				return m_apQueryResults;
			}
		}
	}

	return m_apQueryResults;
}

// $deBlob: FUNCTION 0059cf20
TBOOL AEntityHashFilterOnlyThisType::operator()( ABaseEntity* a_pEntity )
{
	if ( m_pClass == a_pEntity->GetClass() )
		return TTRUE;

	return TFALSE;
}
