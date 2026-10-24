#pragma once

TOSHI_NAMESPACE_START

// A collision surface (12 bytes): what an AI's ground is made of
struct TCollisionSurface
{
	TINT         m_iIndex; // 0x0
	TINT         m_iType;  // 0x4
	const TCHAR* m_szName; // 0x8
};

//-----------------------------------------------------------------------------
// Purpose: the collision surfaces (dust, rock, sand, goo, water, metal, grass, wood), picked by the group flag bits
// 8..15 of a collision
//-----------------------------------------------------------------------------
class CollisionSurfaces
{
public:
	static TCONSTEXPR_VAR TINT NUM_SURFACES = 8;

	static const TCollisionSurface* GetCollisionSurface( TUINT32 a_eGroupFlag );

	// $deBlob: FUNCTION 005ac9f0
	static const TCollisionSurface* GetDefaultSurface() { return &ms_aSurfaces[ 0 ]; }

private:
	static const TCollisionSurface ms_aSurfaces[ NUM_SURFACES ];
};

TOSHI_NAMESPACE_END
