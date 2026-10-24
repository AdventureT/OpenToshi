#pragma once
#include "TShapeBaseClass.h"
#include "Math/TVector4.h"

TOSHI_NAMESPACE_START

// Shared plane shape data
class TPlaneData
    : public TShapeBaseClass
{
public:
	TPlaneData()
	    : m_vNormal( TVector4::VEC_NEGY ), m_vPoint( 0.0f, 0.0f, 0.0f, 1.0f )
	{
		m_eType = Type_Plane;
	}

	// $deBlob: FUNCTION 00733c70
	virtual ~TPlaneData() = default;

	// Returns the cached plane data with these values (creating it if needed) with one more reference
	static TPlaneData* Get( const TVector4& a_rcNormal, const TVector4& a_rcPoint );

	const TVector4& GetNormal() const { return m_vNormal; }
	const TVector4& GetPoint() const { return m_vPoint; }

public:
	static TShapeDataCache ms_oCache;

private:
	TVector4 m_vNormal; // 0x10
	TVector4 m_vPoint;  // 0x20
};

TOSHI_NAMESPACE_END
