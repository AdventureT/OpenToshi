#pragma once
#include "ASpriteMaterial.h"

//-----------------------------------------------------------------------------
// Purpose: the DX11 sprite material.
//-----------------------------------------------------------------------------
class ASpriteMaterialHAL
    : public ASpriteMaterial
{
public:
	TDECLARE_CLASS( ASpriteMaterialHAL, ASpriteMaterial );

	ASpriteMaterialHAL();
	virtual ~ASpriteMaterialHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMaterial
	//-----------------------------------------------------------------------------
	virtual void PreRender() OVERRIDE;  // +0x14
	virtual void PostRender() OVERRIDE; // +0x18
};
