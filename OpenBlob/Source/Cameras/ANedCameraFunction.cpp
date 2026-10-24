#include "pch.h"
#include "ANedCameraFunction.h"
#include "ACurrentLevelInformation.h"
#include "Terrain2/ATerrain2.h"

#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00593380
void ANedCameraFunction::GetBoxCorners( TVector4* a_pOutCorners, const TVector4& a_rvCentre, const TVector4& a_rvAxisX, const TVector4& a_rvAxisY, const TVector4& a_rvAxisZ, const TFLOAT* a_pfHalfExtents )
{
	TVector4 vX, vY, vZ;
	vX.Multiply( a_rvAxisX, a_pfHalfExtents[ 0 ] );
	vY.Multiply( a_rvAxisY, a_pfHalfExtents[ 1 ] );
	vZ.Multiply( a_rvAxisZ, a_pfHalfExtents[ 2 ] );

	const TVector4& c = a_rvCentre;

	a_pOutCorners[ 0 ] = TVector4( ( ( c.x - vX.x ) - vY.x ) - vZ.x, ( ( c.y - vX.y ) - vY.y ) - vZ.y, ( ( c.z - vX.z ) - vY.z ) - vZ.z, c.w );
	a_pOutCorners[ 1 ] = TVector4( ( ( c.x + vX.x ) - vY.x ) - vZ.x, ( ( c.y + vX.y ) - vY.y ) - vZ.y, ( ( c.z + vX.z ) - vY.z ) - vZ.z, c.w );
	a_pOutCorners[ 2 ] = TVector4( ( ( c.x + vX.x ) + vY.x ) - vZ.x, ( ( c.y + vX.y ) + vY.y ) - vZ.y, ( ( c.z + vX.z ) + vY.z ) - vZ.z, c.w );
	a_pOutCorners[ 3 ] = TVector4( ( ( c.x - vX.x ) + vY.x ) - vZ.x, ( ( c.y - vX.y ) + vY.y ) - vZ.y, ( ( c.z - vX.z ) + vY.z ) - vZ.z, c.w );
	a_pOutCorners[ 4 ] = TVector4( ( ( c.x - vX.x ) - vY.x ) + vZ.x, ( ( c.y - vX.y ) - vY.y ) + vZ.y, ( ( c.z - vX.z ) - vY.z ) + vZ.z, c.w );
	a_pOutCorners[ 5 ] = TVector4( ( ( c.x + vX.x ) - vY.x ) + vZ.x, ( ( c.y + vX.y ) - vY.y ) + vZ.y, ( ( c.z + vX.z ) - vY.z ) + vZ.z, c.w );
	a_pOutCorners[ 6 ] = TVector4( ( ( c.x + vX.x ) + vY.x ) + vZ.x, ( ( c.y + vX.y ) + vY.y ) + vZ.y, ( ( c.z + vX.z ) + vY.z ) + vZ.z, c.w );
	a_pOutCorners[ 7 ] = TVector4( ( ( c.x - vX.x ) + vY.x ) + vZ.x, ( ( c.y - vX.y ) + vY.y ) + vZ.y, ( ( c.z - vX.z ) + vY.z ) + vZ.z, c.w );
}

// $deBlob: FUNCTION 00593740
TBOOL ANedCameraFunction::IsInViewAngle( const TVector4& a_rvBlobPos, const TVector4& a_rvCamPos, const TVector4& a_rvTarget, TFLOAT a_fAngle )
{
	if ( 0.0f < a_fAngle && a_fAngle < TMath::PI )
	{
		TVector4 vToBlob;
		vToBlob.w = a_rvBlobPos.w;
		vToBlob.x = a_rvBlobPos.x - a_rvTarget.x;
		vToBlob.z = a_rvBlobPos.z - a_rvTarget.z;

		if ( vToBlob.x != 0.0f || vToBlob.z != 0.0f )
		{
			vToBlob.y = 0.0f;

			TVector4 vToCam;
			vToCam.w = a_rvCamPos.w;
			vToCam.x = a_rvCamPos.x - a_rvTarget.x;
			vToCam.z = a_rvCamPos.z - a_rvTarget.z;

			if ( vToCam.x != 0.0f || vToCam.z != 0.0f )
			{
				vToCam.y = 0.0f;
				return vToCam.GetAngleBetweenVector( vToBlob ) <= a_fAngle;
			}
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00593840
void ANedCameraFunction::ResolveInstances()
{
	if ( m_pHeader == TNULL )
		return;

	for ( TINT i = m_pHeader->m_iNumRegions; i != 0; )
	{
		i--;
		Region& rRegion = m_pHeader->m_pRegions[ i ];

		for ( TINT j = rRegion.m_iNumFixedCams; j != 0; )
		{
			j--;
			FixedCam& rFixedCam = rRegion.m_pFixedCams[ j ];

			for ( TINT k = rFixedCam.m_iNumInstances; k != 0; )
			{
				k--;
				TINT iInstance               = TINT( TREINTERPRETCAST( TUINTPTR, rFixedCam.m_ppInstances[ k ] ) );
				rFixedCam.m_ppInstances[ k ] = ATerrain2::GetInstance()->GetCellInstance( iInstance );
			}
		}
	}
}

// $deBlob: FUNCTION 00593940
ANedCameraFunction::Region* ANedCameraFunction::GetSmallestCylinder( TINT a_iIndex, const TVector4& a_rvPos, TUINT32 a_uiFlags ) const
{
	Region* pRegion = &m_pHeader->m_pRegions[ a_iIndex ];

	if ( !( pRegion->m_uiFlags & RS_CYLINDER ) )
		return pRegion;

	TINT   iBest   = a_iIndex;
	TFLOAT fRadius = pRegion->m_pShape->m_avAxes[ 0 ].x;

	for ( TINT i = m_pHeader->m_iNumRegions; i != 0; )
	{
		i--;

		if ( i == a_iIndex )
			continue;

		Region& rRegion = m_pHeader->m_pRegions[ i ];

		if ( ( rRegion.m_uiFlags & RS_CYLINDER ) && ( a_uiFlags & rRegion.m_uiFlags ) == a_uiFlags && Contains( rRegion, a_rvPos ) &&
		     fRadius > rRegion.m_pShape->m_avAxes[ 0 ].x )
		{
			fRadius = rRegion.m_pShape->m_avAxes[ 0 ].x;
			iBest   = i;
		}
	}

	return &m_pHeader->m_pRegions[ iBest ];
}

// $deBlob: FUNCTION 00593a20
void ANedCameraFunction::Load()
{
	m_pHeader        = TNULL;
	m_pCurrentRegion = TNULL;

	TTRB* pTRB = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_NEDCamera );
	m_pHeader  = TSTATICCAST( Header, pTRB->GetSymbolAddress( "CameraHeader" ) );
	ResolveInstances();

	if ( m_pHeader == TNULL )
		return;

	for ( TINT i = m_pHeader->m_iNumRegions; i != 0; )
	{
		i--;
		m_pHeader->m_pRegions[ i ].m_uiFlags |= RS_ENABLED;
	}
}

// $deBlob: FUNCTION 00593a90
ANedCameraFunction::Region* ANedCameraFunction::FindRegion( TINT a_iID ) const
{
	if ( m_pHeader == TNULL )
		return TNULL;

	for ( TINT i = m_pHeader->m_iNumRegions; i != 0; )
	{
		i--;

		if ( m_pHeader->m_pRegions[ i ].m_iID == a_iID )
			return &m_pHeader->m_pRegions[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 00593ad0
void ANedCameraFunction::SetRegionEnabled( TINT a_iID, TBOOL a_bEnabled )
{
	if ( m_pHeader == TNULL )
		return;

	for ( TINT i = m_pHeader->m_iNumRegions; i != 0; )
	{
		i--;
		Region& rRegion = m_pHeader->m_pRegions[ i ];

		if ( rRegion.m_iID != a_iID )
			continue;

		if ( a_bEnabled )
		{
			rRegion.m_uiFlags |= RS_ENABLED;

			for ( TINT j = rRegion.m_iNumFixedCams; j != 0; )
			{
				j--;

				if ( rRegion.m_pFixedCams[ j ].m_iFLAGS & RS_UNK40000 )
					rRegion.m_pFixedCams[ j ].m_iNumUses = 0;
			}
		}
		else
		{
			rRegion.m_uiFlags &= ~RS_ENABLED;
		}

		return;
	}
}

// $deBlob: FUNCTION 00594120
void ANedCameraFunction::ResetUses()
{
	if ( m_pHeader == TNULL )
		return;

	for ( TINT i = m_pHeader->m_iNumRegions; i != 0; )
	{
		i--;
		Region& rRegion = m_pHeader->m_pRegions[ i ];

		for ( TINT j = rRegion.m_iNumFixedCams; j != 0; )
		{
			j--;
			rRegion.m_pFixedCams[ j ].m_iNumUses = 0;
		}

		for ( TINT j = rRegion.m_iNumSSCamTriggers; j != 0; )
		{
			j--;
			rRegion.m_pSSCamTriggers[ j ].m_iCount = 0;
		}
	}
}

// $deBlob: FUNCTION 00593b60
void ANedCameraFunction::SetAllRegionsEnabled( TBOOL a_bEnabled )
{
	if ( m_pHeader == TNULL )
		return;

	for ( TINT i = m_pHeader->m_iNumRegions; i != 0; )
	{
		i--;

		if ( a_bEnabled )
			m_pHeader->m_pRegions[ i ].m_uiFlags |= RS_ENABLED;
		else
			m_pHeader->m_pRegions[ i ].m_uiFlags &= ~RS_ENABLED;
	}
}

// $deBlob: FUNCTION 00593bb0
ANedCameraFunction::Region* ANedCameraFunction::FindRegionAt( const TVector4& a_rvPos, TUINT32 a_uiFlags ) const
{
	if ( m_pHeader == TNULL )
		return TNULL;

	TUINT32 uiFlags = a_uiFlags | RS_ENABLED;

	for ( TINT i = m_pHeader->m_iNumRegions; i != 0; )
	{
		i--;
		const Region& rRegion = m_pHeader->m_pRegions[ i ];

		if ( ( uiFlags & rRegion.m_uiFlags ) == uiFlags && Contains( rRegion, a_rvPos ) )
			return GetSmallestCylinder( i, a_rvPos, a_uiFlags );
	}

	return TNULL;
}

// $deBlob: FUNCTION 00593d90
ANedCameraFunction::SSCamTrigger* ANedCameraFunction::FindSSCamTriggerAt( const TVector4& a_rvPos, TUINT32 a_uiFlags ) const
{
	TUINT32 uiFlags = a_uiFlags | 0x10080;
	Region* pRegion = FindRegionAt( a_rvPos, uiFlags );

	if ( pRegion == TNULL )
		return TNULL;

	for ( TINT i = pRegion->m_iNumSSCamTriggers; i != 0; )
	{
		i--;
		SSCamTrigger* pEntry = &pRegion->m_pSSCamTriggers[ i ];

		if ( ( uiFlags & pEntry->m_uiFlags ) == uiFlags && ( pEntry->m_iMaxUses == -1 || pEntry->m_iCount < pEntry->m_iMaxUses ) )
			return pEntry;
	}

	return TNULL;
}

// $deBlob: FUNCTION 00593df0
ANedCameraFunction::SplineCam* ANedCameraFunction::FindSplineCamAt( const TVector4& a_rvPos, TUINT32 a_uiFlags ) const
{
	TUINT32 uiFlags = a_uiFlags | 0x10100;
	Region* pRegion = FindRegionAt( a_rvPos, uiFlags );

	if ( pRegion == TNULL )
		return TNULL;

	for ( TINT i = pRegion->m_iNumSplineCams; i != 0; )
	{
		i--;
		SplineCam* pEntry = &pRegion->m_pSplineCams[ i ];

		if ( ( uiFlags & pEntry->m_uiFlags ) == uiFlags )
			return pEntry;
	}

	return TNULL;
}

// $deBlob: FUNCTION 005941d0
void ANedCameraFunction::SetCurrentRegion( TINT a_iID )
{
	if ( a_iID < 0 )
	{
		m_pCurrentRegion = TNULL;
		return;
	}

	if ( m_pHeader == TNULL )
		return;

	for ( TINT i = m_pHeader->m_iNumRegions; i != 0; )
	{
		i--;
		if ( m_pHeader->m_pRegions[ i ].m_iID == a_iID )
		{
			m_pCurrentRegion = &m_pHeader->m_pRegions[ i ];
			return;
		}
	}
}

// $deBlob: FUNCTION 00594230
ANedCameraFunction::FixedCam* ANedCameraFunction::FindVistaCam( TINT a_iVista ) const
{
	if ( m_pHeader == TNULL )
		return TNULL;

	for ( TINT i = m_pHeader->m_iNumRegions; i != 0; )
	{
		i--;
		Region& rRegion = m_pHeader->m_pRegions[ i ];

		for ( TINT j = rRegion.m_iNumFixedCams; j != 0; )
		{
			j--;
			FixedCam& rCam = rRegion.m_pFixedCams[ j ];

			if ( ( rCam.m_iFLAGS & FixedCam::FixedCam_Vista ) != 0 && rCam.m_iVista == a_iVista )
				return &rCam;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 00593e40
ANedCameraFunction::SSCamTrigger* ANedCameraFunction::FindCurrentSSCamTrigger() const
{
	if ( m_pCurrentRegion == TNULL )
		return TNULL;

	for ( TINT i = m_pCurrentRegion->m_iNumSSCamTriggers; i != 0; )
	{
		i--;
		SSCamTrigger* pEntry = &m_pCurrentRegion->m_pSSCamTriggers[ i ];

		if ( pEntry->m_iMaxUses == -1 || pEntry->m_iCount < pEntry->m_iMaxUses )
			return pEntry;
	}

	return TNULL;
}

// $deBlob: FUNCTION 00593e70
ANedCameraFunction::FixedCam* ANedCameraFunction::FindFixedCamAt( const TVector4& a_rvTarget, const TVector4& a_rvBlobPos, TUINT32 a_uiFlags ) const
{
	return FindFixedCam( a_rvTarget, a_rvBlobPos, FindRegionAt( a_rvTarget, a_uiFlags ), a_uiFlags );
}

// $deBlob: FUNCTION 00593ea0
ANedCameraFunction::FixedCam* ANedCameraFunction::FindFixedCam( const TVector4& a_rvTarget, const TVector4& a_rvBlobPos, Region* a_pRegion, TUINT32 a_uiFlags )
{
	if ( a_pRegion == TNULL )
		return TNULL;

	TUINT32   uiFlags     = a_uiFlags | RS_ENABLED;
	FixedCam* pBest       = TNULL;
	TFLOAT    fBestDistSq = TMath::MAXFLOAT;

	for ( TINT i = 0; i < a_pRegion->m_iNumFixedCams; i++ )
	{
		FixedCam* pFixedCam = &a_pRegion->m_pFixedCams[ i ];

		if ( ( uiFlags & pFixedCam->m_iFLAGS ) == uiFlags && ( pFixedCam->m_iMaxUses == -1 || pFixedCam->m_iNumUses < pFixedCam->m_iMaxUses ) )
		{
			TFLOAT fDistSq = TVector4::DistanceSq( pFixedCam->m_vPosition, a_rvTarget );

			if ( fDistSq < fBestDistSq && IsInViewAngle( a_rvBlobPos, pFixedCam->m_vPosition, a_rvTarget, pFixedCam->m_fViewAngle ) )
			{
				fBestDistSq = fDistSq;
				pBest       = pFixedCam;
			}
		}
	}

	return pBest;
}

// $deBlob: FUNCTION 00593f80
ANedCameraFunction::FixedCam* ANedCameraFunction::FindCurrentFixedCam( const TVector4& a_rvTarget, const TVector4& a_rvBlobPos ) const
{
	if ( m_pCurrentRegion == TNULL )
		return TNULL;

	return FindFixedCam( a_rvTarget, a_rvBlobPos, m_pCurrentRegion, m_pCurrentRegion->m_uiFlags );
}

// $deBlob: FUNCTION 00593fb0
TBOOL ANedCameraFunction::Contains( const Region& a_rRegion, const TVector4& a_rvPos )
{
	const Shape* pShape = a_rRegion.m_pShape;

	if ( a_rRegion.m_uiFlags & RS_CYLINDER )
	{
		// -Y is up: from the top down by the height
		if ( a_rvPos.y <= pShape->m_vPosition.y && pShape->m_vPosition.y - pShape->m_avAxes[ 0 ].y <= a_rvPos.y )
		{
			TVector4 vOffset = a_rvPos;
			vOffset.x -= pShape->m_vPosition.x;
			vOffset.y -= pShape->m_vPosition.y;
			vOffset.z -= pShape->m_vPosition.z;

			TFLOAT fRadius = pShape->m_avAxes[ 0 ].x;
			return vOffset.MagnitudeSqXZ() <= fRadius * fRadius;
		}

		return TFALSE;
	}

	TFLOAT fX = a_rvPos.x - pShape->m_vPosition.x;
	TFLOAT fY = a_rvPos.y - pShape->m_vPosition.y;
	TFLOAT fZ = a_rvPos.z - pShape->m_vPosition.z;

	if ( TMath::Abs( pShape->m_avAxes[ 0 ].y * fY + pShape->m_avAxes[ 0 ].x * fX + pShape->m_avAxes[ 0 ].z * fZ ) > pShape->m_afHalfSize[ 0 ] ||
	     TMath::Abs( pShape->m_avAxes[ 1 ].y * fY + fX * pShape->m_avAxes[ 1 ].x + pShape->m_avAxes[ 1 ].z * fZ ) > pShape->m_afHalfSize[ 1 ] )
	{
		return TFALSE;
	}

	return TMath::Abs( pShape->m_avAxes[ 2 ].y * fY + pShape->m_avAxes[ 2 ].x * fX + pShape->m_avAxes[ 2 ].z * fZ ) <= pShape->m_afHalfSize[ 2 ];
}
