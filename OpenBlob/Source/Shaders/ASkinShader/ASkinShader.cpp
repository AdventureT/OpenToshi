#include "pch.h"
#include "ASkinShader.h"
#include "Render/ModelLoading/AModelLoader.h"

#include <Platform/DX11/TRender_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TVector4       ASkinShader::s_fogColor = { 0.0f, 0.0f, 0.0f, 1.0f };
Toshi::TVector4       ASkinShader::s_FogStartEnd;
TINT                  ASkinShader::s_NumTevStages = 1;
Toshi::TVector4       ASkinShader::s_AlphaTest;
Toshi::TVector4       ASkinShader::s_ChanAmbColor0 = { 0.0f, 0.0f, 0.0f, 1.0f };
Toshi::TVector4       ASkinShader::s_ChanMatColor0 = { 1.0f, 1.0f, 1.0f, 1.0f };
const Toshi::TVector4 ASkinShader::s_Light7Color   = { 0.0f, 1.0f, 0.0f, 1.0f };
const Toshi::TVector4 ASkinShader::s_Light7Pos     = { 0.0f, 0.0f, 0.0f, 1.0f };

// $deBlob: FUNCTION 00600a80
// $deBlob: FUNCTION 00600a90
// $deBlob: FUNCTION 00600aa0
// $deBlob: FUNCTION 00600ab0
// $deBlob: FUNCTION 00600ac0
// $deBlob: FUNCTION 0053ce50
// $deBlob: FUNCTION 00858710
TDEFINE_CLASS_NORUNTIME( ASkinShader );

// $deBlob: FUNCTION 00600ba0
// $deBlob: FUNCTION 00600bb0
// $deBlob: FUNCTION 00600bc0
// $deBlob: FUNCTION 00600bd0
// $deBlob: FUNCTION 00600be0
// $deBlob: FUNCTION 0053cdd0
// $deBlob: FUNCTION 008586f0
TDEFINE_CLASS_NORUNTIME( ASkinMaterial );

// $deBlob: FUNCTION 00600cd0
// $deBlob: FUNCTION 00600ce0
// $deBlob: FUNCTION 00600cf0
// $deBlob: FUNCTION 00600d00
// $deBlob: FUNCTION 00600d10
// $deBlob: FUNCTION 0053ce10
// $deBlob: FUNCTION 00858700
TDEFINE_CLASS_NORUNTIME( ASkinMesh );

// $deBlob: FUNCTION 00600ad0
ASkinShader::ASkinShader()
{
	m_uiUnk0C = 0;
	m_uiUnk10 = 0x80;
	m_fUnk14  = 1.0f;
	m_uiUnk18 = 0;
}

// $deBlob: FUNCTION 00600b40
// $deBlob: FUNCTION 00601300
ASkinShader::~ASkinShader()
{
}

// $deBlob: FUNCTION 00567540
const TCHAR* ASkinShader::GetShaderPrefix()
{
	return "ss_";
}

// $deBlob: FUNCTION 00600bf0
void ASkinMaterial::SetBlendMode( BLENDMODE a_eBlendMode, TINT a_iPass )
{
	if ( a_iPass == 0 )
	{
		m_Flags &= ~SKINFLAG_BLENDMODE;
		m_eBlendMode = a_eBlendMode;

		if ( a_eBlendMode != BLENDMODE_DEFAULT )
			m_Flags |= SKINFLAG_BLENDMODE;
	}

	if ( a_eBlendMode == BLENDMODE_JELLY || a_eBlendMode == BLENDMODE_STENCIL )
	{
		m_Flags |= SKINFLAG_BLENDMODE;
		m_iPass      = a_iPass;
		m_eBlendMode = a_eBlendMode;
	}
}

// $deBlob: FUNCTION 00600c30
ASkinMaterial::ASkinMaterial()
{
	m_eBlendMode = BLENDMODE_DEFAULT;
	m_bUnk58     = TTRUE;
	m_iPass      = 0;
}

// $deBlob: FUNCTION 00600c60
// $deBlob: FUNCTION 006012a0
ASkinMaterial::~ASkinMaterial()
{
}

// $deBlob: FUNCTION 00567200
void ASkinMaterial::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode, 0 );
	TMaterial::Create();
}

// $deBlob: FUNCTION 006011c0
ASkinSubMesh::ASkinSubMesh()
{
	uiNumIndices  = 0;
	uiNumVertices = 0;
	pIndexBuffer  = TNULL;
	uiNumBones    = 0;
}

// $deBlob: FUNCTION 00600c70
// $deBlob: FUNCTION 00601230
ASkinSubMesh::~ASkinSubMesh()
{
	if ( pIndexBuffer != TNULL )
	{
		pIndexBuffer->Release();
		pIndexBuffer = TNULL;
	}
}

// $deBlob: FUNCTION 00600c90
TBOOL ASkinSubMesh::CreateIndexBuffer( TUINT32 a_uiUnused, TINT a_iNumIndices, const void* a_pIndices )
{
	if ( pIndexBuffer == TNULL )
	{
		pIndexBuffer = TRenderDX11::Interface()->CreateBuffer( TRenderDX11::BUFFERTYPE_INDEX, a_iNumIndices * sizeof( TUINT16 ), a_pIndices, D3D11_USAGE_IMMUTABLE, 0 );
		uiNumIndices = TUINT16( a_iNumIndices );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00600d20
void ASkinMesh::OnDestroy()
{
	AModelLoader::DestroyMaterial( m_pMaterial );

	if ( m_pMaterial2 != TNULL )
		AModelLoader::DestroyMaterial( m_pMaterial2 );

	if ( m_pVertexBuffer != TNULL )
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = TNULL;
	}

	if ( m_pSubMeshes != TNULL )
		delete[] m_pSubMeshes;

	m_State &= ~State_Created;
	m_pSubMeshes    = TNULL;
	m_uiFlags       = 0;
	m_uiMaxVertices = 0;
}

// $deBlob: FUNCTION 00600d90
TBOOL ASkinMesh::Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT32 a_uiNumSubMeshes )
{
	TASSERT( TFALSE == IsCreated() );

	m_uiFlags       = a_uiFlags;
	m_uiMaxVertices = a_uiMaxVertices;
	TASSERT( a_uiNumSubMeshes <= TMath::TUINT16_MAX );
	m_uiNumSubMeshes = TUINT16( a_uiNumSubMeshes );

	TBOOL bCreated = CreatePools();

	if ( !bCreated )
		return bCreated;

	m_State |= State_Created;
	return TTRUE;
}

// $deBlob: FUNCTION 00600e40
TBOOL ASkinMesh::Validate()
{
	if ( !IsValidated() )
		m_State |= State_Validated;

	return TTRUE;
}

// $deBlob: FUNCTION 00600e60
void ASkinMesh::Invalidate()
{
	if ( IsValidated() )
		m_State &= ~State_Validated;
}

// $deBlob: FUNCTION 00601110
TBOOL ASkinMesh::DestroyPools()
{
	if ( m_pVertexBuffer != TNULL )
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = TNULL;
	}

	if ( m_pSubMeshes != TNULL )
		delete[] m_pSubMeshes;

	m_pSubMeshes    = TNULL;
	m_uiFlags       = 0;
	m_uiMaxVertices = 0;

	return TFALSE;
}

// $deBlob: FUNCTION 00600e80
TBOOL ASkinMesh::Lock( void* a_pLockBuffer )
{
	// NOTE: the DX11 skin meshes can't be locked (their vertices are uploaded by AModelLoaderSkin_Win)
	TASSERT( !( m_uiFlags & FLAG_LOCKED ) );
	TASSERT( TFALSE );

	return TFALSE;
}

// $deBlob: FUNCTION 00600ef0
void ASkinMesh::Unlock( TUINT32 a_uiNumVertices )
{
	TASSERT( m_uiFlags & FLAG_LOCKED );
	TASSERT( TFALSE );

	m_uiFlags &= ~FLAG_LOCKED;
}

// $deBlob: FUNCTION 00600f70
ID3D11Buffer* ASkinMesh::GetVertexPool()
{
	return m_pVertexBuffer;
}

// $deBlob: FUNCTION 00600f80
ASkinMesh::ASkinMesh()
{
	m_uiMaxVertices  = 0;
	m_uiNumSubMeshes = 0;
	m_pVertexBuffer  = TNULL;
	m_pSubMeshes     = TNULL;
	m_uiUnk20        = 0;
	m_pMaterial2     = TNULL;
	m_uiFlags        = 0;
}

// $deBlob: FUNCTION 00600fc0
// $deBlob: FUNCTION 006012d0
ASkinMesh::~ASkinMesh()
{
}

// $deBlob: FUNCTION 00600fd0
TBOOL ASkinMesh::CreatePools()
{
	TASSERT( 0 != m_uiMaxVertices );
	TASSERT( TNULL == m_pVertexBuffer );

	m_pVertexBuffer = TRenderDX11::Interface()->CreateBuffer( TRenderDX11::BUFFERTYPE_VERTEX, m_uiMaxVertices * VERTEX_SIZE, TNULL, D3D11_USAGE_DEFAULT, 0 );

	// NOTE: retail allocates the array with TMemalign16
	m_pSubMeshes = new ASkinSubMesh[ m_uiNumSubMeshes ];

	return TTRUE;
}
