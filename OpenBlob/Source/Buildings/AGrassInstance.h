#pragma once
#include "ABuildingInstance.h"

//-----------------------------------------------------------------------------
// Purpose: a patch of grass of the terrain (cell instance type 4, TerrainTypes::InstanceType_Grass): an
// ABuildingInstance that is painted once (the jelly paint from white, AJellyShader_DX11) and then stays painted
// (ACellBucket gives the painted grass its textures). Created by AModel2::CreateInstance (type 4), its terrain
// callbacks are in ATerrain2Interface's per type tables.
//-----------------------------------------------------------------------------
class AGrassInstance
    : public ABuildingInstance
{
public:
	AGrassInstance( AModel2* a_pModel, TBOOL a_bCreateSoundBreakpoints );
	~AGrassInstance();

	virtual void UpdateImp( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x00
	virtual TINT Reset() OVERRIDE;                          // +0x14

	// The callbacks of TerrainTypes::InstanceType_Grass in ATerrain2Interface's per type tables
	static void OnCreated( TerrainTypes::CellInstance* a_pInstance );
	static void OnPaint( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob );

	// Starts the paint of the grass (once)
	void Paint();

	TBOOL IsPainted() const { return m_bPainted; }

public:
	TBOOL m_bPainted;  // 0xCC the paint is over
	TBOOL m_bPainting; // 0xCD the jelly paint runs
};

TSTATICASSERT( sizeof( AGrassInstance ) == 0xD0 );
