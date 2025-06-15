#pragma once
#include "Toshi/File/TTRB.h"
#include "Toshi/Xui/TXUI.h"
#include "Toshi/Render/TModelHAL.h"
#include "Toshi/Render/TTexture.h"
#include "Toshi2/T2Material.h"

TOSHI_NAMESPACE_BEGIN

class TAssetInit
{
public:
	TAssetInit() = delete;

	static TBOOL        g_bCreateResources;
	static TBOOL        g_bAllowCrossTRBReferences;
	static TTRB*        g_pCurrentTRB;
	static TMemoryHeap* g_pMemHeap;

private:
	using t_fourCCFunction = void (*)(void* a_pData);

	struct FourCCFunction
	{
		TUINT32          name;
		t_fourCCFunction func;
	};

public:
	static t_fourCCFunction constexpr InitTex = [](void* a_pData) {
		TSTATICCAST(TTexture, a_pData)->Init();
	};

	static t_fourCCFunction constexpr InitFXLite = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr InitMaterial = [](void* a_pData) {
		TSTATICCAST(T2Material, a_pData)->Init();
	};

	static t_fourCCFunction constexpr ReloadMaterial = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr InitKeyframes = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr InitModel = [](void* a_pData) {
		TModelHAL* a_modelHal;

		if (g_pMemHeap == TNULL)
		{
			a_modelHal = new TModelHAL();
		}
		else
		{
			a_modelHal = new (g_pMemHeap) TModelHAL();
		}

		a_modelHal->Create(TSTATICCAST(TTMDWin::TTRBWinHeader, a_pData));
		a_modelHal->SetDataHeader(a_pData);
	};

	// Empty ( de blob only calls a func where TASSERT("TFALSE","..\\..\\Source\\Render\\TVertexDecl_DX11.cpp",0x1f,"TASSERT"); gets called
	static t_fourCCFunction constexpr InitVertexDecleration = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr InitXUI = [](void* a_pData) {
		TSTATICCAST(TXUIResourceTRB, a_pData)->Init();
	};

	static t_fourCCFunction constexpr InitFont = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr InitWorld = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr InitParticleMaterial = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr InitParticleSettings = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr InitParticleEffect = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr DeinitParticleEffect = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr DeinitParticleSettings = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr DeinitParticleMaterial = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr DeinitWorld = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr DeinitFont = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr DeinitXUI = [](void* a_pData) {
		TSTATICCAST(TXUIResourceTRB, a_pData)->Deinit();
	};

	static t_fourCCFunction constexpr DeinitModel = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr DeinitKeyframes = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr DeinitMaterial = [](void* a_pData) {
		TSTATICCAST(T2Material, a_pData)->Deinit();
	};

	static t_fourCCFunction constexpr DeinitFXLite = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr DeinitVertexDecleration = [](void* a_pData) {

	};

	static t_fourCCFunction constexpr DeinitTex = [](void* a_pData) {
		TSTATICCAST(TTexture, a_pData)->Deinit();
	};

	static FourCCFunction constexpr g_FourCCReloadFunctions[5] = { { TFourCC("ttex"), InitTex }, { TFourCC("tfxl"), InitFXLite }, { TFourCC("tmat"), ReloadMaterial }, { TFourCC("tkey"), InitKeyframes }, { TFourCC("tmod"), InitModel } };

	static FourCCFunction constexpr g_FourCCInitFunctions[12] = {
		{ TFourCC("ttex"), InitTex },
		{ TFourCC("tvdc"), InitVertexDecleration },
		{ TFourCC("tfxl"), InitFXLite },
		{ TFourCC("tmat"), InitMaterial },
		{ TFourCC("tkey"), InitKeyframes },
		{ TFourCC("tmod"), InitModel },
		{ TFourCC("txui"), InitXUI },
		{ TFourCC("tfnt"), InitFont },
		{ TFourCC("twld"), InitWorld },
		{ TFourCC("tpma"), InitParticleMaterial },
		{ TFourCC("tpse"), InitParticleSettings },
		{ TFourCC("tpef"), InitParticleEffect },
	};

	static FourCCFunction constexpr g_FourCCDeinitFunctions[12] = {
		{ TFourCC("tpef"), DeinitParticleEffect },
		{ TFourCC("tpse"), DeinitParticleSettings },
		{ TFourCC("tpma"), DeinitParticleMaterial },
		{ TFourCC("twld"), DeinitWorld },
		{ TFourCC("tfnt"), DeinitFont },
		{ TFourCC("txui"), DeinitXUI },
		{ TFourCC("tmod"), DeinitModel },
		{ TFourCC("tkey"), DeinitKeyframes },
		{ TFourCC("tmat"), DeinitMaterial },
		{ TFourCC("tfxl"), DeinitFXLite },
		{ TFourCC("tvdc"), DeinitVertexDecleration },
		{ TFourCC("ttex"), DeinitTex },
	};

public:
	static void  InitAssets(TTRB& a_trb, TBOOL createResources, TBOOL allowCrossTRBReferences);
	static void  DeinitAssets(TTRB& a_trb);
	static void  Init(TTRB& a_trb, TUINT32 a_unk, t_fourCCFunction a_fourCCFunc);
	static TTRB* GetInitialisingTRB() { return g_pCurrentTRB; }
};

TOSHI_NAMESPACE_END
