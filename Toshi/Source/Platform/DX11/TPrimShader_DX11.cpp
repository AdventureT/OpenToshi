#include "ToshiPCH.h"
#include "TPrimShader_DX11.h"
#include "TRender_DX11.h"
#include "TRenderContext_DX11.h"

#include <DirectXMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TPrimShader* TPrimShader::s_pSingleton = TNULL;

// Shaders of the prim shader (compiled by Initialise)
static const TCHAR* s_szPrimShaderVS =
    " float4x4 matWVP : register(c0);\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t struct VS_IN\t\t\t\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t     float4 ObjP"
    "os   : POSITION;\t\t\t\t     float4 Color    : COLOR;\t\t\t\t\t     float2 UV\t\t  : TEXCOORD;\t\t\t\t };\t\t\t\t\t\t\t\t\t\t"
    "\t\t\t\t\t\t\t\t\t\t\t\t\t struct VS_OUT\t\t\t\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t     float4 ProjPos  : SV_POSITION;\t\t\t     float2 "
    "UV\t\t  : TEXCOORD0;\t\t\t\t     float4 Color    : COLOR;\t\t\t\t\t };\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t VS_OUT main( VS_I"
    "N In )\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t     VS_OUT Out;\t\t\t\t\t\t\t\t     Out.ProjPos = mul( matWVP, In.ObjPos );\t     "
    "Out.Color = In.Color;\t\t\t\t\t\t     Out.UV = In.UV;\t\t\t\t\t\t\t     return Out;\t\t\t\t\t\t\t\t }\t\t\t\t\t\t\t\t\t\t\t\t";

static const TCHAR* s_szPrimShaderImagePS =
    " uniform sampler2D texture0 : register(s0);\t struct VS_OUT\t\t\t\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t     float4 ProjPos"
    "  : SV_POSITION;\t\t\t     float2 UV\t\t  : TEXCOORD0;\t\t\t\t     float4 Color    : COLOR;\t\t\t\t\t };\t\t\t\t\t\t\t\t\t\t"
    "\t\t\t\t\t\t\t\t\t\t\t\t\t float4 main( VS_OUT In ) : SV_TARGET\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t\t  return tex2D(texture0, In.UV) "
    "* In.Color;\t }\t\t\t\t\t\t\t\t\t\t\t\t";

static const TCHAR* s_szPrimShaderColourPS =
    " struct VS_OUT\t\t\t\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t     float4 ProjPos  : SV_POSITION;\t\t\t     float2 UV\t\t  : TEXCO"
    "ORD0;\t\t\t\t     float4 Color    : COLOR;\t\t\t\t\t };\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t float4 main( VS_OUT In ) : SV_"
    "TARGET\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t\t  return In.Color;\t\t\t\t\t\t\t }\t\t\t\t\t\t\t\t\t\t\t\t";

//-----------------------------------------------------------------------------
// Source\Shaders\PrimShader\TPrimShader.cpp
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00706820
TPrimShader::TPrimShader()
{
	Initialise();

	TASSERT( TNULL == s_pSingleton );
	s_pSingleton = this;
}

// $deBlob: FUNCTION 00706870
// $deBlob: FUNCTION 005f01a0
TPrimShader::~TPrimShader()
{
	TASSERT( TNULL != s_pSingleton );
	s_pSingleton = TNULL;

	Destroy();
}

//-----------------------------------------------------------------------------
// Source\Shaders\PrimShader\TPrimShader_DX11.cpp
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00705f10
void TPrimShader::Initialise()
{
	m_iNumMeshes     = 1000;
	m_iNumUsedMeshes = 0;
	m_uiFrame        = 0;
	m_bLocked        = TFALSE;
	m_bIsRendering   = TFALSE;
	m_ePrimType      = PrimType_TriangleStrip;
	m_eBlendMode     = BlendMode_Opaque;
	m_pImage         = TNULL;

	TRenderDX11*  pRender = TRenderDX11::Interface();
	ID3D11Device* pDevice = pRender->GetDevice();

	ID3DBlob* pBlob = TRenderDX11::CompileShader( s_szPrimShaderVS, "main", "vs_4_0_level_9_3", TNULL );
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

	pBlob = TRenderDX11::CompileShader( s_szPrimShaderImagePS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pImagePixelShader );
	pBlob->Release();

	pBlob = TRenderDX11::CompileShader( s_szPrimShaderColourPS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pColourPixelShader );
	pBlob->Release();

	m_pMeshes = new Mesh[ m_iNumMeshes ];

	for ( TINT i = 0; i < m_iNumMeshes; i++ )
		m_pMeshes[ i ].uiNumVerts = 0;

	// Type 0: vertex buffer
	m_pVertexBuffer = pRender->CreateBuffer( TRenderDX11::BUFFERTYPE_VERTEX, ONE_VERTEX_BUFFER_SIZE * sizeof( Vertex ), TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
}

// $deBlob: FUNCTION 00706150
void TPrimShader::Destroy()
{
	if ( m_pVertexShader )
	{
		m_pVertexShader->Release();
		m_pVertexShader = TNULL;
	}

	if ( m_pImagePixelShader )
	{
		m_pImagePixelShader->Release();
		m_pImagePixelShader = TNULL;
	}

	if ( m_pColourPixelShader )
	{
		m_pColourPixelShader->Release();
		m_pColourPixelShader = TNULL;
	}

	if ( m_pInputLayout )
	{
		m_pInputLayout->Release();
		m_pInputLayout = TNULL;
	}

	if ( m_pVertexBuffer )
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = TNULL;
	}

	delete[] m_pMeshes;
}

// $deBlob: FUNCTION 007061d0
TBOOL TPrimShader::AddMesh()
{
	TASSERT( TFALSE == m_bLocked );

	if ( m_iNumUsedMeshes >= m_iNumMeshes )
		return TFALSE;

	m_bLocked       = TTRUE;
	m_iNumUsedVerts = 0;
	Mesh* pMesh     = &m_pMeshes[ m_iNumUsedMeshes++ ];

	D3D11_MAPPED_SUBRESOURCE oMapped;
	TRenderDX11::Interface()->GetDeviceContext()->Map( m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );

	m_pCurrentVertex     = TSTATICCAST( Vertex, oMapped.pData ) + m_iNumUsedVerts;
	pMesh->uiStartVertex = m_iNumUsedVerts;
	pMesh->uiNumVerts    = 0;

	return TTRUE;
}

// $deBlob: FUNCTION 00706280
void TPrimShader::Unlock()
{
	TASSERT( TTRUE == m_bLocked );
	m_bLocked = TFALSE;

	TASSERT( m_iNumUsedMeshes - 1 < m_iNumMeshes );
	Mesh* pMesh = &m_pMeshes[ m_iNumUsedMeshes - 1 ];

	TRenderDX11::Interface()->GetDeviceContext()->Unmap( m_pVertexBuffer, 0 );
	DrawPrim( m_ePrimType, pMesh->uiNumVerts, pMesh->uiStartVertex );
}

// $deBlob: FUNCTION 00706330
void TPrimShader::AddVert()
{
	if ( m_bLocked )
	{
		m_pCurrentVertex++;
		m_iNumUsedVerts++;

		TASSERT( m_iNumUsedMeshes - 1 < m_iNumMeshes );
		Mesh* pMesh = &m_pMeshes[ m_iNumUsedMeshes - 1 ];

		TASSERT( m_iNumUsedVerts < ONE_VERTEX_BUFFER_SIZE );
		pMesh->uiNumVerts++;
	}
}

// $deBlob: FUNCTION 007063d0
void TPrimShader::DrawPrim( PrimType a_ePrimType, UINT a_uiNumVerts, UINT a_uiStartVertex )
{
	D3D11_PRIMITIVE_TOPOLOGY eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	switch ( a_ePrimType )
	{
		case PrimType_TriangleStrip:
			eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			break;
		case PrimType_TriangleList:
			eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		case PrimType_LineStrip:
			eTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			TASSERT( a_uiNumVerts >= 2 );
			break;
		case PrimType_LineList:
			eTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			TASSERT( ( a_uiNumVerts & 1 ) == 0 );
			break;
		default:
			TASSERT( !"unknown prim type" );
			break;
	}

	TRenderDX11::Interface()->DrawNonIndexed( eTopology, m_pVertexBuffer, a_uiNumVerts, sizeof( Vertex ), a_uiStartVertex, 0 );
}

// $deBlob: FUNCTION 007064e0
void TPrimShader::SetBlendModeAndUnlock( BlendMode a_eBlendMode )
{
	if ( a_eBlendMode != m_eBlendMode )
	{
		m_eBlendMode = a_eBlendMode;

		if ( m_bLocked )
			Unlock();
	}
}

// $deBlob: FUNCTION 00706500
void TPrimShader::SetImageAndUnlock( TTexture* a_pTexture )
{
	if ( a_pTexture != m_pImage )
	{
		m_pImage = a_pTexture;

		if ( m_bLocked )
			Unlock();
	}
}

// $deBlob: FUNCTION 00706520
void TPrimShader::StartRendering( PrimType a_ePrimType )
{
	TASSERT( TRender::GetSingleton()->IsInScene() );
	TRenderDX11* pRender = TRenderDX11::Interface();

	// The meshes of an earlier frame aren't used anymore
	if ( m_uiFrame != pRender->GetNumDrawnFrames() )
	{
		m_uiFrame        = pRender->GetNumDrawnFrames();
		m_iNumUsedMeshes = 0;
	}

	m_ePrimType = a_ePrimType;
	TASSERT( m_bIsRendering == TFALSE );
	m_bIsRendering = TTRUE;

	// NOTE: retail goes on when there's no mesh left (the vertices are written to the old mapping)
	AddMesh();

	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();
	pDeviceContext->VSSetShader( m_pVertexShader, TNULL, 0 );

	if ( m_pImage )
	{
		m_pImage->Bind( 0 );
		pDeviceContext->PSSetShader( m_pImagePixelShader, TNULL, 0 );
	}
	else
	{
		pDeviceContext->PSSetShader( m_pColourPixelShader, TNULL, 0 );
	}

	pRender->SetAlphaUpdate( TFALSE );

	D3D11_DEPTH_WRITE_MASK eDepthWriteMask;

	if ( m_eBlendMode == BlendMode_Opaque )
	{
		pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO );
		eDepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	}
	else if ( m_eBlendMode == BlendMode_Additive )
	{
		pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE );
		eDepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	}
	else
	{
		pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
		eDepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	}

	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, eDepthWriteMask );

	// DirectXMath's XMMatrixMultiply, so the products are summed in its order
	TRenderContextDX11* pRenderContext = TSTATICCAST( TRenderContextDX11, pRender->GetCurrentContext() );
	TMatrix44           oProjection    = pRenderContext->GetProjectionMatrix();
	TMatrix44           oModelView     = pRenderContext->GetModelViewMatrix();

	TMatrix44 oWorldViewProjection;
	DirectX::XMStoreFloat4x4(
	    TREINTERPRETCAST( DirectX::XMFLOAT4X4*, &oWorldViewProjection ),
	    DirectX::XMMatrixMultiply(
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &oModelView ) ),
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &oProjection ) )
	    )
	);
	pRender->SetVec4InVSBuffer( 0, &oWorldViewProjection, 4 );

	pDeviceContext->IASetInputLayout( m_pInputLayout );

	// Cull mode: D3D11_CULL_NONE
	TUINT8& rRasterizerFlags = TREINTERPRETCAST( TUINT8*, &pRender->GetCurrentRasterizer().uiFlags )[ 0 ];
	rRasterizerFlags         = ( rRasterizerFlags & 0xF7 ) | ( D3D11_CULL_NONE << TRenderDX11::RASTERIZERFLAGS_CULLSHIFT );
}

// $deBlob: FUNCTION 00706750
void TPrimShader::StopRendering()
{
	TASSERT( TRender::GetSingleton()->IsInScene() );
	TASSERT( m_bIsRendering == TTRUE );

	if ( m_bLocked )
		Unlock();

	m_bIsRendering = TFALSE;

	TRenderDX11* pRender = TRenderDX11::Interface();
	pRender->SetAlphaUpdate( TTRUE );
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
}

// $deBlob: FUNCTION 00706800
void TPrimShader::Flush()
{
	if ( m_bLocked )
		Unlock();
}

TOSHI_NAMESPACE_END
