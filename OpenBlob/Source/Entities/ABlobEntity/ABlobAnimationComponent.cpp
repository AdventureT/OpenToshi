#include "pch.h"
#include "ABlobAnimationComponent.h"
#include "ABlobEntity.h"
#include "ABlobColourMaps.h"
#include "ARandom.h"
#include "Cameras/ACameraManager.h"
#include "Challenges/AChallengeManager.h"
#include "Animation/AModel2.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/APropTriggerLevelComplete.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Render/Material/AMaterialInfo.h"
#include "Cameras/ACamera.h"
#include "Render/AXYZViewportManager.h"
#include "Render/ARenderer.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Render/TMesh.h>
#include <Render/TSkeleton.h>
#include <Render/TAnimation.h>
#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL                                         ABlobAnimationComponent::ms_bShieldMaterialCreated;
ABlobAnimationComponent::AnimationProperties* ABlobAnimationComponent::ms_pAnimationProperties;
Toshi::TSkeleton*                             ABlobAnimationComponent::ms_pSkeletonChar;
TINT                                          ABlobAnimationComponent::s_iLastLevelStartAnim = -1;
const Toshi::TMatrix44*                       ABlobAnimationComponent::ms_pLeftEyeBoneTransform;
const Toshi::TMatrix44*                       ABlobAnimationComponent::ms_pRightEyeBoneTransform;
ASkinMaterialHAL*                             ABlobAnimationComponent::ms_pLeftEyeMaterial;
ASkinMaterialHAL*                             ABlobAnimationComponent::ms_pRightEyeMaterial;
TINT16                                        ABlobAnimationComponent::ms_iBoneSBBase  = -1;
TINT16                                        ABlobAnimationComponent::ms_iBoneSBBody  = -1;
TINT16                                        ABlobAnimationComponent::ms_iBoneEffect1 = -1;
TINT16                                        ABlobAnimationComponent::ms_iBoneEffect2 = -1;
TINT16                                        ABlobAnimationComponent::ms_iAngeredEyes = -1;

// Rounds like retail's FLD/FISTP (to nearest, ties to even)
static TINT RoundToInt( TFLOAT a_fValue )
{
	return TINT( lrintf( a_fValue ) );
}

// The current challenge (AChallengeManager::m_pCurrentChallenge, its type at 0x170 picks the animations): TNULL when
// Blob is not in one.
static void* GetCurrentSurface()
{
	return AChallengeManager::GetSingleton()->GetCurrentChallenge();
}


// Resolves the sequence and eye overlay IDs of a_pAnims (a_bCheckEyes: LoadAnimationProperties's eye overlay asserts)
static void ResolveAnimations( ABlobAnimationComponent::Anim* a_pAnims, TINT a_iNumAnims, TBOOL a_bCheckEyes )
{
	TSkeleton* pSkeleton = ABlobAnimationComponent::ms_pSkeletonChar;

	for ( TINT i = 0; i < a_iNumAnims; i++ )
	{
		ABlobAnimationComponent::Anim* pAnims = a_pAnims;

		pAnims[ i ].m_iAnim = TINT16( pSkeleton->GetSequenceID( pAnims[ i ].m_szName ) );
		TASSERT( pAnims[ i ].m_iAnim >= 0, "Missing animation [%s]" );

		TCHAR szEyes[ 128 ];
		T2String8::Format( szEyes, sizeof( szEyes ), "%s_eyes", pAnims[ i ].m_szName );
		pAnims[ i ].m_iEyeOLay = TINT16( pSkeleton->GetSequenceID( szEyes ) );

		if ( a_bCheckEyes )
		{
			TASSERT( pAnims[ i ].m_iEyeOLay >= 0, "Animation [%s] is missing an eye overlay." );
			TASSERT( pSkeleton->GetSequence( pAnims[ i ].m_iEyeOLay )->IsOverlay() );
		}
	}
}

// Sets up a ball model instance
static void SetupBallModel( AModelInstance2* a_pModelI )
{
	a_pModelI->GetModelInstance()->SetForceFade( TTRUE );
	a_pModelI->GetModelInstance()->SetFade( ABlobEntity::GetCommonProperties()->m_afUnkA0[ 0 ] );
	a_pModelI->UseLocalAmbientColour( TTRUE );
	a_pModelI->SetLocalAmbientColour( *ABlobColourMaps::GetBodyColourMap() );
	a_pModelI->UseLocalAmbientLight( TTRUE );
	a_pModelI->SetAmbientColour( *ABlobColourMaps::GetLightColourMap() );
}

// The material of the first mesh of a_pModelI's LOD 0
static TMaterial* GetFirstMeshMaterial( AModelInstance2* a_pModelI )
{
	T2ModelPtr pModel = a_pModelI->GetModel()->GetModelPtr();
	return pModel->GetLOD( 0 ).GetMeshes()[ 0 ]->GetMaterial();
}

// The tev mode of a skin material's first pass that overrides channel 2 (the ball's paint)
static TCONSTEXPR_VAR TINT MULTIPASSBLENDTYPE_OVERRIDE_CHAN2 = 0x11;

static TCONSTEXPR_VAR TFLOAT PAINT_FLASH_TIME = 0.3f;

// $deBlob: FUNCTION 007523d0
ABlobAnimationComponent::ABlobAnimationComponent( ABlobEntity* a_pOwner )
{
	m_pOwner     = a_pOwner;
	m_iState     = STATE_BALL;
	m_uiEyeState = 0;
	m_fUnk0C     = ABlobEntity::GetCommonProperties()->m_fUnkB8;

	m_pCurrentAnim    = TNULL;
	m_pStateAnim      = TNULL;
	m_uiUnk18         = 0;
	m_iLookIndex      = 0;
	m_fIdleAnimDelay  = 0.0f;
	m_fCelebrateTimer = 0.0f;
	m_fUnk28          = 0.0f;
	m_fUnk2C          = 0.0f;
	m_fBlinkTimer     = 0.0f;
	m_fLookTimer      = 0.0f;

	m_fAlpha           = 1.0f;
	m_fTargetAlpha     = 1.0f;
	m_fAlphaSpeed      = 1.0f;
	m_iAnimFlags       = 1;
	m_uiUnk48          = 0;
	m_pBallBodyModelI  = TNULL;
	m_pBallEarsModelI  = TNULL;
	m_pCharModelI      = TNULL;
	m_pShieldModelI    = TNULL;
	m_pCrownModelI     = TNULL;
	m_pShockedMaterial = TNULL;
	m_pZPassMaterial   = TNULL;
	m_fPaintFlashTimer = 0.0f;
	m_bUnk6C           = TFALSE;
	m_fShieldTime      = 0.0f;
	m_iCurShieldKey    = 0;

	TASSERT( AGameStatus::IsSingletonCreated() );
	AModelFactory* pModelFactory = AModelFactory::GetSingleton();

	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
	{
		m_pBallBodyModelI = pModelFactory->CreateUnmanagedInstance( "data/blobchar/blobball_body.trb", TTRUE, 0, TTRUE );
		m_pBallEarsModelI = pModelFactory->CreateUnmanagedInstance( "data/blobchar/blobball_ears.trb", TTRUE, 0, TTRUE );
		m_pCharModelI     = pModelFactory->CreateUnmanagedInstance( "data/blobchar/blob_bodya.trb", TTRUE, 0, TTRUE );
		m_pShieldModelI   = TNULL;
	}
	else
	{
		m_pBallBodyModelI = pModelFactory->CreateUnmanagedInstance( "data/blobchar/blobball_body_mp.trb", TTRUE, 0, TTRUE );
		m_pBallEarsModelI = pModelFactory->CreateUnmanagedInstance( "data/blobchar/blobball_ears_mp.trb", TTRUE, 0, TTRUE );
		m_pCharModelI     = pModelFactory->CreateUnmanagedInstance( "data/blobchar/blob_bodya_mp.trb", TTRUE, 0, TTRUE );
		m_pShieldModelI   = pModelFactory->CreateUnmanagedInstance( "data/blobchar/mp_shield.trb", TTRUE, 0, TTRUE );
		m_pCrownModelI    = pModelFactory->CreateUnmanagedInstance( "data/blobchar/crown.trb", TTRUE, 0, TTRUE );
		m_pCrownModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	}

	ASkinShaderHAL* pSkinShader = TSTATICCAST( ASkinShaderHAL, ASkinShader::GetSingleton() );

	// The ball body
	m_pBallBodyModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
	m_pBallBodyModelI->SetFlag( 0x1000, TTRUE );
	m_pBallBodyModelI->SetMeshFlags( 0, 1, TTRUE );
	m_pBallBodyModelI->SetMeshColour( 0, 3, 0xff );
	SetupBallModel( m_pBallBodyModelI );

	TMaterial*   pBallMaterial = GetFirstMeshMaterial( m_pBallBodyModelI );
	const TCHAR* szSilhouette  = AGameStatus::GetSingleton()->IsMultiplayer() ? "blob_silhouette_mp.tga" : "blob_silhouette.tga";
	TTexture*    pSilhouette   = TTextureManager::GetSingleton()->FindTexture( szSilhouette );

	m_pShockedMaterial = TSTATICCAST( ASkinMaterialHAL, pSkinShader->CreateMaterial( "Blob_Shocked" ) );
	m_pShockedMaterial->Create( 0 );
	AMaterialInfo::CopyMaterial( pBallMaterial, m_pShockedMaterial );
	m_pShockedMaterial->SetTevMode( 1, 1 );
	m_pShockedMaterial->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_DEFAULT );
	m_pShockedMaterial->SetTexture( 1, TNULL );
	m_pShockedMaterial->SetTextureNum( 1 );
	m_pShockedMaterial->SetFlags( 0x1000, TFALSE );
	m_pShockedMaterial->SetFlags( 0x2000, TFALSE );
	m_pShockedMaterial->SetTexture( 0, pSilhouette );

	m_pZPassMaterial = TSTATICCAST( ASkinMaterialHAL, pSkinShader->CreateMaterial( "Blob_ZPass" ) );
	m_pZPassMaterial->Create( 0 );
	AMaterialInfo::CopyMaterial( pBallMaterial, m_pZPassMaterial );
	m_pZPassMaterial->SetTevMode( 1, 1 );
	m_pZPassMaterial->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_DEFAULT );
	m_pZPassMaterial->SetTexture( 1, TNULL );
	m_pZPassMaterial->SetTextureNum( 1 );
	m_pZPassMaterial->SetBlendMode( 6, 0 );

	m_pShockedMaterial->SetOrderTable( pSkinShader->GetOrderTableAt( 0 ), TFALSE );
	{
		T2ModelPtr pCharModel = m_pCharModelI->GetModel()->GetModelPtr();
		TSTATICCAST( ASkinMaterialHAL, pCharModel->GetLOD( 0 ).GetMeshes()[ 4 ]->GetMaterial() )->SetOrderTable( pSkinShader->GetOrderTableAt( 1 ), TFALSE );
	}
	m_pZPassMaterial->SetOrderTable( pSkinShader->GetOrderTableAt( 2 ), TFALSE );

	TSTATICCAST( ASkinMaterialHAL, pBallMaterial )->SetBlendMode( 1, 0 );
	pBallMaterial->SetFlags( 0x1000, TTRUE );

	// The ball ears
	m_pBallEarsModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
	m_pBallEarsModelI->SetFlag( 0x1000, TTRUE );
	m_pBallEarsModelI->SetMeshFlags( 0, 1, TTRUE );
	SetupBallModel( m_pBallEarsModelI );

	TMaterial* pEarsMaterial = GetFirstMeshMaterial( m_pBallEarsModelI );
	TSTATICCAST( ASkinMaterialHAL, pEarsMaterial )->SetBlendMode( 1, 0 );
	pEarsMaterial->SetFlags( 0x1000, TTRUE );

	// The character
	T2ModelPtr pCharModel = m_pCharModelI->GetModel()->GetModelPtr();
	TModelLOD& rLOD       = pCharModel->GetLOD( 0 );
	TASSERT( rLOD.GetMeshCount() == MESH_COUNT, "The number of meshes in the blob model has changed. Code needs to be updated to support this." );

	m_pCharModelI->SetFlag( 0x1000, TTRUE );

	for ( TINT i = 0; i < 4; i++ )
	{
		m_pCharModelI->SetMeshFlags( i, 1, TTRUE );
		m_pCharModelI->SetMeshColour( i, 3, 0 );
	}

	m_pCharModelI->SetMeshFlags( 4, 1, TTRUE );
	m_pCharModelI->SetMeshFlags( 4, 2, TFALSE );
	m_pCharModelI->SetMeshColour( 4, 1, 0x80 );
	m_pCharModelI->SetMeshColour( 4, 2, 0x80 );
	m_pCharModelI->SetMeshColour( 4, 0, 0x80 );
	m_pCharModelI->SetMeshColour( 4, 3, 0xff );
	m_pCharModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	m_pCharModelI->GetModelInstance()->SetForceFade( TTRUE );
	m_pCharModelI->GetModelInstance()->SetFade( 0.0f );
	m_pCharModelI->UseLocalAmbientColour( TTRUE );
	m_pCharModelI->UseLocalAmbientLight( TTRUE );
	m_pCharModelI->SetAmbientColour( *ABlobColourMaps::GetLightColourMap() );

	// The multiplayer shield's material is set up once
	if ( m_pShieldModelI != TNULL && !ms_bShieldMaterialCreated )
	{
		ASkinMaterialHAL* pForceFieldMat = TSTATICCAST( ASkinMaterialHAL, GetFirstMeshMaterial( m_pShieldModelI ) );
		TTexture*         pSphereMap     = TTextureManager::GetSingleton()->FindTexture( "spheremap_mp_shield.tga" );
		pSphereMap->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

		TASSERT( pForceFieldMat->GetTextureNum() == 1 );
		pForceFieldMat->SetTextureNum( 2 );
		pForceFieldMat->SetTevMode( 1, 0xc );
		pForceFieldMat->SetTexture( 1, pSphereMap );
		pForceFieldMat->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_ENVMAP );
		pForceFieldMat->SetBlendMode( 0x18, 0 );
		pForceFieldMat->SetOrderTable( pSkinShader->GetAlphaOrderTable( 2 ), TFALSE );
		ms_bShieldMaterialCreated = TTRUE;
		m_pShieldModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	}

	// The character's meshes: rim light on the body meshes 0 and 1, UV offset on 2 and 3
	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		ASkinMaterialHAL* pMaterial = TSTATICCAST( ASkinMaterialHAL, rLOD.GetMeshes()[ i ]->GetMaterial() );

		switch ( i )
		{
			case 0:
			case 1:
			{
				TTexture* pRimTexture = TTextureManager::GetSingleton()->FindTexture( "rim_add_blob.tga" );
				pRimTexture->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );
				pMaterial->SetTextureNum( 2 );
				pMaterial->SetTevMode( 0, 0x11 );
				pMaterial->SetTevMode( 1, 2 );
				pMaterial->SetTexture( 1, pRimTexture );
				pMaterial->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_RIMLIGHT );
				pMaterial->SetBlendMode( 1, 0 );
				pMaterial->SetFlags( 0x1000, TTRUE );
				break;
			}
			case 2:
			case 3:
				pMaterial->SetFlags( 0x80, TTRUE );
				pMaterial->SetBlendMode( 1, 0 );
				pMaterial->SetFlags( 0x1000, TTRUE );
				pMaterial->SetTevMode( 0, 0xf );
				pMaterial->SetPassMode( 0, ASkinMaterialHAL::PASSMODE_UVOFFSET );
				break;
		}
	}

	// The skeleton is shared by every Blob
	if ( ms_pSkeletonChar == TNULL )
	{
		TSkeletonInstance* pSkeletonInstance = m_pCharModelI->GetAnimInterface()->GetSkeletonInstance();
		ms_pSkeletonChar                     = pSkeletonInstance->GetSkeleton();

		ms_iBoneSBBase = TINT16( ms_pSkeletonChar->GetBoneID( "blob_sb_base" ) );
		TASSERT( ms_iBoneSBBase > -1 );
		ms_iBoneSBBody = TINT16( ms_pSkeletonChar->GetBoneID( "blob_sb_body" ) );
		TASSERT( ms_iBoneSBBody > -1 );
		ms_iBoneEffect1 = TINT16( ms_pSkeletonChar->GetBoneID( "blob_lb_particleeffect1" ) );
		TASSERT( ms_iBoneEffect1 > -1 );
		ms_iBoneEffect2 = TINT16( ms_pSkeletonChar->GetBoneID( "blob_lb_particleeffect2" ) );
		TASSERT( ms_iBoneEffect2 > -1 );

		TINT16 iLeftEyeBone  = TINT16( ms_pSkeletonChar->GetBoneID( "blob_lb_eyectrl_lf" ) );
		TINT16 iRightEyeBone = TINT16( ms_pSkeletonChar->GetBoneID( "blob_lb_eyectrl_rt" ) );
		TASSERT( iLeftEyeBone > -1 && iRightEyeBone > -1 );

		ms_pLeftEyeBoneTransform  = &pSkeletonInstance->GetBones()[ iLeftEyeBone ].m_Transform;
		ms_pRightEyeBoneTransform = &pSkeletonInstance->GetBones()[ iRightEyeBone ].m_Transform;

		for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		{
			TMaterial* pMaterial = rLOD.GetMeshes()[ i ]->GetMaterial();

			if ( TStringManager::String8CompareNoCase( pMaterial->GetName(), "ss_blob_eye_lf" ) == 0 )
				ms_pLeftEyeMaterial = ASkinMaterialHAL::Cast( pMaterial );
			else if ( TStringManager::String8CompareNoCase( pMaterial->GetName(), "ss_blob_eye_rt" ) == 0 )
				ms_pRightEyeMaterial = ASkinMaterialHAL::Cast( pMaterial );
		}

		ResolveAnimations( ms_pAnimationProperties->m_pAnims, ms_pAnimationProperties->m_iNumAnims, TFALSE );

		if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
			ResolveAnimations( ms_pAnimationProperties->m_pSPAnims, ms_pAnimationProperties->m_iNumSPAnims, TFALSE );
		else
			ResolveAnimations( ms_pAnimationProperties->m_pMPAnims, ms_pAnimationProperties->m_iNumMPAnims, TFALSE );

		ms_iAngeredEyes = TINT16( ms_pSkeletonChar->GetSequenceID( "Idle_Angered_Eyes" ) );
	}
	else
	{
		TASSERT( ms_pSkeletonChar == m_pCharModelI->GetAnimInterface()->GetSkeletonInstance()->GetSkeleton(), "All Blobs must use the same model, otherwise animation ID structure must be made non-static." );
	}

	m_iState = STATE_BALL;

	Anim* pSurfaceAnims = GetSurfaceAnims( GetCurrentSurface() );
	PlayAnim( ( pSurfaceAnims != TNULL ) ? &pSurfaceAnims[ 6 ] : &ms_pAnimationProperties->m_pCommon->m_pAnims[ 3 ], 0.15f );
}

// Clears the second material of mesh a_iMesh (LOD 0) of a_pModelI's model; the material is owned by the shader
static void ClearMeshMaterial2( AModelInstance2* a_pModelI, TINT a_iMesh )
{
	T2ModelPtr pModel = a_pModelI->GetModel()->GetModelPtr();
	TMesh*     pMesh  = pModel->GetLOD( 0 ).GetMeshes()[ a_iMesh ];

	TSTATICCAST( ASkinMesh, pMesh )->SetMaterial2( TNULL );
}

// $deBlob: FUNCTION 00751300
// $deBlob: FUNCTION 007532f0
ABlobAnimationComponent::~ABlobAnimationComponent()
{
	ClearMeshMaterial2( m_pCharModelI, 0 );
	ClearMeshMaterial2( m_pCharModelI, 1 );
	ClearMeshMaterial2( m_pBallBodyModelI, 0 );
	ClearMeshMaterial2( m_pBallEarsModelI, 0 );

	AModelFactory* pModelFactory = AModelFactory::GetSingleton();
	pModelFactory->DestroyInstance( m_pBallBodyModelI );
	pModelFactory->DestroyInstance( m_pBallEarsModelI );
	pModelFactory->DestroyInstance( m_pCharModelI );

	if ( m_pShieldModelI != TNULL )
	{
		pModelFactory->DestroyInstance( m_pShieldModelI );
	}

	ms_pSkeletonChar = TNULL;

	if ( m_pCrownModelI != TNULL )
	{
		pModelFactory->DestroyInstance( m_pCrownModelI );
		m_pCrownModelI = TNULL;
	}

	// Retail: OnDestroy (vtable +0x0C), then the deleting destructor (vtable +0x08)
	m_pShockedMaterial->OnDestroy();
	delete m_pShockedMaterial;
	m_pShockedMaterial = TNULL;

	m_pZPassMaterial->OnDestroy();
	delete m_pZPassMaterial;
	m_pZPassMaterial = TNULL;

	ms_bShieldMaterialCreated = TFALSE;
}

// $deBlob: FUNCTION 007536d0
void ABlobAnimationComponent::Update( TFLOAT a_fDeltaTime )
{
	if ( 0.0f < m_fPaintFlashTimer )
	{
		m_fPaintFlashTimer -= a_fDeltaTime;

		TUINT32 uiColour = m_pOwner->GetPaintColour();
		if ( uiColour == 0 || ( uiColour & 0x10 ) != 0 )
			m_fPaintFlashTimer = 0.0f;

		ASkinShaderHAL::s_vTevMode1AColour.w = TMath::LerpTowards( 0.0f, 1.0f, m_fPaintFlashTimer / PAINT_FLASH_TIME );

		if ( m_fPaintFlashTimer <= 0.0f )
			ResetPaintFlash();
	}

	if ( ( m_pOwner->m_uiFlags & 0x2000000 ) == 0 )
	{
		if ( ( m_iAnimFlags & 2 ) != 0 )
		{
			TFLOAT fTime  = m_fBlinkTimer;
			m_fBlinkTimer = fTime - a_fDeltaTime;

			if ( fTime - a_fDeltaTime < 0.0f )
				m_iAnimFlags &= ~2;
		}
	}
	else
	{
		// Blinks: m_fBlinkTimer is the time to the next blink (or until the eyes open), m_fLookTimer the time to the next look
		TFLOAT fBlinkTime = m_fBlinkTimer - a_fDeltaTime;
		TFLOAT fLookTime  = m_fLookTimer - a_fDeltaTime;
		m_fBlinkTimer     = fBlinkTime;
		m_fLookTimer      = fLookTime;

		if ( fBlinkTime < 0.0f )
		{
			m_iAnimFlags ^= 2;
			TRandom&                       rRandom = ARandom::GetSingleton()->GetRandom();
			ABlobEntity::CommonProperties* pProps  = ABlobEntity::ms_pBlobCommonProperties;

			if ( ( m_iAnimFlags & 2 ) == 0 )
			{
				m_fBlinkTimer = rRandom.GetFloatMinMax( pProps->m_afEyesOpenTime[ 0 ], pProps->m_afEyesOpenTime[ 1 ] );
			}
			else
			{
				if ( fLookTime < 0.0f )
				{
					TFLOAT fTime = rRandom.GetFloatMinMax( pProps->m_afLookTime[ 0 ], pProps->m_afLookTime[ 1 ] );
					m_iLookIndex++;
					m_fLookTimer = fTime;

					if ( ms_pAnimationProperties->m_pTable->m_iNumLookAnims <= m_iLookIndex )
						m_iLookIndex = 0;
				}

				m_fBlinkTimer = rRandom.GetFloatMinMax( pProps->m_afBlinkTime[ 0 ], pProps->m_afBlinkTime[ 1 ] );
			}
		}
	}

	if ( ( m_iAnimFlags & 2 ) == 0 && m_uiUnk48 != 2 )
	{
		m_pBallBodyModelI->SetFlag( 0x100, TFALSE );
		m_pBallEarsModelI->SetFlag( 0x100, TFALSE );
		m_pCharModelI->SetFlag( 0x100, TFALSE );
		m_pCharModelI->SetMeshFlags( 2, 2, TTRUE );
		m_pCharModelI->SetMeshFlags( 3, 2, TTRUE );
		m_pCharModelI->SetMeshFlags( 4, 2, TFALSE );
	}
	else
	{
		m_pBallBodyModelI->SetFlag( 0x100, TTRUE );
		m_pBallEarsModelI->SetFlag( 0x100, TTRUE );
		m_pCharModelI->SetFlag( 0x100, TTRUE );
		m_pCharModelI->SetMeshFlags( 2, 2, TFALSE );
		m_pCharModelI->SetMeshFlags( 3, 2, TFALSE );
		m_pCharModelI->SetMeshFlags( 4, 2, TTRUE );
	}

	if ( m_fTargetAlpha != m_fAlpha )
	{
		TFLOAT fAlpha = TMath::LERP( m_fAlpha, m_fTargetAlpha, m_fAlphaSpeed * a_fDeltaTime );
		m_fAlpha      = fAlpha;

		if ( fAlpha <= 0.99f )
		{
			if ( fAlpha < 0.01f )
				m_fAlpha = 0.0f;
		}
		else
		{
			m_fAlpha = 1.0f;
		}

		TBOOL bVisible = ( m_pOwner->GetFlags() & ABaseEntity::FLAG_Visible ) != 0;

		if ( m_fAlpha == 0.0f )
		{
			if ( bVisible )
				m_pOwner->SetVisible( TFALSE );
		}
		else if ( !bVisible )
		{
			m_pOwner->SetVisible( TTRUE );
		}
	}

	if ( 0.0f < m_fIdleAnimDelay ) m_fIdleAnimDelay -= a_fDeltaTime;
	if ( 0.0f < m_fCelebrateTimer ) m_fCelebrateTimer -= a_fDeltaTime;
	if ( 0.0f < m_fUnk28 ) m_fUnk28 -= a_fDeltaTime;
	if ( 0.0f < m_fUnk2C ) m_fUnk2C -= a_fDeltaTime;

	switch ( m_iState )
	{
		case STATE_BALL:
			UpdateState0( a_fDeltaTime );
			break;
		case STATE_FADETOCHAR:
			UpdateState1( a_fDeltaTime );
			break;
		case STATE_ANIMTOCHAR:
			UpdateState2( a_fDeltaTime );
			break;
		case STATE_ANIMTOBALL:
			UpdateState4( a_fDeltaTime );
			break;
		case STATE_FADETOBALL:
			UpdateState5( a_fDeltaTime );
			break;
		case STATE_FLOAT:
			UpdateState6( a_fDeltaTime );
			break;
		case STATE_DEAD:
			UpdateState11( a_fDeltaTime );
			break;
		case STATE_LEVEL_START:
		case STATE_RESPAWN_POOL:
			UpdateLevelStart();
			break;
		case STATE_CHAR:
			UpdateState3( a_fDeltaTime );
			break;
		case STATE_LOCKED:
			UpdateState7( a_fDeltaTime );
			break;
		case STATE_HOTPLATE:
			UpdateState8( a_fDeltaTime );
			break;
		case STATE_SHOCK_RECOVER:
			UpdateState9( a_fDeltaTime );
			break;
		case STATE_PAIN:
			UpdateState10( a_fDeltaTime );
			break;
		case STATE_ZJUMPFAIL:
			UpdateState12( a_fDeltaTime );
			break;
		case STATE_LMARK_IN:
			UpdateState13( a_fDeltaTime );
			break;
		case STATE_LMARK_OUT:
			UpdateState14( a_fDeltaTime );
			break;
		case STATE_EXIT_CANNON:
			UpdateState19( a_fDeltaTime );
			break;
		case STATE_BRAKE:
			UpdateState20( a_fDeltaTime );
			break;
		case STATE_RESPAWN:
			UpdateState21( a_fDeltaTime );
			break;
		case STATE_CHALLENGE_VICTORY:
		case STATE_CHALLENGE_DEFEAT:
		case STATE_VICTORY:
		case STATE_DEFEAT:
		case STATE_POWERUP:
		case STATE_GRASSBOMB:
			UpdateStateAnimFinished();
			break;
		case STATE_LEVEL_EXIT:
			UpdateState16( a_fDeltaTime );
			break;
		case STATE_SHIELD_ACTIVATING:
		case STATE_SHIELD_DEACTIVATING:
		case STATE_SHIELD_ACTIVE:
			UpdateStateShield( a_fDeltaTime );
			break;
		case STATE_LOSTLASTLIFE:
			UpdateState30( a_fDeltaTime );
			break;
	}

	// The colours of the models: the body and light colours of the current paint colour (black while blinking), the
	// clear colours of the start colour for the multiplayer modes
	m_uiUnk48                  = 0;
	TUINT32      uiBodyColour  = 0;
	TUINT32      uiLightColour = 0;
	AGameStatus* pGameStatus   = AGameStatus::GetSingleton();

	if ( m_pOwner->GetPaintColour() == ABlobColourComponent::COLOUR_WHITE && pGameStatus->GetMultiPlayerGameType() == 1 )
	{
		m_uiUnk48 = 2 - ( APlayerManager::GetSingleton()->IsNoBlobPainted() ? 1 : 0 );
	}

	ABlobColourComponent* pColour     = m_pOwner->GetColourComponent();
	TUINT32               uiStart     = pColour->GetStartColour();
	TBOOL                 bClearMP    = pGameStatus->IsMultiplayer() && pColour->GetCurrentColour() == ABlobColourComponent::COLOUR_WHITE;
	TUINT32               uiMapColour = ( pColour->GetCurrentColour() & ABlobColourComponent::COLOUR_BLACK ) ? ABlobColourComponent::COLOUR_BLACK : pColour->GetCurrentColour();

	if ( !bClearMP )
	{
		if ( ( m_iAnimFlags & 2 ) == 0 )
		{
			uiBodyColour  = ABlobColourMaps::ms_pBodyColourMap[ uiMapColour ];
			uiLightColour = ABlobColourMaps::ms_pLightColourMap[ uiMapColour ];
		}
		else
		{
			uiBodyColour  = 0xFF000000;
			uiLightColour = 0xFF000000;
		}
	}
	else if ( TINT( m_uiUnk48 ) >= 0 )
	{
		if ( TINT( m_uiUnk48 ) <= 1 )
		{
			uiBodyColour  = ABlobColourMaps::ms_pClearBodyColourMap[ uiStart ];
			uiLightColour = ABlobColourMaps::ms_pClearLightColourMap[ uiStart ];
		}
		else if ( m_uiUnk48 == 2 )
		{
			uiLightColour = ABlobColourMaps::ms_pSkelLightColourMap[ uiStart ];
			uiBodyColour  = 0xFFFFFFFF;
			m_pCharModelI->SetMeshColour( 4, ABlobColourMaps::ms_pBodyColourMap[ uiStart ] );
		}
	}

	m_pBallBodyModelI->UseLocalAmbientLight( TTRUE );
	m_pBallBodyModelI->SetAmbientColour( uiLightColour );

	TFLOAT fAlpha = 1.0f;

	if ( bClearMP )
	{
		if ( TINT( m_uiUnk48 ) >= 0 && TINT( m_uiUnk48 ) <= 1 )
			fAlpha = TFLOAT( uiBodyColour >> 24 ) / 255.0f;
		else if ( m_uiUnk48 != 2 )
			// NOTE: retail uses the delta time as the alpha here (unreachable: the mode is 0, 1 or 2)
			fAlpha = a_fDeltaTime;
	}
	else if ( pColour->GetCurrentColour() == ABlobColourComponent::COLOUR_WHITE )
	{
		fAlpha = TFLOAT( uiBodyColour >> 24 ) / 255.0f;
	}

	TBOOL   bOpaque      = ( m_iAnimFlags & 2 ) != 0 || m_uiUnk48 == 2;
	TUINT32 uiAlpha      = bOpaque ? 0xFF : TUINT8( RoundToInt( m_fAlpha * 255.0f * fAlpha * m_pBallBodyModelI->GetModelInstance()->GetFade() ) );
	TUINT32 uiBallColour = ( uiBodyColour & 0x00FFFFFF ) | ( uiAlpha << 24 );
	m_pBallBodyModelI->SetMeshColour( 0, uiBallColour );
	m_pBallEarsModelI->SetMeshColour( 0, uiBallColour );
	m_pBallEarsModelI->SetFlag( AModelInstance2::FLAG_Visible, ( m_pBallBodyModelI->GetFlags() & AModelInstance2::FLAG_Visible ) != 0 );

	if ( m_pBallEarsModelI->GetFlags() & AModelInstance2::FLAG_Visible )
	{
		m_pBallEarsModelI->UseLocalAmbientLight( TTRUE );
		m_pBallEarsModelI->SetAmbientColour( uiLightColour );
	}

	if ( m_pCharModelI->GetFlags() & AModelInstance2::FLAG_Visible )
	{
		m_pCharModelI->UseLocalAmbientLight( TTRUE );
		m_pCharModelI->SetAmbientColour( uiLightColour );

		TFLOAT fCharFade     = m_pCharModelI->GetModelInstance()->GetFade();
		uiAlpha              = bOpaque ? 0xFF : TUINT8( RoundToInt( m_fAlpha * 255.0f * fAlpha * fCharFade ) );
		TUINT32 uiCharColour = ( uiBodyColour & 0x00FFFFFF ) | ( uiAlpha << 24 );
		m_pCharModelI->SetMeshColour( 3, uiCharColour );
		m_pCharModelI->SetMeshColour( 2, uiCharColour );
		m_pCharModelI->SetMeshColour( 0, uiCharColour );

		// The lips
		TUINT32 uiLipColour = 0;

		if ( bClearMP )
		{
			if ( TINT( m_uiUnk48 ) >= 0 && TINT( m_uiUnk48 ) <= 1 )
				uiLipColour = ABlobColourMaps::ms_pClearLipColourMap[ uiStart ];
			else if ( m_uiUnk48 == 2 )
				uiLipColour = 0xFFFFFFFF;
		}
		else if ( ( m_iAnimFlags & 2 ) != 0 )
		{
			uiLipColour = 0xFFFFFFFF;
		}
		else
		{
			uiLipColour        = ABlobColourMaps::ms_pLipColourMap[ uiMapColour ];
			TUINT32 uiLipAlpha = TUINT8( RoundToInt( m_fAlpha * 255.0f * fCharFade * ( TFLOAT( uiLipColour >> 24 ) / 255.0f ) ) );
			uiLipColour        = ( uiLipColour & 0x00FFFFFF ) | ( uiLipAlpha << 24 );
		}

		m_pCharModelI->SetMeshColour( 1, uiLipColour );

		// The eyes' UV offsets follow the eye bones, snapped to thirds
		if ( ACameraManager::GetSingleton()->GetNumActiveCameras() == 1 )
		{
			TFLOAT fLeftU  = ms_pLeftEyeBoneTransform->m_f41;
			TFLOAT fLeftV  = ms_pLeftEyeBoneTransform->m_f42;
			TFLOAT fRightU = ms_pRightEyeBoneTransform->m_f41;
			TFLOAT fRightV = ms_pRightEyeBoneTransform->m_f42;

			ms_pLeftEyeMaterial->SetUVOffsetNoCheck( TFLOAT( RoundToInt( fLeftU * 3.0f ) ) / 3.0f, TFLOAT( RoundToInt( fLeftV * 3.0f ) ) / 3.0f, 0 );
			ms_pRightEyeMaterial->SetUVOffsetNoCheck( TFLOAT( RoundToInt( fRightU * 3.0f ) ) / 3.0f, TFLOAT( RoundToInt( fRightV * 3.0f ) ) / 3.0f, 0 );
		}
	}

	m_pCharModelI->UpdateImp( a_fDeltaTime );

	if ( m_bUnk6C )
	{
		StartPaintFlash();
		m_bUnk6C = TFALSE;
	}
}

// Mesh a_iMesh (LOD 0) of a_pModelI's model as a skin mesh
static ASkinMesh* GetSkinMesh( AModelInstance2* a_pModelI, TINT a_iMesh )
{
	T2ModelPtr pModel = a_pModelI->GetModel()->GetModelPtr();
	TMesh*     pMesh  = pModel->GetLOD( 0 ).GetMeshes()[ a_iMesh ];
	TASSERT( ( pMesh == TNULL ) || pMesh->IsA( &TGetClass( ASkinMesh ) ) );

	return TSTATICCAST( ASkinMesh, pMesh );
}

// The checked cast of a skin material
static ASkinMaterial* CastSkinMaterial( TMaterial* a_pMaterial )
{
	TASSERT( ( a_pMaterial == TNULL ) || a_pMaterial->IsA( &TGetClass( ASkinMaterial ) ) );
	return TSTATICCAST( ASkinMaterial, a_pMaterial );
}

// $deBlob: FUNCTION 007540c0
void ABlobAnimationComponent::Render()
{
	if ( m_iAnimFlags & 1 )
	{
		if ( !( m_iAnimFlags & 2 ) && m_uiUnk48 != 2 )
		{
			m_pBallBodyModelI->Render();
			m_pBallEarsModelI->Render();
			m_pCharModelI->Render();

			if ( m_pShieldModelI != TNULL && ( m_pShieldModelI->GetFlags() & AModelInstance2::FLAG_Visible ) )
				m_pShieldModelI->Render();
		}
		else
		{
			// The painting/colour change states: the meshes are rendered with the Z pass material, then again with
			// the shocked one, the char model's mesh flags 2 swapped between the passes
			TUINT8 auiMeshFlags[ AModelInstance2::MAX_MESH_SETTINGS ];

			for ( TINT i = 0; i < AModelInstance2::MAX_MESH_SETTINGS; i++ )
			{
				TASSERT( TUINT8( i ) < AModelInstance2::MAX_MESH_SETTINGS );
				auiMeshFlags[ i ] = TUINT8( m_pCharModelI->GetMeshFlags( TUINT8( i ) ) );
			}

			if ( m_pCharModelI->GetFlags() & AModelInstance2::FLAG_Visible )
			{
				// The char model (its meshes 0 and 1)
				ASkinMesh* pMesh0 = GetSkinMesh( m_pCharModelI, 0 );
				ASkinMesh* pMesh1 = GetSkinMesh( m_pCharModelI, 1 );

				pMesh0->SetMaterial2( CastSkinMaterial( m_pZPassMaterial ) );
				pMesh1->SetMaterial2( CastSkinMaterial( m_pZPassMaterial ) );

				m_pCharModelI->SetMeshFlags( 0, 2, TTRUE );
				m_pCharModelI->SetMeshFlags( 1, 2, TTRUE );
				m_pCharModelI->SetMeshFlags( 4, 2, TFALSE );
				m_pCharModelI->Render();

				m_pCharModelI->SetMeshFlags( 0, 2, TFALSE );
				m_pCharModelI->SetMeshFlags( 1, 2, TFALSE );
				m_pCharModelI->SetMeshFlags( 4, 2, TTRUE );

				if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
					m_pCharModelI->UseLocalAmbientLight( TFALSE );

				m_pCharModelI->Render();
				m_pCharModelI->UseLocalAmbientLight( TTRUE );

				pMesh0->SetMaterial2( CastSkinMaterial( m_pShockedMaterial ) );
				pMesh1->SetMaterial2( CastSkinMaterial( m_pShockedMaterial ) );

				m_pCharModelI->SetMeshFlags( 0, 2, TTRUE );
				m_pCharModelI->SetMeshFlags( 1, 2, TTRUE );
				m_pCharModelI->SetMeshFlags( 4, 2, TFALSE );
				m_pCharModelI->Render();
			}
			else
			{
				// The ball (the char model's mesh 0 holds the materials)
				ASkinMesh* pMesh0 = GetSkinMesh( m_pCharModelI, 0 );
				ASkinMesh* pMesh  = GetSkinMesh( m_pCharModelI, 0 );

				pMesh0->SetMaterial2( CastSkinMaterial( m_pZPassMaterial ) );
				pMesh->SetMaterial2( CastSkinMaterial( m_pZPassMaterial ) );

				m_pBallBodyModelI->Render();
				m_pBallEarsModelI->Render();

				m_pCharModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
				m_pCharModelI->SetMeshFlags( 0, 2, TFALSE );
				m_pCharModelI->SetMeshFlags( 1, 2, TFALSE );
				m_pCharModelI->SetMeshFlags( 4, 2, TTRUE );

				if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
					m_pCharModelI->UseLocalAmbientLight( TFALSE );

				m_pCharModelI->Render();
				m_pCharModelI->UseLocalAmbientLight( TTRUE );
				m_pCharModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

				pMesh0->SetMaterial2( CastSkinMaterial( m_pShockedMaterial ) );
				pMesh->SetMaterial2( CastSkinMaterial( m_pShockedMaterial ) );

				m_pBallBodyModelI->Render();
				m_pBallEarsModelI->Render();
			}

			for ( TINT i = 0; i < AModelInstance2::MAX_MESH_SETTINGS; i++ )
			{
				TASSERT( TUINT8( i ) < AModelInstance2::MAX_MESH_SETTINGS );
				m_pCharModelI->SetMeshFlagsLowByte( TUINT8( i ), auiMeshFlags[ i ] );
			}
		}

		if ( m_pCrownModelI != TNULL && ( m_pCrownModelI->GetFlags() & AModelInstance2::FLAG_Visible ) )
		{
			// The crown (multiplayer): over Blob, moved up to a unit towards the camera and facing it
			AXYZViewportManager*          pViewportManager = ARenderer::GetSingleton()->GetViewportManager();
			AXYZViewportManager::VIEWPORT eViewport        = pViewportManager->GetViewportBeingRendered();
			TASSERT( AXYZViewportManager::VIEWPORT_NONE != eViewport );

			ACamera*        pCamera     = pViewportManager->GetCamera( eViewport );
			const TVector4& rvCameraPos = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );

			TVector4 vPosition;
			m_pCharModelI->GetPosition( vPosition );
			vPosition.y -= ABlobEntity::GetProperties( m_pOwner->GetSize() )->m_fRadius;

			TVector4 vToCamera( rvCameraPos.x - vPosition.x, rvCameraPos.y - vPosition.y, rvCameraPos.z - vPosition.z, rvCameraPos.w );
			TFLOAT   fDistance = vToCamera.NormaliseMag();

			if ( fDistance >= 1.0f )
				fDistance = 1.0f;

			TVector4 vOffset;
			vOffset.Multiply( vToCamera, fDistance );
			vPosition.x = vOffset.x + vPosition.x;
			vPosition.y = vOffset.y + vPosition.y;
			vPosition.z = vOffset.z + vPosition.z;

			TMatrix44 oMatrix                                 = TMatrix44::IDENTITY;
			oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vPosition;
			oMatrix.LookAtTarget( rvCameraPos, oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
			m_pCrownModelI->SetTransform( oMatrix );

			TUINT32 uiColour = ABlobColourMaps::ms_pBodyColourMap[ m_pOwner->GetColourComponent()->GetStartColour() ];
			m_pCrownModelI->UseLocalAmbientColour( TTRUE );
			m_pCrownModelI->UseLocalAmbientLight( TTRUE );
			m_pCrownModelI->SetLocalAmbientColour( uiColour );
			m_pCrownModelI->SetAmbientColour( 0 );
			m_pCrownModelI->Render();
		}
	}
}

// $deBlob: FUNCTION 00755980
void ABlobAnimationComponent::Reset()
{
	SetState( STATE_BALL );
	m_fUnk0C         = ABlobEntity::GetCommonProperties()->m_fUnkB8;
	m_pStateAnim     = TNULL;
	m_uiUnk18        = 0;
	m_fIdleAnimDelay = 0.0f;
	m_fAlpha         = 1.0f;
	m_fTargetAlpha   = 1.0f;
	m_fAlphaSpeed    = 1.0f;
	m_iAnimFlags     = 1;
	m_uiUnk48        = 0;
	m_fBlinkTimer    = 0.0f;
	m_fLookTimer     = 0.0f;
	ResetPaintFlash();

	if ( m_pShieldModelI != TNULL )
		m_pShieldModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

	m_bUnk6C = TFALSE;
}

// $deBlob: FUNCTION 00754b30
void ABlobAnimationComponent::SetState( TINT a_iState )
{
	if ( a_iState == m_iState )
		return;

	if ( m_pShieldModelI != TNULL )
		m_pShieldModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

	Anim* pAnims = ms_pAnimationProperties->m_pCommon->m_pAnims;

	switch ( a_iState )
	{
		case STATE_BALL:
			ShowBallModels();
			PlayAnim( &pAnims[ 4 ], 0.15f );

			if ( m_uiEyeState == 1 )
			{
				SetEyeState( 0, TTRUE );
				m_iState = a_iState;
				return;
			}

			break;
		case STATE_FADETOCHAR:
			switch ( m_iState )
			{
				case STATE_BALL:
				case STATE_ANIMTOCHAR:
				case STATE_CHAR:
				case STATE_ANIMTOBALL:
				case STATE_FLOAT:
					ShowBallModels();
					PlayAnim( &pAnims[ 4 ], 0.15f );
					m_iState = a_iState;
					return;
			}

			break;
		case STATE_ANIMTOCHAR:
			ShowCharModel();

			if ( ( m_pOwner->GetPaintColour() >> 4 & 1 ) == 0 )
				PlayAnim( GetSurfaceAnim5( GetCurrentSurface() ), 0.0f );
			else
				PlayAnim( &pAnims[ 12 ], 0.0f );

			break;
		case STATE_CHAR:
			ShowCharModel();

			if ( 0.0f < m_fCelebrateTimer )
			{
				AnimSet* pCommon = ms_pAnimationProperties->m_pCommon;
				Anim*    pIdle   = &pCommon->m_pIdleAnims[ ARandom::GetSingleton()->GetRandom().GetInt( pCommon->m_iNumIdleAnims ) ];
				m_pStateAnim     = pIdle;
				PlayAnim( pIdle, 1.0f );
				m_iState          = a_iState;
				m_fCelebrateTimer = 0.0f;
				return;
			}

			if ( m_fIdleAnimDelay <= 0.0f )
			{
				m_fUnk0C = ABlobEntity::GetCommonProperties()->m_fUnkB8;

				if ( ( m_pOwner->GetPaintColour() >> 4 & 1 ) == 0 )
				{
					PlayAnim( GetSurfaceAnim2( GetCurrentSurface() ), 0.15f );
					m_iState = a_iState;
					return;
				}

				PlayAnim( &pAnims[ 10 ], 0.15f );
				m_iState = a_iState;
				return;
			}

			m_pStateAnim = &pAnims[ 9 ];
			PlayAnim( &pAnims[ 9 ], 0.0f );
			m_iState         = a_iState;
			m_fIdleAnimDelay = 0.0f;
			return;
		case STATE_LEVEL_START:
		{
			// The level start: one of the level start animations, not the same as last time
			TASSERT( !AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
			ShowCharModel();
			StopCurrentAnim( 0.0f );

			AnimTable* pTable = ms_pAnimationProperties->m_pTable;
			TINT       iAnim  = ARandom::GetSingleton()->GetRandom().GetInt( pTable->m_iNumLevelStartAnims );

			if ( iAnim == s_iLastLevelStartAnim )
				iAnim = ( iAnim + 1 ) % pTable->m_iNumLevelStartAnims;

			s_iLastLevelStartAnim = iAnim;
			PlayAnim( &pTable->m_pLevelStartAnims[ iAnim ], 0.0f );
			m_iState = a_iState;
			return;
		}
		case STATE_ANIMTOBALL:
		{
			ShowCharModel();

			Anim* pAnim;

			if ( IsAnimPlaying( &pAnims[ 16 ] ) && m_pOwner->m_pZJump->GetState() == ABlobZJumpComponent::STATE_NODEFLIGHT )
			{
				pAnim = &pAnims[ 17 ];
			}
			else if ( IsAnimPlaying( &pAnims[ 15 ] ) && ( m_pOwner->m_pZJump->GetState() == ABlobZJumpComponent::STATE_ATTACKFLIGHT || m_pOwner->m_pZJump->GetState() == ABlobZJumpComponent::STATE_LMARKFLIGHT || m_pOwner->m_pZJump->GetState() == ABlobZJumpComponent::STATE_SWITCHSLAM ) )
			{
				pAnim = &pAnims[ 18 ];
			}
			else if ( ( m_pOwner->m_pColourComponent->GetCurrentColour() & ABlobColourComponent::COLOUR_BLACK ) == 0 )
			{
				pAnim = GetSurfaceAnim6( GetCurrentSurface() );
			}
			else
			{
				pAnim = &pAnims[ 13 ];
			}

			m_uiUnk18 = TREINTERPRETCAST( TUINT32, pAnim );
			PlayAnim( pAnim, 0.0f );
			m_iState = a_iState;
			return;
		}
		case STATE_FADETOBALL:
			switch ( m_iState )
			{
				case STATE_BALL:
				case STATE_ANIMTOCHAR:
				case STATE_CHAR:
				case STATE_ANIMTOBALL:
				case STATE_FLOAT:
					ShowCharModel();
					PlayAnim( &pAnims[ 4 ], 0.15f );
					break;
			}

			break;
		case STATE_FLOAT:
			ShowCharModel();
			PlayAnim( &pAnims[ 8 ], 0.15f );
			m_iState = a_iState;
			m_fUnk0C = ABlobEntity::GetCommonProperties()->m_fUnkB8;
			return;
		case STATE_LOCKED:
			ShowCharModel();
			PlayAnim( &pAnims[ 14 ], 0.0f );
			break;
		case STATE_HOTPLATE:
		{
			TASSERT( !AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
			ShowCharModel();
			PlayAnim( &ms_pAnimationProperties->m_pTable->m_pAnims00[ 1 ], 0.0f );
			break;
		}
		case STATE_SHOCK_RECOVER:
			TASSERT( !AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
			ShowCharModel();
			PlayAnim( ms_pAnimationProperties->m_pTable->m_pAnims00, 0.0f );
			break;
		case STATE_PAIN:
			ShowCharModel();
			PlayAnim( &pAnims[ 5 ], 0.0f );
			break;
		case STATE_DEAD:
			if ( ( m_iAnimFlags & 1 ) != 0 )
			{
				m_pBallBodyModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
				m_pBallEarsModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
				m_pCharModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
			}

			// The ball and character models are forced to fade out (TModelInstance Flags_ForceFade, fade 0)
			m_pBallBodyModelI->GetModelInstance()->SetForceFade( TTRUE );
			m_pBallBodyModelI->GetModelInstance()->SetFade( 0.0f );
			m_pBallEarsModelI->GetModelInstance()->SetForceFade( TTRUE );
			m_pBallEarsModelI->GetModelInstance()->SetFade( 0.0f );
			m_pCharModelI->GetModelInstance()->SetForceFade( TTRUE );
			m_pCharModelI->GetModelInstance()->SetFade( 0.0f );
			StopCurrentAnim( 0.15f );
			break;
		case STATE_ZJUMPFAIL:
			ShowCharModel();

			if ( m_pOwner->m_pZJump->GetState() != ABlobZJumpComponent::STATE_AIM )
				PlayAnim( &pAnims[ 21 ], 0.15f );
			else
				PlayAnim( &pAnims[ 20 ], 0.15f );

			break;
		case STATE_LMARK_IN:
			TASSERT( !AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
			ShowCharModel();
			PlayAnim( &ms_pAnimationProperties->m_pTable->m_pAnims00[ 3 ], 0.0f );
			break;
		case STATE_LMARK_OUT:
			TASSERT( !AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
			ShowCharModel();
			m_pStateAnim = &ms_pAnimationProperties->m_pTable->m_pAnims00[ 4 ];
			PlayAnim( m_pStateAnim, 0.0f );
			break;
		case STATE_LEVEL_EXIT:
			TASSERT( !AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
			ShowCharModel();
			StopCurrentAnim( 0.0f );
			PlayAnim( &ms_pAnimationProperties->m_pTable->m_pAnims00[ 6 ], 0.0f );
			break;
		case STATE_ENTER_CANNON:
			ShowCharModel();
			PlayAnim( &pAnims[ 23 ], 0.0f );
			break;
		case STATE_INSIDE_CANNON:
			ShowCharModel();
			PlayAnim( &pAnims[ 24 ], 0.0f );
			break;
		case STATE_EXIT_CANNON:
			ShowCharModel();
			PlayAnim( &pAnims[ 25 ], 0.0f );
			break;
		case STATE_BRAKE:
			ShowCharModel();
			m_pStateAnim = &pAnims[ 26 ];
			PlayAnim( m_pStateAnim, 0.0f );
			break;
		case STATE_RESPAWN:
		{
			ShowCharModel();

			AnimSet* pCommon = ms_pAnimationProperties->m_pCommon;

			// No ground under Blob (the ground check is ABlobEntity::FindRespawnGround): an idle animation
			if ( !m_pOwner->FindRespawnGround( TNULL, TNULL ) )
			{
				m_pStateAnim = &pCommon->m_pIdleAnims[ ARandom::GetSingleton()->GetRandom().GetInt( pCommon->m_iNumIdleAnims ) ];
				PlayAnim( m_pStateAnim, 0.0f );
				break;
			}

			m_pStateAnim = &pAnims[ 7 ];
			PlayAnim( m_pStateAnim, 0.0f );
			break;
		}
		case STATE_RESPAWN_POOL:
		{
			ShowCharModel();

			AnimTable* pTable = ms_pAnimationProperties->m_pTable;
			TINT       iAnim  = ARandom::GetSingleton()->GetRandom().GetInt( pTable->m_iNumAnims20 );
			PlayAnim( &pTable->m_pAnims1C[ iAnim ], 0.0f );
			break;
		}
		case STATE_SHIELD_ACTIVATING:
		{
			TASSERT( AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
			m_pShieldModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );

			Toshi::TMatrix44 mTransform;
			m_pCharModelI->GetTransform( mTransform );
			mTransform.Scale( 0.32f, 0.32f, 0.32f );
			m_pShieldModelI->SetTransform( mTransform );

			m_iCurShieldKey = 0;
			m_fShieldTime   = 0.0f;
			ShowCharModel();
			PlayAnim( ms_pAnimationProperties->m_pTable2->m_pAnims, 0.0f );
			break;
		}
		case STATE_SHIELD_DEACTIVATING:
		{
			TASSERT( AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
			m_pShieldModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );

			m_iCurShieldKey = 0;
			m_fShieldTime   = 0.0f;
			ShowCharModel();
			PlayAnim( &ms_pAnimationProperties->m_pTable2->m_pAnims[ 2 ], 0.0f );
			break;
		}
		case STATE_SHIELD_ACTIVE:
		{
			TASSERT( AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
			m_pShieldModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
			ShowCharModel();
			PlayAnim( &ms_pAnimationProperties->m_pTable2->m_pAnims[ 1 ], 0.0f );
			break;
		}
		case STATE_CHALLENGE_VICTORY:
			ShowCharModel();
			PlayAnim( GetSurfaceLandmarkAnim( GetCurrentSurface() ), 0.0f );
			break;
		case STATE_CHALLENGE_DEFEAT:
			ShowCharModel();

			if ( m_uiEyeState != 1 )
				m_uiEyeState = 1;

			PlayAnim( GetSurfaceRandomAnim( GetCurrentSurface() ), 0.0f );
			break;
		case STATE_VICTORY:
			ShowCharModel();
			PlayAnim( GetSurfaceLandmarkAnim( TNULL ), 0.0f );
			break;
		case STATE_DEFEAT:
			ShowCharModel();

			if ( m_uiEyeState != 1 )
				m_uiEyeState = 1;

			PlayAnim( GetSurfaceRandomAnim( TNULL ), 0.0f );
			break;
		case STATE_LOSTLASTLIFE:
			TASSERT( !AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
			ShowCharModel();
			PlayAnim( GetRandomUnk04Anim(), 0.0f );
			break;
		case STATE_POWERUP:
			ShowCharModel();
			PlayAnim( &ms_pAnimationProperties->m_pTable->m_pAnims00[ 7 ], 0.0f );
			break;
		case STATE_GRASSBOMB:
			TASSERT( !AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
			ShowCharModel();
			PlayAnim( &ms_pAnimationProperties->m_pTable->m_pAnims00[ 8 ], 0.0f );
			break;
		default:
			break;
	}

	m_iState = a_iState;
}

// $deBlob: FUNCTION 00755750
void ABlobAnimationComponent::SetEyeState( TINT a_iEyeState, TBOOL a_bRestart )
{
	TINT iOldState = m_uiEyeState;

	if ( a_iEyeState == iOldState )
		return;

	if ( a_bRestart && m_pCurrentAnim != TNULL && ( m_pCurrentAnim->m_uiFlags & 4 ) != 0 )
	{
		switch ( iOldState )
		{
			case 0:
				m_pCharModelI->GetAnimInterface()->StopAnim( m_pCurrentAnim->m_iEyeOLay, 0.0f );
				break;
			case 1:
				m_pCharModelI->GetAnimInterface()->StopAnim( ms_iAngeredEyes, 0.0f );
				break;
		}

		switch ( a_iEyeState )
		{
			case 0:
				m_pCharModelI->GetAnimInterface()->AddAnim( m_pCurrentAnim->m_iEyeOLay, 0.0f, 1.0f, m_pCurrentAnim->m_fSpeed );
				break;
			case 1:
				m_pCharModelI->GetAnimInterface()->AddAnim( ms_iAngeredEyes, 0.0f, 1.0f, m_pCurrentAnim->m_fSpeed );
				break;
		}
	}

	m_uiEyeState = a_iEyeState;
}

// $deBlob: FUNCTION 00755a30
void ABlobAnimationComponent::OnLanded()
{
	if ( m_iState == STATE_ZJUMPFAIL )
	{
		// Landed in the water or ink
		if ( m_pOwner->m_oMotion.m_uiContactFlags & 0x28 )
		{
			PlayAnim( &ms_pAnimationProperties->m_pCommon->m_pAnims[ 22 ], 0.15f );
			return;
		}

		switch ( m_pOwner->m_eState )
		{
			case ABlobEntity::STATE_STUCKWALL:
			{
				TINT iZJumpState = m_pOwner->GetZJumpState();

				if ( iZJumpState != ABlobZJumpComponent::STATE_AIMNODE && iZJumpState != ABlobZJumpComponent::STATE_AIM )
				{
					SetState( STATE_BALL );
					return;
				}
			}
				TFALLTHROUGH;
			case ABlobEntity::STATE_FAIL:
			case ABlobEntity::STATE_VICTORY:
			case ABlobEntity::STATE_LOSTLASTLIFE:
				SetState( STATE_CHAR );
				return;
			case ABlobEntity::STATE_HOTPLATE:
				SetState( STATE_HOTPLATE );
				return;
			case ABlobEntity::STATE_FLOAT:
				SetState( STATE_FLOAT );
				break;
			case ABlobEntity::STATE_DEAD:
				SetState( STATE_DEAD );
				return;
		}
	}
	else if ( m_iState == STATE_LMARK_OUT )
	{
		OnStateFinished();
	}
}

// The names of the animation states
static const TCHAR* s_aszAnimStateNames[] = {
	"STATE_BALL",
	"STATE_FADETOCHAR",
	"STATE_ANIMTOCHAR",
	"STATE_CHAR",
	"STATE_ANIMTOBALL",
	"STATE_FADETOBALL",
	"STATE_FLOAT",
	"STATE_LOCKED",
	"STATE_HOTPLATE",
	"STATE_SHOCK_RECOVER",
	"STATE_PAIN",
	"STATE_DEAD",
	"STATE_ZJUMPFAIL",
	"STATE_LMARK_IN",
	"STATE_LMARK_OUT",
	"STATE_LEVEL_START",
	"STATE_LEVEL_EXIT",
	"STATE_ENTER_CANNON",
	"STATE_INSIDE_CANNON",
	"STATE_EXIT_CANNON",
	"STATE_BRAKE",
	"STATE_RESPAWN",
	"STATE_RESPAWN_POOL",
	"STATE_SHIELD_ACTIVATING",
	"STATE_SHIELD_DEACTIVATING",
	"STATE_SHIELD_ACTIVE",
	"STATE_CHALLENGE_VICTORY",
	"STATE_CHALLENGE_DEFEAT",
	"STATE_VICTORY",
	"STATE_DEFEAT",
	"STATE_LOSTLASTLIFE",
	"STATE_POWERUP",
	"STATE_GRASSBOMB",
};

// $deBlob: FUNCTION 00755f90
const TCHAR* ABlobAnimationComponent::GetStateName( TINT a_eState ) const
{
	if ( a_eState > -1 && TUINT( a_eState ) < TARRAYSIZE( s_aszAnimStateNames ) )
		return s_aszAnimStateNames[ a_eState ];

	return "Unknown State";
}

// $deBlob: FUNCTION 00755fc0
void ABlobAnimationComponent::BlendModels( AModelInstance2* a_pFrom, AModelInstance2* a_pTo, TFLOAT a_fMin, TFLOAT a_fMax, TFLOAT a_fStep )
{
	TMath::Clip( a_fMin, 0.0f, 1.0f );
	TMath::Clip( a_fMax, 0.0f, 1.0f );

	if ( ( a_pTo->GetFlags() & AModelInstance2::FLAG_Visible ) == 0 )
	{
		a_pTo->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
		a_pTo->GetModelInstance()->SetForceFade( TTRUE );
		a_pTo->GetModelInstance()->SetFade( a_fMin );
	}

	TModelInstance* pTo = a_pTo->GetModelInstance();

	if ( pTo->GetFade() < a_fMax )
	{
		TFLOAT fFade = pTo->GetFade() + a_fStep;
		pTo->SetForceFade( TTRUE );
		pTo->SetFade( TMath::Min( fFade, a_fMax ) );
		return;
	}

	if ( ( a_pFrom->GetFlags() & AModelInstance2::FLAG_Visible ) != 0 )
	{
		TModelInstance* pFrom = a_pFrom->GetModelInstance();
		TFLOAT          fFade = pFrom->GetFade() - a_fStep;
		pFrom->SetForceFade( TTRUE );

		if ( fFade < a_fMin )
		{
			pFrom->SetFade( a_fMin );
			a_pFrom->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
			return;
		}

		pFrom->SetFade( fFade );
	}
}

// $deBlob: FUNCTION 007560c0
ABlobAnimationComponent::Anim* ABlobAnimationComponent::GetSurfaceAnims( void* a_pSurface )
{
	if ( a_pSurface != TNULL )
	{
		if ( *TREINTERPRETCAST( TINT*, TREINTERPRETCAST( TUINT8*, a_pSurface ) + 0x170 ) == 8 )
			a_pSurface = *TREINTERPRETCAST( void**, TREINTERPRETCAST( TUINT8*, a_pSurface ) + 0x174 );

		if ( a_pSurface != TNULL )
		{
			switch ( *TREINTERPRETCAST( TINT*, TREINTERPRETCAST( TUINT8*, a_pSurface ) + 0x170 ) )
			{
				case 0:
				case 2:
				case 6:
					return ms_pAnimationProperties->m_pTable->m_apSurfaceAnims[ 0 ];
				case 1:
					return ms_pAnimationProperties->m_pTable->m_apSurfaceAnims[ 1 ];
				case 3:
				case 5:
					return ms_pAnimationProperties->m_pTable->m_apSurfaceAnims[ 3 ];
				case 4:
					return ms_pAnimationProperties->m_pTable->m_apSurfaceAnims[ 2 ];
			}
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 007562b0
ABlobAnimationComponent::Anim* ABlobAnimationComponent::GetSurfaceAnim2( void* a_pSurface )
{
	Anim* pAnims = GetSurfaceAnims( a_pSurface );

	if ( pAnims != TNULL )
		return &pAnims[ 2 ];

	return &ms_pAnimationProperties->m_pCommon->m_pAnims[ 0 ];
}

// $deBlob: FUNCTION 00756340
ABlobAnimationComponent::Anim* ABlobAnimationComponent::GetSurfaceAnim6( void* a_pSurface )
{
	Anim* pAnims = GetSurfaceAnims( a_pSurface );

	if ( pAnims != TNULL )
		return &pAnims[ 6 ];

	return &ms_pAnimationProperties->m_pCommon->m_pAnims[ 3 ];
}

// $deBlob: FUNCTION 00756370
ABlobAnimationComponent::Anim* ABlobAnimationComponent::GetSurfaceAnim5( void* a_pSurface )
{
	Anim* pAnims = GetSurfaceAnims( a_pSurface );

	if ( pAnims != TNULL )
		return &pAnims[ 5 ];

	return &ms_pAnimationProperties->m_pCommon->m_pAnims[ 2 ];
}

// $deBlob: FUNCTION 007562e0
ABlobAnimationComponent::Anim* ABlobAnimationComponent::GetSurfaceAnim3( void* a_pSurface )
{
	Anim* pAnims = GetSurfaceAnims( a_pSurface );

	if ( pAnims != TNULL )
		return &pAnims[ ARandom::GetSingleton()->GetRandom().GetInt( 2 ) + 3 ];

	return &ms_pAnimationProperties->m_pCommon->m_pRandomAnims00[ ARandom::GetSingleton()->GetRandom().GetInt( ms_pAnimationProperties->m_pCommon->m_iNumRandomAnims04 ) ];
}

// $deBlob: FUNCTION 00756150
ABlobAnimationComponent::Anim* ABlobAnimationComponent::GetSurfaceLandmarkAnim( void* a_pSurface )
{
	Anim* pSurfaceAnims = GetSurfaceAnims( a_pSurface );

	if ( pSurfaceAnims == ms_pAnimationProperties->m_pTable->m_apSurfaceAnims[ 1 ] )
	{
		TINT8 iLandmark = *TREINTERPRETCAST( TINT8*, TREINTERPRETCAST( TUINT8*, a_pSurface ) + 0x176 );

		if ( iLandmark > -1 )
		{
			TASSERT( iLandmark < ms_pAnimationProperties->m_pTable->m_iNumLandmarkAnims );
			return &ms_pAnimationProperties->m_pTable->m_pLandmarkAnims[ iLandmark ];
		}
	}

	if ( pSurfaceAnims != TNULL )
		return &pSurfaceAnims[ 7 + ARandom::GetSingleton()->GetRandom().GetInt( 3 ) ];

	return &ms_pAnimationProperties->m_pCommon->m_pRandomAnims18[ ARandom::GetSingleton()->GetRandom().GetInt( ms_pAnimationProperties->m_pCommon->m_iNumRandomAnims1C ) ];
}

// $deBlob: FUNCTION 00756220
ABlobAnimationComponent::Anim* ABlobAnimationComponent::GetSurfaceRandomAnim( void* a_pSurface )
{
	Anim* pSurfaceAnims = GetSurfaceAnims( a_pSurface );

	if ( pSurfaceAnims != TNULL )
		return &pSurfaceAnims[ ARandom::GetSingleton()->GetRandom().GetInt( 2 ) ];

	return &ms_pAnimationProperties->m_pCommon->m_pRandomAnims20[ ARandom::GetSingleton()->GetRandom().GetInt( ms_pAnimationProperties->m_pCommon->m_iNumRandomAnims24 ) ];
}

// $deBlob: FUNCTION 00756280
ABlobAnimationComponent::Anim* ABlobAnimationComponent::GetRandomUnk04Anim()
{
	return &ms_pAnimationProperties->m_pTable->m_pAnims04[ ARandom::GetSingleton()->GetRandom().GetInt( ms_pAnimationProperties->m_pTable->m_iNumAnims08 ) ];
}

// $deBlob: FUNCTION 00756700
void ABlobAnimationComponent::PlayAnim( Anim* a_pAnim, TFLOAT a_fBlendInSpeed )
{
	if ( m_pCurrentAnim != TNULL )
	{
		m_pCharModelI->GetAnimInterface()->StopAnim( m_pCurrentAnim->m_iAnim, 0.15f );
		m_pCharModelI->GetAnimInterface()->StopAnim( m_pCurrentAnim->m_iEyeOLay, 0.0f );

		if ( m_pStateAnim == m_pCurrentAnim )
			m_pStateAnim = TNULL;

		m_pCurrentAnim = TNULL;
	}

	m_pCharModelI->GetAnimInterface()->AddAnim( a_pAnim->m_iAnim, a_fBlendInSpeed, 1.0f, a_pAnim->m_fSpeed );

	if ( m_uiEyeState == 0 || ( a_pAnim->m_uiFlags & 4 ) == 0 )
		m_pCharModelI->GetAnimInterface()->AddAnim( a_pAnim->m_iEyeOLay, 0.0f, 1.0f, a_pAnim->m_fSpeed );
	else if ( m_uiEyeState == 1 )
		m_pCharModelI->GetAnimInterface()->AddAnim( ms_iAngeredEyes, 0.0f, 1.0f, a_pAnim->m_fSpeed );

	m_pCurrentAnim = a_pAnim;
}

// $deBlob: FUNCTION 00754a50
void ABlobAnimationComponent::SetVisible( TBOOL a_bVisible )
{
	if ( a_bVisible == TBOOL( m_iAnimFlags & 1 ) )
		return;

	if ( a_bVisible )
	{
		m_iAnimFlags |= 1;

		switch ( m_iState )
		{
			case STATE_BALL:
				ShowBallModels();
				break;
			case STATE_FADETOCHAR:
			case STATE_ANIMTOCHAR:
			case STATE_ANIMTOBALL:
			case STATE_FADETOBALL:
				m_pBallBodyModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
				m_pBallEarsModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
				m_pCharModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
				break;
			case STATE_CHAR:
			case STATE_FLOAT:
			case STATE_LOCKED:
			case STATE_HOTPLATE:
			case STATE_SHOCK_RECOVER:
			case STATE_PAIN:
			case STATE_ZJUMPFAIL:
			case STATE_LMARK_IN:
			case STATE_LMARK_OUT:
			case STATE_LEVEL_START:
			case STATE_LEVEL_EXIT:
			case STATE_ENTER_CANNON:
			case STATE_INSIDE_CANNON:
			case STATE_EXIT_CANNON:
			case STATE_BRAKE:
			case STATE_RESPAWN:
			case STATE_RESPAWN_POOL:
			case STATE_SHIELD_ACTIVATING:
			case STATE_SHIELD_DEACTIVATING:
			case STATE_SHIELD_ACTIVE:
			case STATE_CHALLENGE_VICTORY:
			case STATE_CHALLENGE_DEFEAT:
			case STATE_VICTORY:
			case STATE_DEFEAT:
			case STATE_LOSTLASTLIFE:
			case STATE_POWERUP:
			case STATE_GRASSBOMB:
				ShowCharModel();
				break;
		}
	}
	else
	{
		m_pBallBodyModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		m_pBallEarsModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		m_pCharModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		m_iAnimFlags &= ~1;
	}
}

// $deBlob: FUNCTION 00756840
void ABlobAnimationComponent::ShowBallModels()
{
	if ( m_iAnimFlags & 1 )
	{
		m_pBallBodyModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
		m_pBallEarsModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
		m_pCharModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	}

	m_pBallBodyModelI->GetModelInstance()->SetForceFade( TTRUE );
	m_pBallBodyModelI->GetModelInstance()->SetFade( 1.0f );
	m_pBallEarsModelI->GetModelInstance()->SetForceFade( TTRUE );
	m_pBallEarsModelI->GetModelInstance()->SetFade( 1.0f );
	m_pCharModelI->GetModelInstance()->SetForceFade( TTRUE );
	m_pCharModelI->GetModelInstance()->SetFade( 0.0f );
}

// Stops the current animation and its eyes overlay
// $deBlob: FUNCTION 007567e0
void ABlobAnimationComponent::StopCurrentAnim( TFLOAT a_fBlendOutSpeed )
{
	if ( m_pCurrentAnim != TNULL )
	{
		m_pCharModelI->GetAnimInterface()->StopAnim( m_pCurrentAnim->m_iAnim, a_fBlendOutSpeed );
		m_pCharModelI->GetAnimInterface()->StopAnim( m_pCurrentAnim->m_iEyeOLay, 0.0f );

		if ( m_pStateAnim == m_pCurrentAnim )
			m_pStateAnim = TNULL;

		m_pCurrentAnim = TNULL;
	}
}

// State 0xF (and 0x16): the level start animation, kept in sync with the level start gate's; the blob stays out of
// the physics until it's near its end (3.75 s), then it's dropped
// $deBlob: FUNCTION 00757d70
void ABlobAnimationComponent::UpdateLevelStart()
{
	AToshiAnimInterface2* pAnimInterface = m_pCharModelI->GetAnimInterface();
	TSkeletonInstance*    pSkeleton      = pAnimInterface->GetSkeletonInstance();

	if ( pSkeleton != TNULL && pSkeleton->IsAnimating( m_pCurrentAnim->m_iAnim ) != TNULL && !pAnimInterface->IsAnimationFinished( m_pCurrentAnim->m_iAnim ) )
	{
		TFLOAT fTime = pAnimInterface->GetAnimationSeqTime( m_pCurrentAnim->m_iAnim );

		if ( fTime <= 3.75f )
		{
			if ( AGameStatus::GetSingleton()->IsLevelStart() )
			{
				fTime = AGameStatus::GetSingleton()->GetLevelStartAnimTime();

				if ( pSkeleton != TNULL )
				{
					TAnimation* pAnim = pSkeleton->IsAnimating( m_pCurrentAnim->m_iAnim );

					if ( pAnim != TNULL )
						pAnim->SetTime( fTime );
				}

				if ( pSkeleton != TNULL )
				{
					TAnimation* pAnim = pSkeleton->IsAnimating( m_pCurrentAnim->m_iEyeOLay );

					if ( pAnim != TNULL )
						pAnim->SetTime( fTime );
				}

				if ( pSkeleton->GetOverlayAnimationCount() + pSkeleton->GetBaseAnimationCount() != 0 )
					pSkeleton->UpdateState( TFALSE );
			}

			m_pOwner->SetLevelStartAttached( TTRUE );
			return;
		}
	}

	m_pOwner->SetLevelStartAttached( TFALSE );

	TVector4 vVelocity;
	vVelocity.Multiply( TVector4::VEC_NEGY, 15.0f );
	TPhysicsManager::SetLinearVelocity( m_pOwner->m_pBody, vVelocity );

	// The level data flag is cleared in level mode 1 or while the HUD control 5 is shown
	Toshi::TXUIControl* pControl = ABlobHud::GetSingleton()->GetLevelTimerControl();

	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint || ( pControl != TNULL && pControl->IsVisible() ) )
		AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;

	SetState( STATE_FLOAT );
}

// $deBlob: FUNCTION 007568c0
void ABlobAnimationComponent::ShowCharModel()
{
	if ( m_iAnimFlags & 1 )
	{
		m_pBallBodyModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		m_pBallEarsModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		m_pCharModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
	}

	m_pBallBodyModelI->GetModelInstance()->SetForceFade( TTRUE );
	m_pBallBodyModelI->GetModelInstance()->SetFade( 0.0f );
	m_pBallEarsModelI->GetModelInstance()->SetForceFade( TTRUE );
	m_pBallEarsModelI->GetModelInstance()->SetFade( 0.0f );
	m_pCharModelI->GetModelInstance()->SetForceFade( TTRUE );
	m_pCharModelI->GetModelInstance()->SetFade( 1.0f );
}

// $deBlob: FUNCTION 00756940
void ABlobAnimationComponent::OnStateFinished()
{
	switch ( m_pOwner->m_eState )
	{
		case ABlobEntity::STATE_ROLL:
		case ABlobEntity::STATE_GRASSBOMB:
		case ABlobEntity::STATE_LEVEL_EXIT:
		case ABlobEntity::STATE_FAIL:
		case ABlobEntity::STATE_VICTORY:
			SetState( STATE_CHAR );
			return;
		case ABlobEntity::STATE_BALLISTIC:
		case ABlobEntity::STATE_CANNON:
			SetState( STATE_BALL );
			return;
		case ABlobEntity::STATE_STUCKWALL:
		{
			TINT iZJumpState = m_pOwner->GetZJumpState();

			if ( iZJumpState != ABlobZJumpComponent::STATE_AIMNODE && iZJumpState != ABlobZJumpComponent::STATE_AIM )
				SetState( STATE_BALL );
			else
				SetState( STATE_CHAR );

			return;
		}
		case ABlobEntity::STATE_HOTPLATE:
			SetState( STATE_HOTPLATE );
			return;
		case ABlobEntity::STATE_FLOAT:
			SetState( STATE_FLOAT );
			return;
		case ABlobEntity::STATE_RESPAWN:
			SetState( STATE_RESPAWN );
			return;
		case ABlobEntity::STATE_DEAD:
			SetState( STATE_DEAD );
			return;
		case ABlobEntity::STATE_LOSTLASTLIFE:
			SetState( STATE_LOSTLASTLIFE );
			return;
	}
}

// The blob's speed compared with a_fFactor * its size's m_fBlinkTimer
static TFLOAT GetBlobSpeedLimit( ABlobEntity* a_pBlob, TINT a_iSize, TFLOAT a_fFactor, TFLOAT& a_rfOutSpeed )
{
	TVector4 vVelocity;
	a_pBlob->GetBlobVelocity( vVelocity );
	a_rfOutSpeed = vVelocity.Magnitude();
	return ABlobEntity::GetProperties( a_iSize )->m_fMaxSpeed * a_fFactor;
}

// $deBlob: FUNCTION 007569e0
void ABlobAnimationComponent::UpdateState0( TFLOAT a_fDeltaTime )
{
	ABlobEntity* pOwner = m_pOwner;

	if ( pOwner->m_iLeechbotsAttached <= 0 )
	{
		switch ( pOwner->m_eState )
		{
			case ABlobEntity::STATE_ROLL:
			case ABlobEntity::STATE_FLOAT:
			case ABlobEntity::STATE_LEVEL_EXIT:
			{
				TFLOAT fSpeed;
				if ( GetBlobSpeedLimit( pOwner, pOwner->m_iSize, ABlobEntity::GetCommonProperties()->m_afUnkA0[ 3 ], fSpeed ) <= fSpeed )
					return;
				break;
			}
			default:
				return;
			case ABlobEntity::STATE_STUCKWALL:
			{
				TINT iZJumpState = pOwner->GetZJumpState();

				if ( iZJumpState != ABlobZJumpComponent::STATE_AIMNODE )
				{
					if ( iZJumpState != ABlobZJumpComponent::STATE_AIM )
						return;

					SetState( STATE_FADETOCHAR );
					return;
				}

				break;
			}
			case ABlobEntity::STATE_HOTPLATE:
			case ABlobEntity::STATE_GRASSBOMB:
			case ABlobEntity::STATE_DEAD:
			case ABlobEntity::STATE_FAIL:
			case ABlobEntity::STATE_VICTORY:
			case ABlobEntity::STATE_LOSTLASTLIFE:
				break;
		}

		SetState( STATE_FADETOCHAR );
	}
}

// $deBlob: FUNCTION 00757230
void ABlobAnimationComponent::UpdateState4( TFLOAT a_fDeltaTime )
{
	ABlobEntity* pOwner = m_pOwner;

	if ( pOwner->m_iLeechbotsAttached < 1 )
	{
		switch ( pOwner->m_eState )
		{
			case ABlobEntity::STATE_ROLL:
			case ABlobEntity::STATE_FLOAT:
			case ABlobEntity::STATE_LEVEL_EXIT:
			{
				TVector4 vVelocity;
				pOwner->GetBlobVelocity( vVelocity );

				if ( vVelocity.Magnitude() < ABlobEntity::GetProperties( pOwner->m_iSize )->m_fMaxSpeed * ABlobEntity::GetCommonProperties()->m_afUnkA0[ 3 ] )
				{
					SetState( STATE_ANIMTOCHAR );
					return;
				}

				break;
			}
			case ABlobEntity::STATE_STUCKWALL:
				if ( pOwner->m_pZJump->GetState() == ABlobZJumpComponent::STATE_AIMNODE || pOwner->m_pZJump->GetState() == ABlobZJumpComponent::STATE_AIM )
					SetState( STATE_ANIMTOCHAR );

				break;
			case ABlobEntity::STATE_HOTPLATE:
			case ABlobEntity::STATE_GRASSBOMB:
			case ABlobEntity::STATE_DEAD:
			case ABlobEntity::STATE_FAIL:
			case ABlobEntity::STATE_VICTORY:
			case ABlobEntity::STATE_LOSTLASTLIFE:
				SetState( STATE_ANIMTOCHAR );
				return;
		}
	}

	// The transition animation is over: the next state
	Anim* pAnim = TREINTERPRETCAST( Anim*, m_uiUnk18 );

	if ( !IsAnimPlaying( pAnim ) || IsAnimFinished( pAnim ) )
	{
		m_uiUnk18 = 0;
		SetState( STATE_FADETOBALL );
	}
}

// $deBlob: FUNCTION 00757340
void ABlobAnimationComponent::UpdateState5( TFLOAT a_fDeltaTime )
{
	if ( ( m_pCharModelI->GetFlags() & AModelInstance2::FLAG_Visible ) == 0 )
	{
		SetState( STATE_BALL );
		return;
	}

	ABlobEntity* pOwner = m_pOwner;

	if ( pOwner->m_iLeechbotsAttached < 1 )
	{
		switch ( pOwner->m_eState )
		{
			case ABlobEntity::STATE_ROLL:
			case ABlobEntity::STATE_FLOAT:
			case ABlobEntity::STATE_LEVEL_EXIT:
			{
				TVector4 vVelocity;
				pOwner->GetBlobVelocity( vVelocity );

				if ( vVelocity.Magnitude() < ABlobEntity::GetProperties( pOwner->m_iSize )->m_fMaxSpeed * ABlobEntity::GetCommonProperties()->m_afUnkA0[ 3 ] )
				{
					SetState( STATE_FADETOCHAR );
					return;
				}

				break;
			}
			case ABlobEntity::STATE_STUCKWALL:
				if ( pOwner->m_pZJump->GetState() == ABlobZJumpComponent::STATE_AIMNODE || pOwner->m_pZJump->GetState() == ABlobZJumpComponent::STATE_AIM )
					SetState( STATE_FADETOCHAR );

				break;
			case ABlobEntity::STATE_HOTPLATE:
			case ABlobEntity::STATE_GRASSBOMB:
			case ABlobEntity::STATE_DEAD:
			case ABlobEntity::STATE_FAIL:
			case ABlobEntity::STATE_VICTORY:
			case ABlobEntity::STATE_LOSTLASTLIFE:
				SetState( STATE_FADETOCHAR );
				return;
		}
	}

	if ( m_fPaintFlashTimer <= 0.0f )
	{
		// Blend from the character to the ball
		ABlobEntity::CommonProperties* pCommon = ABlobEntity::GetCommonProperties();
		BlendModels( m_pCharModelI, m_pBallBodyModelI, 0.0f, pCommon->m_afUnkA0[ 0 ], ( a_fDeltaTime * 2.0f ) / pCommon->m_afUnkA0[ 1 ] );
		return;
	}

	// The paint flash: show the ball at once
	m_pBallBodyModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
	m_pBallBodyModelI->GetModelInstance()->SetForceFade( TTRUE );
	m_pBallBodyModelI->GetModelInstance()->SetFade( 1.0f );
	m_pCharModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
}

// $deBlob: FUNCTION 007574a0
void ABlobAnimationComponent::UpdateState6( TFLOAT a_fDeltaTime )
{
	ABlobEntity*                   pOwner      = m_pOwner;
	ABlobEntity::Properties*       pProperties = ABlobEntity::GetProperties( pOwner->m_iSize );
	ABlobEntity::CommonProperties* pCommon     = ABlobEntity::GetCommonProperties();
	ABlobZJumpComponent*           pZJump      = pOwner->m_pZJump;
	TVector4                       vVelocity;

	switch ( pOwner->m_eState )
	{
		case ABlobEntity::STATE_ROLL:
		case ABlobEntity::STATE_LEVEL_EXIT:
			pOwner->GetBlobVelocity( vVelocity );

			if ( vVelocity.Magnitude() <= pProperties->m_fMaxSpeed * pCommon->m_afUnkA0[ 2 ] )
			{
				if ( pZJump->GetState() == ABlobZJumpComponent::STATE_POINT || ( pZJump->GetState() == ABlobZJumpComponent::STATE_LOCK && pZJump->GetCurrentPriorityLock() != TNULL ) )
					SetState( STATE_LOCKED );
				else
					SetState( STATE_CHAR );

				return;
			}

			// Falls through
		case ABlobEntity::STATE_BALLISTIC:
		case ABlobEntity::STATE_CANNON:
			SetState( STATE_ANIMTOBALL );
			return;
		case ABlobEntity::STATE_STUCKWALL:
			if ( pZJump->GetState() == ABlobZJumpComponent::STATE_AIMNODE || pZJump->GetState() == ABlobZJumpComponent::STATE_AIM )
				SetState( STATE_CHAR );

			return;
		case ABlobEntity::STATE_FLOAT:
		{
			pOwner->GetBlobVelocity( vVelocity );

			if ( TMath::Abs( pOwner->m_oPhysicsMatrix.m_f42 - pOwner->m_oMotion.m_vLiquidHit.y ) < pProperties->m_fRadius )
				vVelocity.y = 0.0f;

			TFLOAT fSpeed = vVelocity.Magnitude();
			TFLOAT fLimit = pProperties->m_fMaxSpeed;

			if ( pCommon->m_afUnkA0[ 2 ] * fLimit < fSpeed )
			{
				SetState( STATE_ANIMTOBALL );
				return;
			}

			if ( pZJump->GetState() == ABlobZJumpComponent::STATE_POINT || ( pZJump->GetState() == ABlobZJumpComponent::STATE_LOCK && pZJump->GetCurrentPriorityLock() != TNULL ) )
			{
				SetState( STATE_LOCKED );
				return;
			}

			if ( pCommon->m_afUnkA0[ 5 ] * fLimit <= fSpeed )
				return;

			// Standing still: the idle animation, with a fidget now and then
			Anim* pIdleAnim = &ms_pAnimationProperties->m_pCommon->m_pAnims[ 8 ];

			if ( !IsAnimPlaying( pIdleAnim ) )
			{
				if ( m_pStateAnim != TNULL && ( !IsAnimPlaying( m_pStateAnim ) || IsAnimFinished( m_pStateAnim ) ) )
					m_pStateAnim = TNULL;

				if ( m_pStateAnim == TNULL )
				{
					PlayAnim( pIdleAnim, 0.15f );
					m_fUnk0C = ARandom::GetSingleton()->GetRandom().GetFloatMinMax( pCommon->m_fUnkB8, pCommon->m_afUnkBC[ 0 ] );
				}
			}
			else if ( IsAnimPlaying( pIdleAnim ) )
			{
				TBOOL bInked = ( pOwner->m_pColourComponent->GetCurrentColour() & ABlobColourComponent::COLOUR_BLACK ) != 0;

				if ( bInked )
					a_fDeltaTime *= 2.0f;

				m_fUnk0C -= a_fDeltaTime;

				if ( m_fUnk0C <= 0.0f && !bInked )
				{
					m_pStateAnim = &ms_pAnimationProperties->m_pCommon->m_pAnims[ 7 ];
					PlayAnim( m_pStateAnim, 0.15f );
				}
			}

			return;
		}
	}
}

// $deBlob: FUNCTION 00756d90
void ABlobAnimationComponent::UpdateState3( TFLOAT a_fDeltaTime )
{
	ABlobEntity*         pOwner  = m_pOwner;
	ABlobZJumpComponent* pZJump  = pOwner->GetZJumpComponent();
	AnimSet*             pCommon = ms_pAnimationProperties->m_pCommon;

	if ( pOwner->m_iLeechbotsAttached > 0 )
	{
		SetState( STATE_ANIMTOBALL );
		return;
	}

	switch ( pOwner->m_eState )
	{
		case ABlobEntity::STATE_ROLL:
		case ABlobEntity::STATE_LEVEL_EXIT:
		{
			TVector4 vVelocity;
			pOwner->GetBlobVelocity( vVelocity );

			TFLOAT                         fSpeed = vVelocity.Magnitude();
			TFLOAT                         fLimit = ABlobEntity::GetProperties( pOwner->m_iSize )->m_fMaxSpeed;
			ABlobEntity::CommonProperties* pProps = ABlobEntity::GetCommonProperties();

			if ( fSpeed > pProps->m_afUnkA0[ 2 ] * fLimit )
			{
				SetState( STATE_ANIMTOBALL );
				return;
			}

			TINT iZJumpState = pZJump->GetState();

			if ( iZJumpState == ABlobZJumpComponent::STATE_POINT || ( iZJumpState == ABlobZJumpComponent::STATE_LOCK && pZJump->GetCurrentPriorityLock() != TNULL ) )
			{
				SetState( STATE_LOCKED );
			}
			else if ( fSpeed <= pProps->m_afUnkA0[ 4 ] * fLimit )
			{
				if ( fSpeed < pProps->m_afUnkA0[ 5 ] * fLimit )
				{
					// Not inked: the horizontal speed decides whether the blob is sliding
					if ( ( pOwner->GetPaintColour() & ABlobColourComponent::COLOUR_BLACK ) == 0 )
					{
						// NOTE: retail compares the squared horizontal speed with the unsquared limit (a_fLimit * m_afUnkA0[ 2 ])
						fSpeed = vVelocity.MagnitudeSqXZ();

						if ( pProps->m_afUnkA0[ 2 ] * fLimit < fSpeed )
						{
							Anim* pAnim = &pCommon->m_pAnims[ 6 ];

							if ( IsAnimPlaying( pAnim ) )
								return;

							PlayAnim( pAnim, 0.15f );
							return;
						}
					}

					Anim* pAnim;

					if ( ( pOwner->m_uiFlags & 0x2000000 ) == 0 || AGameStatus::GetSingleton()->IsMultiplayer() )
					{
						if ( ( pOwner->GetPaintColour() & ABlobColourComponent::COLOUR_BLACK ) == 0 )
							pAnim = GetSurfaceAnim2( GetCurrentSurface() );
						else
							pAnim = &pCommon->m_pAnims[ 10 ];
					}
					else
					{
						pAnim = &ms_pAnimationProperties->m_pTable->m_pLookAnims0C[ m_iLookIndex ];
					}

					if ( !IsAnimPlaying( pAnim ) )
					{
						if ( m_pStateAnim != TNULL && ( !IsAnimPlaying( m_pStateAnim ) || IsAnimFinished( m_pStateAnim ) ) )
							m_pStateAnim = TNULL;

						if ( m_pStateAnim == TNULL )
						{
							TFLOAT fBlendInSpeed = 0.15f;

							if ( ( pOwner->m_uiFlags & 0x2000000 ) != 0 )
							{
								StopCurrentAnim( 0.0f );
								fBlendInSpeed = 0.0f;
							}

							PlayAnim( pAnim, fBlendInSpeed );
							m_fUnk0C = ARandom::GetSingleton()->GetRandom().GetFloatMinMax( pProps->m_fUnkB8, pProps->m_afUnkBC[ 0 ] );
						}
					}
					else if ( m_pStateAnim == TNULL && IsIdleAllowed() )
					{
						TFLOAT fDelta = a_fDeltaTime;

						if ( ( pOwner->GetPaintColour() & ABlobColourComponent::COLOUR_BLACK ) != 0 )
							fDelta *= 2.0f;

						m_fUnk0C -= fDelta;

						if ( m_fUnk28 > 0.0f )
						{
							m_pStateAnim = &pCommon->m_pAnims[ 0x13 ];
							PlayAnim( m_pStateAnim, 0.0f );
							m_fUnk28 = 0.0f;
							return;
						}

						if ( m_fUnk0C <= 0.0f )
						{
							Anim* pIdle;

							if ( ( pOwner->GetPaintColour() & ABlobColourComponent::COLOUR_BLACK ) == 0 )
								pIdle = GetSurfaceAnim3( GetCurrentSurface() );
							else
								pIdle = &pCommon->m_pAnims[ 0xB ];

							m_pStateAnim = pIdle;
							PlayAnim( pIdle, 0.15f );
							return;
						}
					}
				}
			}
			else
			{
				Anim* pAnim = &pCommon->m_pAnims[ 1 ];

				if ( !IsAnimPlaying( pAnim ) )
				{
					PlayAnim( pAnim, 0.15f );
					return;
				}
			}

			break;
		}
		case ABlobEntity::STATE_STUCKWALL:
		{
			TINT iZJumpState = pZJump->GetState();

			if ( iZJumpState != ABlobZJumpComponent::STATE_AIMNODE && iZJumpState != ABlobZJumpComponent::STATE_AIM )
			{
				SetState( STATE_ANIMTOBALL );
				return;
			}

			Anim* pAnim = &pCommon->m_pAnims[ 0x10 ];

			if ( !IsAnimPlaying( pAnim ) )
			{
				PlayAnim( pAnim, 0.15f );
				return;
			}

			break;
		}
		case ABlobEntity::STATE_FLOAT:
		{
			TVector4 vVelocity;
			pOwner->GetBlobVelocity( vVelocity );

			if ( vVelocity.Magnitude() <= ABlobEntity::GetProperties( pOwner->m_iSize )->m_fMaxSpeed * ABlobEntity::GetCommonProperties()->m_afUnkA0[ 2 ] )
			{
				TINT iZJumpState = pZJump->GetState();

				if ( iZJumpState != ABlobZJumpComponent::STATE_POINT && ( iZJumpState != ABlobZJumpComponent::STATE_LOCK || pZJump->GetCurrentPriorityLock() == TNULL ) )
				{
					SetState( STATE_FLOAT );
					return;
				}

				SetState( STATE_LOCKED );
				return;
			}

			SetState( STATE_ANIMTOBALL );
			return;
		}
		case ABlobEntity::STATE_BALLISTIC:
		case ABlobEntity::STATE_CANNON:
			SetState( STATE_ANIMTOBALL );
			return;
	}
}

// $deBlob: FUNCTION 00757790
void ABlobAnimationComponent::UpdateState7( TFLOAT a_fDeltaTime )
{
	ABlobEntity*         pOwner = m_pOwner;
	ABlobZJumpComponent* pZJump = pOwner->GetZJumpComponent();

	switch ( pOwner->m_eState )
	{
		case ABlobEntity::STATE_ROLL:
		case ABlobEntity::STATE_LEVEL_EXIT:
		{
			TVector4 vVelocity;
			pOwner->GetBlobVelocity( vVelocity );

			TFLOAT fLimit = ABlobEntity::GetProperties( pOwner->m_iSize )->m_fMaxSpeed;

			if ( vVelocity.Magnitude() > ABlobEntity::GetCommonProperties()->m_afUnkA0[ 2 ] * fLimit )
			{
				SetState( STATE_ANIMTOBALL );
				return;
			}

			TINT iZJumpState = pZJump->GetState();

			if ( iZJumpState != ABlobZJumpComponent::STATE_POINT && ( iZJumpState != ABlobZJumpComponent::STATE_LOCK || pZJump->GetCurrentPriorityLock() == TNULL ) )
			{
				SetState( STATE_CHAR );
				return;
			}

			break;
		}
		case ABlobEntity::STATE_STUCKWALL:
		{
			TINT iZJumpState = pOwner->GetZJumpState();

			if ( iZJumpState == ABlobZJumpComponent::STATE_AIMNODE || iZJumpState == ABlobZJumpComponent::STATE_AIM )
				return;

			SetState( STATE_ANIMTOBALL );
			return;
		}
		case ABlobEntity::STATE_BALLISTIC:
		case ABlobEntity::STATE_CANNON:
			SetState( STATE_ANIMTOBALL );
			return;
		case ABlobEntity::STATE_FLOAT:
		{
			TVector4 vVelocity;
			pOwner->GetBlobVelocity( vVelocity );

			if ( vVelocity.Magnitude() > ABlobEntity::GetCommonProperties()->m_afUnkA0[ 2 ] * ABlobEntity::GetProperties( pOwner->m_iSize )->m_fMaxSpeed )
			{
				SetState( STATE_ANIMTOBALL );
				return;
			}

			TINT iZJumpState = pZJump->GetState();

			if ( iZJumpState != ABlobZJumpComponent::STATE_POINT && ( iZJumpState != ABlobZJumpComponent::STATE_LOCK || pZJump->GetCurrentPriorityLock() == TNULL ) )
			{
				SetState( STATE_FLOAT );
				return;
			}

			break;
		}
		default:
			return;
	}

	// The idle after the state: the current animation is over, the next one is played
	AnimSet* pCommon = ms_pAnimationProperties->m_pCommon;
	Anim*    pAnim   = &pCommon->m_pAnims[ 0xF ];

	if ( !IsAnimPlaying( m_pCurrentAnim ) || IsAnimFinished( m_pCurrentAnim ) )
	{
		if ( m_pCurrentAnim != &pCommon->m_pAnims[ 0xE ] )
		{
			if ( m_pCurrentAnim != pAnim )
				return;

			if ( IsAnimPlaying( m_pCurrentAnim ) )
				return;
		}

		PlayAnim( pAnim, 0.15f );
	}
}

// $deBlob: FUNCTION 00757950
void ABlobAnimationComponent::UpdateState8( TFLOAT a_fDeltaTime )
{
	if ( m_pOwner->m_eState != ABlobEntity::STATE_HOTPLATE )
	{
		OnStateFinished();
		return;
	}

	if ( IsAnimPlaying( m_pCurrentAnim ) && !IsAnimFinished( m_pCurrentAnim ) )
		return;

	Anim* pAnims = ms_pAnimationProperties->m_pTable->m_pAnims00;

	if ( m_pCurrentAnim == &pAnims[ 1 ] )
	{
		PlayAnim( &pAnims[ 2 ], 0.15f );
		return;
	}

	if ( m_pCurrentAnim == &pAnims[ 2 ] && !IsAnimPlaying( &pAnims[ 2 ] ) )
		PlayAnim( &pAnims[ 2 ], 0.15f );
}

// $deBlob: FUNCTION 00757ad0
void ABlobAnimationComponent::UpdateState9( TFLOAT a_fDeltaTime )
{
	if ( IsAnimPlaying( &ms_pAnimationProperties->m_pTable->m_pAnims00[ 0 ] ) && !IsAnimFinished( &ms_pAnimationProperties->m_pTable->m_pAnims00[ 0 ] ) )
		return;

	OnStateFinished();
}

// $deBlob: FUNCTION 00757a00
void ABlobAnimationComponent::UpdateState10( TFLOAT a_fDeltaTime )
{
	if ( IsAnimPlaying( &ms_pAnimationProperties->m_pCommon->m_pAnims[ 5 ] ) && !IsAnimFinished( &ms_pAnimationProperties->m_pCommon->m_pAnims[ 5 ] ) )
		return;

	OnStateFinished();
}

// $deBlob: FUNCTION 00757b20
void ABlobAnimationComponent::UpdateState12( TFLOAT a_fDeltaTime )
{
	if ( m_pOwner->m_eState != ABlobEntity::STATE_BALLISTIC && m_pOwner->m_eState != ABlobEntity::STATE_ROLL )
	{
		OnStateFinished();
		return;
	}

	if ( IsAnimPlaying( m_pCurrentAnim ) && !IsAnimFinished( m_pCurrentAnim ) )
		return;

	Anim* pAnims = ms_pAnimationProperties->m_pCommon->m_pAnims;

	if ( m_pCurrentAnim == &pAnims[ 0x14 ] )
	{
		PlayAnim( &pAnims[ 0x15 ], 0.15f );
		return;
	}

	if ( m_pCurrentAnim == &pAnims[ 0x15 ] )
	{
		if ( !IsAnimPlaying( &pAnims[ 0x15 ] ) )
		{
			PlayAnim( &pAnims[ 0x15 ], 0.15f );
			return;
		}
	}
	else if ( m_pCurrentAnim == &pAnims[ 0x16 ] )
	{
		OnStateFinished();
	}
}

// $deBlob: FUNCTION 00757c10
void ABlobAnimationComponent::UpdateState13( TFLOAT a_fDeltaTime )
{
	AnimTable* pTable = ms_pAnimationProperties->m_pTable;

	if ( IsAnimPlaying( &pTable->m_pAnims00[ 3 ] ) && !IsAnimFinished( &pTable->m_pAnims00[ 3 ] ) )
		return;

	if ( m_iAnimFlags & 1 )
	{
		m_pBallBodyModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		m_pBallEarsModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		m_pCharModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		m_iAnimFlags &= ~1;
	}
}

// $deBlob: FUNCTION 00757c90
void ABlobAnimationComponent::UpdateState14( TFLOAT a_fDeltaTime )
{
	AnimTable* pTable = ms_pAnimationProperties->m_pTable;

	if ( IsAnimPlaying( &pTable->m_pAnims00[ 5 ] ) )
		return;

	if ( m_pStateAnim != TNULL )
	{
		// NOTE: retail asserts once
		static TBOOL s_bFidgetAnimAsserted = TFALSE;

		if ( !s_bFidgetAnimAsserted && m_pStateAnim != &pTable->m_pAnims00[ 4 ] )
		{
			TASSERT( TFALSE && "m_pFidgetAnim == &ms_pAnimProperties->m_oSP.m_oOtherAnims.m_oLandmarkOut" );
			s_bFidgetAnimAsserted = TTRUE;
		}

		if ( !IsAnimPlaying( m_pStateAnim ) || IsAnimFinished( m_pStateAnim ) )
			m_pStateAnim = TNULL;
	}

	if ( m_pStateAnim == TNULL )
		PlayAnim( &pTable->m_pAnims00[ 5 ], 0.0f );
}

// Leaving the level: the animation follows the door's camera animation
// $deBlob: FUNCTION 00757ef0
void ABlobAnimationComponent::UpdateState16( TFLOAT a_fDeltaTime )
{
	if ( m_pOwner->m_eState != ABlobEntity::STATE_LEVEL_EXIT )
		return;

	TFLOAT             fTime         = TSTATICCAST( APropTriggerLevelComplete, m_pOwner->m_pLevelExitTrigger )->GetDoorCameraAnimTime();
	TSkeletonInstance* pSkeletonInst = m_pCharModelI->GetAnimInterface()->GetSkeletonInstance();

	if ( pSkeletonInst != TNULL )
	{
		TAnimation* pAnim = pSkeletonInst->IsAnimating( m_pCurrentAnim->m_iAnim );

		if ( pAnim != TNULL )
			pAnim->SetTime( fTime );
	}

	if ( pSkeletonInst != TNULL )
	{
		TAnimation* pAnim = pSkeletonInst->IsAnimating( m_pCurrentAnim->m_iEyeOLay );

		if ( pAnim != TNULL )
			pAnim->SetTime( fTime );
	}

	// NOTE: retail doesn't check the skeleton instance for TNULL here
	if ( pSkeletonInst->GetBaseAnimationCount() + pSkeletonInst->GetOverlayAnimationCount() != 0 )
		pSkeletonInst->UpdateState( TFALSE );
}

// The keys of the shield (multiplayer): the time, the scale and the fade
struct ShieldKey
{
	TFLOAT fTime;
	TFLOAT fScale;
	TFLOAT fFade;
};

static const ShieldKey s_afShieldOnScales[]  = { { 0.0f, 0.32f, 1.0f }, { 0.133f, 0.32f, 1.0f }, { 0.233f, 1.3f, 1.0f }, { 0.3f, 1.0f, 1.0f } };
static const ShieldKey s_afShieldOffScales[] = { { 0.0f, 1.0f, 1.0f }, { 0.133f, 1.0f, 1.0f }, { 0.2f, 1.34f, 1.0f }, { 0.266f, 3.5f, 0.0f } };

static TFLOAT s_fShieldScale = 1.5f;

// $deBlob: FUNCTION 007580d0
void ABlobAnimationComponent::UpdateStateShield( TFLOAT a_fDeltaTime )
{
	TMatrix44 mTransform;
	m_pCharModelI->GetTransform( mTransform );

	if ( m_iState == STATE_SHIELD_ACTIVATING || m_iState == STATE_SHIELD_DEACTIVATING )
	{
		const ShieldKey* pKeys = ( m_iState == STATE_SHIELD_ACTIVATING ) ? s_afShieldOnScales : s_afShieldOffScales;
		m_fShieldTime += a_fDeltaTime;

		if ( m_iState == STATE_SHIELD_ACTIVATING )
		{
			TASSERT( m_iCurShieldKey + 1 < TINT( TARRAYSIZE( s_afShieldOnScales ) ) );
		}
		else
		{
			TASSERT( m_iCurShieldKey + 1 < TINT( TARRAYSIZE( s_afShieldOffScales ) ) );
		}

		const ShieldKey& rFrom = pKeys[ m_iCurShieldKey ];
		const ShieldKey& rTo   = pKeys[ m_iCurShieldKey + 1 ];

		TFLOAT fT     = ( m_fShieldTime - rFrom.fTime ) / ( rTo.fTime - rFrom.fTime );
		TFLOAT fScale = TMath::LerpTowards( rFrom.fScale, rTo.fScale, fT );
		TFLOAT fFade  = TMath::LerpTowards( rFrom.fFade, rTo.fFade, fT );

		TModelInstance* pShieldInstance = m_pShieldModelI->GetModelInstance();
		pShieldInstance->SetForceFade( TTRUE );
		pShieldInstance->SetFade( fFade );

		mTransform.Scale( s_fShieldScale * fScale, s_fShieldScale * fScale, s_fShieldScale * fScale );
		m_pShieldModelI->SetTransform( mTransform );

		if ( m_fShieldTime > rTo.fTime && ++m_iCurShieldKey == 3 )
		{
			if ( m_iState == STATE_SHIELD_ACTIVATING )
			{
				SetState( STATE_SHIELD_ACTIVE );
			}
			else
			{
				OnStateFinished();
				m_pShieldModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
			}
		}
	}
	else if ( m_iState == STATE_SHIELD_ACTIVE )
	{
		mTransform.Scale( s_fShieldScale, s_fShieldScale, s_fShieldScale );
		m_pShieldModelI->SetTransform( mTransform );
	}
}

// The animation of AnimTable's m_pAnims04 (SetState 0x1E) is over
// $deBlob: FUNCTION 007583f0
void ABlobAnimationComponent::UpdateState30( TFLOAT a_fDeltaTime )
{
	if ( m_pCurrentAnim == TNULL )
		return;

	if ( !IsAnimPlaying( m_pCurrentAnim ) || IsAnimFinished( m_pCurrentAnim ) )
	{
		m_pOwner->PopDeath();
		StopCurrentAnim( 0.15f );
	}
}

// $deBlob: FUNCTION 00757f90
void ABlobAnimationComponent::UpdateState19( TFLOAT a_fDeltaTime )
{
	if ( IsAnimPlaying( &ms_pAnimationProperties->m_pCommon->m_pAnims[ 0x19 ] ) && !IsAnimFinished( &ms_pAnimationProperties->m_pCommon->m_pAnims[ 0x19 ] ) )
		return;

	OnStateFinished();
}

// $deBlob: FUNCTION 00757ff0
void ABlobAnimationComponent::UpdateState20( TFLOAT a_fDeltaTime )
{
	AnimSet* pCommon = ms_pAnimationProperties->m_pCommon;
	Anim*    pAnims  = pCommon->m_pAnims;

	if ( m_pStateAnim != TNULL && !( IsAnimPlaying( m_pStateAnim ) && !IsAnimFinished( m_pStateAnim ) ) )
	{
		if ( m_pStateAnim == &pAnims[ 0x1A ] )
		{
			if ( m_pOwner->m_eState == ABlobEntity::STATE_BALLISTIC )
			{
				m_pStateAnim = &pAnims[ 0x1B ];
				PlayAnim( m_pStateAnim, 0.0f );
			}
		}
		else
		{
			m_pStateAnim = TNULL;
		}
	}

	if ( m_pOwner->m_eState == ABlobEntity::STATE_BALLISTIC && ( *TREINTERPRETCAST( TUINT8*, TREINTERPRETCAST( TUINT8*, m_pOwner ) + 0x32C ) & 0x3C ) == 0 )
	{
		if ( m_pStateAnim == TNULL && !IsAnimPlaying( &pAnims[ 0x1C ] ) )
			PlayAnim( &pAnims[ 0x1C ], 0.0f );
	}
	else
	{
		OnStateFinished();
	}
}

// $deBlob: FUNCTION 00757a50
void ABlobAnimationComponent::UpdateState11( TFLOAT a_fDeltaTime )
{
	if ( m_pOwner->m_eState != ABlobEntity::STATE_DEAD )
		OnStateFinished();
}

// $deBlob: FUNCTION 00757a70
void ABlobAnimationComponent::UpdateState21( TFLOAT a_fDeltaTime )
{
	if ( m_pStateAnim != TNULL )
	{
		if ( IsAnimPlaying( m_pStateAnim ) && !IsAnimFinished( m_pStateAnim ) )
			return;

		m_pStateAnim = TNULL;
	}
	else if ( m_pOwner->m_eState != ABlobEntity::STATE_RESPAWN )
	{
		OnStateFinished();
	}
}

// The states 0x1A..0x20 (except 0x1E) wait for their animation to end (no own function)
void ABlobAnimationComponent::UpdateStateAnimFinished()
{
	if ( !IsAnimPlaying( m_pCurrentAnim ) || IsAnimFinished( m_pCurrentAnim ) )
		OnStateFinished();
}

// Whether the idle animations may play
// $deBlob: FUNCTION 007563a0
TBOOL ABlobAnimationComponent::IsIdleAllowed()
{
	ABlobEntity* pOwner = m_pOwner;

	if ( ( TUINT8( pOwner->m_uiButtons >> 5 ) & 1 ) != 0 || m_uiEyeState != 0 || ( pOwner->m_uiFlags & 0x2080000 ) != 0 )
		return TFALSE;

	if ( pOwner->m_eState == ABlobEntity::STATE_LEVEL_EXIT || pOwner->m_eState == ABlobEntity::STATE_GRASSBOMB || ( *TREINTERPRETCAST( TUINT32*, TREINTERPRETCAST( TUINT8*, pOwner ) + 0x318 ) & 0x100000 ) != 0 )
		return TFALSE;

	return TTRUE;
}

// $deBlob: FUNCTION 00758670
TBOOL ABlobAnimationComponent::IsAnimPlaying( Anim* a_pAnim )
{
	TSkeletonInstance* pSkeletonInstance = m_pCharModelI->GetAnimInterface()->GetSkeletonInstance();

	TUINT16 iAnim = a_pAnim->m_iAnim;

	TAnimation* pAnimation = pSkeletonInstance != TNULL ? pSkeletonInstance->IsAnimating( iAnim ) : TNULL;
	return pAnimation != TNULL;
}

// $deBlob: FUNCTION 00758650
TBOOL ABlobAnimationComponent::IsAnimFinished( Anim* a_pAnim )
{
	return m_pCharModelI->GetAnimInterface()->IsAnimationFinished( a_pAnim->m_iAnim );
}

// $deBlob: FUNCTION 00756aa0
void ABlobAnimationComponent::UpdateState1( TFLOAT a_fDeltaTime )
{
	ABlobEntity* pOwner = m_pOwner;

	if ( pOwner->m_iLeechbotsAttached <= 0 )
	{
		if ( ( m_pBallBodyModelI->GetFlags() & AModelInstance2::FLAG_Visible ) == 0 )
		{
			SetState( STATE_ANIMTOCHAR );
			return;
		}

		switch ( pOwner->m_eState )
		{
			case ABlobEntity::STATE_ROLL:
			case ABlobEntity::STATE_FLOAT:
			case ABlobEntity::STATE_LEVEL_EXIT:
			{
				TFLOAT fSpeed;
				if ( GetBlobSpeedLimit( pOwner, pOwner->m_iSize, ABlobEntity::GetCommonProperties()->m_afUnkA0[ 2 ], fSpeed ) < fSpeed )
				{
					SetState( STATE_FADETOBALL );
					return;
				}
				break;
			}
			case ABlobEntity::STATE_BALLISTIC:
			case ABlobEntity::STATE_CANNON:
				SetState( STATE_FADETOBALL );
				return;
			case ABlobEntity::STATE_STUCKWALL:
			{
				TINT iZJumpState = pOwner->GetZJumpState();

				if ( iZJumpState != ABlobZJumpComponent::STATE_AIMNODE && iZJumpState != ABlobZJumpComponent::STATE_AIM )
				{
					SetState( STATE_FADETOBALL );
					return;
				}
				break;
			}
		}

		if ( m_fPaintFlashTimer <= 0.0f )
		{
			ABlobEntity::CommonProperties* pCommon = ABlobEntity::GetCommonProperties();
			BlendModels( m_pBallBodyModelI, m_pCharModelI, 0.0f, pCommon->m_afUnkA0[ 0 ], ( a_fDeltaTime * 2.0f ) / pCommon->m_afUnkA0[ 1 ] );
			return;
		}

		m_pCharModelI->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
		m_pCharModelI->GetModelInstance()->SetForceFade( TTRUE );
		m_pCharModelI->GetModelInstance()->SetFade( 1.0f );
		m_pBallBodyModelI->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		return;
	}

	SetState( STATE_FADETOBALL );
}

// $deBlob: FUNCTION 00756c00
void ABlobAnimationComponent::UpdateState2( TFLOAT a_fDeltaTime )
{
	ABlobEntity* pOwner = m_pOwner;

	if ( 0 < pOwner->m_iLeechbotsAttached )
	{
		SetState( STATE_ANIMTOBALL );
		return;
	}

	switch ( pOwner->m_eState )
	{
		case ABlobEntity::STATE_ROLL:
		case ABlobEntity::STATE_FLOAT:
		case ABlobEntity::STATE_LEVEL_EXIT:
		{
			TFLOAT fSpeed;
			if ( GetBlobSpeedLimit( pOwner, pOwner->m_iSize, ABlobEntity::GetCommonProperties()->m_afUnkA0[ 2 ], fSpeed ) < fSpeed )
			{
				SetState( STATE_ANIMTOBALL );
				return;
			}
			break;
		}
		case ABlobEntity::STATE_BALLISTIC:
		case ABlobEntity::STATE_CANNON:
			SetState( STATE_ANIMTOBALL );
			return;
		case ABlobEntity::STATE_STUCKWALL:
		{
			TINT iZJumpState = pOwner->GetZJumpState();

			if ( iZJumpState != ABlobZJumpComponent::STATE_AIMNODE && iZJumpState != ABlobZJumpComponent::STATE_AIM )
			{
				SetState( STATE_ANIMTOBALL );
				return;
			}
			break;
		}
	}

	Anim* pAnim;

	if ( ( pOwner->GetPaintColour() >> 4 & 1 ) == 0 )
		pAnim = GetSurfaceAnim5( GetCurrentSurface() );
	else
		pAnim = &ms_pAnimationProperties->m_pCommon->m_pAnims[ 12 ];

	AToshiAnimInterface2* pAnimInterface = m_pCharModelI->GetAnimInterface();

	if ( pAnimInterface->GetSkeletonInstance() == TNULL || pAnimInterface->GetSkeletonInstance()->IsAnimating( pAnim->m_iAnim ) == TNULL )
	{
		PlayAnim( pAnim, 0.15f );
	}
	else if ( pAnimInterface->IsAnimationFinished( pAnim->m_iAnim ) )
	{
		if ( pOwner->m_eState == ABlobEntity::STATE_FLOAT )
			SetState( STATE_FLOAT );
		else
			SetState( STATE_CHAR );
	}
}

// $deBlob: FUNCTION 007585d0
void ABlobEntity::GetBlobVelocity( TVector4& a_rOutVelocity ) const
{
	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBody, a_rOutVelocity );

	if ( ( m_oMotion.m_uiContactFlags & 0x20 ) != 0 )
	{
		a_rOutVelocity.x -= m_oMotion.m_vUnk220.x;
		a_rOutVelocity.y -= m_oMotion.m_vUnk220.y;
		a_rOutVelocity.z -= m_oMotion.m_vUnk220.z;
	}
}

// $deBlob: FUNCTION 00755f80
void ABlobAnimationComponent::UnloadAnimationProperties()
{
	ms_pAnimationProperties = TNULL;
}

// $deBlob: FUNCTION 00755af0
void ABlobAnimationComponent::LoadAnimationProperties( TTRB* a_pTRB )
{
	ms_pAnimationProperties = TSTATICCAST( AnimationProperties, a_pTRB->GetSymbolAddress( "BlobAnimationProperties" ) );

	if ( ms_pSkeletonChar == TNULL )
		return;

	ResolveAnimations( ms_pAnimationProperties->m_pAnims, ms_pAnimationProperties->m_iNumAnims, TTRUE );

	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
		ResolveAnimations( ms_pAnimationProperties->m_pSPAnims, ms_pAnimationProperties->m_iNumSPAnims, TTRUE );
	else
		ResolveAnimations( ms_pAnimationProperties->m_pMPAnims, ms_pAnimationProperties->m_iNumMPAnims, TTRUE );
}

// $deBlob: FUNCTION 00756560
void ABlobAnimationComponent::StartPaintFlash()
{
	T2ModelPtr pModel = m_pCharModelI->GetModel()->GetModelPtr();
	TModelLOD& rLOD   = pModel->GetLOD( 0 );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		ASkinMaterialHAL* pSkinMat = TSTATICCAST( ASkinMaterialHAL, rLOD.GetMeshes()[ i ]->GetMaterial() );

		if ( TUINT( i ) < 2 )
		{
			TOrderTable* pOrderTable = pSkinMat->GetOrderTable();
			pSkinMat->SetBlendMode( 0xD, 0 );
			pSkinMat->SetOrderTable( pOrderTable, TFALSE );
			pSkinMat->SetFlags( ASkinMaterial::SKINFLAG_DEPTHWRITE, TTRUE );
			m_fPaintFlashTimer = PAINT_FLASH_TIME;

			TASSERT( pSkinMat->GetNumTextures() == 2 );
			TASSERT( MULTIPASSBLENDTYPE_OVERRIDE_CHAN2 == pSkinMat->GetTevMode( 0 ) );
			pSkinMat->SetTevMode( 1, 0x1A );
			ASkinShaderHAL::s_vTevMode1AColour.w = 0.0f;
		}
	}
}

// $deBlob: FUNCTION 007563f0
void ABlobAnimationComponent::ResetPaintFlash()
{
	m_fPaintFlashTimer = 0.0f;

	T2ModelPtr pModel = m_pCharModelI->GetModel()->GetModelPtr();
	TModelLOD& rLOD   = pModel->GetLOD( 0 );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		ASkinMaterialHAL* pSkinMat = TSTATICCAST( ASkinMaterialHAL, rLOD.GetMeshes()[ i ]->GetMaterial() );

		if ( TUINT( i ) < 2 )
		{
			pSkinMat->SetBlendMode( 1, 0 );
			pSkinMat->SetFlags( ASkinMaterial::SKINFLAG_DEPTHWRITE, TTRUE );

			TASSERT( pSkinMat->GetNumTextures() == 2 );
			TASSERT( MULTIPASSBLENDTYPE_OVERRIDE_CHAN2 == pSkinMat->GetTevMode( 0 ) );
			pSkinMat->SetTevMode( 1, 2 );
			ASkinShaderHAL::s_vTevMode1AColour.w = 0.0f;
		}
	}
}

// $deBlob: FUNCTION 00755950
void ABlobAnimationComponent::OnColourChanged()
{
	TUINT32 uiColour = m_pOwner->GetPaintColour();

	if ( uiColour != 0 && !( TUINT8( uiColour >> 4 ) & 1 ) && ACameraManager::GetSingleton()->GetNumActiveCameras() <= 1 )
		m_bUnk6C = TTRUE;
}

// $deBlob: FUNCTION 00755800
TBOOL ABlobAnimationComponent::PlayOneShot( Anim* a_pAnim )
{
	if ( m_pStateAnim != TNULL || m_iState != STATE_CHAR )
		return TFALSE;

	TASSERT( ms_pSkeletonChar->GetSequence( a_pAnim->m_iAnim )->GetMode() != TSkeletonSequence::MODE_LOOPED, "!ms_pSkeletonChar->GetSequence(a_roAnim.m_iAnim)->IsLooping()" );
	TASSERT( ms_pSkeletonChar->GetSequence( a_pAnim->m_iEyeOLay )->GetMode() != TSkeletonSequence::MODE_LOOPED, "!ms_pSkeletonChar->GetSequence(a_roAnim.m_iEyeOLay)->IsLooping()" );

	m_pStateAnim = a_pAnim;
	PlayAnim( a_pAnim, 0.15f );
	return TTRUE;
}
