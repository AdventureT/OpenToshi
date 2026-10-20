#pragma once
#include "TShapeBaseClass.h"

TOSHI_NAMESPACE_START

// Shared cylinder (along z) shape data
class TCylinderData
    : public TShapeBaseClass
{
public:
	// $deBlob: FUNCTION 00733640
	TCylinderData()
	    : m_fHeight( 1.0f ), m_fRadius( 0.5f )
	{
		m_eType = Type_Cylinder;
	}

	// $deBlob: FUNCTION 00733c40
	virtual ~TCylinderData() = default;

	// Returns the cached cylinder (along z) data with these values (creating it if needed) with one more reference
	static TCylinderData* Get( TFLOAT a_fHeight, TFLOAT a_fRadius );

	TFLOAT GetHeight() const { return m_fHeight; }
	TFLOAT GetRadius() const { return m_fRadius; }

	// Inlined stores of the callers
	void SetHeight( TFLOAT a_fHeight ) { m_fHeight = a_fHeight; }
	void SetRadius( TFLOAT a_fRadius ) { m_fRadius = a_fRadius; }

public:
	static TShapeDataCache ms_oCache;

private:
	TFLOAT m_fHeight; // 0x10
	TFLOAT m_fRadius; // 0x14
};

TOSHI_NAMESPACE_END
