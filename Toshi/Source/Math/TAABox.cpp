#include "ToshiPCH.h"
#include "TAABox.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0068b440
TBOOL TAABox::IntersectSphere( const TSphere& a_rcSphere ) const
{
	TFLOAT fMinX = m_vCentre.x - m_vExtents.x;
	TFLOAT fMaxX = m_vExtents.x + m_vCentre.x;
	TFLOAT fMinY = m_vCentre.y - m_vExtents.y;
	TFLOAT fMinZ = m_vCentre.z - m_vExtents.z;
	TFLOAT fMaxY = m_vExtents.y + m_vCentre.y;
	TFLOAT fMaxZ = m_vExtents.z + m_vCentre.z;

	const TVector3& rcOrigin = a_rcSphere.GetOrigin();

	// The vector from the box to the sphere's centre (0 on the axes where the centre is inside the box)
	TVector4 vDelta = TVector4::VEC_ZERO;

	if ( fMinX >= rcOrigin.x )
		vDelta.x = rcOrigin.x - fMinX;
	else if ( rcOrigin.x >= fMaxX )
		vDelta.x = rcOrigin.x - fMaxX;

	if ( fMinY >= rcOrigin.y )
		vDelta.y = rcOrigin.y - fMinY;
	else if ( rcOrigin.y >= fMaxY )
		vDelta.y = rcOrigin.y - fMaxY;

	if ( fMinZ >= rcOrigin.z )
		vDelta.z = rcOrigin.z - fMinZ;
	else if ( rcOrigin.z >= fMaxZ )
		vDelta.z = rcOrigin.z - fMaxZ;

	TFLOAT fRadius = a_rcSphere.GetRadius();
	return fRadius * fRadius >= vDelta.MagnitudeSq();
}

// $deBlob: FUNCTION 0068b530
TBOOL TAABox::IsPointInside( const TVector4& a_rcPoint ) const
{
	return a_rcPoint.x >= m_vCentre.x - m_vExtents.x && a_rcPoint.y >= m_vCentre.y - m_vExtents.y && a_rcPoint.z >= m_vCentre.z - m_vExtents.z &&
	    m_vExtents.x + m_vCentre.x >= a_rcPoint.x && m_vExtents.y + m_vCentre.y >= a_rcPoint.y && m_vExtents.z + m_vCentre.z >= a_rcPoint.z;
}

// $deBlob: FUNCTION 0068b640
void TAABox::SetUnion( const TAABox* a_pBoxes, TUINT a_uiNumBoxes )
{
	if ( a_pBoxes == TNULL || a_uiNumBoxes == 0 )
		return;

	TFLOAT fW = a_pBoxes[ 0 ].m_vCentre.w;

	TFLOAT fMinX = a_pBoxes[ 0 ].m_vCentre.x - a_pBoxes[ 0 ].m_vExtents.x;
	TFLOAT fMinY = a_pBoxes[ 0 ].m_vCentre.y - a_pBoxes[ 0 ].m_vExtents.y;
	TFLOAT fMinZ = a_pBoxes[ 0 ].m_vCentre.z - a_pBoxes[ 0 ].m_vExtents.z;
	TFLOAT fMaxX = a_pBoxes[ 0 ].m_vExtents.x + a_pBoxes[ 0 ].m_vCentre.x;
	TFLOAT fMaxY = a_pBoxes[ 0 ].m_vExtents.y + a_pBoxes[ 0 ].m_vCentre.y;
	TFLOAT fMaxZ = a_pBoxes[ 0 ].m_vExtents.z + a_pBoxes[ 0 ].m_vCentre.z;

	for ( TUINT i = 1; i < a_uiNumBoxes; i++ )
	{
		const TAABox& rcBox = a_pBoxes[ i ];

		TFLOAT fBoxMinX = rcBox.m_vCentre.x - rcBox.m_vExtents.x;
		TFLOAT fBoxMinY = rcBox.m_vCentre.y - rcBox.m_vExtents.y;
		TFLOAT fBoxMinZ = rcBox.m_vCentre.z - rcBox.m_vExtents.z;
		TFLOAT fBoxMaxX = rcBox.m_vCentre.x + rcBox.m_vExtents.x;
		TFLOAT fBoxMaxY = rcBox.m_vCentre.y + rcBox.m_vExtents.y;
		TFLOAT fBoxMaxZ = rcBox.m_vCentre.z + rcBox.m_vExtents.z;

		if ( fBoxMinX <= fMinX ) fMinX = fBoxMinX;
		if ( fBoxMinY <= fMinY ) fMinY = fBoxMinY;
		if ( fBoxMinZ <= fMinZ ) fMinZ = fBoxMinZ;
		if ( fMaxX <= fBoxMaxX ) fMaxX = fBoxMaxX;
		if ( fMaxY <= fBoxMaxY ) fMaxY = fBoxMaxY;
		if ( fMaxZ <= fBoxMaxZ ) fMaxZ = fBoxMaxZ;
	}

	// NOTE: both w components are the first box's centre w
	TVector4 vSum( fMaxX + fMinX, fMaxY + fMinY, fMaxZ + fMinZ, fW );
	m_vCentre.Multiply( vSum, 0.5f );
	m_vExtents = TVector4( fMaxX - m_vCentre.x, fMaxY - m_vCentre.y, fMaxZ - m_vCentre.z, fW );
}

// Where the box is after a_rcMatrix (the clip space test of its corners): 1 inside, 0 outside, 2 crossing
// $deBlob: FUNCTION 0068b8c0
TINT TAABox::ClassifyClip( const TMatrix44& a_rcMatrix ) const
{
	TFLOAT afMin[ 3 ] = { m_vCentre.x - m_vExtents.x, m_vCentre.y - m_vExtents.y, m_vCentre.z - m_vExtents.z };
	TFLOAT afMax[ 3 ] = { m_vCentre.x + m_vExtents.x, m_vExtents.y + m_vCentre.y, m_vExtents.z + m_vCentre.z };

	TUINT uiOr  = 0;
	TUINT uiAnd = 0xFFFF;

	for ( TINT i = 0; i < 8; i++ )
	{
		TVector4 vCorner( ( i & 1 ) ? afMin[ 0 ] : afMax[ 0 ], ( i & 2 ) ? afMin[ 1 ] : afMax[ 1 ], ( i & 4 ) ? afMin[ 2 ] : afMax[ 2 ], 1.0f );
		TVector4 vClip;
		TMatrix44::TransformVector( vClip, a_rcMatrix, vCorner );

		TUINT uiCode = 0;

		if ( !( -vClip.w <= vClip.x ) ) uiCode = 1;
		else if ( vClip.x > vClip.w ) uiCode = 2;

		if ( !( -vClip.w <= vClip.y ) ) uiCode |= 4;
		else if ( vClip.y > vClip.w ) uiCode |= 8;

		if ( !( -vClip.w <= vClip.z ) ) uiCode |= 0x20;
		else if ( vClip.z > vClip.w ) uiCode |= 0x10;

		uiOr |= uiCode;
		uiAnd &= uiCode;
	}

	if ( uiOr == 0 )
		return 1;

	if ( uiAnd != 0 )
		return 0;

	return 2;
}

TOSHI_NAMESPACE_END
