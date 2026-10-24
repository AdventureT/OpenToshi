#include "pch.h"
#include "ABlobColourMaps.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TUINT32* ABlobColourMaps::ms_pColourInterpolationMap;
TUINT32* ABlobColourMaps::ms_pBodyColourMap;
TUINT32* ABlobColourMaps::ms_pLipColourMap;
TUINT32* ABlobColourMaps::ms_pLightColourMap;
TUINT32* ABlobColourMaps::ms_pClearBodyColourMap;
TUINT32* ABlobColourMaps::ms_pClearLipColourMap;
TUINT32* ABlobColourMaps::ms_pClearLightColourMap;
TUINT32* ABlobColourMaps::ms_pSkelLightColourMap;
TUINT32* ABlobColourMaps::ms_pEnemyBodyColourMap;
TUINT32* ABlobColourMaps::ms_pEnemyLightColourMap;
TUINT32* ABlobColourMaps::ms_pGlintColourMap;
TUINT32* ABlobColourMaps::ms_pLandmarkGlowColourMap;
TUINT32* ABlobColourMaps::ms_pLockedBuildingColour;
TUINT32* ABlobColourMaps::ms_pTreeColourMap;

// Swaps the bytes 0 and 2 (red and blue) of a colour
static void SwapRedBlue( TUINT32* a_pColour )
{
	TUINT8* pBytes = TREINTERPRETCAST( TUINT8*, a_pColour );
	TUINT8  uiTemp = pBytes[ 0 ];
	pBytes[ 0 ]    = pBytes[ 2 ];
	pBytes[ 2 ]    = uiTemp;
}

// $deBlob: FUNCTION 0075cb70
void ABlobColourMaps::Load( TTRB* a_pTRB )
{
	ms_pColourInterpolationMap = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "BlobColourInterpolationMap" ) );
	ms_pBodyColourMap          = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "BlobBodyColourMap" ) );
	ms_pLipColourMap           = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "BlobLipColourMap" ) );
	ms_pLightColourMap         = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "BlobLightColourMap" ) );
	ms_pClearBodyColourMap     = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "BlobClearBodyColourMap" ) );
	ms_pClearLipColourMap      = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "BlobClearLipColourMap" ) );
	ms_pClearLightColourMap    = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "BlobClearLightColourMap" ) );
	ms_pSkelLightColourMap     = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "BlobSkelLightColourMap" ) );
	ms_pEnemyBodyColourMap     = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "EnemyBodyColourMap" ) );
	ms_pEnemyLightColourMap    = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "EnemyLightColourMap" ) );
	ms_pGlintColourMap         = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "GlintColourMap" ) );
	ms_pLandmarkGlowColourMap  = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "LandmarkGlowColourMap" ) );
	ms_pLockedBuildingColour   = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "LockedBuildingColour" ) );
	ms_pTreeColourMap          = TSTATICCAST( TUINT32, a_pTRB->GetSymbolAddress( "TreeColourMap" ) );

	for ( TINT i = 0; i < NUM_COLOURS; i++ )
	{
		SwapRedBlue( &ms_pBodyColourMap[ i ] );
		SwapRedBlue( &ms_pLipColourMap[ i ] );
		SwapRedBlue( &ms_pLightColourMap[ i ] );
		SwapRedBlue( &ms_pClearBodyColourMap[ i ] );
		SwapRedBlue( &ms_pClearLipColourMap[ i ] );
		SwapRedBlue( &ms_pClearLightColourMap[ i ] );
		SwapRedBlue( &ms_pSkelLightColourMap[ i ] );
		SwapRedBlue( &ms_pEnemyBodyColourMap[ i ] );
		SwapRedBlue( &ms_pEnemyLightColourMap[ i ] );
		SwapRedBlue( &ms_pColourInterpolationMap[ i * 2 ] );
		SwapRedBlue( &ms_pColourInterpolationMap[ i * 2 + 1 ] );
		SwapRedBlue( &ms_pGlintColourMap[ i ] );
		SwapRedBlue( &ms_pLandmarkGlowColourMap[ i ] );
		SwapRedBlue( &ms_pTreeColourMap[ i ] );
	}

	SwapRedBlue( ms_pLockedBuildingColour );
}

// $deBlob: FUNCTION 0075d1a0
void ABlobColourMaps::Unload()
{
	ms_pBodyColourMap          = TNULL;
	ms_pColourInterpolationMap = TNULL;
	ms_pLipColourMap           = TNULL;
	ms_pLightColourMap         = TNULL;
	ms_pClearBodyColourMap     = TNULL;
	ms_pClearLipColourMap      = TNULL;
	ms_pClearLightColourMap    = TNULL;
	ms_pSkelLightColourMap     = TNULL;
	ms_pGlintColourMap         = TNULL;
	ms_pLandmarkGlowColourMap  = TNULL;
	ms_pLockedBuildingColour   = TNULL;
	ms_pTreeColourMap          = TNULL;
}
