#pragma once
#include "File/TTRB.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: runs the per-asset init/deinit callbacks of a TRB by the FourCC of its symbols ("ttex", "tmat", "txui"...)
// Here: InitAssets, DeinitAssets and the callbacks of the engine assets; the
// platform-specific callbacks are in Platform/DX11/TAssetInit_DX11.cpp.
//-----------------------------------------------------------------------------
class TAssetInit
{
public:
	using t_fourCCFunction = void ( * )( void* a_pData );

	struct FourCCFunction
	{
		TUINT32          uiName; // 0x00 the FourCC, compared with the first 4 bytes of the symbol's name
		t_fourCCFunction fnFunc; // 0x04
	};

public:
	TAssetInit() = delete;

	// Calls the init (a_bCreateResources) or reload callbacks of every symbol of the TRB, then destroys the
	// sections that are only needed while initialising (flag 8)
	static void InitAssets( TTRB& a_rTRB, TBOOL a_bCreateResources, TBOOL a_bAllowCrossTRBReferences );

	// Calls the deinit callbacks of every symbol of the TRB
	static void DeinitAssets( TTRB& a_rTRB );

	// The TRB whose assets are being initialised (T2ResourceManager::CreateResource records it)
	static TTRB* GetInitialisingTRB() { return g_pCurrentTRB; }

private:
	// Calls a_fnFunc for every symbol of the TRB whose name starts with the FourCC a_uiName (inlined)
	static void Init( TTRB& a_rTRB, TUINT32 a_uiName, t_fourCCFunction a_fnFunc );

	// Render/TAssetInit.cpp
	static void InitTex( void* a_pData );
	static void InitKeyframes( void* a_pData );
	static void InitParticleMaterial( void* a_pData );
	static void InitParticleSettings( void* a_pData );
	static void InitParticleEffect( void* a_pData );
	static void DeinitTex( void* a_pData );
	static void DeinitKeyframes( void* a_pData );
	static void DeinitModel( void* a_pData );
	static void DeinitParticleMaterial( void* a_pData );
	static void DeinitParticleSettings( void* a_pData );
	static void DeinitParticleEffect( void* a_pData );

	// Platform/DX11/TAssetInit_DX11.cpp
	static void InitVertexDecleration( void* a_pData );
	static void InitFXLite( void* a_pData );
	static void InitMaterial( void* a_pData );
	static void ReloadMaterial( void* a_pData );
	static void InitModel( void* a_pData );
	static void InitWorld( void* a_pData );
	static void InitXUI( void* a_pData );
	static void InitFont( void* a_pData );
	static void DeinitFXLite( void* a_pData );
	static void DeinitVertexDecleration( void* a_pData );
	static void DeinitMaterial( void* a_pData );
	static void DeinitWorld( void* a_pData );
	static void DeinitXUI( void* a_pData );
	static void DeinitFont( void* a_pData );

public:
	static TBOOL        g_bCreateResources;
	static TBOOL        g_bAllowCrossTRBReferences;
	static TTRB*        g_pCurrentTRB;
	static TMemoryHeap* g_pMemHeap; // the heap of the models

private:
	static const FourCCFunction g_FourCCInitFunctions[ 12 ];
	static const FourCCFunction g_FourCCReloadFunctions[ 5 ];
	static const FourCCFunction g_FourCCDeinitFunctions[ 12 ];
};

TOSHI_NAMESPACE_END
