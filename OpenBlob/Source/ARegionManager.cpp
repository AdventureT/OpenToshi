#include "pch.h"
#include "ARegionManager.h"
#include "AAssetStreaming.h"
#include "ACurrentLevelInformation.h"
#include "GameController/AGameStatus.h"
#include "Sound/ASoundLoadHelper.h"
#include "Sound/ASoundInterface.h"
#include <Render/TAssetInit.h>
#include <Sound/TSound.h>
#include <Toshi/TStringManager.h>
#include <Shaders/WorldShader/TWorldShader.h>
#include <Platform/DX11/TMaterial_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Render/Material/AMaterialManager.h"

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ARegionManager* ARegionManager::sm_instance;

// $deBlob: FUNCTION 00575450
void ARegionManager::RegionState::AddAsset( TTRB* a_pTRB )
{
	TASSERT( m_iAssetCount < MaxAssets );
	m_apAssets[ m_iAssetCount ] = a_pTRB;
	m_iAssetCount++;
	m_iQueuedAssetCount++;
}

// $deBlob: FUNCTION 005754b0
ARegionManager::ARegionManager( RegionRuntimeData* a_pRuntimeData )
{
	m_pRegionStates = TSTATICCAST( RegionState, TMemalign( 16, a_pRuntimeData->m_iNumRegions * sizeof( RegionState ) ) );
	m_pRuntimeData  = a_pRuntimeData;

	TASSERT( !sm_instance );
	sm_instance = this;

	TUtil::MemClear( m_pRegionStates, a_pRuntimeData->m_iNumRegions * sizeof( RegionState ) );
}

// $deBlob: FUNCTION 00575530
// $deBlob: FUNCTION 005c3c60
ARegionManager::~ARegionManager()
{
	TASSERT( sm_instance == this );
	sm_instance = TNULL;
	TFree( m_pRegionStates );
}

// $deBlob: FUNCTION 00575580
void ARegionManager::LoadRegion( TINT a_iRegion )
{
	if ( a_iRegion >= 0 && a_iRegion < m_pRuntimeData->m_iNumRegions )
		LoadRegion( m_pRuntimeData->m_pRegions[ a_iRegion ].m_szName );
}

// $deBlob: FUNCTION 005755b0
void ARegionManager::LoadRegion( const TCHAR* a_szRegion )
{
	RegionState* pRegionState = GetRegionState( a_szRegion );

	if ( pRegionState != TNULL && pRegionState->m_eState == STATE_None )
	{
		TASSERT( !pRegionState->m_iAssetCount, "Region already has loaded assets" );

		Region* pRegion = GetRegion( a_szRegion );

		if ( pRegion != TNULL )
		{
			TBOOL bRegionAssets    = QueueRegionAssets( pRegion, pRegionState );
			TBOOL bFXAssets        = QueueFXAssets( pRegion, pRegionState );
			TBOOL bCharacterAssets = QueueCharacterAssets( pRegion, pRegionState );

			if ( bRegionAssets || bFXAssets || bCharacterAssets )
			{
				pRegionState->m_eState = STATE_Loading;
				return;
			}
		}

		pRegionState->m_eState = STATE_Loaded;
	}
}

// $deBlob: FUNCTION 00575660
void ARegionManager::LoadRegionSounds( TINT a_iRegion )
{
	if ( a_iRegion >= 0 && a_iRegion < m_pRuntimeData->m_iNumRegions )
	{
		Region* pRegion = GetRegion( m_pRuntimeData->m_pRegions[ a_iRegion ].m_szName );

		if ( pRegion != TNULL )
			LoadSoundGroups( pRegion );
	}
}

// $deBlob: FUNCTION 005756c0
void ARegionManager::LoadRegionSounds( const TCHAR* a_szRegion )
{
	Region* pRegion = GetRegion( a_szRegion );

	if ( pRegion != TNULL )
		LoadSoundGroups( pRegion );
}

void ARegionManager::DeinitAsset( TTRB* a_pTRB )
{
	T2Material::sm_pTRBUnloadCallback = OnMaterialUnloaded;
	TWorldShader::GetSingleton()->SetMaterialDestroyedCallback( OnWorldMaterialDestroyed );
	TAssetInit::DeinitAssets( *a_pTRB );
	T2Material::sm_pTRBUnloadCallback     = TNULL;
	T2Material::sm_pTRBUnloadCallbackData = TNULL;
	TWorldShader::GetSingleton()->SetMaterialDestroyedCallback( TNULL );

	if ( a_pTRB != TNULL )
	{
		a_pTRB->Close();
		delete a_pTRB;
	}
}

// $deBlob: FUNCTION 00575700
void ARegionManager::UnloadRegion( const TCHAR* a_szRegion )
{
	RegionState* pRegionState = GetRegionState( a_szRegion );

	if ( pRegionState != TNULL && pRegionState->m_eState == STATE_Loaded )
	{
		for ( TINT i = 0; i < pRegionState->m_iAssetCount; i++ )
		{
			DeinitAsset( pRegionState->m_apAssets[ i ] );
			pRegionState->m_apAssets[ i ] = TNULL;
		}

		pRegionState->m_iAssetCount = 0;
		pRegionState->m_eState      = STATE_None;
	}

	if ( TSound::GetSingleton()->IsInitialised() )
		ASoundInterface::UnloadEventSystem();
}

// $deBlob: FUNCTION 005757e0
void ARegionManager::UnloadAllRegions()
{
	for ( TINT i = 0; i < m_pRuntimeData->m_iNumRegions; i++ )
	{
		// T2Material's unload callback data is this
		RegionState* pRegionState = GetRegionState( m_pRuntimeData->m_pRegions[ i ].m_szName );

		if ( pRegionState != TNULL && pRegionState->m_eState == STATE_Loaded )
		{
			for ( TINT j = 0; j < pRegionState->m_iAssetCount; j++ )
			{
				T2Material::sm_pTRBUnloadCallbackData = this;
				DeinitAsset( pRegionState->m_apAssets[ j ] );
				pRegionState->m_apAssets[ j ] = TNULL;
			}

			pRegionState->m_iAssetCount = 0;
			pRegionState->m_eState      = STATE_None;
		}

		if ( TSound::GetSingleton()->IsInitialised() )
			ASoundInterface::UnloadEventSystem();
	}
}

// $deBlob: FUNCTION 00575930
TBOOL ARegionManager::IsLoading()
{
	for ( TINT i = 0; i < m_pRuntimeData->m_iNumRegions; i++ )
	{
		if ( m_pRegionStates[ i ].m_eState == STATE_Loading )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00575960
TBOOL ARegionManager::IsLoadingSounds()
{
	if ( ASoundLoadHelper::GetSingleton() != TNULL )
		return ASoundLoadHelper::GetSingleton()->GetState() != 5;

	return TFALSE;
}

// $deBlob: FUNCTION 00575980
void ARegionManager::UpdateLoading()
{
	ASoundLoadHelper* pSoundLoadHelper = ASoundLoadHelper::GetSingleton();

	if ( pSoundLoadHelper != TNULL )
		pSoundLoadHelper->Update();
}

// $deBlob: FUNCTION 00575990
void ARegionManager::UpdateSoundLoading()
{
	ASoundLoadHelper* pSoundLoadHelper = ASoundLoadHelper::GetSingleton();

	if ( pSoundLoadHelper != TNULL )
		pSoundLoadHelper->Update();
}

// $deBlob: FUNCTION 005759a0
ARegionManager::RegionState* ARegionManager::GetRegionState( const TCHAR* a_szRegion )
{
	for ( TINT i = 0; i < m_pRuntimeData->m_iNumRegions; i++ )
	{
		if ( TStringManager::String8Compare( m_pRuntimeData->m_pRegions[ i ].m_szName, a_szRegion ) == 0 )
			return &m_pRegionStates[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 005759f0
ARegionManager::Region* ARegionManager::GetRegion( const TCHAR* a_szRegion )
{
	for ( TINT i = 0; i < m_pRuntimeData->m_iNumRegions; i++ )
	{
		if ( TStringManager::String8Compare( m_pRuntimeData->m_pRegions[ i ].m_szName, a_szRegion ) == 0 )
			return &m_pRuntimeData->m_pRegions[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 00575a50
TBOOL ARegionManager::QueueRegionAssets( Region* a_pRegion, RegionState* a_pRegionState )
{
	TCHAR*                    szFileName        = TStringManager::GetTempString8();
	ACurrentLevelInformation* pLevelInformation = ACurrentLevelInformation::GetSingleton();

	for ( TINT i = 0; i < a_pRegion->m_iNumAssets; i++ )
	{
		TStringManager::String8Format( szFileName, 1024, "data/%s/%s.trb", pLevelInformation->GetLevelProperties()->m_directory, a_pRegion->m_ppAssets[ i ] );

		ATRBLoaderJob* pJob         = CreateTRBJob( szFileName );
		pJob->m_fnOnFinished        = OnAssetLoaded;
		pJob->m_pOnFinishedUserData = a_pRegionState;
		a_pRegionState->AddAsset( pJob->m_pTRB );
	}

	return a_pRegion->m_iNumAssets > 0;
}

// $deBlob: FUNCTION 00575bb0
TBOOL ARegionManager::QueueCharacterAssets( Region* a_pRegion, RegionState* a_pRegionState )
{
	TASSERT( AGameStatus::IsSingletonCreated() );

	const TCHAR* szFileName = "data/BlobChar_MP/AssetPack.trb";

	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
		szFileName = "data/BlobChar/AssetPack.trb";

	ATRBLoaderJob* pJob         = CreateTRBJob( szFileName );
	pJob->m_fnOnFinished        = OnAssetLoaded;
	pJob->m_pOnFinishedUserData = a_pRegionState;
	a_pRegionState->AddAsset( pJob->m_pTRB );

	return TFALSE;
}

// $deBlob: FUNCTION 00575cf0
TBOOL ARegionManager::QueueFXAssets( Region* a_pRegion, RegionState* a_pRegionState )
{
	static const TCHAR* s_szBlobFXAssetPack     = "data/Blob_FX/AssetPack.trb";
	static const TCHAR* s_szComposedFXAssetPack = "data/ComposedFX/AssetPack.trb";

	ATRBLoaderJob* pJob         = CreateTRBJob( s_szBlobFXAssetPack );
	pJob->m_fnOnFinished        = OnAssetLoaded;
	pJob->m_pOnFinishedUserData = a_pRegionState;
	a_pRegionState->AddAsset( pJob->m_pTRB );

	pJob                        = CreateTRBJob( s_szComposedFXAssetPack );
	pJob->m_fnOnFinished        = OnAssetLoaded;
	pJob->m_pOnFinishedUserData = a_pRegionState;
	a_pRegionState->AddAsset( pJob->m_pTRB );

	return TFALSE;
}

// $deBlob: FUNCTION 00575ec0
void ARegionManager::LoadSoundGroups( Region* a_pRegion )
{
	if ( ASoundLoadHelper::GetSingleton() != TNULL )
	{
		ASoundLoadHelper::GetSingleton()->LoadSounds( a_pRegion->m_iNumSoundGroups, a_pRegion->m_ppSoundGroups );
		ASoundLoadHelper::GetSingleton()->LoadInGameSounds();
	}
}

// $deBlob: FUNCTION 00575ef0
ATRBLoaderJob* ARegionManager::CreateTRBJob( const TCHAR* a_szFileName )
{
	TTRB*          pTRB = new TTRB();
	ATRBLoaderJob* pJob = AAssetStreaming::GetSingleton()->GetFreeJob();
	pJob->Init( pTRB, a_szFileName );
	AAssetStreaming::GetSingleton()->AddMainThreadJob2( pJob );

	return pJob;
}

// $deBlob: FUNCTION 00575f80
void ARegionManager::OnAssetLoaded( ATRBLoaderJob* a_pJob, void* a_pRegionState )
{
	RegionState* pRegionState = TSTATICCAST( RegionState, a_pRegionState );

	T2Material::sm_pTRBLoadCallback     = OnMaterialLoaded;
	T2Material::sm_pTRBLoadCallbackData = TNULL;
	TWorldShader::GetSingleton()->SetMaterialCreatedCallback( OnWorldMaterialCreated );
	TAssetInit::InitAssets( *a_pJob->m_pTRB, TTRUE, TTRUE );
	T2Material::sm_pTRBLoadCallback     = TNULL;
	T2Material::sm_pTRBLoadCallbackData = TNULL;
	TWorldShader::GetSingleton()->SetMaterialCreatedCallback( TNULL );

	TASSERT( pRegionState->m_iQueuedAssetCount > 0, "Finished loading a TRB even though there's none queued?" );

	if ( --pRegionState->m_iQueuedAssetCount == 0 )
		pRegionState->m_eState = STATE_Loaded;

	AAssetStreaming::GetSingleton()->ReleaseJob( a_pJob );
}

// $deBlob: FUNCTION 00576040
void __stdcall ARegionManager::OnMaterialLoaded( T2Material* a_pMaterial, void* a_pUserData )
{
	AMaterialManager::GetSingleton()->CreateMaterialUVAnimation( a_pMaterial, TNULL, TNULL );
}

// $deBlob: FUNCTION 00576060
void __stdcall ARegionManager::OnMaterialUnloaded( T2Material* a_pMaterial, void* a_pUserData )
{
	AMaterialManager::GetSingleton()->DestroyMaterialUVAnimation( a_pMaterial, TNULL );
}

// $deBlob: FUNCTION 00576080
void __stdcall ARegionManager::OnWorldMaterialCreated( TWorldMaterial* a_pMaterial, void* a_pT2Material )
{
	TASSERT( a_pMaterial->GetMetaMaterial() == a_pT2Material );
	a_pMaterial->SetUVAnimation( AMaterialManager::GetSingleton()->FindMaterialUVAnimation( TSTATICCAST( T2Material, a_pT2Material ), TNULL ) );
}

// $deBlob: FUNCTION 005760e0
void __stdcall ARegionManager::OnWorldMaterialDestroyed( TWorldMaterial* a_pMaterial )
{
	a_pMaterial->SetUVAnimation( TNULL );
}
