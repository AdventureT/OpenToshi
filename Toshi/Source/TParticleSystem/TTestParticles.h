#pragma once
#include "TParticles.h"


TOSHI_NAMESPACE_START

// A test quad at the bounding sphere's centre that lives for 3 seconds (0x2C bytes), pooled by
// TParticleSystem as PARTICLES_TEST
class TTestParticles : public TParticles
{
public:
	TTestParticles();
	~TTestParticles();

	//-----------------------------------------------------------------------------
	// TParticles
	//-----------------------------------------------------------------------------
	virtual void  Initialise() OVERRIDE;                                    // +0x04
	virtual void  Deinitialise() OVERRIDE;                                  // +0x08
	virtual TINT  GetVertsPerNumParticles( TINT a_iNumParticles ) OVERRIDE; // +0x0C
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE;                   // +0x24
	virtual void  Render() OVERRIDE;                                        // +0x28

	static void __stdcall SetPosition( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetUV( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetColour( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );

private:
	TFLOAT m_fTime; // 0x28
};

TSTATICASSERT( sizeof( TTestParticles ) == 0x2C );

TOSHI_NAMESPACE_END
