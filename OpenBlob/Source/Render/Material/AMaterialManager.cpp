#include "pch.h"
#include "AMaterialManager.h"
#include "Terrain2/TerrainTypes.h"
#include "Platform/DX11/ASkinShader_DX11.h"

#include <Render/TMaterial.h>
#include <Render/TMesh.h>
#include <Platform/DX11/TMaterial_DX11.h>
#include <Platform/DX11/TWorldShader_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// A meta material symbol of a TRB ("tmat...")
static TBOOL IsMetaMaterialSymbol( const TCHAR* a_szSymbolName )
{
	return *TREINTERPRETCAST( const TUINT32*, a_szSymbolName ) == 0x74616d74;
}

// Sets up the AMaterialInfo a_pInfo (just allocated) for a_pMaterial
static void InitialiseMaterialInfo( AMaterialInfo* a_pInfo, T2Material* a_pMaterial, AUVAnimationPool* a_pUVPool, AUVAnimation** a_ppUVAnims )
{
	TINT iNumPasses = a_pMaterial->GetNumPasses();

	for ( TINT i = 0; i < iNumPasses; i++ )
	{
		TINT iUVFunctionType = a_pMaterial->GetPassUVFunctionType( i );
		TASSERT( 0 <= iUVFunctionType );

		if ( iUVFunctionType == AMaterialManager::AUVFunctionType_None || iUVFunctionType == AMaterialManager::AUVFunctionType_Unk4 )
		{
			a_ppUVAnims[ i ] = TNULL;
		}
		else
		{
			TASSERT( AMaterialManager::AUVFunctionType_NumOf > iUVFunctionType );
			AUVAnimation* pUVAnim = a_pUVPool->Alloc();

			if ( iUVFunctionType == AMaterialManager::AUVFunctionType_Offset )
				pUVAnim->SetupOffset();
			else if ( TUINT( iUVFunctionType - 2 ) < 2 )
				pUVAnim->SetupUnk1( iUVFunctionType == AMaterialManager::AUVFunctionType_Unk3 );

			a_ppUVAnims[ i ] = pUVAnim;
		}
	}
}

// $deBlob: FUNCTION 005f80b0
AMaterialManager::AMaterialManager()
    : m_oMaterialInfoPool( 0x40, "Common Mat Info Pool" )
    , m_oUVAnimPool( 0x40, "Common UV Anim Pool" )
{
	m_bPaused = TFALSE;
	m_fTime   = 0.0f;
	m_iFrame  = 0;
}

// $deBlob: FUNCTION 0056bd40
// $deBlob: FUNCTION 0056c0d0
AMaterialManager::~AMaterialManager()
{
}

// $deBlob: FUNCTION 005f8160
void AMaterialManager::UpdateUVs( TMaterial* a_pMaterial, TBOOL a_bShiftPass, TBOOL a_bUnk )
{
	AMaterialInfo* pInfo = TSTATICCAST( AMaterialInfo, a_pMaterial->GetUVAnimation() );

	if ( pInfo == TNULL )
		return;

	TASSERT( pInfo->m_pOwnerPool->m_lUsed.IsInList( pInfo ) );
	TINT iNumPasses = pInfo->m_pMaterial->GetNumPasses();

	for ( TINT i = 0; i < iNumPasses; i++ )
	{
		AUVAnimation* pUVAnim = pInfo->m_apUVAnimations[ i ];

		if ( pUVAnim == TNULL || pUVAnim->GetType() == AUVAnimation::TYPE_NONE )
			continue;

		TFLOAT fUOffset = 0.0f;
		TFLOAT fVOffset = 0.0f;

		T2Material* pMaterial = pInfo->m_pMaterial;
		pUVAnim->Update( pMaterial->GetPassUFunction( i ), pMaterial->GetPassVFunction( i ), pMaterial->GetPassUParam1( i ), pMaterial->GetPassUParam2( i ), pMaterial->GetPassVParam1( i ), pMaterial->GetPassVParam2( i ), fUOffset, fVOffset, m_fTime, m_iFrame, a_bUnk );

		if ( a_pMaterial->GetClass() == &TGetClass( TWorldMaterialHAL ) )
		{
			TINT iPass = i;

			if ( a_bShiftPass )
			{
				if ( i > 2 )
					continue;

				iPass = i + 1;
			}

			TSTATICCAST( TWorldMaterialHAL, a_pMaterial )->SetUVOffset( fUOffset, fVOffset, iPass );
		}
		else if ( a_pMaterial->GetClass() == &TGetClass( ASkinMaterialHAL ) )
		{
			TSTATICCAST( ASkinMaterialHAL, a_pMaterial )->SetUVOffset( fUOffset, fVOffset, i );
		}
	}
}

// $deBlob: FUNCTION 005f8310
void AMaterialManager::UpdateUVs( TModel* a_pModel )
{
	if ( m_bPaused )
		return;

	TModelLOD& rLOD = a_pModel->GetLOD( 0 );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		UpdateUVs( rLOD.GetMesh( i )->GetMaterial(), TFALSE, TTRUE );
}

// $deBlob: FUNCTION 005f8390
void AMaterialManager::UpdateUVs( TerrainTypes::InstanceMeshes* a_pMeshes )
{
	if ( m_bPaused )
		return;

	for ( TUINT i = 0; i < a_pMeshes->m_uiNumMeshes; i++ )
	{
		TWorldMeshInstanceData::Source& rMesh = a_pMeshes->m_pMeshes[ i ];
		UpdateUVs( rMesh.m_pMaterial, rMesh.m_szLightmap != TNULL, TTRUE );
	}
}

// $deBlob: FUNCTION 005f8590
const TCHAR* AMaterialManager::GetBlendModeString( TINT a_eBlendMode )
{
	switch ( a_eBlendMode )
	{
		case 0: return "BlendMode_Opaque";
		case 1: return "BlendMode_Modulate";
		case 2: return "BlendMode_Additive";
		case 3: return "BlendMode_Subtractive";
		case 4: return "BlendMode_ZPass";
		case 5: return "BlendMode_NoZWrite";
		case 6: return "BlendMode_NoZWriteAlpha";
		case 7: return "BlendMode_Translucent";
		default: return "Unknown blend mode";
	}
}

// $deBlob: FUNCTION 005f8620
TBOOL AMaterialManager::HasUVAnimation( T2Material* a_pMaterial )
{
	TINT iNumPasses = a_pMaterial->GetNumPasses();

	if ( iNumPasses < 1 )
		return TFALSE;

	for ( TINT i = 0; i < iNumPasses; i++ )
	{
		if ( a_pMaterial->GetPassUVFunctionType( i ) != AUVFunctionType_None && a_pMaterial->GetPassUVFunctionType( i ) != AUVFunctionType_Unk4 )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005f8670
void AMaterialManager::CreateMaterialUVAnimations( TTRB* a_pTRB, AMaterialInfoPool* a_pPool, AUVAnimationPool* a_pUVPool )
{
	TINT iNumSymbols = a_pTRB->GetNumSymbols();

	for ( TINT i = 0; i < iNumSymbols; i++ )
	{
		TTRB::TTRBSymbol* pSymbol = a_pTRB->GetSymbol( i );

		if ( !IsMetaMaterialSymbol( a_pTRB->GetSymbolName( pSymbol ) ) )
			continue;

		T2Material* pMaterial = TSTATICCAST( T2Material, a_pTRB->GetSymbolAddress( *pSymbol ) );

		if ( !HasUVAnimation( pMaterial ) )
			continue;

		AMaterialInfoPool* pPool   = a_pPool ? a_pPool : &m_oMaterialInfoPool;
		AUVAnimationPool*  pUVPool = a_pUVPool ? a_pUVPool : &m_oUVAnimPool;
		TBOOL              bFound  = TFALSE;

		for ( auto it = pPool->m_lUsed.Begin(); it != pPool->m_lUsed.End(); it++ )
		{
			if ( it->m_pMaterial == pMaterial || TStringManager::String8Compare( it->m_pMaterial->GetName(), pMaterial->GetName() ) == 0 )
			{
				bFound = TTRUE;
				break;
			}
		}

		if ( bFound )
			continue;

		AMaterialInfo* pInfo = pPool->Alloc();
		pInfo->m_pMaterial   = pMaterial;
		InitialiseMaterialInfo( pInfo, pMaterial, pUVPool, pInfo->m_apUVAnimations );
	}
}

// $deBlob: FUNCTION 005f88a0
void AMaterialManager::DestroyMaterialUVAnimations( TTRB* a_pTRB, AMaterialInfoPool* a_pPool )
{
	TINT iNumSymbols = a_pTRB->GetNumSymbols();

	for ( TINT i = 0; i < iNumSymbols; i++ )
	{
		TTRB::TTRBSymbol* pSymbol = a_pTRB->GetSymbol( i );

		if ( IsMetaMaterialSymbol( a_pTRB->GetSymbolName( pSymbol ) ) )
			DestroyMaterialUVAnimation( TSTATICCAST( T2Material, a_pTRB->GetSymbolAddress( *pSymbol ) ), a_pPool );
	}
}

// $deBlob: FUNCTION 005f8970
AMaterialInfo* AMaterialManager::CreateMaterialUVAnimation( T2Material* a_pMaterial, AMaterialInfoPool* a_pPool, AUVAnimationPool* a_pUVPool )
{
	if ( !HasUVAnimation( a_pMaterial ) )
		return TNULL;

	if ( a_pPool == TNULL )
		a_pPool = &m_oMaterialInfoPool;

	if ( a_pUVPool == TNULL )
		a_pUVPool = &m_oUVAnimPool;

	for ( auto it = a_pPool->m_lUsed.Begin(); it != a_pPool->m_lUsed.End(); it++ )
	{
		if ( it->m_pMaterial == a_pMaterial )
			return it;

		if ( TStringManager::String8Compare( it->m_pMaterial->GetName(), a_pMaterial->GetName() ) == 0 )
			return it;
	}

	AMaterialInfo* pInfo = a_pPool->Alloc();
	pInfo->m_pMaterial   = a_pMaterial;
	InitialiseMaterialInfo( pInfo, a_pMaterial, a_pUVPool, pInfo->m_apUVAnimations );

	return pInfo;
}

// $deBlob: FUNCTION 005f8b30
void AMaterialManager::DestroyMaterialUVAnimation( T2Material* a_pMaterial, AMaterialInfoPool* a_pPool )
{
	AMaterialInfo* pInfo = FindMaterialUVAnimation( a_pMaterial, a_pPool );

	if ( pInfo == TNULL )
		return;

	for ( TINT i = 0; i < pInfo->m_pMaterial->GetNumPasses(); i++ )
	{
		if ( pInfo->m_apUVAnimations[ i ] != TNULL )
		{
			AUVAnimationPool::Free( pInfo->m_apUVAnimations[ i ] );
			pInfo->m_apUVAnimations[ i ] = TNULL;
		}
	}

	pInfo->m_pMaterial = TNULL;
	AMaterialInfoPool::Free( *pInfo );
}

// $deBlob: FUNCTION 005f8b80
void AMaterialManager::SetupModelUVAnimations( TModel* a_pModel, AMaterialInfoPool* a_pPool )
{
	TModelLOD& rLOD = a_pModel->GetLOD( 0 );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		TMaterial* pMaterial = rLOD.GetMeshes()[ i ]->GetMaterial();

		if ( pMaterial != TNULL && pMaterial->GetMetaMaterial() != TNULL )
			pMaterial->SetUVAnimation( FindMaterialUVAnimation( TSTATICCAST( T2Material, pMaterial->GetMetaMaterial() ), a_pPool ) );
	}
}

// $deBlob: FUNCTION 005f8c10
AMaterialInfo* AMaterialManager::FindMaterialUVAnimation( T2Material* a_pMaterial, AMaterialInfoPool* a_pPool )
{
	if ( !HasUVAnimation( a_pMaterial ) )
		return TNULL;

	if ( a_pPool == TNULL )
		a_pPool = &m_oMaterialInfoPool;

	for ( auto it = a_pPool->m_lUsed.Begin(); it != a_pPool->m_lUsed.End(); it++ )
	{
		if ( it->m_pMaterial == a_pMaterial )
			return it;
	}

	return TNULL;
}

// $deBlob: FUNCTION 005f8cb0
AMaterialInfo* AMaterialManager::FindMaterialUVAnimation( const TCHAR* a_szMaterialName, AMaterialInfoPool* a_pPool )
{
	if ( a_pPool == TNULL )
		a_pPool = &m_oMaterialInfoPool;

	for ( auto it = a_pPool->m_lUsed.Begin(); it != a_pPool->m_lUsed.End(); it++ )
	{
		if ( TStringManager::String8Compare( it->m_pMaterial->GetName(), a_szMaterialName ) == 0 )
			return it;
	}

	return TNULL;
}
