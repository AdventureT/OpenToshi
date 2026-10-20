#pragma once
#include "TVector4.h"

TOSHI_NAMESPACE_START

class TPlane
{
public:
	enum PlaneComparison
	{
		PlaneComparison_InFront = -1,
		PlaneComparison_Intersects,
		PlaneComparison_InBack,
	};

public:
	TCONSTEXPR TPlane() = default;

	TCONSTEXPR TPlane( const TVector4& a_PlaneData )
	    : m_Normal( a_PlaneData.AsVector3() ), m_fDistance( a_PlaneData.w )
	{}

	TCONSTEXPR TPlane( const TVector3& a_Normal, TFLOAT a_fDistance )
	    : m_Normal( a_Normal ), m_fDistance( a_fDistance )
	{}

	// $deBlob: FUNCTION 0058c6d0
	// $deBlob: FUNCTION 007a52a0
	TCONSTEXPR TPlane( const TVector3& a_Normal, const TVector3& a_Point )
	    : m_Normal( a_Normal ), m_fDistance( TVector3::DotProduct( a_Normal, a_Point ) )
	{}

	TCONSTEXPR TPlane( TFLOAT a_fNormalX, TFLOAT a_fNormalY, TFLOAT a_fNormalZ, TFLOAT a_fDistance )
	    : m_Normal( a_fNormalX, a_fNormalY, a_fNormalZ ), m_fDistance( a_fDistance )
	{}

	TCONSTEXPR TPlane( const TPlane& a_Other )
	    : m_Normal( a_Other.m_Normal ), m_fDistance( a_Other.m_fDistance )
	{}

	// $deBlob: FUNCTION 00672110
	// $deBlob: FUNCTION 00672130
	TPlane( const TVector3& a_rcPoint1, const TVector3& a_rcPoint2, const TVector3& a_rcPoint3 ) { Set( a_rcPoint1, a_rcPoint2, a_rcPoint3 ); }

	// Plane through three points (counter clockwise)
	void Set( const TVector3& a_rcPoint1, const TVector3& a_rcPoint2, const TVector3& a_rcPoint3 );

	// Signed distance of a_rcPoint from the plane
	// $deBlob: FUNCTION 00672220
	TCONSTEXPR TFLOAT GetDistance( const TVector3& a_rcPoint ) const { return ( a_rcPoint.y * m_Normal.y + a_rcPoint.x * m_Normal.x + a_rcPoint.z * m_Normal.z ) - m_fDistance; }

	// $deBlob: FUNCTION 00672260
	TCONSTEXPR TFLOAT GetDistance( const TVector4& a_rcPoint ) const { return ( a_rcPoint.y * m_Normal.y + a_rcPoint.x * m_Normal.x + a_rcPoint.z * m_Normal.z ) - m_fDistance; }

	// Projects a_rcPoint onto the plane
	// $deBlob: FUNCTION 006723a0
	void ProjectPoint( TVector3& a_rOut, const TVector3& a_rcPoint ) const
	{
		TFLOAT fDistance = ( m_Normal.x * a_rcPoint.x + m_Normal.y * a_rcPoint.y + m_Normal.z * a_rcPoint.z ) - m_fDistance;
		a_rOut.x         = a_rcPoint.x - m_Normal.x * fDistance;
		a_rOut.y         = a_rcPoint.y - m_Normal.y * fDistance;
		a_rOut.z         = a_rcPoint.z - m_Normal.z * fDistance;
	}

	TCONSTEXPR void Set( const TVector4& a_PlaneData )
	{
		m_Normal    = a_PlaneData.AsVector3();
		m_fDistance = a_PlaneData.w;
	}

	TCONSTEXPR void Set( const TVector3& a_Normal, TFLOAT a_fDistance )
	{
		m_Normal    = a_Normal;
		m_fDistance = a_fDistance;
	}

	// $deBlob: FUNCTION 0058cef0
	// $deBlob: FUNCTION 005f74b0
	TCONSTEXPR void Set( const TVector3& a_Normal, const TVector3& a_Point )
	{
		m_Normal    = a_Normal;
		m_fDistance = TVector3::DotProduct( a_Normal, a_Point );
	}

	TCONSTEXPR void Set( TFLOAT a_fNormalX, TFLOAT a_fNormalY, TFLOAT a_fNormalZ, TFLOAT a_fDistance )
	{
		m_Normal.Set( a_fNormalX, a_fNormalY, a_fNormalZ );
		m_fDistance = a_fDistance;
	}

	TCONSTEXPR void Set( const TPlane& a_Other )
	{
		m_Normal    = a_Other.m_Normal;
		m_fDistance = a_Other.m_fDistance;
	}

	TFORCEINLINE TCONSTEXPR TFLOAT GetD() const
	{
		return m_fDistance;
	}

	TFORCEINLINE TCONSTEXPR void SetD( TFLOAT a_fDistance )
	{
		m_fDistance = a_fDistance;
	}

	TFORCEINLINE TCONSTEXPR const TVector3& GetNormal() const
	{
		return m_Normal;
	}

	TFORCEINLINE TCONSTEXPR void SetNormal( const TVector3& a_rNormal )
	{
		m_Normal = a_rNormal;
	}

	TFORCEINLINE TCONSTEXPR void SetNormal( TFLOAT a_fNormalX, TFLOAT a_fNormalY, TFLOAT a_fNormalZ )
	{
		m_Normal.Set( a_fNormalX, a_fNormalY, a_fNormalZ );
	}

	TFORCEINLINE void GetPoint( TVector3& a_rPoint )
	{
		a_rPoint.Multiply( m_Normal, m_fDistance );
	}

	TFORCEINLINE void SetPoint( const TVector3& a_rPoint )
	{
		m_fDistance = TVector3::DotProduct( m_Normal, a_rPoint );
	}

	TFORCEINLINE void Negate( const TPlane& a_rPlane )
	{
		AsVector4().Negate4( a_rPlane.AsVector4() );
	}

	TFORCEINLINE void Negate()
	{
		AsVector4().Negate4();
	}

	// NOTE: retail copies the whole plane at once (movdqu)
	TPlane& operator=( const TPlane& a_rPlane )
	{
		memcpy( this, &a_rPlane, sizeof( TPlane ) );
		return *this;
	}

	TFORCEINLINE TCONSTEXPR TVector3&       AsNormal() { return m_Normal; }
	TFORCEINLINE TCONSTEXPR const TVector3& AsNormal() const { return m_Normal; }

	// Projects a_rcPoint onto the plane (w of the result is 1.0f)
	// $deBlob: FUNCTION 00672320
	// $Barnyard: FUNCTION 004a6b30
	TVector4 ProjectPoint( const TVector4& a_rcPoint ) const
	{
		// NOTE: de Blob sums x, y, z (Barnyard x, z, y)
		TFLOAT fDistance = ( m_Normal.x * a_rcPoint.x + a_rcPoint.y * m_Normal.y + a_rcPoint.z * m_Normal.z ) - m_fDistance;
		return TVector4( a_rcPoint.x - fDistance * m_Normal.x, a_rcPoint.y - fDistance * m_Normal.y, a_rcPoint.z - fDistance * m_Normal.z, 1.0f );
	}

	TFORCEINLINE TVector4&       AsVector4() { return *TREINTERPRETCAST( TVector4*, this ); }
	TFORCEINLINE const TVector4& AsVector4() const { return *TREINTERPRETCAST( const TVector4*, this ); }

private:
	TVector3 m_Normal;
	TFLOAT   m_fDistance;
};

TOSHI_NAMESPACE_END
