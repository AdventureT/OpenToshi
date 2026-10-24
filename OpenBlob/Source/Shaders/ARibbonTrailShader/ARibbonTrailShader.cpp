#include "pch.h"
#include "ARibbonTrailShader.h"

#include <Render/TRenderPacket.h>
#include <Platform/DX11/TRender_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <Toshi/TStringManager.h>

#include <DirectXMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ARibbonTrailShader* ARibbonTrailShader::ms_pSingleton;

// The shaders
static const TCHAR* s_szRibbonTrailVS =
    "struct VS_IN { float3 ObjPos : POSITION; float4 Color : COLOR; float2 UV : TEXCOORD; }; struct VS_OUT { float4 ProjPos "
    ": SV_POSITION; float4 Color : COLOR; float2 UV : TEXCOORD0; }; float4x4 matWVP : register(c0); float4x4 matWV : regis"
    "ter(c4); float3x3 matNorm : register(c8); float4x4 texMtx0 : register(c12); float4x4 texMtx1 : register(c16); float4x4"
    " texMtx2 : register(c20); float4x4 texMtx3 : register(c24); float4x4 texMtx6 : register(c28); float4x4 texMtx7 : regis"
    "ter(c32); float4x3 bones[30] : register(c40); float3x3 bonesNrm[30] : register(c130); VS_OUT main(VS_IN In) { VS_OUT O"
    "ut; Out.ProjPos = mul(matWVP, float4(In.ObjPos, 1.0)); Out.Color = In.Color; Out.UV = In.UV; return Out; }";

static const TCHAR* s_szRibbonTrailPS =
    "struct VS_OUT { float4 ProjPos : SV_POSITION; float4 Color : COLOR; float2 UV : TEXCOORD0; }; uniform sampler2D textu"
    "re0 : register(s0); uniform sampler2D texture1 : register(s1); uniform sampler2D texture2 : register(s2); uniform samp"
    "ler2D texture3 : register(s3); float4 alphaTest : register(c0); float4 kColor0 : register(c1); float4 kColor1 : regist"
    "er(c2); float4 kColor2 : register(c3); float4 kColor3 : register(c4); float4 tevReg0 : register(c5); float4 tevReg1 : "
    "register(c6); float4 tevReg2 : register(c7); float4 chanMatColor0 : register(c8); float4 chanMatColor1 : register(c9);"
    " float4 chanAmbColor0 : register(c10); float4 chanAmbColor1 : register(c11); float4 light0Pos : register(c12); float4 "
    "light0Color : register(c13); float4 light6Pos : register(c14); float4 light6Color : register(c15); float4 light7Pos : "
    "register(c16); float4 light7Color : register(c17); float4 main(VS_OUT In) : SV_TARGET { float4 result = In.Color * tex"
    "2D(texture0, In.UV); if (alphaTest.x > 0 && result.a <= alphaTest.y) discard; return result; }";

// The alpha test of the material: on, alpha > 1/255
static const TVector4 s_vAlphaTest = TVector4( 1.0f, 1.0f / 255.0f, 0.0f, 0.0f );

// The depth bias of the material
static TFLOAT s_fDepthBias = 2.0f;

//-----------------------------------------------------------------------------
// ARibbonTrailShader
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 005fece0
// $deBlob: FUNCTION 005fecf0
// $deBlob: FUNCTION 005fed90
// $deBlob: FUNCTION 005fee30
// $deBlob: FUNCTION 005fee40
// $deBlob: FUNCTION 0053cd90
// $deBlob: FUNCTION 008586e0
TDEFINE_CLASS( ARibbonTrailShader );

// $deBlob: FUNCTION 005fee50
ARibbonTrailShader::ARibbonTrailShader()
{
	m_bReady        = TFALSE;
	m_pVertexShader = TNULL;
	m_pPixelShader  = TNULL;
	m_pInputLayout  = TNULL;
	ms_pSingleton   = this;
}

// $deBlob: FUNCTION 005feed0
// $deBlob: FUNCTION 005ffee0
ARibbonTrailShader::~ARibbonTrailShader()
{
	ms_pSingleton = TNULL;

	if ( m_pVertexShader != TNULL )
	{
		m_pVertexShader->Release();
		m_pVertexShader = TNULL;
	}

	if ( m_pPixelShader != TNULL )
	{
		m_pPixelShader->Release();
		m_pPixelShader = TNULL;
	}

	if ( m_pInputLayout != TNULL )
	{
		m_pInputLayout->Release();
		m_pInputLayout = TNULL;
	}
}

// $deBlob: FUNCTION 005fef80
void ARibbonTrailShader::Render( TRenderPacket* a_pRenderPacket )
{
	ARibbonTrailMesh* pMesh        = TSTATICCAST( ARibbonTrailMesh, a_pRenderPacket->GetMesh() );
	TRenderDX11*      pRender      = TRenderDX11::Interface();
	TUINT16           uiNumIndices = pMesh->GetNumIndices();

	if ( uiNumIndices == 0 )
		return;

	TRenderContextDX11* pContext = TSTATICCAST( TRenderContextDX11, pRender->GetCurrentContext() );

	// World view projection = ModelView * Projection (row vectors, DirectXMath's XMMatrixMultiply)
	TMatrix44 oProjection = pContext->GetProjectionMatrix();
	TMatrix44 oModelView  = a_pRenderPacket->GetModelViewMatrix();
	TMatrix44 oWorldViewProjection;
	DirectX::XMStoreFloat4x4(
	    TREINTERPRETCAST( DirectX::XMFLOAT4X4*, &oWorldViewProjection ),
	    DirectX::XMMatrixMultiply(
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &oModelView ) ),
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &oProjection ) )
	    )
	);

	pRender->SetVec4InVSBuffer( 0, &oWorldViewProjection, 4 );
	pRender->SetAlphaUpdate( pMesh->IsAlphaUpdate() );

	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();
	pDeviceContext->IASetInputLayout( m_pInputLayout );
	pDeviceContext->VSSetShader( m_pVertexShader, TNULL, 0 );
	pDeviceContext->PSSetShader( m_pPixelShader, TNULL, 0 );

	pRender->DrawImmediately( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, uiNumIndices, pMesh->GetIndices(), DXGI_FORMAT_R16_UINT, pMesh->GetVertices(), sizeof( ARibbonTrailMesh::Vertex ), pMesh->GetNumVertices() );
}

// $deBlob: FUNCTION 005ff0e0
TBOOL ARibbonTrailShader::Create()
{
	TASSERT( TFALSE == IsCreated() );

	m_oOrderTable.Create( this, 5500, 0xFF );

	TRenderDX11*  pRender = TRenderDX11::Interface();
	ID3D11Device* pDevice = pRender->GetDevice();

	ID3DBlob* pVSBlob = TRenderDX11::CompileShader( s_szRibbonTrailVS, "main", "vs_4_0_level_9_3", TNULL );
	pRender->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexShader );

	D3D11_INPUT_ELEMENT_DESC aInputDescs[ 3 ];
	aInputDescs[ 0 ].SemanticName         = "POSITION";
	aInputDescs[ 0 ].SemanticIndex        = 0;
	aInputDescs[ 0 ].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
	aInputDescs[ 0 ].InputSlot            = 0;
	aInputDescs[ 0 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDescs[ 0 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDescs[ 0 ].InstanceDataStepRate = 0;
	aInputDescs[ 1 ].SemanticName         = "COLOR";
	aInputDescs[ 1 ].SemanticIndex        = 0;
	aInputDescs[ 1 ].Format               = DXGI_FORMAT_B8G8R8A8_UNORM;
	aInputDescs[ 1 ].InputSlot            = 0;
	aInputDescs[ 1 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDescs[ 1 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDescs[ 1 ].InstanceDataStepRate = 0;
	aInputDescs[ 2 ].SemanticName         = "TEXCOORD";
	aInputDescs[ 2 ].SemanticIndex        = 0;
	aInputDescs[ 2 ].Format               = DXGI_FORMAT_R32G32_FLOAT;
	aInputDescs[ 2 ].InputSlot            = 0;
	aInputDescs[ 2 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDescs[ 2 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDescs[ 2 ].InstanceDataStepRate = 0;

	pDevice->CreateInputLayout( aInputDescs, 3, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout );
	pVSBlob->Release();

	ID3DBlob* pPSBlob = TRenderDX11::CompileShader( s_szRibbonTrailPS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShader );
	pPSBlob->Release();

	m_bReady = TTRUE;
	return TShader::Create();
}

// $deBlob: FUNCTION 005ff2a0
void ARibbonTrailShader::StartFlush()
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	pRender->SetColorUpdate( TTRUE );
	pRender->SetDstAlpha( 0.0f );

	// Cull mode: D3D11_CULL_NONE
	TUINT8& rRasterizerFlags = TREINTERPRETCAST( TUINT8*, &pRender->GetCurrentRasterizer().uiFlags )[ 0 ];
	rRasterizerFlags         = ( rRasterizerFlags & 0xF3 ) | 0x04;

	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
}

// $deBlob: FUNCTION 005ff300
void ARibbonTrailShader::EndFlush()
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	pRender->SetColorUpdate( TTRUE );
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
}

//-----------------------------------------------------------------------------
// ARibbonTrailMaterial
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 005ff330
// $deBlob: FUNCTION 005ff340
// $deBlob: FUNCTION 005ff3b0
// $deBlob: FUNCTION 005ff410
// $deBlob: FUNCTION 005ff420
// $deBlob: FUNCTION 0053cc90
// $deBlob: FUNCTION 008586a0
TDEFINE_CLASS( ARibbonTrailMaterial );

// $deBlob: FUNCTION 005ff430
ARibbonTrailMaterial::ARibbonTrailMaterial()
{
	m_pOrderTable = TNULL;
	SetShader( ARibbonTrailShader::GetSingleton() );
}

// $deBlob: FUNCTION 005ff490
// $deBlob: FUNCTION 005ffe20
ARibbonTrailMaterial::~ARibbonTrailMaterial()
{
}

// $deBlob: FUNCTION 005ff4a0
void ARibbonTrailMaterial::PreRender()
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	if ( m_pTextures[ 0 ] != TNULL )
		m_pTextures[ 0 ]->Bind( 0 );
	else
		TASSERT( TFALSE );

	pRender->SetVec4InPSBuffer( 0, &s_vAlphaTest, 1 );
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
	pRender->SetDstAlpha( -1.0f );
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, ( m_Flags & 0x1000 ) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO );
	pRender->SetDepthBias( s_fDepthBias, 0.0f );
}

// $deBlob: FUNCTION 005ff580
void ARibbonTrailMaterial::PostRender()
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
	pRender->SetDstAlpha( 0.0f );
	pRender->SetDepthBias( 1.0f, 0.0f );
}

// $deBlob: FUNCTION 005ff5c0
void ARibbonTrailMaterial::SetOrderTable( TOrderTable* a_pOrderTable )
{
	if ( a_pOrderTable != m_pOrderTable )
	{
		if ( m_pOrderTable != TNULL )
			TOrderTable::DeregisterMaterial( m_pRegMaterial );

		if ( a_pOrderTable != TNULL )
			a_pOrderTable->RegisterMaterial( this );

		m_pOrderTable = a_pOrderTable;
	}
}

//-----------------------------------------------------------------------------
// ARibbonTrailMesh
//-----------------------------------------------------------------------------

// NOTE: abstract; retail's CreateObject/CreateObjectInPlace return TNULL without an assert
// $deBlob: FUNCTION 005ff600
// $deBlob: FUNCTION 005ff610
// $deBlob: FUNCTION 005ff620
// $deBlob: FUNCTION 005ff630
// $deBlob: FUNCTION 005ff640
// $deBlob: FUNCTION 0053ccd0
// $deBlob: FUNCTION 008586b0
TDEFINE_CLASS_NORUNTIME( ARibbonTrailMesh );

// $deBlob: FUNCTION 005ff650
ARibbonTrailMesh::ARibbonTrailMesh()
{
	m_uiNumVertices = 0;
	m_uiNumIndices  = 0;
	m_bAlphaUpdate  = TFALSE;
}

// $deBlob: FUNCTION 005ff680
TBOOL ARibbonTrailMesh::Render()
{
	if ( ARibbonTrailShader::GetSingleton()->IsReady() )
	{
		TRenderContext* pContext = TRenderDX11::Interface()->GetCurrentContext();
		TRenderPacket*  pPacket  = m_pMaterial->GetRegMaterial()->AddRenderPacket( this );

		pPacket->SetModelViewMatrix( pContext->GetModelViewMatrix() );
		pPacket->SetFade( TUINT8( TINT( pContext->GetAlphaBlend() * 255.0f ) ) );
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// ARibbonTrailMeshStatic
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 005ff6e0
// $deBlob: FUNCTION 005ff6f0
// $deBlob: FUNCTION 005ff790
// $deBlob: FUNCTION 005ff820
// $deBlob: FUNCTION 005ff830
// $deBlob: FUNCTION 0053cd50
// $deBlob: FUNCTION 008586d0
TDEFINE_CLASS( ARibbonTrailMeshStatic );

// $deBlob: FUNCTION 005ff840
ARibbonTrailMeshStatic::ARibbonTrailMeshStatic()
{
	m_pIndices  = TNULL;
	m_pVertices = TNULL;
}

// $deBlob: FUNCTION 005ff880
TBOOL ARibbonTrailMeshStatic::Create( TUINT16 a_uiNumVertices, TUINT16 a_uiNumIndices, const TVector3* a_pPositions, const TUINT32* a_pColour, const TVector2* a_pUVs, const TUINT16* a_pIndices )
{
	m_uiNumVertices = a_uiNumVertices;
	m_uiNumIndices  = a_uiNumIndices;
	m_pVertices     = new Vertex[ a_uiNumVertices ];

	for ( TINT i = 0; i < m_uiNumVertices; i++ )
	{
		m_pVertices[ i ].vPosition = a_pPositions[ i ];
		m_pVertices[ i ].vUV       = a_pUVs[ i ];
	}

	m_uiColour = 0xFFFFFFFF;

	if ( *a_pColour != 0xFFFFFFFF )
	{
		m_uiColour = *a_pColour;

		for ( TINT i = 0; i < m_uiNumVertices; i++ )
			m_pVertices[ i ].uiColour = m_uiColour;
	}

	m_pIndices = new TUINT16[ m_uiNumIndices ];

	for ( TINT i = 0; i < m_uiNumIndices; i++ )
		m_pIndices[ i ] = a_pIndices[ i * 3 ];

	return TTRUE;
}

// $deBlob: FUNCTION 005ff9c0
void ARibbonTrailMeshStatic::SetColour( TUINT32 a_uiColour )
{
	if ( m_uiColour != a_uiColour )
	{
		m_uiColour = a_uiColour;

		for ( TINT i = 0; i < m_uiNumVertices; i++ )
			m_pVertices[ i ].uiColour = m_uiColour;
	}
}

// NOTE: doesn't call TMesh::OnDestroy, it clears State_Created itself
// $deBlob: FUNCTION 005ffa00
void ARibbonTrailMeshStatic::OnDestroy()
{
	delete[] m_pIndices;
	m_pIndices = TNULL;
	delete[] m_pVertices;
	m_State &= ~State_Created;
	m_pVertices = TNULL;
}

//-----------------------------------------------------------------------------
// ARibbonTrailMeshDynamic
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 005ffa30
// $deBlob: FUNCTION 005ffa40
// $deBlob: FUNCTION 005ffaf0
// $deBlob: FUNCTION 005ffb90
// $deBlob: FUNCTION 005ffba0
// $deBlob: FUNCTION 0053cd10
// $deBlob: FUNCTION 008586c0
TDEFINE_CLASS( ARibbonTrailMeshDynamic );

// $deBlob: FUNCTION 005ffbb0
ARibbonTrailMeshDynamic::ARibbonTrailMeshDynamic()
{
	m_pIndexData        = TNULL;
	m_pVertexData       = TNULL;
	m_pWriteIndices     = TNULL;
	m_pWriteVertices    = TNULL;
	m_pIndices          = TNULL;
	m_pVertices         = TNULL;
	m_uiMaxVerts        = 0;
	m_uiMaxIndices      = 0;
	m_uiCurVert         = 0;
	m_uiCurIndex        = 0;
	m_uiNumWriteVerts   = 0;
	m_uiNumWriteIndices = 0;
}

// $deBlob: FUNCTION 005ffc00
TBOOL ARibbonTrailMeshDynamic::Create( TUINT16 a_uiMaxSegments )
{
	TASSERT( !IsCreated() );

	m_uiNumVertices = 0;
	m_uiNumIndices  = 0;
	m_uiMaxVerts    = TUINT16( 2 * a_uiMaxSegments );
	m_uiMaxIndices  = TUINT16( 2 * a_uiMaxSegments );

	// Two index buffers (drawn, written) and three vertex buffers (two drawn alternately, written)
	m_pIndexData  = new TUINT16[ 2 * m_uiMaxIndices ];
	m_pVertexData = new Vertex[ 3 * m_uiMaxVerts ];

	m_pIndices       = m_pIndexData;
	m_pVertices      = m_pVertexData;
	m_pWriteVertices = m_pVertexData + 2 * m_uiMaxVerts;
	m_pWriteIndices  = m_pIndexData + m_uiMaxIndices;

	m_State |= State_Created;

	m_uiCurVert         = 0;
	m_uiCurIndex        = 0;
	m_uiNumWriteVerts   = 0;
	m_uiNumWriteIndices = 0;

	return TTRUE;
}

// NOTE: doesn't call TMesh::OnDestroy, it clears State_Created itself
// $deBlob: FUNCTION 005ffcf0
void ARibbonTrailMeshDynamic::OnDestroy()
{
	delete[] m_pIndexData;
	m_pIndexData = TNULL;
	delete[] m_pVertexData;
	m_State &= ~State_Created;
	m_pVertexData    = TNULL;
	m_pIndices       = TNULL;
	m_pVertices      = TNULL;
	m_pWriteIndices  = TNULL;
	m_pWriteVertices = TNULL;
}

// $deBlob: FUNCTION 005ffd40
void ARibbonTrailMeshDynamic::Swap()
{
	if ( m_pVertices == m_pVertexData )
		m_pVertices = m_pVertexData + m_uiMaxVerts;
	else
		m_pVertices = m_pVertexData;

	// The last m_uiNumWriteIndices indices before m_uiCurIndex, the ring buffer may wrap
	TINT iFirst = m_uiCurIndex - m_uiNumWriteIndices;

	if ( iFirst >= 0 )
	{
		TUtil::MemCopy( m_pIndices, m_pWriteIndices + iFirst, m_uiNumWriteIndices * sizeof( TUINT16 ) );
	}
	else
	{
		TINT  iWrapped   = m_uiMaxIndices + iFirst;
		TSIZE uiTailSize = ( m_uiMaxIndices - iWrapped ) * sizeof( TUINT16 );
		TUtil::MemCopy( m_pIndices, m_pWriteIndices + iWrapped, uiTailSize );
		TUtil::MemCopy( TREINTERPRETCAST( TUINT8*, m_pIndices ) + uiTailSize, m_pWriteIndices, m_uiCurIndex * sizeof( TUINT16 ) );
	}

	TUtil::MemCopy( m_pVertices, m_pWriteVertices, m_uiMaxVerts * sizeof( Vertex ) );

	m_uiNumVertices = m_uiMaxVerts;
	m_uiNumIndices  = m_uiNumWriteIndices;
}
