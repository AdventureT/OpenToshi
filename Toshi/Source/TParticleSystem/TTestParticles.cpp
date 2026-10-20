#include "ToshiPCH.h"
#include "TTestParticles.h"
#include "TParticleSystem.h"
#include "TParticleMesh.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 007126b0
TTestParticles::TTestParticles()
{
}

// $deBlob: FUNCTION 007126d0
// $deBlob: FUNCTION 00712a10
// $deBlob: FUNCTION 0070ed50
TTestParticles::~TTestParticles()
{
}

// $deBlob: FUNCTION 007126e0
void TTestParticles::Initialise()
{
	m_fTime = 0.0f;
}

// $deBlob: FUNCTION 007126f0
void TTestParticles::Deinitialise()
{
	TParticles::Deinitialise();
}

// $deBlob: FUNCTION 00712ad0
TINT TTestParticles::GetVertsPerNumParticles( TINT a_iNumParticles )
{
	return a_iNumParticles << 2;
}

// $deBlob: FUNCTION 00712700
TBOOL TTestParticles::Update( TFLOAT a_fDeltaTime )
{
	TFLOAT fTime = a_fDeltaTime + m_fTime;
	m_fTime      = fTime;
	return fTime <= 3.0f;
}

// $deBlob: FUNCTION 00712720
void TTestParticles::Render()
{
	RenderVerts( SetPosition, SetUV, SetColour, TParticles::SetQuadIndices, this );
}

// $deBlob: FUNCTION 00712740
void __stdcall TTestParticles::SetPosition( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TParticleSystem* pSystem = TParticleSystem::s_pCreatingSystem;

	TVector4 v0, v1, v2, v3;
	v0.Multiply( pSystem->m_aOffsets[ 0 ], 3.0f );
	v1.Multiply( pSystem->m_aOffsets[ 2 ], 3.0f );
	v2.Multiply( pSystem->m_aOffsets[ 1 ], 3.0f );
	v3.Multiply( pSystem->m_aOffsets[ 3 ], 3.0f );

	const TVector4& rCentre = a_pParticles->m_oBoundingSphere.AsVector4();
	a_pMesh->SetPosition( a_iVertIndex, v0, rCentre );
	a_pMesh->SetPosition( a_iVertIndex + 1, v1, rCentre );
	a_pMesh->SetPosition( a_iVertIndex + 2, v2, rCentre );
	a_pMesh->SetPosition( a_iVertIndex + 3, v3, rCentre );
}

// $deBlob: FUNCTION 00712870
void __stdcall TTestParticles::SetUV( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	a_pMesh->SetUV( a_iVertIndex, 0.0f, 1.0f );
	a_pMesh->SetUV( a_iVertIndex + 1, 0.0f, 1.0f );
	a_pMesh->SetUV( a_iVertIndex + 2, 0.0f, 1.0f );
	a_pMesh->SetUV( a_iVertIndex + 3, 0.0f, 1.0f );
}

// $deBlob: FUNCTION 00712960
void __stdcall TTestParticles::SetColour( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	a_pMesh->SetColour( a_iVertIndex, 0xFFFFFFFF );
	a_pMesh->SetColour( a_iVertIndex + 1, 0xFFFFFFFF );
	a_pMesh->SetColour( a_iVertIndex + 2, 0xFFFFFFFF );
	a_pMesh->SetColour( a_iVertIndex + 3, 0xFFFFFFFF );
}

TOSHI_NAMESPACE_END
