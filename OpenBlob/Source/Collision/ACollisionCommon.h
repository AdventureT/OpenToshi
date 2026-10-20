#pragma once

//-----------------------------------------------------------------------------
// Purpose: the collision helpers shared by the game's collision: the surface types of the collision materials and their
// physics parameters.
//-----------------------------------------------------------------------------
class ACollisionCommon
{
public:
	using SURFACE = TUINT32;
	enum SURFACE_ : SURFACE
	{
		SURFACE_None           = 0,
		SURFACE_Reserved       = BITFLAG( 0 ),
		SURFACE_Stone          = BITFLAG( 1 ),
		SURFACE_Dirt           = BITFLAG( 2 ),
		SURFACE_Mud            = BITFLAG( 3 ),
		SURFACE_Water          = BITFLAG( 4 ),
		SURFACE_Metal          = BITFLAG( 5 ),
		SURFACE_Grass          = BITFLAG( 6 ),
		SURFACE_Road           = BITFLAG( 7 ),
		SURFACE_Bush           = BITFLAG( 8 ),
		SURFACE_Tree           = BITFLAG( 9 ),
		SURFACE_Sand           = BITFLAG( 10 ),
		SURFACE_Wood           = BITFLAG( 11 ),
		SURFACE_Blob           = BITFLAG( 12 ),
		SURFACE_Ink            = BITFLAG( 13 ),
		SURFACE_Ice            = BITFLAG( 14 ),
		SURFACE_Electric       = BITFLAG( 15 ),
		SURFACE_Hot            = BITFLAG( 16 ),
		SURFACE_InkWall        = BITFLAG( 17 ),
		SURFACE_Decal          = BITFLAG( 18 ), // "decal" and "render"
		SURFACE_InkGround      = BITFLAG( 19 ),
		SURFACE_Spikes         = BITFLAG( 20 ),
		SURFACE_MetalSmall     = BITFLAG( 21 ),
		SURFACE_ForceField     = BITFLAG( 22 ),
		SURFACE_Stamper        = BITFLAG( 23 ),
		SURFACE_Glass          = BITFLAG( 24 ),
		SURFACE_Rockslide      = BITFLAG( 25 ),
		SURFACE_ForceFieldEdge = BITFLAG( 26 ),
	};

public:
	// The surface type of a collision material name (case-insensitive)
	static SURFACE GetSurfaceType( const TCHAR* a_szName );

	// The name of a surface flag ("default" for none or several; SURFACE_Decal is "render")
	static const TCHAR* GetSurfaceName( SURFACE a_eSurface );

	// The friction and restitution of a surface type
	static void GetSurfaceParams( SURFACE a_eSurface, TFLOAT& a_rfFriction, TFLOAT& a_rfRestitution );
};
