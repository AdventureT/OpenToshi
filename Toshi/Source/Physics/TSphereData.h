#pragma once
#include "TShapeBaseClass.h"

TOSHI_NAMESPACE_START

// Shared sphere shape data
class TSphereData
    : public TShapeBaseClass
{
public:
	// $deBlob: FUNCTION 00733460
	TSphereData()
	    : m_fRadius( 1.0f )
	{
		m_eType = Type_Sphere;
	}

	// $deBlob: FUNCTION 00733cd0
	// $deBlob: FUNCTION 0058c770
	virtual ~TSphereData() = default;

	// Returns the cached sphere data with these values (creating it if needed) with one more reference
	static TSphereData* Get( TFLOAT a_fRadius );

	TFLOAT GetRadius() const { return m_fRadius; }
	void   SetRadius( TFLOAT a_fRadius ) { m_fRadius = a_fRadius; }

public:
	static TShapeDataCache ms_oCache;

private:
	TFLOAT m_fRadius; // 0x10
};

TOSHI_NAMESPACE_END
