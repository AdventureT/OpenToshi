#pragma once
#include "TVector4.h"
#include "TSphere.h"
#include "TMatrix44.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: an axis aligned box as its centre and half extents (0x20 bytes on the caller's stack)
//-----------------------------------------------------------------------------
class TAABox
{
public:
	TAABox() = default;

	TAABox( const TVector4& a_rcCentre, const TVector4& a_rcExtents )
	    : m_vCentre( a_rcCentre ), m_vExtents( a_rcExtents )
	{}

	// Does the sphere touch the box? (the squared distance from the sphere's centre to the box <= radius^2)
	TBOOL IntersectSphere( const TSphere& a_rcSphere ) const;

	// The point is inside the box (the faces included)
	TBOOL IsPointInside( const TVector4& a_rcPoint ) const;
	// The box around a_pBoxes
	void SetUnion( const TAABox* a_pBoxes, TUINT a_uiNumBoxes );
	// Where the box is after a_rcMatrix (the clip space test of its corners): 1 inside, 0 outside, 2 crossing
	TINT ClassifyClip( const TMatrix44& a_rcMatrix ) const;

public:
	TVector4 m_vCentre;  // 0x00
	TVector4 m_vExtents; // 0x10 the half extents
};

TOSHI_NAMESPACE_END
