#pragma once
#include "TVector4.h"
#include "TPlane.h"

TOSHI_NAMESPACE_START

class TSphere
{
public:
	TCONSTEXPR TSphere() = default;

	TCONSTEXPR TSphere( const TVector4& a_rSphereData )
	    : m_Origin( a_rSphereData.AsVector3() ), m_fRadius( a_rSphereData.w )
	{}

	TCONSTEXPR TSphere( const TVector3& a_rOrigin, TFLOAT a_fRadius )
	    : m_Origin( a_rOrigin ), m_fRadius( a_fRadius )
	{}

	TCONSTEXPR TSphere( TFLOAT a_fOriginX, TFLOAT a_fOriginY, TFLOAT a_fOriginZ, TFLOAT a_fRadius )
	    : m_Origin( a_fOriginX, a_fOriginY, a_fOriginZ ), m_fRadius( a_fRadius )
	{}

	TCONSTEXPR TSphere( const TSphere& a_rSphere )
	    : m_Origin( a_rSphere.m_Origin ), m_fRadius( a_rSphere.m_fRadius )
	{}

	TCONSTEXPR void Set( const TVector4& a_rSphereData )
	{
		m_Origin  = a_rSphereData.AsVector3();
		m_fRadius = a_rSphereData.w;
	}

	TCONSTEXPR void Set( const TVector3& a_rOrigin, TFLOAT a_fRadius )
	{
		m_Origin  = a_rOrigin;
		m_fRadius = a_fRadius;
	}

	TCONSTEXPR void Set( TFLOAT a_fOriginX, TFLOAT a_fOriginY, TFLOAT a_fOriginZ, TFLOAT a_fRadius )
	{
		m_Origin.Set( a_fOriginX, a_fOriginY, a_fOriginZ );
		m_fRadius = a_fRadius;
	}

	TCONSTEXPR void Set( const TSphere& a_rSphere )
	{
		m_Origin  = a_rSphere.m_Origin;
		m_fRadius = a_rSphere.m_fRadius;
	}

	// The smallest sphere around both (one of them if it holds the other); NOTE: de Blob's (Barnyard's compares the
	// other way and places the centre differently)
	// $deBlob: FUNCTION 00831020
	void Union( const TSphere& a_rSphere1, const TSphere& a_rSphere2 )
	{
		TFLOAT fDiffY = a_rSphere2.m_Origin.y - a_rSphere1.m_Origin.y;
		TFLOAT fDiffX = a_rSphere2.m_Origin.x - a_rSphere1.m_Origin.x;
		TFLOAT fDiffZ = a_rSphere2.m_Origin.z - a_rSphere1.m_Origin.z;
		TFLOAT fMag   = fDiffY * fDiffY + fDiffX * fDiffX + fDiffZ * fDiffZ;

		TFLOAT fRadDiff   = a_rSphere2.m_fRadius - a_rSphere1.m_fRadius;
		TFLOAT fRadDiffSq = fRadDiff * fRadDiff;

		if ( fRadDiff <= 0.0f )
		{
			if ( fRadDiffSq >= fMag )
			{
				Set( a_rSphere1 );
				return;
			}
		}
		else if ( fRadDiffSq >= fMag )
		{
			Set( a_rSphere2 );
			return;
		}

		TFLOAT fLength = TFLOAT( sqrt( fMag ) );
		m_fRadius      = ( a_rSphere1.m_fRadius + fLength + a_rSphere2.m_fRadius ) * 0.5f;

		TFLOAT fT  = ( m_fRadius - a_rSphere1.m_fRadius ) / fLength;
		m_Origin.x = a_rSphere1.m_Origin.x + fDiffX * fT;
		m_Origin.y = a_rSphere1.m_Origin.y + fDiffY * fT;
		m_Origin.z = a_rSphere1.m_Origin.z + fDiffZ * fT;
	}

	// The radius of the union with a_rSphere (this one's when it holds the other strictly, a_rSphere's when it holds
	// this one)
	// $deBlob: FUNCTION 00831160
	TFLOAT GetUnionRadius( const TSphere& a_rSphere ) const { return GetUnionRadius( *this, a_rSphere ); }

	// The same for two spheres
	// $deBlob: FUNCTION 00831220
	static TFLOAT GetUnionRadius( const TSphere& a_rSphere1, const TSphere& a_rSphere2 )
	{
		TFLOAT fDiffZ = a_rSphere2.m_Origin.z - a_rSphere1.m_Origin.z;
		TFLOAT fMag   = ( a_rSphere2.m_Origin.y - a_rSphere1.m_Origin.y ) * ( a_rSphere2.m_Origin.y - a_rSphere1.m_Origin.y ) +
		    ( a_rSphere2.m_Origin.x - a_rSphere1.m_Origin.x ) * ( a_rSphere2.m_Origin.x - a_rSphere1.m_Origin.x ) + fDiffZ * fDiffZ;

		TFLOAT fRadDiff   = a_rSphere2.m_fRadius - a_rSphere1.m_fRadius;
		TFLOAT fRadDiffSq = fRadDiff * fRadDiff;

		if ( fRadDiff <= 0.0f )
		{
			if ( fRadDiffSq > fMag )
				return a_rSphere1.m_fRadius;
		}
		else if ( fRadDiffSq >= fMag )
		{
			return a_rSphere2.m_fRadius;
		}

		return ( TFLOAT( sqrt( fMag ) ) + a_rSphere1.m_fRadius + a_rSphere2.m_fRadius ) * 0.5f;
	}

	// $deBlob: FUNCTION 008312e0
	TCONSTEXPR TPlane::PlaneComparison ComparePlane( const TPlane& a_rPlane )
	{
		float fDist = TVector3::DotProduct( a_rPlane.GetNormal(), m_Origin ) - a_rPlane.GetD();

		if ( !( fDist <= m_fRadius ) )
			return TPlane::PlaneComparison_InFront;

		if ( -m_fRadius > fDist )
			return TPlane::PlaneComparison_InBack;

		return TPlane::PlaneComparison_Intersects;
	}

	TCONSTEXPR TSphere& operator=( const TSphere& a_rSphere )
	{
		m_Origin  = a_rSphere.m_Origin;
		m_fRadius = a_rSphere.m_fRadius;
		return *this;
	}

	TCONSTEXPR TFORCEINLINE TVector3& GetOrigin()
	{
		return m_Origin;
	}

	TCONSTEXPR TFORCEINLINE const TVector3& GetOrigin() const
	{
		return m_Origin;
	}

	TCONSTEXPR TFORCEINLINE TFLOAT GetRadius() const
	{
		return m_fRadius;
	}

	TCONSTEXPR TFORCEINLINE TFLOAT& GetRadius()
	{
		return m_fRadius;
	}

	TFORCEINLINE TVector4&       AsVector4() { return *TREINTERPRETCAST( TVector4*, this ); }
	TFORCEINLINE const TVector4& AsVector4() const { return *TREINTERPRETCAST( const TVector4*, this ); }

private:
	TVector3 m_Origin;
	TFLOAT   m_fRadius;
};

TOSHI_NAMESPACE_END
