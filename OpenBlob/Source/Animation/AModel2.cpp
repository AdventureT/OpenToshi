#include "pch.h"
#include "AModel2.h"
#include "AModelInstance2.h"
#include "Buildings/ABuildingInstance.h"
#include "Buildings/ALandmarkInstance.h"
#include "Buildings/ATreeInstance.h"
#include "Buildings/AGrassInstance.h"
#include "AAssetStreaming.h"

#include <Render/TModelManager.h>
#include <Render/TSkeleton.h>
#include <Platform/DX11/TRender_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The models whose skeletons play more animations at once than their TRB says (by TXS name)
static const struct
{
	const TCHAR* szName;
	TINT16       iAnimationMaxCount;
} s_aAnimationMaxCounts[] = {
	{ "civ1.txs", 3 },
	{ "civ2.txs", 3 },
	{ "civ3.txs", 3 },
	{ "civshell.txs", 3 },
	{ "bossmech.txs", 10 },
};

// $deBlob: FUNCTION 0056fd50
static void SetAnimationMaxCount( T2ModelPtr a_pModel, const TCHAR* a_szTXSName )
{
	for ( TINT i = 0; i < TINT( TARRAYSIZE( s_aAnimationMaxCounts ) ); i++ )
	{
		if ( TStringManager::String8Compare( s_aAnimationMaxCounts[ i ].szName, a_szTXSName, -1 ) == 0 )
		{
			a_pModel->GetSkeleton()->SetAnimationMaxCount( s_aAnimationMaxCounts[ i ].iAnimationMaxCount );
			break;
		}
	}
}

TModelInstance* AModel2::CreateModelInstance( TModel* a_pModel )
{
	TModelInstance* pModelInstance = a_pModel->CreateInstance();
	pModelInstance->m_eFlags |= TModelInstance::Flags_DistanceFade;
	pModelInstance->m_fFadeStart    = 200.0f;
	pModelInstance->m_fFadeEnd      = 210.0f;
	pModelInstance->m_fFadeRangeInv = 0.1f;

	return pModelInstance;
}

// $deBlob: FUNCTION 0056f3a0
AModel2::AModel2()
    : m_pModel( T2ResourcePtr::IDINVALID )
{
	m_uiFlags = 2;
}

// $deBlob: FUNCTION 0056f420
// $deBlob: FUNCTION 0056ff50
AModel2::~AModel2()
{
	Unload();
}

// $deBlob: FUNCTION 0056f4e0
void AModel2::Load()
{
	TASSERT( TFALSE == HASANYFLAG( m_uiFlags, FLAG_Loaded ) );

	TString8 strFileName = m_FileName;
	strFileName.MakeLower();

	TCHAR* szTempString = TStringManager::GetTempString8();
	TCHAR* szTXSName    = szTempString + AAssetStreaming::GetTXSNameFromTRB( strFileName, szTempString, 1024 );

	T2ModelPtr pModel = TModelManager::GetSingleton()->FindModel( szTXSName );

	if ( pModel.GetID() != T2ResourcePtr::IDINVALID )
	{
		TTRB* pTRB = T2ResourceManager::GetSingleton()->GetResourceData( pModel->GetResourceId() )->GetCreatedInTRB();
		pModel->SetTRB( pTRB, TFALSE );

		SetAnimationMaxCount( pModel, szTXSName );
		Initialise( pModel );
	}
}

// $deBlob: FUNCTION 0056f660
void AModel2::Unload()
{
	if ( m_pModel.GetID() != T2ResourcePtr::IDINVALID )
	{
		TRenderDX11::Interface()->WaitForEndOfRender();

		for ( auto it = m_lInstances.Begin(); it != m_lInstances.End(); it++ )
			it->OnUnloaded();

		m_uiFlags &= ~FLAG_Loaded;
	}
}

// $deBlob: FUNCTION 0056f730
AModelInstance2* AModel2::CreateInstance( TBOOL a_bCreateSoundBreakpoints, TUINT a_eType, TBOOL a_bCreateAnimInterface )
{
	AModelInstance2* pInstance;

	switch ( a_eType & 7 )
	{
		case TerrainTypes::InstanceType_Building:
			pInstance = new ABuildingInstance( this, a_bCreateSoundBreakpoints );
			break;
		case TerrainTypes::InstanceType_Landmark:
			pInstance = new ALandmarkInstance( this, a_bCreateSoundBreakpoints );
			break;
		case TerrainTypes::InstanceType_Tree:
			pInstance = new ATreeInstance( this, a_bCreateSoundBreakpoints );
			break;
		case TerrainTypes::InstanceType_Grass:
			pInstance = new AGrassInstance( this, a_bCreateSoundBreakpoints );
			break;
		default:
			pInstance = new AModelInstance2( this, a_bCreateSoundBreakpoints, a_bCreateAnimInterface );
			break;
	}

	if ( HASANYFLAG( m_uiFlags, FLAG_Loaded ) )
		pInstance->OnLoaded( CreateModelInstance( m_pModel.GetData() ) );

	if ( pInstance->IsRenderable() )
		m_lInstances.PushFront( pInstance );
	else
		m_lInstances.PushBack( pInstance );

	return pInstance;
}

// $deBlob: FUNCTION 0056f920
void AModel2::DestroyInstance( AModelInstance2* a_pInstance )
{
	TASSERT( m_lInstances.IsInList( a_pInstance ) );

	a_pInstance->Remove();
	delete a_pInstance;
}

// $deBlob: FUNCTION 0056fac0
void AModel2::RenderLoadingScreenInstances()
{
	for ( auto it = m_lInstances.Begin(); it != m_lInstances.End(); it++ )
	{
		if ( it->GetFlags() & AModelInstance2::FLAG_RenderOnLoadingScreen )
			it->Render();
	}
}

// Renders the renderable instances (kept at the front of the list)
// $deBlob: FUNCTION 0056f990
void AModel2::RenderAllInstances()
{
	for ( auto it = m_lInstances.Begin(); it != m_lInstances.End(); it++ )
	{
		if ( !it->IsRenderable() )
			return;

		it->Render();
	}
}

// $deBlob: FUNCTION 0056fbe0
void AModel2::Initialise( T2ModelPtr a_pModel )
{
	m_pModel = a_pModel;

	for ( auto it = m_lInstances.Begin(); it != m_lInstances.End(); it++ )
		it->OnLoaded( CreateModelInstance( m_pModel.GetData() ) );

	m_uiFlags |= FLAG_Loaded;
}
