#include "ToshiPCH.h"
#include "TSkinShader.h"
#include "Render/TRender.h"
#include "Render/TVertexFactoryResourceInterface.h"
#include "Render/TVertexPoolResourceInterface.h"
#include "Math/TMathInline.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: TMaterial bit 3 (FLAGS_GLOW in Barnyard) marks a material with a blend mode in de Blob's skin materials
static TCONSTEXPR_VAR TMaterial::Flags SKINMATERIAL_FLAG_BLENDMODE = BITFLAG( 3 );

// $deBlob: FUNCTION 0070b0e0
// $deBlob: FUNCTION 0070b0f0
// $deBlob: FUNCTION 0070b100
// $deBlob: FUNCTION 0070b110
// $deBlob: FUNCTION 0070b120
// $deBlob: FUNCTION 0053fdb0
// $deBlob: FUNCTION 008595d0
TDEFINE_CLASS_NORUNTIME( TSkinShader );

// $deBlob: FUNCTION 0070b200
// $deBlob: FUNCTION 0070b210
// $deBlob: FUNCTION 0070b220
// $deBlob: FUNCTION 0070b230
// $deBlob: FUNCTION 0070b240
// $deBlob: FUNCTION 0053fd30
// $deBlob: FUNCTION 008595b0
TDEFINE_CLASS_NORUNTIME( TSkinMaterial );

// $deBlob: FUNCTION 0070b2a0
// $deBlob: FUNCTION 0070b2b0
// $deBlob: FUNCTION 0070b2c0
// $deBlob: FUNCTION 0070b2d0
// $deBlob: FUNCTION 0070b2e0
// $deBlob: FUNCTION 0053fd70
// $deBlob: FUNCTION 008595c0
TDEFINE_CLASS_NORUNTIME( TSkinMesh );

// $deBlob: FUNCTION 0070b130
TSkinShader::TSkinShader()
{
	m_uiUnk0C = 0;
	m_uiUnk10 = 0x80;
	m_fUnk14  = 1.0f;
	m_uiUnk18 = 0;
}

// $deBlob: FUNCTION 0070b1a0
// $deBlob: FUNCTION 0070b8e0
TSkinShader::~TSkinShader()
{
}

// $deBlob: FUNCTION 0070b980
const TCHAR* TSkinShader::GetShaderPrefix()
{
	return "ss_";
}

// $deBlob: FUNCTION 0070b250
void TSkinMaterial::SetBlendMode( BLENDMODE a_eBlendMode, TINT a_iPass )
{
	if ( a_iPass == 0 )
	{
		m_Flags &= ~SKINMATERIAL_FLAG_BLENDMODE;
		m_eBlendMode = a_eBlendMode;

		if ( a_eBlendMode != 0 )
			m_Flags |= SKINMATERIAL_FLAG_BLENDMODE;
	}
}

// $deBlob: FUNCTION 0070b270
TSkinMaterial::TSkinMaterial()
{
	m_eBlendMode = 0;
	m_bUnk58     = TTRUE;
}

// $deBlob: FUNCTION 0070b290
// $deBlob: FUNCTION 0070b880
TSkinMaterial::~TSkinMaterial()
{
}

// $deBlob: FUNCTION 0070b920
void TSkinMaterial::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode, 0 );
	TMaterial::Create();
}

// $deBlob: FUNCTION 0070b2f0
void TSkinMesh::OnDestroy()
{
	DestroyPools();
	m_State &= ~State_Created;
}

// $deBlob: FUNCTION 0070b300
TBOOL TSkinMesh::Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT32 a_uiNumSubMeshes )
{
	TASSERT( TFALSE == IsCreated() );

	m_uiFlags       = a_uiFlags;
	m_uiMaxVertices = a_uiMaxVertices;
	TASSERT( a_uiNumSubMeshes <= TMath::TUINT16_MAX );
	m_uiNumSubMeshes = TUINT16( a_uiNumSubMeshes );

	if ( !CreatePools() )
		return TFALSE;

	m_State |= State_Created;
	return TTRUE;
}

// $deBlob: FUNCTION 0070b3b0
TBOOL TSkinMesh::Validate()
{
	if ( !IsValidated() )
	{
		m_pVertexPool->Validate();
		m_State |= State_Validated;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0070b3d0
void TSkinMesh::Invalidate()
{
	if ( IsValidated() )
		m_State &= ~State_Validated;
}

// $deBlob: FUNCTION 0070b3f0
TBOOL TSkinMesh::Lock( void* a_pLockBuffer )
{
	TASSERT( !( m_uiFlags & FLAG_LOCKED ) );

	if ( TSTATICCAST( TVertexPoolResourceInterface, m_pVertexPool )->Lock( TSTATICCAST( TVertexPoolResourceInterface::LockBuffer, a_pLockBuffer ) ) )
	{
		m_uiFlags |= FLAG_LOCKED;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0070b460
void TSkinMesh::Unlock( TUINT32 a_uiNumVertices )
{
	TASSERT( m_uiFlags & FLAG_LOCKED );

	TVertexPoolResourceInterface* pVertexPool = TSTATICCAST( TVertexPoolResourceInterface, m_pVertexPool );

	if ( TINT16( a_uiNumVertices ) == -1 )
	{
		a_uiNumVertices = pVertexPool->GetNumVertices();
		TASSERT( a_uiNumVertices <= TMath::TUINT16_MAX );
	}

	pVertexPool->Unlock( TUINT16( a_uiNumVertices ) );
	m_uiFlags &= ~FLAG_LOCKED;
}

// $deBlob: FUNCTION 0070b500
TResource* TSkinMesh::GetVertexPool()
{
	return m_pVertexPool;
}

// $deBlob: FUNCTION 0070b510
TSkinMesh::TSkinMesh()
{
	m_uiMaxVertices  = 0;
	m_uiNumSubMeshes = 0;
	m_pVertexPool    = TNULL;
	m_pSubMeshes     = TNULL;
	m_uiUnk20        = 0;
	m_pMaterial2     = TNULL;
	m_uiFlags        = 0;
}

// $deBlob: FUNCTION 0070b550
// $deBlob: FUNCTION 0070b8b0
TSkinMesh::~TSkinMesh()
{
}

// $deBlob: FUNCTION 0070b560
TBOOL TSkinMesh::CreatePools()
{
	TASSERT( 0 != m_uiMaxVertices );
	TASSERT( TNULL == m_pVertexPool );

	TVertexFactoryResourceInterface* pFactory = TRender::GetSingleton()->GetSystemResource<TVertexFactoryResourceInterface>( TRender::SYSRESOURCE_VFSKIN );
	m_pVertexPool                             = pFactory->CreatePoolResource( m_uiMaxVertices, 1 );
	m_pSubMeshes                              = new TSkinSubMesh[ m_uiNumSubMeshes ];

	return TTRUE;
}

// $deBlob: FUNCTION 0070b670
void TSkinMesh::DestroyPools()
{
	if ( m_pVertexPool != TNULL )
	{
		TRender::GetSingleton()->DestroyResource( m_pVertexPool );
		m_pVertexPool = TNULL;
	}

	if ( m_pSubMeshes != TNULL )
		delete[] m_pSubMeshes;

	m_pSubMeshes     = TNULL;
	m_uiFlags        = 0;
	m_uiMaxVertices  = 0;
}

// $deBlob: FUNCTION 0070b750
TSkinSubMesh::TSkinSubMesh()
{
	uiUnk0        = 0;
	uiNumVertices = 0;
	pIndexPool    = TNULL;
	uiNumBones    = 0;
}

// $deBlob: FUNCTION 0070b7c0
// $deBlob: FUNCTION 0070b7e0
TSkinSubMesh::~TSkinSubMesh()
{
	if ( pIndexPool != TNULL )
	{
		TRender::GetSingleton()->DestroyResource( pIndexPool );
		pIndexPool = TNULL;
	}
}

TOSHI_NAMESPACE_END
