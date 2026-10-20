#include "pch.h"
#include "ASunFlare.h"
#include "Cameras/ACameraManager.h"

#include <Toshi/TScheduler.h>
#include <Toshi/TSystem.h>
#include <Math/TQuaternion.h>
#include <Math/TMatrix44.h>
#include <Render/TRender.h>
#include <Render/TRenderContext.h>
#include <Shaders/SysShader/TSysShader.h>
#include <Platform/DX11/TSysShader_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// A vertex of the system shader's meshes (0x18 bytes)
struct SunFlareVertex
{
	TVector3 vPosition; // 0x00
	TUINT32  uiColour;  // 0x0C
	TVector2 vUV;       // 0x10
};

TSTATICASSERT( sizeof( SunFlareVertex ) == 0x18 );

// The order table of the blend mode 7 materials (TSysShaderHAL + 0xAC)
static TCONSTEXPR_VAR TINT SUNFLARE_ORDERTABLE = 4;

// The flags, vertices and indices of the meshes
static TCONSTEXPR_VAR TSysMesh::FLAGS SUNFLARE_MESH_FLAGS       = TSysMesh::FLAGS_VERTICES_DYNAMIC | TSysMesh::FLAGS_INDICES_DYNAMIC | TSysMesh::FLAGS_UNK6;
static TCONSTEXPR_VAR TUINT16         SUNFLARE_MESH_MAXVERTICES = 40;
static TCONSTEXPR_VAR TUINT16         SUNFLARE_MESH_MAXINDICES  = 60;

// The segments of the corona
static TCONSTEXPR_VAR TINT iNumSegments = 10;

// The offset of the inner ring of the corona along the projected direction
static TFLOAT s_fCoronaInnerOffset = -0.0f;

// $deBlob: FUNCTION 005fce50
void __stdcall ASunFlare::PreFlushCallback( TOrderTable* a_pOrderTable, void* a_pCustomData )
{
	TASSERT( TFALSE );
	TRender::GetSingleton()->GetCurrentContext();
}

// $deBlob: FUNCTION 005fce90
void __stdcall ASunFlare::PostFlushCallback( TOrderTable* a_pOrderTable, void* a_pCustomData )
{
}

// $deBlob: FUNCTION 005fc0d0
void __stdcall ASunFlare::CreateMaterial( TSysMaterial** a_ppMaterial, TTexture** a_ppTexture )
{
	if ( *a_ppMaterial == TNULL )
	{
		TSysMaterial* pMaterial = TSysShader::GetSingleton()->CreateMaterial( TNULL );
		*a_ppMaterial           = pMaterial;
		pMaterial->Create( TSysMaterial::BLENDMODE_6 );
		( *a_ppMaterial )->SetTextureNum( 1 );
		( *a_ppMaterial )->SetTexture( 0, *a_ppTexture );
	}
}

// $deBlob: FUNCTION 005fc110
void __stdcall ASunFlare::CreateMesh( TSysMesh** a_ppMesh, TSysMaterial* a_pMaterial )
{
	TSysShader* pSysShader = TSysShader::GetSingleton();
	TASSERT( TNULL == *a_ppMesh && "TNULL == a_pMesh" );

	TSysMesh* pMesh = pSysShader->CreateMesh( TNULL );
	*a_ppMesh       = pMesh;

	TBOOL bRes = pMesh->Create( SUNFLARE_MESH_FLAGS, SUNFLARE_MESH_MAXVERTICES, SUNFLARE_MESH_MAXINDICES );
	TASSERT( TTRUE == bRes );

	( *a_ppMesh )->SetMaterial( a_pMaterial );
}

// $deBlob: FUNCTION 005fbcd0
ASunFlare::ASunFlare()
{
	TQuaternion qRotation( -0.0462438f, -0.477332f, -0.873416f, -0.0846163f );

	m_pSunMesh        = TNULL;
	m_pSunMaterial    = TNULL;
	m_pCoronaMesh     = TNULL;
	m_pCoronaMaterial = TNULL;
	m_bEnabled        = TTRUE;
	m_bVisible        = TFALSE;
	m_bInFront        = TFALSE;
	m_fSunSizeX       = 0.1f;
	m_fSunSizeY       = 0.2f;
	m_fFadeTimer      = 0.0f;
	m_fMagnitude      = 1.0f;
	m_fFadeTime       = 0.15f;
	m_fFadeRate       = 1.0f / 0.15f;
	m_fMaxInFrontTime = 0.8f;
	m_fUnk5C          = 1.25f;

	qRotation /= qRotation.Magnitude();
	TQuaternion::RotateVector( m_vDir.AsVector3(), qRotation, TVector3::VEC_POSZ );
	m_vDir.w  = 0.0f;
	m_uiUnk6C = 0;
	m_vUnk10  = m_vDir;
	m_vUnk20  = m_vDir;

	TTextureManager* pTextureManager = TTextureManager::GetSingleton();
	TTexture*        pSunTexture     = pTextureManager->FindTexture( "sun.tga" );
	TTexture*        pCoronaTexture  = pTextureManager->FindTexture( "corona.tga" );

	CreateMaterial( &m_pSunMaterial, &pSunTexture );
	CreateMaterial( &m_pCoronaMaterial, &pCoronaTexture );
	m_pCoronaMaterial->SetBlendMode( TSysMaterial::BLENDMODE_7 );

	CreateMesh( &m_pSunMesh, m_pSunMaterial );
	CreateMesh( &m_pCoronaMesh, m_pCoronaMaterial );

	TSysShaderHAL* pSysShader = TSTATICCAST( TSysShaderHAL, TSysShader::GetSingleton() );
	pSysShader->GetOrderTable( SUNFLARE_ORDERTABLE )->SetPreFlushCallback( PreFlushCallback, TNULL );
	pSysShader->GetOrderTable( SUNFLARE_ORDERTABLE )->SetPostFlushCallback( PostFlushCallback, TNULL );
}

// $deBlob: FUNCTION 005fbe80
ASunFlare::~ASunFlare()
{
	TSysShaderHAL* pSysShader = TSTATICCAST( TSysShaderHAL, TSysShader::GetSingleton() );
	pSysShader->GetOrderTable( SUNFLARE_ORDERTABLE )->SetPreFlushCallback( TNULL, TNULL );
	pSysShader->GetOrderTable( SUNFLARE_ORDERTABLE )->SetPostFlushCallback( TNULL, TNULL );

	m_pSunMaterial->Destroy();
	m_pCoronaMaterial->Destroy();

	m_pSunMesh->DestroyResource();
	m_pCoronaMesh->DestroyResource();
}

// $deBlob: FUNCTION 005fc300
TBOOL __stdcall ASunFlare::ProjectToScreen( TVector4* a_pOutScreenPos, const TVector4* a_pDir, TBOOL* a_pOutInFront, TINT a_iUnused, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();
	TVector4        vPos     = *a_pDir;
	ACameraManager::GetSingleton()->GetCurrentCamera();

	vPos.Multiply( 180.0f );
	a_pOutScreenPos->z = 0.0f;
	a_pOutScreenPos->w = 1.0f;
	TMatrix44::RotateVector( *a_pOutScreenPos, pContext->GetWorldViewMatrix(), vPos );

	const TRenderContext::PROJECTIONPARAMS& rProjParams = pContext->GetProjectionParameters();

	if ( rProjParams.m_fNearClip <= a_pOutScreenPos->z )
	{
		TFLOAT fInvZ       = 1.0f / a_pOutScreenPos->z;
		a_pOutScreenPos->x = a_pOutScreenPos->x * fInvZ * rProjParams.m_Proj.x + rProjParams.m_Centre.x;
		a_pOutScreenPos->z = 1.0f;
		a_pOutScreenPos->w = 1.0f;
		a_pOutScreenPos->y = a_pOutScreenPos->y * fInvZ * rProjParams.m_Proj.y + rProjParams.m_Centre.y;

		// NOTE: retail never sets it back to FALSE
		*a_pOutInFront = TTRUE;

		return -( a_fWidth * 0.5f ) < a_pOutScreenPos->x && -( a_fHeight * 0.5f ) < a_pOutScreenPos->y &&
		    a_pOutScreenPos->x < pContext->GetWidth() + a_fWidth * 0.5f && a_pOutScreenPos->y < pContext->GetHeight() + a_fHeight * 0.5f;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005fc1b0
void ASunFlare::Update( TFLOAT a_fDeltaTime )
{
	TVector4 vScreenPos;
	TBOOL    bOnScreen = ProjectToScreen( &vScreenPos, &m_vDir, &m_bInFront, 1, 100.0f, 100.0f );
	m_bVisible         = bOnScreen;

	if ( m_bInFront == TTRUE )
	{
		m_fInFrontTime += a_fDeltaTime;

		if ( m_fMaxInFrontTime <= m_fInFrontTime )
			m_fInFrontTime = m_fMaxInFrontTime;
	}
	else
	{
		m_fInFrontTime -= a_fDeltaTime;

		if ( m_fInFrontTime < 0.0f )
			m_fInFrontTime = 0.0f;
	}

	TFLOAT fFade = 1.0f;

	if ( 0.01f < m_fMagnitude )
	{
		if ( !bOnScreen )
		{
			m_fFadeTimer += a_fDeltaTime;
			fFade = 1.0f - m_fFadeRate * m_fFadeTimer;

			if ( fFade < 0.0f )
			{
				fFade        = 0.0f;
				m_fFadeTimer = m_fFadeTime;
			}
		}
		else if ( 0.0f < m_fFadeTimer )
		{
			m_fFadeTimer -= a_fDeltaTime;
			fFade = 1.0f - m_fFadeRate * m_fFadeTimer;

			if ( 1.0f <= fFade )
			{
				fFade        = 1.0f;
				m_fFadeTimer = 0.0f;
			}
		}
	}
	else
	{
		fFade = 0.0f;
	}

	fFade = m_fMagnitude * fFade;

	if ( bOnScreen )
		m_bVisible = 0.0f < fFade;

	if ( m_bVisible )
	{
		BuildSunMesh( &vScreenPos, fFade, 0 );
		BuildCoronaMesh( &vScreenPos, fFade );
	}
}

// $deBlob: FUNCTION 005fc440
void ASunFlare::BuildSunMesh( const TVector4* a_pScreenPos, TFLOAT a_fMag, TINT a_iMode )
{
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

	TSysMesh* pMesh  = TNULL;
	TFLOAT    fSizeX = TFLOAT( pDisplayParams->uiWidth ) * m_fSunSizeX;
	TFLOAT    fSizeY = TFLOAT( pDisplayParams->uiHeight ) * m_fSunSizeY;

	if ( a_iMode == 0 )
		pMesh = m_pSunMesh;

	TSysMesh::TLockBuffer oLockBuffer;
	oLockBuffer.oIndices.uiOffset = 0;
	oLockBuffer.oIndices.pBuffer  = TNULL;
	pMesh->Lock( &oLockBuffer );

	TASSERT( oLockBuffer.oIndices.pBuffer != TNULL && "m_pLockedBuffer!=TNULL" );
	TIndexType* pIndices = oLockBuffer.oIndices.pBuffer;
	TASSERT( oLockBuffer.oVertices.uiNumStreams != 0 && "a_uiBuffer<m_uiNumStreams" );
	SunFlareVertex* pVertices = TREINTERPRETCAST( SunFlareVertex*, oLockBuffer.oVertices.apStreams[ 0 ] );

	TVector2 aUVs[ 4 ] = {
		TVector2( 1.0f, 0.0f ),
		TVector2( 0.0f, 0.0f ),
		TVector2( 1.0f, 1.0f ),
		TVector2( 0.0f, 1.0f ),
	};

	TFLOAT  fHalfWidth;
	TFLOAT  fHalfHeight;
	TUINT32 uiColour;

	if ( a_iMode == 0 )
	{
		// Bigger (horizontally) towards the centre of the screen
		TVector4 vFromCentre( a_pScreenPos->x / TFLOAT( pDisplayParams->uiWidth ) - 0.5f, a_pScreenPos->y / TFLOAT( pDisplayParams->uiHeight ) - 0.5f, 0.0f, 0.0f );
		TFLOAT   fMag = vFromCentre.Magnitude();
		TFLOAT   fT   = fMag + fMag + 0.2f;
		TMath::Clip( fT, 0.0f, 1.0f );

		TFLOAT fSize = TMath::LERP( fSizeY, fSizeX, fT ) * a_fMag;

		TUINT32 uiAlpha = TUINT32( a_fMag * 255.0f );
		uiColour        = ( ( ( uiAlpha | 0xFFFFFF00 ) << 8 | uiAlpha ) << 8 ) | uiAlpha;
		fHalfWidth      = fSize;
		fHalfHeight     = fSize;
	}
	else
	{
		// NOTE: retail uses uninitialised locals here (and this as the colour); never reached, pMesh is TNULL then
		uiColour    = 0;
		fHalfWidth  = 0.0f;
		fHalfHeight = 0.0f;
	}

	TFLOAT fX = a_pScreenPos->x;
	TFLOAT fY = a_pScreenPos->y;
	TFLOAT fZ = a_pScreenPos->z - 201.0f;

	pVertices[ 0 ].uiColour = uiColour;
	pVertices[ 1 ].uiColour = uiColour;
	pVertices[ 2 ].uiColour = uiColour;
	pVertices[ 3 ].uiColour = uiColour;

	pVertices[ 0 ].vPosition.Set( fX + fHalfWidth, -fHalfHeight + fY, fZ );
	pVertices[ 0 ].vUV = aUVs[ 0 ];
	pVertices[ 1 ].vPosition.Set( -fHalfWidth + fX, -fHalfHeight + fY, fZ );
	pVertices[ 1 ].vUV = aUVs[ 1 ];
	pVertices[ 2 ].vPosition.Set( fX + fHalfWidth, fY + fHalfHeight, fZ );
	pVertices[ 2 ].vUV = aUVs[ 2 ];
	pVertices[ 3 ].vPosition.Set( -fHalfWidth + fX, fY + fHalfHeight, fZ );
	pVertices[ 3 ].vUV = aUVs[ 3 ];

	// A strip with the first and last vertices doubled
	TINT iVert     = 0;
	TINT iCurIndex = 0;

	do
	{
		pIndices[ iCurIndex + 0 ] = TIndexType( iVert );
		pIndices[ iCurIndex + 1 ] = TIndexType( iVert );
		pIndices[ iCurIndex + 2 ] = TIndexType( iVert + 1 );
		pIndices[ iCurIndex + 3 ] = TIndexType( iVert + 2 );
		pIndices[ iCurIndex + 4 ] = TIndexType( iVert + 3 );
		pIndices[ iCurIndex + 5 ] = TIndexType( iVert + 3 );
		iVert += 4;
		iCurIndex += 6;
	} while ( iVert < 4 );

	TASSERT( iVert <= pMesh->GetMaxVertices() );
	TASSERT( iCurIndex <= pMesh->GetMaxIndices() );

	pMesh->Unlock( 4, TMath::Int32ToUInt16( iCurIndex ) );
	pMesh->Validate();

	if ( a_iMode == 0 )
		pMesh->SetMaterial( m_pSunMaterial );
}

// $deBlob: FUNCTION 005fc870
void ASunFlare::BuildCoronaMesh( const TVector4* a_pScreenPos, TFLOAT a_fMag )
{
	TRender*        pRender  = TRender::GetSingleton();
	TRenderContext* pContext = pRender->GetCurrentContext();
	pRender->GetCurrentDisplayParams();

	TSysMesh* pMesh = m_pCoronaMesh;

	TSysMesh::TLockBuffer oLockBuffer;
	oLockBuffer.oIndices.uiOffset = 0;
	oLockBuffer.oIndices.pBuffer  = TNULL;
	pMesh->Lock( &oLockBuffer );

	TASSERT( oLockBuffer.oIndices.pBuffer != TNULL && "m_pLockedBuffer!=TNULL" );
	TIndexType* pIndices = oLockBuffer.oIndices.pBuffer;
	TASSERT( oLockBuffer.oVertices.uiNumStreams != 0 && "a_uiBuffer<m_uiNumStreams" );
	SunFlareVertex* pVertices = TREINTERPRETCAST( SunFlareVertex*, oLockBuffer.oVertices.apStreams[ 0 ] );

	TASSERT( ( iNumSegments + 1 ) * 2 <= pMesh->GetMaxVertices() );

	TINT     iVert         = 0;
	TFLOAT   fSegmentAngle = TMath::TWO_PI / iNumSegments;
	TVector4 vSegmentDir   = TVector4( 1.0f, 0.0f, 0.0f, 1.0f );
	TUINT32  uiColour      = TUINT32( TINT( a_fMag * 255.0f ) << 24 ) | 0xE4D049;
	TFLOAT   fU            = -1.5f;

	// The direction in view space turns the ring
	TVector4 vOuter;
	TMatrix44::RotateVector( vOuter, pContext->GetModelViewMatrix(), m_vDir );
	vSegmentDir.RotateZ( ( vOuter.y + vOuter.x ) * 0.6f );

	TVector4 vInner;
	vInner.Multiply( vOuter, -2.0f );
	vInner.z = -vInner.z;
	vOuter.Multiply( 180.0f );

	TFLOAT fWidth   = pContext->GetWidth();
	TFLOAT fHeight  = pContext->GetHeight();
	TFLOAT fCentreY = a_pScreenPos->y - fHeight * 0.5f;

	static TFLOAT s_fScaleX = ( 640.0f / fWidth ) * 1.6f;
	static TFLOAT s_fScaleY = ( 448.0f / fHeight ) * 2.3f;

	const TRenderContext::PROJECTIONPARAMS& rProjParams = pContext->GetProjectionParameters();

	vInner.z = 1.6f;
	vOuter.z = 170.0f;
	vOuter.x = ( 1.0f / ( rProjParams.m_Proj.x / rProjParams.m_Centre.x ) ) * ( s_fScaleX / 3.0f ) * ( a_pScreenPos->x - fWidth * 0.5f );
	vInner.x = s_fCoronaInnerOffset * vOuter.x;
	vOuter.y = ( 1.0f / ( rProjParams.m_Proj.y / rProjParams.m_Centre.y ) ) * ( s_fScaleY / 3.0f ) * fCentreY;
	vInner.y = s_fCoronaInnerOffset * vOuter.y;

	// Smaller towards the edges of the screen
	TFLOAT fT = 0.9f - ( 1.0f - ( 1.0f - TMath::Abs( vOuter.x / ( fWidth * 0.125f ) ) ) * ( 1.0f - TMath::Abs( vOuter.y / ( fHeight * 0.125f ) ) ) );
	TMath::Clip( fT, 0.0f, 1.0f );
	fT = TMath::Gain075( fT );

	TFLOAT fInnerRadius = fT * 6.0f + 4.0f;

	for ( TINT i = iNumSegments + 1; i != 0; i-- )
	{
		TVector4 vVert;
		vVert.Multiply( vSegmentDir, fInnerRadius );
		pVertices[ 0 ].vPosition.Set( vVert.x + vInner.x, vVert.y + vInner.y, vInner.z + 0.0f );
		pVertices[ 0 ].vUV.Set( fU, 0.2f );
		pVertices[ 0 ].uiColour = uiColour;

		vVert.Multiply( vSegmentDir, 10.0f );
		iVert += 2;
		pVertices[ 1 ].vPosition.Set( vVert.x + vOuter.x, vVert.y + vOuter.y, vOuter.z + 0.0f );
		pVertices[ 1 ].vUV.Set( fU, 0.95f );
		pVertices[ 1 ].uiColour = uiColour;

		pVertices += 2;
		vSegmentDir.RotateZ( fSegmentAngle );
		fU += 0.3f;
	}

	TINT iCurIndex = 0;

	for ( TINT i = 0; i < iVert; i++ )
		pIndices[ iCurIndex++ ] = TIndexType( i );

	TASSERT( iVert <= pMesh->GetMaxVertices() );
	TASSERT( iVert >= 3 );
	TASSERT( iCurIndex <= pMesh->GetMaxIndices() );

	pMesh->Unlock( TMath::Int32ToUInt16( iVert ), TMath::Int32ToUInt16( iCurIndex ) );
	pMesh->Validate();
}

// $deBlob: FUNCTION 005fbee0
void ASunFlare::Render()
{
	if ( m_bEnabled && 0.01f < m_fMagnitude )
	{
		TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();

		TMatrix44                        mModelView  = pContext->GetModelViewMatrix();
		TRenderContext::PROJECTIONPARAMS oProjParams = pContext->GetProjectionParameters();

		// NOTE: retail leaves m_fUnk uninitialised
		TRenderContext::PROJECTIONPARAMS oOrthoParams;
		oOrthoParams.Set( 0.0f, 0.0f, 1.0f, 1.0f, 0.05f, 200.0f, oProjParams.m_fUnk );

		Update( TSystemManager::GetSingleton()->GetScheduler()->GetCurrentDeltaTime() );

		TMatrix44 mIdentity = TMatrix44::IDENTITY;

		if ( m_bVisible == TTRUE )
		{
			pContext->SetModelViewMatrix( TMatrix44::IDENTITY );
			m_pCoronaMesh->Render();
		}

		pContext->SetProjType( TRenderContext::CameraMode_Orthographic );
		pContext->SetProjectionParameters( oOrthoParams );
		pContext->Update();
		pContext->SetModelViewMatrix( mIdentity );

		if ( m_bVisible == TTRUE )
			m_pSunMesh->Render();

		pContext->SetProjType( TRenderContext::CameraMode_Perspective );
		pContext->SetProjectionParameters( oProjParams );
		pContext->Update();
		pContext->SetModelViewMatrix( mModelView );
	}
}

// $deBlob: FUNCTION 005fc060
void ASunFlare::SetMagnitude( TFLOAT a_fMag )
{
	TASSERT( a_fMag >= 0.0f && a_fMag <= 1.0f );
	m_fMagnitude = a_fMag;
}
