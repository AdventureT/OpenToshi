#include "pch.h"
#include "Cameras/ACameraManager.h"
#include "AModelInstance2.h"
#include "AModel2.h"
#include "AToshiAnimInterface2.h"
#include "Render/Lighting/ALightingData.h"
#include "Fx/AEnvironmentFX.h"
#include "Render/Material/AMaterialManager.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include "Cameras/ACamera.h"
#include "Collision/ACollisionInstance2.h"
#include "Sound/ASoundBreakpointComponent.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include "Platform/DX11/ASkinShader_DX11.h"

#include <Render/TRender.h>
#include <Render/TRenderContext.h>
#include <Platform/DX11/TWorldShader_DX11.h>

#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TUINT32 AModelInstance2::ms_uiDefaultLocalAmbientColour = 0x00FFFFFF;
TINT    AModelInstance2::ms_RenderCount;

// The custom render callback of the model instances (set by OnLoaded)
// $deBlob: FUNCTION 0054e620
void __stdcall AModelInstance2::RenderGeneric( TModelInstance* a_pModelInstance, void* a_pUserData )
{
	AModelInstance2* pThis = TSTATICCAST( AModelInstance2, a_pUserData );

	T2ModelPtr         pModel            = a_pModelInstance->GetModel();
	TSkeletonInstance* pSkeletonInstance = a_pModelInstance->GetSkeletonInstance();

	if ( pSkeletonInstance->GetOverlayAnimationCount() + pSkeletonInstance->GetBaseAnimationCount() != 0 )
		pSkeletonInstance->UpdateState( TFALSE );

	TModelLOD& rLOD = pModel->GetLOD( a_pModelInstance->GetLOD() );

	if ( rLOD.GetMeshCount() <= 0 )
		return;

	TBOOL  bSkinMesh = TFALSE;
	TUINT8 uiFlags   = TUINT8( pThis->m_uiFlags );

	if ( TUINT8( uiFlags >> 7 ) & 1 )
	{
		// Finds out once what kind of mesh the model has
		const TClass* pMeshClass = rLOD.GetMesh( 0 )->GetClass();

		if ( pMeshClass == &TGetClass( TWorldMeshHAL ) )
		{
			pThis->SetFlag( 0x10, TTRUE );
		}
		else
		{
			bSkinMesh = pMeshClass == &TGetClass( ASkinMeshHAL );

			if ( bSkinMesh )
				pThis->SetFlag( 0x20, TTRUE );
		}

		pThis->SetFlag( 0x80, TFALSE );
	}
	else if ( !( uiFlags & 0x10 ) && ( TUINT8( uiFlags >> 5 ) & 1 ) )
	{
		bSkinMesh = TTRUE;
	}

	TRender*        pRender  = TRender::GetSingleton();
	TRenderContext* pContext = pRender->GetCurrentContext();
	pContext->SetSkeletonInstance( pSkeletonInstance );

	TBOOL     bScaled;
	TMatrix44 oOldModelView;

	if ( pThis->m_vScale.x == 1.0f && pThis->m_vScale.y == 1.0f && pThis->m_vScale.z == 1.0f && pThis->m_vScale.w == 1.0f )
	{
		bScaled = TFALSE;
	}
	else
	{
		bScaled = TTRUE;

		TRenderContext* pCurrentContext = pRender->GetCurrentContext();
		oOldModelView                   = pCurrentContext->GetModelViewMatrix();

		TMatrix44 oModelView = pCurrentContext->GetModelViewMatrix();
		oModelView.Scale( pThis->m_vScale );
		pCurrentContext->SetModelViewMatrix( oModelView );
	}

	AMaterialManager::GetSingleton()->UpdateUVs( a_pModelInstance->GetModel().GetData() );

	TMatrix44 oOldLightDirection = pRender->GetLightDirection();
	TMatrix44 oOldLightColour    = pRender->GetLightColour();
	TVector4  vOldAmbient        = pRender->GetCurrentContext()->GetAmbientColour();
	TFLOAT    fOldShadeCoeff     = pContext->GetShadeCoeff();

	if ( TUINT8( pThis->m_uiFlags >> 9 ) & 1 )
		pContext->SetShadeCoeff( pThis->m_fAmbientScale );

	pThis->SetRenderLighting( pThis->m_fAmbientScale, ( pThis->m_uiFlags >> 13 ) & 1 );

	TMaterial* pOldMaterial  = TNULL;
	TMaterial* pSkinMaterial = TNULL;
	TBOOL      bSkinMaterial = bSkinMesh && ( TUINT8( pThis->m_uiFlags >> 8 ) & 1 );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		TBOOL bMeshSettings = TFALSE;

		if ( ( pThis->m_uiFlags & 0x1000 ) && i <= 4 )
		{
			TASSERT( i < MAX_MESH_SETTINGS );

			if ( pThis->m_aMeshSettings[ i ].uiFlags & 1 )
			{
				bMeshSettings = TTRUE;

				// Hidden mesh
				if ( !( pThis->m_aMeshSettings[ i ].uiFlags & 2 ) )
					continue;
			}
		}

		TMesh* pMesh = rLOD.GetMesh( i );
		TASSERT( pMesh == TNULL || pMesh->IsA( &TGetClass( TMesh ) ) );

		TMaterial* pOverrideMaterial = TNULL;

		if ( bSkinMaterial )
		{
			TASSERT( pMesh == TNULL || pMesh->IsA( &TGetClass( ASkinMesh ) ) );
			pSkinMaterial     = TSTATICCAST( ASkinMesh, pMesh )->GetMaterial2();
			pOverrideMaterial = pSkinMaterial;
		}

		if ( pOverrideMaterial == TNULL )
			pOverrideMaterial = TREINTERPRETCAST( TMaterial*, pThis->m_uiOverrideMaterial );

		if ( pOverrideMaterial != TNULL )
		{
			pOldMaterial = pMesh->GetMaterial();
			pMesh->SetMaterial( pOverrideMaterial );
		}

		if ( !bMeshSettings )
		{
			pMesh->Render();
		}
		else
		{
			TFLOAT   fOldAlpha       = pContext->GetAlphaBlend();
			TVector4 vOldMeshAmbient = pContext->GetAmbientColour();
			TUINT8*  pColour         = pThis->m_aMeshSettings[ TUINT8( i ) ].aColour;

			pContext->SetAlphaBlend( TFLOAT( pColour[ 3 ] ) / 255.0f );
			pContext->SetAmbientColour( TVector4(
			    ( TFLOAT( pColour[ 2 ] ) + 0.0f ) * ( 1.0f / 255.0f ),
			    ( TFLOAT( pColour[ 1 ] ) + 0.0f ) * ( 1.0f / 255.0f ),
			    ( TFLOAT( pColour[ 0 ] ) + 0.0f ) * ( 1.0f / 255.0f ),
			    ( TFLOAT( pColour[ 3 ] ) + 0.0f ) * ( 1.0f / 255.0f )
			) );

			pMesh->Render();

			pContext->SetAlphaBlend( fOldAlpha );
			pContext->SetAmbientColour( vOldMeshAmbient );
		}

		if ( ( bSkinMaterial && pSkinMaterial != TNULL ) || pThis->m_uiOverrideMaterial != 0 )
			pMesh->SetMaterial( pOldMaterial );
	}

	if ( TUINT8( pThis->m_uiFlags >> 9 ) & 1 )
		pContext->SetShadeCoeff( fOldShadeCoeff );

	if ( bScaled )
		pRender->GetCurrentContext()->SetModelViewMatrix( oOldModelView );

	pRender->GetCurrentContext()->SetAmbientColour( vOldAmbient );
	pRender->SetLightDirectionMatrix( oOldLightDirection );
	pRender->SetLightColourMatrix( oOldLightColour );
}

// $deBlob: FUNCTION 00571bf0
void AModelInstance2::Render()
{
	if ( m_pModelInstance != TNULL && ( m_uiFlags & FLAG_Visible ) )
		Render( m_pModelInstance );
}

// $deBlob: FUNCTION 00571c10
void AModelInstance2::SetTransform( const TMatrix44& a_rMatrix )
{
	if ( m_pModelInstance )
		m_pModelInstance->GetTransform().SetMatrix( a_rMatrix );

	if ( m_pCollisionInstance )
		TSTATICCAST( ACollisionInstance2, m_pCollisionInstance )->SetMatrix( a_rMatrix );
}

// $deBlob: FUNCTION 00571c40
void AModelInstance2::GetTransform( TMatrix44& a_rOutMatrix )
{
	// GetMatrix asserts MODE_MATRIX==GetMode()
	a_rOutMatrix = m_pModelInstance->GetTransform().GetMatrix();
}

// $deBlob: FUNCTION 00571cb0
void AModelInstance2::SetPosition( const TVector4& a_rPosition )
{
	TMatrix44 mTransform;
	GetTransform( mTransform );
	mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_rPosition;

	if ( m_pModelInstance )
		m_pModelInstance->GetTransform().SetMatrix( mTransform );

	if ( m_pCollisionInstance )
		TSTATICCAST( ACollisionInstance2, m_pCollisionInstance )->SetMatrix( mTransform );
}

// $deBlob: FUNCTION 00571d00
void AModelInstance2::GetPosition( TVector4& a_rOutPosition )
{
	TMatrix44 mTransform;
	GetTransform( mTransform );
	a_rOutPosition = mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// $deBlob: FUNCTION 00571d30
void AModelInstance2::SetFlag( FLAG a_uiFlag, TBOOL a_bSet )
{
	TBOOL bWasRenderable = IsRenderable();

	if ( a_bSet )
		m_uiFlags |= a_uiFlag;
	else
		m_uiFlags &= ~a_uiFlag;

	// The model keeps its renderable instances at the front of its list
	if ( bWasRenderable )
	{
		if ( !IsRenderable() )
		{
			Remove();
			m_pModel->m_lInstances.PushBack( this );
		}
	}
	else if ( IsRenderable() )
	{
		Remove();
		m_pModel->m_lInstances.PushFront( this );
	}
}

// $deBlob: FUNCTION 00571e30
TINT16 AModelInstance2::FindBone( const TCHAR* a_szBoneName )
{
	TSkeletonInstance* pSkeletonInstance = m_pModelInstance->GetSkeletonInstance();

	if ( pSkeletonInstance )
		return pSkeletonInstance->GetBoneID( a_szBoneName );

	return -1;
}

// The bone transform in the model space (scaled like the instance)
// $deBlob: FUNCTION 00571e50
void AModelInstance2::GetBoneTransformLocal( TINT a_iBoneId, TMatrix44& a_rOutMatrix )
{
	TASSERT( a_iBoneId >= 0 );
	TSkeletonInstance* pSkeletonInstance = m_pModelInstance->GetSkeletonInstance();

	if ( pSkeletonInstance != TNULL )
	{
		pSkeletonInstance->GetBoneTransformCurrent( a_iBoneId, a_rOutMatrix );
		a_rOutMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ).Multiply( m_vScale );
	}
}

// $deBlob: FUNCTION 00571ec0
void AModelInstance2::GetBoneTransform( TINT a_iBoneId, TMatrix44& a_rOutMatrix )
{
	TASSERT( a_iBoneId >= 0 );
	TSkeletonInstance* pSkeletonInstance = m_pModelInstance->GetSkeletonInstance();

	if ( pSkeletonInstance != TNULL )
	{
		TMatrix44 mBone;
		pSkeletonInstance->GetBoneTransformCurrent( a_iBoneId, mBone );
		mBone.AsBasisVector4( BASISVECTOR_TRANSLATION ).Multiply( m_vScale );

		TMatrix44 mTransform = m_pModelInstance->GetTransform().GetMatrix();
		a_rOutMatrix         = mTransform;
		a_rOutMatrix.Multiply( mTransform, mBone );
	}
}

// $deBlob: FUNCTION 00572010
void AModelInstance2::UseLocalAmbientColour( TBOOL a_bUse )
{
	SetFlag( FLAG_LocalAmbientColour, a_bUse );
}

// $deBlob: FUNCTION 00572030
void AModelInstance2::SetLocalAmbientColour( TUINT32 a_uiColour )
{
	m_uiLocalAmbientColour = a_uiColour;
}

// Sets up the light direction, light colour and ambient colour of the renderer for this instance
// $deBlob: FUNCTION 00572040
void AModelInstance2::SetRenderLighting( TFLOAT a_fAmbientScale, TBOOL a_bUseEnvironmentScale )
{
	TRender*              pRender = TRender::GetSingleton();
	ALightingData::Light* pLight  = ALightingData::GetSingleton()->GetCurrent();

	TASSERT( pLight->vLightDirection.w == 0.0f );
	pRender->GetLightDirection().AsBasisVector4( BASISVECTOR_RIGHT ) = pLight->vLightDirection;

	TVector4& rLightColour = pRender->GetLightColour().AsBasisVector4( BASISVECTOR_RIGHT );

	if ( !( m_uiFlags & FLAG_LocalAmbient ) )
		rLightColour = pLight->vLightColour;
	else
		UnpackColour( m_uiAmbientColour, rLightColour );

	TRenderContext* pContext = pRender->GetCurrentContext();
	TVector4        vAmbient;
	UnpackColour( ( m_uiFlags & FLAG_LocalAmbientColour ) ? m_uiLocalAmbientColour : ms_uiDefaultLocalAmbientColour, vAmbient );

	if ( !a_bUseEnvironmentScale )
	{
		if ( !( m_uiFlags & 0x200 ) )
		{
			vAmbient.Multiply( a_fAmbientScale );
			rLightColour.Multiply( a_fAmbientScale );
		}
	}
	else if ( AEnvironmentFX::GetSingleton() != TNULL )
	{
		// The lighting scale of the environment (blended by a_fAmbientScale)
		TVector4 vScale;
		AEnvironmentFX::GetLightingScale( &vScale, a_fAmbientScale );
		vAmbient.Multiply( vScale );
		rLightColour.Multiply( vScale );
	}

	// NOTE: retail writes the context's ambient colour first and scales it in place
	pContext->SetAmbientColour( vAmbient );
}

// Culls the instance against the distance limits and the frustum of the camera being rendered and draws it
// $deBlob: FUNCTION 00572190
TBOOL AModelInstance2::Render( TModelInstance* a_pObj )
{
	TASSERT( a_pObj->IsVisible() );

	TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();
	const TSphere*  pSphere  = GetBoundingSphere();

	TTransformObject& rTransform = a_pObj->GetTransform();
	const TVector3&   rScale     = rTransform.GetScale();

	TFLOAT fMaxInstanceScale = TMath::Max( TMath::Max( m_vScale.x, m_vScale.y ), m_vScale.z );
	TFLOAT fMaxObjScale      = TMath::Max( TMath::Max( rScale.x, rScale.y ), rScale.z );

	TVector3 vCentre(
	    rScale.x * pSphere->GetOrigin().x * fMaxInstanceScale,
	    rScale.y * pSphere->GetOrigin().y * fMaxInstanceScale,
	    rScale.z * pSphere->GetOrigin().z * fMaxInstanceScale
	);

	TFLOAT fRadius = fMaxObjScale * pSphere->GetRadius() * fMaxInstanceScale;

	TMatrix44 oMatrix = rTransform.GetLocalMatrix();
	TMatrix44::TransformVector( vCentre, oMatrix, vCentre );

	TVector4 vMatrixScale;
	oMatrix.GetScale( vMatrixScale );

	if ( vMatrixScale.x < vMatrixScale.y ) vMatrixScale.x = vMatrixScale.y;
	if ( vMatrixScale.z <= vMatrixScale.x ) vMatrixScale.z = vMatrixScale.x;

	fRadius = vMatrixScale.z * fRadius;

	AXYZViewportManager* pViewportManager = ARenderer::GetSingleton()->GetViewportManager();
	TASSERT( pViewportManager->GetViewportBeingRendered() != AXYZViewportManager::VIEWPORT_NONE );
	ACamera* pCamera = pViewportManager->GetCamera( pViewportManager->GetViewportBeingRendered() );

	if ( !( m_uiFlags & 0x8000 ) )
	{
		const TMatrix44& rCameraMatrix = pCamera->GetMatrix();

		TFLOAT fDY        = rCameraMatrix.m_f42 - vCentre.y;
		TFLOAT fDX        = rCameraMatrix.m_f41 - vCentre.x;
		TFLOAT fDZ        = rCameraMatrix.m_f43 - vCentre.z;
		TFLOAT fDistance2 = fDY * fDY + fDX * fDX + fDZ * fDZ;

		if ( !( m_uiFlags & 0x4000 ) )
		{
			if ( ATerrain2Interface::GetInstanceLODDistanceSq( 1 ) < fDistance2 )
				return TFALSE;
		}
		else if ( ATerrain2Interface::GetInstanceLODDistanceSq( 2 ) < fDistance2 )
		{
			return TFALSE;
		}
	}

	if ( 0.0f < fRadius && !TRenderContext::CullSphereToFrustumSimple( TSphere( vCentre, fRadius ), pCamera->GetViewPlanes(), 6 ) )
		return TFALSE;

	ms_RenderCount++;
	a_pObj->SetLOD( 0 );
	pContext->SetAlphaBlend( 1.0f );
	rTransform.Push();

	TFLOAT fOldAlpha = pContext->GetAlphaBlend();

	if ( m_pModelInstance->IsForceFade() )
		pContext->SetAlphaBlend( m_pModelInstance->GetFade() );

	a_pObj->GetCustomRenderCb()( a_pObj, a_pObj->GetCustomRenderCbUserData() );

	pContext->SetAlphaBlend( fOldAlpha );
	rTransform.Pop();
	return TTRUE;
}

// The bounding sphere of the current animation, or of the model's LOD 0
// NOTE: retail returns the TModelLOD (its render volume is its first member)
// $deBlob: FUNCTION 005724f0
void AModelInstance2::GetBoundingSphereWorld( TVector4& a_rOutSphere )
{
	// The bounding sphere: the radius stays in w
	const TSphere* pSphere = TNULL;

	if ( m_pAnimInterface != TNULL && m_pAnimInterface->GetSkeletonInstance() != TNULL )
		pSphere = m_pAnimInterface->GetSkeletonInstance()->GetAnimationBoundingSphere();

	if ( pSphere == TNULL )
	{
		T2ModelPtr pModel = m_pModelInstance->GetModel();
		pSphere           = &pModel->GetLOD( 0 ).m_RenderVolume;
	}

	a_rOutSphere = *TREINTERPRETCAST( const TVector4*, pSphere );

	TMatrix44 oMatrix = m_pModelInstance->GetTransform().GetMatrix();
	TMatrix44::TransformVector( a_rOutSphere.AsVector3(), oMatrix, a_rOutSphere.AsVector3() );
}

// $deBlob: FUNCTION 00572630
const TSphere* AModelInstance2::GetBoundingSphere()
{
	if ( m_pAnimInterface != TNULL && m_pAnimInterface->GetSkeletonInstance() != TNULL )
	{
		// The animation bounding sphere of the skeleton
		const TSphere* pSphere = m_pAnimInterface->GetSkeletonInstance()->GetAnimationBoundingSphere();

		if ( pSphere != TNULL )
			return pSphere;
	}

	T2ModelPtr pModel = m_pModelInstance->GetModel();
	return &pModel->GetLOD( 0 ).m_RenderVolume;
}

// $deBlob: FUNCTION 005726f0
AModelInstance2::AModelInstance2( AModel2* a_pModel, TBOOL a_bCreateSoundBreakpoints, TBOOL a_bCreateAnimInterface )
{
	m_pModel                    = a_pModel;
	m_pAnimInterface            = TNULL;
	m_pModelInstance            = TNULL;
	m_vScale.Set( 1.0f, 1.0f, 1.0f, 1.0f );
	m_pCollisionInstance        = TNULL;
	m_pSoundBreakpointComponent = TNULL;
	m_uiFlags                   = 0x2885;
	m_uiUnk3C                   = 0;
	m_uiUnk40                   = 0;
	m_uiOverrideMaterial        = 0;

	for ( TINT i = 0; i < TARRAYSIZE( m_aMeshSettings ); i++ )
	{
		*TREINTERPRETCAST( TINT*, m_aMeshSettings[ i ].aColour ) = -1;
		m_aMeshSettings[ i ].uiFlags                             = 2;
	}

	m_fAmbientScale = 1.0f;

	if ( a_bCreateAnimInterface )
		m_pAnimInterface = new AToshiAnimInterface2();

	if ( a_bCreateSoundBreakpoints )
	{
		ASoundBreakpointComponent* pBreakpoint = new ASoundBreakpointComponent( this );
		m_pSoundBreakpointComponent            = pBreakpoint;

		// A model without breakpoint entries has no sounds to keep
		if ( !pBreakpoint->HasEntry() )
		{
			delete pBreakpoint;
			m_pSoundBreakpointComponent = TNULL;
		}
	}

	m_uiAmbientColour      = 0xFFA3A3A3;
	m_uiLocalAmbientColour = ms_uiDefaultLocalAmbientColour;
}

// $deBlob: FUNCTION 00572860
// $deBlob: FUNCTION 00572c20
AModelInstance2::~AModelInstance2()
{
	AModelInstance2::OnUnloaded();

	if ( m_pSoundBreakpointComponent )
	{
		delete TSTATICCAST( ASoundBreakpointComponent, m_pSoundBreakpointComponent );
	}

	if ( m_pAnimInterface )
		delete m_pAnimInterface;
}

// $deBlob: FUNCTION 00572910
void AModelInstance2::OnLoaded( TModelInstance* a_pModelInstance )
{
	m_pModelInstance = a_pModelInstance;

	a_pModelInstance->SetCustomRenderMethod( RenderGeneric, this );

	if ( m_pAnimInterface )
		m_pAnimInterface->SetSkeletonInstance( a_pModelInstance->GetSkeletonInstance() );

	if ( m_pSoundBreakpointComponent )
		TSTATICCAST( ASoundBreakpointComponent, m_pSoundBreakpointComponent )->PatchAnimIDs( a_pModelInstance );
}

// $deBlob: FUNCTION 00572950
void AModelInstance2::OnUnloaded()
{
	if ( ACameraManager::GetSingleton() != TNULL )
		ACameraManager::GetSingleton()->OnModelInstanceUnloaded( this );

	if ( m_pCollisionInstance )
	{
		delete TSTATICCAST( ACollisionInstance2, m_pCollisionInstance );
	}

	m_pCollisionInstance = TNULL;

	if ( m_pModelInstance )
		delete m_pModelInstance;

	m_pModelInstance = TNULL;

	if ( m_pAnimInterface )
		m_pAnimInterface->SetSkeletonInstance( TNULL );
}

// UpdateImp without the check of m_pModelInstance
// $deBlob: FUNCTION 005729c0
void AModelInstance2::UpdateAnimation( TFLOAT a_fDeltaTime )
{
	// The breakpoint sounds tick while the instance's animations run (not with FLAG_NoAnimSounds)
	if ( m_pAnimInterface != TNULL && !( TUINT8( m_uiFlags >> 10 ) & 1 ) )
	{
		Toshi::TSkeletonInstance* pSkeleton = m_pAnimInterface->GetSkeletonInstance();

		if ( pSkeleton != TNULL && TUINT16( pSkeleton->GetOverlayAnimationCount() ) + TUINT16( pSkeleton->GetBaseAnimationCount() ) > 0 )
		{
			ASoundBreakpointComponent* pBreakpoint = TSTATICCAST( ASoundBreakpointComponent, m_pSoundBreakpointComponent );

			if ( pBreakpoint != TNULL && pBreakpoint->HasEntry() )
				pBreakpoint->TickImp();
		}
	}

	if ( m_pModelInstance->GetSkeletonInstance() )
		m_pModelInstance->Update( a_fDeltaTime );
}

// $deBlob: FUNCTION 00572f40
void AModel2::SortInstance( AModelInstance2* a_pInstance, TBOOL a_bRenderable )
{
	a_pInstance->Remove();

	if ( a_bRenderable )
		m_lInstances.PushFront( a_pInstance );
	else
		m_lInstances.PushBack( a_pInstance );
}

// $deBlob: FUNCTION 00573040
void AModelInstance2::UpdateImp( TFLOAT a_fDeltaTime )
{
	if ( m_pModelInstance )
	{
		// The breakpoint sounds tick while the instance's animations run (not with FLAG_NoAnimSounds)
		if ( m_pAnimInterface != TNULL && !( TUINT8( m_uiFlags >> 10 ) & 1 ) )
		{
			Toshi::TSkeletonInstance* pSkeleton = m_pAnimInterface->GetSkeletonInstance();

			if ( pSkeleton != TNULL && TUINT16( pSkeleton->GetOverlayAnimationCount() ) + TUINT16( pSkeleton->GetBaseAnimationCount() ) > 0 )
			{
				ASoundBreakpointComponent* pBreakpoint = TSTATICCAST( ASoundBreakpointComponent, m_pSoundBreakpointComponent );

				if ( pBreakpoint != TNULL && pBreakpoint->HasEntry() )
					pBreakpoint->TickImp();
			}
		}

		if ( m_pModelInstance->GetSkeletonInstance() )
			m_pModelInstance->Update( a_fDeltaTime );
	}
}
