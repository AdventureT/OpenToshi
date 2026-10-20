#include "pch.h"
#include "ALineShader_DX11.h"

#include <Platform/DX11/TRender_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The vertex shader
static const TCHAR* s_szLineVS =
    " float4x4 matWVP : register(c0);\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t struct VS_IN\t\t\t\t\t\t\t\t\t {\t"
    "\t\t\t\t\t\t\t\t\t\t\t     float3 ObjPos   : POSITION;\t\t\t\t     float4 Color    : COLOR;\t\t\t\t\t"
    " };\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t struct VS_OUT\t\t\t\t\t\t\t\t\t {\t\t\t\t\t\t\t\t"
    "\t\t\t\t     float4 ProjPos  : SV_POSITION;\t\t\t     float4 Color    : COLOR;\t\t\t\t\t };\t\t\t\t\t"
    "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t VS_OUT main( VS_IN In )\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t  "
    "   VS_OUT Out;\t\t\t\t\t\t\t\t     Out.ProjPos = mul( matWVP, float4(In.ObjPos, 1.0) );\t     Out.Co"
    "lor = In.Color;\t\t\t\t\t\t     return Out;\t\t\t\t\t\t\t\t }\t\t\t\t\t\t\t\t\t\t\t\t";

// The pixel shader
static const TCHAR* s_szLinePS =
    " struct VS_OUT\t\t\t\t\t\t\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t     float4 ProjPos  : SV_POSITION;\t\t\t "
    "    float4 Color    : COLOR;\t\t\t\t\t };\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t float4 main("
    " VS_OUT In ) : SV_TARGET\t\t\t {\t\t\t\t\t\t\t\t\t\t\t\t\t  return In.Color;\t\t\t\t\t\t\t }\t\t\t\t"
    "\t\t\t\t\t\t\t\t";

// Whether the triangles are depth tested (always on: nothing changes it)
static TBOOL s_bTriangleZTest = TTRUE;

//-----------------------------------------------------------------------------
// ALineShaderHAL
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0053b460
// $deBlob: FUNCTION 0055e6e0
// $deBlob: FUNCTION 0055e6f0
// $deBlob: FUNCTION 0055e790
// $deBlob: FUNCTION 0055e830
// $deBlob: FUNCTION 0055e840
// $deBlob: FUNCTION 00857ec0
TDEFINE_CLASS( ALineShaderHAL );

// $deBlob: FUNCTION 0055e850
ALineShaderHAL::ALineShaderHAL()
{
	m_pLineBuffer        = TNULL;
	m_pTriangleBuffer    = TNULL;
	m_uiNumLineVerts     = 0;
	m_uiNumTriangleVerts = 0;
	m_pVertexShader      = TNULL;
	m_pPixelShader       = TNULL;
	m_pInputLayout       = TNULL;
}

// $deBlob: FUNCTION 0055f380
ALineShaderHAL::~ALineShaderHAL()
{
}

// $deBlob: FUNCTION 0055e8a0
TBOOL ALineShaderHAL::Create()
{
	TASSERT( TFALSE == IsCreated() );

	TRenderDX11*  pRender = TRenderDX11::Interface();
	ID3D11Device* pDevice = pRender->GetDevice();

	ID3DBlob* pVSBlob = TRenderDX11::CompileShader( s_szLineVS, "main", "vs_4_0_level_9_3", TNULL );
	pRender->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexShader );

	D3D11_INPUT_ELEMENT_DESC aInputDescs[ 2 ];
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

	pDevice->CreateInputLayout( aInputDescs, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout );
	pVSBlob->Release();

	ID3DBlob* pPSBlob = TRenderDX11::CompileShader( s_szLinePS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShader );
	pPSBlob->Release();

	return ALineShader::Create();
}

// $deBlob: FUNCTION 0055ea00
void ALineShaderHAL::OnDestroy()
{
	if ( m_pLineBuffer != TNULL )
	{
		m_pLineBuffer->Release();
		m_pLineBuffer = TNULL;
	}

	if ( m_pTriangleBuffer != TNULL )
	{
		m_pTriangleBuffer->Release();
		m_pTriangleBuffer = TNULL;
	}

	if ( m_pInputLayout != TNULL )
	{
		m_pInputLayout->Release();
		m_pInputLayout = TNULL;
	}

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

	TASSERT( TTRUE == IsCreated() );
	TShader::OnDestroy();
}

// $deBlob: FUNCTION 0055eab0
TMaterial* ALineShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	return TNULL;
}

// $deBlob: FUNCTION 0055eac0
AMesh* ALineShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	return TNULL;
}

// $deBlob: FUNCTION 0055ead0
TBOOL ALineShaderHAL::Validate()
{
	if ( m_pLineBuffer == TNULL )
	{
		m_pLineBuffer = TRenderDX11::Interface()->CreateBuffer( TRenderDX11::BUFFERTYPE_VERTEX, MAX_VERTICES * sizeof( Vertex ), TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );

		if ( m_pLineBuffer == TNULL )
			return TFALSE;

		m_uiNumLineVerts = 0;
	}

	if ( m_pTriangleBuffer == TNULL )
	{
		m_pTriangleBuffer = TRenderDX11::Interface()->CreateBuffer( TRenderDX11::BUFFERTYPE_VERTEX, MAX_VERTICES * sizeof( Vertex ), TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );

		if ( m_pTriangleBuffer == TNULL )
			return TFALSE;

		m_uiNumTriangleVerts = 0;
	}

	return TShader::Validate();
}

// $deBlob: FUNCTION 0055eb40
void ALineShaderHAL::Invalidate()
{
	if ( m_pLineBuffer != TNULL )
	{
		m_pLineBuffer->Release();
		m_pLineBuffer = TNULL;
	}

	if ( m_pTriangleBuffer != TNULL )
	{
		m_pTriangleBuffer->Release();
		m_pTriangleBuffer = TNULL;
	}

	TShader::Invalidate();
}

// $deBlob: FUNCTION 0055eb80
void ALineShaderHAL::RenderBuffers()
{
	if ( !m_bEnabled )
		return;

	TRenderDX11* pRender = TRenderDX11::Interface();
	TASSERT( pRender->IsInScene() );

	if ( ( m_pLineBuffer == TNULL && m_pTriangleBuffer == TNULL ) || ( m_uiNumLineVerts == 0 && m_uiNumTriangleVerts == 0 ) )
		return;

	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();
	TRenderContextDX11*  pContext       = TSTATICCAST( TRenderContextDX11, pRender->GetCurrentContext() );

	TMatrix44 oProjection = pContext->GetProjectionMatrix();
	pRender->SetVec4InVSBuffer( 0, &oProjection, 4 );

	TUINT64 uiSavedDepth = pRender->GetCurrentDepth().uiDepth;
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );

	pDeviceContext->IASetInputLayout( m_pInputLayout );
	pDeviceContext->VSSetShader( m_pVertexShader, TNULL, 0 );
	pDeviceContext->PSSetShader( m_pPixelShader, TNULL, 0 );

	if ( m_pTriangleBuffer != TNULL && m_uiNumTriangleVerts != 0 )
	{
		if ( !s_bTriangleZTest )
			pRender->SetZMode( TTRUE, D3D11_COMPARISON_ALWAYS, D3D11_DEPTH_WRITE_MASK_ALL );

		pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );

		// Cull mode: D3D11_CULL_NONE while drawing the triangles
		TUINT8& rRasterizerFlags = TREINTERPRETCAST( TUINT8*, &pRender->GetCurrentRasterizer().uiFlags )[ 0 ];
		TUINT8  uiOldCullMode    = ( rRasterizerFlags >> 2 ) & 3;
		rRasterizerFlags         = ( rRasterizerFlags & 0xF7 ) | 0x04;

		pRender->DrawNonIndexed( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pTriangleBuffer, m_uiNumTriangleVerts, sizeof( Vertex ), 0, 0 );

		rRasterizerFlags ^= ( ( uiOldCullMode << 2 ) ^ rRasterizerFlags ) & 0x0C;
	}

	if ( m_pLineBuffer != TNULL && m_uiNumLineVerts != 0 )
		pRender->DrawNonIndexed( D3D11_PRIMITIVE_TOPOLOGY_LINELIST, m_pLineBuffer, m_uiNumLineVerts, sizeof( Vertex ), 0, 0 );

	pRender->GetCurrentDepth().uiDepth = uiSavedDepth;
}

// $deBlob: FUNCTION 0055ed70
void ALineShaderHAL::AddLineDirect( const TVector4& a_rFrom, const TVector4& a_rTo, TUINT32 a_uiFromColour, TUINT32 a_uiToColour )
{
	if ( !m_bEnabled )
		return;

	if ( m_uiNumLineVerts < MAX_VERTICES - 1 )
	{
		Validate();

		static Vertex s_aVertices[ 2 ];
		s_aVertices[ 0 ].uiColour = a_uiFromColour;
		s_aVertices[ 1 ].uiColour = a_uiToColour;
		s_aVertices[ 0 ].fX       = a_rFrom.x;
		s_aVertices[ 0 ].fY       = a_rFrom.y;
		s_aVertices[ 0 ].fZ       = a_rFrom.z;
		s_aVertices[ 1 ].fX       = a_rTo.x;
		s_aVertices[ 1 ].fY       = a_rTo.y;
		s_aVertices[ 1 ].fZ       = a_rTo.z;

		TRenderDX11::Interface()->UpdateBuffer( m_pLineBuffer, sizeof( s_aVertices ), s_aVertices, m_uiNumLineVerts * sizeof( Vertex ), D3D11_MAP_WRITE_NO_OVERWRITE );
		m_uiNumLineVerts += 2;
	}
	else
	{
		TASSERT( TFALSE );
	}
}

// $deBlob: FUNCTION 0055ee40
void ALineShaderHAL::AddLine( const TVector4& a_rFrom, const TVector4& a_rTo, TUINT32 a_uiFromColour, TUINT32 a_uiToColour )
{
	Validate();

	// NOTE: retail draws the full buffer but doesn't reset the count, so the vertices go past the end of the buffer
	if ( m_uiNumLineVerts >= MAX_VERTICES - 1 )
		RenderBuffers();

	static Vertex s_aVertices[ 2 ];
	s_aVertices[ 0 ].uiColour = a_uiFromColour;
	s_aVertices[ 1 ].uiColour = a_uiToColour;
	s_aVertices[ 0 ].fX       = a_rFrom.x;
	s_aVertices[ 0 ].fY       = a_rFrom.y;
	s_aVertices[ 0 ].fZ       = a_rFrom.z;
	s_aVertices[ 1 ].fX       = a_rTo.x;
	s_aVertices[ 1 ].fY       = a_rTo.y;
	s_aVertices[ 1 ].fZ       = a_rTo.z;

	TRenderDX11::Interface()->UpdateBuffer( m_pLineBuffer, sizeof( s_aVertices ), s_aVertices, m_uiNumLineVerts * sizeof( Vertex ), D3D11_MAP_WRITE_NO_OVERWRITE );
	m_uiNumLineVerts += 2;
}

// $deBlob: FUNCTION 0055eee0
void ALineShaderHAL::AddTriangleArray( const TVector3* a_pVertices, TUINT32 a_uiColour )
{
	Validate();

	// NOTE: as in AddLine, the count isn't reset after drawing
	if ( m_uiNumTriangleVerts >= MAX_VERTICES - 2 )
		RenderBuffers();

	static Vertex s_aVertices[ 3 ];
	s_aVertices[ 0 ].uiColour = a_uiColour;
	s_aVertices[ 1 ].uiColour = a_uiColour;
	s_aVertices[ 2 ].uiColour = a_uiColour;

	for ( TINT i = 0; i < 3; i++ )
	{
		s_aVertices[ i ].fX = a_pVertices[ i ].x;
		s_aVertices[ i ].fY = a_pVertices[ i ].y;
		s_aVertices[ i ].fZ = a_pVertices[ i ].z;
	}

	TRenderDX11::Interface()->UpdateBuffer( m_pTriangleBuffer, sizeof( s_aVertices ), s_aVertices, m_uiNumTriangleVerts * sizeof( Vertex ), D3D11_MAP_WRITE_NO_OVERWRITE );
	m_uiNumTriangleVerts += 3;
}

// $deBlob: FUNCTION 0055ef90
void ALineShaderHAL::AddTriangle( const TVector4& a_rVertex1, const TVector4& a_rVertex2, const TVector4& a_rVertex3, TUINT32 a_uiColour )
{
	Validate();

	// NOTE: as in AddLine, the count isn't reset after drawing
	if ( m_uiNumTriangleVerts >= MAX_VERTICES - 2 )
		RenderBuffers();

	static Vertex s_aVertices[ 3 ];
	s_aVertices[ 0 ].uiColour = a_uiColour;
	s_aVertices[ 1 ].uiColour = a_uiColour;
	s_aVertices[ 2 ].uiColour = a_uiColour;
	s_aVertices[ 0 ].fX       = a_rVertex1.x;
	s_aVertices[ 0 ].fY       = a_rVertex1.y;
	s_aVertices[ 0 ].fZ       = a_rVertex1.z;
	s_aVertices[ 1 ].fX       = a_rVertex2.x;
	s_aVertices[ 1 ].fY       = a_rVertex2.y;
	s_aVertices[ 1 ].fZ       = a_rVertex2.z;
	s_aVertices[ 2 ].fX       = a_rVertex3.x;
	s_aVertices[ 2 ].fY       = a_rVertex3.y;
	s_aVertices[ 2 ].fZ       = a_rVertex3.z;

	TRenderDX11::Interface()->UpdateBuffer( m_pTriangleBuffer, sizeof( s_aVertices ), s_aVertices, m_uiNumTriangleVerts * sizeof( Vertex ), D3D11_MAP_WRITE_NO_OVERWRITE );
	m_uiNumTriangleVerts += 3;
}

// $deBlob: FUNCTION 0055f040
void ALineShaderHAL::Reset()
{
	m_uiNumTriangleVerts = 0;
	m_uiNumLineVerts     = 0;
}

// $deBlob: FUNCTION 0055f050
TBOOL ALineShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

// $deBlob: FUNCTION 0055f060
TBOOL ALineShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

// $deBlob: FUNCTION 0055f3c0
void ALineShaderHAL::EndFlush()
{
}

// $deBlob: FUNCTION 0055f470
void ALineShaderHAL::Render( TRenderPacket* a_pRenderPacket )
{
}

// $deBlob: FUNCTION 0055f4c0
void ALineShaderHAL::StartFlush()
{
}

//-----------------------------------------------------------------------------
// ALineMaterialHAL
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0053b3e0
// $deBlob: FUNCTION 0055f080
// $deBlob: FUNCTION 0055f090
// $deBlob: FUNCTION 0055f100
// $deBlob: FUNCTION 0055f170
// $deBlob: FUNCTION 0055f180
// $deBlob: FUNCTION 00857ea0
TDEFINE_CLASS( ALineMaterialHAL );

// $deBlob: FUNCTION 0055f2a0
ALineMaterialHAL::ALineMaterialHAL()
{
}

// $deBlob: FUNCTION 0055f320
ALineMaterialHAL::~ALineMaterialHAL()
{
}

//-----------------------------------------------------------------------------
// ALineMeshHAL
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0053b420
// $deBlob: FUNCTION 0055f190
// $deBlob: FUNCTION 0055f1a0
// $deBlob: FUNCTION 0055f210
// $deBlob: FUNCTION 0055f280
// $deBlob: FUNCTION 0055f290
// $deBlob: FUNCTION 00857eb0
TDEFINE_CLASS( ALineMeshHAL );

// $deBlob: FUNCTION 0055f2c0
ALineMeshHAL::ALineMeshHAL()
{
}

// $deBlob: FUNCTION 0055f350
ALineMeshHAL::~ALineMeshHAL()
{
}
