#include "ToshiPCH.h"
#include "TSysShader_DX11.h"
#include "TRender_DX11.h"
#include "TRenderContext_DX11.h"
#include "TTexture_DX11.h"

#include <DirectXMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TUINT32  TSysMaterialHAL::s_eSysMaterialLazyState = LAZYSTATE_RESET;
TVector4 TSysMaterialHAL::s_AlphaTest;
TBOOL    TSysMeshHAL::s_bStopRendering = TFALSE;

// The shaders of the system shader
static const TCHAR* s_szSysShaderVS =
    "struct VS_IN { float3 ObjPos : POSITION; float4 Color : COLOR; float2 UV : TEXCOORD0; }; struct VS_OUT { float4 ProjPos "
    ": SV_POSITION; float4 Color : COLOR; float2 UV : TEXCOORD0; }; float4x4 matWVP : register(c0); float4x4 matWV : regis"
    "ter(c4); float3x3 matNorm : register(c8); float4x4 texMtx0 : register(c12); float4x4 texMtx1 : register(c16); float4x4"
    " texMtx2 : register(c20); float4x4 texMtx3 : register(c24); float4x4 texMtx6 : register(c28); float4x4 texMtx7 : regis"
    "ter(c32); float4 fogStartEnd : register(c36); VS_OUT main(VS_IN In) { VS_OUT Out; Out.ProjPos = mul(matWVP, float4(In"
    ".ObjPos, 1.0)); Out.Color = In.Color; Out.UV = In.UV; return Out; }";

static const TCHAR* s_szSysShaderPS =
    "struct VS_OUT { float4 ProjPos : SV_POSITION; float4 Color : COLOR; float2 UV : TEXCOORD0; }; float4 alphaTest : regis"
    "ter(c0); float4 main(VS_OUT In) : SV_TARGET { float4 final = In.Color; if (alphaTest.x > 0 && final.a < alphaTest.y) d"
    "iscard; return final; }";

static const TCHAR* s_szSysShaderTexturePS =
    "struct VS_OUT { float4 ProjPos : SV_POSITION; float4 Color : COLOR; float2 UV : TEXCOORD0; }; uniform sampler2D textu"
    "re0 : register(s0); float4 alphaTest : register(c0); float4 main(VS_OUT In) : SV_TARGET { float4 final = tex2D(textur"
    "e0, In.UV) * In.Color; if (alphaTest.x > 0 && final.a < alphaTest.y) discard; return final; }";

static const TCHAR* s_szSysShaderAlphaTexturePS =
    "struct VS_OUT { float4 ProjPos : SV_POSITION; float4 Color : COLOR; float2 UV : TEXCOORD0; }; uniform sampler2D textu"
    "re0 : register(s0); float4 alphaTest : register(c0); float4 main(VS_OUT In) : SV_TARGET { float4 final = In.Color; fi"
    "nal.a *= tex2D(texture0, In.UV).a; if (alphaTest.x > 0 && final.a < alphaTest.y) discard; return final; }";

//-----------------------------------------------------------------------------
// TSysShaderHAL
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 007093b0
// $deBlob: FUNCTION 007093c0
// $deBlob: FUNCTION 007093e0
// $deBlob: FUNCTION 00709400
// $deBlob: FUNCTION 00709410
// $deBlob: FUNCTION 0053fc30
// $deBlob: FUNCTION 00859570
TDEFINE_CLASS( TSysShaderHAL );

// $deBlob: FUNCTION 00709420
TSysShaderHAL::TSysShaderHAL( const OrderTablePriorities* a_pPriorities )
{
	m_pVertexShader            = TNULL;
	m_pPixelShaderNoTex        = TNULL;
	m_pPixelShaderTexture      = TNULL;
	m_pPixelShaderAlphaTexture = TNULL;
	m_pInputLayout             = TNULL;

	if ( a_pPriorities != TNULL )
	{
		m_oPriorities = *a_pPriorities;
		return;
	}

	m_oPriorities.iPriority1 = 100;
	m_oPriorities.iPriority2 = 200;
	m_oPriorities.iPriority3 = 300;
	m_oPriorities.iPriority4 = 400;
	m_oPriorities.iPriority5 = 500;
	m_oPriorities.iPriority6 = 500;
	m_oPriorities.iPriority7 = 400;
}

// $deBlob: FUNCTION 00709520
// $deBlob: FUNCTION 0070a6c0
TSysShaderHAL::~TSysShaderHAL()
{
}

// $deBlob: FUNCTION 00709570
void TSysShaderHAL::Flush()
{
	m_aOrderTables[ 0 ].Render();
	m_aOrderTables[ 1 ].Render();
}

// $deBlob: FUNCTION 00709590
void TSysShaderHAL::StartFlush()
{
	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();

	TSysMaterialHAL::s_eSysMaterialLazyState = TSysMaterialHAL::LAZYSTATE_RESET;

	pRender->SetColorUpdate( TTRUE );
	pRender->SetDstAlpha( 0.0f );

	// Cull mode: D3D11_CULL_NONE
	TUINT8& rRasterizerFlags = TREINTERPRETCAST( TUINT8*, &pRender->GetCurrentRasterizer().uiFlags )[ 0 ];
	rRasterizerFlags         = ( rRasterizerFlags & 0xF7 ) | ( D3D11_CULL_NONE << TRenderDX11::RASTERIZERFLAGS_CULLSHIFT );

	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );

	pDeviceContext->IASetInputLayout( m_pInputLayout );
	pDeviceContext->VSSetShader( m_pVertexShader, TNULL, 0 );
}

// $deBlob: FUNCTION 00709620
void TSysShaderHAL::EndFlush()
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	pRender->SetColorUpdate( TTRUE );
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
}

// $deBlob: FUNCTION 00709650
TBOOL TSysShaderHAL::Create()
{
	TASSERT( TFALSE == IsCreated() );

	TBOOL bResult;

	bResult = m_aOrderTables[ 0 ].Create( this, m_oPriorities.iPriority1, 0xFF );
	TASSERT( bResult );

	bResult = m_aOrderTables[ 1 ].Create( this, m_oPriorities.iPriority2, 0xFF );
	TASSERT( bResult );

	bResult = m_aOrderTables[ 2 ].Create( this, m_oPriorities.iPriority3, 0xFF );
	TASSERT( bResult );

	bResult = m_aOrderTables[ 3 ].Create( this, m_oPriorities.iPriority7, 0xFF );
	TASSERT( bResult );

	bResult = m_aOrderTables[ 4 ].Create( this, m_oPriorities.iPriority5, 0xFF );
	TASSERT( bResult );

	bResult = m_aOrderTables[ 5 ].Create( this, m_oPriorities.iPriority6, 0xFF );
	TASSERT( bResult );

	TRenderDX11* pRender = TRenderDX11::Interface();

	ID3DBlob* pVSBlob = TRenderDX11::CompileShader( s_szSysShaderVS, "main", "vs_4_0_level_9_3", TNULL );
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

	pRender->GetDevice()->CreateInputLayout( aInputDescs, 3, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout );
	pVSBlob->Release();

	ID3DBlob* pPSBlob = TRenderDX11::CompileShader( s_szSysShaderPS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShaderNoTex );
	pPSBlob->Release();

	pPSBlob = TRenderDX11::CompileShader( s_szSysShaderTexturePS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShaderTexture );
	pPSBlob->Release();

	pPSBlob = TRenderDX11::CompileShader( s_szSysShaderAlphaTexturePS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShaderAlphaTexture );
	pPSBlob->Release();

	TASSERT( TFALSE == IsCreated() );
	return TShader::Create();
}

// $deBlob: FUNCTION 00709a50
void TSysShaderHAL::OnDestroy()
{
	if ( m_pVertexShader != TNULL )
	{
		m_pVertexShader->Release();
		m_pVertexShader = TNULL;
	}

	if ( m_pPixelShaderNoTex != TNULL )
	{
		m_pPixelShaderNoTex->Release();
		m_pPixelShaderNoTex = TNULL;
	}

	if ( m_pPixelShaderTexture != TNULL )
	{
		m_pPixelShaderTexture->Release();
		m_pPixelShaderTexture = TNULL;
	}

	if ( m_pPixelShaderAlphaTexture != TNULL )
	{
		m_pPixelShaderAlphaTexture->Release();
		m_pPixelShaderAlphaTexture = TNULL;
	}

	if ( m_pInputLayout != TNULL )
	{
		m_pInputLayout->Release();
		m_pInputLayout = TNULL;
	}

	TShader::OnDestroy();
}

// $deBlob: FUNCTION 00709ae0
TBOOL TSysShaderHAL::Validate()
{
	TASSERT( TTRUE == IsCreated() );
	return TShader::Validate();
}

// $deBlob: FUNCTION 00709b20
void TSysShaderHAL::Invalidate()
{
	if ( IsValidated() )
		TShader::Invalidate(); // inlined
}

// $deBlob: FUNCTION 00709b30
TSysMaterial* TSysShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	TASSERT( TTRUE == IsCreated() );
	Validate();

	TSysMaterialHAL* pMaterial = new TSysMaterialHAL();
	pMaterial->SetShader( this );

	return pMaterial;
}

// $deBlob: FUNCTION 00709be0
TSysMesh* TSysShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	TASSERT( TTRUE == IsCreated() );
	Validate();

	TSysMeshHAL* pMesh = new TSysMeshHAL();
	pMesh->SetOwnerShader( this );

	return pMesh;
}

// $deBlob: FUNCTION 00709c80
void TSysShaderHAL::Render( TRenderPacket* a_pRenderPacket )
{
	TRenderDX11*        pRender  = TRenderDX11::Interface();
	TSysMeshHAL*        pMesh    = TSTATICCAST( TSysMeshHAL, a_pRenderPacket->GetMesh() );
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
	pRender->SetVec4InPSBuffer( 0, &TSysMaterialHAL::s_AlphaTest, 1 );
	pRender->DrawIndexed( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, pMesh->GetIndexCount(), pMesh->GetIndexBuffer(), 0, DXGI_FORMAT_R16_UINT, pMesh->GetVertexBuffer(), 0x18, 0 );
}

//-----------------------------------------------------------------------------
// TSysMaterialHAL
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00709da0
// $deBlob: FUNCTION 00709db0
// $deBlob: FUNCTION 00709de0
// $deBlob: FUNCTION 00709e10
// $deBlob: FUNCTION 00709e20
// $deBlob: FUNCTION 0053fbb0
// $deBlob: FUNCTION 00859550
TDEFINE_CLASS( TSysMaterialHAL );

// $deBlob: FUNCTION 00709e30
TSysMaterialHAL::TSysMaterialHAL()
{
	m_pOrderTable = TNULL;
}

// $deBlob: FUNCTION 00709e50
// $deBlob: FUNCTION 0070a650
TSysMaterialHAL::~TSysMaterialHAL()
{
	if ( m_pOrderTable != TNULL )
	{
		TOrderTable::DeregisterMaterial( m_pRegMaterial );
		m_pOrderTable = TNULL;
	}
}

// $deBlob: FUNCTION 00709e80
TBOOL TSysMaterialHAL::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode );
	return TMaterial::Create();
}

// $deBlob: FUNCTION 00709ea0
void TSysMaterialHAL::SetBlendMode( BLENDMODE a_eBlendMode )
{
	TSysShaderHAL* pShader = TSTATICCAST( TSysShaderHAL, m_pShader );
	TOrderTable*   pOrderTable;

	switch ( a_eBlendMode )
	{
		case BLENDMODE_DEFAULT:
			pOrderTable = pShader->GetOrderTable( 0 );
			break;
		default:
			pOrderTable = pShader->GetOrderTable( 1 );
			break;
		case BLENDMODE_ADDITIVE:
			pOrderTable = pShader->GetOrderTable( 2 );
			break;
		case BLENDMODE_ALPHATEST:
			pOrderTable = pShader->GetOrderTable( 3 );
			break;
		case BLENDMODE_6:
			pOrderTable = pShader->GetOrderTable( 4 );
			break;
		case BLENDMODE_7:
			pOrderTable = pShader->GetOrderTable( 5 );
			break;
	}

	SetOrderTable( pOrderTable );
	TSysMaterial::SetBlendMode( a_eBlendMode );
}

// $deBlob: FUNCTION 0070a290
void TSysMaterialHAL::SetOrderTable( TOrderTable* a_pOrderTable )
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

// $deBlob: FUNCTION 00709f40
void TSysMaterialHAL::PreRender()
{
	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();
	TSysShaderHAL*       pShader        = TSTATICCAST( TSysShaderHAL, m_pShader );

	s_eSysMaterialLazyState = LAZYSTATE_RESET;
	TTexture* pTexture      = m_pTextures[ 0 ];

	if ( pTexture == TNULL )
	{
		pDeviceContext->PSSetShader( pShader->m_pPixelShaderNoTex, TNULL, 0 );
	}
	else
	{
		pTexture->Bind( 0 );
		pDeviceContext->PSSetShader( ( m_Flags & FLAGS_ALPHATEXTURE ) ? pShader->m_pPixelShaderAlphaTexture : pShader->m_pPixelShaderTexture, TNULL, 0 );
	}

	s_AlphaTest.x = 1.0f;
	s_AlphaTest.y = 1.0f / 255.0f;
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );

	if ( !( m_Flags & FLAGS_ALPHAUPDATE ) )
	{
		pRender->SetAlphaUpdate( TTRUE );
		pRender->SetDstAlpha( -1.0f );
	}
	else
	{
		pRender->SetAlphaUpdate( TFALSE );
	}

	TBOOL bNoDepthTest = ( m_Flags & FLAGS_NO_DEPTH_TEST ) != 0;

	if ( bNoDepthTest )
		pRender->SetZMode( TFALSE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );

	TBOOL bDepthEnable = !bNoDepthTest;

	switch ( m_eBlendMode )
	{
		case BLENDMODE_DEFAULT:
			if ( pTexture == TNULL )
			{
				if ( s_eSysMaterialLazyState != LAZYSTATE_DEFAULT_UNTEXTURED )
				{
					pRender->SetZMode( bDepthEnable, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
					s_eSysMaterialLazyState = LAZYSTATE_DEFAULT_UNTEXTURED;
				}
			}
			else if ( ( pTexture->GetFlags1() & TTexture::Flags1_Alpha ) == 0 )
			{
				if ( s_eSysMaterialLazyState != LAZYSTATE_DEFAULT_TEXTURED )
				{
					pRender->SetZMode( bDepthEnable, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
					s_eSysMaterialLazyState = LAZYSTATE_DEFAULT_TEXTURED;
				}
			}
			else if ( s_eSysMaterialLazyState != LAZYSTATE_DEFAULT_ALPHATEXTURE )
			{
				s_AlphaTest.x           = 1.0f;
				s_AlphaTest.y           = 1.0f / 255.0f;
				s_eSysMaterialLazyState = LAZYSTATE_DEFAULT_ALPHATEXTURE;
			}
			break;
		case BLENDMODE_1:
			if ( s_eSysMaterialLazyState != LAZYSTATE_BLENDMODE_1 )
			{
				s_AlphaTest.x           = 1.0f;
				s_AlphaTest.y           = 1.0f / 255.0f;
				s_eSysMaterialLazyState = LAZYSTATE_BLENDMODE_1;
			}
			break;
		case BLENDMODE_2:
			if ( s_eSysMaterialLazyState != LAZYSTATE_BLENDMODE_2 )
			{
				s_AlphaTest.x           = 1.0f;
				s_AlphaTest.y           = 1.0f / 255.0f;
				s_eSysMaterialLazyState = LAZYSTATE_BLENDMODE_2;
			}
			break;
		case BLENDMODE_ADDITIVE:
			if ( s_eSysMaterialLazyState != LAZYSTATE_ADDITIVE )
			{
				s_AlphaTest.x = 1.0f;
				s_AlphaTest.y = 1.0f / 255.0f;
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE );
				s_eSysMaterialLazyState = LAZYSTATE_ADDITIVE;
			}
			break;
		case BLENDMODE_4:
			if ( s_eSysMaterialLazyState != LAZYSTATE_BLENDMODE_4 )
			{
				s_AlphaTest.x           = 1.0f;
				s_AlphaTest.y           = 1.0f / 255.0f;
				s_eSysMaterialLazyState = LAZYSTATE_BLENDMODE_4;
			}
			break;
		case BLENDMODE_ALPHATEST:
			if ( s_eSysMaterialLazyState != LAZYSTATE_ALPHATEST )
			{
				s_AlphaTest.x           = 1.0f;
				s_AlphaTest.y           = 128.0f / 255.0f;
				s_eSysMaterialLazyState = LAZYSTATE_ALPHATEST;
			}
			break;
		case BLENDMODE_6:
		case BLENDMODE_7:
			if ( s_eSysMaterialLazyState != LAZYSTATE_BLENDMODE_6_7 )
			{
				s_AlphaTest.x = 1.0f;
				s_AlphaTest.y = 1.0f / 255.0f;
				pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE );
				s_eSysMaterialLazyState = LAZYSTATE_BLENDMODE_6_7;
			}
			break;
		default:
			TASSERT( !"TSysshader default\n" );
			break;
	}
}

// $deBlob: FUNCTION 0070a270
void TSysMaterialHAL::PostRender()
{
	if ( m_Flags & FLAGS_NO_DEPTH_TEST )
		TRenderDX11::Interface()->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
}

//-----------------------------------------------------------------------------
// TSysMeshHAL
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0070a2f0
// $deBlob: FUNCTION 0070a300
// $deBlob: FUNCTION 0070a330
// $deBlob: FUNCTION 0070a360
// $deBlob: FUNCTION 0070a370
// $deBlob: FUNCTION 0053fbf0
// $deBlob: FUNCTION 00859560
TDEFINE_CLASS( TSysMeshHAL );

// $deBlob: FUNCTION 0070a380
TSysMeshHAL::TSysMeshHAL()
{
	// NOTE: retail doesn't initialise the buffers and counts (Create sets the buffers)
	m_pVertexBuffer = TNULL;
	m_pIndexBuffer  = TNULL;
	m_uiNumVertices = 0;
	m_uiIndexCount  = 0;
}

// $deBlob: FUNCTION 0070a690
// $deBlob: FUNCTION 0070a3a0
TSysMeshHAL::~TSysMeshHAL()
{
}

// $deBlob: FUNCTION 0070a3b0
TBOOL TSysMeshHAL::Render()
{
	if ( !s_bStopRendering )
	{
		TRenderContext* pContext = TRenderDX11::Interface()->GetCurrentContext();
		TRenderPacket*  pPacket  = m_pMaterial->GetRegMaterial()->AddRenderPacket( this );

		pPacket->SetModelViewMatrix( pContext->GetModelViewMatrix() );
		pPacket->SetFade( TUINT8( TINT( pContext->GetAlphaBlend() * 255.0f ) ) );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0070a5b0
TBOOL TSysMeshHAL::Create( FLAGS a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiMaxIndices )
{
	TASSERT( TFALSE == IsCreated() );

	m_uiFlags       = a_uiFlags;
	m_uiMaxVertices = a_uiMaxVertices;
	m_uiMaxIndices  = a_uiMaxIndices;

	TRenderDX11* pRender = TRenderDX11::Interface();
	m_pVertexBuffer      = pRender->CreateBuffer( TRenderDX11::BUFFERTYPE_VERTEX, a_uiMaxVertices * 0x18, TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
	m_pIndexBuffer       = pRender->CreateBuffer( TRenderDX11::BUFFERTYPE_INDEX, a_uiMaxIndices * 2, TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
	m_State |= State_Created;

	return TTRUE;
}

// $deBlob: FUNCTION 0070a400
TBOOL TSysMeshHAL::Lock( TLockBuffer* a_pLockBuffer )
{
	TASSERT( ( m_uiFlags & FLAGS_LOCKED ) == 0 );

	ID3D11DeviceContext*     pDeviceContext = TRenderDX11::Interface()->GetDeviceContext();
	D3D11_MAPPED_SUBRESOURCE oMapped;

	pDeviceContext->Map( m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
	a_pLockBuffer->oVertices.apStreams[ 0 ] = TSTATICCAST( TBYTE, oMapped.pData );

	pDeviceContext->Map( m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
	a_pLockBuffer->oIndices.pBuffer = TSTATICCAST( TIndexType, oMapped.pData );

	a_pLockBuffer->oVertices.uiNumStreams = 1;
	m_uiFlags |= FLAGS_LOCKED;

	return TTRUE;
}

// $deBlob: FUNCTION 0070a4b0
void TSysMeshHAL::Unlock( TUINT32 a_uiNumVertices, TUINT32 a_uiNumIndices )
{
	TASSERT( m_uiFlags & FLAGS_LOCKED );
	TASSERT( TUINT16( a_uiNumVertices ) >= 3 );
	TASSERT( TUINT16( a_uiNumIndices ) >= 3 );

	ID3D11DeviceContext* pDeviceContext = TRenderDX11::Interface()->GetDeviceContext();
	pDeviceContext->Unmap( m_pVertexBuffer, 0 );
	pDeviceContext->Unmap( m_pIndexBuffer, 0 );

	m_uiFlags &= ~FLAGS_LOCKED;
	m_uiNumVertices = TUINT16( a_uiNumVertices );
	m_uiIndexCount  = TUINT16( a_uiNumIndices );
}

// $deBlob: FUNCTION 0070a880
void TSysMeshHAL::SetZBias( TINT a_iZBias )
{
	TASSERT( a_iZBias >= 0 );
}

TOSHI_NAMESPACE_END
