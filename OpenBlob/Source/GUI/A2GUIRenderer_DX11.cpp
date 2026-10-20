#include "pch.h"
#include "A2GUIRenderer_DX11.h"
#include "Memory/AMemory.h"

#include <GUI/T2GUI.h>
#include <Platform/DX11/TRender_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>
#include <Platform/DX11/TPrimShader_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0054f600
A2GUIRenderer::A2GUIRenderer()
{
	m_pTransforms            = TNULL;
	m_iTransformStackPointer = 0;
	m_bTransformChanged      = TFALSE;
	m_pMaterial              = TNULL;

	T2GUITransform* pTransforms = TSTATICCAST( T2GUITransform, TMemoryHeap_Malloc( AMemory::GetPool( AMemory::POOL_FrequentAllocations ), TRANSFORMSTACKSIZE * sizeof( T2GUITransform ) ) );

	if ( pTransforms != TNULL )
	{
		for ( TINT i = 0; i < TRANSFORMSTACKSIZE; i++ )
			new ( &pTransforms[ i ] ) T2GUITransform();
	}

	m_pTransforms = pTransforms;
}

// $deBlob: FUNCTION 00550c10
// $deBlob: FUNCTION 0054f6f0
A2GUIRenderer::~A2GUIRenderer()
{
	delete[] m_pTransforms;
}

// $deBlob: FUNCTION 0054f760
T2GUIMaterial* A2GUIRenderer::CreateMaterial( const TCHAR* a_szTextureName )
{
	return CreateMaterial( GetTexture( a_szTextureName ) );
}

// $deBlob: FUNCTION 0054f710
T2GUIMaterial* A2GUIRenderer::CreateMaterial( TTexture* a_pTexture )
{
	// NOTE: retail goes on with a TNULL material when the allocation fails
	T2GUIMaterial* pMaterial = new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) T2GUIMaterial();
	pMaterial->Create();
	pMaterial->SetTextureResource( a_pTexture );

	return pMaterial;
}

// $deBlob: FUNCTION 0054f780
void A2GUIRenderer::DestroyMaterial( T2GUIMaterial* a_pMaterial )
{
	if ( a_pMaterial != TNULL )
	{
		TASSERT( a_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );
		a_pMaterial->OnDestroy();
		delete a_pMaterial;
	}
}

// $deBlob: FUNCTION 0054f7f0
TTexture* A2GUIRenderer::GetTexture( const TCHAR* a_szTextureName )
{
	TCHAR szLowerCaseTexName[ 512 ];
	TASSERT( T2String8::Length( a_szTextureName ) < sizeof( szLowerCaseTexName ) );

	TStringManager::String8Copy( szLowerCaseTexName, a_szTextureName, -1 );
	TStringManager::String8ToLowerCase( szLowerCaseTexName );

	// The file name: after the last '\' or '/'
	TINT iStart = TStringManager::String8Length( szLowerCaseTexName );

	while ( TTRUE )
	{
		TINT iIndex = iStart - 1;

		if ( iIndex < 0 )
		{
			iStart = 0;
			break;
		}

		if ( szLowerCaseTexName[ iIndex ] == '\\' || szLowerCaseTexName[ iIndex ] == '/' )
			break;

		iStart = iIndex;
	}

	return TTextureManager::GetSingleton()->FindTexture( szLowerCaseTexName + iStart );
}

// $deBlob: FUNCTION 0054f8b0
TUINT A2GUIRenderer::GetWidth( T2GUIMaterial* a_pMaterial )
{
	TASSERT( a_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );

	if ( a_pMaterial->GetTextureResource() == TNULL )
		return 0;

	return a_pMaterial->GetTextureResource()->GetWidth();
}

// $deBlob: FUNCTION 0054f910
TUINT A2GUIRenderer::GetHeight( T2GUIMaterial* a_pMaterial )
{
	TASSERT( a_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );

	if ( a_pMaterial->GetTextureResource() == TNULL )
		return 0;

	return a_pMaterial->GetTextureResource()->GetHeight();
}

// $deBlob: FUNCTION 0054f970
void A2GUIRenderer::BeginScene()
{
	TASSERT( 0 == m_iTransformStackPointer );

	TRenderDX11*        pRender        = TRenderDX11::Interface();
	TRenderContextDX11* pRenderContext = TSTATICCAST( TRenderContextDX11, pRender->GetCurrentContext() );
	pRender->FlushShaders();

	TRender::DISPLAYPARAMS* pDisplayParams = pRender->GetCurrentDisplayParams();
	m_bTransformChanged                    = TTRUE;

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

// $deBlob: FUNCTION 0054fad0
void A2GUIRenderer::EndScene()
{
	TASSERT( 0 == m_iTransformStackPointer );
}

// $deBlob: FUNCTION 0054fb10
void A2GUIRenderer::SetMaterial( T2GUIMaterial* a_pMaterial )
{
	if ( a_pMaterial != TNULL )
	{
		TASSERT( a_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );
		TPrimShader::GetSingleton()->SetImageAndUnlock( a_pMaterial->GetTextureResource() );
		m_pMaterial = a_pMaterial;
		return;
	}

	TPrimShader::GetSingleton()->SetImageAndUnlock( TNULL );
	m_pMaterial = TNULL;
}

// $deBlob: FUNCTION 0054fb90
void A2GUIRenderer::PushTransform( const T2GUITransform& a_rTransform, const TVector2& a_rVec1, const TVector2& a_rVec2 )
{
	T2GUITransform oTransform1;
	T2GUITransform oTransform2;

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

// $deBlob: FUNCTION 0054fda0
void A2GUIRenderer::PopTransform()
{
	TASSERT( m_iTransformStackPointer > 0 );
	m_iTransformStackPointer--;
	m_bTransformChanged = TTRUE;
}

// $deBlob: FUNCTION 0054fdf0
void A2GUIRenderer::SetTransform( const T2GUITransform& a_rTransform )
{
	m_pTransforms[ m_iTransformStackPointer ] = a_rTransform;
	m_bTransformChanged                       = TTRUE;
}

// $deBlob: FUNCTION 00550fb0
void A2GUIRenderer::SetColour( TUINT32 a_uiColour )
{
	m_uiColour = a_uiColour;
}

void A2GUIRenderer::UpdateTransform()
{
	TRenderContext* pRenderContext = TRender::GetSingleton()->GetCurrentContext();

	TMatrix44 oMatrix;
	m_pTransforms[ m_iTransformStackPointer ].Matrix44( oMatrix );
	pRenderContext->SetModelViewMatrix( oMatrix );
	pRenderContext->SetWorldViewMatrix( oMatrix );
	m_bTransformChanged = TFALSE;
}

// NOTE: the vertices of A2GUIRenderer only set what retail sets (the UVs of the lines and the untextured
// rectangles are whatever the vertex buffer holds)

// $deBlob: FUNCTION 0054fe40
void A2GUIRenderer::RenderRectangle( const TVector2& a_rA, const TVector2& a_rB, const TVector2& a_rUV1, const TVector2& a_rUV2 )
{
	if ( m_bTransformChanged )
		UpdateTransform();

	TPrimShader::GetSingleton()->StartRendering( TPrimShader::PrimType_TriangleStrip );

	TPrimShader::Vertex* pVertex;

	pVertex           = TPrimShader::GetSingleton()->GetCurrentVertex();
	pVertex->UV       = a_rUV1;
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rA.x, a_rA.y, 0.0f );
	TPrimShader::GetSingleton()->AddVert();

	pVertex           = TPrimShader::GetSingleton()->GetCurrentVertex();
	pVertex->UV       = TVector2( a_rUV2.x, a_rUV1.y );
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rB.x, a_rA.y, 0.0f );
	TPrimShader::GetSingleton()->AddVert();

	pVertex           = TPrimShader::GetSingleton()->GetCurrentVertex();
	pVertex->UV       = TVector2( a_rUV1.x, a_rUV2.y );
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rA.x, a_rB.y, 0.0f );
	TPrimShader::GetSingleton()->AddVert();

	pVertex           = TPrimShader::GetSingleton()->GetCurrentVertex();
	pVertex->UV       = a_rUV2;
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rB.x, a_rB.y, 0.0f );
	TPrimShader::GetSingleton()->AddVert();

	TPrimShader::GetSingleton()->StopRendering();
}

// NOTE: retail ignores the scales and needs the UVs
// $deBlob: FUNCTION 0054ffe0
void A2GUIRenderer::RenderTriStrip( TVector2* a_pVertices, TVector2* a_pUVs, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY )
{
	if ( m_bTransformChanged )
		UpdateTransform();

	TINT iNumVerts = TINT( a_uiNumVerts );
	TASSERT( iNumVerts < MAXVERTS );

	TPrimShader* pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->StartRendering( TPrimShader::PrimType_TriangleStrip );

	for ( TINT i = 0; i < iNumVerts; i++ )
	{
		TPrimShader::Vertex* pVertex = pPrimShader->GetCurrentVertex();
		pVertex->UV                  = a_pUVs[ i ];
		pVertex->Colour              = m_uiColour;
		pVertex->Position            = TVector3( a_pVertices[ i ].x, a_pVertices[ i ].y, 0.0f );
		pPrimShader->AddVert();
	}

	pPrimShader->StopRendering();
}

// $deBlob: FUNCTION 00550210
void A2GUIRenderer::RenderLine( const TVector2& a_rA, const TVector2& a_rB )
{
	T2GUIMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( TNULL );

	if ( m_bTransformChanged )
		UpdateTransform();

	TPrimShader::GetSingleton()->StartRendering( TPrimShader::PrimType_LineList );

	TPrimShader::Vertex* pVertex;

	pVertex           = TPrimShader::GetSingleton()->GetCurrentVertex();
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rA.x, a_rA.y, 0.0f );
	TPrimShader::GetSingleton()->AddVert();

	pVertex           = TPrimShader::GetSingleton()->GetCurrentVertex();
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_rB.x, a_rB.y, 0.0f );
	TPrimShader::GetSingleton()->AddVert();

	TPrimShader::GetSingleton()->StopRendering();
	SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 00550100
void A2GUIRenderer::RenderLine( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2 )
{
	T2GUIMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( TNULL );

	if ( m_bTransformChanged )
		UpdateTransform();

	TPrimShader::GetSingleton()->StartRendering( TPrimShader::PrimType_LineList );

	TPrimShader::Vertex* pVertex;

	pVertex           = TPrimShader::GetSingleton()->GetCurrentVertex();
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_fX1, a_fY1, 0.0f );
	TPrimShader::GetSingleton()->AddVert();

	pVertex           = TPrimShader::GetSingleton()->GetCurrentVertex();
	pVertex->Colour   = m_uiColour;
	pVertex->Position = TVector3( a_fX2, a_fY2, 0.0f );
	TPrimShader::GetSingleton()->AddVert();

	TPrimShader::GetSingleton()->StopRendering();
	SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 00550320
void A2GUIRenderer::RenderOutlineRectangle( const TVector2& a_rA, const TVector2& a_rB )
{
	T2GUIMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( TNULL );

	if ( m_bTransformChanged )
		UpdateTransform();

	TPrimShader::GetSingleton()->StartRendering( TPrimShader::PrimType_LineStrip );

	// NOTE: retail's strip is A, B, (A.x, B.y), A: the corner (B.x, A.y) is missing
	const TVector3 aPositions[ 4 ] = {
		TVector3( a_rA.x, a_rA.y, 0.0f ),
		TVector3( a_rB.x, a_rB.y, 0.0f ),
		TVector3( a_rA.x, a_rB.y, 0.0f ),
		TVector3( a_rA.x, a_rA.y, 0.0f ),
	};

	for ( TINT i = 0; i < 4; i++ )
	{
		TPrimShader::Vertex* pVertex = TPrimShader::GetSingleton()->GetCurrentVertex();
		pVertex->Colour              = m_uiColour;
		pVertex->Position            = aPositions[ i ];
		TPrimShader::GetSingleton()->AddVert();
	}

	TPrimShader::GetSingleton()->StopRendering();
	SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 005504c0
void A2GUIRenderer::RenderFilledRectangle( const TVector2& a_rA, const TVector2& a_rB )
{
	T2GUIMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( TNULL );

	if ( m_bTransformChanged )
		UpdateTransform();

	TPrimShader* pPrimShader = TPrimShader::GetSingleton();
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
		pVertex->Colour              = m_uiColour;
		pVertex->Position            = aPositions[ i ];
		pPrimShader->AddVert();
	}

	pPrimShader->StopRendering();
	SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 00550e40
void A2GUIRenderer::RenderIndexedTriList( TVector2* a_pVertices, TVector2* a_pUVs, void* a_pIndices, TUINT32 a_uiNumIndices, TINT a_iIndexSize, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 00550e70
void A2GUIRenderer::RenderIndexedTriStripColoursList( TVector2* a_pVertices, TVector2* a_pUVs, TUINT32* a_pColours, void* a_pIndices, TUINT32 a_uiNumIndices, TINT a_iIndexSize, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 00550a70
void A2GUIRenderer::ScaleCoords( TFLOAT& a_rX, TFLOAT& a_rY )
{
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

	TVector2 vCanvas;
	T2GUI::GetSingleton()->GetRootElement()->GetDimensions( vCanvas.x, vCanvas.y );

	TFLOAT fScaleX = TFLOAT( TINT( pDisplayParams->uiWidth ) ) / vCanvas.x;
	TFLOAT fScaleY = TFLOAT( TINT( pDisplayParams->uiHeight ) ) / vCanvas.y;
	a_rX           = a_rX / fScaleX;
	a_rY           = a_rY / fScaleY;
}

// Clips with the viewport: the rectangle (in the space of the top transform) becomes the viewport, the projection
// keeps the display coordinates
// $deBlob: FUNCTION 00550660
void A2GUIRenderer::SetScissor( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2 )
{
	TRenderContext*         pRenderContext = TRender::GetSingleton()->GetCurrentContext();
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

	const T2GUITransform& rTransform     = m_pTransforms[ m_iTransformStackPointer ];
	TINT                  iDisplayWidth  = TINT( pDisplayParams->uiWidth );
	TINT                  iDisplayHeight = TINT( pDisplayParams->uiHeight );

	TINT iLeft = TMath::FloorToInt( TFLOAT( iDisplayWidth ) * 0.5f + rTransform.m_Rot[ 1 ].GetX() * a_fY1 + rTransform.m_Rot[ 0 ].GetX() * a_fX1 + rTransform.m_Pos.x );
	if ( iLeft < 0 ) iLeft = 0;

	TINT iRight = TMath::CeilToInt( TFLOAT( iDisplayWidth ) * 0.5f + rTransform.m_Rot[ 1 ].GetX() * a_fY2 + rTransform.m_Rot[ 0 ].GetX() * a_fX2 + rTransform.m_Pos.x );
	if ( TINT( pDisplayParams->uiWidth ) < iRight ) iRight = TINT( pDisplayParams->uiWidth );

	TINT iTop = TMath::CeilToInt( TFLOAT( iDisplayHeight ) * 0.5f - ( rTransform.m_Rot[ 0 ].GetY() * a_fX2 + rTransform.m_Rot[ 1 ].GetY() * a_fY2 + rTransform.m_Pos.y ) );
	if ( iTop < 0 ) iTop = 0;

	TINT iBottom = TMath::FloorToInt( TFLOAT( iDisplayHeight ) * 0.5f - ( rTransform.m_Rot[ 0 ].GetY() * a_fX1 + rTransform.m_Rot[ 1 ].GetY() * a_fY1 + rTransform.m_Pos.y ) );
	if ( TINT( pDisplayParams->uiHeight ) < iBottom ) iBottom = TINT( pDisplayParams->uiHeight );

	D3D11_VIEWPORT oViewport;
	oViewport.TopLeftX = TFLOAT( iLeft );
	oViewport.TopLeftY = TFLOAT( iTop );
	oViewport.Width    = TFLOAT( iRight - iLeft );
	oViewport.Height   = TFLOAT( iBottom - iTop );
	oViewport.MinDepth = 0.0f;
	oViewport.MaxDepth = 1.0f;

	if ( oViewport.Width <= 0.0f ) oViewport.Width = 1.0f;
	if ( oViewport.Height <= 0.0f ) oViewport.Height = 1.0f;

	TRenderDX11::Interface()->GetDeviceContext()->RSSetViewports( 1, &oViewport );

	pRenderContext->SetModelViewMatrix( TMatrix44::IDENTITY );
	pRenderContext->SetWorldViewMatrix( TMatrix44::IDENTITY );

	// NOTE: retail leaves m_fUnk of its (stack) parameters uninitialised, the current value is kept here
	TRenderContext::PROJECTIONPARAMS oProjParams = pRenderContext->GetProjectionParameters();
	oProjParams.m_Centre.x                       = oViewport.Width * 0.5f;
	oProjParams.m_fNearClip                      = 0.0f;
	oProjParams.m_fFarClip                       = 1.0f;
	oProjParams.m_Proj.x                         = oViewport.Width / TFLOAT( TINT( pDisplayParams->uiWidth ) );
	oProjParams.m_Centre.y                       = oViewport.Height * 0.5f;
	oProjParams.m_Proj.y                         = -( oViewport.Height / TFLOAT( TINT( pDisplayParams->uiHeight ) ) );

	pRenderContext->SetProjType( TRenderContext::CameraMode_Orthographic );
	pRenderContext->SetProjectionParameters( oProjParams );
	pRenderContext->Update();
}

// $deBlob: FUNCTION 00550940
void A2GUIRenderer::ClearScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 )
{
	TRenderContext*         pRenderContext = TRender::GetSingleton()->GetCurrentContext();
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

	D3D11_VIEWPORT oViewport;
	oViewport.TopLeftX = 0.0f;
	oViewport.TopLeftY = 0.0f;
	oViewport.Width    = TFLOAT( TINT( pDisplayParams->uiWidth ) );
	oViewport.Height   = TFLOAT( TINT( pDisplayParams->uiHeight ) );
	oViewport.MinDepth = 0.0f;
	oViewport.MaxDepth = 1.0f;
	TRenderDX11::Interface()->GetDeviceContext()->RSSetViewports( 1, &oViewport );

	pRenderContext->SetModelViewMatrix( TMatrix44::IDENTITY );
	pRenderContext->SetWorldViewMatrix( TMatrix44::IDENTITY );

	// NOTE: retail leaves m_fUnk of its (stack) parameters uninitialised, the current value is kept here
	TRenderContext::PROJECTIONPARAMS oProjParams = pRenderContext->GetProjectionParameters();
	oProjParams.m_fNearClip                      = 0.0f;
	oProjParams.m_fFarClip                       = 1.0f;
	oProjParams.m_Proj.x                         = oViewport.Width / TFLOAT( TINT( pDisplayParams->uiWidth ) );
	oProjParams.m_Centre.x                       = oViewport.Width * 0.5f;
	oProjParams.m_Centre.y                       = oViewport.Height * 0.5f;
	oProjParams.m_Proj.y                         = -( oViewport.Height / TFLOAT( TINT( pDisplayParams->uiHeight ) ) );

	pRenderContext->SetProjType( TRenderContext::CameraMode_Orthographic );
	pRenderContext->SetProjectionParameters( oProjParams );
	pRenderContext->Update();
}
