#include "ToshiPCH.h"
#include "XUI/TXUIRenderer.h"
#include "XUI/TXUI.h"
#include "GUI/T2GUI.h"

#include "TRender_DX11.h"
#include "TRenderContext_DX11.h"
#include "TPrimShader_DX11.h"
#include "TTexture_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START


// The vertices of the indexed lists (drawn immediately)
// $deBlob: FUNCTION 0053f230
// $deBlob: FUNCTION 006d6e30
static TPrimShader::Vertex s_Vertices[ TXUIRenderer::MAXVERTS ];

// $deBlob: FUNCTION 006d5030
TXUIRenderer::TXUIRenderer()
{
	m_pTransforms            = TNULL;
	m_iTransformStackPointer = 0;
	m_bTransformChanged      = TFALSE;
	m_uiOldDepthState        = 0;

	T2GUITransform* pTransforms = TSTATICCAST( T2GUITransform, TMemoryHeap_Malloc( TXUI::ms_pXUIMemoryBlock, TRANSFORMSTACKSIZE * sizeof( T2GUITransform ) ) );

	if ( pTransforms != TNULL )
	{
		for ( TINT i = 0; i < TRANSFORMSTACKSIZE; i++ )
			new ( &pTransforms[ i ] ) T2GUITransform();
	}

	m_pTransforms = pTransforms;
}

// $deBlob: FUNCTION 006d50e0
// $deBlob: FUNCTION 006d6e40
TXUIRenderer::~TXUIRenderer()
{
	operator delete( m_pTransforms );
}

// $deBlob: FUNCTION 006d5100
TUINT TXUIRenderer::GetWidth( T2GUIMaterial* a_pMaterial )
{
	TASSERT( a_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );

	if ( a_pMaterial->GetTextureResource() == TNULL )
		return 0;

	return a_pMaterial->GetTextureResource()->GetWidth();
}

// $deBlob: FUNCTION 006d5160
TUINT TXUIRenderer::GetHeight( T2GUIMaterial* a_pMaterial )
{
	TASSERT( a_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );

	if ( a_pMaterial->GetTextureResource() == TNULL )
		return 0;

	return a_pMaterial->GetTextureResource()->GetHeight();
}

// $deBlob: FUNCTION 006d51c0
void TXUIRenderer::BeginScene()
{
	TASSERT( 0 == m_iTransformStackPointer );

	TRenderDX11*        pRender        = TRenderDX11::Interface();
	TRenderContextDX11* pRenderContext = TSTATICCAST( TRenderContextDX11, pRender->GetCurrentContext() );
	pRender->FlushShaders();

	TRender::DISPLAYPARAMS* pDisplayParams = pRender->GetCurrentDisplayParams();

	TFLOAT fCanvasWidth, fCanvasHeight;
	T2GUI::GetSingleton()->GetRootElement()->GetDimensions( fCanvasWidth, fCanvasHeight );

	// The canvas fills the display, y up
	T2GUITransform& rTransform = m_pTransforms[ m_iTransformStackPointer ];
	rTransform.m_Rot[ 0 ].m_X  = T2GUITransform::Float( TINT( ( TFLOAT( TINT( pDisplayParams->uiWidth ) ) / fCanvasWidth ) * 256.0f ) );
	rTransform.m_Rot[ 0 ].m_Y  = 0;
	rTransform.m_Rot[ 1 ].m_X  = 0;
	rTransform.m_Rot[ 1 ].m_Y  = T2GUITransform::Float( TINT( ( TFLOAT( TINT( pDisplayParams->uiHeight ) ) / fCanvasHeight ) * -256.0f ) );
	rTransform.m_Pos.x         = 0.0f;
	rTransform.m_Pos.y         = 0.0f;
	m_bTransformChanged        = TTRUE;

	static TMatrix44 s_oIdentityMatrix = TMatrix44::IDENTITY;

	pRenderContext->SetModelViewMatrix( s_oIdentityMatrix );
	pRenderContext->SetWorldViewMatrix( s_oIdentityMatrix );

	// NOTE: retail leaves m_fUnk of its (stack) parameters uninitialised, the current value is kept here
	TRenderContext::PROJECTIONPARAMS oProjParams = pRenderContext->GetProjectionParameters();
	oProjParams.m_Proj.x                         = 1.0f;
	oProjParams.m_Proj.y                         = -1.0f;
	oProjParams.m_Centre.x                       = TFLOAT( TINT( pDisplayParams->uiWidth ) ) * 0.5f;
	oProjParams.m_fNearClip                      = 0.0f;
	oProjParams.m_fFarClip                       = 1.0f;
	oProjParams.m_Centre.y                       = TFLOAT( TINT( pDisplayParams->uiHeight ) ) * 0.5f;

	pRenderContext->SetProjType( TRenderContext::CameraMode_Orthographic );
	pRenderContext->SetProjectionParameters( oProjParams );
	pRenderContext->Update();

	SetColour( 0xFFFFFFFF );
}

// $deBlob: FUNCTION 006d53a0
void TXUIRenderer::EndScene()
{
	TASSERT( 0 == m_iTransformStackPointer );
}

// $deBlob: FUNCTION 006d53e0
void TXUIRenderer::SetMaterial( T2GUIMaterial* a_pMaterial )
{
	if ( a_pMaterial != TNULL )
	{
		TASSERT( a_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );

		TTexture* pTexture = a_pMaterial->GetTextureResource();
		TRenderDX11::Interface()->SetSamplerState( 0, a_pMaterial->GetSamplerId(), TRUE );
		TPrimShader::GetSingleton()->SetImageAndUnlock( pTexture );
		m_pMaterial = a_pMaterial;
		return;
	}

	TPrimShader::GetSingleton()->SetImageAndUnlock( TNULL );
	m_pMaterial = TNULL;
}

// $deBlob: FUNCTION 006d5480
void TXUIRenderer::PushTransform( const T2GUITransform& a_rTransform, const TVector2& a_rVec1, const TVector2& a_rVec2 )
{
	T2GUITransform oTransform2;
	T2GUITransform oTransform1;

	m_iTransformStackPointer++;
	TASSERT( m_iTransformStackPointer < TRANSFORMSTACKSIZE );

	T2GUITransform& rOutTransform  = m_pTransforms[ m_iTransformStackPointer ];
	T2GUITransform& rPrevTransform = m_pTransforms[ m_iTransformStackPointer - 1 ];

	oTransform2.m_Rot[ 0 ] = rPrevTransform.m_Rot[ 0 ];
	oTransform2.m_Rot[ 1 ] = rPrevTransform.m_Rot[ 1 ];
	oTransform1.m_Rot[ 0 ] = a_rTransform.m_Rot[ 0 ];
	oTransform1.m_Rot[ 1 ] = a_rTransform.m_Rot[ 1 ];
	rPrevTransform.Transform( oTransform2.m_Pos, a_rVec1 );
	a_rTransform.Transform( oTransform1.m_Pos, a_rVec2 );

	T2GUITransform::Multiply( rOutTransform, oTransform2, oTransform1 );
	m_bTransformChanged = TTRUE;
}

// $deBlob: FUNCTION 006d5690
void TXUIRenderer::PopTransform()
{
	TASSERT( m_iTransformStackPointer > 0 );
	m_iTransformStackPointer--;
	m_bTransformChanged = TTRUE;
}

// $deBlob: FUNCTION 006d56e0
void TXUIRenderer::SetTransform( const T2GUITransform& a_rTransform )
{
	m_pTransforms[ m_iTransformStackPointer ] = a_rTransform;
	m_bTransformChanged                       = TTRUE;
}

// The clip of an element with the depth buffer: the clip shape drawn in between (colour 0) writes the depth, the
// elements are then only drawn where the depth is equal
// $deBlob: FUNCTION 006d5730
void TXUIRenderer::SetScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 )
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	TASSERT( m_uiOldDepthState == 0 );
	m_uiOldDepthState = pRender->GetCurrentDepth().uiDepth;

	pRender->SetZMode( TFALSE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
	SetColour( 0 );
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
}

// $deBlob: FUNCTION 006d57b0
void TXUIRenderer::ClearScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 )
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	pRender->SetZMode( TTRUE, D3D11_COMPARISON_ALWAYS, D3D11_DEPTH_WRITE_MASK_ALL );
	SetColour( 0 );
	pRender->GetCurrentDepth().uiDepth = m_uiOldDepthState;
	m_uiOldDepthState                  = 0;
}

// $deBlob: FUNCTION 006d6b20
void TXUIRenderer::SetViewportRect( TFLOAT a_fLeft, TFLOAT a_fTop, TFLOAT a_fRight, TFLOAT a_fBottom )
{
	TRenderDX11*                    pRender    = TRenderDX11::Interface();
	TRender::DISPLAYPARAMS*         pParams    = pRender->GetCurrentDisplayParams();
	ID3D11DeviceContext*            pContext   = pRender->GetDeviceContext();
	const T2GUITransform&           rTransform = m_pTransforms[ m_iTransformStackPointer ];
	const T2GUITransform::Rotation* pRot       = rTransform.m_Rot;
	const TVector2&                 rPos       = rTransform.GetPos();

	TFLOAT fHalfWidth  = TFLOAT( TINT( pParams->uiWidth ) ) * 0.5f;
	TFLOAT fHalfHeight = TFLOAT( TINT( pParams->uiHeight ) ) * 0.5f;

	TINT iLeft = TMath::FloorToInt( fHalfWidth + pRot[ 1 ].GetX() * a_fTop + pRot[ 0 ].GetX() * a_fLeft + rPos.x );

	if ( iLeft < 0 )
		iLeft = 0;

	TINT iRight = TMath::CeilToInt( fHalfWidth + pRot[ 1 ].GetX() * a_fBottom + pRot[ 0 ].GetX() * a_fRight + rPos.x );

	if ( TINT( pParams->uiWidth ) < iRight )
		iRight = pParams->uiWidth;

	TINT iTop = TMath::CeilToInt( fHalfHeight - ( pRot[ 0 ].GetY() * a_fRight + pRot[ 1 ].GetY() * a_fBottom + rPos.y ) );

	if ( iTop < 0 )
		iTop = 0;

	TINT iBottom = TMath::FloorToInt( fHalfHeight - ( pRot[ 0 ].GetY() * a_fLeft + pRot[ 1 ].GetY() * a_fTop + rPos.y ) );

	if ( TINT( pParams->uiHeight ) < iBottom )
		iBottom = pParams->uiHeight;

	if ( iLeft < iRight && iTop < iBottom )
	{
		D3D11_VIEWPORT oViewport;
		oViewport.TopLeftX = TFLOAT( iLeft );
		oViewport.TopLeftY = TFLOAT( iTop );
		oViewport.Width    = TFLOAT( iRight - iLeft );
		oViewport.Height   = TFLOAT( iBottom - iTop );
		oViewport.MinDepth = 0.0f;
		oViewport.MaxDepth = 1.0f;

		if ( oViewport.Width <= 0.0f )
			oViewport.Width = 1.0f;

		if ( oViewport.Height <= 0.0f )
			oViewport.Height = 1.0f;

		pContext->RSSetViewports( 1, &oViewport );
	}
}

// $deBlob: FUNCTION 006d6d60
void TXUIRenderer::ResetViewportRect()
{
	TRenderDX11*            pRender  = TRenderDX11::Interface();
	ID3D11DeviceContext*    pContext = pRender->GetDeviceContext();
	TRender::DISPLAYPARAMS* pParams  = pRender->GetCurrentDisplayParams();

	D3D11_VIEWPORT oViewport;
	oViewport.TopLeftX = 0.0f;
	oViewport.TopLeftY = 0.0f;
	oViewport.Width    = TFLOAT( TINT( pParams->uiWidth ) );
	oViewport.Height   = TFLOAT( TINT( pParams->uiHeight ) );
	oViewport.MinDepth = 0.0f;
	oViewport.MaxDepth = 1.0f;
	pContext->RSSetViewports( 1, &oViewport );
}

// $deBlob: FUNCTION 006d6de0
// $deBlob: FUNCTION 00550ae0
void TXUIRenderer::UpdateTransform()
{
	TRenderContext* pRenderContext = TRender::GetSingleton()->GetCurrentContext();

	TMatrix44 oMatrix;
	m_pTransforms[ m_iTransformStackPointer ].Matrix44( oMatrix );
	pRenderContext->SetModelViewMatrix( oMatrix );
	pRenderContext->SetWorldViewMatrix( oMatrix );
	m_bTransformChanged = TFALSE;
}

// $deBlob: FUNCTION 006d5800
void TXUIRenderer::RenderRectangle( const TVector2& a_rA, const TVector2& a_rB, const TVector2& a_rUV1, const TVector2& a_rUV2 )
{
	if ( m_bTransformChanged )
		UpdateTransform();

	TPrimShader* pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock( TPrimShader::BlendMode_Alpha );
	pPrimShader->StartRendering( TPrimShader::PrimType_TriangleStrip );

	TPrimShader::Vertex* pVertex;

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = a_rUV1;
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rA.x, a_rA.y, 0.0f );
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2( a_rUV2.x, a_rUV1.y );
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rB.x, a_rA.y, 0.0f );
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2( a_rUV1.x, a_rUV2.y );
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rA.x, a_rB.y, 0.0f );
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = a_rUV2;
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rB.x, a_rB.y, 0.0f );
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
}

// $deBlob: FUNCTION 006d59b0
void TXUIRenderer::RenderTriStrip( TVector2* a_pVertices, TVector2* a_pUVs, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY )
{
	if ( m_bTransformChanged )
		UpdateTransform();

	TINT iNumVerts = TINT( a_uiNumVerts );
	TASSERT( iNumVerts < MAXVERTS );

	TPrimShader* pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock( TPrimShader::BlendMode_Alpha );
	pPrimShader->StartRendering( TPrimShader::PrimType_TriangleStrip );

	for ( TINT i = 0; i < iNumVerts; i++ )
	{
		TPrimShader::Vertex* pVertex = pPrimShader->GetCurrentVertex();
		pVertex->UV                  = ( a_pUVs == TNULL ) ? TVector2( 0.0f, 0.0f ) : a_pUVs[ i ];
		pVertex->Colour              = m_uiColour;
		pVertex->Position            = TVector3( a_pVertices[ i ].x * a_fPosScaleX, a_pVertices[ i ].y * a_fPosScaleY, 0.0f );
		pPrimShader->AddVert();
	}

	pPrimShader->StopRendering();
}

// $deBlob: FUNCTION 006d5b70
void TXUIRenderer::RenderTriStripColours( TVector2* a_pVertices, TVector2* a_pUVs, TUINT32* a_pColours, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY )
{
	if ( m_bTransformChanged )
		UpdateTransform();

	TINT iNumVerts = TINT( a_uiNumVerts );
	TASSERT( iNumVerts < MAXVERTS );

	TPrimShader* pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock( TPrimShader::BlendMode_Alpha );
	pPrimShader->StartRendering( TPrimShader::PrimType_TriangleStrip );

	for ( TINT i = 0; i < iNumVerts; i++ )
	{
		TPrimShader::Vertex* pVertex = pPrimShader->GetCurrentVertex();
		pVertex->UV                  = ( a_pUVs == TNULL ) ? TVector2( 0.0f, 0.0f ) : a_pUVs[ i ];
		pVertex->Colour              = a_pColours[ i ];
		pVertex->Position            = TVector3( a_pVertices[ i ].x * a_fPosScaleX, a_pVertices[ i ].y * a_fPosScaleY, 0.0f );
		pPrimShader->AddVert();
	}

	pPrimShader->StopRendering();
}

// $deBlob: FUNCTION 006d5d20
void TXUIRenderer::RenderLine( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2 )
{
	T2GUIMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( TNULL );

	if ( m_bTransformChanged )
		UpdateTransform();

	TPrimShader* pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock( TPrimShader::BlendMode_Opaque );
	pPrimShader->StartRendering( TPrimShader::PrimType_LineList );

	TPrimShader::Vertex* pVertex;

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2( 0.0f, 0.0f );
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_fX1, a_fY1, 0.0f );
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2( 0.0f, 0.0f );
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_fX2, a_fY2, 0.0f );
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
	SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 006d5e40
void TXUIRenderer::RenderLine( const TVector2& a_rA, const TVector2& a_rB )
{
	T2GUIMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( TNULL );

	if ( m_bTransformChanged )
		UpdateTransform();

	TPrimShader* pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock( TPrimShader::BlendMode_Opaque );
	pPrimShader->StartRendering( TPrimShader::PrimType_LineList );

	TPrimShader::Vertex* pVertex;

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2( 0.0f, 0.0f );
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rA.x, a_rA.y, 0.0f );
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2( 0.0f, 0.0f );
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rB.x, a_rB.y, 0.0f );
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
	SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 006d5f50
void TXUIRenderer::RenderOutlineRectangle( const TVector2& a_rA, const TVector2& a_rB )
{
	T2GUIMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( TNULL );

	if ( m_bTransformChanged )
		UpdateTransform();

	TPrimShader* pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock( TPrimShader::BlendMode_Opaque );
	pPrimShader->StartRendering( TPrimShader::PrimType_LineStrip );

	// NOTE: retail's strip is A, B, (A.x, B.y), A: the corner (B.x, A.y) is missing
	const TVector3 aPositions[ 4 ] = {
		TVector3( a_rA.x, a_rA.y, 0.0f ),
		TVector3( a_rB.x, a_rB.y, 0.0f ),
		TVector3( a_rA.x, a_rB.y, 0.0f ),
		TVector3( a_rA.x, a_rA.y, 0.0f ),
	};

	for ( TINT i = 0; i < 4; i++ )
	{
		TPrimShader::Vertex* pVertex = pPrimShader->GetCurrentVertex();
		pVertex->UV                  = TVector2( 0.0f, 0.0f );
		pVertex->Colour              = m_uiColour;
		pVertex->Position            = aPositions[ i ];
		pPrimShader->AddVert();
	}

	pPrimShader->StopRendering();
	SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 006d6100
void TXUIRenderer::RenderFilledRectangle( const TVector2& a_rA, const TVector2& a_rB )
{
	T2GUIMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( TNULL );

	if ( m_bTransformChanged )
		UpdateTransform();

	TPrimShader* pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock( TPrimShader::BlendMode_Opaque );
	pPrimShader->StartRendering( TPrimShader::PrimType_TriangleStrip );

	const TVector3 aPositions[ 4 ] = {
		TVector3( a_rA.x, a_rA.y, 0.0f ),
		TVector3( a_rB.x, a_rA.y, 0.0f ),
		TVector3( a_rA.x, a_rB.y, 0.0f ),
		TVector3( a_rB.x, a_rB.y, 0.0f ),
	};

	for ( TINT i = 0; i < 4; i++ )
	{
		TPrimShader::Vertex* pVertex = pPrimShader->GetCurrentVertex();
		pVertex->UV                  = TVector2( 0.0f, 0.0f );
		pVertex->Colour              = m_uiColour;
		pVertex->Position            = aPositions[ i ];
		pPrimShader->AddVert();
	}

	pPrimShader->StopRendering();
	SetMaterial( pOldMaterial );
}

// Binds the prim shader's vertex shader and input layout with the world view projection of the render context
static void SetupImmediateDraw( TRenderDX11* a_pRender, TPrimShader* a_pPrimShader )
{
	TRenderContextDX11* pRenderContext = TSTATICCAST( TRenderContextDX11, a_pRender->GetCurrentContext() );
	TMatrix44           oProjection    = pRenderContext->GetProjectionMatrix();
	TMatrix44           oModelView     = pRenderContext->GetModelViewMatrix();

	TMatrix44 oWorldViewProjection;
	oWorldViewProjection.Multiply( oProjection, oModelView );
	a_pRender->SetVec4InVSBuffer( 0, &oWorldViewProjection, 4 );

	a_pRender->GetDeviceContext()->IASetInputLayout( a_pPrimShader->GetInputLayout() );

	// Cull mode: D3D11_CULL_NONE
	TUINT8& rRasterizerFlags = TREINTERPRETCAST( TUINT8*, &a_pRender->GetCurrentRasterizer().uiFlags )[ 0 ];
	rRasterizerFlags         = ( rRasterizerFlags & 0xF7 ) | ( D3D11_CULL_NONE << TRenderDX11::RASTERIZERFLAGS_CULLSHIFT );
}

// $deBlob: FUNCTION 006d62b0
void TXUIRenderer::RenderIndexedTriList( TVector2* a_pVertices, TVector2* a_pUVs, void* a_pIndices, TUINT32 a_uiNumIndices, TINT a_iIndexSize, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY )
{
	if ( m_bTransformChanged )
		UpdateTransform();

	TINT iNumVerts = TINT( a_uiNumVerts );
	TASSERT( iNumVerts < MAXVERTS );

	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();

	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );

	TPrimShader* pPrimShader = TPrimShader::GetSingleton();
	SetupImmediateDraw( pRender, pPrimShader );

	TASSERT( TRender::GetSingleton()->IsInScene() );
	pDeviceContext->VSSetShader( pPrimShader->GetVertexShader(), TNULL, 0 );

	for ( TINT i = 0; i < iNumVerts; i++ )
	{
		s_Vertices[ i ].Position = TVector3( a_pVertices[ i ].x * a_fPosScaleX, a_pVertices[ i ].y * a_fPosScaleY, 0.0f );
		s_Vertices[ i ].Colour   = m_uiColour;
		s_Vertices[ i ].UV       = ( a_pUVs == TNULL ) ? TVector2::VEC_ZERO : a_pUVs[ i ];
	}

	if ( m_pMaterial == TNULL )
	{
		pDeviceContext->PSSetShader( pPrimShader->GetColourPixelShader(), TNULL, 0 );
	}
	else
	{
		TASSERT( TTRUE == m_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );
		m_pMaterial->GetTextureResource()->Bind( 0 );
		pRender->GetDeviceContext()->PSSetShader( pPrimShader->GetImagePixelShader(), TNULL, 0 );
	}

	// NOTE: retail passes the index pointer as the format of an index size other than 2 and 4
	DXGI_FORMAT eFormat = ( a_iIndexSize == 2 ) ? DXGI_FORMAT_R16_UINT : ( a_iIndexSize == 4 ) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT( TUINTPTR( a_pIndices ) );
	pRender->DrawImmediately( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, a_uiNumIndices, a_pIndices, eFormat, s_Vertices, sizeof( TPrimShader::Vertex ), a_uiNumVerts );

	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO );
}

// $deBlob: FUNCTION 006d66d0
void TXUIRenderer::RenderIndexedTriStripColoursList( TVector2* a_pVertices, TVector2* a_pUVs, TUINT32* a_pColours, void* a_pIndices, TUINT32 a_uiNumIndices, TINT a_iIndexSize, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY )
{
	if ( m_bTransformChanged )
		UpdateTransform();

	TINT iNumVerts = TINT( a_uiNumVerts );
	TASSERT( iNumVerts < MAXVERTS );

	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();
	TPrimShader*         pPrimShader    = TPrimShader::GetSingleton();
	SetupImmediateDraw( pRender, pPrimShader );

	TASSERT( TRender::GetSingleton()->IsInScene() );
	pDeviceContext->VSSetShader( pPrimShader->GetVertexShader(), TNULL, 0 );

	for ( TINT i = 0; i < iNumVerts; i++ )
	{
		s_Vertices[ i ].Position = TVector3( a_pVertices[ i ].x * a_fPosScaleX, a_pVertices[ i ].y * a_fPosScaleY, 0.0f );
		s_Vertices[ i ].Colour   = a_pColours[ i ];
		s_Vertices[ i ].UV       = ( a_pUVs == TNULL ) ? TVector2::VEC_ZERO : a_pUVs[ i ];
	}

	if ( m_pMaterial == TNULL )
	{
		pDeviceContext->PSSetShader( pPrimShader->GetColourPixelShader(), TNULL, 0 );
	}
	else
	{
		TASSERT( TTRUE == m_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );
		m_pMaterial->GetTextureResource()->Bind( 0 );
		pRender->GetDeviceContext()->PSSetShader( pPrimShader->GetImagePixelShader(), TNULL, 0 );
	}

	// NOTE: retail passes the index pointer as the format of an index size other than 2 and 4
	DXGI_FORMAT eFormat = ( a_iIndexSize == 2 ) ? DXGI_FORMAT_R16_UINT : ( a_iIndexSize == 4 ) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT( TUINTPTR( a_pIndices ) );
	pRender->DrawImmediately( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, a_uiNumIndices, a_pIndices, eFormat, s_Vertices, sizeof( TPrimShader::Vertex ), a_uiNumVerts );
}

// Converts display coordinates to canvas coordinates
// $deBlob: FUNCTION 006d6ab0
void TXUIRenderer::ScaleCoords( TFLOAT& a_rX, TFLOAT& a_rY )
{
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

	TFLOAT fCanvasWidth, fCanvasHeight;
	T2GUI::GetSingleton()->GetRootElement()->GetDimensions( fCanvasWidth, fCanvasHeight );

	TINT iHeight = TINT( pDisplayParams->uiHeight );
	a_rX         = a_rX / ( TFLOAT( TINT( pDisplayParams->uiWidth ) ) / fCanvasWidth );
	a_rY         = a_rY / ( TFLOAT( iHeight ) / fCanvasHeight );
}

// $deBlob: FUNCTION 006d6eb0
void TXUIRenderer::SetColour( TUINT32 a_uiColour )
{
	m_uiColour = a_uiColour;
}

TOSHI_NAMESPACE_END
