#include "ToshiPCH.h"
#include "T2GUITransform.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

const TFLOAT T2GUITransform::FLOAT_QUALITY = 256.0f;
const TFLOAT T2GUITransform::FLOAT_SCALE   = 1.0f / FLOAT_QUALITY;

// $deBlob: FUNCTION 006f45a0
// $Barnyard: FUNCTION 006c2000
void T2GUITransform::PreMultiply( const T2GUITransform& a_rTransform )
{
	TFLOAT fRot0X  = m_Rot[ 0 ].GetX();
	TFLOAT fRot0Y  = m_Rot[ 0 ].GetY();
	TFLOAT fTrans0X = a_rTransform.m_Rot[ 0 ].GetX();
	TFLOAT fTrans0Y = a_rTransform.m_Rot[ 0 ].GetY();
	TFLOAT fRot1X  = m_Rot[ 1 ].GetX();
	TFLOAT fRot1Y  = m_Rot[ 1 ].GetY();
	TFLOAT fTrans1Y = a_rTransform.m_Rot[ 1 ].GetY();
	TFLOAT fTrans1X = a_rTransform.m_Rot[ 1 ].GetX();

	m_Rot[ 0 ].SetX( fTrans0Y * fRot1X + fTrans0X * fRot0X );
	m_Rot[ 0 ].SetY( fTrans0Y * fRot1Y + fTrans0X * fRot0Y );
	m_Rot[ 1 ].SetX( fTrans1Y * fRot1X + fTrans1X * fRot0X );
	m_Rot[ 1 ].SetY( fTrans1Y * fRot1Y + fTrans1X * fRot0Y );
}

// $deBlob: FUNCTION 006f46b0
void T2GUITransform::PostMultiply( const T2GUITransform& a_rTransform )
{
	TFLOAT fRot0X   = m_Rot[ 0 ].GetX();
	TFLOAT fRot1X   = m_Rot[ 1 ].GetX();
	TFLOAT fRot0Y   = m_Rot[ 0 ].GetY();
	TFLOAT fTrans0X = a_rTransform.m_Rot[ 0 ].GetX();
	TFLOAT fTrans1X = a_rTransform.m_Rot[ 1 ].GetX();
	TFLOAT fTrans1Y = a_rTransform.m_Rot[ 1 ].GetY();
	TFLOAT fRot1Y   = m_Rot[ 1 ].GetY();
	TFLOAT fTrans0Y = a_rTransform.m_Rot[ 0 ].GetY();

	m_Rot[ 0 ].SetX( fTrans1X * fRot0Y + fTrans0X * fRot0X );
	m_Rot[ 0 ].SetY( fTrans1Y * fRot0Y + fTrans0Y * fRot0X );
	m_Rot[ 1 ].SetX( fTrans1X * fRot1Y + fTrans0X * fRot1X );
	m_Rot[ 1 ].SetY( fTrans1Y * fRot1Y + fTrans0Y * fRot1X );
}

// $deBlob: FUNCTION 006f47d0
// $Barnyard: FUNCTION 006c2060
void T2GUITransform::Rotate( TFLOAT a_fAngle )
{
	TFLOAT fSin, fCos;
	TMath::SinCos( a_fAngle, fSin, fCos );

	TFLOAT fRot0X = m_Rot[ 0 ].GetX();
	TFLOAT fRot1X = m_Rot[ 1 ].GetX();
	TFLOAT fRot0Y = m_Rot[ 0 ].GetY();
	TFLOAT fRot1Y = m_Rot[ 1 ].GetY();

	m_Rot[ 0 ].SetX( fRot1X * fSin + fRot0X * fCos );
	m_Rot[ 0 ].SetY( fRot1Y * fSin + fRot0Y * fCos );
	m_Rot[ 1 ].SetX( -( fRot0X * fSin ) + fRot1X * fCos );
	m_Rot[ 1 ].SetY( -( fRot0Y * fSin ) + fRot1Y * fCos );
}

// $deBlob: FUNCTION 006f48d0
// $Barnyard: FUNCTION 006c21a0
void T2GUITransform::GetInverse( T2GUITransform& a_rOutTransform ) const
{
	TASSERT( m_Rot[ 0 ].GetX() * m_Rot[ 1 ].GetY() - m_Rot[ 1 ].GetX() * m_Rot[ 0 ].GetY() != 0.0f );

	TFLOAT fRot1Y = m_Rot[ 1 ].GetY();
	TFLOAT fInvDet = 1.0f / ( m_Rot[ 0 ].GetX() * fRot1Y - m_Rot[ 1 ].GetX() * m_Rot[ 0 ].GetY() );

	a_rOutTransform.m_Rot[ 0 ].SetX( fRot1Y * fInvDet );
	a_rOutTransform.m_Rot[ 0 ].SetY( -m_Rot[ 0 ].GetY() * fInvDet );
	a_rOutTransform.m_Rot[ 1 ].SetX( -m_Rot[ 1 ].GetX() * fInvDet );
	a_rOutTransform.m_Rot[ 1 ].SetY( m_Rot[ 0 ].GetX() * fInvDet );

	a_rOutTransform.m_Pos.x = 0.0f;
	a_rOutTransform.m_Pos.y = 0.0f;
	a_rOutTransform.m_Pos.x = a_rOutTransform.m_Rot[ 1 ].GetX() * m_Pos.y + a_rOutTransform.m_Rot[ 0 ].GetX() * m_Pos.x + a_rOutTransform.m_Pos.x;
	a_rOutTransform.m_Pos.y = a_rOutTransform.m_Rot[ 1 ].GetY() * m_Pos.y + a_rOutTransform.m_Rot[ 0 ].GetY() * m_Pos.x + a_rOutTransform.m_Pos.y;
	a_rOutTransform.m_Pos.x = -a_rOutTransform.m_Pos.x;
	a_rOutTransform.m_Pos.y = -a_rOutTransform.m_Pos.y;
}

// $deBlob: FUNCTION 006f4ad0
// $Barnyard: FUNCTION 006c2120
void T2GUITransform::Multiply( T2GUITransform& a_rOutTransform, const T2GUITransform& a_rA, const T2GUITransform& a_rB )
{
	TFLOAT fA0Y = a_rA.m_Rot[ 0 ].GetY();
	TFLOAT fA0X = a_rA.m_Rot[ 0 ].GetX();
	TFLOAT fA1Y = a_rA.m_Rot[ 1 ].GetY();
	TFLOAT fB0X = a_rB.m_Rot[ 0 ].GetX();
	TFLOAT fB0Y = a_rB.m_Rot[ 0 ].GetY();
	TFLOAT fA1X = a_rA.m_Rot[ 1 ].GetX();
	TFLOAT fB1Y = a_rB.m_Rot[ 1 ].GetY();
	TFLOAT fB1X = a_rB.m_Rot[ 1 ].GetX();

	a_rOutTransform.m_Rot[ 0 ].SetX( fB0Y * fA1X + fB0X * fA0X );
	a_rOutTransform.m_Rot[ 0 ].SetY( fB0Y * fA1Y + fB0X * fA0Y );
	a_rOutTransform.m_Rot[ 1 ].SetX( fB1Y * fA1X + fB1X * fA0X );
	a_rOutTransform.m_Rot[ 1 ].SetY( fB1Y * fA1Y + fB1X * fA0Y );
	a_rOutTransform.m_Pos.x = a_rA.m_Rot[ 1 ].GetX() * a_rB.m_Pos.y + a_rA.m_Rot[ 0 ].GetX() * a_rB.m_Pos.x + a_rA.m_Pos.x;
	a_rOutTransform.m_Pos.y = a_rA.m_Rot[ 1 ].GetY() * a_rB.m_Pos.y + a_rA.m_Rot[ 0 ].GetY() * a_rB.m_Pos.x + a_rA.m_Pos.y;
}

// $deBlob: FUNCTION 005e5670
void T2GUITransform::PreMultiply( TFLOAT a_fScaleX, TFLOAT a_fScaleY )
{
	T2GUITransform transform( a_fScaleX, a_fScaleY );
	PreMultiply( transform );
}

// NOTE: an instance of it lives in the game's A2GUIRenderer_DX11 code
// $deBlob: FUNCTION 00550cc0
void T2GUITransform::Matrix44( TMatrix44& a_rOutMatrix ) const
{
	a_rOutMatrix.m_f13 = 0.0f;
	a_rOutMatrix.m_f14 = 0.0f;
	a_rOutMatrix.m_f23 = 0.0f;
	a_rOutMatrix.m_f24 = 0.0f;
	a_rOutMatrix.m_f31 = 0.0f;
	a_rOutMatrix.m_f32 = 0.0f;
	a_rOutMatrix.m_f12 = m_Rot[ 0 ].GetY();
	a_rOutMatrix.m_f21 = m_Rot[ 1 ].GetX();
	a_rOutMatrix.m_f11 = m_Rot[ 0 ].GetX();
	a_rOutMatrix.m_f22 = m_Rot[ 1 ].GetY();
	a_rOutMatrix.m_f33 = 1.0f;
	a_rOutMatrix.m_f34 = 0.0f;
	a_rOutMatrix.m_f41 = m_Pos.x;
	a_rOutMatrix.m_f42 = m_Pos.y;
	a_rOutMatrix.m_f43 = 0.0f;
	a_rOutMatrix.m_f44 = 1.0f;
}

TOSHI_NAMESPACE_END
