#include "pch.h"
#include "ATreeInstance.h"
#include "ABuildingManager.h"
#include "Animation/AModel2.h"
#include "Animation/AModelFactory.h"
#include "Animation/AToshiAnimInterface2.h"
#include "Challenges/AChallengeManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ABlobEntity/ABlobColourMaps.h"
#include "Fx/AParticleEffectHelper.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Render/Material/AMaterialInfo.h"
#include "Sound/ASoundInterface.h"
#include "Terrain2/ATerrain2Interface.h"

#include <Render/TAnimation.h>
#include <Render/TSkeleton.h>
#include <Render/TRender.h>
#include <Render/TRenderContext.h>
#include <Render/TCameraObject.h>
#include <Render/TMesh.h>
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <Shaders/WorldShader/TWorldShader.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffect.h>

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TParticleEffect* ATreeInstance::sm_pTreeEmergeEffects[ NUM_EMERGE_EFFECTS ];

// The scale of the tree model while it grows (tree_up forwards) and of the dead tree while it shrinks
static const ATreeInstance::ScaleKey s_aGrowKeys[] = {
	{ 0.0f, 0.01f, 0.01f },
	{ 0.33f, 0.01f, 0.01f },
	{ 0.36f, 0.2f, 0.2f },
	{ 0.43f, 0.25f, 2.2f },
	{ 0.46f, 1.1f, 0.6f },
	{ 0.53f, 1.0f, 1.1f },
	{ 0.6f, 1.0f, 0.85f },
	{ 0.66f, 1.0f, 1.0f },
};
static TINT s_iNumGrowKeys = TARRAYSIZE( s_aGrowKeys );

// The scale of the tree model while it shrinks (tree_up backwards: inked) and of the dead tree while it grows back
static const ATreeInstance::ScaleKey s_aShrinkKeys[] = {
	{ 0.0f, 1.0f, 1.0f },
	{ 0.06f, 0.5f, 2.6f },
	{ 0.1f, 0.6f, 0.1f },
	{ 0.23f, 0.01f, 0.01f },
};
static TINT s_iNumShrinkKeys = TARRAYSIZE( s_aShrinkKeys );

// The mesh of the tree model with the glint material (the dead tree's is mesh 0)
static TINT s_iTreeGlintMesh = 1;

// $deBlob: FUNCTION 00581980
void ATreeInstance::OnCreated( TerrainTypes::CellInstance* a_pInstance )
{
	ATreeInstance* pTree    = TSTATICCAST( ATreeInstance, a_pInstance->m_pModelInstance2 );
	pTree->m_pDeadTreeModel = pTree->CreateModel( TSTATICCAST( const TCHAR*, a_pInstance->m_pTypeData )[ 0 ], a_pInstance );

	if ( pTree->m_pDeadTreeModel != TNULL )
		pTree->m_iTreeDownId = TINT16( pTree->m_pDeadTreeModel->GetAnimInterface()->GetAnimID( "tree_down" ) );

	AModelInstance2* pTreeModel = pTree->CreateModel( TSTATICCAST( const TCHAR*, a_pInstance->m_pTypeData )[ 1 ], a_pInstance );
	pTree->m_pTreeModel         = pTreeModel;

	if ( pTreeModel != TNULL )
	{
		pTreeModel->SetFlag( FLAG_Visible, TFALSE );
		pTree->m_iTreeUpId = TINT16( pTreeModel->GetAnimInterface()->GetAnimID( "tree_up" ) );
	}
}

// $deBlob: FUNCTION 005819f0
void ATreeInstance::OnBlobTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const TVector4* a_pPosition )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Tree );

	ATreeInstance* pTree = TSTATICCAST( ATreeInstance, a_pInstance->m_pModelInstance2 );

	if ( pTree->m_bTransforming )
		return;

	if ( !a_pBlob->IsA( &TGetClass( ABlobEntity ) ) )
		return;

	TUINT32 uiColour = a_pBlob->GetPaintColour();

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
		uiColour = ABlobColourComponent::COLOUR_BLACK;

	// A paint challenge target gets the target's colour
	if ( a_pBlob->GetBlobFlags() & 0x200000 )
	{
		TUINT32 uiTargetColour = AChallengeManager::GetSingleton()->GetPaintChallengeIndex( TINT( TREINTERPRETCAST( TUINTPTR, a_pInstance ) ) );

		if ( uiTargetColour != TUINT32( -1 ) && uiTargetColour != 0 )
			uiColour = uiTargetColour;
	}

	if ( pTree->m_bTransformed )
	{
		if ( uiColour == ABlobColourComponent::COLOUR_BLACK )
		{
			// Inked: the score of the tree is taken back
			Ink( a_pInstance );

			AGameStatus* pGameStatus = AGameStatus::GetSingleton();
			TINT         iPlayer     = APlayerManager::GetSingleton()->GetPlayerIndex( a_pBlob );
			TINT         iPoints     = pGameStatus->GetLevelData().GetPoints( iPlayer );
			TINT*        pScore      = &iPoints;

			if ( TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x9C / 4 ] <= iPoints )
				pScore = &TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x9C / 4 ];

			pGameStatus->GetLevelData().AddScore( iPlayer, -*pScore, -1 );
			return;
		}
	}
	else if ( uiColour == ABlobColourComponent::COLOUR_BLACK )
	{
		return;
	}

	if ( uiColour != ABlobColourComponent::COLOUR_WHITE && TUINT32( pTree->m_iColour ) != uiColour )
	{
		pTree->Paint( a_pInstance, a_pBlob, a_pPosition, uiColour );
		a_pBlob->RemovePaintPoints( 1, TFALSE );
	}
}

// $deBlob: FUNCTION 00581b30
TBOOL ATreeInstance::OnTick( TerrainTypes::CellInstance* a_pInstance, TFLOAT a_fDeltaTime, TFLOAT a_fTime )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Tree );

	return TSTATICCAST( ATreeInstance, a_pInstance->m_pModelInstance2 )->UpdateTransform( a_fDeltaTime );
}

// $deBlob: FUNCTION 00581b90
void ATreeInstance::OnReset( TerrainTypes::CellInstance* a_pInstance )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Tree );

	ATreeInstance*   pTree      = TSTATICCAST( ATreeInstance, a_pInstance->m_pModelInstance2 );
	AModelInstance2* pDeadModel = pTree->m_pDeadTreeModel;

	pTree->m_bTransformed  = TFALSE;
	pTree->m_bTransforming = TFALSE;
	pTree->m_iColour       = ABlobColourComponent::COLOUR_BLACK;

	if ( pDeadModel != TNULL )
	{
		pDeadModel->SetFlag( FLAG_Visible, TTRUE );
		pDeadModel->GetAnimInterface()->StopAllAnims( 0.0f );
		pDeadModel->SetScale( 1.0f );
	}

	AModelInstance2* pTreeModel = pTree->m_pTreeModel;

	if ( pTreeModel != TNULL )
	{
		pTreeModel->SetFlag( FLAG_Visible, TFALSE );
		pTreeModel->GetAnimInterface()->StopAllAnims( 0.0f );
		pTreeModel->SetScale( 1.0f );
	}
}

// $deBlob: FUNCTION 00581c70
ATreeInstance::ATreeInstance( AModel2* a_pModel, TBOOL a_bCreateSoundBreakpoints )
    : AModelInstance2( a_pModel, a_bCreateSoundBreakpoints, TFALSE )
{
	m_iGlintIndex           = -1;
	m_pDeadTreeModel        = TNULL;
	m_pTreeModel            = TNULL;
	m_iTreeUpId             = -1;
	m_iTreeDownId           = -1;
	m_iColour               = ABlobColourComponent::COLOUR_BLACK;
	m_bTransformed          = TFALSE;
	m_bTransforming         = TFALSE;
	m_apGlintMaterials[ 0 ] = TNULL;
	m_apGlintMaterials[ 1 ] = TNULL;
	SetFlag( 0x2000, TFALSE );
}

// $deBlob: FUNCTION 00582ea0
// $deBlob: FUNCTION 00581d20
ATreeInstance::~ATreeInstance()
{
	for ( TINT i = 0; i < 2; i++ )
	{
		if ( m_apGlintMaterials[ i ] != TNULL )
			m_apGlintMaterials[ i ]->Destroy();
	}

	if ( m_pDeadTreeModel != TNULL )
		AModelFactory::GetSingleton()->DestroyInstance( m_pDeadTreeModel );

	if ( m_pTreeModel != TNULL )
		AModelFactory::GetSingleton()->DestroyInstance( m_pTreeModel );
}

// $deBlob: FUNCTION 00581dd0
TBOOL ATreeInstance::UpdateTransform( TFLOAT a_fDeltaTime )
{
	TASSERT( m_bTransforming );

	m_pTreeModel->SetFlag( FLAG_Visible, TTRUE );
	m_pTreeModel->UpdateImp( a_fDeltaTime );
	m_pDeadTreeModel->UpdateImp( a_fDeltaTime );

	TSkeletonInstance* pTreeSkeleton = m_pTreeModel->GetAnimInterface()->GetSkeletonInstance();
	TAnimation*        pTreeAnim     = ( pTreeSkeleton != TNULL ) ? pTreeSkeleton->IsAnimating( TINT16( m_iTreeUpId ) ) : TNULL;

	TFLOAT     fDuration = pTreeAnim->GetSequencePtr()->GetDuration();
	TSkeleton* pDeadSkel = m_pDeadTreeModel->GetModelInstance()->GetSkeletonInstance()->GetSkeleton();
	TASSERT( m_iTreeDownId < pDeadSkel->GetSequenceCount() );

	TFLOAT fEnd = ( pTreeAnim->GetSpeed() < 0.0f ) ? fDuration * 2.0f : fDuration;

	if ( pTreeAnim->GetTotalTime() < fEnd )
	{
		if ( pTreeAnim->GetSpeed() < 0.0f )
		{
			if ( m_iColour == ABlobColourComponent::COLOUR_BLACK )
			{
				// Inked: the dead tree comes back once the tree has shrunk
				TFLOAT fTime = fDuration * 2.0f - pTreeAnim->GetTotalTime();

				if ( fTime < pDeadSkel->GetSequences()[ m_iTreeDownId ].GetDuration() )
				{
					m_pDeadTreeModel->GetAnimInterface()->AddAnim( TINT16( m_iTreeDownId ), 0.0f, 1.0f, -1.0f )->SetTime( fTime );
					m_pDeadTreeModel->SetFlag( FLAG_Visible, TTRUE );
				}
			}
			else
			{
				// Repainted in another colour: the tree grows again in the new colour once it has shrunk
				TFLOAT fTime = fDuration - pTreeAnim->GetSeqTime();

				if ( fTime > s_aShrinkKeys[ s_iNumShrinkKeys - 1 ].m_fTime )
				{
					pTreeAnim->SetSpeed( 1.0f );
					pTreeAnim->SetTime( fTime );
					ApplyColour();
				}
			}
		}

		if ( pTreeAnim->GetSpeed() >= 0.0f )
			SetScaleFromCurve( m_pTreeModel, pTreeAnim->GetSeqTime(), s_aGrowKeys, s_iNumGrowKeys );
		else
			SetScaleFromCurve( m_pTreeModel, fDuration - pTreeAnim->GetSeqTime(), s_aShrinkKeys, s_iNumShrinkKeys );

		TSkeletonInstance* pDeadSkeleton = m_pDeadTreeModel->GetAnimInterface()->GetSkeletonInstance();

		if ( pDeadSkeleton != TNULL )
		{
			TAnimation* pDeadAnim = pDeadSkeleton->IsAnimating( TINT16( m_iTreeDownId ) );

			if ( pDeadAnim != TNULL )
			{
				if ( pTreeAnim->GetSpeed() < 0.0f )
				{
					SetScaleFromCurve( m_pDeadTreeModel, fDuration - pDeadAnim->GetSeqTime(), s_aGrowKeys, s_iNumGrowKeys );
					return TFALSE;
				}

				SetScaleFromCurve( m_pDeadTreeModel, pDeadAnim->GetSeqTime(), s_aShrinkKeys, s_iNumShrinkKeys );
			}
		}

		return TFALSE;
	}

	// The transform is over
	m_bTransformed = m_iColour != ABlobColourComponent::COLOUR_BLACK;
	m_pTreeModel->SetFlag( FLAG_Visible, m_bTransformed );
	m_pDeadTreeModel->SetFlag( FLAG_Visible, !m_bTransformed );
	m_pTreeModel->SetScale( 1.0f );
	m_pDeadTreeModel->SetScale( 1.0f );
	m_bTransforming = TFALSE;
	m_pTreeModel->GetAnimInterface()->StopAllAnims( 0.0f );
	m_pDeadTreeModel->GetAnimInterface()->StopAllAnims( 0.0f );

	return TTRUE;
}

// $deBlob: FUNCTION 00582170
void __stdcall ATreeInstance::RenderCallback( TModelInstance* a_pModelInstance, void* a_pUserData )
{
	TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, a_pUserData );
	ATreeInstance*              pTree     = TSTATICCAST( ATreeInstance, pInstance->m_pModelInstance2 );

	if ( !( pInstance->m_pCell->m_uiFlags & 0x10000 ) )
		return;

	TTransformObject& rCamTransform = TRender::GetSingleton()->GetCurrentContext()->GetCameraObject()->GetTransformObject();
	const TVector3&   rCamPos       = rCamTransform.GetTranslation();
	const TVector4&   rCentre       = *pInstance->m_ppCentre[ 0 ];

	TFLOAT fDZ = rCamPos.z - rCentre.z;
	TFLOAT fDX = rCamPos.x - rCentre.x;

	if ( ( fDZ * fDZ + fDX * fDX ) - pInstance->m_ppCentre[ 1 ]->MagnitudeSqXZ() >= ATerrain2Interface::GetInstanceLODDistanceSq( 2 ) )
		return;

	TBOOL      bSwapped     = TFALSE;
	T2ModelPtr pModel       = a_pModelInstance->GetModel();
	TModelLOD& rLOD         = pModel->GetLOD( 0 );
	TMesh*     pMesh        = TNULL;
	TMaterial* pOldMaterial = TNULL;

	if ( !pTree->m_bTransforming && a_pModelInstance != TNULL && pTree->m_iGlintIndex != -1 )
	{
		bSwapped = TTRUE;
		TWorldMaterialHAL* pGlint;

		if ( pTree->m_bTransformed )
		{
			TASSERT( pTree->GetTreeModel()->GetModelInstance() == a_pModelInstance );
			pMesh  = rLOD.GetMeshes()[ s_iTreeGlintMesh ];
			pGlint = TSTATICCAST( TWorldMaterialHAL, pTree->m_apGlintMaterials[ 0 ] );
		}
		else
		{
			TASSERT( pTree->GetDeadTreeModel()->GetModelInstance() == a_pModelInstance );
			pGlint = TSTATICCAST( TWorldMaterialHAL, pTree->m_apGlintMaterials[ 1 ] );
			pMesh  = rLOD.GetMeshes()[ 0 ];
		}

		pOldMaterial = pMesh->GetMaterial();
		pGlint->SetUserDataIndex( TWorldMaterialHAL::UData_0, pTree->m_iGlintIndex );
		pMesh->SetMaterial( pGlint );
	}

	AModelInstance2::RenderGeneric( a_pModelInstance, ( a_pModelInstance == pTree->m_pTreeModel->GetModelInstance() ) ? pTree->m_pTreeModel : pTree->m_pDeadTreeModel );

	if ( bSwapped )
		pMesh->SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 005823a0
void ATreeInstance::OnPaintRandom( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Tree );

	TINT iColour = ABlobColourComponent::GetRandomColour();
	TSTATICCAST( ATreeInstance, a_pInstance->m_pModelInstance2 )->Paint( a_pInstance, a_pBlob, a_pInstance->m_ppCentre[ 0 ], iColour );
}

// $deBlob: FUNCTION 00582400
void ATreeInstance::Ink( TerrainTypes::CellInstance* a_pInstance )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Tree );

	ATreeInstance* pTree = TSTATICCAST( ATreeInstance, a_pInstance->m_pModelInstance2 );
	TASSERT( pTree->m_bTransformed );
	TASSERT( !pTree->m_bTransforming );

	pTree->m_bTransforming = TTRUE;
	ABuildingManager::GetSingleton()->AddTickInstance( a_pInstance );
	pTree->m_iColour = ABlobColourComponent::COLOUR_BLACK;

	// The tree shrinks (tree_up backwards)
	TAnimation* pAnim = pTree->m_pTreeModel->GetAnimInterface()->AddAnim( TINT16( pTree->m_iTreeUpId ), 0.0f, 1.0f, -1.0f );
	pAnim->SetTime( pAnim->GetSequencePtr()->GetDuration() );

	TVector4 vPosition;
	pTree->GetPosition( vPosition );
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_LEVELS_COMMON_LEVELS, ASoundMap::EVENTID( 0x94 ), vPosition, 0 );

	AChallengeManager::GetSingleton()->OnPaintEvent2( pTree );

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	if ( pGameStatus->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE )
		pGameStatus->GetLevelData().AddTreesPainted( 0, -1 );
}

// $deBlob: FUNCTION 005825b0
void ATreeInstance::CreateEffects()
{
	TParticleEffect* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_TreeEmerge" );

	for ( TINT i = 0; i < NUM_EMERGE_EFFECTS; i++ )
	{
		TASSERT( TNULL == sm_pTreeEmergeEffects[ i ] );
		sm_pTreeEmergeEffects[ i ] = pEffect->Copy();
		AParticleEffectHelper::SetColour( sm_pTreeEmergeEffects[ i ], ABlobColourComponent::ms_auiColours[ i + 1 ], "Blob_TreeEmerge_Leaves" );
	}
}

// $deBlob: FUNCTION 00582640
void ATreeInstance::DestroyEffects()
{
	for ( TINT i = 0; i < NUM_EMERGE_EFFECTS; i++ )
	{
		if ( sm_pTreeEmergeEffects[ i ] != TNULL )
			delete sm_pTreeEmergeEffects[ i ];

		sm_pTreeEmergeEffects[ i ] = TNULL;
	}
}

// $deBlob: FUNCTION 00582680
void ATreeInstance::CreateGlintMaterials()
{
	TTexture* pTexture = TTextureManager::GetSingleton()->FindTexture( "building_glint.tga" );
	pTexture->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

	TWorldMaterialHAL* apSources[ 2 ];

	{
		T2ModelPtr pModel = m_pTreeModel->GetModelInstance()->GetModel();
		TMaterial* pMat   = pModel->GetLOD( 0 ).GetMeshes()[ s_iTreeGlintMesh ]->GetMaterial();
		TASSERT( pMat->IsA( &TGetClass( TWorldMaterialHAL ) ) );
		apSources[ 0 ] = TSTATICCAST( TWorldMaterialHAL, pMat );
	}

	{
		T2ModelPtr pModel = m_pDeadTreeModel->GetModelInstance()->GetModel();
		TMaterial* pMat   = pModel->GetLOD( 0 ).GetMeshes()[ 0 ]->GetMaterial();
		TASSERT( pMat->IsA( &TGetClass( TWorldMaterialHAL ) ) );
		apSources[ 1 ] = TSTATICCAST( TWorldMaterialHAL, pMat );
	}

	TWorldShaderHAL* pWorldShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );

	for ( TINT i = 0; i < 2; i++ )
	{
		TWorldMaterialHAL* pMaterial = TSTATICCAST( TWorldMaterialHAL, pWorldShader->CreateMaterial( TNULL ) );
		TASSERT( pMaterial == TNULL || pMaterial->IsA( &TGetClass( TWorldMaterialHAL ) ) );
		m_apGlintMaterials[ i ] = pMaterial;
		pMaterial->Create( 0 );

		TWorldMaterialHAL* pSource = apSources[ i ];
		AMaterialInfo::CopyMaterial( pSource, pMaterial );

		TUINT uiStage = pMaterial->GetTextureNum();
		pMaterial->SetTextureNum( uiStage + 1 );
		pMaterial->SetTexture( uiStage, pTexture );
		pMaterial->SetUVMode( uiStage, TWorldMaterialHAL::UVMODE_PROJECTED );

		const TCHAR* szName = pMaterial->GetMaterialName();

		if ( szName == TNULL || ( TStringManager::String8Compare( szName, "window", 6 ) != 0 && TStringManager::String8Compare( szName, "sticker", 7 ) != 0 ) )
		{
			pMaterial->SetBlendMode( 13 );
			pMaterial->SetMultiPassBlendType( uiStage, TWorldMaterialHAL::MULTIPASSBLENDTYPE_BLENDWRITEALPHA );
		}
		else
		{
			pMaterial->SetMultiPassBlendType( uiStage, TWorldMaterialHAL::MULTIPASSBLENDTYPE_WINDOWSPROJ );
		}

		pMaterial->SetOrderTable( pSource->GetOrderTable(), 0, TTRUE );
	}
}

// $deBlob: FUNCTION 005829f0
void ATreeInstance::Paint( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const TVector4* a_pPosition, TINT a_iColour )
{
	if ( m_bTransforming )
		return;

	m_iColour       = a_iColour;
	m_bTransforming = TTRUE;
	ABuildingManager::GetSingleton()->AddTickInstance( a_pInstance );

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	TINT*        pScoring    = TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() );

	if ( !m_bTransformed )
	{
		// The dead tree goes down, the tree grows in the colour
		m_pDeadTreeModel->GetAnimInterface()->AddAnim( TINT16( m_iTreeDownId ), 0.0f, 1.0f, 1.0f );
		m_pTreeModel->GetAnimInterface()->AddAnim( TINT16( m_iTreeUpId ), 0.0f, 1.0f, 1.0f );
		ApplyColour();

		TINT iPlayer = APlayerManager::GetSingleton()->GetPlayerIndex( a_pBlob );
		pGameStatus->AddBonusScore( iPlayer, pScoring[ 0x9C / 4 ], a_pPosition );
		pGameStatus->GetLevelData().AddTreesPainted( iPlayer, 1 );

		ABuildingManager::TriggerLinks( a_pInstance, AEntityIOPacket::MESSAGE_TRIGGER );
	}
	else
	{
		// Repainted: the tree shrinks first (UpdateTransform grows it again in the new colour)
		TAnimation* pAnim = m_pTreeModel->GetAnimInterface()->AddAnim( TINT16( m_iTreeUpId ), 0.0f, 1.0f, -1.0f );
		pAnim->SetTime( pAnim->GetSequencePtr()->GetDuration() );

		TINT iPlayer = APlayerManager::GetSingleton()->GetPlayerIndex( a_pBlob );
		pGameStatus->AddBonusScore( iPlayer, TINT( TFLOAT( pScoring[ 0x9C / 4 ] ) * TREINTERPRETCAST( TFLOAT*, pScoring )[ 0x04 / 4 ] ), a_pPosition );
	}

	TASSERT( m_iColour > ABlobColourComponent::COLOUR_WHITE && m_iColour < ABlobColourComponent::COLOUR_BLACK );

	TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( sm_pTreeEmergeEffects[ m_iColour - 1 ] );

	if ( pEffect != TNULL )
		GetTransform( pEffect->GetTransform() );

	TVector4 vPosition;
	GetPosition( vPosition );
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_LEVELS_COMMON_LEVELS, ASoundMap::EVENTID( 0x93 ), vPosition, 0 );

	AChallengeManager::GetSingleton()->OnPaintEvent2( this );
}

// $deBlob: FUNCTION 00582c40
AModelInstance2* ATreeInstance::CreateModel( const TCHAR* a_szName, TerrainTypes::CellInstance* a_pInstance )
{
	if ( a_szName == TNULL || a_szName[ 0 ] == '\0' )
		return TNULL;

	TCHAR szName[ 128 ];
	TStringManager::String8Copy( szName, a_szName, 128 );
	TINT iLen = TStringManager::String8Length( szName );
	TStringManager::String8Copy( szName + iLen, ".trb", 128 - iLen );

	AModelInstance2* pInstance      = AModelFactory::GetSingleton()->CreateInstance( szName, TTRUE, TTRUE );
	TModelInstance*  pModelInstance = pInstance->GetModelInstance();
	pModelInstance->SetCustomRenderMethod( RenderCallback, a_pInstance );

	TMatrix44 oMatrix;
	GetTransform( oMatrix );
	pInstance->SetTransform( oMatrix );
	pInstance->SetFlag( FLAG_Visible, TTRUE );

	T2ModelPtr pModel     = pInstance->GetModelInstance()->GetModel();
	TModelLOD& rLOD       = pModel->GetLOD( 0 );
	TINT       iNumMeshes = rLOD.GetMeshCount();

	for ( TINT i = 0; i < iNumMeshes; i++ )
	{
		TWorldMaterialHAL* pMaterial = TSTATICCAST( TWorldMaterialHAL, rLOD.GetMeshes()[ i ]->GetMaterial() );
		TASSERT( pMaterial == TNULL || pMaterial->IsA( &TGetClass( TWorldMaterialHAL ) ) );

		const TCHAR* szMatName = pMaterial->GetMaterialName();

		// The leaves are drawn from both sides
		if ( szMatName != TNULL && ( TStringManager::String8FindString( szMatName, "tree04_leaf" ) != TNULL || TStringManager::String8FindString( szMatName, "tree_dam_leaf" ) != TNULL ) )
			pMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
	}

	return pInstance;
}

// $deBlob: FUNCTION 00582e70
void ATreeInstance::ApplyColour()
{
	m_pTreeModel->SetFlag( 0x1000, TTRUE );

	AModelInstance2* pTreeModel = m_pTreeModel;
	pTreeModel->SetMeshFlags( 0, 1, TTRUE );
	pTreeModel->SetMeshColour( 0, ABlobColourMaps::GetTreeColourMap()[ m_iColour ] );
}

// $deBlob: FUNCTION 005809b0
void __stdcall ATreeInstance::SetScaleFromCurve( AModelInstance2* a_pModel, TFLOAT a_fTime, const ScaleKey* a_pKeys, TINT a_iNumKeys )
{
	// The first key at or after a_fTime (retail unrolls the search by four)
	TINT i = 0;

	while ( i < a_iNumKeys && a_fTime > a_pKeys[ i ].m_fTime )
		i++;

	TFLOAT fScaleX;
	TFLOAT fScaleY;

	if ( i == 0 )
	{
		fScaleX = a_pKeys[ 0 ].m_fScaleX;
		fScaleY = a_pKeys[ 0 ].m_fScaleY;
	}
	else if ( i == a_iNumKeys )
	{
		fScaleX = a_pKeys[ i - 1 ].m_fScaleX;
		fScaleY = a_pKeys[ i - 1 ].m_fScaleY;
	}
	else
	{
		TFLOAT fT = ( a_fTime - a_pKeys[ i - 1 ].m_fTime ) / ( a_pKeys[ i ].m_fTime - a_pKeys[ i - 1 ].m_fTime );
		fScaleX   = TMath::LERP( a_pKeys[ i - 1 ].m_fScaleX, a_pKeys[ i ].m_fScaleX, fT );
		fScaleY   = TMath::LERP( a_pKeys[ i - 1 ].m_fScaleY, a_pKeys[ i ].m_fScaleY, fT );
	}

	a_pModel->SetScale( TVector4( fScaleX, fScaleY, fScaleX, 1.0f ) );
}
