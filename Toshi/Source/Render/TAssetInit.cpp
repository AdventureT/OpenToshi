#include "ToshiPCH.h"
#include "TAssetInit.h"
#include "TParticleSystem/TParticleSystem.h"
#include "TParticleSystem/TParticleEffectManager.h"
#include "TParticleSystem/TParticleMaterialManager.h"
#include "TParticleSystem/TParticleMaterial.h"
#include "TParticleSystem/TParticleEffect.h"
#include "TParticleSystem/TGenericParticlesSettings.h"
#include "TModelManager.h"
#include "TRender.h"

#ifdef TRENDERINTERFACE_DX11
#  include "Platform/DX11/TTexture_DX11.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TBOOL        TAssetInit::g_bCreateResources         = TTRUE;
TBOOL        TAssetInit::g_bAllowCrossTRBReferences = TFALSE;
TTRB*        TAssetInit::g_pCurrentTRB              = TNULL;
TMemoryHeap* TAssetInit::g_pMemHeap                 = TNULL;

const TAssetInit::FourCCFunction TAssetInit::g_FourCCInitFunctions[ 12 ] = {
	{ TFourCC( "ttex" ), InitTex },
	{ TFourCC( "tvdc" ), InitVertexDecleration },
	{ TFourCC( "tfxl" ), InitFXLite },
	{ TFourCC( "tmat" ), InitMaterial },
	{ TFourCC( "tkey" ), InitKeyframes },
	{ TFourCC( "tmod" ), InitModel },
	{ TFourCC( "txui" ), InitXUI },
	{ TFourCC( "tfnt" ), InitFont },
	{ TFourCC( "twld" ), InitWorld },
	{ TFourCC( "tpma" ), InitParticleMaterial },
	{ TFourCC( "tpse" ), InitParticleSettings },
	{ TFourCC( "tpef" ), InitParticleEffect },
};

const TAssetInit::FourCCFunction TAssetInit::g_FourCCReloadFunctions[ 5 ] = {
	{ TFourCC( "ttex" ), InitTex },
	{ TFourCC( "tfxl" ), InitFXLite },
	{ TFourCC( "tmat" ), ReloadMaterial },
	{ TFourCC( "tkey" ), InitKeyframes },
	{ TFourCC( "tmod" ), InitModel },
};

const TAssetInit::FourCCFunction TAssetInit::g_FourCCDeinitFunctions[ 12 ] = {
	{ TFourCC( "tpef" ), DeinitParticleEffect },
	{ TFourCC( "tpse" ), DeinitParticleSettings },
	{ TFourCC( "tpma" ), DeinitParticleMaterial },
	{ TFourCC( "twld" ), DeinitWorld },
	{ TFourCC( "tfnt" ), DeinitFont },
	{ TFourCC( "txui" ), DeinitXUI },
	{ TFourCC( "tmod" ), DeinitModel },
	{ TFourCC( "tkey" ), DeinitKeyframes },
	{ TFourCC( "tmat" ), DeinitMaterial },
	{ TFourCC( "tfxl" ), DeinitFXLite },
	{ TFourCC( "tvdc" ), DeinitVertexDecleration },
	{ TFourCC( "ttex" ), DeinitTex },
};

// $deBlob: FUNCTION 006b1830
void TAssetInit::InitAssets( TTRB& a_rTRB, TBOOL a_bCreateResources, TBOOL a_bAllowCrossTRBReferences )
{
	g_bAllowCrossTRBReferences = a_bAllowCrossTRBReferences;
	g_bCreateResources         = a_bCreateResources;
	g_pCurrentTRB              = &a_rTRB;

	const FourCCFunction* pFunctions;
	TINT                  iNumFunctions;

	if ( a_bCreateResources )
	{
		pFunctions    = g_FourCCInitFunctions;
		iNumFunctions = TARRAYSIZE( g_FourCCInitFunctions );
	}
	else
	{
		pFunctions    = g_FourCCReloadFunctions;
		iNumFunctions = TARRAYSIZE( g_FourCCReloadFunctions );
	}

	for ( TINT i = 0; i < iNumFunctions; i++ )
		Init( a_rTRB, pFunctions[ i ].uiName, pFunctions[ i ].fnFunc );

	// The sections only needed to initialise the assets
	for ( TINT i = 0; i < a_rTRB.GetHeader()->m_i32SectionCount; i++ )
	{
		if ( a_rTRB.GetSectionInfo( i )->m_Unk2 & 8 )
			a_rTRB.DestroySection( i );
	}

	g_pCurrentTRB = TNULL;
}

// $deBlob: FUNCTION 006b1950
void TAssetInit::DeinitAssets( TTRB& a_rTRB )
{
	for ( TINT i = 0; i < TARRAYSIZE( g_FourCCDeinitFunctions ); i++ )
		Init( a_rTRB, g_FourCCDeinitFunctions[ i ].uiName, g_FourCCDeinitFunctions[ i ].fnFunc );
}

// $deBlob: FUNCTION 006b1a50
void TAssetInit::Init( TTRB& a_rTRB, TUINT32 a_uiName, t_fourCCFunction a_fnFunc )
{
	TINT iNumSymbols = a_rTRB.GetSymbolTable()->m_i32SymbCount;

	for ( TINT i = 0; i < iNumSymbols; i++ )
	{
		TTRB::TTRBSymbol* pSymbol = a_rTRB.GetSymbol( i );

		if ( *TREINTERPRETCAST( const TUINT32*, a_rTRB.GetSymbolName( pSymbol ) ) == a_uiName )
			a_fnFunc( a_rTRB.GetSymbolAddress( *pSymbol ) );
	}
}

// $deBlob: FUNCTION 006b1ac0
void TAssetInit::InitTex( void* a_pData )
{
	TSTATICCAST( TTexture, a_pData )->Init();
}

// $deBlob: FUNCTION 006b1ad0
void TAssetInit::InitKeyframes( void* a_pData )
{
	TRender::GetSingleton()->GetKeyframeLibraryManager().LoadLibrary( TSTATICCAST( TKeyframeLibrary::TRBHeader, a_pData ) );
}

// $deBlob: FUNCTION 006b1af0
void TAssetInit::InitParticleMaterial( void* a_pData )
{
	TParticleSystem::GetSingleton()->GetMaterialManager()->AddMaterial( TSTATICCAST( TParticleMaterial, a_pData ) );
}

// $deBlob: FUNCTION 006b1b10
void TAssetInit::InitParticleSettings( void* a_pData )
{
	TParticleSystem::GetSingleton()->GetEffectManager()->AddSetting( TSTATICCAST( TGenericParticlesSettings, a_pData ) );
}

// $deBlob: FUNCTION 006b1b30
void TAssetInit::InitParticleEffect( void* a_pData )
{
	TParticleSystem::GetSingleton()->GetEffectManager()->AddEffect( TSTATICCAST( TParticleEffect, a_pData ) );
}

// $deBlob: FUNCTION 006b1b50
void TAssetInit::DeinitTex( void* a_pData )
{
	TSTATICCAST( TTexture, a_pData )->Deinit();
}

// $deBlob: FUNCTION 006b1b60
void TAssetInit::DeinitKeyframes( void* a_pData )
{
	TKeyframeLibraryManager& rManager = TRender::GetSingleton()->GetKeyframeLibraryManager();
	TKeyframeLibrary*        pLibrary = rManager.GetLibrary( TSTATICCAST( TKeyframeLibrary::TRBHeader, a_pData )->m_szName );

	if ( pLibrary != TNULL )
		rManager.UnloadLibrary( pLibrary );
}

// $deBlob: FUNCTION 006b1b90
void TAssetInit::DeinitModel( void* a_pData )
{
	TModel* pModel = TNULL;

	{
		T2ModelPtr pIt = TModelManager::GetSingleton()->GetFirstModel();

		while ( pIt )
		{
			if ( pIt->GetAssetData() == a_pData )
			{
				pModel = pIt.GetData();
				break;
			}

			pIt = TModelManager::GetSingleton()->GetNextModel( pIt );
		}
	}

	// NOTE: retail doesn't check that the model was found
	pModel->Delete();
}

// $deBlob: FUNCTION 006b1c10
void TAssetInit::DeinitParticleMaterial( void* a_pData )
{
	TParticleSystem::GetSingleton()->GetMaterialManager()->RemoveMaterial( TSTATICCAST( TParticleMaterial, a_pData ) );
}

// $deBlob: FUNCTION 006b1c30
void TAssetInit::DeinitParticleSettings( void* a_pData )
{
	TParticleSystem::GetSingleton()->GetEffectManager()->RemoveSetting( TSTATICCAST( TGenericParticlesSettings, a_pData ) );
}

// $deBlob: FUNCTION 006b1c50
void TAssetInit::DeinitParticleEffect( void* a_pData )
{
	TParticleSystem::GetSingleton()->GetEffectManager()->RemoveEffect( TSTATICCAST( TParticleEffect, a_pData ) );
}

TOSHI_NAMESPACE_END
