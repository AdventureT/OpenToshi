#include "pch.h"
#include "ASky_DX11.h"
#include "Cameras/ACamera.h"
#include "Render/Material/AMaterialInfo.h"

#include <Render/TRender.h>
#include <Render/TRenderContext.h>
#include <Render/TModel.h>
#include <Render/TModelManager.h>
#include <Render/TMesh.h>
#include <Render/TTMDWin.h>
#include <Platform/DX11/TRender_DX11.h>
#include <Platform/DX11/TWorldShader_DX11.h>

#include <DirectXMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL ASky::s_bHideSky      = TFALSE;
TBOOL ASky::s_bHideGradient = TFALSE;
TBOOL ASky::s_bHideClouds   = TFALSE;

// The gradient: positions in clip space (z 1, at the far plane), the colour as it comes
static const TCHAR* s_szSkyVS = "struct VS_IN { float3 ObjPos : POSITION; float4 Color : COLOR; }; struct VS_OUT { float4 ProjPos : SV_POSITION; float4 Color : COLOR; }; float4x4 matWVP : register(c0); float4x4 matWV : register(c4); float3x3 matNorm : register(c8); float4x4 texMtx0 : register(c12); float4x4 texMtx1 : register(c16); float4x4 texMtx2 : register(c20); float4x4 texMtx3 : register(c24); float4x4 texMtx6 : register(c28); float4x4 texMtx7 : register(c32); float4x3 bones[30] : register(c40); float3x3 bonesNrm[30] : register(c130); VS_OUT main(VS_IN In) { VS_OUT Out; Out.ProjPos = mul(matWVP, float4(In.ObjPos, 1.0)); Out.Color = In.Color; return Out; }";
static const TCHAR* s_szSkyPS = "struct VS_OUT { float4 ProjPos : SV_POSITION; float4 Color : COLOR; }; uniform sampler2D texture0 : register(s0); uniform sampler2D texture1 : register(s1); uniform sampler2D texture2 : register(s2); uniform sampler2D texture3 : register(s3); float4 alphaTest : register(c0); float4 kColor0 : register(c1); float4 kColor1 : register(c2); float4 kColor2 : register(c3); float4 kColor3 : register(c4); float4 tevReg0 : register(c5); float4 tevReg1 : register(c6); float4 tevReg2 : register(c7); float4 chanMatColor0 : register(c8); float4 chanMatColor1 : register(c9); float4 chanAmbColor0 : register(c10); float4 chanAmbColor1 : register(c11); float4 light0Pos : register(c12); float4 light0Color : register(c13); float4 light6Pos : register(c14); float4 light6Color : register(c15); float4 light7Pos : register(c16); float4 light7Color : register(c17); float4 main(VS_OUT In) : SV_TARGET { float4 result = In.Color; return result; }";

static const TFLOAT s_afCloudRotSpeeds[ ASky::NUM_LAYERS ] = { 1.0f, 2.0f, 3.0f };
static const TFLOAT s_afStarsRotSpeeds[ ASky::NUM_LAYERS ] = { 1.0f, 2.0f, 5.0f };
static TFLOAT       s_fGradientScale                       = 0.1f; // (no writer)

static const TVector4 s_vWorldUp = { 0.0f, -1.0f, 0.0f, 1.0f };

struct SkyVertex
{
	TFLOAT  x, y, z;
	TUINT32 uiColour;
};

static TUINT32 PackColour( TFLOAT a_fR, TFLOAT a_fG, TFLOAT a_fB, TFLOAT a_fA )
{
	return TUINT32( ( ( ( ( TINT( a_fA * 255.0f ) << 8 ) | TINT( a_fR * 255.0f ) ) << 8 | TINT( a_fG * 255.0f ) ) << 8 ) | TINT( a_fB * 255.0f ) );
}

static TWorldShaderHAL* GetWorldShader()
{
	TWorldShaderHAL* pWorldShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );
	TASSERT( ( pWorldShader == TNULL ) || pWorldShader->IsA( &TGetClass( TWorldShaderHAL ) ) );
	return pWorldShader;
}

// $deBlob: FUNCTION 00556450
// $deBlob: FUNCTION 00557ec0
ASky::ASky()
{
	m_pVertexShader = TNULL;
	m_pPixelShader  = TNULL;
	m_pInputLayout  = TNULL;

	m_avCorners[ 0 ] = TVector2( -1.0f, 1.0f );
	m_avCorners[ 1 ] = TVector2( 1.0f, 1.0f );
	m_avCorners[ 2 ] = TVector2( -1.0f, -1.0f );
	m_avCorners[ 3 ] = TVector2( 1.0f, -1.0f );
	m_uiTopColour    = 0xFF000000;
	m_uiBottomColour = 0xFF000000;

	m_oSkyColours.m_vColour1 = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_oSkyColours.m_vColour2 = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );

	TWorldShaderHAL* pWorldShader = GetWorldShader();
	TOrderTable*     pOrderTable  = pWorldShader->GetOrderTable( ORDERTABLE_SKY );
	pOrderTable->SetPreFlushCallback( OnPreFlush, this );

	static const TCHAR* s_aszModels[ NUM_LAYERS ] = { "sky_base.txs", "sky_cloudsbig.txs", "sky_cloudssmall.txs" };

	for ( TINT i = 0; i < NUM_LAYERS; i++ )
	{
		AEnvFXColours& rColours = m_aCloudColours[ i ];
		rColours.m_vColour1     = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		rColours.m_vColour2     = rColours.m_vColour1;
		rColours.m_vUnk10       = rColours.m_vColour1;
		rColours.m_vUnk30       = rColours.m_vColour1;

		T2ModelPtr pModel = TModelManager::GetSingleton()->FindModel( s_aszModels[ i ] );

		TASSERT( 0 < pModel->GetLODCount() );
		TWorldMaterialHAL* pCloudMaterial = TSTATICCAST( TWorldMaterialHAL, pModel->GetLOD( 0 ).GetMesh( 0 )->GetMaterial() );
		TASSERT( ( pCloudMaterial == TNULL ) || pCloudMaterial->IsA( &TGetClass( TWorldMaterialHAL ) ) );

		pCloudMaterial->SetOrderTable( pOrderTable, 0, TTRUE );
		TASSERT( pCloudMaterial->GetTextureNum() == 2 );
		pCloudMaterial->SetBlendMode( 19 );
		AMaterialInfo::SwapLightMapPasses( pCloudMaterial );
		pCloudMaterial->SetMultiPassBlendType( 0, TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPCOLOUR );
		pCloudMaterial->SetMultiPassBlendType( 1, TWorldMaterialHAL::MULTIPASSBLENDTYPE_WRITEALPHA );
		pCloudMaterial->SetUVStream( 1, 1 );

		m_apClouds[ i ] = pModel->CreateInstance();
		m_apClouds[ i ]->SetDistanceFade( TFALSE );

		TTMDWin::TTRBWorldHeader* pHeader = TSTATICCAST( TTMDWin::TTRBWorldHeader, pModel->GetAssetData() );
		m_aCloudVis[ i ].Create( pHeader->m_pWorldData->m_ppGroups[ 0 ], TNULL );
		m_aCloudVis[ i ].SetRenderCallback( RenderCellMesh );

		// The layer's colour in TWorldShaderHAL's table
		pCloudMaterial->SetUserDataIndex( TWorldMaterialHAL::UData_2, TINT16( i ) );
	}

	m_fCloudSpeed           = 0.0f;
	m_iNumRenderCalls       = 0;
	m_bSkySpace             = TFALSE;
	m_bSkySpaceReached      = TFALSE;
	m_bHasStars             = TFALSE;
	m_fCloudDescendSpeed    = 0.0f;
	m_fCloudDescendTime     = 0.0f;
	m_vCloudOffset          = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_fCloudDescendAccel    = 0.0f;
	m_fCloudDescendDuration = 1.0f;
	m_fAlpha                = 1.0f;
	m_bRenderClouds         = TTRUE;
	m_fStarsSpeed           = 0.0f;
	m_fStarsTime            = 0.0f;
	m_vStarsOffset          = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_bRenderStars          = TFALSE;
	m_apStars[ 0 ]          = TNULL;
	m_afStarsPhases[ 0 ]    = 1.0f;
	m_apStars[ 1 ]          = TNULL;
	m_afStarsPhases[ 1 ]    = 1.0f;
	m_apStars[ 2 ]          = TNULL;
	m_afStarsPhases[ 2 ]    = 1.0f;

	TRenderDX11*  pRender = TRenderDX11::Interface();
	ID3D11Device* pDevice = pRender->GetDevice();

	ID3DBlob* pBlob = TRenderDX11::CompileShader( s_szSkyVS, "main", "vs_4_0_level_9_3", TNULL );
	pRender->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pVertexShader );

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
	pDevice->CreateInputLayout( aInputDescs, 2, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pInputLayout );
	pBlob->Release();

	pBlob = TRenderDX11::CompileShader( s_szSkyPS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pPixelShader );
	pBlob->Release();
}

// $deBlob: FUNCTION 005569f0
// $deBlob: FUNCTION 005b80d0
// $deBlob: FUNCTION 00557f80
ASky::~ASky()
{
	if ( m_pVertexShader )
	{
		m_pVertexShader->Release();
		m_pVertexShader = TNULL;
	}

	if ( m_pPixelShader )
	{
		m_pPixelShader->Release();
		m_pPixelShader = TNULL;
	}

	if ( m_pInputLayout )
	{
		m_pInputLayout->Release();
		m_pInputLayout = TNULL;
	}

	GetWorldShader()->GetOrderTable( ORDERTABLE_SKY )->SetPreFlushCallback( TNULL, TNULL );

	for ( TINT i = 0; i < NUM_LAYERS; i++ )
	{
		if ( m_apClouds[ i ] )
			delete m_apClouds[ i ];

		m_apClouds[ i ] = TNULL;
	}

	for ( TINT i = 0; i < NUM_LAYERS; i++ )
	{
		if ( m_apStars[ i ] )
		{
			delete m_apStars[ i ];
			m_apStars[ i ] = TNULL;
		}
	}
}

// $deBlob: FUNCTION 00556ba0
void ASky::Render( ACamera* a_pCamera )
{
	if ( s_bHideSky )
		return;

	if ( !s_bHideGradient )
	{
		TASSERT( m_iNumRenderCalls < MAX_RENDER_CALLS );
		m_apRenderCalls[ m_iNumRenderCalls++ ] = a_pCamera;
	}

	if ( s_bHideClouds )
		return;

	TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();

	TMatrix44 mProjection;
	TRenderContext::ComputePerspectiveProjection( mProjection, pContext->GetViewportParameters(), pContext->GetProjectionParameters() );
	TMatrix44 mWorldView = pContext->GetModelViewMatrix();

	const TVector3& vCamPos = a_pCamera->GetMatrix().AsBasisVector3( BASISVECTOR_TRANSLATION );

	if ( m_bRenderClouds )
	{
		TVector3 vPos( m_vCloudOffset.x + vCamPos.x, m_vCloudOffset.y + vCamPos.y, m_vCloudOffset.z + vCamPos.z );

		for ( TINT i = NUM_LAYERS - 1; i >= 0; i-- )
		{
			TTransformObject& rTransform = m_apClouds[ i ]->GetTransform();
			rTransform.SetTranslate( vPos );

			TMatrix44 mModelView;
			mModelView.Multiply( mWorldView, rTransform.GetMatrix() );
			m_aCloudVis[ i ].Reset();
			m_aCloudVis[ i ].Build( mModelView, mProjection, 0, TFALSE );
			rTransform.Push();
			m_aCloudVis[ i ].Render( mModelView, mProjection, TFALSE );
			rTransform.Pop();
		}
	}

	if ( m_bRenderStars )
	{
		TVector3 vPos( m_vStarsOffset.x + vCamPos.x, m_vStarsOffset.y + vCamPos.y, m_vStarsOffset.z + vCamPos.z );

		for ( TINT i = NUM_LAYERS - 1; i >= 0; i-- )
		{
			TTransformObject& rTransform = m_apStars[ i ]->GetTransform();
			rTransform.SetTranslate( vPos );

			TMatrix44 mModelView;
			mModelView.Multiply( mWorldView, rTransform.GetMatrix() );
			m_aStarsVis[ i ].Reset();
			m_aStarsVis[ i ].Build( mModelView, mProjection, 0, TFALSE );
			rTransform.Push();
			m_aStarsVis[ i ].Render( mModelView, mProjection, TFALSE );
			rTransform.Pop();
		}
	}
}

// $deBlob: FUNCTION 00556f70
void ASky::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bSkySpace )
	{
		if ( !m_bSkySpaceReached )
		{
			// The clouds fade out and descend, the stars come down
			TBOOL bReached = TTRUE;

			if ( m_fAlpha > 0.0f )
			{
				bReached = TFALSE;
				m_fAlpha -= a_fDeltaTime * 2.0f;

				if ( m_fAlpha <= 0.0f )
					m_fAlpha = 0.0f;
			}

			if ( m_fCloudDescendTime > 0.0f )
			{
				TFLOAT fTime  = m_fCloudDescendTime - a_fDeltaTime;
				bReached      = TFALSE;
				TFLOAT fSpeed = m_fCloudDescendAccel * a_fDeltaTime + m_fCloudDescendSpeed;

				m_fCloudDescendTime  = fTime;
				m_fCloudDescendSpeed = fSpeed;
				m_vCloudOffset.y     = fSpeed * a_fDeltaTime + m_vCloudOffset.y;

				if ( fTime <= 0.0f )
				{
					m_fCloudDescendTime = 0.0f;
					m_bRenderClouds     = TFALSE;
				}
			}

			if ( m_fStarsTime > 0.0f )
			{
				bReached       = TFALSE;
				TFLOAT fOffset = m_fStarsSpeed * a_fDeltaTime + m_vStarsOffset.y;
				m_fStarsTime -= a_fDeltaTime;
				m_vStarsOffset.y = fOffset;
			}

			m_bSkySpaceReached = bReached;
		}
		else if ( m_bHasStars )
		{
			// The stars twinkle (not the base layer)
			static const TBOOL  s_abTwinkle[ NUM_LAYERS ]      = { TFALSE, TTRUE, TTRUE };
			static const TFLOAT s_afTwinkleSpeed[ NUM_LAYERS ] = { 1.0f, 0.5f, 0.2f };
			const TFLOAT        fMinTwinkle                    = 0.4f;

			TWorldShaderHAL* pWorldShader = GetWorldShader();

			for ( TINT i = 0; i < NUM_LAYERS; i++ )
			{
				TFLOAT fScale = 1.0f;

				if ( s_abTwinkle[ i ] )
				{
					m_afStarsPhases[ i ] = s_afTwinkleSpeed[ i ] * a_fDeltaTime + m_afStarsPhases[ i ];
					TFLOAT fTwinkle      = TFLOAT( ( TMath::Sin( m_afStarsPhases[ i ] ) + 1.0f ) * 0.5f );
					fScale               = ( fTwinkle <= 0.4f ) ? fMinTwinkle : fTwinkle;
				}

				const AEnvFXColours& rColours  = m_aStarsColours[ i ];
				TUINT32              uiColour1 = PackColour( rColours.m_vColour1.x, rColours.m_vColour1.y, rColours.m_vColour1.z, rColours.m_vColour1.w * fScale );
				TUINT32              uiColour2 = PackColour( rColours.m_vColour2.x, rColours.m_vColour2.y, rColours.m_vColour2.z, rColours.m_vColour2.w * fScale );

				TWorldShaderHAL::Colour* pColour = pWorldShader->GetColour( i + NUM_LAYERS );
				pColour->m_uiColour1             = uiColour1;
				pColour->m_uiColour2             = uiColour2;
			}
		}
	}

	if ( m_bRenderClouds )
	{
		for ( TINT i = 0; i < NUM_LAYERS; i++ )
		{
			TTransformObject& rTransform = m_apClouds[ i ]->GetTransform();
			TMatrix44         mMatrix    = rTransform.GetMatrix();
			mMatrix.RotateY( ( a_fDeltaTime * m_fCloudSpeed ) * s_afCloudRotSpeeds[ i ] );
			rTransform.SetMatrix( mMatrix );
		}
	}

	if ( m_bRenderStars )
	{
		for ( TINT i = 0; i < NUM_LAYERS; i++ )
		{
			TTransformObject& rTransform = m_apStars[ i ]->GetTransform();
			TMatrix44         mMatrix    = rTransform.GetMatrix();
			mMatrix.RotateY( ( a_fDeltaTime * m_fCloudSpeed ) * s_afStarsRotSpeeds[ i ] );
			rTransform.SetMatrix( mMatrix );
		}
	}
}

// $deBlob: FUNCTION 005573f0
void ASky::Reset()
{
	m_bSkySpace             = TFALSE;
	m_bSkySpaceReached      = TFALSE;
	m_fCloudDescendSpeed    = 0.0f;
	m_fCloudDescendAccel    = 0.0f;
	m_vCloudOffset          = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_fAlpha                = 1.0f;
	m_fCloudDescendTime     = 0.0f;
	m_fCloudDescendDuration = 1.0f;
	m_bRenderClouds         = TTRUE;
	m_bRenderStars          = TFALSE;
	m_afStarsPhases[ 0 ]    = 1.0f;
	m_afStarsPhases[ 1 ]    = 1.0f;
	m_afStarsPhases[ 2 ]    = 1.0f;
}

// $deBlob: FUNCTION 00557470
void ASky::SetColours( TFLOAT a_fBlend, const AEnvFXColours& a_rSky, const AEnvFXColours& a_rCloud1, const AEnvFXColours& a_rCloud2, const AEnvFXColours& a_rCloud3 )
{
	m_oSkyColours.Lerp( a_rSky, a_fBlend );

	const TVector4& vTop    = m_oSkyColours.m_vColour1;
	const TVector4& vBottom = m_oSkyColours.m_vColour2;
	m_uiTopColour           = PackColour( vTop.x, vTop.y, vTop.z, vTop.w * m_fAlpha );
	m_uiBottomColour        = PackColour( vBottom.x, vBottom.y, vBottom.z, vBottom.w * m_fAlpha );

	TWorldShaderHAL* pWorldShader = GetWorldShader();

	const AEnvFXColours* apClouds[ NUM_LAYERS ] = { &a_rCloud1, &a_rCloud2, &a_rCloud3 };

	if ( !m_bSkySpace )
	{
		for ( TINT i = 0; i < NUM_LAYERS; i++ )
		{
			AEnvFXColours& rColours = m_aCloudColours[ i ];
			rColours.Lerp( *apClouds[ i ], a_fBlend );

			TWorldShaderHAL::Colour* pColour = pWorldShader->GetColour( i );
			pColour->m_uiColour1             = PackColour( rColours.m_vColour1.x, rColours.m_vColour1.y, rColours.m_vColour1.z, rColours.m_vColour1.w );
			pColour->m_uiColour2             = PackColour( rColours.m_vColour2.x, rColours.m_vColour2.y, rColours.m_vColour2.z, rColours.m_vColour2.w );
		}
	}

	if ( m_bHasStars )
	{
		for ( TINT i = 0; i < NUM_LAYERS; i++ )
		{
			AEnvFXColours& rColours = m_aStarsColours[ i ];
			rColours.Lerp( *apClouds[ i ], a_fBlend );

			TUINT32 uiColour1 = PackColour( rColours.m_vColour1.x, rColours.m_vColour1.y, rColours.m_vColour1.z, rColours.m_vColour1.w );
			TUINT32 uiColour2 = PackColour( rColours.m_vColour2.x, rColours.m_vColour2.y, rColours.m_vColour2.z, rColours.m_vColour2.w );

			TWorldShaderHAL::Colour* pColour = pWorldShader->GetColour( i + NUM_LAYERS );
			pColour->m_uiColour1             = uiColour1;
			pColour->m_uiColour2             = uiColour2;
		}
	}
}

// $deBlob: FUNCTION 00557820
void ASky::CreateStars()
{
	TWorldShaderHAL* pWorldShader = GetWorldShader();
	TOrderTable*     pOrderTable  = pWorldShader->GetOrderTable( ORDERTABLE_SKY );

	static const TCHAR*                                s_aszModels[ NUM_LAYERS ]    = { "sky_base_stars.txs", "sky_cloudsbig_stars.txs", "sky_cloudssmall_stars.txs" };
	static const TWorldMaterial::BLENDMODE             s_aeBlendModes[ NUM_LAYERS ] = { 20, 21, 21 };
	static const TWorldMaterialHAL::MULTIPASSBLENDTYPE s_aePassBlends[ NUM_LAYERS ] = { TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPCOLOUR, TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPCOLOUR, TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPCOLOUR };

	for ( TINT i = 0; i < NUM_LAYERS; i++ )
	{
		AEnvFXColours& rColours = m_aStarsColours[ i ];
		rColours.m_vColour1     = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		rColours.m_vColour2     = rColours.m_vColour1;
		rColours.m_vUnk10       = rColours.m_vColour1;
		rColours.m_vUnk30       = rColours.m_vColour1;

		T2ModelPtr pModel = TModelManager::GetSingleton()->FindModel( s_aszModels[ i ] );

		TASSERT( 0 < pModel->GetLODCount() );
		TWorldMaterialHAL* pCloudMaterial = TSTATICCAST( TWorldMaterialHAL, pModel->GetLOD( 0 ).GetMesh( 0 )->GetMaterial() );
		TASSERT( ( pCloudMaterial == TNULL ) || pCloudMaterial->IsA( &TGetClass( TWorldMaterialHAL ) ) );

		pCloudMaterial->SetOrderTable( pOrderTable, 0, TTRUE );
		TASSERT( pCloudMaterial->GetTextureNum() == 1 );
		pCloudMaterial->SetBlendMode( s_aeBlendModes[ i ] );
		pCloudMaterial->SetMultiPassBlendType( 0, s_aePassBlends[ i ] );

		m_apStars[ i ] = pModel->CreateInstance();
		m_apStars[ i ]->SetDistanceFade( TFALSE );

		TTMDWin::TTRBWorldHeader* pHeader = TSTATICCAST( TTMDWin::TTRBWorldHeader, pModel->GetAssetData() );
		m_aStarsVis[ i ].Create( pHeader->m_pWorldData->m_ppGroups[ 0 ], TNULL );
		m_aStarsVis[ i ].SetRenderCallback( RenderCellMesh );

		pCloudMaterial->SetUserDataIndex( TWorldMaterialHAL::UData_2, TINT16( i + NUM_LAYERS ) );
	}

	m_bHasStars = TTRUE;
}

// $deBlob: FUNCTION 00557b70
void ASky::StartSkySpace( TFLOAT a_fCloudSpeed, TFLOAT a_fCloudDescendTime, TFLOAT a_fStarsHeight, TFLOAT a_fStarsSpeed, TFLOAT a_fStarsTime )
{
	if ( m_bSkySpace )
		return;

	TASSERT( a_fCloudDescendTime > 0.0f );

	m_fCloudDescendAccel    = 2.0f;
	m_bRenderClouds         = TTRUE;
	m_vCloudOffset          = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_fCloudDescendSpeed    = a_fCloudSpeed;
	m_fCloudDescendTime     = a_fCloudDescendTime;
	m_fCloudDescendDuration = a_fCloudDescendTime;
	m_bRenderStars          = TTRUE;
	m_vStarsOffset          = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_vStarsOffset.y        = m_vStarsOffset.y - a_fStarsHeight;
	m_fStarsSpeed           = a_fStarsSpeed;
	m_fStarsTime            = a_fStarsTime;
	m_afStarsPhases[ 0 ]    = 1.0f;
	m_afStarsPhases[ 1 ]    = TMath::HALF_PI;
	m_afStarsPhases[ 2 ]    = TMath::HALF_PI;
	m_bSkySpace             = TTRUE;
	m_bSkySpaceReached      = TFALSE;
}

// $deBlob: FUNCTION 00555c30
void ASky::RenderGradient( ACamera* const* a_ppCamera )
{
	TRenderDX11*    pRender  = TRenderDX11::Interface();
	TRenderContext* pContext = pRender->GetCurrentContext();

	TBOOL bFog = pContext->IsFogEnabled();
	pContext->EnableFog( TFALSE );

	ACamera* pCamera = *a_ppCamera;

	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
	pRender->SetDstAlpha( -1.0f );
	pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
	pRender->SetAlphaUpdate( TTRUE );
	pRender->SetColorUpdate( TTRUE );

	// No culling
	TUINT8& uiRasterizerFlags = *TREINTERPRETCAST( TUINT8*, &pRender->GetCurrentRasterizer().uiFlags );
	uiRasterizerFlags         = TUINT8( ( uiRasterizerFlags & 0xF7 ) | 4 );

	// The world's up in screen space gives the camera's roll
	TMatrix44 mView = pCamera->GetMatrix();
	mView.InvertOrthonormal();

	TVector4 vSSWorldUp = s_vWorldUp;
	vSSWorldUp.w        = 0.0f;
	TMatrix44::RotateVector( vSSWorldUp, mView, vSSWorldUp );
	TASSERT( vSSWorldUp.IsNormalised() );
	vSSWorldUp.z = 0.0f;

	TFLOAT fCamRot = vSSWorldUp.MagnitudeSq();

	if ( 0.025f <= fCamRot )
		vSSWorldUp.Normalise();
	else
		vSSWorldUp = TVector4( 0.0f, -1.0f, 0.0f, 0.0f );

	TFLOAT fCos = vSSWorldUp.y * -1.0f + vSSWorldUp.x * 0.0f + vSSWorldUp.z * 0.0f;
	TMath::Clip( fCos, -1.0f, 1.0f );
	fCamRot = TMath::ACos( fCos );

	if ( vSSWorldUp.x < 0.0f )
		fCamRot = TMath::TWO_PI - fCamRot;

	TASSERT( fCamRot >= 0.f && fCamRot <= TMath::TWO_PI );

	// The quad is scaled up so that its corners stay off screen while rolled
	static TFLOAT s_fMinRot1 = TMath::HALF_PI * 0.5f;
	static TFLOAT s_fMinRot2 = s_fMinRot1 + TMath::HALF_PI;

	TFLOAT fRot = fCamRot;

	if ( TMath::PI < fCamRot )
		fRot = fCamRot - TMath::PI;

	TFLOAT fDiff1  = TMath::Abs( s_fMinRot1 - fRot );
	TFLOAT fDiff2  = TMath::Abs( s_fMinRot2 - fRot );
	TFLOAT fMinRot = ( fDiff2 <= fDiff1 ) ? fDiff2 : fDiff1;
	TASSERT( fMinRot >= 0.f && fMinRot <= TMath::HALF_PI );

	TFLOAT fScale = TMath::LERP( s_fGradientScale + 1.4142135f, 1.0f, fMinRot / TMath::HALF_PI );

	TFLOAT fSin, fCos2;
	DirectX::XMScalarSinCos( &fSin, &fCos2, -fCamRot );

	DirectX::XMMATRIX mRotation(
	    DirectX::XMVectorSet( fCos2, fSin, 0.0f, 0.0f ),
	    DirectX::XMVectorSet( -fSin, fCos2, 0.0f, 0.0f ),
	    DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ),
	    DirectX::XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f )
	);
	DirectX::XMMATRIX mScale(
	    DirectX::XMVectorSet( fScale, 0.0f, 0.0f, 0.0f ),
	    DirectX::XMVectorSet( 0.0f, fScale, 0.0f, 0.0f ),
	    DirectX::XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ),
	    DirectX::XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f )
	);

	DirectX::XMFLOAT4X4 mWVP;
	DirectX::XMStoreFloat4x4( &mWVP, DirectX::XMMatrixMultiply( mRotation, mScale ) );

	// The colours are swapped when looking down
	TVector4 vForward = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_FORWARD );
	vForward.w        = 1.0f;
	TASSERT( vForward.IsNormalised() );

	TFLOAT fDot   = s_vWorldUp.x * vForward.x + s_vWorldUp.y * vForward.y + s_vWorldUp.z * vForward.z;
	TFLOAT fBlend = TMath::Abs( fDot );

	TVector4 vColour;

	if ( !( 0.0f < fDot ) )
	{
		vColour.Lerp4( m_oSkyColours.m_vColour1, m_oSkyColours.m_vColour2, fBlend );

		const TVector4& vBottom = m_oSkyColours.m_vColour2;
		m_uiTopColour           = PackColour( vColour.x, vColour.y, vColour.z, m_fAlpha * vColour.w );
		m_uiBottomColour        = PackColour( vBottom.x, vBottom.y, vBottom.z, vBottom.w * m_fAlpha );
	}
	else
	{
		vColour.Lerp4( m_oSkyColours.m_vColour2, m_oSkyColours.m_vColour1, fBlend );

		const TVector4& vTop = m_oSkyColours.m_vColour1;
		m_uiTopColour        = PackColour( vTop.x, vTop.y, vTop.z, vTop.w * m_fAlpha );
		m_uiBottomColour     = PackColour( vColour.x, vColour.y, vColour.z, m_fAlpha * vColour.w );
	}

	pRender->SetVec4InVSBuffer( 0, &mWVP, 4 );

	SkyVertex aVertices[ 4 ];

	for ( TINT i = 0; i < 4; i++ )
	{
		aVertices[ i ].x        = m_avCorners[ i ].x;
		aVertices[ i ].y        = m_avCorners[ i ].y;
		aVertices[ i ].z        = 1.0f;
		aVertices[ i ].uiColour = ( &m_uiTopColour )[ TINT( floor( TFLOAT( i ) * 0.5f ) ) ];
	}

	pRender->DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, aVertices, 4, sizeof( SkyVertex ), 0 );

	pContext->EnableFog( bFog );
}

// $deBlob: FUNCTION 00557c90
void __stdcall ASky::OnPreFlush( TOrderTable* a_pOrderTable, void* a_pSky )
{
	ASky* pSky = TSTATICCAST( ASky, a_pSky );

	if ( pSky->m_iNumRenderCalls > 0 )
	{
		ID3D11DeviceContext* pDeviceContext = TRenderDX11::Interface()->GetDeviceContext();
		pDeviceContext->IASetInputLayout( pSky->m_pInputLayout );
		pDeviceContext->VSSetShader( pSky->m_pVertexShader, TNULL, 0 );
		pDeviceContext->PSSetShader( pSky->m_pPixelShader, TNULL, 0 );
	}

	for ( TINT i = 0; i < pSky->m_iNumRenderCalls; i++ )
		pSky->RenderGradient( &pSky->m_apRenderCalls[ i ] );

	pSky->m_iNumRenderCalls = 0;
}

// $deBlob: FUNCTION 00557d30
void __stdcall ASky::RenderCellMesh( TTMDWin::TRBWorldCellMesh* a_pCellMesh, TWorldVis::RenderData* a_pRenderData )
{
	TWorldMeshHAL* pMesh = TSTATICCAST( TTRBWorldMesh, a_pCellMesh->m_pWorldMesh )->m_pMesh;
	TASSERT( ( (TMesh*)( pMesh ) )->IsA( &TGetClass( TWorldMeshHAL ) ) );

	TBOOL bRes = pMesh->Render();
	TASSERT( bRes );
}
