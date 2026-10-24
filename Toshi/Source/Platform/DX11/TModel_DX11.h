#pragma once
#include "Render/TModel.h"
#include "Render/TTMDWin.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: the PC model, made from a "tmod" TRB asset by TAssetInit::InitModel.
//-----------------------------------------------------------------------------
class TModelHAL : public TModel
{
public:
	using t_TRBLoadCallback2 = TBOOL( __stdcall* )( TModelHAL* a_pModel, TTMDWin::TTRBWinHeader* a_pHeader );

public:
	TModelHAL();
	~TModelHAL();

	using TModel::Create;
	using TModel::LoadTRB;

	TBOOL CreateTRB( const TCHAR* a_szFileName, TBOOL a_bLoadImmediately );
	TBOOL LoadTRB( const TCHAR* a_szFileName );
	TBOOL Create( TTMDWin::TTRBWinHeader* a_pTMDHeader );

	TTMDWin::TTRBWinHeader* GetTMDHeader() const { return m_pTMDHeader; }

	// $deBlob: FUNCTION 005f9ad0
	static void SetTRBLoadCallback2( t_TRBLoadCallback2 a_fnCallback ) { sm_pTRBLoadCallback2 = a_fnCallback; }

	// The default sm_pTRBLoadCallback2: creates the skin meshes of every LOD
	static TBOOL __stdcall LoadSkinModel( TModelHAL* a_pModel, TTMDWin::TTRBWinHeader* a_pHeader );

private:
	static void __stdcall LoadSkinLOD( TModelHAL* a_pModel, TINT a_iLODIndex, TModelLOD* a_pLOD, TTMDWin::TRBLODHeader* a_pTRBLOD );

private:
	static t_TRBLoadCallback2 sm_pTRBLoadCallback2;
	static const TClass*      s_pSkinMaterial;      // looked up by LoadSkinLOD, unused

private:
	TTMDWin::TTRBWinHeader* m_pTMDHeader; // 0xBC
}; // 0xC0

TOSHI_NAMESPACE_END
