#include "ToshiPCH.h"
#include "TParticles.h"
#include "TParticleSystem.h"
#include "TParticleMesh.h"
#include "TParticleMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00719910
TParticles::TParticles()
{
	m_oBoundingSphere.AsVector4() = TVector4::VEC_ZERO;
	m_pMaterial                   = TNULL;
	m_iNumParticles               = 0;
	m_iNumRenderingParticles      = 0;
	m_fLODDistanceScale           = 1.0f / ( 300.0f * 300.0f );
	m_iMinNumParticles            = 0;
}

// $deBlob: FUNCTION 00719950
// $deBlob: FUNCTION 00719e70
TParticles::~TParticles()
{
}

// $deBlob: FUNCTION 00719960
void TParticles::Deinitialise()
{
	SetNumParticles( 0 );
}

// $deBlob: FUNCTION 00719ec0
TINT TParticles::GetVertsPerNumParticles( TINT a_iNumParticles )
{
	return a_iNumParticles * 4;
}

// $deBlob: FUNCTION 00712aa0
TINT TParticles::GetMaxNumParticles()
{
	return m_iNumParticles;
}

// $deBlob: FUNCTION 00712b70
void TParticles::SetNumRenderingParticles( TINT a_iNumParticles )
{
	TASSERT( a_iNumParticles >= 0 && a_iNumParticles <= m_iNumParticles );
	m_iNumRenderingParticles = a_iNumParticles;
}

// $deBlob: FUNCTION 00719970
void TParticles::SetNumParticles( TINT a_iNumParticles )
{
	m_iNumParticles = TUINT16( a_iNumParticles );

	if ( m_iNumRenderingParticles > m_iNumParticles )
		m_iNumRenderingParticles = m_iNumParticles;
}

// $deBlob: FUNCTION 00719990
TBOOL TParticles::IsVisible()
{
	return IsSphereVisible( &m_oBoundingSphere.GetOrigin(), m_oBoundingSphere.GetRadius() );
}

// $deBlob: FUNCTION 007199b0
void TParticles::PreUpdate( TFLOAT a_fDeltaTime )
{
}

// $deBlob: FUNCTION 007199c0
TBOOL __stdcall TParticles::IsSphereVisible( const TVector3* a_pCentre, TFLOAT a_fRadius )
{
	TParticleSystem* pSystem = TParticleSystem::s_pCreatingSystem;

	if ( pSystem == TNULL )
		return TFALSE;

	for ( TUINT i = 0; i < TUINT( pSystem->m_iNumWorldPlanes ); i++ )
	{
		const TPlane& rPlane = pSystem->m_aWorldPlanes[ i ];

		if ( rPlane.GetDistance( *a_pCentre ) < -a_fRadius )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00719a50
TBOOL TParticles::IsAnyPositionVisible( TParticleV4Buffer& a_rPositions, TFLOAT a_fRadius )
{
	TParticleSystem* pSystem = TParticleSystem::s_pCreatingSystem;

	if ( pSystem == TNULL )
		return TFALSE;

	TUINT uiNumPlanes = pSystem->m_iNumWorldPlanes;

	for ( TUINT i = 0; i < TUINT( m_iNumParticles ); i++ )
	{
		TUINT j = 0;

		if ( uiNumPlanes == 0 )
			return TTRUE;

		while ( TTRUE )
		{
			const TPlane&   rPlane    = pSystem->m_aWorldPlanes[ j ];
			const TVector4& rPosition = a_rPositions[ i ];

			if ( rPlane.GetDistance( rPosition ) < -a_fRadius )
				break;

			if ( ++j >= uiNumPlanes )
				return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00719b30
void TParticles::RenderVerts( t_SetVertsFn a_fnSetPosition, t_SetVertsFn a_fnSetUV, t_SetVertsFn a_fnSetColour, t_SetIndicesFn a_fnSetIndices, TParticles* a_pParticles )
{
	if ( m_iNumRenderingParticles <= 0 )
		return;

	TINT  iNumVertsLeft = m_iNumRenderingParticles * 4;
	TBOOL bDone         = TFALSE;
	TINT  iParticle     = 0;

	do
	{
		TParticleMaterial* pMaterial = m_pMaterial;
		TParticleMesh*     pMesh     = TParticleSystem::GetSingleton()->GetMesh( iNumVertsLeft, GetVertsPerNumParticles( 1 ), pMaterial );

		if ( pMesh == TNULL )
			return;

		pMesh->SetMeshFlags( 0x80, TTRUE );
		pMesh->SetMeshFlags( 0x100, TFALSE );

		TINT iNumParticles = pMesh->GetMaxVertices() / GetVertsPerNumParticles( 1 );

		if ( iNumParticles == 0 )
			return;

		if ( m_iNumRenderingParticles <= iNumParticles + iParticle )
		{
			bDone         = TTRUE;
			iNumParticles = m_iNumRenderingParticles - iParticle;
		}

		iNumVertsLeft -= GetVertsPerNumParticles( iNumParticles );

		for ( TINT i = 0; i < iNumParticles; i++ )
		{
			TINT iVertIndex = i * 4;
			a_fnSetColour( pMesh, iVertIndex, iParticle, a_pParticles );
			a_fnSetUV( pMesh, iVertIndex, iParticle, a_pParticles );
			a_fnSetPosition( pMesh, iVertIndex, iParticle, a_pParticles );
			iParticle++;
		}

		TINT iNumVerts   = GetVertsPerNumParticles( iNumParticles );
		TINT iNumIndices = a_fnSetIndices( pMesh, iNumParticles, a_pParticles );
		pMesh->Unlock( iNumVerts, iNumIndices );
	} while ( !bDone );
}

// $deBlob: FUNCTION 00719cf0
TINT __stdcall TParticles::SetQuadIndices( TParticleMesh* a_pMesh, TINT a_iNumParticles, TParticles* a_pParticles )
{
	TASSERT( a_iNumParticles >= 0 );

	TUINT16 uiIndex = 0;
	TUINT16 uiVert  = 0;

	for ( TINT i = 0; i < a_iNumParticles; i++ )
	{
		a_pMesh->SetIndex( uiIndex, uiVert );
		a_pMesh->SetIndex( uiIndex + 1, uiVert );
		a_pMesh->SetIndex( uiIndex + 2, uiVert + 1 );
		a_pMesh->SetIndex( uiIndex + 3, uiVert + 2 );
		a_pMesh->SetIndex( uiIndex + 4, uiVert + 3 );
		a_pMesh->SetIndex( uiIndex + 5, uiVert + 3 );
		uiIndex += 6;
		uiVert += 4;
	}

	return a_iNumParticles * 6;
}

TOSHI_NAMESPACE_END
