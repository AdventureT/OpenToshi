#pragma once
#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Purpose: the colour maps of BlobProperties.trb (16 colours each, indexed by paint colour), read by
// ABlobEntity::LoadProperties. Load swaps their red and blue bytes in place.
//-----------------------------------------------------------------------------
class ABlobColourMaps
{
public:
	static TCONSTEXPR_VAR TINT NUM_COLOURS = 16;

	static void Load( Toshi::TTRB* a_pTRB );
	// Forgets the maps (ABlobEntity::UnloadProperties); NOTE: retail keeps the enemy maps
	static void Unload();

	static TUINT32* GetBodyColourMap() { return ms_pBodyColourMap; }
	static TUINT32* GetLightColourMap() { return ms_pLightColourMap; }
	static TUINT32* GetTreeColourMap() { return ms_pTreeColourMap; }

public:
	static TUINT32* ms_pColourInterpolationMap; // two colours per entry
	static TUINT32* ms_pBodyColourMap;
	static TUINT32* ms_pLipColourMap;
	static TUINT32* ms_pLightColourMap;
	static TUINT32* ms_pClearBodyColourMap;
	static TUINT32* ms_pClearLipColourMap;
	static TUINT32* ms_pClearLightColourMap;
	static TUINT32* ms_pSkelLightColourMap;
	static TUINT32* ms_pEnemyBodyColourMap;
	static TUINT32* ms_pEnemyLightColourMap;
	static TUINT32* ms_pGlintColourMap;
	static TUINT32* ms_pLandmarkGlowColourMap;
	static TUINT32* ms_pLockedBuildingColour; // one colour
	static TUINT32* ms_pTreeColourMap;
};
