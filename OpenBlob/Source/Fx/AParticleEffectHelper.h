#pragma once

namespace Toshi
{
class TParticleEffect;
class TVector4;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: helpers for particle effects.
//-----------------------------------------------------------------------------
class AParticleEffectHelper
{
public:
	// Sets the RGB (not the alpha) of the fixed colour init and the colour keys of the systems of a_pEffect whose
	// settings name starts with a_szSystem (all when TNULL)
	static void SetColour( Toshi::TParticleEffect* a_pEffect, TUINT32 a_uiColour, const TCHAR* a_szSystem );
	// Sets the radius (0x0C) of the TYPEV4_RANDDIR2 position inits of the systems of a_pEffect
	static void SetPositionRadius( Toshi::TParticleEffect* a_pEffect, TFLOAT a_fRadius );
	// Sets the half extents of the TYPEV4_MINMAX (-extent...+extent) and TYPEV4_CUBESURFACE position inits of the
	// systems of a_pEffect (Y only when a_fY >= 0); SetPositionExtentsXZ uses a_fXZ for X and Z
	static void SetPositionExtents( Toshi::TParticleEffect* a_pEffect, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ );
	static void SetPositionExtentsXZ( Toshi::TParticleEffect* a_pEffect, TFLOAT a_fXZ );
	// Gets the size init of the system a_iSystem of a_pEffect (TYPEFLOAT_FIXED: both its value, TYPEFLOAT_MINMAX: its
	// range); returns its type
	static TINT GetSize( Toshi::TParticleEffect* a_pEffect, TINT a_iSystem, TFLOAT& a_rfMin, TFLOAT& a_rfMax );
	// Sets the size init of the systems of a_pEffect (its type to a_iType first when it's not -1); see GetSize
	static void SetSize( Toshi::TParticleEffect* a_pEffect, TFLOAT a_fMin, TFLOAT a_fMax, TINT a_iType );

	// Sets the particle count of the only system of a_pEffect
	static void SetNumParticles( Toshi::TParticleEffect* a_pEffect, TINT a_iNumParticles );

	// Sets the particle count of the systems of a_pDstEffect (whose settings name starts with a_szSystem, all when
	// TNULL) to the one of a_pSrcEffect scaled by a_fScale; returns the highest new count
	// Scales the position init of the systems of a_pEffect (TYPEV4_MINMAX and TYPEV4_CUBESURFACE) to the one of
	// a_pBaseEffect times a_pScale (its Y only when >= 0)
	static void ScalePositionInit( Toshi::TParticleEffect* a_pEffect, const Toshi::TParticleEffect* a_pBaseEffect, const Toshi::TVector4* a_pScale );

	static TINT ScaleNumParticles( const Toshi::TParticleEffect* a_pSrcEffect, Toshi::TParticleEffect* a_pDstEffect, TFLOAT a_fScale, const TCHAR* a_szSystem );
};
