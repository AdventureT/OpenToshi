#include "ToshiPCH.h"
#include "Render/TAssetInit.h"
#include "XUI/TXUI.h"
#include "GUI/T2GUIFont.h"

#ifdef TRENDERINTERFACE_DX11
#  include "TMaterial_DX11.h"
#  include "TModel_DX11.h"
#  include "TModelLoaderWorld_DX11.h"
#  include "TVertexDecl_DX11.h"
#  include "TFXManager_DX11.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// The platform-specific TAssetInit callbacks (the renderer's and the game's assets: vertex declarations, FX, materials,
// models, worlds, XUI, fonts).
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006baea0
void TAssetInit::InitVertexDecleration( void* a_pData )
{
	TSTATICCAST( TVertexDecl_DX11::Decl, a_pData )->Init();
}

// $deBlob: FUNCTION 006baeb0
void TAssetInit::InitFXLite( void* a_pData )
{
	TSTATICCAST( TFXManager_DX11::FXLite, a_pData )->Init();
}

// $deBlob: FUNCTION 006baec0
void TAssetInit::InitMaterial( void* a_pData )
{
	TSTATICCAST( T2Material, a_pData )->Init();
}

// $deBlob: FUNCTION 006bb0c0
void TAssetInit::ReloadMaterial( void* a_pData )
{
	T2Material* pMaterial = TSTATICCAST( T2Material, a_pData );
	T2Material* pLoaded   = TMaterialManager::GetSingleton()->FindMaterial( pMaterial->GetName() );

	if ( pLoaded != TNULL )
		pLoaded->InitResource();
	else
		pMaterial->Init();
}

// $deBlob: FUNCTION 006baed0
void TAssetInit::InitModel( void* a_pData )
{
	TModelHAL* pModel = ( g_pMemHeap != TNULL ) ? new ( g_pMemHeap ) TModelHAL() : new TModelHAL();

	pModel->Create( TSTATICCAST( TTMDWin::TTRBWinHeader, a_pData ) );
	pModel->SetAssetData( a_pData );
}

// $deBlob: FUNCTION 006baf50
void TAssetInit::InitWorld( void* a_pData )
{
	TMemoryHeap* pHeap  = TModelLoaderWorld_DX11::GetWorldMemHeap();
	TModelHAL*   pModel = ( pHeap != TNULL ) ? new ( pHeap ) TModelHAL() : new TModelHAL();

	auto pHeader = TSTATICCAST( TTMDWin::TTRBWorldHeader, a_pData );

	pModel->SetAssetData( a_pData );
	pModel->SetLODCount( 1 );
	TModelLOD*             pLOD       = &pModel->m_LODs[ 0 ];
	TTMDWin::TRBWorldData* pWorldData = pHeader->m_pWorldData;

	pModel->Create( pHeader->m_pName, TFALSE );
	pModel->CreateSkeleton( pHeader->m_pSkeletonHeader, pHeader->m_pSkeleton, TFALSE );
	pModel->CreateCollision( pHeader->m_pModelCollision );
	pModel->m_fUnknown   = pHeader->m_fUnknown;
	pLOD->m_RenderVolume = pHeader->m_RenderVolume;

	TINT iMeshCount = 0;

	for ( TINT i = 0; i < pWorldData->m_iNumGroups; i++ )
	{
		TTMDWin::TRBWorldCellGroup* pGroup = pWorldData->m_ppGroups[ i ];

		for ( TINT k = 0; k < pGroup->m_iNumCells; k++ )
			iMeshCount += pGroup->m_ppCells[ k ]->m_iMeshCount;
	}

	pLOD->m_iMeshCount = iMeshCount;

	if ( pHeap != TNULL )
		pLOD->m_pMeshes = TSTATICCAST( TMesh*, TMemoryHeap_Malloc( pHeap, iMeshCount * sizeof( TMesh* ) ) );
	else
		pLOD->m_pMeshes = TSTATICCAST( TMesh*, TMalloc( iMeshCount * sizeof( TMesh* ) ) );

	TModelLoaderWorld_DX11::LoadWorldMeshTRB( pWorldData, pLOD );
}

// $deBlob: FUNCTION 006baf30
void TAssetInit::InitXUI( void* a_pData )
{
	TSTATICCAST( TXUIResourceTRB, a_pData )->Init();
}

// $deBlob: FUNCTION 006baf40
void TAssetInit::InitFont( void* a_pData )
{
	TSTATICCAST( T2GUIFontDef2, a_pData )->Init();
}

// $deBlob: FUNCTION 006bb0f0
void TAssetInit::DeinitFXLite( void* a_pData )
{
	TSTATICCAST( TFXManager_DX11::FXLite, a_pData )->Deinit();
}

// $deBlob: FUNCTION 006bb100
void TAssetInit::DeinitVertexDecleration( void* a_pData )
{
	TSTATICCAST( TVertexDecl_DX11::Decl, a_pData )->Deinit();
}

// $deBlob: FUNCTION 006bb110
void TAssetInit::DeinitMaterial( void* a_pData )
{
	TSTATICCAST( T2Material, a_pData )->Deinit();
}

// NOTE: retail jumps to DeinitModel (a world is a model)
// $deBlob: FUNCTION 006bb140
void TAssetInit::DeinitWorld( void* a_pData )
{
	DeinitModel( a_pData );
}

// $deBlob: FUNCTION 006bb130
void TAssetInit::DeinitXUI( void* a_pData )
{
	TSTATICCAST( TXUIResourceTRB, a_pData )->Deinit();
}

// $deBlob: FUNCTION 006bb120
void TAssetInit::DeinitFont( void* a_pData )
{
	TSTATICCAST( T2GUIFontDef2, a_pData )->Deinit();
}

TOSHI_NAMESPACE_END
