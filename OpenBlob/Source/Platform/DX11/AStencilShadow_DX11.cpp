#include "pch.h"
#include "AStencilShadow_DX11.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Collision/ACollisionInstance2.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsPhantomContainer.h>
#include <Render/TModel.h>
#include <Render/TModelManager.h>
#include <Render/TMesh.h>
#include <Render/TSkeleton.h>
#include <Render/TAnimation.h>
#include <Toshi/TUtil.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TUINT8 AStencilShadow::ms_uiStencilColour = 0x20;
TBOOL  AStencilShadow::ms_bRenderPass0    = TTRUE;
TBOOL  AStencilShadow::ms_bRenderPass1    = TTRUE;

TSTATICASSERT( sizeof( AStencilShadow ) == 0x8CC );

// The length of the rays down from the blob
static TFLOAT s_fRayLength = 40.0f;
// The distance from the bottom of the blob to the ground at which the shadow has shrunk by s_fScaleShrink
static TFLOAT s_fScaleDistance = 20.0f;
// The shadow's scale per unit of the blob's radius
static TFLOAT s_fScale       = 0.28f;
static TFLOAT s_fScaleShrink = 2.0f;

// $deBlob: FUNCTION 0055f4e0
AStencilShadow::AStencilShadow()
{
	TASSERT( APlayerManager::IsSingletonCreated() );
	TASSERT( AGameStatus::IsSingletonCreated() );

	T2ModelPtr pShadowModel;

	if ( AGameStatus::GetSingleton()->IsMultiplayer() )
		pShadowModel = TModelManager::GetSingleton()->FindModel( "blob_shad_mp.txs" );
	else
		pShadowModel = TModelManager::GetSingleton()->FindModel( "blob_shad.txs" );

	for ( TINT i = 0; i < MAX_SHADOWS; i++ )
	{
		m_apInstances[ i ] = pShadowModel->CreateInstance();
		m_apInstances[ i ]->SetDistanceFade( TFALSE );
		m_apGroundPhantoms[ i ] = TNULL;
		m_apDecalPhantoms[ i ]  = TNULL;
		m_abHidden[ i ]         = TTRUE;
	}

	ASkinShader* pSkinShader = ASkinShader::GetSingleton();

	for ( TINT i = 0; i < MAX_MATERIALS; i++ )
	{
		TMaterial* pMat = pSkinShader->CreateMaterial( TNULL );
		TASSERT( ( pMat == TNULL ) || pMat->IsA( &TGetClass( ASkinMaterialHAL ) ) );

		m_oMatAlloc[ i ].m_pMat = pMat;
		TSTATICCAST( ASkinMaterialHAL, pMat )->Create( ASkinMaterial::BLENDMODE_STENCIL );
		m_oMatFreeList.PushFront( &m_oMatAlloc[ i ] );
	}

	TModelLOD& rLOD       = pShadowModel->GetLOD( 0 );
	TINT       iNumMeshes = rLOD.GetMeshCount();
	TASSERT( iNumMeshes <= MAX_MESHES );

	for ( TINT iPass = 0; iPass < NUM_PASSES; iPass++ )
	{
		for ( TINT i = 0; i < MAX_MESHES; i++ )
		{
			TASSERT( !m_oMatFreeList.IsEmpty() );
			MatAlloc* pAlloc = m_oMatFreeList.Back();
			pAlloc->Remove();

			ASkinMaterialHAL* pMat = TSTATICCAST( ASkinMaterialHAL, pAlloc->m_pMat );
			pMat->SetBlendMode( ASkinMaterial::BLENDMODE_STENCIL, iPass );
			m_apMaterials[ iPass ][ i ] = pMat;
		}
	}

	for ( TINT i = 0; i < iNumMeshes; i++ )
		m_apOriginalMaterials[ i ] = pShadowModel->GetLOD( 0 ).GetMesh( i )->GetMaterial();

	// The shadow plays the blob's animations: map the blob's sequences to the shadow's by name
	T2ModelPtr pBlobModel;

	if ( AGameStatus::GetSingleton()->IsMultiplayer() )
		pBlobModel = TModelManager::GetSingleton()->FindModel( "blob_bodya_mp.txs" );
	else
		pBlobModel = TModelManager::GetSingleton()->FindModel( "blob_bodya.txs" );

	TINT iSeqCount = pBlobModel->GetSkeleton()->GetSequenceCount();
	TASSERT( iSeqCount < MAX_SEQUENCES );

	if ( iSeqCount > 0 )
		TUtil::MemSet( m_aiBlobSequences, 0xFF, iSeqCount * sizeof( TINT ) );

	for ( TINT i = 0; i < iSeqCount; i++ )
	{
		TINT iShadowSeq          = pShadowModel->GetSkeleton()->GetSequenceID( pBlobModel->GetSkeleton()->GetSequence( i )->GetName() );
		m_aiShadowSequences[ i ] = iShadowSeq;

		if ( iShadowSeq > -1 )
			m_aiBlobSequences[ iShadowSeq ] = i;
		else
			TASSERT( pBlobModel->GetSkeleton()->GetSequence( i )->IsOverlay(), "Shadow is missing base animation \"%s\"" );
	}
}

// $deBlob: FUNCTION 0055fac0
AStencilShadow::~AStencilShadow()
{
	for ( TINT i = 0; i < MAX_SHADOWS; i++ )
	{
		if ( m_apGroundPhantoms[ i ] != TNULL )
		{
			TPhysicsManager::GetSingleton()->DestroyPhantom( m_apGroundPhantoms[ i ] );
			m_apGroundPhantoms[ i ] = TNULL;
		}

		if ( m_apDecalPhantoms[ i ] != TNULL )
		{
			TPhysicsManager::GetSingleton()->DestroyPhantom( m_apDecalPhantoms[ i ] );
			m_apDecalPhantoms[ i ] = TNULL;
		}
	}

	// The shadow model gets its own materials back
	T2ModelPtr pModel = m_apInstances[ 0 ]->GetModel();
	TModelLOD& rLOD   = pModel->GetLOD( 0 );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		rLOD.GetMesh( i )->SetMaterial( m_apOriginalMaterials[ i ] );

	for ( TINT i = 0; i < MAX_SHADOWS; i++ )
	{
		if ( m_apInstances[ i ] != TNULL )
			delete m_apInstances[ i ];

		m_apInstances[ i ] = TNULL;
	}

	m_oMatFreeList.Clear();

	for ( TINT i = 0; i < MAX_MATERIALS; i++ )
	{
		TASSERT( TFALSE == m_oMatAlloc[ i ].IsLinked() );

		TSTATICCAST( ASkinMaterialHAL, m_oMatAlloc[ i ].m_pMat )->OnDestroy();
		delete m_oMatAlloc[ i ].m_pMat;
	}
}

#ifdef DEBLOB_COMMUNITY_PATCH
// Whether both ends of a_rcRay are inside the broadphase (the test of TPhysicsManager::CollideRay)
static TBOOL IsRayInWorld( const TPhysicsRay& a_rcRay, const TVector4& a_rcMin, const TVector4& a_rcMax )
{
	for ( const TVector4* pPoint : { &a_rcRay.m_vFrom, &a_rcRay.m_vTo } )
	{
		if ( pPoint->x < a_rcMin.x || pPoint->y < a_rcMin.y || pPoint->z < a_rcMin.z || a_rcMax.x < pPoint->x || a_rcMax.y < pPoint->y || a_rcMax.z < pPoint->z )
			return TFALSE;
	}

	return TTRUE;
}
#endif

// $deBlob: FUNCTION 0055fd20
void AStencilShadow::Update( TFLOAT a_fDeltaTime )
{
	TPhysicsManager* pPM            = TPhysicsManager::GetSingleton();
	APlayerManager*  pPlayerManager = APlayerManager::GetSingleton();

	TINT iOldWorld = pPM->GetDefaultWorld();
	pPM->SetDefaultWorld( APhysicsWorldType_GamePlay );

	if ( m_apGroundPhantoms[ 0 ] == TNULL )
	{
		for ( TINT i = 0; i < pPlayerManager->GetNumPlayers(); i++ )
		{
			ABaseEntity* pBlob      = APlayerManager::GetSingleton()->GetPlayerBlob( i );
			m_apGroundPhantoms[ i ] = pPM->CreateAabbPhantom();
			pPM->SetPhantomCollisionFilterInfo( m_apGroundPhantoms[ i ], 0x4cf000 );
			pPM->AddPhantomToSimulation( m_apGroundPhantoms[ i ], pBlob->GetTranslation(), TNULL );
		}
	}

	pPM->SetDefaultWorld( APhysicsWorldType_Decal );

	if ( m_apDecalPhantoms[ 0 ] == TNULL )
	{
		for ( TINT i = 0; i < pPlayerManager->GetNumPlayers(); i++ )
		{
			ABaseEntity* pBlob     = APlayerManager::GetSingleton()->GetPlayerBlob( i );
			m_apDecalPhantoms[ i ] = pPM->CreateAabbPhantom();
			pPM->SetPhantomCollisionFilterInfo( m_apDecalPhantoms[ i ], 0x8000 );
			pPM->AddPhantomToSimulation( m_apDecalPhantoms[ i ], pBlob->GetTranslation(), TNULL );
		}
	}

	pPM->SetDefaultWorld( APhysicsWorldType_GamePlay );

	for ( TINT i = 0; i < pPlayerManager->GetNumPlayers(); i++ )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( i ) );

		// The rays go s_fRayLength down from the blob; the phantoms are the box around them
		TVector4 vFacing = pBlob->m_vFacing;

		TPhysicsRay oRay;
		oRay.m_vFrom = pBlob->GetTranslation();

		TVector4 vDown;
		vDown.Multiply( ACamera::sm_vWorldUp, s_fRayLength );
		oRay.m_vTo.Set( oRay.m_vFrom.x - vDown.x, oRay.m_vFrom.y - vDown.y, oRay.m_vFrom.z - vDown.z, oRay.m_vFrom.w );

		TVector4 avBox[ 2 ];
		TVector4 vSum( oRay.m_vTo.x + oRay.m_vFrom.x, oRay.m_vTo.y + oRay.m_vFrom.y, oRay.m_vTo.z + oRay.m_vFrom.z, oRay.m_vFrom.w );
		avBox[ 0 ].Multiply( vSum, 0.5f );
		TVector4 vDiff( oRay.m_vFrom.x - oRay.m_vTo.x, oRay.m_vFrom.y - oRay.m_vTo.y, oRay.m_vFrom.z - oRay.m_vTo.z, oRay.m_vFrom.w );
		avBox[ 1 ].Multiply( vDiff, 0.5f );
		avBox[ 1 ].Abs4();

		pPM->SetAabbPhantomBox( m_apGroundPhantoms[ i ], avBox );
		pPM->SetAabbPhantomBox( m_apDecalPhantoms[ i ], avBox );

		TPhysicsShapeContact oDecalContact;
		TPhysicsShapeContact oGroundContact;

#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail casts the rays even when they leave the broadphase (a blob falling out of the level, the rays end 40
		// below it): CollideRay then asserts and returns no hit. Skip them instead, with the same result (no shadow)
		const TPhysicsManagerInitData& rcInitData = pPM->GetInitData();
		TBOOL                          bInWorld   = IsRayInWorld( oRay, rcInitData.m_vBroadPhaseMin, rcInitData.m_vBroadPhaseMax );
#else
		TBOOL bInWorld = TTRUE;
#endif

		pPM->SetDefaultWorld( APhysicsWorldType_Decal );
		oRay.m_uiCollisionLayer = 8;
		TBOOL bDecalHit         = bInWorld && pPM->CollideRay( oRay, oDecalContact, m_apDecalPhantoms[ i ] );

		pPM->SetDefaultWorld( APhysicsWorldType_GamePlay );
		oRay.m_uiCollisionLayer = 0x4cf;
		TBOOL bGroundHit        = bInWorld && pPM->CollideRay( oRay, oGroundContact, m_apGroundPhantoms[ i ] );

		if ( !bDecalHit && !bGroundHit )
		{
			// Nothing under the blob
			m_apInstances[ i ]->GetTransform().SetMatrix( pBlob->GetMatrix() );
			m_abHidden[ i ] = TTRUE;
		}
		else
		{
			// The closer hit (the ground one on a tie)
			const TPhysicsShapeContact* pContact = &oDecalContact;

			if ( !bDecalHit || ( bGroundHit && oGroundContact.m_fDistance <= oDecalContact.m_fDistance ) )
				pContact = &oGroundContact;

			TVector4 vHit = pContact->m_vPosition;

			TMatrix44 mShadow = TMatrix44::IDENTITY;
			TVector4  vBack;
			vBack.Multiply( vFacing, -1.0f );
			mShadow.LookAtDirection( vBack, ACamera::sm_vWorldUp );
			mShadow.m_f41 = vHit.x;
			mShadow.m_f42 = vHit.y;
			mShadow.m_f43 = vHit.z;

#ifdef DEBLOB_COMMUNITY_PATCH
			// The shadow moves horizontally with the position Blob is drawn at this frame (the ray is cast from the body's
			// last fixed step, see ABlobEntity::GetRenderPhysicsMatrix)
			const TMatrix44& rcRender = pBlob->GetLastRenderMatrix();
			mShadow.m_f41 += rcRender.m_f41 - oRay.m_vFrom.x;
			mShadow.m_f43 += rcRender.m_f43 - oRay.m_vFrom.z;
#endif
			m_apInstances[ i ]->GetTransform().SetMatrix( mShadow );

			// Smaller the further the blob is from the ground
			TFLOAT fRadius = ABlobEntity::ms_pBlobProperties[ pBlob->m_iSize ].m_fRadius;
			TFLOAT fScale  = fRadius * s_fScale;

			TVector4 vUp;
			vUp.Multiply( ACamera::sm_vWorldUp, fRadius );
			TVector4 vBottom( oRay.m_vFrom.x - vUp.x, oRay.m_vFrom.y - vUp.y, oRay.m_vFrom.z - vUp.z, oRay.m_vFrom.w );

			TFLOAT fDistance = TVector4::Distance( vBottom, vHit );
			fScale           = TMath::LerpTowards( fScale, fScale / s_fScaleShrink, fDistance / s_fScaleDistance );
			m_apInstances[ i ]->GetTransform().SetScale( fScale, fScale, fScale );

			m_abHidden[ i ] = ( pBlob->m_uiFlags & 0x180 ) != 0;
		}

		// The shadow plays the base animations of the blob's character model at the same times
		TSkeletonInstance* pBlobSkelInst   = pBlob->m_pAnimationComponent->m_pCharModelI->GetAnimInterface()->GetSkeletonInstance();
		TSkeletonInstance* pShadowSkelInst = m_apInstances[ i ]->GetSkeletonInstance();

		for ( auto it = pBlobSkelInst->GetBaseAnimations().Begin(); it != pBlobSkelInst->GetBaseAnimations().End(); it++ )
		{
			TINT iBlobSeqNum = it->GetSequence();
			TASSERT( iBlobSeqNum < MAX_SEQUENCES );

			TINT iAnimSeqNum = m_aiShadowSequences[ iBlobSeqNum ];
			TASSERT( iAnimSeqNum < pShadowSkelInst->GetSkeleton()->GetSequenceCount() );

			TAnimation* pAnim = pShadowSkelInst->IsAnimating( iAnimSeqNum );

			if ( pAnim == TNULL )
				pAnim = pShadowSkelInst->AddAnimationFull( iAnimSeqNum, 1.0f, 0.0f, 0.0f, 0 );

			pAnim->SetTime( it->GetSeqTime() );
		}

		// The animations the blob stopped
		TAnimation* apAnimsToRemove[ MAX_ANIMS ];
		TINT        iNumAnimsToRemove = 0;

		for ( auto it = pShadowSkelInst->GetBaseAnimations().Begin(); it != pShadowSkelInst->GetBaseAnimations().End(); it++ )
		{
			if ( pBlobSkelInst->IsAnimating( m_aiBlobSequences[ it->GetSequence() ] ) == TNULL && it->IsActive() )
			{
				TASSERT( iNumAnimsToRemove < MAX_ANIMS );
				apAnimsToRemove[ iNumAnimsToRemove++ ] = it;
			}
		}

		for ( TINT j = 0; j < iNumAnimsToRemove; j++ )
			pShadowSkelInst->RemoveAnimation( apAnimsToRemove[ j ], 0.0f );

		if ( TUINT16( pBlobSkelInst->GetOverlayAnimationCount() ) + TUINT16( pBlobSkelInst->GetBaseAnimationCount() ) == 0 )
		{
			TINT16 iSeq = ABlobAnimationComponent::ms_pAnimationProperties->m_pCommon->m_pAnims[ 4 ].m_iAnim;
			pShadowSkelInst->AddAnimationFull( TINT16( TUINT16( m_aiShadowSequences[ iSeq ] ) ), 1.0f, 0.0f, 0.0f, 0 );
		}

		pShadowSkelInst->UpdateTime( a_fDeltaTime );
		pShadowSkelInst->UpdateState( TFALSE );
	}

	TASSERT( iOldWorld == APhysicsWorldType_GamePlay );
	pPM->SetDefaultWorld( iOldWorld );
}

// $deBlob: FUNCTION 00560890
void AStencilShadow::Render( TFLOAT a_fDeltaTime, ACamera* a_pCamera )
{
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	TBOOL           bSplitScreen   = ACameraManager::GetSingleton()->GetNumActiveCameras() > 1;

	for ( TINT i = 0; i < pPlayerManager->GetNumPlayers(); i++ )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( i ) );

		if ( m_abHidden[ i ] || !( pBlob->GetFlags() & ABaseEntity::FLAG_Visible ) )
			continue;

		// A lighter shadow for the white (unpainted) blob, faded with the blob in single player
		TUINT8 uiColour = 32;

		if ( !bSplitScreen )
		{
			TUINT32 uiBlobColour = pBlob->m_pColourComponent->GetCurrentColour();

			// NOTE: retail checks COLOUR_BLACK first (32) and then for 0
			if ( uiBlobColour == 0 )
				uiColour = 16;

			TFLOAT fFade = pBlob->m_pAnimationComponent->m_fAlpha;

			if ( fFade != 1.0f )
				uiColour = TUINT8( TINT( uiColour * fFade ) );
		}

		ms_uiStencilColour = uiColour;

		if ( ms_bRenderPass0 )
		{
			SetPassMaterials( m_apInstances[ i ], 0 );
			m_apInstances[ i ]->Render();
		}

		if ( ms_bRenderPass1 )
		{
			SetPassMaterials( m_apInstances[ i ], 1 );
			m_apInstances[ i ]->Render();
		}
	}
}

// $deBlob: FUNCTION 00560a20
void AStencilShadow::SetPassMaterials( TModelInstance* a_pInstance, TINT a_iPass )
{
	TASSERT( a_iPass < NUM_PASSES );

	T2ModelPtr pModel = a_pInstance->GetModel();
	TModelLOD& rLOD   = pModel->GetLOD( 0 );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		ASkinMaterial* pMat = m_apMaterials[ a_iPass ][ i ];
		TASSERT( pMat->GetBlendMode() == ASkinMaterial::BLENDMODE_STENCIL );
		TASSERT( pMat->GetPass() == a_iPass );
		rLOD.GetMesh( i )->SetMaterial( pMat );
	}
}

// $deBlob: FUNCTION 00560990
void AStencilShadow::GetShadowPosition( TINT a_iIndex, TVector4& a_rOutPosition )
{
	TASSERT( a_iIndex < MAX_SHADOWS );
	a_rOutPosition = m_apInstances[ a_iIndex ]->GetTransform().GetMatrix().GetTranslation();
}
