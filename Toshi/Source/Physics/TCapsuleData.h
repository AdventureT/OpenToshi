#pragma once
#include "TShapeBaseClass.h"

TOSHI_NAMESPACE_START

// Shared capsule (along z) shape data
class TCapsuleData
    : public TShapeBaseClass
{
public:
	// $deBlob: FUNCTION 00733740
	TCapsuleData()
	    : m_fHeight( 1.0f ), m_fRadius( 0.5f )
	{
		m_eType = Type_Capsule;
	}

	// $deBlob: FUNCTION 00733c10
	virtual ~TCapsuleData() = default;

	// Returns the cached capsule (along z) data with these values (creating it if needed) with one more reference
	static TCapsuleData* Get( TFLOAT a_fHeight, TFLOAT a_fRadius );

	TFLOAT GetHeight() const { return m_fHeight; }
	TFLOAT GetRadius() const { return m_fRadius; }

	// Inlined stores of the callers (ALeechBot2::Create)
	void SetHeight( TFLOAT a_fHeight ) { m_fHeight = a_fHeight; }
	void SetRadius( TFLOAT a_fRadius ) { m_fRadius = a_fRadius; }

public:
	static TShapeDataCache ms_oCache;

private:
	TFLOAT m_fHeight; // 0x10 (distance between the two segment ends)
	TFLOAT m_fRadius; // 0x14
};

TOSHI_NAMESPACE_END
