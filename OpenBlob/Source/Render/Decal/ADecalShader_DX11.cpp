#include "pch.h"
#include "ADecalShader_DX11.h"

#include <Render/TRender.h>
#include <Platform/DX11/TRender_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>

#include <DirectXMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The vertex shader
static const TCHAR* s_szDecalShaderVS =
    "struct VS_IN { float3 ObjPos : POSITION; float4 Color : COLOR; float2 UV : TEXCOORD; }; struct VS_OUT { float4 "
    "ProjPos : SV_POSITION; float4 Color : COLOR; float2 UV : TEXCOORD0; }; float4x4 matWVP : register(c0); float4x4 "
    "matWV : register(c4); float3x3 matNorm : register(c8); float4x4 texMtx0 : register(c12); float4x4 texMtx1 : "
    "register(c16); float4x4 texMtx2 : register(c20); float4x4 texMtx3 : register(c24); float4x4 texMtx6 : "
    "register(c28); float4x4 texMtx7 : register(c32); float4x3 bones[30] : register(c40); float3x3 bonesNrm[30] : "
    "register(c130); VS_OUT main(VS_IN In) { VS_OUT Out; Out.ProjPos = mul(matWVP, float4(In.ObjPos, 1.0)); Out.Color "
    "= In.Color; Out.UV = In.UV; return Out; }";

// The pixel shader
static const TCHAR* s_szDecalShaderPS =
    "struct VS_OUT { float4 ProjPos : SV_POSITION; float4 Color : COLOR; float2 UV : TEXCOORD0; }; uniform sampler2D "
    "texture0 : register(s0); uniform sampler2D texture1 : register(s1); uniform sampler2D texture2 : register(s2); "
    "uniform sampler2D texture3 : register(s3); float4 alphaTest : register(c0); float4 kColor0 : register(c1); float4 "
    "kColor1 : register(c2); float4 kColor2 : register(c3); float4 kColor3 : register(c4); float4 tevReg0 : "
    "register(c5); float4 tevReg1 : register(c6); float4 tevReg2 : register(c7); float4 chanMatColor0 : register(c8); "
    "float4 chanMatColor1 : register(c9); float4 chanAmbColor0 : register(c10); float4 chanAmbColor1 : register(c11); "
    "float4 light0Pos : register(c12); float4 light0Color : register(c13); float4 light6Pos : register(c14); float4 "
    "light6Color : register(c15); float4 light7Pos : register(c16); float4 light7Color : register(c17); float4 "
    "main(VS_OUT In) : SV_TARGET { float4 result = In.Color * tex2D(texture0, In.UV); if (alphaTest.x > 0 && "
    "result.a <= alphaTest.y) discard; return result; }";

// The alpha test reference
static TUINT8 s_uiAlphaRef = 200;

// $deBlob: FUNCTION 00553570
ADecalShader::ADecalShader( TINT a_iNumVertices )
{
	m_pTexture      = TNULL;
	m_iNumVertices  = a_iNumVertices;
	m_pVertexShader = TNULL;
	m_pPixelShader  = TNULL;
	m_pInputLayout  = TNULL;
	Create( a_iNumVertices );
}

// $deBlob: FUNCTION 00553620
// $deBlob: FUNCTION 005f6f10
ADecalShader::~ADecalShader()
{
	m_pTexture = TNULL;

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

// $deBlob: FUNCTION 00553770
void ADecalShader::Create( TINT a_iNumVertices )
{
	m_iNumVertices = a_iNumVertices;
	m_pTexture     = TTextureManager::GetSingleton()->GetInvalidTexture();
	TASSERT( m_iNumVertices % INT_NUM_VERTS_PER_PRIMITIVE == 0 );

	TRenderDX11*  pRender = TRenderDX11::Interface();
	ID3D11Device* pDevice = pRender->GetDevice();

	ID3DBlob* pBlob = TRenderDX11::CompileShader( s_szDecalShaderVS, "main", "vs_4_0_level_9_3", TNULL );
	pRender->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pVertexShader );

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

	pDevice->CreateInputLayout( aInputDescs, 3, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pInputLayout );
	pBlob->Release();

	pBlob = TRenderDX11::CompileShader( s_szDecalShaderPS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pPixelShader );
	pBlob->Release();
}

// $deBlob: FUNCTION 00553930
ADecalShader::Mesh* ADecalShader::CreateMesh( TINT a_iNumVerts, TUINT32 a_uiUnk )
{
	TASSERT( a_iNumVerts % 3 == 0 );

	Mesh* pMesh                 = new Mesh;
	pMesh->m_uiUnk10            = a_uiUnk;
	pMesh->m_iNumVertices       = a_iNumVerts;
	pMesh->m_pVertexBuffer      = TRenderDX11::Interface()->CreateBuffer( TRenderDX11::BUFFERTYPE_VERTEX, a_iNumVerts * sizeof( Vertex ), TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
	pMesh->m_uiVertexBufferSize = a_iNumVerts;

	ID3D11DeviceContext*     pDeviceContext = TRenderDX11::Interface()->GetDeviceContext();
	D3D11_MAPPED_SUBRESOURCE oMapped;
	pDeviceContext->Map( pMesh->m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
	memset( oMapped.pData, 0, pMesh->m_uiVertexBufferSize * sizeof( Vertex ) );
	pDeviceContext->Unmap( pMesh->m_pVertexBuffer, 0 );

	pMesh->m_uiCurVertex = 0;
	pMesh->m_uiFlags     = 0;
	return pMesh;
}

// $deBlob: FUNCTION 00553d60
void ADecalShader::DestroyMesh( Mesh* a_pMesh )
{
	if ( a_pMesh != TNULL )
	{
		if ( a_pMesh->m_pVertexBuffer != TNULL )
		{
			a_pMesh->m_pVertexBuffer->Release();
			a_pMesh->m_pVertexBuffer = TNULL;
		}

		delete a_pMesh;
	}
}

// $deBlob: FUNCTION 00553d90
void ADecalShader::ValidateMesh( Mesh* a_pMesh )
{
	a_pMesh->m_uiFlags |= MeshFlags_Valid;
}

// $deBlob: FUNCTION 00553da0
void ADecalShader::ResetMesh( Mesh* a_pMesh )
{
	ID3D11DeviceContext*     pDeviceContext = TRenderDX11::Interface()->GetDeviceContext();
	D3D11_MAPPED_SUBRESOURCE oMapped;
	pDeviceContext->Map( a_pMesh->m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
	memset( oMapped.pData, 0, a_pMesh->m_uiVertexBufferSize * sizeof( Vertex ) );
	pDeviceContext->Unmap( a_pMesh->m_pVertexBuffer, 0 );

	a_pMesh->m_uiCurVertex = 0;
}

// $deBlob: FUNCTION 005536e0
ADecalShader::Vertex* ADecalShader::LockDiscard( Mesh* a_pMesh )
{
	D3D11_MAPPED_SUBRESOURCE oMapped;
	TRenderDX11::Interface()->GetDeviceContext()->Map( a_pMesh->m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
	return TSTATICCAST( Vertex, oMapped.pData );
}

// $deBlob: FUNCTION 00553710
ADecalShader::Vertex* ADecalShader::Lock( Mesh* a_pMesh )
{
	D3D11_MAPPED_SUBRESOURCE oMapped;
	TRenderDX11::Interface()->GetDeviceContext()->Map( a_pMesh->m_pVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &oMapped );
	return TSTATICCAST( Vertex, oMapped.pData ) + a_pMesh->m_uiCurVertex;
}

// $deBlob: FUNCTION 00553750
void ADecalShader::Unlock( Mesh* a_pMesh )
{
	TRenderDX11::Interface()->GetDeviceContext()->Unmap( a_pMesh->m_pVertexBuffer, 0 );
}

// $deBlob: FUNCTION 00553a20
void ADecalShader::PreRender()
{
	TRenderDX11*        pRender        = TRenderDX11::Interface();
	TRenderContextDX11* pRenderContext = TSTATICCAST( TRenderContextDX11, pRender->GetCurrentContext() );

	pRender->SetColorUpdate( TTRUE );
	pRender->SetDstAlpha( 0.0f );

	// Cull mode: D3D11_CULL_NONE
	TUINT8& rRasterizerFlags = TREINTERPRETCAST( TUINT8*, &pRender->GetCurrentRasterizer().uiFlags )[ 0 ];
	rRasterizerFlags         = ( rRasterizerFlags & 0xF7 ) | 0x04;

	pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
	m_pTexture->Bind( 0 );
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );

	// alphaTest: enabled, the reference
	TVector4 vAlphaTest( 1.0f, TFLOAT( s_uiAlphaRef ) / 255.0f, 0.0f, 0.0f );
	pRender->SetVec4InPSBuffer( 0, &vAlphaTest, 1 );

	// World view projection = ModelView * Projection (row vectors, DirectXMath's XMMatrixMultiply)
	TRenderContextDX11* pCurrentContext = TSTATICCAST( TRenderContextDX11, pRender->GetCurrentContext() );
	TMatrix44           oProjection     = pCurrentContext->GetProjectionMatrix();
	TMatrix44           oModelView      = pRenderContext->GetModelViewMatrix();

	TMatrix44 oWorldViewProjection;
	DirectX::XMStoreFloat4x4(
	    TREINTERPRETCAST( DirectX::XMFLOAT4X4*, &oWorldViewProjection ),
	    DirectX::XMMatrixMultiply(
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &oModelView ) ),
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &oProjection ) )
	    )
	);
	pRender->SetVec4InVSBuffer( 0, &oWorldViewProjection, 4 );
}

// $deBlob: FUNCTION 00553bb0
void ADecalShader::Render( Mesh* a_pMesh )
{
	TASSERT( IsValid( a_pMesh ) );

	TUINT uiCurBufPtr        = a_pMesh->m_uiCurVertex;
	TINT  iNumVertsRemaining = a_pMesh->m_uiVertexBufferSize - uiCurBufPtr;
	TINT  iNumVertsRendered  = 0;

	while ( TTRUE )
	{
		TASSERT( uiCurBufPtr < a_pMesh->m_uiVertexBufferSize );

		TRenderDX11*         pRender        = TRenderDX11::Interface();
		ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();
		pDeviceContext->IASetInputLayout( m_pInputLayout );
		pDeviceContext->VSSetShader( m_pVertexShader, TNULL, 0 );
		pDeviceContext->PSSetShader( m_pPixelShader, TNULL, 0 );

		TASSERT( iNumVertsRemaining % 3 == 0 );
		pRender->DrawNonIndexed( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, a_pMesh->m_pVertexBuffer, iNumVertsRemaining, sizeof( Vertex ), uiCurBufPtr, 0 );
		iNumVertsRendered += iNumVertsRemaining;

		if ( iNumVertsRendered == m_iNumVertices )
			break;

		TASSERT( iNumVertsRendered < m_iNumVertices );
		uiCurBufPtr += iNumVertsRemaining;
		TASSERT( uiCurBufPtr == a_pMesh->m_uiVertexBufferSize );

		iNumVertsRemaining = a_pMesh->m_uiCurVertex;
		uiCurBufPtr        = 0;
	}
}

// $deBlob: FUNCTION 00553ba0
void ADecalShader::PostRender()
{
	// DOES_NOTHING
}
