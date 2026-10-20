#include "ToshiPCH.h"
#include "TParticleSystem.h"
#include "TParticleEffectManager.h"
#include "TParticleMaterialManager.h"
#include "TParticleMesh.h"
#include "TGenericParticles.h"
#include "TParticleMaterial.h"
#include "TTestParticles.h"
#include "Shaders/SysShader/TSysShader.h"
#include "File/TTRB.h"
#include "Render/TAssetInit.h"
#include "Render/TRender.h"
#include "Render/TRenderContext.h"
#include "Render/TMesh.h"
#include "Platform/DX11/TRender_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TParticleSystem* TParticleSystem::s_pSingleton;
TParticleSystem* TParticleSystem::s_pCreatingSystem;
// $deBlob: FUNCTION 0053c540
TParticleSystem::ACreateData TParticleSystem::s_oCreateData;

static void* GenericParticles_GetSettings( TParticles* a_pParticles )
{
	return TSTATICCAST( TGenericParticles, a_pParticles )->GetSettings();
}

// $deBlob: FUNCTION 0070bb30
TParticleSystem::TParticleSystem( const ACreateData& a_rCreateData )
{
	m_pTRB                  = TNULL;
	m_pAllocedParticleNodes = TNULL;
	m_pLockedMeshes         = TNULL;
	m_ppAppendedTRBs        = TNULL;
	m_uiFlags               = 1;

	TASSERT( TNULL == s_pSingleton );
	s_pSingleton = this;

	m_iUnk16C          = 0;
	m_iUnk170          = 0;
	m_pEffectManager   = TNULL;
	m_pMaterialManager = new TParticleMaterialManager();
	m_iDetail          = -1;
	m_fLODStart        = 0.7f;
	m_iMaxParticles    = 2000;
	m_bMaterialsDirty  = TFALSE;

	m_aWorldPlanes[ 0 ] = TPlane( TVector4( 1.0f, 0.0f, 0.0f, 1.0f ) );
	m_aWorldPlanes[ 1 ] = TPlane( TVector4( -1.0f, 0.0f, 0.0f, 1.0f ) );
	m_aWorldPlanes[ 2 ] = TPlane( TVector4( 0.0f, 1.0f, 0.0f, 1.0f ) );
	m_aWorldPlanes[ 3 ] = TPlane( TVector4( 0.0f, -1.0f, 0.0f, 1.0f ) );
	m_aWorldPlanes[ 4 ] = TPlane( TVector4( 0.0f, 0.0f, 1.0f, 1.0f ) );
	m_aWorldPlanes[ 5 ] = TPlane( TVector4( 0.0f, 0.0f, -1.0f, 1.0f ) );

	m_pTestParticles                     = TNULL;
	m_aiNumSystems[ PARTICLES_TEST ]    = 0;
	m_pGenericParticles                  = TNULL;
	m_aiNumSystems[ PARTICLES_GENERIC ] = 0;

	m_iNumAppendedTRBs = a_rCreateData.m_iNumAppendedTRBs;

	if ( m_iNumAppendedTRBs > 0 )
	{
		m_ppAppendedTRBs = TSTATICCAST( TTRB*, TMemalign( 16, m_iNumAppendedTRBs * sizeof( TTRB* ) ) );
		TUtil::MemClear( m_ppAppendedTRBs, m_iNumAppendedTRBs * sizeof( TTRB* ) );
	}

	s_pCreatingSystem = this;
	Create( a_rCreateData );
}

// $deBlob: FUNCTION 0070bdd0
// $deBlob: FUNCTION 0056c2e0
TParticleSystem::~TParticleSystem()
{
	StopAllParticles();
	TRenderDX11::Interface()->WaitForEndOfRender();
	DestroyMeshes();
	Destroy();

	if ( m_pEffectManager )
		delete m_pEffectManager;

	operator delete( m_pMaterialManager );
	operator delete( m_ppAppendedTRBs );
	s_pSingleton = TNULL;
}

// $deBlob: FUNCTION 0070bf70
void TParticleSystem::LoadTRB( const TCHAR* a_szName )
{
	TCHAR szPath[ 256 ];
	T2String8::Format( szPath, sizeof( szPath ), "%s.trb", a_szName );

	TTRB*       pTRB    = new TTRB();
	TTRB::ERROR eResult = pTRB->Load( szPath );
	TASSERT( eResult == TTRB::ERROR_OK );

	LoadTRB( pTRB, a_szName, TTRUE );
}

// $deBlob: FUNCTION 0070c020
void TParticleSystem::LoadTRB( TTRB* a_pTRB, const TCHAR* a_szName, TBOOL a_bOwnsTRB )
{
	if ( a_bOwnsTRB )
		m_uiFlags |= Flags_OwnsTRB;
	else
		m_uiFlags &= ~Flags_OwnsTRB;

	StopAllParticles();
	Destroy();

	TASSERT( TNULL == m_pTRB );
	m_pTRB = a_pTRB;
	TAssetInit::InitAssets( *a_pTRB, TTRUE, TFALSE );

	// ResizeLockedMeshes (inlined, without the wait and the free)
	TUINT uiNumMaterials = m_pMaterialManager->GetMaterialCount();

	if ( uiNumMaterials != 0 )
	{
		m_pLockedMeshes = TSTATICCAST( TParticleMesh, TMemalign( 16, uiNumMaterials * sizeof( TParticleMesh ) ) );

		for ( TUINT i = 0; i < uiNumMaterials; i++ )
			new ( &m_pLockedMeshes[ i ] ) TParticleMesh();
	}
}

// $deBlob: FUNCTION 0070c110
TINT TParticleSystem::AppendTRB( TTRB* a_pTRB )
{
	for ( TINT i = 0; i < m_iNumAppendedTRBs; i++ )
	{
		if ( m_ppAppendedTRBs[ i ] == TNULL )
		{
			m_ppAppendedTRBs[ i ] = a_pTRB;
			StopAllParticles();
			TAssetInit::InitAssets( *m_ppAppendedTRBs[ i ], TTRUE, TFALSE );
			ResizeLockedMeshes();
			return i;
		}
	}

	return -1;
}

// $deBlob: FUNCTION 0070c180
void TParticleSystem::RemoveTRB( TINT a_iIndex )
{
	if ( a_iIndex >= 0 && a_iIndex < m_iNumAppendedTRBs && m_ppAppendedTRBs[ a_iIndex ] != TNULL )
	{
		StopAllParticles();
		TAssetInit::DeinitAssets( *m_ppAppendedTRBs[ a_iIndex ] );
		ResizeLockedMeshes();
		m_ppAppendedTRBs[ a_iIndex ]->Close();

		if ( TTRB* pTRB = m_ppAppendedTRBs[ a_iIndex ] )
			delete pTRB;

		m_ppAppendedTRBs[ a_iIndex ] = TNULL;
	}
}

// $deBlob: FUNCTION 0070c210
void TParticleSystem::Destroy()
{
	TRenderDX11::Interface()->WaitForEndOfRender();
	operator delete[]( m_pLockedMeshes );
	m_pLockedMeshes = TNULL;

	if ( m_pTRB != TNULL )
	{
		TAssetInit::DeinitAssets( *m_pTRB );

		if ( m_uiFlags & 1 )
		{
			m_pTRB->Close();

			if ( m_pTRB )
				delete m_pTRB;
		}

		m_pTRB = TNULL;
	}

	for ( TINT i = 0; i < m_iNumAppendedTRBs; i++ )
	{
		if ( m_ppAppendedTRBs[ i ] != TNULL )
		{
			TAssetInit::DeinitAssets( *m_ppAppendedTRBs[ i ] );
			m_ppAppendedTRBs[ i ]->Close();

			if ( TTRB* pTRB = m_ppAppendedTRBs[ i ] )
				delete pTRB;

			m_ppAppendedTRBs[ i ] = TNULL;
		}
	}

	TRender::GetSingleton()->FlushDyingResources();
	TRender::GetSingleton()->FlushDyingResources();
	TRender::GetSingleton()->FlushDyingResources();
}

// $deBlob: FUNCTION 0070c300
void TParticleSystem::SetDetail( TINT a_iDetail, const ACreateData* a_pCreateData )
{
	TBOOL bChanged = m_iDetail != a_iDetail;
	m_iDetail      = a_iDetail;

	if ( bChanged )
	{
		TASSERT( m_oUsedMeshes.Count() == 0 );
		DestroyMeshes();
	}

	TINT iNumSystems = 50;

	if ( a_pCreateData != TNULL )
	{
		m_iMaxNumMeshes     = a_pCreateData->m_iMaxNumMeshes;
		m_iMaxParticles = a_pCreateData->m_iMaxParticles;
		iNumSystems   = a_pCreateData->m_iNumSystems;
	}

	TASSERT( !( iNumSystems != m_aiNumSystems[ PARTICLES_GENERIC ] ) || m_oUsedParticles.Count() == 0 );

	m_fLODStart   = 0.7f;
	m_aiNumSystems[ PARTICLES_TEST ]    = 0;
	m_aiNumSystems[ PARTICLES_GENERIC ] = iNumSystems;

	if ( a_iDetail == 0 )
	{
		m_fLODStart     = 1.0f;
		m_iMaxParticles = 2000;
		m_iMaxNumMeshes       = 20;
	}
	else if ( a_iDetail == 1 )
	{
		m_fLODStart     = 0.85f;
		m_iMaxParticles = 5000;
		m_iMaxNumMeshes       = 35;
	}
	else
	{
		m_fLODStart     = 0.7f;
		m_iMaxParticles = 10000;
		m_iMaxNumMeshes       = 50;
	}

	m_aiNumSystems[ PARTICLES_TEST ]    = 0;
	m_aiNumSystems[ PARTICLES_GENERIC ] = iNumSystems;

	if ( bChanged )
		CreateMeshes();
}

// $deBlob: FUNCTION 0070c460
void TParticleSystem::Render()
{
	if ( m_pMaterialManager == TNULL )
		return;

	// The meshes rendered last frame can be used again
	if ( m_bUpdated )
		m_oFreeMeshes.InsertListTail( m_oUsedMeshes );

	GetParticleOffsets();

	// The planes the particles are culled with (facing in)
	TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();
	m_iNumWorldPlanes        = 6;
	pContext->GetWorldPlanes( m_aWorldPlanes, 6 );

	for ( TINT i = 0; i < m_iNumWorldPlanes; i++ )
	{
		TVector4& rPlane = m_aWorldPlanes[ i ].AsVector4();
		rPlane.x         = -rPlane.x;
		rPlane.y         = -rPlane.y;
		rPlane.z         = -rPlane.z;
		rPlane.w         = -rPlane.w;
	}

	const TMatrix44& rViewWorld = TRender::GetSingleton()->GetCurrentContext()->GetViewWorldMatrix();
	TVector4         vCamPos    = rViewWorld.GetTranslation();

	TFLOAT fLODStart      = m_fLODStart;
	TFLOAT fInvLODStart   = 1.0f / fLODStart;
	TINT   iNumMaterials  = m_pMaterialManager->GetMaterialCount();
	TUINT  uiNumVerts     = 0;

	// The number of particles of each system, by the distance (LOD)
	for ( auto it = m_oUsedParticles.Begin(); it != m_oUsedParticles.End(); it++ )
	{
		TParticles* pParticles = it->m_pParticles;

		if ( !pParticles->IsVisible() )
		{
			pParticles->SetNumRenderingParticles( 0 );
			continue;
		}

		TFLOAT fDistanceSq = TVector4::DistanceSq( vCamPos, pParticles->m_oBoundingSphere.AsVector4() );
		TFLOAT fLOD        = 1.0f - pParticles->m_fLODDistanceScale * fDistanceSq;
		TINT   iNumParticles;

		if ( 0.0f <= fLOD )
		{
			TASSERT( pParticles->GetMaxNumParticles() >= pParticles->GetMinNumParticles() );

			TINT iMinParticles = pParticles->m_iMinNumParticles;
			TINT iRange        = pParticles->GetMaxNumParticles() - iMinParticles;

			fLOD          = ( fLOD < fLODStart ) ? fLOD * fInvLODStart : 1.0f;
			iNumParticles = TINT( TFLOAT( iRange ) * fLOD ) + pParticles->m_iMinNumParticles;
		}
		else
		{
			iNumParticles = 0;
		}

		pParticles->SetNumRenderingParticles( iNumParticles );

		if ( it->m_pParticles->m_pMaterial != TNULL )
			uiNumVerts += pParticles->GetVertsPerNumParticles( iNumParticles );
	}

	// Too many vertices: fewer particles for every system
	if ( TUINT( m_iMaxParticles ) < uiNumVerts )
	{
		TFLOAT fScale = TFLOAT( m_iMaxParticles ) / TFLOAT( TINT( uiNumVerts ) );

		for ( auto it = m_oUsedParticles.Begin(); it != m_oUsedParticles.End(); it++ )
		{
			TParticles* pParticles = it->m_pParticles;

			if ( TINT( pParticles->m_pMaterial->m_uiVer ) >= 0 )
				pParticles->SetNumRenderingParticles( TINT( TUINT( TFLOAT( pParticles->m_iNumRenderingParticles ) * fScale ) ) );
		}
	}

	// The particles of each material, written to the material's meshes
	for ( TINT iMaterial = iNumMaterials - 1; iMaterial >= 0; iMaterial-- )
	{
		for ( auto it = m_oUsedParticles.Begin(); it != m_oUsedParticles.End(); it++ )
		{
			if ( TINT( it->m_pParticles->m_pMaterial->m_uiVer ) == iMaterial )
				it->m_pParticles->Render();
		}

		if ( m_pLockedMeshes[ iMaterial ].m_pMesh != TNULL )
			m_pLockedMeshes[ iMaterial ].ForceUnlock();

		pContext                 = TRender::GetSingleton()->GetCurrentContext();
		TMatrix44 oModelView     = pContext->GetModelViewMatrix();

		for ( auto it = m_oRenderMeshes.Begin(); it != m_oRenderMeshes.End(); it++ )
		{
			pContext->SetModelViewMatrix( it->m_oModelView );
			it->m_pMesh->Render();
		}

		m_oUsedMeshes.InsertListTail( m_oRenderMeshes ); // inlined
		pContext->SetModelViewMatrix( oModelView );
	}

	// The particles without a material ID render themselves (the meshes)
	for ( auto it = m_oUsedParticles.Begin(); it != m_oUsedParticles.End(); it++ )
	{
		if ( TINT( it->m_pParticles->m_pMaterial->m_uiVer ) < 0 )
			it->m_pParticles->Render();
	}

	m_bUpdated = TFALSE;
}

// $deBlob: FUNCTION 0070c870
void TParticleSystem::Update( TFLOAT a_fDeltaTime )
{
	if ( m_pMaterialManager == TNULL )
		return;

	if ( m_bMaterialsDirty )
	{
		m_pMaterialManager->UpdateMaterials();
		m_bMaterialsDirty = TFALSE;
	}

	for ( auto it = m_oUsedParticles.Begin(); it != m_oUsedParticles.End(); it++ )
		it->m_pParticles->PreUpdate( a_fDeltaTime );

	m_pEffectManager->Update( a_fDeltaTime );

	for ( auto it = m_oUsedParticles.Begin(); it != m_oUsedParticles.End(); )
	{
		TParticlesNode* pNode = it++;

		if ( !pNode->m_pParticles->Update( a_fDeltaTime ) )
		{
			m_oUsedParticles.Remove( pNode );
			pNode->m_pParticles->Deinitialise();

			TASSERT( pNode->m_pParticles->m_iNumParticles == 0 && "pNode->m_pParticles->GetNumParticles() == 0" );
			TASSERT( pNode->m_pParticles->m_iNumRenderingParticles == 0 && "pNode->m_pParticles->GetNumRenderingParticles() == 0" );
			TASSERT( pNode->m_eType >= 0 );
			TASSERT( pNode->m_eType < PARTICLES_NUMOF );

			m_aFreeParticles[ pNode->m_eType ].InsertHead( pNode );
		}
	}

	m_bUpdated = TTRUE;
}

// $deBlob: FUNCTION 0070cb00
TParticles* TParticleSystem::CreateParticles( PARTICLES a_eType )
{
	TASSERT( a_eType < PARTICLES_NUMOF );

	if ( m_aFreeParticles[ a_eType ].Count() > 0 )
	{
		TParticlesNode* pNode = m_aFreeParticles[ a_eType ].RemoveHead();
		m_oUsedParticles.InsertTail( pNode );

		return pNode->m_pParticles;
	}

	return TNULL;
}

// $deBlob: FUNCTION 0070cbc0
void TParticleSystem::StopAllParticles()
{
	m_pEffectManager->StopAllEffects();

	while ( !m_oUsedParticles.IsEmpty() )
	{
		TParticlesNode* pNode = m_oUsedParticles.RemoveHead();
		pNode->m_pParticles->Deinitialise();
		m_aFreeParticles[ pNode->m_eType ].InsertHead( pNode );
	}
}

// $deBlob: FUNCTION 0070ccb0
void TParticleSystem::StopParticles( TParticles* a_pParticles )
{
	if ( a_pParticles == TNULL )
		return;

	for ( auto it = m_oUsedParticles.Begin(); it != m_oUsedParticles.End(); )
	{
		TParticlesNode* pNode = it++;

		if ( pNode->m_pParticles == a_pParticles )
		{
			m_oUsedParticles.Remove( pNode );
			pNode->m_pParticles->Deinitialise();
			m_aFreeParticles[ pNode->m_eType ].InsertHead( pNode );
			return;
		}
	}
}

// $deBlob: FUNCTION 0070cdf0
void TParticleSystem::StopAllGenericParticles( void* a_pSettings )
{
	for ( auto it = m_oUsedParticles.Begin(); it != m_oUsedParticles.End(); )
	{
		TParticlesNode* pNode = it++;

		if ( pNode->m_eType == PARTICLES_GENERIC && GenericParticles_GetSettings( pNode->m_pParticles ) == a_pSettings )
		{
			m_oUsedParticles.Remove( pNode );
			pNode->m_pParticles->Deinitialise();
			m_aFreeParticles[ pNode->m_eType ].InsertHead( pNode );
		}
	}
}

// $deBlob: FUNCTION 0070cf40
TMatrix44& TParticleSystem::GetModelView()
{
	TRender* pRender = TRender::GetSingleton();
	TASSERT( pRender->IsInScene() && "pRI->IsInScene()" );

	return pRender->GetCurrentContext()->GetModelViewMatrix();
}

// $deBlob: FUNCTION 0070cf90
void TParticleSystem::SetModelView( const TMatrix44& a_rModelView )
{
	TRender* pRender = TRender::GetSingleton();
	TASSERT( pRender->IsInScene() && "pRI->IsInScene()" );

	for ( TINT i = 0; i < m_pMaterialManager->GetMaterialCount(); i++ )
	{
		if ( m_pLockedMeshes[ i ].m_pMesh != TNULL )
			m_pLockedMeshes[ i ].ForceUnlock();
	}

	pRender->GetCurrentContext()->SetModelViewMatrix( a_rModelView );

	// GetParticleOffsets (inlined)
	GetParticleOffsets();
}

// $deBlob: FUNCTION 0070d1b0
TINT TParticleSystem::GetMaxQuads() const
{
	return m_iMaxParticles >> 2;
}

// $deBlob: FUNCTION 0070d1c0
TINT TParticleSystem::GetNumGenericSystems() const
{
	return m_aiNumSystems[ PARTICLES_GENERIC ];
}

// $deBlob: FUNCTION 0070d1d0
TINT TParticleSystem::GetNumRenderingParticles() const
{
	TINT iCount = 0;

	for ( auto it = m_oUsedParticles.Begin(); it != m_oUsedParticles.End(); it++ )
		iCount += it->m_pParticles->m_iNumRenderingParticles;

	return iCount;
}

// $deBlob: FUNCTION 0070d1f0
TINT TParticleSystem::GetNumParticles() const
{
	TINT iCount = 0;

	for ( auto it = m_oUsedParticles.Begin(); it != m_oUsedParticles.End(); it++ )
		iCount += it->m_pParticles->m_iNumParticles;

	return iCount;
}

// $deBlob: FUNCTION 0070d210
TINT TParticleSystem::GetNumUsedSystems() const
{
	return TINT( m_oUsedParticles.Count() );
}

// $deBlob: FUNCTION 0070d220
TINT TParticleSystem::GetNumUsedGenericSystems() const
{
	return m_aiNumSystems[ PARTICLES_GENERIC ] - TINT( m_aFreeParticles[ PARTICLES_GENERIC ].Count() );
}

// $deBlob: FUNCTION 0070d230
TBOOL TParticleSystem::ReleaseMeshes()
{
	DestroyMeshes();
	return TTRUE;
}

// $deBlob: FUNCTION 0070d240
TBOOL TParticleSystem::RecreateMeshes()
{
	CreateMeshes();
	return TTRUE;
}

// $deBlob: FUNCTION 0070d250
TParticleMesh* TParticleSystem::GetMesh( TUINT a_uiNumVerts, TUINT a_uiMinVerts, TParticleMaterial* a_pMaterial )
{
	if ( a_pMaterial == TNULL )
		return TNULL;

	TINT a_iMatID = TINT( a_pMaterial->m_uiVer );
	TASSERT( a_iMatID >= 0 );
	TASSERT( a_iMatID < m_pMaterialManager->GetMaterialCount() );

	TMaterial* pMaterial = a_pMaterial->m_pMaterial;
	TASSERT( ( pMaterial == TNULL ) || pMaterial->IsA( &TGetClass( TSysMaterial ) ) );

	TParticleMesh* pLockedMesh = &m_pLockedMeshes[ a_iMatID ];

	if ( pLockedMesh->m_pMesh != TNULL )
	{
		if ( a_uiMinVerts <= pLockedMesh->GetMaxVertices() )
			return pLockedMesh;

		pLockedMesh->ForceUnlock();
	}

	if ( m_pLockedMeshes[ a_iMatID ].m_pMesh != TNULL || m_oFreeMeshes.Count() < 1 )
		return TNULL;

	// A free mesh big enough for all the vertices
	for ( auto it = m_oFreeMeshes.Begin(); it != m_oFreeMeshes.End(); it++ )
	{
		if ( a_uiNumVerts <= it->m_pMesh->GetMaxVertices() )
		{
			AMeshNode* pMeshNode = it;
			m_oFreeMeshes.Remove( pMeshNode );
			m_oRenderMeshes.InsertHead( pMeshNode );

			TASSERT( TNULL != pMeshNode->m_pMesh );
			pMeshNode->m_oModelView = TRender::GetSingleton()->GetCurrentContext()->GetModelViewMatrix();
			m_pLockedMeshes[ a_iMatID ].Lock( pMeshNode->m_pMesh, pMaterial );

			TASSERT( m_pLockedMeshes[ a_iMatID ].GetMaxVertices() >= a_uiMinVerts && "m_apLockedMeshes[a_iMatID].GetMaxVertices() >= a_uiMinVerts" );
			return &m_pLockedMeshes[ a_iMatID ];
		}
	}

	// Else the last free one
	AMeshNode* pMeshNode = m_oFreeMeshes.IsEmpty() ? TNULL : m_oFreeMeshes.Remove( m_oFreeMeshes.Tail() );
	m_oRenderMeshes.InsertHead( pMeshNode );

	TASSERT( TNULL != pMeshNode->m_pMesh );
	pMeshNode->m_oModelView = TRender::GetSingleton()->GetCurrentContext()->GetModelViewMatrix();
	m_pLockedMeshes[ a_iMatID ].Lock( pMeshNode->m_pMesh, pMaterial );

	TASSERT( m_pLockedMeshes[ a_iMatID ].GetMaxVertices() >= a_uiMinVerts && "m_apLockedMeshes[a_iMatID].GetMaxVertices() >= a_uiMinVerts" );
	return &m_pLockedMeshes[ a_iMatID ];
}

// $deBlob: FUNCTION 0070d660
void TParticleSystem::Create( const ACreateData& a_rCreateData )
{
	TParticleFloatBuffer::CreateSharedBuffers( a_rCreateData.m_iMaxFloats / 32 );
	TParticleV4Buffer::CreateSharedBuffers( a_rCreateData.m_iMaxVec4s / 32 );
	TParticleUCharBuffer::CreateSharedBuffers( a_rCreateData.m_iMaxUChars / 32 );
	TParticleUIntBuffer::CreateSharedBuffers( a_rCreateData.m_iMaxUInts / 32 );
	TParticleQuatBuffer::CreateSharedBuffers( a_rCreateData.m_iMaxQuaternions / 32 );

	SetDetail( 2, &a_rCreateData );
	CreateMeshes();

	m_pEffectManager = new TParticleEffectManager( a_rCreateData.m_iMaxEffectInstances );
	m_pEffectManager->Create();
}

// $deBlob: FUNCTION 0070d710
void TParticleSystem::GetParticleOffsets()
{
	const TMatrix44& rViewModel = TRender::GetSingleton()->GetCurrentContext()->GetViewModelMatrix();

	m_vRight.Multiply( rViewModel.AsBasisVector4( BASISVECTOR_RIGHT ), 0.5f );
	m_vUp.Multiply( rViewModel.AsBasisVector4( BASISVECTOR_UP ), 0.5f );
	m_vForward.Multiply( rViewModel.AsBasisVector4( BASISVECTOR_FORWARD ), 0.5f );

	m_aOffsets[ 2 ].x = m_vUp.x - m_vRight.x;
	m_aOffsets[ 2 ].y = m_vUp.y - m_vRight.y;
	m_aOffsets[ 2 ].z = m_vUp.z - m_vRight.z;
	m_aOffsets[ 2 ].w = m_vUp.w;

	m_aOffsets[ 3 ].x = m_vUp.x + m_vRight.x;
	m_aOffsets[ 3 ].y = m_vRight.y + m_vUp.y;
	m_aOffsets[ 3 ].z = m_vRight.z + m_vUp.z;
	m_aOffsets[ 3 ].w = m_vUp.w;

	m_aOffsets[ 0 ].x = ( TVector4::VEC_ZERO.x - m_vUp.x ) - m_vRight.x;
	m_aOffsets[ 0 ].y = ( TVector4::VEC_ZERO.y - m_vUp.y ) - m_vRight.y;
	m_aOffsets[ 0 ].z = ( TVector4::VEC_ZERO.z - m_vUp.z ) - m_vRight.z;
	m_aOffsets[ 0 ].w = TVector4::VEC_ZERO.w;

	m_aOffsets[ 1 ].x = m_vRight.x - m_vUp.x;
	m_aOffsets[ 1 ].y = m_vRight.y - m_vUp.y;
	m_aOffsets[ 1 ].z = m_vRight.z - m_vUp.z;
	m_aOffsets[ 1 ].w = m_vRight.w;
}

// $deBlob: FUNCTION 0070d8b0
void TParticleSystem::CreateMeshes()
{
	TSysShader* pSysShader = TSysShader::GetSingleton();
	TASSERT( m_oUsedMeshes.Count() == 0 );

	TINT iNumMeshes = m_oFreeMeshes.Count();

	if ( m_iMaxNumMeshes < iNumMeshes )
	{
		TASSERT( m_iMaxNumMeshes >= 0 );

		while ( m_iMaxNumMeshes < TINT( m_oFreeMeshes.Count() ) )
		{
			if ( m_oFreeMeshes.IsEmpty() )
				continue;

			AMeshNode* pMeshNode = m_oFreeMeshes.RemoveHead();
			delete pMeshNode;
		}
	}
	else
	{
		for ( ; iNumMeshes < m_iMaxNumMeshes; iNumMeshes++ )
		{
			AMeshNode* pMeshNode = new AMeshNode;
			pMeshNode->m_pMesh   = TNULL;
			pMeshNode->m_pMesh   = pSysShader->CreateMesh( TNULL );

			if ( pMeshNode->m_pMesh == TNULL || pMeshNode->m_pMesh->Create( 0xE4, 0x200, 0x300 ) != TTRUE )
			{
				if ( pMeshNode->m_pMesh != TNULL )
				{
					pMeshNode->m_pMesh->DestroyResource();
					pMeshNode->m_pMesh = TNULL;
				}

				delete pMeshNode;
				break;
			}

			m_oFreeMeshes.InsertHead( pMeshNode );
		}
	}

	if ( m_pAllocedParticleNodes != TNULL )
		return;

	// The pools of particles and their nodes
	TINT iTotalNumParticleNodes = 0;

	for ( TINT i = 0; i < PARTICLES_NUMOF; i++ )
	{
		TASSERT( m_aiNumSystems[ i ] >= 0 );
		iTotalNumParticleNodes += m_aiNumSystems[ i ];
	}

	TASSERT( m_pAllocedParticleNodes == TNULL );
	m_pAllocedParticleNodes = new TParticlesNode[ iTotalNumParticleNodes ];

	TINT iCurParticleNode = 0;

	for ( TINT iType = 0; iType < PARTICLES_NUMOF; iType++ )
	{
		TINT iNumSystems = m_aiNumSystems[ iType ];

		if ( iNumSystems <= 0 )
			continue;

		if ( iType == PARTICLES_TEST )
			m_pTestParticles = new TTestParticles[ iNumSystems ];
		else if ( iType == PARTICLES_GENERIC )
			m_pGenericParticles = new TGenericParticles[ iNumSystems ];
		else
			TASSERT( 0 );

		for ( TINT i = 0; i < m_aiNumSystems[ iType ]; i++ )
		{
			TASSERT( iCurParticleNode < iTotalNumParticleNodes );

			TParticlesNode* pNode = &m_pAllocedParticleNodes[ iCurParticleNode++ ];
			pNode->m_eType        = PARTICLES( iType );

			if ( iType == PARTICLES_TEST )
				pNode->m_pParticles = &m_pTestParticles[ i ];
			else if ( iType == PARTICLES_GENERIC )
				pNode->m_pParticles = &m_pGenericParticles[ i ];
			else
				TASSERT( 0 );

			pNode->m_pParticles->Deinitialise();
			m_aFreeParticles[ iType ].InsertHead( pNode );

			TASSERT( TFALSE == TParticleFloatBuffer::IsAnyBuffersUsed() && "TFALSE == AParticleFloatBuf::IsAnyBuffersUsed()" );
			TASSERT( TFALSE == TParticleV4Buffer::IsAnyBuffersUsed() && "TFALSE == AParticleVector4Buf::IsAnyBuffersUsed()" );
			TASSERT( TFALSE == TParticleUCharBuffer::IsAnyBuffersUsed() && "TFALSE == AParticleTUINT8Buf::IsAnyBuffersUsed()" );
			TASSERT( TFALSE == TParticleUIntBuffer::IsAnyBuffersUsed() && "TFALSE == AParticleTUINT32Buf::IsAnyBuffersUsed()" );
			TASSERT( TFALSE == TParticleQuatBuffer::IsAnyBuffersUsed() && "TFALSE == AParticleQuaternionBuf::IsAnyBuffersUsed()" );
		}
	}
}

// $deBlob: FUNCTION 0070dfd0
void TParticleSystem::DestroyMeshes()
{
	// The particles in use are stopped
	for ( auto it = m_oUsedParticles.Begin(); it != m_oUsedParticles.End(); )
	{
		TParticlesNode* pNode = it++;
		m_oUsedParticles.Remove( pNode );
		pNode->m_pParticles->Deinitialise();

		TASSERT( pNode->m_pParticles->m_iNumParticles == 0 && "pNode->m_pParticles->GetNumParticles() == 0" );
		TASSERT( pNode->m_pParticles->m_iNumRenderingParticles == 0 && "pNode->m_pParticles->GetNumRenderingParticles() == 0" );

		m_aFreeParticles[ pNode->m_eType ].InsertHead( pNode );
	}

	if ( m_pAllocedParticleNodes == TNULL )
		return;

	for ( TINT iType = 0; iType < PARTICLES_NUMOF; iType++ )
	{
		m_aFreeParticles[ iType ].RemoveAll();

		if ( iType == PARTICLES_TEST )
		{
			if ( m_pTestParticles != TNULL )
				delete[] m_pTestParticles;

			m_pTestParticles = TNULL;
		}
		else if ( iType == PARTICLES_GENERIC )
		{
			if ( m_pGenericParticles != TNULL )
				delete[] m_pGenericParticles;

			m_pGenericParticles = TNULL;
		}
		else
		{
			TASSERT( 0 );
		}
	}

	delete[] m_pAllocedParticleNodes;
	m_pAllocedParticleNodes = TNULL;
}

// $deBlob: FUNCTION 0070e2b0
void TParticleSystem::ResizeLockedMeshes()
{
	TRenderDX11::Interface()->WaitForEndOfRender();
	operator delete[]( m_pLockedMeshes );
	m_pLockedMeshes = TNULL;

	TUINT uiNumMaterials = m_pMaterialManager->GetMaterialCount();

	if ( uiNumMaterials != 0 )
	{
		m_pLockedMeshes = TSTATICCAST( TParticleMesh, TMemalign( 16, uiNumMaterials * sizeof( TParticleMesh ) ) );

		for ( TUINT i = 0; i < uiNumMaterials; i++ )
			new ( &m_pLockedMeshes[ i ] ) TParticleMesh();
	}
}

// $deBlob: FUNCTION 0070bae0
// $deBlob: FUNCTION 0070edc0
TParticleSystem::AMeshNode::~AMeshNode()
{
	if ( m_pMesh != TNULL )
		m_pMesh->DestroyResource();
}

// $deBlob: FUNCTION 0070ba80
// $deBlob: FUNCTION 0070ee50
// $deBlob: FUNCTION 0070eca0
TParticleSystem::TParticlesNode::~TParticlesNode()
{
	// NOTE: every node destroys the particles' shared buffer pools (inlined in the node array's destruction in
	// DestroyMeshes); after the first one they're already empty
	TParticleFloatBuffer::DestroySharedBuffers();
	TParticleV4Buffer::DestroySharedBuffers();
	TParticleUCharBuffer::DestroySharedBuffers();
	TParticleUIntBuffer::DestroySharedBuffers();
	TParticleQuatBuffer::DestroySharedBuffers();
}

TOSHI_NAMESPACE_END
