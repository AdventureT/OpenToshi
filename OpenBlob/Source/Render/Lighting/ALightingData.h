#pragma once
#include <Math/TVector4.h>
#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Purpose: the current lighting of the level (4 bytes): points to the light settings in use (blended between the
// level's lighting sets)
// NOTE: only the parts the shaders read are ported
//-----------------------------------------------------------------------------
class ALightingData
{
public:
	// The light settings (partial)
	struct Light
	{
		Toshi::TVector4 vLightDirection;         // 0x00 direction of the sun's light
		Toshi::TVector4 vLightColour;            // 0x10
		TFLOAT*         pfInstanceAmbientScale;  // 0x20
		TINT16          numInstanceAmbientScale; // 0x24
	};

public:
	ALightingData();
	~ALightingData();

	Light* GetCurrent() const { return m_pCurrent; }
	void   SetCurrent( Light* a_pLight ) { m_pCurrent = a_pLight; }

	void   Load( Toshi::TTRB* a_pTRB );
	TFLOAT GetInstanceAmbientScale( TINT a_iIndex ) const;

	static ALightingData* GetSingleton() { return sm_singleton; }

private:
	static ALightingData* sm_singleton;

private:
	Light* m_pCurrent; // 0x00
};                     // 0x04
