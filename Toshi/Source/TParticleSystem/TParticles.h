#pragma once
#include "Math/TVector4.h"
#include "Math/TSphere.h"
#include "Math/TQuaternion.h"
#include "Toshi/TSharedBuffer.h"


TOSHI_NAMESPACE_START

class TParticleMesh;
class TParticleMaterial;

// The shared buffers of the particles (the pools are created by TParticleSystem::Create)
using TParticleV4Buffer    = TSharedBuffer<TVector4, 32, 6223538>;
using TParticleFloatBuffer = TSharedBuffer<TFLOAT, 32, 6052264>;
using TParticleUIntBuffer  = TSharedBuffer<TUINT32, 32, 5884706>;
using TParticleUCharBuffer = TSharedBuffer<TUINT8, 32, 5716292>;
using TParticleQuatBuffer  = TSharedBuffer<TQuaternion, 32, 6391556>;

// The base of the particles a TParticleSystem::TParticlesNode owns (TGenericParticles, TTestParticles), 0x28 bytes
class TParticles
{
public:
	// Writes the vertices of a particle (4 for quads, 2 for ribbons) to the locked mesh
	using t_SetVertsFn = void( __stdcall* )( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	// Writes the indices of a_iNumParticles particles and returns their number
	using t_SetIndicesFn = TINT( __stdcall* )( TParticleMesh* a_pMesh, TINT a_iNumParticles, TParticles* a_pParticles );

public:
	TParticles();
	virtual ~TParticles(); // +0x00

	virtual void  Initialise() = 0;                                          // +0x04
	virtual void  Deinitialise();                                            // +0x08
	virtual TINT  GetVertsPerNumParticles( TINT a_iNumParticles );           // +0x0C
	virtual TINT  GetMaxNumParticles();                                      // +0x10
	virtual void  SetNumRenderingParticles( TINT a_iNumParticles );          // +0x14
	virtual void  SetNumParticles( TINT a_iNumParticles );                   // +0x18
	virtual TBOOL IsVisible();                                               // +0x1C
	virtual void  PreUpdate( TFLOAT a_fDeltaTime );                         // +0x20
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) = 0;                         // +0x24 TFALSE when the particles are finished
	virtual void  Render()                      = 0;                         // +0x28

	TINT GetNumParticles() const { return m_iNumParticles; }
	TINT GetNumRenderingParticles() const { return m_iNumRenderingParticles; }
	TINT GetMinNumParticles() const { return m_iMinNumParticles; }

	// $deBlob: FUNCTION 00705b20
	TParticleMaterial* GetMaterial() const { return m_pMaterial; }

	const TSphere& GetBoundingSphere() const { return m_oBoundingSphere; }

	// Is the sphere in the particle system's world planes
	static TBOOL __stdcall IsSphereVisible( const TVector3* a_pCentre, TFLOAT a_fRadius );

	// Is any of the particles' positions in the particle system's world planes (with a_fRadius)
	TBOOL IsAnyPositionVisible( TParticleV4Buffer& a_rPositions, TFLOAT a_fRadius );

	// Locks meshes of the particles' material and writes the rendering particles to them
	void RenderVerts( t_SetVertsFn a_fnSetPosition, t_SetVertsFn a_fnSetUV, t_SetVertsFn a_fnSetColour, t_SetIndicesFn a_fnSetIndices, TParticles* a_pParticles );

	// The default index writer (two triangles per quad, as a strip with degenerate triangles)
	static TINT __stdcall SetQuadIndices( TParticleMesh* a_pMesh, TINT a_iNumParticles, TParticles* a_pParticles );

public:
	TSphere            m_oBoundingSphere;        // 0x04
	TINT               m_iNumParticles;          // 0x14
	TINT               m_iNumRenderingParticles; // 0x18
	TParticleMaterial* m_pMaterial;              // 0x1C
	TFLOAT             m_fLODDistanceScale;      // 0x20 1/(LOD distance)^2
	TINT               m_iMinNumParticles;       // 0x24
};

TSTATICASSERT( sizeof( TParticles ) == 0x28 );

TOSHI_NAMESPACE_END
