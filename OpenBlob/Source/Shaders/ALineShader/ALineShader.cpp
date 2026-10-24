#include "pch.h"
#include "ALineShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

//-----------------------------------------------------------------------------
// ALineShader
//-----------------------------------------------------------------------------

// NOTE: abstract; retail's CreateObject/CreateObjectInPlace return TNULL without an assert
// $deBlob: FUNCTION 0053cc10
// $deBlob: FUNCTION 005fdf00
// $deBlob: FUNCTION 005fdf10
// $deBlob: FUNCTION 005fdf20
// $deBlob: FUNCTION 005fdf30
// $deBlob: FUNCTION 005fdf40
// $deBlob: FUNCTION 00858680
TDEFINE_CLASS_NORUNTIME( ALineShader );

// $deBlob: FUNCTION 005fdf50
// $deBlob: FUNCTION 005feba0
ALineShader::~ALineShader()
{
}

// $deBlob: FUNCTION 005fdfb0
TBOOL ALineShader::Create()
{
	return TShader::Create();
}

// $deBlob: FUNCTION 005fdfc0
void ALineShader::AddBox( const TVector4& a_rCentre, const TVector4& a_rExtents, TUINT32 a_uiColour, const TMatrix44* a_pRotation )
{
	TVector4 vAxisX( a_rExtents.x, 0.0f, 0.0f, 1.0f );
	TVector4 vAxisY( 0.0f, a_rExtents.y, 0.0f, 1.0f );
	TVector4 vAxisZ( 0.0f, 0.0f, a_rExtents.z, 1.0f );

	if ( a_pRotation != TNULL )
	{
		TMatrix44::RotateVector( vAxisX, *a_pRotation, vAxisX );
		TMatrix44::RotateVector( vAxisY, *a_pRotation, vAxisY );
		TMatrix44::RotateVector( vAxisZ, *a_pRotation, vAxisZ );
	}

	// The centres of the two faces along X, and the diagonals of the faces
	TFLOAT fPosX = a_rCentre.x + vAxisX.x, fPosY = a_rCentre.y + vAxisX.y, fPosZ = a_rCentre.z + vAxisX.z;
	TFLOAT fNegX = a_rCentre.x - vAxisX.x, fNegY = a_rCentre.y - vAxisX.y, fNegZ = a_rCentre.z - vAxisX.z;
	TFLOAT fSumX = vAxisZ.x + vAxisY.x, fSumY = vAxisZ.y + vAxisY.y, fSumZ = vAxisZ.z + vAxisY.z;
	TFLOAT fDifX = vAxisZ.x - vAxisY.x, fDifY = vAxisZ.y - vAxisY.y, fDifZ = vAxisZ.z - vAxisY.z;

	TVector4 aCorners[ 8 ] = {
		TVector4( fNegX - fSumX, fNegY - fSumY, fNegZ - fSumZ, a_rCentre.w ),
		TVector4( fDifX + fNegX, fDifY + fNegY, fDifZ + fNegZ, a_rCentre.w ),
		TVector4( fDifX + fPosX, fDifY + fPosY, fDifZ + fPosZ, a_rCentre.w ),
		TVector4( fPosX - fSumX, fPosY - fSumY, fPosZ - fSumZ, a_rCentre.w ),
		TVector4( fNegX - fDifX, fNegY - fDifY, fNegZ - fDifZ, a_rCentre.w ),
		TVector4( fSumX + fNegX, fSumY + fNegY, fSumZ + fNegZ, a_rCentre.w ),
		TVector4( fSumX + fPosX, fSumY + fPosY, fSumZ + fPosZ, a_rCentre.w ),
		TVector4( fPosX - fDifX, fPosY - fDifY, fPosZ - fDifZ, a_rCentre.w ),
	};

	AddLine( aCorners[ 0 ], aCorners[ 1 ], a_uiColour, a_uiColour );
	AddLine( aCorners[ 1 ], aCorners[ 2 ], a_uiColour, a_uiColour );
	AddLine( aCorners[ 2 ], aCorners[ 3 ], a_uiColour, a_uiColour );
	AddLine( aCorners[ 3 ], aCorners[ 0 ], a_uiColour, a_uiColour );
	AddLine( aCorners[ 4 ], aCorners[ 5 ], a_uiColour, a_uiColour );
	AddLine( aCorners[ 5 ], aCorners[ 6 ], a_uiColour, a_uiColour );
	AddLine( aCorners[ 6 ], aCorners[ 7 ], a_uiColour, a_uiColour );
	AddLine( aCorners[ 7 ], aCorners[ 4 ], a_uiColour, a_uiColour );
	AddLine( aCorners[ 0 ], aCorners[ 4 ], a_uiColour, a_uiColour );
	AddLine( aCorners[ 1 ], aCorners[ 5 ], a_uiColour, a_uiColour );
	AddLine( aCorners[ 2 ], aCorners[ 6 ], a_uiColour, a_uiColour );
	AddLine( aCorners[ 3 ], aCorners[ 7 ], a_uiColour, a_uiColour );
}

// $deBlob: FUNCTION 005fe470
void ALineShader::AddSolidBox( const TVector4& a_rCentre, const TVector4& a_rExtents, TUINT32 a_uiColour, const TMatrix44* a_pRotation )
{
	TVector4 vAxisX( a_rExtents.x, 0.0f, 0.0f, 1.0f );
	TVector4 vAxisY( 0.0f, a_rExtents.y, 0.0f, 1.0f );
	TVector4 vAxisZ( 0.0f, 0.0f, a_rExtents.z, 1.0f );

	if ( a_pRotation != TNULL )
	{
		TMatrix44::RotateVector( vAxisX, *a_pRotation, vAxisX );
		TMatrix44::RotateVector( vAxisY, *a_pRotation, vAxisY );
		TMatrix44::RotateVector( vAxisZ, *a_pRotation, vAxisZ );
	}

	// The same corners as AddBox
	TFLOAT fPosX = a_rCentre.x + vAxisX.x, fPosY = a_rCentre.y + vAxisX.y, fPosZ = a_rCentre.z + vAxisX.z;
	TFLOAT fNegX = a_rCentre.x - vAxisX.x, fNegY = a_rCentre.y - vAxisX.y, fNegZ = a_rCentre.z - vAxisX.z;
	TFLOAT fSumX = vAxisZ.x + vAxisY.x, fSumY = vAxisZ.y + vAxisY.y, fSumZ = vAxisZ.z + vAxisY.z;
	TFLOAT fDifX = vAxisZ.x - vAxisY.x, fDifY = vAxisZ.y - vAxisY.y, fDifZ = vAxisZ.z - vAxisY.z;

	TVector4 aCorners[ 8 ] = {
		TVector4( fNegX - fSumX, fNegY - fSumY, fNegZ - fSumZ, a_rCentre.w ),
		TVector4( fDifX + fNegX, fDifY + fNegY, fDifZ + fNegZ, a_rCentre.w ),
		TVector4( fDifX + fPosX, fDifY + fPosY, fDifZ + fPosZ, a_rCentre.w ),
		TVector4( fPosX - fSumX, fPosY - fSumY, fPosZ - fSumZ, a_rCentre.w ),
		TVector4( fNegX - fDifX, fNegY - fDifY, fNegZ - fDifZ, a_rCentre.w ),
		TVector4( fSumX + fNegX, fSumY + fNegY, fSumZ + fNegZ, a_rCentre.w ),
		TVector4( fSumX + fPosX, fSumY + fPosY, fSumZ + fPosZ, a_rCentre.w ),
		TVector4( fPosX - fDifX, fPosY - fDifY, fPosZ - fDifZ, a_rCentre.w ),
	};

	AddTriangle( aCorners[ 4 ], aCorners[ 0 ], aCorners[ 3 ], a_uiColour );
	AddTriangle( aCorners[ 4 ], aCorners[ 3 ], aCorners[ 7 ], a_uiColour );
	AddTriangle( aCorners[ 6 ], aCorners[ 2 ], aCorners[ 1 ], a_uiColour );
	AddTriangle( aCorners[ 6 ], aCorners[ 1 ], aCorners[ 5 ], a_uiColour );
	AddTriangle( aCorners[ 0 ], aCorners[ 1 ], aCorners[ 2 ], a_uiColour );
	AddTriangle( aCorners[ 0 ], aCorners[ 2 ], aCorners[ 3 ], a_uiColour );
	AddTriangle( aCorners[ 7 ], aCorners[ 6 ], aCorners[ 5 ], a_uiColour );
	AddTriangle( aCorners[ 7 ], aCorners[ 5 ], aCorners[ 4 ], a_uiColour );
	AddTriangle( aCorners[ 5 ], aCorners[ 1 ], aCorners[ 0 ], a_uiColour );
	AddTriangle( aCorners[ 5 ], aCorners[ 0 ], aCorners[ 4 ], a_uiColour );
	AddTriangle( aCorners[ 7 ], aCorners[ 3 ], aCorners[ 2 ], a_uiColour );
	AddTriangle( aCorners[ 7 ], aCorners[ 2 ], aCorners[ 6 ], a_uiColour );
}

// $deBlob: FUNCTION 005fe940
ALineShader::ALineShader()
{
	m_bEnabled = TTRUE;
	m_fUnk10   = 0.0f;
}

// $deBlob: FUNCTION 005febd0
void ALineShader::AddLineDirect( const TVector4& a_rFrom, const TVector4& a_rTo, TUINT32 a_uiFromColour, TUINT32 a_uiToColour )
{
}

// $deBlob: FUNCTION 005febe0
void ALineShader::DrawTriangle( const TVector4& a_rVertex1, const TVector4& a_rVertex2, const TVector4& a_rVertex3, TUINT32 a_uiColour )
{
	AddTriangle( a_rVertex1, a_rVertex2, a_rVertex3, a_uiColour );
}

// $deBlob: FUNCTION 005febf0
void ALineShader::DrawLine( const TVector4& a_rFrom, const TVector4& a_rTo, TUINT32 a_uiFromColour, TUINT32 a_uiToColour )
{
	AddLine( a_rFrom, a_rTo, a_uiFromColour, a_uiToColour );
}

// $deBlob: FUNCTION 005fec00
void ALineShader::Reset()
{
}

//-----------------------------------------------------------------------------
// ALineMaterial
//-----------------------------------------------------------------------------

// NOTE: retail's CreateObject/CreateObjectInPlace return TNULL without an assert
// $deBlob: FUNCTION 0053cb90
// $deBlob: FUNCTION 005fe9a0
// $deBlob: FUNCTION 005fe9b0
// $deBlob: FUNCTION 005fe9c0
// $deBlob: FUNCTION 005fe9d0
// $deBlob: FUNCTION 005fe9e0
// $deBlob: FUNCTION 00858660
TDEFINE_CLASS_NORUNTIME( ALineMaterial );

// $deBlob: FUNCTION 005fea00
ALineMaterial::ALineMaterial()
{
}

// $deBlob: FUNCTION 005fe9f0
// $deBlob: FUNCTION 005feb40
ALineMaterial::~ALineMaterial()
{
}

//-----------------------------------------------------------------------------
// ALineMesh
//-----------------------------------------------------------------------------

// NOTE: retail's CreateObject/CreateObjectInPlace return TNULL without an assert
// $deBlob: FUNCTION 0053cbd0
// $deBlob: FUNCTION 005fea20
// $deBlob: FUNCTION 005fea30
// $deBlob: FUNCTION 005fea40
// $deBlob: FUNCTION 005fea50
// $deBlob: FUNCTION 005fea60
// $deBlob: FUNCTION 00858670
TDEFINE_CLASS_NORUNTIME( ALineMesh );

// $deBlob: FUNCTION 005fea80
ALineMesh::ALineMesh()
{
}

// $deBlob: FUNCTION 005fea70
// $deBlob: FUNCTION 005feb70
ALineMesh::~ALineMesh()
{
}
