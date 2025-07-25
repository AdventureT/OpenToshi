#pragma once
#include "TVector4.h"
#include "TPlane.h"

TOSHI_NAMESPACE_START

class TSphere
{
public:
	TSphere() = default;

	TSphere(const TVector4& a_rSphereData)
	{
		m_Origin  = a_rSphereData.AsVector3();
		m_fRadius = a_rSphereData.w;
	}

	TSphere(const TVector3& a_rOrigin, TFLOAT a_fRadius)
	{
		m_Origin  = a_rOrigin;
		m_fRadius = a_fRadius;
	}

	TSphere(TFLOAT a_fOriginX, TFLOAT a_fOriginY, TFLOAT a_fOriginZ, TFLOAT a_fRadius)
	{
		m_Origin.Set(a_fOriginX, a_fOriginY, a_fOriginZ);
		m_fRadius = a_fRadius;
	}

	TSphere(const TSphere& a_rSphere)
	{
		m_Origin  = a_rSphere.m_Origin;
		m_fRadius = a_rSphere.m_fRadius;
	}

	void Set(const TVector4& a_rSphereData)
	{
		m_Origin  = a_rSphereData.AsVector3();
		m_fRadius = a_rSphereData.w;
	}

	void Set(const TVector3& a_rOrigin, TFLOAT a_fRadius)
	{
		m_Origin  = a_rOrigin;
		m_fRadius = a_fRadius;
	}

	void Set(TFLOAT a_fOriginX, TFLOAT a_fOriginY, TFLOAT a_fOriginZ, TFLOAT a_fRadius)
	{
		m_Origin.Set(a_fOriginX, a_fOriginY, a_fOriginZ);
		m_fRadius = a_fRadius;
	}

	void Set(const TSphere& a_rSphere)
	{
		m_Origin  = a_rSphere.m_Origin;
		m_fRadius = a_rSphere.m_fRadius;
	}

	void Union(const TSphere& a_rSphere1, const TSphere& a_rSphere2)
	{
		TVector3 diff = a_rSphere2.m_Origin - a_rSphere1.m_Origin;

		TFLOAT fMag     = diff.MagnitudeSq();
		TFLOAT fRadDiff = a_rSphere2.m_fRadius - a_rSphere1.m_fRadius;

		if (fRadDiff <= 0.0f)
		{
			if (fMag < fRadDiff * fRadDiff) Set(a_rSphere1);
		}
		else if (fMag < fRadDiff * fRadDiff)
		{
			Set(a_rSphere2);
		}
		else
		{
			m_fRadius = (a_rSphere1.m_fRadius + a_rSphere2.m_fRadius + TMath::Sqrt(fMag)) * 0.5f;
			diff.Multiply((fRadDiff - a_rSphere1.m_fRadius) / TMath::Sqrt(fMag));
			m_Origin.Add(a_rSphere1.m_Origin, diff);
		}
	}

	TPlane::PlaneComparison ComparePlane(const TPlane& a_rPlane)
	{
		TFLOAT fDist = TVector3::DotProduct(a_rPlane.GetNormal(), m_Origin) - a_rPlane.GetD();

		if (m_fRadius < fDist) return TPlane::PlaneComparison_InFront;

		if (fDist < -m_fRadius) return TPlane::PlaneComparison_InBack;

		return TPlane::PlaneComparison_Intersects;
	}

	TSphere& operator=(const TSphere& a_rSphere)
	{
		m_Origin  = a_rSphere.m_Origin;
		m_fRadius = a_rSphere.m_fRadius;
		return *this;
	}

	__forceinline const TVector3& GetOrigin() const { return m_Origin; }

	__forceinline TFLOAT GetRadius() const { return m_fRadius; }

	__forceinline TVector4&       AsVector4() { return *TREINTERPRETCAST(TVector4*, this); }
	__forceinline const TVector4& AsVector4() const { return *TREINTERPRETCAST(const TVector4*, this); }

private:
	TVector3 m_Origin;
	TFLOAT   m_fRadius;
};

TOSHI_NAMESPACE_END
