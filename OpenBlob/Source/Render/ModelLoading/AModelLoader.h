#pragma once
#include <Render/TModel.h>
#include <Render/TShader.h>
#include <Render/TMaterial.h>
#include <Render/TTMDWin.h>
#include <Toshi/T2DList.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TString8.h>
#include <File/TTRB.h>

#ifdef TRENDERINTERFACE_DX11
#  include <Platform/DX11/TModel_DX11.h>
#endif

//-----------------------------------------------------------------------------
// Purpose: loads the game's models (AModelLoaderSkin_Win.cpp): the TRB callbacks of TModel/TModelHAL, the materials of
// the meshes (shared by name within a TRB, reference counted).
//-----------------------------------------------------------------------------
class AModelLoader : public Toshi::TSingleton<AModelLoader>
{
public:
	static TCONSTEXPR_VAR TINT MAX_MATERIALS = 512;

	struct MaterialNode : public Toshi::T2DList<MaterialNode>::Node
	{
		Toshi::TMaterial* m_pMaterial;       // 0x08
		TCHAR             m_szName[ 0x40 ];  // 0x0C
		TINT              m_iReferenceCount; // 0x4C
		Toshi::TTRB*      m_pOwnerTRB;       // 0x50
	};                                       // 0x54

public:
	AModelLoader();
	~AModelLoader();

	void GetMaterial( Toshi::TShader* a_pShader, const TCHAR* a_szName, Toshi::TMaterial*& a_rOutMaterial );

	static void DestroyMaterial( Toshi::TMaterial* a_pMaterial );

private:
	Toshi::TMaterial* GetCreatedMaterial( const Toshi::TString8& a_rName, Toshi::TTRB* a_pTRB );

	static TBOOL __stdcall AModelLoaderLoadTRBCallback( Toshi::TTRB* a_pTRB, Toshi::TModel* a_pModel );
	static TBOOL __stdcall AModelLoaderLoadTRBCallback2( Toshi::TModelHAL* a_pModel, Toshi::TTMDWin::TTRBWinHeader* a_pHeader );
	TBOOL LoadTRBCallback2( Toshi::TModel* a_pModel, Toshi::TTMDWin::TTRBWinHeader* a_pHeader );

	// AModelLoaderSkin_Win.cpp
	static void LoadSkinMeshTRB( Toshi::TModel* a_pModel, TINT a_iLODIndex, Toshi::TModelLOD* a_pModelLOD, Toshi::TTMDWin::TRBLODHeader* a_pLODHeader );

private:
	static MaterialNode                 ms_aNodes[ MAX_MATERIALS ];
	static Toshi::T2DList<MaterialNode> ms_oFreeMaterials;
	static Toshi::T2DList<MaterialNode> ms_oMaterialList;
	static const Toshi::TClass*         ms_pSkinMaterialClass; // looked up, unused
};
