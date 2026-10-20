#pragma once
#include "TShapeBaseClass.h"
#include "Math/TVector4.h"

TOSHI_NAMESPACE_START

// Shared box shape data
class TBoxData
    : public TShapeBaseClass
{
public:
	// $deBlob: FUNCTION 00733540
	TBoxData()
	    : m_vHalfExtents( 1.0f, 1.0f, 1.0f, 1.0f )
	{
		m_eType = Type_Box;
	}

	// $deBlob: FUNCTION 00733bb0
	// $deBlob: FUNCTION 005a9ba0
	virtual ~TBoxData() = default;

	// Returns the cached box data with these half extents (creating it if needed) with one more reference
	static TBoxData* Get( const TVector4& a_rcHalfExtents );

	const TVector4& GetHalfExtents() const { return m_vHalfExtents; }
	void            SetHalfExtents( const TVector4& a_rcHalfExtents ) { m_vHalfExtents = a_rcHalfExtents; }

public:
	static TShapeDataCache ms_oCache;

private:
	TVector4 m_vHalfExtents; // 0x10
};

TOSHI_NAMESPACE_END
