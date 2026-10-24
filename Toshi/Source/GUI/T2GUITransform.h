#pragma once
#include "Math/TVector2.h"
#include "Math/TMatrix44.h"

TOSHI_NAMESPACE_START

// NOTE: de Blob's T2GUITransform (0x10 bytes) stores the 2x2 rotation/scale part as 8.8 fixed point shorts
// (Barnyard's is 0x18 bytes of floats); the translation stays a TVector2.
class T2GUITransform
{
public:
	static const TFLOAT FLOAT_QUALITY;
	static const TFLOAT FLOAT_SCALE;

	using Float = TINT16;

	class Rotation
	{
	public:
		// $deBlob: FUNCTION 00550b30
		TCONSTEXPR Rotation()
		    : m_X( 0 ), m_Y( 0 ) {}

		TCONSTEXPR Rotation( Float a_iX, Float a_iY )
		    : m_X( a_iX ), m_Y( a_iY ) {}

		// NOTE: retail copies the two halves separately (movzx/mov word)
		TCONSTEXPR Rotation( const Rotation& a_rcOther )
		    : m_X( a_rcOther.m_X ), m_Y( a_rcOther.m_Y ) {}

		TCONSTEXPR Rotation& operator=( const Rotation& a_rcOther )
		{
			m_X = a_rcOther.m_X;
			m_Y = a_rcOther.m_Y;
			return *this;
		}

		void SetX( TFLOAT a_fX ) { m_X = PackFloat( a_fX ); }
		void SetY( TFLOAT a_fY ) { m_Y = PackFloat( a_fY ); }

		TFLOAT GetX() const { return UnpackFloat( m_X ); }
		TFLOAT GetY() const { return UnpackFloat( m_Y ); }

	public:
		Float m_X; // 0x0
		Float m_Y; // 0x2
	};

public:
	T2GUITransform()
	{
		m_Rot[ 0 ].m_X = 256;
		m_Rot[ 0 ].m_Y = 0;
		m_Rot[ 1 ].m_X = 0;
		m_Rot[ 1 ].m_Y = 256;
		m_Pos.x        = 0.0f;
		m_Pos.y        = 0.0f;
	}

	TCONSTEXPR T2GUITransform( TFLOAT a_fScaleX, TFLOAT a_fScaleY )
	    : m_Pos( 0.0f, 0.0f )
	{
		m_Rot[ 0 ] = Rotation( Float( a_fScaleX * FLOAT_QUALITY ), 0 );
		m_Rot[ 1 ] = Rotation( 0, Float( a_fScaleY * FLOAT_QUALITY ) );
	}

	void SetIdentity()
	{
		m_Rot[ 0 ] = { 256, 0 };
		m_Rot[ 1 ] = { 0, 256 };
		m_Pos.x    = 0.0f;
		m_Pos.y    = 0.0f;
	}

	void Rotate( TFLOAT a_fAngle );
	void GetInverse( T2GUITransform& a_rOutTransform ) const;
	void PreMultiply( const T2GUITransform& a_rTransform );
	void PreMultiply( TFLOAT a_fScaleX, TFLOAT a_fScaleY );
	void PostMultiply( const T2GUITransform& a_rTransform );
	void Matrix44( TMatrix44& a_rOutMatrix ) const;

	// $deBlob: FUNCTION 005510c0
	void Transform( TVector2& a_rOutVec, const TVector2& a_rTransformVec ) const
	{
		a_rOutVec.x = m_Rot[ 1 ].GetX() * a_rTransformVec.y + m_Rot[ 0 ].GetX() * a_rTransformVec.x + m_Pos.x;
		a_rOutVec.y = m_Rot[ 0 ].GetY() * a_rTransformVec.x + m_Rot[ 1 ].GetY() * a_rTransformVec.y + m_Pos.y;
	}

	void SetTranslation( TFLOAT a_fX, TFLOAT a_fY )
	{
		m_Pos.x = a_fX;
		m_Pos.y = a_fY;
	}

	void SetTranslation( const TVector2& a_rcTranslation ) { m_Pos = a_rcTranslation; }

	TVector2&       GetPos() { return m_Pos; }
	const TVector2& GetPos() const { return m_Pos; }

	static TCONSTEXPR TFLOAT UnpackFloat( Float a_iValue ) { return a_iValue * FLOAT_SCALE; }
	static TCONSTEXPR Float  PackFloat( TFLOAT a_fValue ) { return Float( TINT( a_fValue * FLOAT_QUALITY ) ); }

	static void Multiply( T2GUITransform& a_rOutTransform, const T2GUITransform& a_rA, const T2GUITransform& a_rB );

public:
	Rotation m_Rot[ 2 ]; // 0x00
	TVector2 m_Pos;      // 0x08
};

TOSHI_NAMESPACE_END
