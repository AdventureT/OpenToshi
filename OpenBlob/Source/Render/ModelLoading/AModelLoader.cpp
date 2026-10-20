#include "pch.h"
#include "AModelLoader.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include "Platform/DX11/ASkinShader_DX11.h"

#include <Render/TAssetInit.h>
#include <Toshi/TStringManager.h>
#include <Platform/DX11/TMaterial_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Render/Material/AMaterialManager.h"

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Dynamic initializer (eh vector constructor + atexit) and its atexit destructor
// $deBlob: FUNCTION 0053cb60
// $deBlob: FUNCTION 00858640
AModelLoader::MaterialNode AModelLoader::ms_aNodes[ MAX_MATERIALS ];
// $deBlob: FUNCTION 0053cb20
// $deBlob: FUNCTION 00858620
Toshi::T2DList<AModelLoader::MaterialNode> AModelLoader::ms_oFreeMaterials;
// $deBlob: FUNCTION 0053cb40
// $deBlob: FUNCTION 00858630
Toshi::T2DList<AModelLoader::MaterialNode> AModelLoader::ms_oMaterialList;
const Toshi::TClass*                       AModelLoader::ms_pSkinMaterialClass;

// $deBlob: FUNCTION 005f8ed0
AModelLoader::AModelLoader()
{
	TModel::SetTRBLoadCallback( AModelLoaderLoadTRBCallback );
	TModelHAL::SetTRBLoadCallback2( AModelLoaderLoadTRBCallback2 );

	for ( TINT i = 0; i < MAX_MATERIALS; i++ )
		ms_oFreeMaterials.PushBack( &ms_aNodes[ i ] );
}

// $deBlob: FUNCTION 005f8f90
// $deBlob: FUNCTION 0056c130
AModelLoader::~AModelLoader()
{
	ms_oFreeMaterials.Clear();
}

// $deBlob: FUNCTION 005f9020
void AModelLoader::GetMaterial( TShader* a_pShader, const TCHAR* a_szName, TMaterial*& a_rOutMaterial )
{
	TASSERT( TAssetInit::GetInitialisingTRB() );

	a_rOutMaterial = TNULL;

	const TCHAR* szShaderPrefix = a_pShader->GetShaderPrefix();

	if ( TStringManager::String8Length( szShaderPrefix ) == 0 )
		return;

	TString8 strFullMatName( szShaderPrefix );
	strFullMatName += a_szName;

	ASkinMaterialHAL* pCreatedMaterial = TSTATICCAST( ASkinMaterialHAL, GetCreatedMaterial( strFullMatName, TAssetInit::GetInitialisingTRB() ) );

	if ( pCreatedMaterial == TNULL )
	{
		TCHAR szMetaMatName[ 128 ];
		TStringManager::String8CopySafe( szMetaMatName, a_szName, sizeof( szMetaMatName ) );
		TStringManager::String8ToLowerCase( szMetaMatName );

		T2Material* pMetaMat = TMaterialManager::GetSingleton()->FindMaterial( szMetaMatName );
		TASSERT( pMetaMat != TNULL, "Failed to find mat" );

		TTexture* pTexture  = pMetaMat->GetTexture( 0 );
		TBOOL     bHasAlpha = pTexture->IsMultiBitAlpha();

		if ( a_pShader->IsA( &TGetClass( ASkinShader ) ) )
		{
			pCreatedMaterial = TSTATICCAST( ASkinMaterialHAL, TSTATICCAST( ASkinShader, a_pShader )->CreateMaterial( TNULL ) );
			pCreatedMaterial->Create( bHasAlpha ? 1 : 0 );
			pCreatedMaterial->SetMetaMaterial( pMetaMat );
			pCreatedMaterial->SetTexture( 0, pTexture );
			pCreatedMaterial->SetTextureNum( 1 );
		}

		TASSERT( !ms_oFreeMaterials.IsEmpty() );
		MaterialNode* pMatNode = ms_oFreeMaterials.PopFront();
		pMatNode->m_pMaterial       = pCreatedMaterial;
		pMatNode->m_iReferenceCount = 1;
		TStringManager::String8Copy( pMatNode->m_szName, strFullMatName.GetString(), sizeof( pMatNode->m_szName ) );

		if ( strFullMatName.Length() > Toshi::TMaterial::NAMESIZELIMIT - 1 )
			strFullMatName[ Toshi::TMaterial::NAMESIZELIMIT ] = '\0';

		pCreatedMaterial->SetName( strFullMatName.GetString() );
		pMatNode->m_pOwnerTRB = TAssetInit::GetInitialisingTRB();
		ms_oMaterialList.PushFront( pMatNode );

		if ( pMetaMat != TNULL )
		{
			pMatNode->m_pMaterial->SetUVAnimation( AMaterialManager::GetSingleton()->FindMaterialUVAnimation( pMetaMat, TNULL ) );
		}

		TUINT32 uiMetaFlags = pMetaMat->GetFlags();

		if ( pCreatedMaterial->IsA( &TGetClass( ASkinMaterialHAL ) ) && ( uiMetaFlags & 0x10000 ) )
		{
			switch ( pMetaMat->GetUnk1C() )
			{
				case 0:
				case 1:
					pCreatedMaterial->SetFlags( TMaterial::FLAGS_BLENDING, TTRUE );
					break;
				case 2:
					pCreatedMaterial->SetFlags( TMaterial::FLAGS_UNK2, TTRUE );
					break;
				case 3:
					pCreatedMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
					break;
			}

			pCreatedMaterial->SetBlendMode( pMetaMat->GetPassBlendMode( 0 ), 0 );

			ASkinShaderHAL* pSkinShader = TSTATICCAST( ASkinShaderHAL, ASkinShader::GetSingleton() );
			TOrderTable*    pOrderTable = pSkinShader->GetOrderTableByIndex( pMetaMat->GetOrderTableIndex() );

			if ( pOrderTable != TNULL )
				pCreatedMaterial->SetOrderTable( pOrderTable, TFALSE );

			for ( TINT i = 0; i < pMetaMat->GetNumPasses(); i++ )
			{
				pMetaMat->GetPassInfo( i );
				TASSERT( i < ASkinMaterialHAL::MAX_PASSES );

				pCreatedMaterial->SetTextureNum( i + 1 );
				pCreatedMaterial->SetTexture( i, TTextureManager::GetSingleton()->FindTexture( pMetaMat->GetPassTextureName( i ) ) );
				pCreatedMaterial->SetTevMode( i, pMetaMat->GetPassTevMode( i ) );

				if ( pMetaMat->GetPassUVFunctionType( i ) == 4 )
					pCreatedMaterial->SetPassMode( i, ASkinMaterialHAL::PASSMODE_ENVMAP );
			}
		}
	}

	a_rOutMaterial = pCreatedMaterial;
}

// $deBlob: FUNCTION 005f9480
void AModelLoader::DestroyMaterial( TMaterial* a_pMaterial )
{
	for ( auto it = ms_oMaterialList.Begin(); it != ms_oMaterialList.End(); it++ )
	{
		if ( it->m_pMaterial == a_pMaterial )
		{
			if ( --it->m_iReferenceCount < 1 )
			{
				a_pMaterial->SetUVAnimation( TNULL );
				it->Remove();

				TMaterial* pMaterial = it->m_pMaterial;
				pMaterial->Destroy();
				it->m_pMaterial = TNULL;

				ms_oFreeMaterials.PushFront( it );
			}

			return;
		}
	}
}

// $deBlob: FUNCTION 005f9550
TMaterial* AModelLoader::GetCreatedMaterial( const TString8& a_rName, TTRB* a_pTRB )
{
	for ( auto it = ms_oMaterialList.Begin(); it != ms_oMaterialList.End(); it++ )
	{
		if ( a_rName.Compare( it->m_szName, -1 ) == 0 && it->m_pOwnerTRB == a_pTRB )
		{
			it->m_iReferenceCount++;
			return it->m_pMaterial;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 005f9670
TBOOL __stdcall AModelLoader::AModelLoaderLoadTRBCallback( TTRB* a_pTRB, TModel* a_pModel )
{
	// NOTE: TModel::LoadTRB asserts "Failed To Load Model TRB" on this (the models are made by TAssetInit)
	return TFALSE;
}

// $deBlob: FUNCTION 005f9680
TBOOL __stdcall AModelLoader::AModelLoaderLoadTRBCallback2( TModelHAL* a_pModel, TTMDWin::TTRBWinHeader* a_pHeader )
{
	return GetSingleton()->LoadTRBCallback2( a_pModel, a_pHeader );
}

// $deBlob: FUNCTION 005f9f90
TBOOL AModelLoader::LoadTRBCallback2( TModel* a_pModel, TTMDWin::TTRBWinHeader* a_pHeader )
{
	a_pModel->m_iLODCount = a_pHeader->m_uiLODCount;
	a_pModel->m_fUnknown  = a_pHeader->m_fUnknown;

	for ( TUINT i = 0; i < a_pHeader->m_uiLODCount; i++ )
	{
		TModelLOD*             pLOD    = &a_pModel->m_LODs[ i ];
		TTMDWin::TRBLODHeader* pTRBLOD = a_pHeader->m_ppLODs[ i ];
		TTMDWin::TRBLODInfo*   pInfo   = pTRBLOD->m_pInfo;

		pLOD->m_iMeshCount   = pInfo->m_iMeshCount1 + pInfo->m_iMeshCount2;
		pLOD->m_pMeshes      = TSTATICCAST( TMesh*, TMalloc( pLOD->m_iMeshCount * sizeof( TMesh* ) ) );
		pLOD->m_RenderVolume = pInfo->m_RenderVolume;

		if ( pInfo->m_eShader != TTMDBase::SHADERTYPE_SKIN )
			return TFALSE;

		LoadSkinMeshTRB( a_pModel, i, pLOD, pTRBLOD );
	}

	return TTRUE;
}
