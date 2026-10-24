#include "pch.h"
#include "ACollisionCommon.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The surface names in the order GetSurfaceType compares them
struct SurfaceName
{
	const TCHAR*              szName;
	ACollisionCommon::SURFACE eSurface;
};

static const SurfaceName s_aSurfaceNames[] = {
	{ "none", ACollisionCommon::SURFACE_None },
	{ "default", ACollisionCommon::SURFACE_None },
	{ "reserved", ACollisionCommon::SURFACE_Reserved },
	{ "stone", ACollisionCommon::SURFACE_Stone },
	{ "dirt", ACollisionCommon::SURFACE_Dirt },
	{ "mud", ACollisionCommon::SURFACE_Mud },
	{ "water", ACollisionCommon::SURFACE_Water },
	{ "metal", ACollisionCommon::SURFACE_Metal },
	{ "metalsmall", ACollisionCommon::SURFACE_MetalSmall },
	{ "grass", ACollisionCommon::SURFACE_Grass },
	{ "road", ACollisionCommon::SURFACE_Road },
	{ "bush", ACollisionCommon::SURFACE_Bush },
	{ "tree", ACollisionCommon::SURFACE_Tree },
	{ "sand", ACollisionCommon::SURFACE_Sand },
	{ "wood", ACollisionCommon::SURFACE_Wood },
	{ "blob", ACollisionCommon::SURFACE_Blob },
	{ "ink", ACollisionCommon::SURFACE_Ink },
	{ "ice", ACollisionCommon::SURFACE_Ice },
	{ "electric", ACollisionCommon::SURFACE_Electric },
	{ "hot", ACollisionCommon::SURFACE_Hot },
	{ "inkwall", ACollisionCommon::SURFACE_InkWall },
	{ "render", ACollisionCommon::SURFACE_Decal },
	{ "inkground", ACollisionCommon::SURFACE_InkGround },
	{ "spikes", ACollisionCommon::SURFACE_Spikes },
	{ "forcefield", ACollisionCommon::SURFACE_ForceField },
	{ "stamper", ACollisionCommon::SURFACE_Stamper },
	{ "glass", ACollisionCommon::SURFACE_Glass },
	{ "rockslide", ACollisionCommon::SURFACE_Rockslide },
	{ "decal", ACollisionCommon::SURFACE_Decal },
	{ "forcefieldedge", ACollisionCommon::SURFACE_ForceFieldEdge },
};

// $deBlob: FUNCTION 005a2c30
const TCHAR* ACollisionCommon::GetSurfaceName( SURFACE a_eSurface )
{
	switch ( a_eSurface )
	{
		case SURFACE_Reserved: return "reserved";
		case SURFACE_Stone: return "stone";
		case SURFACE_Dirt: return "dirt";
		case SURFACE_Mud: return "mud";
		case SURFACE_Water: return "water";
		case SURFACE_Metal: return "metal";
		case SURFACE_Grass: return "grass";
		case SURFACE_Road: return "road";
		case SURFACE_Bush: return "bush";
		case SURFACE_Tree: return "tree";
		case SURFACE_Sand: return "sand";
		case SURFACE_Wood: return "wood";
		case SURFACE_Blob: return "blob";
		case SURFACE_Ink: return "ink";
		case SURFACE_Ice: return "ice";
		case SURFACE_Electric: return "electric";
		case SURFACE_Hot: return "hot";
		case SURFACE_InkWall: return "inkwall";
		case SURFACE_Decal: return "render";
		case SURFACE_InkGround: return "inkground";
		case SURFACE_Spikes: return "spikes";
		case SURFACE_MetalSmall: return "metalsmall";
		case SURFACE_ForceField: return "forcefield";
		case SURFACE_Stamper: return "stamper";
		case SURFACE_Glass: return "glass";
		case SURFACE_Rockslide: return "rockslide";
		case SURFACE_ForceFieldEdge: return "forcefieldedge";
		default: return "default";
	}
}

// $deBlob: FUNCTION 005a2e40
ACollisionCommon::SURFACE ACollisionCommon::GetSurfaceType( const TCHAR* a_szName )
{
	for ( TSIZE i = 0; i < TARRAYSIZE( s_aSurfaceNames ); i++ )
	{
		if ( TStringManager::String8CompareNoCase( a_szName, s_aSurfaceNames[ i ].szName ) == 0 )
			return s_aSurfaceNames[ i ].eSurface;
	}

	TASSERT( 0, "ACollisionInstance2: Unknown surface type.\n" );
	return SURFACE_None;
}

// $deBlob: FUNCTION 005a3220
void ACollisionCommon::GetSurfaceParams( SURFACE a_eSurface, TFLOAT& a_rfFriction, TFLOAT& a_rfRestitution )
{
	switch ( a_eSurface )
	{
		case SURFACE_Ice:
			a_rfFriction    = 0.0f;
			a_rfRestitution = 0.0f;
			break;
		case SURFACE_InkWall:
		case SURFACE_Glass:
			a_rfFriction    = 0.0f;
			a_rfRestitution = 0.1f;
			break;
		case SURFACE_Metal:
		case SURFACE_MetalSmall:
			a_rfFriction    = 0.5f;
			a_rfRestitution = 0.1f;
			break;
		default:
			a_rfFriction    = 0.75f;
			a_rfRestitution = 0.1f;
			break;
	}
}
