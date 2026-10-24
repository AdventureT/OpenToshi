#pragma once
#include "ASpriteMesh.h"

//-----------------------------------------------------------------------------
// Purpose: the DX11 sprite mesh.
//-----------------------------------------------------------------------------
class ASpriteMeshHAL
    : public ASpriteMesh
{
public:
	TDECLARE_CLASS( ASpriteMeshHAL, ASpriteMesh );

	ASpriteMeshHAL();
	virtual ~ASpriteMeshHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() OVERRIDE; // +0x18

	//-----------------------------------------------------------------------------
	// ASpriteMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiMaxIndices ) OVERRIDE; // +0x20
};
