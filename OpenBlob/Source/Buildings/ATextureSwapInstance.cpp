#include "pch.h"
#include "Sound/ASoundTextureSwapMap.h"
#include "ATextureSwapInstance.h"
#include "ABuildingManager.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Shaders/ASkinShader/ASkinShader.h"

#include <Render/TModel.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <Toshi/TStringManager.h>

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00580af0
void ATextureSwapInstance::OnCreated( TerrainTypes::CellInstance* a_pInstance )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_TextureSwap );

	TModelInstance* pLowLOD = a_pInstance->m_pLowLODInstance;

	for ( TerrainTypes::TextureOverride* pOverride = TSTATICCAST( TerrainTypes::TextureOverride, a_pInstance->m_pTypeData ); pOverride != TNULL; pOverride = pOverride->m_pNext )
	{
		// The new texture (its name is in m_pTexture until it is loaded here)
		TTexture* pTexture    = TTextureManager::GetSingleton()->FindTexture( TREINTERPRETCAST( const TCHAR*, pOverride->m_pTexture ) );
		pOverride->m_pTexture = pTexture;

		if ( pLowLOD == TNULL )
			continue;

		// The first mesh whose texture is the one to replace gets a copy of its material with the new texture
		T2ModelPtr pModel = pLowLOD->GetModel();
		TModelLOD& rLOD   = pModel->GetLOD( 0 );
		pModel.Release();

		for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		{
			TMaterial* pSource = rLOD.GetMeshes()[ i ]->GetMaterial();

			if ( TStringManager::String8Compare( pSource->GetTexture( 0 )->GetName(), pOverride->m_szName, -1 ) != 0 )
				continue;

			ASkinMaterial* pMaterial = TSTATICCAST( ASkinMaterial, ASkinShader::GetSingleton()->CreateMaterial( TNULL ) );
			pMaterial->Create( 0 );
			pMaterial->SetTextureNum( pSource->GetTextureNum() );

			for ( TUINT32 j = 0; j < pSource->GetTextureNum(); j++ )
				pMaterial->SetTexture( j, pSource->GetTexture( j ) );

			pMaterial->SetName( pSource->GetName() );
			pMaterial->SetTexture( 0, pTexture );
			pOverride->m_pMaterial = pMaterial;
			pOverride->m_pSource   = pSource;
			break;
		}
	}
}

// $deBlob: FUNCTION 00580d70
void ATextureSwapInstance::OnBlobTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const TVector4* a_pPosition )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_TextureSwap );

	// NOTE: retail tests the ABaseEntity flags (blob+0x70, FLAG_Player set by the blob's ctor), not the BFLAG word
	if ( ( a_pInstance->m_uiFlags & 0x400 ) && ( a_pBlob->GetFlags() & ABaseEntity::FLAG_Player ) )
	{
		TUINT32 uiColour = a_pBlob->GetPaintColour();

		if ( ( uiColour & 0x10 ) == 0 && uiColour != 0 && uiColour != 0x10 )
			ABuildingManager::TriggerTransformation( a_pInstance, a_pBlob );
	}
}

// $deBlob: FUNCTION 00580e20
void ATextureSwapInstance::OnPaint( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_TextureSwap );


	if ( a_pInstance->m_uiFlags & 0x2000000 )
		return;

	a_pInstance->m_uiFlags |= 0x2000000;

	TINT iPlayer = TINT( APlayerManager::GetSingleton()->GetPlayerIndex( a_pBlob ) );
	AGameStatus::GetSingleton()->AddBonusScore( iPlayer, TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0xA4 / 4 ], *a_pInstance->m_ppCentre );

	ABuildingManager::TriggerLinks( a_pInstance, AEntityIOPacket::MESSAGE_TRIGGER );

	if ( ( a_pInstance->m_uiFlags & 0x1000000 ) && ( a_pInstance->m_uiFlags & 0x40 ) )
	{
		// the meshes of the instance (InstanceMeshes): a mesh whose texture is an override's takes a copy of its material
		// (a light mapped mesh, flag 0x40 symbol, uses the second stage)
		TerrainTypes::InstanceMeshes* pMeshes = a_pInstance->m_pSymbolData;

		for ( TUINT32 uiMesh = 0; uiMesh < pMeshes->m_uiNumMeshes; uiMesh++ )
		{
			Toshi::TWorldMeshInstanceData::Source* pSource = &pMeshes->m_pMeshes[ uiMesh ];

			TBOOL              bLightMap = pSource->m_szLightmap != TNULL;
			TWorldMaterialHAL* pMaterial = pSource->m_pMaterial;
			TUINT32            uiStage   = bLightMap ? 1 : 0;
			Toshi::TTexture*   pTexture  = pMaterial->GetTexture( uiStage );

			for ( TerrainTypes::TextureOverride* pOverride = TSTATICCAST( TerrainTypes::TextureOverride, a_pInstance->m_pTypeData ); pOverride != TNULL; pOverride = pOverride->m_pNext )
			{
				if ( Toshi::TStringManager::String8Compare( pTexture->GetName(), pOverride->m_szName, 0xFFFFFFFF ) != 0 )
					continue;

				Toshi::TTexture* pNewTexture = pOverride->m_pTexture;
				ASoundTextureSwapMap::GetSingleton()->PlayTextureSounds( pNewTexture, a_pBlob->GetBodyPosition() );

				Clone* pClone = AShaderEffects::GetSingleton()->AddClone( pMaterial, pNewTexture, TINT16( uiStage ), bLightMap, UnlinkClone, pOverride );
				if ( pClone == TNULL )
				{
					pMaterial->SetTexture( uiStage, pNewTexture );
				}
				else
				{
					// the copy is the new head of the override's copies
					if ( pOverride->m_pClones != TNULL )
						pClone->m_pNextCopy = TREINTERPRETCAST( Clone*, pOverride->m_pClones );

					pOverride->m_pClones = pClone;
				}
				break;
			}
		}
	}
}

// $deBlob: FUNCTION 00580fe0
void ATextureSwapInstance::ClearClones( TerrainTypes::CellInstance* a_pInstance )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_TextureSwap );

	for ( TerrainTypes::TextureOverride* pOverride = TSTATICCAST( TerrainTypes::TextureOverride, a_pInstance->m_pTypeData ); pOverride != TNULL; pOverride = pOverride->m_pNext )
	{
		Clone* pClone = TREINTERPRETCAST( Clone*, pOverride->m_pClones );

		while ( pClone != TNULL )
		{
			Clone* pCurrent = pClone;
			pClone          = pClone->m_pNextCopy;
			AShaderEffects::GetSingleton()->ReleaseClone( pCurrent, 0 );
		}

		pOverride->m_pClones = TNULL;
	}
}

// $deBlob: FUNCTION 00581060
void __stdcall ATextureSwapInstance::UnlinkClone( Clone* a_pClone, void* a_pOverride )
{
	if ( a_pOverride == TNULL )
		return;

	TerrainTypes::TextureOverride* pOverride = TREINTERPRETCAST( TerrainTypes::TextureOverride*, a_pOverride );

	Clone* pCurrent = TREINTERPRETCAST( Clone*, pOverride->m_pClones );
	Clone* pPrev    = TNULL;

	if ( pCurrent == TNULL )
		return;

	while ( pCurrent != a_pClone )
	{
		pPrev    = pCurrent;
		pCurrent = pCurrent->m_pNextCopy;

		if ( pCurrent == TNULL )
			return;
	}

	Clone* pNext = pCurrent->m_pNextCopy;
	if ( pPrev != TNULL )
		pPrev->m_pNextCopy = pNext;
	else
		pOverride->m_pClones = pNext;

	pCurrent->m_pNextCopy = TNULL;
}

// $deBlob: FUNCTION 00580e00
void ATextureSwapInstance::OnRemovedFromWorld( TerrainTypes::CellInstance* a_pInstance )
{
	// NOTE: retail's entry is a jump to ClearClones
	ClearClones( a_pInstance );
}

// $deBlob: FUNCTION 00580e10
void ATextureSwapInstance::OnReset( TerrainTypes::CellInstance* a_pInstance )
{
	// NOTE: retail's entry is a jump to ClearClones
	ClearClones( a_pInstance );
}
