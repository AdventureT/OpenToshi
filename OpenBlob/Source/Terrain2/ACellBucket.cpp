#include "pch.h"
#include "GameInterface/APlayerManager.h"
#include "ACellBucket.h"
#include "Render/Material/AMaterialInfo.h"
#include "ATerrain2.h"
#include "Collision/ACollisionInstance2.h"
#include "Collision/ACollisionObject.h"
#include "Animation/AModelInstance2.h"
#include "Buildings/ABuildingInstance.h"
#include "Buildings/AGrassInstance.h"
#include "Entities/AGndMotorManager.h"
#include "Challenges/AChallengeManager.h"
#include "Challenges/AChallenge.h"
#include "Challenges/AUberChallenge.h"
#include "GameController/AGameStatus.h"
#include "Render/Material/AMaterialManager.h"

#include <Render/TAssetInit.h>
#include <Render/TModelManager.h>
#include <Render/TWorldVIS.h>
#include <Shaders/WorldShader/TWorldShader.h>
#include <Platform/DX11/TMaterial_DX11.h>
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Toshi/TStringManager.h>
#include <Physics/Havok/TPhysicsManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ACellBucket* ACellBucket::ms_pCurrentBucket;

TSTATICASSERT( sizeof( ACellBucket ) == 0x40 );

// $deBlob: FUNCTION 0061bb60
ACellBucket::ACellBucket()
{
	m_pUnk18 = TNULL;

	m_pMatInfoPool = new AMaterialInfoPool( 0x10, TNULL );
	m_pUVAnimPool  = new AUVAnimationPool( 0x10, TNULL );

	m_heap = TNULL;
	Reset();
}

// $deBlob: FUNCTION 0061bc20
// $deBlob: FUNCTION 0061fe20
ACellBucket::~ACellBucket()
{
	TMemory_DestroyHeap( m_heap );

	if ( m_pUnk18 )
		TFree( m_pUnk18 );

	Reset();
	TASSERT( m_oUsedList.IsEmpty() );


	if ( m_pMatInfoPool != TNULL )
		delete m_pMatInfoPool;

	m_pMatInfoPool = TNULL;

	if ( m_pUVAnimPool != TNULL )
		delete m_pUVAnimPool;

	m_pUVAnimPool = TNULL;
}

// $deBlob: FUNCTION 0061bd60
void ACellBucket::Reset()
{
	m_pCell   = TNULL;
	m_pTRB    = TNULL;
	m_iRegion = TerrainTypes::CellVis::NONE;
	m_iCell   = TerrainTypes::CellVis::NONE;
	m_uiFlags = 0;

	ReleaseMaterials();
}

// $deBlob: FUNCTION 0061c5e0
void ACellBucket::ReleaseMaterials()
{
	while ( !m_oUsedList.IsEmpty() )
		ATerrain2::GetInstance()->ReleaseMaterial( TSTATICCAST( ATerrain2::MaterialEntry, m_oUsedList.PopFront() ) );
}

// $deBlob: FUNCTION 0061bdb0
void ACellBucket::Set( TerrainTypes::Cell* a_pCell, TINT a_iRegion, TINT a_iCell )
{
	m_pCell   = a_pCell;
	m_iRegion = a_iRegion;
	m_iCell   = a_iCell;
	m_uiFlags = 0;
}

// $deBlob: FUNCTION 0061c610
void ACellBucket::CreateInstanceMaterials( TerrainTypes::CellInstance* a_pInstance )
{
	TerrainTypes::InstanceMeshes* pMeshes         = a_pInstance->m_pSymbolData;
	AModelInstance2*              pModelInstance2 = TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 );

	{
		// The names of the meshes become the pointers of the model's meshes (RenderLightmappedInstance compares them)
		T2ModelPtr             pModel     = pModelInstance2->GetModelInstance()->GetModel();
		TTMDWin::TRBWorldData* pWorldData = TSTATICCAST( TTMDWin::TTRBWorldHeader, pModel->GetAssetData() )->m_pWorldData;

		if ( pWorldData->m_iNumGroups > 0 )
		{
			TTMDWin::TRBWorldCellGroup* pGroup = pWorldData->m_ppGroups[ 0 ];

			if ( pGroup != TNULL && pGroup->m_iNumCells > 0 && pGroup->m_ppCells[ 0 ]->m_iMeshCount != 1 )
			{
				TModelLOD& rLOD = pModel->GetLOD( 0 );

				for ( TUINT i = 0; i < pMeshes->m_uiNumMeshes; i++ )
				{
					TWorldMeshInstanceData::Source& rSource = pMeshes->m_pMeshes[ i ];

					for ( TINT j = 0; j < rLOD.GetMeshCount(); j++ )
					{
						TWorldMeshHAL* pMesh = TSTATICCAST( TWorldMeshHAL, rLOD.GetMesh( j ) );
						TASSERT( ( pMesh == TNULL ) || pMesh->IsA( &TGetClass( TWorldMeshHAL ) ) );
						TTRBWorldMesh* pTRBMesh = pMesh->GetTRBMesh();

						if ( rSource.m_uiMeshId == pTRBMesh->m_uiUnk20 && TStringManager::String8Compare( rSource.m_szName, pTRBMesh->m_szMaterialName ) == 0 )
						{
							rSource.m_szName = pTRBMesh->m_szMaterialName;
							break;
						}
					}
				}
			}
		}
	}

	TUINT32            uiType    = a_pInstance->m_uiFlags & 7;
	ABuildingInstance* pBuilding = TNULL;
	TBOOL              bPainted  = TFALSE;

	if ( uiType == TerrainTypes::InstanceType_Building )
	{
		pBuilding = TSTATICCAST( ABuildingInstance, pModelInstance2 );
		bPainted  = pBuilding->GetBuildingFlags() & ABuildingInstance::Flags_Painted;
	}

	TBOOL bGrass = TFALSE;

	if ( uiType == TerrainTypes::InstanceType_Grass )
		bGrass = TSTATICCAST( AGrassInstance, pModelInstance2 )->IsPainted();

	TTextureManager* pTexManager = TTextureManager::GetSingleton();
	TTexture*        pSphereMap  = pTexManager->FindTexture( "spheremap_windows.tga" );
	pSphereMap->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

	TWorldShaderHAL* pWorldShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );

	for ( TUINT i = 0; i < pMeshes->m_uiNumMeshes; i++ )
	{
		TWorldMeshInstanceData::Source& rSource = pMeshes->m_pMeshes[ i ];
		const TCHAR*                    szName  = rSource.m_szName;
		TBOOL                           bWindow = szName != TNULL && TStringManager::String8Compare( szName, "window", 6 ) == 0;
		TUINT32                         uiFlags = a_pInstance->m_uiFlags;
		TBOOL                           bShared = TFALSE;
		TBOOL                           bLookUp;

		switch ( uiFlags & 7 )
		{
			case TerrainTypes::InstanceType_Building:
				bLookUp = !( uiFlags & 0x10 ) || bWindow;
				break;
			case TerrainTypes::InstanceType_Landmark:
			case TerrainTypes::InstanceType_Grass:
			case 5:
				bLookUp = TFALSE;
				break;
			case 6:
				bLookUp = !( uiFlags & 0x2000 );
				break;
			default:
				bLookUp = TTRUE;
				break;
		}

		TWorldMaterialHAL* pMat = TNULL;

		if ( bLookUp )
		{
			// A material of the cell with the same name and light map
			bShared = TTRUE;

			for ( T2GenericSList::Node* it = m_oUsedList.Begin(); it != m_oUsedList.End(); it = it->Next() )
			{
				ATerrain2::MaterialEntry* pEntry = TSTATICCAST( ATerrain2::MaterialEntry, it );

				if ( !( pEntry->m_uiFlags & 1 ) || T2String8::Compare( pEntry->m_pMaterial->GetMaterialName(), szName ) != 0 )
					continue;

				const TCHAR* szLightmap = TNULL;

				if ( !bWindow && pEntry->m_pMaterial->GetTextureNum() > 1 && pEntry->m_pMaterial->GetTexture( 0 ) != TNULL )
					szLightmap = pEntry->m_pMaterial->GetTexture( 0 )->GetName();

				TBOOL bMatch;

				if ( szLightmap == TNULL )
				{
					bMatch = ( rSource.m_szLightmap == TNULL ) == !( pEntry->m_uiFlags & 2 );
				}
				else
				{
					if ( rSource.m_szLightmap == TNULL )
						continue;

					bMatch = T2String8::Compare( szLightmap, rSource.m_szLightmap ) == 0;
				}

				if ( bMatch )
				{
					pMat = TSTATICCAST( TWorldMaterialHAL, pEntry->m_pMaterial );
					break;
				}
			}
		}

		if ( pMat == TNULL )
		{
			ATerrain2::MaterialEntry* pEntry = ATerrain2::GetInstance()->AllocMaterial();
			m_oUsedList.PushFront( pEntry );

			if ( bShared )
				pEntry->m_uiFlags |= 1;
			else
				pEntry->m_uiFlags &= ~1;

			T2Material* pMetaMat  = TMaterialManager::GetSingleton()->FindMaterial( szName );
			TTexture*   pFirstTex = pMetaMat->GetTexture( 0 );

			TBOOL bLightMapped = rSource.m_szLightmap != TNULL;
			TASSERT( bLightMapped == ( ( a_pInstance->m_uiFlags & 0x10 ) != 0 ) );

			if ( bLightMapped )
				pEntry->m_uiFlags |= 2;
			else
				pEntry->m_uiFlags &= ~2;

			pMat = TSTATICCAST( TWorldMaterialHAL, pEntry->m_pMaterial );
			pMat->SetTextureNum( bLightMapped + 1 );
			pMat->Create( pFirstTex->GetFlags1() & 1 );
			pMat->SetMetaMaterial( pMetaMat );
			pMat->SetMaterialName( szName );

			TINT  iNumMetaTex = pMetaMat->GetNumTextures();
			TBOOL bAlpha      = TFALSE;
			TINT  iPass       = 0;

			for ( ; iPass < iNumMetaTex; iPass++ )
			{
				TASSERT( iPass < TWorldMaterial::TEXTURETYPE_NUMOF );
				pMat->SetTexture( iPass, pMetaMat->GetTexture( iPass ) );

				if ( pMat->GetTexture( iPass )->GetFlags1() & 1 )
					pMat->SetBlendMode( 1 );

				pMat->SetMultiPassBlendType( iPass, pMetaMat->GetPassTevMode( iPass ) );
				pMat->SetUVStream( iPass, 0 );

				if ( pMetaMat->GetPassUVFunctionType( iPass ) == 4 )
					pMat->SetUVMode( iPass, TWorldMaterialHAL::UVMODE_ENVMAP );

				bAlpha = bAlpha || ( pMat->GetTexture( iPass ) != TNULL && ( pMat->GetTexture( iPass )->GetFlags1() & 1 ) );
			}

			if ( bLightMapped )
			{
				TASSERT( iPass < TWorldMaterial::TEXTURETYPE_NUMOF );
				pMat->SetTexture( iPass, pTexManager->FindTexture( rSource.m_szLightmap ) );
				pMat->SetUVStream( iPass, 1 );
				pMat->SetMultiPassBlendType( iPass, TWorldMaterialHAL::MULTIPASSBLENDTYPE_MODULATE );
				iPass++;
			}

			TASSERT( iPass <= TWorldMaterial::TEXTURETYPE_NUMOF );
			pMat->SetTextureNum( iPass );

			// The passes without a texture of their own
			for ( TINT iMetaPass = iNumMetaTex; iMetaPass < pMetaMat->GetNumPasses(); iMetaPass++, iPass++ )
			{
				TASSERT( iPass < TWorldMaterial::TEXTURETYPE_NUMOF );
				pMat->SetTextureNum( iPass + 1 );
				pMat->SetTexture( iPass, pTexManager->FindTexture( pMetaMat->GetPassTextureName( iMetaPass ) ) );
				pMat->SetMultiPassBlendType( iPass, pMetaMat->GetPassTevMode( iMetaPass ) );
				pMat->SetUVStream( iPass, 0 );

				if ( pMetaMat->GetPassUVFunctionType( iMetaPass ) == 4 )
					pMat->SetUVMode( iPass, TWorldMaterialHAL::UVMODE_ENVMAP );

				bAlpha = bAlpha || ( pMat->GetTexture( iPass ) != TNULL && ( pMat->GetTexture( iPass )->GetFlags1() & 1 ) );
			}

			const TCHAR* szMatName = pMat->GetMaterialName();
			TBOOL        bFlag400;

			if ( szMatName != TNULL && ( TStringManager::String8Compare( szMatName, "window", 6 ) == 0 || TStringManager::String8Compare( szMatName, "sticker", 7 ) == 0 ) )
			{
				bFlag400 = TTRUE;
			}
			else
			{
				if ( bPainted && pBuilding->m_uiUnkC6 != 0xFF )
				{
					// The pattern of the painted building as a texture scaled pass
					TINT iNumTextures = pMat->GetTextureNum();
					TASSERT( iNumTextures < TWorldMaterial::TEXTURETYPE_NUMOF );
					pMat->SetTextureNum( iNumTextures + 1 );

					ARegionManager::PatternTexture* pPattern = APlayerManager::GetSingleton()->GetPatternTexture( TINT8( pBuilding->m_uiUnkC6 ) );
					pMat->SetTexture( iNumTextures, pPattern->m_pTex );
					pMat->SetMultiPassBlendType( iNumTextures, TWorldMaterialHAL::MULTIPASSBLENDTYPE( pPattern->m_eBlendType ) );
					pMat->SetUserDataIndex( TWorldMaterialHAL::UData_1, pBuilding->m_iTexScaleIndex );

					TWorldShaderHAL::TexScale* pTexScale = pWorldShader->GetTexScale( pBuilding->m_iTexScaleIndex );
					pTexScale->m_fScaleU                 = pBuilding->m_fUnkBC;
					pTexScale->m_fScaleV                 = pBuilding->m_fUnkC0;
					pMat->SetUVMode( iNumTextures, TWorldMaterialHAL::UVMODE_TEXSCALE );
				}

				bFlag400 = bAlpha;
			}

			pMat->SetFlags( 0x400, bFlag400 );

			if ( bGrass )
			{
				TerrainTypes::GrassData* pGrassData = TSTATICCAST( TerrainTypes::GrassData, a_pInstance->m_pTypeData );
				TASSERT( pGrassData->numtextures > 0 );
				pMat->SetTexture( 0, pTexManager->FindTexture( pGrassData->m_ppTextures[ 0 ] ) );
			}
			else if ( a_pInstance->m_uiFlags & 0x2000000 )
			{
				TTexture* pTexture = pMat->GetTexture( 0 );

				for ( TerrainTypes::TextureOverride* pOverride = TSTATICCAST( TerrainTypes::TextureOverride, a_pInstance->m_pTypeData ); pOverride != TNULL; pOverride = pOverride->m_pNext )
				{
					if ( TStringManager::String8Compare( pTexture->GetName(), pOverride->m_szName ) == 0 )
					{
						pMat->SetTexture( 0, pOverride->m_pTexture );
						break;
					}
				}
			}

			if ( pMetaMat->GetNumPasses() > 0 )
				pMat->SetBlendMode( pMetaMat->GetPassBlendMode( 0 ) );

			pMat->SetOrderTable( pWorldShader->GetOrderTable( pMetaMat->GetOrderTableIndex() ), 0, TFALSE );

			switch ( pMetaMat->GetUnk1C() )
			{
				case 0:
				case 1:
					pMat->SetFlags( TMaterial::FLAGS_BLENDING, TTRUE );
					break;
				case 2:
					pMat->SetFlags( TMaterial::FLAGS_UNK2, TTRUE );
					break;
				case 3:
					pMat->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
					break;
			}

			pMat->SetUVAnimation( AMaterialManager::GetSingleton()->FindMaterialUVAnimation( pMetaMat, TNULL ) );

			if ( szName != TNULL && ( TStringManager::String8Compare( szName, "transp_", 7 ) == 0 || TStringManager::String8FindString( szName, "tube_glass" ) ) )
			{
				pMat->SetTextureNum( 1 );
				pMat->SetBlendMode( 1 );
				pMat->SetMultiPassBlendType( 0, TWorldMaterialHAL::MULTIPASSBLENDTYPE_OVERRIDE );
				pMat->SetOrderTable( pWorldShader->GetOrderTable( 8 ), 0, TFALSE );
				pMat->SetUVMode( 0, TWorldMaterialHAL::UVMODE_ENVMAP );
				pMat->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
			}
			else if ( szName != TNULL && TStringManager::String8FindString( szName, "sticker_arrow" ) )
			{
				pMat->SetTextureNum( 1 );
				pMat->SetBlendMode( 0 );
				pMat->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
				pMat->SetMultiPassBlendType( 0, TWorldMaterialHAL::MULTIPASSBLENDTYPE_OVERRIDE );
				pMat->SetOrderTable( pWorldShader->GetOrderTable( 4 ), 0, TFALSE );
			}
			else
			{
				if ( bLightMapped )
				{
					TASSERT( pMat->GetTextureNum() > 1 );
					TASSERT( pMat->m_iUVStream[ 0 ] == 0 && pMat->m_iUVStream[ 1 ] == 1 );
					AMaterialInfo::SwapLightMapPasses( pMat );
					pMat->SetMultiPassBlendType( 0, TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPCOLOUR2 );
					pMat->SetMultiPassBlendType( 1, TWorldMaterialHAL::MULTIPASSBLENDTYPE_MODULATE );
					TASSERT( pMat->m_iUVStream[ 0 ] == 1 && pMat->m_iUVStream[ 1 ] == 0 );
				}

				if ( bWindow )
				{
					// The window: the sphere map over the texture
					TTexture* pTexture = pMat->GetTexture( bLightMapped ? 1 : 0 );
					pMat->SetTextureNum( 2 );
					pMat->SetUVMode( 0, TWorldMaterialHAL::UVMODE_ENVMAP );
					pMat->SetTexture( 0, pSphereMap );
					pMat->SetMultiPassBlendType( 0, TWorldMaterialHAL::MULTIPASSBLENDTYPE_WINDOWS );
					pMat->SetUVMode( 1, TWorldMaterialHAL::UVMODE_STREAM );
					pMat->SetTexture( 1, pTexture );
					pMat->SetMultiPassBlendType( 1, TWorldMaterialHAL::MULTIPASSBLENDTYPE_WINDOWS2 );
					pMat->SetBlendMode( 12 );
				}
			}
		}

		rSource.m_pMaterial = pMat;
	}

	// The paint challenge (also the current sub challenge of an uber one) projects the pattern onto the building
	AChallenge* pPaintChallenge = TNULL;

	if ( AChallengeManager::GetSingleton() != TNULL )
	{
		AChallenge* pChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();

		if ( pChallenge != TNULL )
		{
			if ( pChallenge->GetType() == AChallenge::Type_Uber )
				pChallenge = TSTATICCAST( AUberChallenge, pChallenge )->GetCurrentSubChallenge();

			if ( pChallenge != TNULL )
			{
				if ( pChallenge->GetType() == AChallenge::Type_Paint )
					pPaintChallenge = pChallenge;
			}
		}
	}

	ABuildingInstance* pProjected = ( uiType == TerrainTypes::InstanceType_Landmark ) ? TSTATICCAST( ABuildingInstance, a_pInstance->m_pModelInstance2 ) : pBuilding;

	if ( pProjected != TNULL && pProjected->m_iGlintIndex >= 8 && pProjected->m_iGlintIndex < 91 )
	{
		if ( ( pPaintChallenge != TNULL && ( a_pInstance->m_szModel == TNULL || TStringManager::String8Compare( "prop_doorway_mainmesh", a_pInstance->m_szModel, -1 ) != 0 ) ) || ( pProjected->GetBuildingFlags() & ABuildingInstance::Flags_BossPlatform ) )
			AGameStatus::SetBuildingProjection( pProjected, pProjected->m_iGlintIndex, TFALSE );
	}
}

// $deBlob: FUNCTION 0061bdd0
void ACellBucket::OnLoaded()
{
	TASSERT( m_pMatInfoPool->IsUsedEmpty() );
	TASSERT( m_pUVAnimPool->IsUsedEmpty() );
	m_pMatInfoPool->SetName( m_pCell->m_szName );
	m_pUVAnimPool->SetName( m_pCell->m_szName );

	T2Material::sm_pTRBLoadCallback     = OnMaterialLoaded;
	ms_pCurrentBucket                   = this;
	T2Material::sm_pTRBLoadCallbackData = this;
	TWorldShader::GetSingleton()->SetMaterialCreatedCallback( OnWorldMaterialCreated );
	TAssetInit::InitAssets( *m_pTRB, TTRUE, TTRUE );
	m_uiFlags |= Flag_AssetsInitialised;
	T2Material::sm_pTRBLoadCallback     = TNULL;
	T2Material::sm_pTRBLoadCallbackData = TNULL;
	ms_pCurrentBucket                   = TNULL;
	TWorldShader::GetSingleton()->SetMaterialCreatedCallback( TNULL );

	if ( m_pCell->m_uiFlags & TerrainTypes::CellFlag_HasModel )
	{
		TerrainTypes::CellData* pData = m_pCell->m_pData;

		TCHAR* szModelName = TStringManager::GetTempString8();
		TStringManager::String8Format( szModelName, "%s.txs", m_pCell->m_szName );
		T2String8::ToLowerCase( szModelName );

		T2ModelPtr pT2Model = TModelManager::GetSingleton()->FindModel( szModelName );
		TASSERT( pT2Model.IsValid() && pT2Model->IsCreated() );

		TModel* pModel = pT2Model.GetData();
		pModel->SetTRB( T2ResourceManager::GetSingleton()->GetResourceData( pModel->GetResourceId() )->GetCreatedInTRB(), TFALSE );

		TModelInstance* pInstance = pModel->CreateInstance();
		pData->m_pModelInstance   = pInstance;

		{
			T2ModelPtr pInstanceModel = pInstance->GetModel();
			AMaterialManager::GetSingleton()->SetupModelUVAnimations( pInstanceModel.GetData(), m_pMatInfoPool );
		}

		if ( pModel->GetCollisionDataCount() < 1 || !( m_pCell->m_uiFlags & TerrainTypes::CellFlag_Collision ) )
		{
			pData->m_pCollision       = TNULL;
			pData->m_pCollisionObject = TNULL;
		}
		else
		{
			TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
			pPhysicsManager->BeginBatchOperation();
			pPhysicsManager->SuspendBatchOperation();
			CreateCollision( *m_pCell->m_pMatrix );
			pPhysicsManager->ResumeBatchOperation();
			pPhysicsManager->EndBatchOperation();
		}

		pInstance->GetTransform().SetMatrix( *m_pCell->m_pMatrix );

		TTMDWin::TTRBWorldHeader* pHeader    = TSTATICCAST( TTMDWin::TTRBWorldHeader, pModel->GetAssetData() );
		TTMDWin::TRBWorldData*    pWorldData = pHeader->m_pWorldData;
		TWorldVis*                pWorldVis  = TNULL;

		if ( pWorldData->m_iNumGroups > 0 && pWorldData->m_ppGroups[ 0 ]->m_iNumCells > 0 )
		{
			pWorldVis = new ( m_heap ) TWorldVis();
			pWorldVis->Create( pWorldData->m_ppGroups[ 0 ], TNULL );
		}

		pData->m_pWorldVis = pWorldVis;

		TASSERT( pModel->GetLODCount() > 0 );
		TModelLOD& rLOD = pModel->GetLOD( 0 );

		for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		{
			TWorldMaterialHAL* pCurMat      = TDYNAMICCAST( TWorldMaterialHAL, rLOD.GetMesh( i )->GetMaterial() );
			const TCHAR*       szName       = pCurMat->GetMaterialName();
			TBOOL              bLightMapped = pCurMat->GetUVStream( 1 ) == 1;

			if ( szName != TNULL && ( TStringManager::String8Compare( szName, "transp_", 7 ) == 0 || TStringManager::String8FindString( szName, "tube_glass" ) ) )
			{
				pCurMat->SetTextureNum( 1 );
				pCurMat->SetBlendMode( 1 );
				pCurMat->SetMultiPassBlendType( 0, TWorldMaterialHAL::MULTIPASSBLENDTYPE_OVERRIDE );
				pCurMat->SetOrderTable( TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() )->GetOrderTable( 8 ), 0, TTRUE );
				pCurMat->SetUVMode( 0, TWorldMaterialHAL::UVMODE_ENVMAP );
				pCurMat->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
				continue;
			}

			if ( szName != TNULL && TStringManager::String8FindString( szName, "sticker_arrow" ) )
			{
				pCurMat->SetTextureNum( 1 );
				pCurMat->SetBlendMode( 0 );
				pCurMat->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
				pCurMat->SetMultiPassBlendType( 0, TWorldMaterialHAL::MULTIPASSBLENDTYPE_OVERRIDE );
				pCurMat->SetOrderTable( TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() )->GetOrderTable( 4 ), 0, TTRUE );
				continue;
			}

			if ( bLightMapped )
			{
				TASSERT( pCurMat->GetTextureNum() > 1 );
				TASSERT( pCurMat->m_iUVStream[ 0 ] == 0 && pCurMat->m_iUVStream[ 1 ] == 1 );
				AMaterialInfo::SwapLightMapPasses( pCurMat );
				pCurMat->SetMultiPassBlendType( 0, TWorldMaterialHAL::MULTIPASSBLENDTYPE_LERPCOLOUR2 );
				pCurMat->SetMultiPassBlendType( 1, TWorldMaterialHAL::MULTIPASSBLENDTYPE_MODULATE );
				TASSERT( pCurMat->m_iUVStream[ 0 ] == 1 && pCurMat->m_iUVStream[ 1 ] == 0 );
			}
		}
	}

	m_pCell->m_uiFlags &= ~TerrainTypes::CellRuntimeFlag_Loading;
	m_pCell->m_uiFlags |= TerrainTypes::CellRuntimeFlag_Loaded;
}

// $deBlob: FUNCTION 0061d5d0
void ACellBucket::CreateCollision( const TMatrix44& a_rMatrix )
{
	TerrainTypes::CellData* pData     = m_pCell->m_pData;
	TModelInstance*         pInstance = pData->m_pModelInstance;

	TASSERT( pInstance->GetModel().GetData()->GetCollisionDataCount() > 0 );

	ACollisionInstance2* pCollision = new ( m_heap ) ACollisionInstance2();
	pCollision->SetModelInstance( pInstance, TNULL );

	ACollisionInstance2::OwnerInfo oOwnerInfo;
	oOwnerInfo.m_eType  = 4;
	oOwnerInfo.m_pOwner = pData;
	pCollision->Create( pInstance, &oOwnerInfo, TFALSE, 0, TNULL );
	pCollision->SetMatrix( a_rMatrix );

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
	pPhysicsManager->BeginBatchOperation();
	pCollision->AddToWorld( ACollisionInstance2::PhysicsObjectType_Standard | ACollisionInstance2::PhysicsObjectType_Custom );
	pPhysicsManager->EndBatchOperation();

	pData->m_pCollision       = pCollision;
	pData->m_pCollisionObject = new ( m_heap ) ACollisionObject( TNULL, 0, 0xff );
}

// $deBlob: FUNCTION 0061c3a0
void ACellBucket::Unload()
{
	while ( !m_oUsedList.IsEmpty() )
		ATerrain2::GetInstance()->ReleaseMaterial( TSTATICCAST( ATerrain2::MaterialEntry, m_oUsedList.PopFront() ) );

	m_pCell->m_uiFlags &= ~TerrainTypes::CellRuntimeFlag_Loaded;
	m_pCell->m_uiFlags &= ~TerrainTypes::CellRuntimeFlag_Loading;
	m_pCell->m_uiFlags &= ~TerrainTypes::CellRuntimeFlag_CancelLoad;

	if ( m_pCell->m_uiFlags & TerrainTypes::CellFlag_HasModel )
	{
		TerrainTypes::CellData* pData = m_pCell->m_pData;

		ACollisionObject* pCollisionObject = TSTATICCAST( ACollisionObject, pData->m_pCollisionObject );

		if ( pCollisionObject )
		{
			if ( AGndMotorManager::GetSingleton() != TNULL )
				AGndMotorManager::GetSingleton()->OnCollisionObjectDestroyed( pCollisionObject );
			delete pCollisionObject;
			pData->m_pCollisionObject = TNULL;
		}

		if ( pData->m_pCollision )
		{
			TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
			pPhysicsManager->BeginBatchOperation();
			pPhysicsManager->SuspendBatchOperation();
			TSTATICCAST( ACollisionInstance2, pData->m_pCollision )->RemoveFromWorld( ACollisionInstance2::PhysicsObjectType_Standard | ACollisionInstance2::PhysicsObjectType_Custom );
			pPhysicsManager->ResumeBatchOperation();
			pPhysicsManager->EndBatchOperation();

			ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, pData->m_pCollision );

			if ( pCollision )
				delete pCollision;

			pData->m_pCollision = TNULL;
		}

		if ( pData->m_pModelInstance )
		{
			delete pData->m_pModelInstance;
			pData->m_pModelInstance = TNULL;
		}

		if ( pData->m_pWorldVis )
		{
			delete pData->m_pWorldVis;
			pData->m_pWorldVis = TNULL;
		}
	}

	if ( m_pTRB )
	{
		if ( m_uiFlags & Flag_AssetsInitialised )
		{
			T2Material::sm_pTRBUnloadCallback     = OnMaterialUnloaded;
			ms_pCurrentBucket                     = this;
			T2Material::sm_pTRBUnloadCallbackData = this;
			TWorldShader::GetSingleton()->SetMaterialDestroyedCallback( OnWorldMaterialDestroyed );
			TAssetInit::DeinitAssets( *m_pTRB );
			T2Material::sm_pTRBUnloadCallback     = TNULL;
			T2Material::sm_pTRBUnloadCallbackData = TNULL;
			ms_pCurrentBucket                     = TNULL;
			TWorldShader::GetSingleton()->SetMaterialDestroyedCallback( TNULL );

			TASSERT( m_pMatInfoPool->IsUsedEmpty() );
			TASSERT( m_pUVAnimPool->IsUsedEmpty() );

			m_uiFlags &= ~Flag_AssetsInitialised;
		}

		m_pTRB->Close();
		delete m_pTRB;
		m_pTRB = TNULL;
	}

	Reset();
}

// $deBlob: FUNCTION 0061d760
void __stdcall ACellBucket::OnMaterialLoaded( T2Material* a_pMaterial, void* a_pBucket )
{
	ACellBucket* pBucket = TSTATICCAST( ACellBucket, a_pBucket );
	AMaterialManager::GetSingleton()->CreateMaterialUVAnimation( a_pMaterial, pBucket->m_pMatInfoPool, pBucket->m_pUVAnimPool );
}

// $deBlob: FUNCTION 0061d780
void __stdcall ACellBucket::OnMaterialUnloaded( T2Material* a_pMaterial, void* a_pBucket )
{
	ACellBucket* pBucket = TSTATICCAST( ACellBucket, a_pBucket );
	AMaterialManager::GetSingleton()->DestroyMaterialUVAnimation( a_pMaterial, pBucket->m_pMatInfoPool );
}

// $deBlob: FUNCTION 0061d7a0
void __stdcall ACellBucket::OnWorldMaterialCreated( TWorldMaterial* a_pMaterial, void* a_pT2Material )
{
	TASSERT( a_pMaterial->GetMetaMaterial() == a_pT2Material );
	a_pMaterial->SetUVAnimation( AMaterialManager::GetSingleton()->FindMaterialUVAnimation( TSTATICCAST( T2Material, a_pT2Material ), ms_pCurrentBucket->m_pMatInfoPool ) );
}

// $deBlob: FUNCTION 0061d800
void __stdcall ACellBucket::OnWorldMaterialDestroyed( TWorldMaterial* a_pMaterial )
{
	a_pMaterial->SetUVAnimation( TNULL );
}
