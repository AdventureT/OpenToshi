#include "pch.h"
#include "AShadow.h"
#include "Render/AInstanceRenderer.h"
#include "Animation/AModelInstance2.h"
#include "GameController/AGameStatus.h"
#include "Cameras/ACameraManager.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsPhantomContainer.h>
#include <Render/TRender.h>
#include <Render/TRenderContext.h>
#include <Render/TModel.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL            AShadow::ms_bFixRenderVolume = TTRUE;
TFLOAT           AShadow::ms_fFadeStartSq     = 8100.0f;
TFLOAT           AShadow::ms_fFadeEndSq       = 10000.0f;
TFLOAT           AShadow::ms_fFlatNormalY     = -0.99f;
TFLOAT           AShadow::ms_fNoGroundNormalY = 2.0f;
TFLOAT           AShadow::ms_fMoveThreshold   = 0.0001f;
AShadow*         AShadow::ms_apShadows[ MAX_SHADOWS ];
TINT             AShadow::ms_iNumShadows;
Toshi::TMatrix44 AShadow::ms_aMatrices[ MAX_SHADOWS ];

TSTATICASSERT( sizeof( AShadow ) == 0x60 );

AShadow::ShadowType AShadow::ms_aTypes[] = {
	{ "Fx_Shadow.trb", 0xFFFFFFFF },
};

static const TVector4 s_vDefaultDirection( 1.0f, 0.0f, 0.0f, 1.0f );
static const TVector4 s_vDown( 0.0f, 1.0f, 0.0f, 1.0f );

// $deBlob: FUNCTION 005b8be0
AShadow::AShadow( TFLOAT a_fSize, TUINT8 a_eType )
{
	m_vPosition         = TVector4::VEC_ZERO;
	m_vDirection        = s_vDefaultDirection;
	m_Normal            = TVector4::VEC_NEGY;
	m_vRayPosition      = TVector4::VEC_ZERO;
	m_eType             = a_eType;
	m_fGroundNormalY    = ms_fNoGroundNormalY;
	m_fFade             = 1.0f;
	m_fAlpha            = 1.0f;
	m_fWidth            = a_fSize;
	m_fLength           = a_fSize;
	m_fHeight           = 0.0f;
	m_shadowQueryHelper = TNULL;
	m_iMesh             = -1;
	m_uiFlags           = FLAG_Dirty;
}

// $deBlob: FUNCTION 005b8c60
// $deBlob: FUNCTION 007c4210
AShadow::~AShadow()
{
	if ( m_iMesh != -1 )
		AInstanceRenderer::GetInstance()->UnregisterModel( m_iMesh );

	TPhysicsPhantomContainer* pHelper = m_shadowQueryHelper;
	ms_bFixRenderVolume = TTRUE;

	if ( pHelper != TNULL )
	{
		TPhysicsManager::GetSingleton()->DestroyPhantom( pHelper );
		m_shadowQueryHelper = TNULL;
	}
}

// $deBlob: FUNCTION 005b8ca0
void AShadow::Init( TFLOAT a_fSize, TUINT8 a_eType )
{
	TASSERT( !IsInitialised() );

	m_eType   = a_eType;
	m_fWidth  = a_fSize;
	m_fLength = a_fSize;
	m_iMesh   = AInstanceRenderer::GetInstance()->RegisterModel( ms_aTypes[ a_eType ].m_szModel );

	AModelInstance2* pInstance = AInstanceRenderer::GetInstance()->FindInstance( ms_aTypes[ a_eType ].m_szModel );

	if ( ms_bFixRenderVolume && pInstance != TNULL )
	{
		T2ModelPtr pModel                              = pInstance->GetModelInstance()->GetModel();
		pModel->GetLOD( 0 ).m_RenderVolume.GetRadius() = 0.0f;
		ms_bFixRenderVolume                            = TFALSE;
	}

	AInstanceRenderer::GetInstance()->SetupMaterials( m_iMesh );
	m_uiFlags |= FLAG_Initialised;

	if ( m_shadowQueryHelper == TNULL )
	{
		m_shadowQueryHelper = TPhysicsManager::GetSingleton()->CreateAabbPhantom();
		TPhysicsManager::GetSingleton()->SetPhantomCollisionFilterInfo( m_shadowQueryHelper, 0x83000 );
	}
}

// $deBlob: FUNCTION 005b8e30
void AShadow::Update( const TVector4& a_rPosition, const TVector4& a_rDirection, TBOOL a_bForce )
{
	if ( !IsInitialised() )
		return;

	if ( m_uiFlags & FLAG_Hidden )
		m_uiFlags &= ~FLAG_OnGround;
	else
		UpdateVisibility( a_rPosition, a_rDirection, a_bForce );
}

// $deBlob: FUNCTION 005b8e50
void AShadow::SetHidden( TBOOL a_bHidden )
{
	if ( !a_bHidden )
		m_uiFlags = ( m_uiFlags & ~FLAG_Hidden ) | FLAG_Dirty;
	else
		m_uiFlags |= FLAG_Hidden;

	if ( m_shadowQueryHelper != TNULL && a_bHidden && m_shadowQueryHelper->IsInSimulation() )
		TPhysicsManager::GetSingleton()->RemovePhantomFromSimulation( m_shadowQueryHelper, TNULL );
}

// $deBlob: FUNCTION 005b8e90
void AShadow::BatchSumbit()
{
	if ( ms_iNumShadows == 0 )
		return;

	for ( TINT i = 0; i < ms_iNumShadows; i++ )
	{
		AShadow*   pShadow = ms_apShadows[ i ];
		TMatrix44& rMatrix = ms_aMatrices[ i ];
		TVector4&  vX      = rMatrix.AsBasisVector4( BASISVECTOR_RIGHT );
		TVector4&  vY      = rMatrix.AsBasisVector4( BASISVECTOR_UP );
		TVector4&  vZ      = rMatrix.AsBasisVector4( BASISVECTOR_FORWARD );

		vY.Set( pShadow->m_Normal.x, pShadow->m_Normal.y, pShadow->m_Normal.z, 0.0f );
		TASSERT( vY.IsNormalised() );

		vX.Set( -pShadow->m_vDirection.x, pShadow->m_vDirection.y, pShadow->m_vDirection.z, 0.0f );
		vZ.CrossProduct( vX, vY );
		vX.CrossProduct( vZ, vY );
		rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = TVector4::VEC_ZERO;

		vX.Multiply( pShadow->m_fWidth );
		vY.Multiply( pShadow->m_fWidth );
		vZ.Multiply( pShadow->m_fLength );
	}

	for ( TINT i = 0; i < ms_iNumShadows; i++ )
	{
		AShadow* pShadow = ms_apShadows[ i ];

		AInstanceRenderer::GetInstance()->SubmitForBatchRender(
		    pShadow->m_iMesh, &ms_aMatrices[ i ], &pShadow->m_vPosition, pShadow->m_fAlpha * pShadow->m_fFade, ms_aTypes[ pShadow->m_eType ].m_uiColour
		);
	}

	ms_iNumShadows = 0;
}

// $deBlob: FUNCTION 005b9060
// NOTE: a_bForce is unused
void AShadow::UpdatePosition( const TVector4& a_rPosition, const TVector4& a_rDirection, TBOOL a_bForce )
{
	if ( !IsInitialised() )
		return;

	m_vDirection = a_rDirection;

	if ( !AGameStatus::GetSingleton()->IsMultiplayer() && m_fGroundNormalY < ms_fFlatNormalY )
	{
		// On flat ground the shadow only follows the owner until it moved far enough (relative to the camera)
		TFLOAT fCameraDistanceSq = TVector4::DistanceSq( ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation(), a_rPosition );

		if ( !( m_uiFlags & FLAG_Dirty ) && TVector4::DistanceSq( m_vRayPosition, a_rPosition ) < ms_fMoveThreshold * fCameraDistanceSq )
		{
			TASSERT( m_Normal.IsNormalised() );
			m_vPosition.x = a_rPosition.x;
			m_uiFlags |= FLAG_OnGround;
			m_vPosition.z = a_rPosition.z;
			return;
		}
	}

	m_uiFlags &= ~FLAG_OnGround;
	m_vRayPosition = a_rPosition;

	TFLOAT           fRayLength      = ( m_eType == 0 ) ? 12.0f : 6.0f;
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
	TASSERT( m_shadowQueryHelper, "phantomContainer" );

	if ( !m_shadowQueryHelper->IsInSimulation() )
		pPhysicsManager->AddPhantomToSimulation( m_shadowQueryHelper, a_rPosition, TNULL );

	TPhysicsRay oRayCast;
	oRayCast.m_vFrom = a_rPosition;
	oRayCast.m_vFrom.y -= 0.2f;

	TVector4 vRay;
	vRay.Multiply( s_vDown, fRayLength );
	oRayCast.m_vTo.Set( vRay.x + oRayCast.m_vFrom.x, vRay.y + oRayCast.m_vFrom.y, vRay.z + oRayCast.m_vFrom.z, oRayCast.m_vFrom.w );
	oRayCast.m_uiCollisionLayer = 0x83;

	// The phantom's box around the ray
	TVector4 aBox[ 2 ];
	aBox[ 0 ].Multiply( oRayCast.m_vTo + oRayCast.m_vFrom, 0.5f );
	aBox[ 1 ].Multiply( oRayCast.m_vFrom - oRayCast.m_vTo, 0.5f );
	aBox[ 1 ].Abs4();
	pPhysicsManager->SetAabbPhantomBox( m_shadowQueryHelper, aBox );

	TASSERT( pPhysicsManager->GetCollisionTestFlags( m_shadowQueryHelper ) == oRayCast.m_uiCollisionLayer );

	TPhysicsShapeContact oContact;

	if ( !pPhysicsManager->CollideRay( oRayCast, oContact, m_shadowQueryHelper ) )
	{
		m_fGroundNormalY = ms_fNoGroundNormalY;
		return;
	}

	TVector4 normal  = oContact.m_vNormal;
	m_fGroundNormalY = normal.y;

	if ( m_fGroundNormalY <= -0.5f )
	{
		TASSERT( normal.IsNormalised() );
		m_uiFlags |= FLAG_OnGround;
		m_Normal    = normal;
		m_vPosition = oContact.m_vPosition;
		m_vPosition.y -= m_fHeight + 0.06f;
	}
}

// $deBlob: FUNCTION 005b9440
void AShadow::UpdateVisibility( const TVector4& a_rPosition, const TVector4& a_rDirection, TBOOL a_bForce )
{
	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
	{
		TFLOAT fDistanceSq = TVector4::DistanceSq( ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation(), a_rPosition );

		if ( fDistanceSq <= ms_fFadeStartSq )
			m_fFade = 1.0f;
		else
			m_fFade = TMath::LERP( 1.0f, 0.0f, ( fDistanceSq - ms_fFadeStartSq ) / ( ms_fFadeEndSq - ms_fFadeStartSq ) );

		if ( fDistanceSq <= ms_fFadeEndSq )
		{
			if ( !( m_uiFlags & FLAG_OnGround ) )
				m_uiFlags |= FLAG_Dirty;

			m_uiFlags |= FLAG_OnGround;
		}
		else
		{
			m_uiFlags &= ~FLAG_OnGround;
		}
	}

	if ( ( m_uiFlags & FLAG_OnGround ) || AGameStatus::GetSingleton()->IsMultiplayer() )
	{
		if ( a_bForce || ( m_uiFlags & FLAG_Dirty ) )
		{
			UpdatePosition( a_rPosition, a_rDirection, a_bForce );
			m_uiFlags &= ~FLAG_Dirty;
		}
	}

	if ( ( m_uiFlags & FLAG_OnGround ) && ms_iNumShadows < MAX_SHADOWS )
	{
		TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();
		TSphere         oSphere( m_vPosition.x, m_vPosition.y, m_vPosition.z, TMath::Max( m_fWidth, m_fLength ) );

		if ( TRenderContext::CullSphereToFrustumSimple( oSphere, pContext->GetWorldPlanes(), 6 ) )
			ms_apShadows[ ms_iNumShadows++ ] = this;
	}
}
