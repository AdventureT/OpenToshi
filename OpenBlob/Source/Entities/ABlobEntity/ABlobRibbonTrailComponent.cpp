#include "pch.h"
#include "ABlobRibbonTrailComponent.h"
#include "ABlobEntity.h"
#include "ABlobColourComponent.h"
#include "ARandom.h"
#include "Cameras/ACamera.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Shaders/ARibbonTrailShader/ARibbonTrailShader.h"

#include <Math/TMatrix44.h>
#include <Math/TQuaternion.h>
#include <Physics/Havok/TPhysicsManager.h>
#include <Platform/DX11/TRender_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobRibbonTrailComponent ) == 0x2D0 );

// The tweakables of the trail
static TINT   s_iRainbowColourAdd  = 64; // added to each channel of the rainbow colour
static TFLOAT s_fRainbowAlphaMin   = 0.75f;
static TFLOAT s_fRainbowAlphaMax   = 1.0f;
static TFLOAT s_fRainbowAlphaSpeed = 1.0f;
static TUINT8 s_uiNoColourAlpha    = 127;
static TINT   s_iColourAdd         = 32; // added to each channel of the paint colour
static TUINT8 s_uiColourAlpha      = 192;
static TFLOAT s_fSideTurnSpeed     = 0.1f;
static TFLOAT s_fArrowVScale       = 0.5f;
static TFLOAT s_fArrowSpinSpeed    = 1.75f; // turns per second
static TBOOL  s_bRenderTrail       = TTRUE;
static TBOOL  s_bRenderArrowTrail  = TTRUE;
static TFLOAT s_fWobbleTime        = 0.15f; // the time of the wobble splat
static TINT   s_iWobbleSteps       = 4;

// The star splats: their points, the scales of their radius
static const TINT s_aiStarPoints[ 3 ]   = { 5, 7, 9 };
static TFLOAT     s_afStarMinScale[ 3 ] = { 0.75f, 0.75f, 0.75f };
static TFLOAT     s_afStarMaxScale[ 3 ] = { 2.0f, 2.0f, 2.0f };
static TFLOAT     s_fStarRadiusScale    = 2.0f;

// NOTE: the V of the first arrow trail vertices is a static retail only reads (0.0f)
static TFLOAT s_fArrowStartV = 0.0f;

// Turns the direction a_rvFrom towards a_rvTo by a_fT.
// $deBlob: FUNCTION 007bde40
static void SlerpDirection( const TVector4& a_rvFrom, const TVector4& a_rvTo, TFLOAT a_fT, TVector4& a_rvOut )
{
	TQuaternion qFrom;
	qFrom.SetVectorDirection( a_rvFrom.AsVector3(), TVector3::VEC_POSZ );
	TQuaternion qTo;
	qTo.SetVectorDirection( a_rvTo.AsVector3(), TVector3::VEC_POSZ );

	TQuaternion qResult;
	TQuaternion::Slerp( qResult, qFrom, qTo, a_fT );
	TQuaternion::RotateVector( a_rvOut, qResult, TVector4::VEC_POSZ );
	a_rvOut.Normalise();
}

const ABlobRibbonTrailComponent::Properties& ABlobRibbonTrailComponent::GetProperties()
{
	return *TSTATICCAST( const Properties, ABlobEntity::ms_pBlobRibbonProperties );
}

// $deBlob: FUNCTION 007ba1f0
ABlobRibbonTrailComponent::ABlobRibbonTrailComponent( ABlobEntity* a_pBlob )
{
	const Properties& rProps = GetProperties();

	m_pBlob   = a_pBlob;
	m_eState  = STATE_TRAIL;
	m_uiFlags = FLAGS_VISIBLE;

	m_avPoints[ 0 ] = TVector4::VEC_ZERO;
	m_avPoints[ 1 ] = TVector4::VEC_ZERO;
	m_avPoints[ 2 ] = TVector4::VEC_ZERO;
	m_avTangents[ 0 ].Multiply( TVector4::VEC_POSY, rProps.m_fSegmentLength );
	m_avTangents[ 1 ].Multiply( TVector4::VEC_POSY, rProps.m_fSegmentLength );
	m_avTangents[ 2 ].Multiply( TVector4::VEC_POSY, rProps.m_fSegmentLength );

	m_vLastPos    = TVector4::VEC_ZERO;
	m_vSide       = TVector4::VEC_NEGY;
	m_vSideTarget = TVector4::VEC_NEGY;
	m_vLastDir    = TVector4::VEC_POSY;
	m_vArrowScale = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	m_vDown       = TVector4::VEC_NEGY;
	m_vMove       = TVector4::VEC_ZERO;

	m_fWobbleAngle    = 0.0f;
	m_fControlDist    = 0.0f;
	m_fDist           = 0.0f;
	m_fTesselation    = 0.0f;
	m_fShrink         = 0.0f;
	m_fArrowAngle     = 0.0f;
	m_fV              = 0.0f;
	m_fLength         = 0.0f;
	m_fAlphaPulse     = 0.0f;
	m_fAlphaPulseDir  = 1.0f;
	m_uiColour        = 0;
	m_iNumSegments    = 0;
	m_iMinSegments    = 8;
	m_iTargetSegments = 0;
	m_iWantedSegments = 0;
	m_iSplatIndex     = 0;
	m_uiLastVert      = 0;

	m_pTrailMaterial = TNULL;
	m_pTrailMesh     = TNULL;
	m_pArrowMaterial = TNULL;
	m_pArrowMesh     = TNULL;
	m_pArrowHeadMesh = TNULL;
	m_pArrowBaseMesh = TNULL;

	ARibbonTrailShader* pShader         = ARibbonTrailShader::GetSingleton();
	TTextureManager*    pTextureManager = TTextureManager::GetSingleton();

	// The trail
	TTexture* pTexture = pTextureManager->FindTexture( "textaribbon.tga" );
	pTexture->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

	m_pTrailMaterial = new ARibbonTrailMaterial();
	m_pTrailMaterial->Create();
	m_pTrailMaterial->SetTexture( 0, pTexture );
	m_pTrailMaterial->SetTextureNum( 1 );
	m_pTrailMaterial->SetOrderTable( pShader->GetOrderTable() );

	m_pTrailMesh = new ARibbonTrailMeshDynamic();
	m_pTrailMesh->Create( MAX_SEGMENTS );
	m_pTrailMesh->SetMaterial( m_pTrailMaterial );
	m_pTrailMesh->SetOwnerShader( pShader );

	// The arrow
	pTexture = pTextureManager->FindTexture( "textaarrow.tga" );
	pTexture->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

	m_pArrowMaterial = new ARibbonTrailMaterial();
	m_pArrowMaterial->Create();
	m_pArrowMaterial->SetTexture( 0, pTexture );
	m_pArrowMaterial->SetTextureNum( 1 );
	m_pArrowMaterial->SetOrderTable( pShader->GetOrderTable() );

	m_pArrowMesh = new ARibbonTrailMeshDynamic();
	m_pArrowMesh->Create( NUM_ARROW_POINTS );
	m_pArrowMesh->SetMaterial( m_pArrowMaterial );
	m_pArrowMesh->SetOwnerShader( pShader );

	m_pArrowHeadMesh = new ARibbonTrailMeshStatic();
	m_pArrowHeadMesh->Create( TUINT16( rProps.m_oArrowHead.iNumVertices ), TUINT16( rProps.m_oArrowHead.iNumIndices ), rProps.m_oArrowHead.pPositions, &m_uiColour, rProps.m_oArrowHead.pUVs, rProps.m_oArrowHead.pIndices );
	m_pArrowHeadMesh->SetMaterial( m_pArrowMaterial );
	m_pArrowHeadMesh->SetOwnerShader( pShader );

	m_pArrowBaseMesh = new ARibbonTrailMeshStatic();
	m_pArrowBaseMesh->Create( TUINT16( rProps.m_oArrowBase.iNumVertices ), TUINT16( rProps.m_oArrowBase.iNumIndices ), rProps.m_oArrowBase.pPositions, &m_uiColour, rProps.m_oArrowBase.pUVs, rProps.m_oArrowBase.pIndices );
	m_pArrowBaseMesh->SetMaterial( m_pTrailMaterial );
	m_pArrowBaseMesh->SetOwnerShader( pShader );
}

// $deBlob: FUNCTION 007ba740
// $deBlob: FUNCTION 00751420
ABlobRibbonTrailComponent::~ABlobRibbonTrailComponent()
{
	m_pTrailMesh->DestroyResource();
	m_pTrailMesh = TNULL;
	m_pArrowBaseMesh->DestroyResource();
	m_pArrowBaseMesh = TNULL;
	m_pTrailMaterial->Destroy();
	m_pTrailMaterial = TNULL;

	m_pArrowMesh->DestroyResource();
	m_pArrowMesh = TNULL;
	m_pArrowHeadMesh->DestroyResource();
	m_pArrowHeadMesh = TNULL;
	m_pArrowMaterial->Destroy();
	m_pArrowMaterial = TNULL;
}

// $deBlob: FUNCTION 007ba7e0
void ABlobRibbonTrailComponent::Tick( TFLOAT a_fDeltaTime )
{
	const Properties&              rProps     = GetProperties();
	const ABlobEntity::Properties& rSizeProps = ABlobEntity::ms_pBlobProperties[ m_pBlob->m_iSize ];

	TVector4 vVelocity;
	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBlob->m_pBody, vVelocity );

	// The wobble gets faster with the speed
	TFLOAT fSpeed = ( vVelocity.Magnitude() - rSizeProps.m_fMaxSpeed ) / rSizeProps.m_fMaxSpeed;
	fSpeed        = TFLOAT( sin( rProps.m_fWobbleFrequency * 6.2831855f * AGameStatus::GetSingleton()->GetTimeLeft() ) ) * 0.5f + fSpeed;
	TMath::Clip( fSpeed, 0.0f, 1.0f );
	UpdateWobble( a_fDeltaTime, TMath::LerpTowards( rProps.m_fWobbleSpeedMin, rProps.m_fWobbleSpeedMax, fSpeed ) );

	// The point under Blob
	TVector4 vPos;
	vPos.Multiply( m_vDown, rSizeProps.m_fRadius + rProps.m_fHeight );
	vPos.Add( m_pBlob->m_vBallPosition );

	TVector4 vMove     = vPos - m_vLastPos;
	TFLOAT   fMoveDist = vMove.NormaliseMag();

	// The colour
	TUINT8* pColour = TREINTERPRETCAST( TUINT8*, &m_uiColour );

	if ( !( TUINT8( m_pBlob->m_uiFlags >> 21 ) & 1 ) )
	{
		TUINT32 uiPaint = m_pBlob->m_pColourComponent->m_uiCurrentColour;
		TUINT8  uiAlpha;

		if ( uiPaint == 0 || ( TUINT8( uiPaint >> 4 ) & 1 ) )
		{
			pColour[ 1 ] = 0;
			pColour[ 2 ] = 0;
			pColour[ 0 ] = 0;
			uiAlpha      = s_uiNoColourAlpha;
		}
		else
		{
			m_uiColour   = m_pBlob->m_pColourComponent->m_uiFadeColour;
			pColour[ 2 ] = TUINT8( TMath::Min( TUINT32( pColour[ 2 ] + s_iColourAdd ), 255u ) );
			pColour[ 1 ] = TUINT8( TMath::Min( TUINT32( pColour[ 1 ] + s_iColourAdd ), 255u ) );
			pColour[ 0 ] = TUINT8( TMath::Min( TUINT32( pColour[ 0 ] + s_iColourAdd ), 255u ) );
			uiAlpha      = s_uiColourAlpha;
		}

		pColour[ 3 ] = uiAlpha;
		m_pTrailMesh->SetAlphaUpdate( TFALSE );
		m_pArrowMesh->SetAlphaUpdate( TFALSE );
	}
	else
	{
		// The rainbow colour, its alpha pulses
		m_uiColour   = *TREINTERPRETCAST( TUINT32*, m_pBlob->m_pColourComponent->m_aRainbowColour );
		pColour[ 2 ] = TUINT8( TMath::Min( TUINT32( pColour[ 2 ] + s_iRainbowColourAdd ), 255u ) );
		pColour[ 1 ] = TUINT8( TMath::Min( TUINT32( pColour[ 1 ] + s_iRainbowColourAdd ), 255u ) );
		pColour[ 0 ] = TUINT8( TMath::Min( TUINT32( pColour[ 0 ] + s_iRainbowColourAdd ), 255u ) );

		m_fAlphaPulse += ( s_fRainbowAlphaMax - s_fRainbowAlphaMin ) * 2.0f * m_fAlphaPulseDir * a_fDeltaTime * s_fRainbowAlphaSpeed;

		if ( s_fRainbowAlphaMin <= m_fAlphaPulse )
		{
			if ( s_fRainbowAlphaMax < m_fAlphaPulse )
			{
				m_fAlphaPulse    = s_fRainbowAlphaMax;
				m_fAlphaPulseDir = m_fAlphaPulseDir * -1.0f;
			}
		}
		else
		{
			m_fAlphaPulse    = s_fRainbowAlphaMin;
			m_fAlphaPulseDir = m_fAlphaPulseDir * -1.0f;
		}

		pColour[ 3 ] = TUINT8( TMath::TruncToInt( m_fAlphaPulse * 255.0f ) );
		m_pTrailMesh->SetAlphaUpdate( TTRUE );
		m_pArrowMesh->SetAlphaUpdate( TTRUE );
	}

	if ( 0.0f < fMoveDist )
		m_vMove = vMove;

	SlerpDirection( m_vSide, m_vSideTarget, s_fSideTurnSpeed * fMoveDist, m_vSide );
	UpdateLength( a_fDeltaTime, fMoveDist );

	if ( m_eState == STATE_TRAIL )
	{
		// The trail lies flat towards the camera
		ACamera*        pCamera  = APlayerManager::GetSingleton()->GetBlobCamera( m_pBlob );
		const TVector4& rForward = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_FORWARD );

		if ( 0.0f < fMoveDist && rForward.y * vMove.y + rForward.x * vMove.x + rForward.z * vMove.z < 0.995f )
		{
			m_vSideTarget.CrossProduct( rForward, vMove );
			m_vSideTarget.Normalise();
		}

		Advance( fMoveDist, &ABlobRibbonTrailComponent::AddTrailControlPoint );
	}
	else if ( m_eState == STATE_SPLAT )
	{
		m_vSideTarget.CrossProduct( m_avTangents[ 1 ], m_avTangents[ 0 ] );
		m_vSideTarget.Normalise();

		Advance( ( TFLOAT( m_iNumSplatPoints ) * rProps.m_fSegmentLength * a_fDeltaTime ) / rProps.m_fSplatTime, &ABlobRibbonTrailComponent::AddSplatControlPoint );
	}

	m_pTrailMesh->Swap();

	// The arrow trail along the head of the spline
	if ( 0.0f < m_fLength )
	{
		TFLOAT fRatio = m_fLength / rProps.m_fSegmentLength;

		m_pArrowMesh->m_uiCurVert         = 0;
		m_pArrowMesh->m_uiNumWriteVerts   = 0;
		m_pArrowMesh->m_uiCurIndex        = 0;
		m_pArrowMesh->m_uiNumWriteIndices = 0;

		TFLOAT fT     = 0.0f;
		TFLOAT fTStep = fRatio / 9.0f;
		TFLOAT fV     = s_fArrowStartV;
		TFLOAT fVStep = ( fRatio - s_fArrowStartV ) / ( s_fArrowVScale * 9.0f );

		for ( TINT i = 0; i < NUM_ARROW_POINTS; i++ )
		{
			TVector4 vPoint;

			if ( m_iNumSegments <= 0 || i != 9 )
				TMath::HermitePosition( vPoint, m_vSplinePos, m_vSplineTangent, m_vControlPos, m_vControlTangent, 1.0f - fT );
			else
				vPoint = m_vSplinePos;

			TFLOAT fHalfWidth = rProps.m_fArrowWidth * 0.5f * rSizeProps.m_fTrailScale;

			ARibbonTrailMesh::Vertex& rLeft = m_pArrowMesh->GetVertex( m_pArrowMesh->m_uiCurVert );
			rLeft.vPosition.x               = m_vSide.x * fHalfWidth;
			rLeft.vPosition.y               = m_vSide.y * fHalfWidth;
			rLeft.vPosition.z               = m_vSide.z * fHalfWidth;
			rLeft.vPosition.x               = vPoint.x + rLeft.vPosition.x;
			rLeft.vPosition.y               = rLeft.vPosition.y + vPoint.y;
			rLeft.vPosition.z               = rLeft.vPosition.z + vPoint.z;
			rLeft.uiColour                  = m_uiColour;
			rLeft.vUV.x                     = 0.0f;
			rLeft.vUV.y                     = fV;

			m_pArrowMesh->GetIndex( m_pArrowMesh->m_uiCurIndex ) = m_pArrowMesh->m_uiCurVert;
			m_pArrowMesh->m_uiCurVert++;
			m_pArrowMesh->m_uiNumWriteVerts++;
			m_pArrowMesh->m_uiCurIndex++;
			m_pArrowMesh->m_uiNumWriteIndices++;

			fHalfWidth = rProps.m_fArrowWidth * -0.5f * rSizeProps.m_fTrailScale;

			ARibbonTrailMesh::Vertex& rRight = m_pArrowMesh->GetVertex( m_pArrowMesh->m_uiCurVert );
			rRight.vPosition.x               = fHalfWidth * m_vSide.x;
			rRight.vPosition.y               = m_vSide.y * fHalfWidth;
			rRight.vPosition.z               = m_vSide.z * fHalfWidth;
			rRight.vPosition.x               = vPoint.x + rRight.vPosition.x;
			rRight.vPosition.y               = vPoint.y + rRight.vPosition.y;
			rRight.vPosition.z               = vPoint.z + rRight.vPosition.z;
			rRight.uiColour                  = m_uiColour;
			rRight.vUV.x                     = 1.0f;
			rRight.vUV.y                     = fV;

			m_pArrowMesh->GetIndex( m_pArrowMesh->m_uiCurIndex ) = m_pArrowMesh->m_uiCurVert;
			m_pArrowMesh->m_uiCurVert++;
			m_pArrowMesh->m_uiNumWriteVerts++;
			m_pArrowMesh->m_uiCurIndex++;
			m_pArrowMesh->m_uiNumWriteIndices++;

			fT += fTStep;
			fV += fVStep;
		}

		m_pArrowMesh->Swap();
	}

	if ( TUINT8( m_uiFlags >> 4 ) & 1 )
	{
		m_fArrowAngle = s_fArrowSpinSpeed * 6.2831855f * a_fDeltaTime + m_fArrowAngle;

		if ( 6.2831855f < m_fArrowAngle )
			m_fArrowAngle = TFLOAT( fmod( m_fArrowAngle, TFLOAT( 6.2831855f ) ) );
	}

	m_vLastPos = vPos;
}

// $deBlob: FUNCTION 007bb410
void ABlobRibbonTrailComponent::Render()
{
	if ( !( TUINT8( m_uiFlags >> 3 ) & 1 ) )
		return;

	if ( s_bRenderTrail )
		m_pTrailMesh->Render();

	if ( s_bRenderArrowTrail && 0.0f < m_fLength )
		m_pArrowMesh->Render();

	if ( TUINT8( m_uiFlags >> 4 ) & 1 )
	{
		m_pArrowBaseMesh->SetColour( m_uiColour );
		m_pArrowHeadMesh->SetColour( m_uiColour );

		// The arrow spins under Blob
		TRenderContext* pContext   = TRenderDX11::Interface()->GetCurrentContext();
		TMatrix44       mModelView = pContext->GetModelViewMatrix();
		TMatrix44       mArrow     = mModelView;
		TMatrix44::TransformVector( mArrow.AsBasisVector4( BASISVECTOR_TRANSLATION ), mModelView, m_pBlob->m_vBallPosition );
		mArrow.RotateY( m_fArrowAngle );
		mArrow.Scale( m_vArrowScale );

		pContext->SetModelViewMatrix( mArrow );
		m_pArrowBaseMesh->Render();
		m_pArrowHeadMesh->Render();
		pContext->SetModelViewMatrix( mModelView );
	}
}

// $deBlob: FUNCTION 007bb550
TINT ABlobRibbonTrailComponent::SetWantedSegments( TINT16 a_iNumSegments )
{
	m_iWantedSegments = a_iNumSegments;

	if ( a_iNumSegments < 0 )
		m_iWantedSegments = 0;

	if ( MAX_SEGMENTS < m_iWantedSegments )
		m_iWantedSegments = MAX_SEGMENTS;

	if ( m_eState != STATE_SPLAT )
		m_iTargetSegments = m_iWantedSegments;

	return m_iWantedSegments;
}

// $deBlob: FUNCTION 007bb5a0
void ABlobRibbonTrailComponent::SetPosition( const TVector4& a_rvPosition )
{
	m_avPoints[ 2 ] = a_rvPosition;
	m_avPoints[ 1 ] = a_rvPosition;
	m_avPoints[ 0 ] = a_rvPosition;
	m_vControlPos   = a_rvPosition;

	TVector4 vTangent;
	vTangent.Multiply( TVector4::VEC_POSY, GetProperties().m_fSegmentLength );

	m_uiFlags &= ~FLAGS_STARTED;
	m_avTangents[ 2 ] = vTangent;
	m_avTangents[ 1 ] = vTangent;
	m_avTangents[ 0 ] = vTangent;
	m_vControlTangent = vTangent;
}

// $deBlob: FUNCTION 007bb620
void ABlobRibbonTrailComponent::Reset()
{
	ResetTrail();

	m_fLength      = 0.0f;
	m_fControlDist = 0.0f;
	m_vLastPos     = m_pBlob->m_vBallPosition;
	m_vLastDir     = TVector4::VEC_POSY;
	SetPosition( m_vLastPos );
	m_iTargetSegments = m_iWantedSegments;
	m_eState          = STATE_TRAIL;
}

// $deBlob: FUNCTION 007bb680
void ABlobRibbonTrailComponent::OnLanding( const TVector4& a_rvDirection )
{
	if ( m_eState != STATE_SPLAT && !( TUINT8( m_uiFlags >> 4 ) & 1 ) )
	{
		m_avPoints[ 0 ] = m_pBlob->m_vBallPosition;
		TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBlob->m_pBody, m_avTangents[ 0 ] );
		m_avTangents[ 0 ].Normalise();
		m_avTangents[ 0 ].Multiply( TVector4::Distance( m_avPoints[ 0 ], m_avPoints[ 1 ] ) );

		TINT iPattern = ARandom::GetSingleton()->GetRandom().GetInt( 3 );

		if ( iPattern == 0 )
			CreateSplatWobble( a_rvDirection );
		else if ( iPattern == 1 )
			CreateSplatSpiral( a_rvDirection );
		else
			CreateSplatStar( a_rvDirection );

		m_eState          = STATE_SPLAT;
		m_iSplatIndex     = 0;
		m_iTargetSegments = MAX_SEGMENTS;
	}
}

// $deBlob: FUNCTION 007bb740
void ABlobRibbonTrailComponent::ResetTrail()
{
	m_iNumSegments                         = 0;
	m_pTrailMesh->m_uiCurIndex             = 0;
	m_pTrailMesh->m_uiCurVert              = 0;
	m_pTrailMesh->m_uiNumWriteIndices      = 0;
	m_pTrailMesh->m_uiNumWriteVerts        = 0;
	m_uiLastVert                           = 0;
	m_fDist                                = 0.0f;
	m_fTesselation                         = GetProperties().m_fSegmentLength;
	m_fShrink                              = 0.0f;
	m_vSplinePos                           = m_pBlob->m_vBallPosition;
	m_vSplineTangent                       = TVector4::VEC_POSY;
	m_vTailRight                           = m_vLastPos;
	m_vTailLeft                            = m_vLastPos;
	m_pTrailMesh->GetVertex( 0 ).vPosition = m_vLastPos.AsVector3();
	m_pTrailMesh->GetVertex( 1 ).vPosition = m_vLastPos.AsVector3();
}

// $deBlob: FUNCTION 007bb890
void ABlobRibbonTrailComponent::AddSegment()
{
	if ( m_iNumSegments < 2 )
	{
		m_vLastDir = m_avTangents[ 2 ];
	}
	else
	{
		TINT iPrevVert = TINT( TINT16( m_uiLastVert ) ) - 2;

		if ( iPrevVert < 0 )
			iPrevVert += m_pTrailMesh->GetMaxIndices();

		const TVector3& rPrevLeft = m_pTrailMesh->GetVertex( TUINT16( iPrevVert ) ).vPosition;
		const TVector3& rLeft     = m_pTrailMesh->GetVertex( m_uiLastVert ).vPosition;
		m_vLastDir.x              = rLeft.x - rPrevLeft.x;
		m_vLastDir.y              = rLeft.y - rPrevLeft.y;
		m_vLastDir.z              = rLeft.z - rPrevLeft.z;
		m_vLastDir.w              = 1.0f;

		const TVector3& rPrevRight = m_pTrailMesh->GetVertex( TUINT16( iPrevVert + 1 ) ).vPosition;
		const TVector3& rRight     = m_pTrailMesh->GetVertex( TUINT16( m_uiLastVert + 1 ) ).vPosition;
		m_vLastDir.x += rRight.x - rPrevRight.x;
		m_vLastDir.y += rRight.y - rPrevRight.y;
		m_vLastDir.z += rRight.z - rPrevRight.z;
	}

	m_vLastDir.Normalise();

	m_pTrailMesh->GetIndex( m_pTrailMesh->m_uiCurIndex ) = m_pTrailMesh->m_uiCurVert;
	m_pTrailMesh->m_uiCurIndex++;
	m_pTrailMesh->GetIndex( m_pTrailMesh->m_uiCurIndex ) = m_pTrailMesh->m_uiCurVert + 1;
	m_pTrailMesh->m_uiCurIndex++;

	if ( m_iNumSegments < 2 )
	{
		m_vTailLeft.AsVector3()  = m_pTrailMesh->GetVertex( m_uiLastVert ).vPosition;
		m_vTailLeft.w            = 1.0f;
		m_vTailRight.AsVector3() = m_pTrailMesh->GetVertex( TUINT16( m_uiLastVert + 1 ) ).vPosition;
		m_vTailRight.w           = 1.0f;
	}

	m_uiLastVert = m_pTrailMesh->m_uiCurVert;
	m_pTrailMesh->m_uiCurVert += 2;

	if ( m_pTrailMesh->GetMaxVerts() <= m_pTrailMesh->m_uiCurVert )
		m_pTrailMesh->m_uiCurVert = 0;

	if ( m_pTrailMesh->GetMaxIndices() <= m_pTrailMesh->m_uiCurIndex )
		m_pTrailMesh->m_uiCurIndex = 0;

	m_iNumSegments++;

	if ( MAX_SEGMENTS < m_iNumSegments )
	{
		// Full: the oldest segment is the tail
		m_iNumSegments                    = MAX_SEGMENTS;
		m_pTrailMesh->m_uiNumWriteVerts   = 2 * MAX_SEGMENTS;
		m_pTrailMesh->m_uiNumWriteIndices = 2 * MAX_SEGMENTS;

		TUINT16 uiTail           = m_pTrailMesh->GetIndex( m_pTrailMesh->m_uiCurIndex );
		m_vTailLeft.AsVector3()  = m_pTrailMesh->GetVertex( uiTail ).vPosition;
		m_vTailLeft.w            = 1.0f;
		uiTail                   = m_pTrailMesh->GetIndex( TUINT16( m_pTrailMesh->m_uiCurIndex + 1 ) );
		m_vTailRight.AsVector3() = m_pTrailMesh->GetVertex( uiTail ).vPosition;
		m_vTailRight.w           = 1.0f;
		return;
	}

	m_pTrailMesh->m_uiNumWriteVerts += 2;
	m_pTrailMesh->m_uiNumWriteIndices += 2;
}

// $deBlob: FUNCTION 007bbe90
void ABlobRibbonTrailComponent::AddTrailControlPoint()
{
	const Properties&              rProps     = GetProperties();
	const ABlobEntity::Properties& rSizeProps = ABlobEntity::ms_pBlobProperties[ m_pBlob->m_iSize ];

	m_avPoints[ 2 ]   = m_avPoints[ 1 ];
	m_avTangents[ 2 ] = m_avTangents[ 1 ];
	m_avPoints[ 1 ]   = m_avPoints[ 0 ];
	m_avTangents[ 1 ] = m_avTangents[ 0 ];

	m_avPoints[ 0 ].Multiply( m_vDown, rSizeProps.m_fRadius + rProps.m_fHeight );
	m_avPoints[ 0 ].x = m_pBlob->m_vBallPosition.x + m_avPoints[ 0 ].x;
	m_avPoints[ 0 ].y = m_pBlob->m_vBallPosition.y + m_avPoints[ 0 ].y;
	m_avPoints[ 0 ].z = m_pBlob->m_vBallPosition.z + m_avPoints[ 0 ].z;
	m_avTangents[ 0 ].Multiply( m_vMove, rProps.m_fSegmentLength );
	m_uiFlags |= FLAGS_STARTED;

	// More segments in turns: 20 degrees and more between the tangents is the most
	static TFLOAT s_fCosMaxTurn = TFLOAT( cos( 0.3490658402442932 ) );

	TFLOAT fCos = ( m_avTangents[ 1 ].y * m_avTangents[ 2 ].y + m_avTangents[ 2 ].x * m_avTangents[ 1 ].x + m_avTangents[ 1 ].z * m_avTangents[ 2 ].z ) / ( rProps.m_fSegmentLength * rProps.m_fSegmentLength );
	TMath::Clip( fCos, s_fCosMaxTurn, 1.0f );

	TFLOAT fTurn = 1.0f - ( fCos - s_fCosMaxTurn ) / ( 1.0f - s_fCosMaxTurn );
	TMath::Clip( fTurn, 0.0f, 1.0f );

	TINT iExtra    = TMath::FloorToInt( ( rProps.m_fMaxSubdivisions - rProps.m_fMinSubdivisions ) * fTurn );
	m_fTesselation = rProps.m_fSegmentLength / ( rProps.m_fMinSubdivisions + TFLOAT( iExtra ) );
	TASSERT( m_fTesselation > 0.0f );

	m_fControlDist = 0.0f;
}

// $deBlob: FUNCTION 007bc0b0
void ABlobRibbonTrailComponent::AddSplatControlPoint()
{
	const Properties& rProps = GetProperties();

	TINT16 iSplat     = m_iSplatIndex;
	m_avPoints[ 2 ]   = m_avPoints[ 1 ];
	m_avTangents[ 2 ] = m_avTangents[ 1 ];
	m_avPoints[ 1 ]   = m_avPoints[ 0 ];
	m_avTangents[ 1 ] = m_avTangents[ 0 ];

	if ( iSplat < m_iNumSplatPoints )
	{
		m_avPoints[ 0 ]   = m_avSplatPoints[ iSplat ];
		m_avTangents[ 0 ] = m_avSplatTangents[ iSplat ];
		m_iSplatIndex     = iSplat + 1;
	}
	else
	{
		// The pattern is over, back to the trail
		m_avPoints[ 0 ] = m_pBlob->m_vBallPosition;
		TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBlob->m_pBody, m_avTangents[ 0 ] );

		if ( m_avTangents[ 0 ].NormaliseMag() <= 0.1f )
			m_avTangents[ 0 ].Multiply( m_pBlob->m_vFacing, rProps.m_fSegmentLength );
		else
			m_avTangents[ 0 ].Multiply( rProps.m_fSegmentLength );

		m_iTargetSegments = m_iWantedSegments;
		m_eState          = STATE_TRAIL;
	}

	m_uiFlags |= FLAGS_STARTED;

	// More segments in turns: 20 degrees and more between the tangents is the most
	static TFLOAT s_fCosMaxTurn = TFLOAT( cos( 0.3490658402442932 ) );

	TFLOAT fScale = 1.0f;
	TFLOAT fCos   = ( m_avTangents[ 1 ].y * m_avTangents[ 2 ].y + m_avTangents[ 1 ].x * m_avTangents[ 2 ].x + m_avTangents[ 1 ].z * m_avTangents[ 2 ].z ) / ( rProps.m_fSegmentLength * rProps.m_fSegmentLength );
	TMath::Clip( fCos, s_fCosMaxTurn, 1.0f );

	TFLOAT fTurn = 1.0f - ( fCos - s_fCosMaxTurn ) / ( 1.0f - s_fCosMaxTurn );
	TMath::Clip( fTurn, 0.0f, 1.0f );

	// The splat tangents are longer than the segment length
	TFLOAT fTangentLength = ( m_avTangents[ 1 ].Magnitude() + m_avTangents[ 2 ].Magnitude() ) * 0.5f;

	if ( 0.0f < fTangentLength )
		fScale = rProps.m_fSegmentLength / fTangentLength;

	TINT iExtra    = TMath::FloorToInt( ( rProps.m_fMaxSubdivisions - rProps.m_fMinSubdivisions ) * fTurn );
	m_fTesselation = ( fScale * rProps.m_fSegmentLength ) / ( rProps.m_fMinSubdivisions + TFLOAT( iExtra ) );
	TASSERT( m_fTesselation > 0.0f );

	m_fControlDist = 0.0f;
}

// $deBlob: FUNCTION 007bc350
void ABlobRibbonTrailComponent::UpdateLength( TFLOAT a_fDeltaTime, TFLOAT a_fMoveDist )
{
	const Properties& rProps = GetProperties();
	TINT16            iNum   = m_iNumSegments;

	if ( 1 < iNum )
	{
		// The tail is cut faster the more segments there are too many
		TINT16 iTarget = m_iTargetSegments;
		TFLOAT fShrink;

		if ( iTarget < iNum )
		{
			TINT iSpeed = ( TINT( iNum ) - TINT( iTarget ) ) / 4;
			TMath::Clip( iSpeed, 4, 16 );
			fShrink = TFLOAT( iSpeed ) * TMath::Max( rProps.m_fShrinkSpeed * a_fDeltaTime, a_fMoveDist );
			m_fShrink += fShrink;
		}
		else if ( iTarget - 2 < iNum )
		{
			fShrink = TMath::Max( rProps.m_fShrinkSpeed * a_fDeltaTime, a_fMoveDist );
			m_fShrink += fShrink;
		}
		else if ( m_iMinSegments < iNum )
		{
			fShrink = rProps.m_fShrinkSpeed * a_fDeltaTime;
			m_fShrink += fShrink;
		}

		// The tail segment and the next one
		TINT iTailIndex = TINT( m_pTrailMesh->m_uiCurIndex ) + m_iNumSegments * -2;
		TINT iNextIndex = iTailIndex + 2;

		if ( iTailIndex < 0 )
			iTailIndex += m_pTrailMesh->GetMaxIndices();

		if ( iNextIndex < 0 )
			iNextIndex += m_pTrailMesh->GetMaxIndices();

		TUINT16 uiTail = m_pTrailMesh->GetIndex( TUINT16( iTailIndex ) );
		TUINT16 uiNext = m_pTrailMesh->GetIndex( TUINT16( iNextIndex ) );

		TFLOAT fSegmentLength = ( TVector3::Distance( m_pTrailMesh->GetVertex( uiNext ).vPosition, m_vTailLeft.AsVector3() ) +
		                          TVector3::Distance( m_pTrailMesh->GetVertex( TUINT16( uiNext + 1 ) ).vPosition, m_vTailRight.AsVector3() ) ) *
		    0.5f;

		while ( fSegmentLength < m_fShrink )
		{
			// Drop the tail segment
			m_vTailLeft.AsVector3()  = m_pTrailMesh->GetVertex( uiNext ).vPosition;
			m_vTailLeft.w            = 1.0f;
			m_vTailRight.AsVector3() = m_pTrailMesh->GetVertex( TUINT16( uiNext + 1 ) ).vPosition;
			m_vTailRight.w           = 1.0f;

			m_iNumSegments--;
			m_fShrink -= fSegmentLength;
			m_pTrailMesh->m_uiNumWriteVerts -= 2;
			m_pTrailMesh->m_uiNumWriteIndices -= 2;

			if ( m_iNumSegments < 2 )
			{
				m_fLength = m_fLength - m_fShrink;

				if ( 0.0f <= m_fLength )
					ResetTrail();
				else
					Reset();

				break;
			}

			iTailIndex = TINT( m_pTrailMesh->m_uiCurIndex ) + m_iNumSegments * -2;
			iNextIndex = iTailIndex + 2;

			if ( iTailIndex < 0 )
				iTailIndex += m_pTrailMesh->GetMaxIndices();

			if ( iNextIndex < 0 )
				iNextIndex += m_pTrailMesh->GetMaxIndices();

			uiTail = m_pTrailMesh->GetIndex( TUINT16( iTailIndex ) );
			uiNext = m_pTrailMesh->GetIndex( TUINT16( iNextIndex ) );

			fSegmentLength = ( TVector3::Distance( m_pTrailMesh->GetVertex( uiNext ).vPosition, m_vTailLeft.AsVector3() ) +
			                   TVector3::Distance( m_pTrailMesh->GetVertex( TUINT16( uiNext + 1 ) ).vPosition, m_vTailRight.AsVector3() ) ) *
			    0.5f;
		}

		if ( 0.0f < fSegmentLength && 1 < m_iNumSegments )
		{
			// Move the tail towards the next segment and fade the V along the trail
			TFLOAT fT = m_fShrink / fSegmentLength;

			m_pTrailMesh->GetVertex( uiTail ).vPosition.Lerp( m_vTailLeft.AsVector3(), m_pTrailMesh->GetVertex( uiNext ).vPosition, fT );
			m_pTrailMesh->GetVertex( TUINT16( uiTail + 1 ) ).vPosition.Lerp( m_vTailRight.AsVector3(), m_pTrailMesh->GetVertex( TUINT16( uiNext + 1 ) ).vPosition, fT );
			m_pTrailMesh->GetVertex( uiTail ).vUV.y                = 1.0f;
			m_pTrailMesh->GetVertex( TUINT16( uiTail + 1 ) ).vUV.y = 1.0f;

			TINT   iHalf = m_iNumSegments / 2;
			TINT   iLeft = iHalf + 1;
			TFLOAT fStep = 1.0f / TFLOAT( iHalf );
			TFLOAT fV    = 1.0f - ( 1.0f - fT ) * fStep;

			do
			{
				uiTail                                                 = ( TUINT32( uiTail ) + 2 < m_pTrailMesh->GetMaxVerts() ) ? TUINT16( uiTail + 2 ) : 0;
				m_pTrailMesh->GetVertex( uiTail ).vUV.y                = fV;
				m_pTrailMesh->GetVertex( TUINT16( uiTail + 1 ) ).vUV.y = fV;
				fV -= fStep;
			} while ( --iLeft != 0 );
		}

		return;
	}

	if ( 0 < iNum )
		return;

	if ( m_fLength <= 0.0f )
		return;

	// No segments: the arrow trail shrinks
	if ( m_iTargetSegments < 1 )
	{
		m_fLength -= TMath::Max( rProps.m_fShrinkSpeed * a_fDeltaTime, a_fMoveDist ) * 4.0f;
	}
	else if ( m_iMinSegments <= 0 )
	{
		m_fLength -= rProps.m_fShrinkSpeed * a_fDeltaTime;
	}

	if ( 0.0f < m_fLength || m_fLength == 0.0f )
		return;

	Reset();
}

// $deBlob: FUNCTION 007bcb50
void ABlobRibbonTrailComponent::UpdateSpline( TFLOAT a_fLerpT )
{
	TASSERT( a_fLerpT >= 0.0f );
	TASSERT( a_fLerpT <= 1.0f );

	const Properties&              rProps     = GetProperties();
	const ABlobEntity::Properties& rSizeProps = ABlobEntity::ms_pBlobProperties[ m_pBlob->m_iSize ];

	if ( 0 < m_iNumSegments )
	{
		TMath::HermitePosition( m_vSplinePos, m_avPoints[ 2 ], m_avTangents[ 2 ], m_avPoints[ 1 ], m_avTangents[ 1 ], a_fLerpT );
		TMath::HermiteTangent( m_vSplineTangent, m_avPoints[ 2 ], m_avTangents[ 2 ], m_avPoints[ 1 ], m_avTangents[ 1 ], a_fLerpT );

		// The head segment follows the spline
		TFLOAT                    fHalfWidth = rProps.m_fTrailWidth * 0.5f * rSizeProps.m_fTrailScale;
		ARibbonTrailMesh::Vertex& rLeft      = m_pTrailMesh->GetVertex( m_uiLastVert );
		rLeft.vPosition.x                    = m_vSide.x * fHalfWidth;
		rLeft.vPosition.y                    = m_vSide.y * fHalfWidth;
		rLeft.vPosition.z                    = m_vSide.z * fHalfWidth;
		rLeft.vPosition.x                    = m_vSplinePos.x + rLeft.vPosition.x;
		rLeft.vPosition.y                    = m_vSplinePos.y + rLeft.vPosition.y;
		rLeft.vPosition.z                    = m_vSplinePos.z + rLeft.vPosition.z;
		rLeft.uiColour                       = m_uiColour;
		rLeft.vUV.x                          = 0.0f;
		rLeft.vUV.y                          = m_fV;

		fHalfWidth                       = rProps.m_fTrailWidth * -0.5f * rSizeProps.m_fTrailScale;
		ARibbonTrailMesh::Vertex& rRight = m_pTrailMesh->GetVertex( TUINT16( m_uiLastVert + 1 ) );
		rRight.vPosition.x               = m_vSide.x * fHalfWidth;
		rRight.vPosition.y               = m_vSide.y * fHalfWidth;
		rRight.vPosition.z               = m_vSide.z * fHalfWidth;
		rRight.vPosition.x               = m_vSplinePos.x + rRight.vPosition.x;
		rRight.vPosition.y               = m_vSplinePos.y + rRight.vPosition.y;
		rRight.vPosition.z               = m_vSplinePos.z + rRight.vPosition.z;
		rRight.uiColour                  = m_uiColour;
		rRight.vUV.x                     = 1.0f;
		rRight.vUV.y                     = m_fV;
		return;
	}

	if ( 0.0f < m_fLength )
	{
		TMath::HermitePosition( m_vSplinePos, m_avPoints[ 2 ], m_avTangents[ 2 ], m_avPoints[ 1 ], m_avTangents[ 1 ], a_fLerpT );
		TMath::HermiteTangent( m_vSplineTangent, m_avPoints[ 2 ], m_avTangents[ 2 ], m_avPoints[ 1 ], m_avTangents[ 1 ], a_fLerpT );
	}
	else
	{
		TVector4 vVelocity;
		TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBlob->m_pBody, vVelocity );

		if ( 0.0f < vVelocity.MagnitudeSq() )
		{
			m_vSplinePos     = m_pBlob->m_vBallPosition;
			m_vSplineTangent = vVelocity;
			m_vSplineTangent.Normalise();
			m_vSplineTangent.Multiply( rProps.m_fSegmentLength );
		}
	}
}

// $deBlob: FUNCTION 007bd190
void ABlobRibbonTrailComponent::Advance( TFLOAT a_fDist, t_ControlPointFunc a_fnAddControlPoint )
{
	const Properties& rProps = GetProperties();

	if ( m_fLength < rProps.m_fSegmentLength && 0 < m_iTargetSegments )
		m_fLength = TMath::Min( m_fLength + a_fDist, rProps.m_fSegmentLength );

	TASSERT( m_fControlDist >= 0.0f );
	TASSERT( m_fTesselation >= 0.0f );

	TFLOAT fT       = 0.0f;
	TBOOL  bStarted = m_uiFlags & FLAGS_STARTED;

	if ( !bStarted || m_fLength < rProps.m_fSegmentLength || ( m_iTargetSegments <= 0 && m_iNumSegments < 2 ) )
	{
		// No segments yet: only the control points move
		if ( !bStarted )
			m_fTesselation = rProps.m_fSegmentLength;

		m_fDist += a_fDist;

		if ( m_fTesselation < m_fDist )
			m_fDist = TFLOAT( fmod( m_fDist, m_fTesselation ) );

		m_fControlDist += a_fDist;

		if ( rProps.m_fSegmentLength <= m_fControlDist )
		{
			( this->*a_fnAddControlPoint )();
			m_fDist = 0.0f;
		}

		fT = m_fControlDist / rProps.m_fSegmentLength;
	}
	else
	{
		m_fDist += a_fDist;

		while ( m_fTesselation < m_fDist )
		{
			AddSegment();

			m_fDist -= m_fTesselation;
			m_fControlDist += m_fTesselation;

			if ( rProps.m_fSegmentLength <= m_fControlDist )
			{
				( this->*a_fnAddControlPoint )();
				m_fDist = 0.0f;
			}

			UpdateSpline( m_fControlDist / rProps.m_fSegmentLength );
		}

		fT = ( m_fControlDist + m_fDist ) / rProps.m_fSegmentLength;
		TMath::Clip( fT, 0.0f, 1.0f );
	}

	UpdateSpline( fT );
	TMath::HermitePosition( m_vControlPos, m_avPoints[ 1 ], m_avTangents[ 1 ], m_avPoints[ 0 ], m_avTangents[ 0 ], fT );
	TMath::HermiteTangent( m_vControlTangent, m_avPoints[ 1 ], m_avTangents[ 1 ], m_avPoints[ 0 ], m_avTangents[ 0 ], fT );
}

// $deBlob: FUNCTION 007bd490
void ABlobRibbonTrailComponent::UpdateWobble( TFLOAT a_fDeltaTime, TFLOAT a_fSpeed )
{
	m_fWobbleAngle = a_fSpeed * 0.25f * a_fDeltaTime + m_fWobbleAngle;

	if ( 6.2831855f < m_fWobbleAngle )
		m_fWobbleAngle = TFLOAT( fmod( m_fWobbleAngle, TFLOAT( 6.2831855f ) ) );

	TVector4 vAxis;
	TMath::SinCos( m_fWobbleAngle, vAxis.y, vAxis.x );
	vAxis.z = 0.0f;
	vAxis.w = 1.0f;

	TVector4 vDown;
	TVector4::RotateArbitaryVector( vDown, m_vDown, vAxis, a_fDeltaTime * a_fSpeed );
	m_vDown   = vDown;
	m_vDown.w = 1.0f;
	m_vDown.Normalise();
}

// A spiral around a_rvDirection (or the last tangent)
// $deBlob: FUNCTION 007bd560
void ABlobRibbonTrailComponent::CreateSplatSpiral( const TVector4& a_rvDirection )
{
	TVector4 vDirection;

	if ( a_rvDirection.MagnitudeSq() <= 0.1f )
	{
		vDirection = m_avTangents[ 1 ];
		vDirection.Normalise();
	}
	else
	{
		vDirection = a_rvDirection;
	}

	TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

	TVector4 vOffset;
	vOffset.w = 1.0f;
	vOffset.z = rRandom.GetFloat();
	vOffset.y = rRandom.GetFloat();
	vOffset.x = rRandom.GetFloat();
	vOffset.Normalise();
	vOffset.Multiply( ABlobEntity::ms_pBlobProperties[ m_pBlob->m_iSize ].m_fRadius * 2.0f );

	TVector4 vCentre = m_avPoints[ 0 ];

	for ( TINT i = 0; i < MAX_SPLAT_POINTS; i++ )
	{
		m_avSplatPoints[ i ].x = vCentre.x + vOffset.x;
		m_avSplatPoints[ i ].y = vCentre.y + vOffset.y;
		m_avSplatPoints[ i ].z = vCentre.z + vOffset.z;
		m_avSplatPoints[ i ].w = vCentre.w;

		TVector4 vStep;
		vStep.Multiply( vDirection, rRandom.GetFloat() * 0.5f + 0.5f );
		vCentre.x = vStep.x + vCentre.x;
		vCentre.y = vStep.y + vCentre.y;
		vCentre.z = vStep.z + vCentre.z;

		TVector4 vRotated;
		TVector4::RotateArbitaryVector( vRotated, vOffset, vDirection, 1.0471976f );
		vOffset = vRotated;
	}

	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBlob->m_pBody, m_avSplatTangents[ 0 ] );
	m_avSplatTangents[ 0 ].Normalise();
	m_avSplatTangents[ 0 ].Multiply( GetProperties().m_fSegmentLength );
	m_avSplatTangents[ MAX_SPLAT_POINTS - 1 ] = m_avSplatTangents[ 0 ];

	TFLOAT fRadius = ABlobEntity::ms_pBlobProperties[ m_pBlob->m_iSize ].m_fRadius;

	for ( TINT i = 1; i < MAX_SPLAT_POINTS - 1; i++ )
	{
		m_avSplatTangents[ i ].x = m_avSplatPoints[ i + 1 ].x - m_avSplatPoints[ i - 1 ].x;
		m_avSplatTangents[ i ].y = m_avSplatPoints[ i + 1 ].y - m_avSplatPoints[ i - 1 ].y;
		m_avSplatTangents[ i ].z = m_avSplatPoints[ i + 1 ].z - m_avSplatPoints[ i - 1 ].z;
		m_avSplatTangents[ i ].w = m_avSplatPoints[ i + 1 ].w;
		m_avSplatTangents[ i ].Normalise();
		m_avSplatTangents[ i ].Multiply( fRadius * 2.0f * 1.0471976f );
	}

	m_iNumSplatPoints = MAX_SPLAT_POINTS;
}

// The wobble of the trail played fast, under Blob
// $deBlob: FUNCTION 007bd8d0
void ABlobRibbonTrailComponent::CreateSplatWobble( const TVector4& a_rvDirection )
{
	const Properties& rProps = GetProperties();

	TFLOAT   fHeight = ABlobEntity::ms_pBlobProperties[ m_pBlob->m_iSize ].m_fRadius + rProps.m_fHeight;
	TVector4 vPrev   = m_pBlob->GetBodyPosition();

	for ( TINT i = 0; i < MAX_SPLAT_POINTS; i++ )
	{
		TFLOAT fLength = 0.0f;

		for ( TINT j = 0; j < s_iWobbleSteps; j++ )
		{
			TFLOAT fStep  = s_fWobbleTime / TFLOAT( s_iWobbleSteps );
			TFLOAT fSpeed = rProps.m_fWobbleSpeedMax;

			m_fWobbleAngle = fSpeed * 0.25f * fStep + m_fWobbleAngle;

			if ( 6.2831855f < m_fWobbleAngle )
				m_fWobbleAngle = TFLOAT( fmod( m_fWobbleAngle, TFLOAT( 6.2831855f ) ) );

			TVector4 vAxis;
			TMath::SinCos( m_fWobbleAngle, vAxis.y, vAxis.x );
			vAxis.z = 0.0f;
			vAxis.w = 1.0f;

			TVector4 vDown;
			TVector4::RotateArbitaryVector( vDown, m_vDown, vAxis, fStep * fSpeed );
			m_vDown   = vDown;
			m_vDown.w = 1.0f;
			m_vDown.Normalise();

			TVector4 vPoint;
			vPoint.Multiply( m_vDown, ABlobEntity::ms_pBlobProperties[ m_pBlob->m_iSize ].m_fRadius + rProps.m_fHeight );
			vPoint.x = m_pBlob->m_vBallPosition.x + vPoint.x;
			vPoint.y = m_pBlob->m_vBallPosition.y + vPoint.y;
			vPoint.z = m_pBlob->m_vBallPosition.z + vPoint.z;

			m_vMove.x = vPoint.x - vPrev.x;
			m_vMove.y = vPoint.y - vPrev.y;
			m_vMove.z = vPoint.z - vPrev.z;
			m_vMove.w = vPoint.w;
			fLength += m_vMove.NormaliseMag();

			vPrev = vPoint;
		}

		m_avSplatPoints[ i ].Multiply( m_vDown, fHeight );
		m_avSplatPoints[ i ].x = m_avSplatPoints[ i ].x + m_pBlob->m_vBallPosition.x;
		m_avSplatPoints[ i ].y = m_pBlob->m_vBallPosition.y + m_avSplatPoints[ i ].y;
		m_avSplatPoints[ i ].z = m_pBlob->m_vBallPosition.z + m_avSplatPoints[ i ].z;
		m_avSplatTangents[ i ].Multiply( m_vMove, fLength );
	}

	m_iNumSplatPoints = MAX_SPLAT_POINTS;
}

// A star of 5, 7 or 9 points around Blob, facing the camera
// $deBlob: FUNCTION 007bdba0
void ABlobRibbonTrailComponent::CreateSplatStar( const TVector4& a_rvDirection )
{
	TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

	TINT iType   = rRandom.GetInt( 3 );
	TINT iPoints = s_aiStarPoints[ iType ];

	m_iNumSplatPoints = TINT16( iPoints + 1 );

	if ( MAX_SPLAT_POINTS < m_iNumSplatPoints )
		m_iNumSplatPoints = MAX_SPLAT_POINTS;

	TMatrix44 mCamera = TMatrix44::IDENTITY;
	ACamera*  pCamera = m_pBlob->GetViewCamera();

	if ( pCamera != TNULL )
	{
		mCamera                                           = pCamera->GetMatrix();
		mCamera.AsBasisVector4( BASISVECTOR_TRANSLATION ) = TVector4::VEC_ZERO;
	}

	TINT     iPoint  = 0;
	TFLOAT   fRadius = m_pBlob->GetBoundingSphereRadius() * s_fStarRadiusScale;
	TVector4 vCentre = m_pBlob->GetMatrix().GetTranslation();

	if ( 0 < iPoints )
	{
		TINT iSkip = iPoints / 2;

		for ( TINT i = 0; i < iPoints; i++ )
		{
			TVector4& rPoint = m_avSplatPoints[ i ];
			TFLOAT    fAngle = ( TFLOAT( iPoint ) * 6.2831855f ) / TFLOAT( iPoints );

			rPoint.x = TFLOAT( sin( fAngle ) );
			rPoint.z = 0.0f;
			rPoint.w = 1.0f;
			rPoint.y = -TFLOAT( cos( fAngle ) );

			TFLOAT fMinScale = s_afStarMinScale[ iType ];
			TFLOAT fMaxScale = s_afStarMaxScale[ iType ];
			rPoint.Multiply( ( rRandom.GetFloat() * ( fMaxScale - fMinScale ) + fMinScale ) * fRadius );
			TMatrix44::TransformVector( rPoint, mCamera, rPoint );

			rPoint.x = rPoint.x + vCentre.x;
			iPoint   = ( iPoint + iSkip ) % iPoints;
			rPoint.y = rPoint.y + vCentre.y;
			rPoint.z = rPoint.z + vCentre.z;

			m_avSplatTangents[ i ] = TVector4::VEC_ZERO;
		}
	}

	// Closed
	m_avSplatPoints[ iPoints ]   = m_avSplatPoints[ 0 ];
	m_avSplatTangents[ iPoints ] = TVector4::VEC_ZERO;
}
