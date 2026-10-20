#include "pch.h"
#include "AFXCompositionManager.h"
#include "AFXCompositionInstance.h"
#include "Console/AConsoleState.h"
#include "AAssetStreaming.h"
#include "ACurrentLevelInformation.h"
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The console command fx_reload
// $deBlob: FUNCTION 005b60e0
void __stdcall AFXCompositionManager::FXReloadCommand( Toshi::TString8* a_pArgs, TINT a_iNumArgs )
{
	AFXCompositionManager::GetSingleton()->m_bReloadRequested = TTRUE;
}

// $deBlob: FUNCTION 005b5b70
AFXCompositionManager::AFXCompositionManager()
{
	m_pLoadJob         = TNULL;
	m_pTRB             = TNULL;
	m_bOwnsTRB         = TFALSE;
	m_pFXDataList      = TNULL;
	m_bReloading       = TFALSE;
	m_bReloadRequested = TFALSE;

	AConsoleState::RegisterCommand( "fx_reload", FXReloadCommand );
}

// $deBlob: FUNCTION 005b5c40
// $deBlob: FUNCTION 0056bfb0
AFXCompositionManager::~AFXCompositionManager()
{
	m_pFXDataList = TNULL;

	if ( m_bOwnsTRB && m_pTRB != TNULL )
		delete m_pTRB;

	m_pTRB     = TNULL;
	m_bOwnsTRB = TFALSE;
	m_oInstances.DeleteAll();
	m_strFileName = "";

	AConsoleState::UnregisterCommand( FXReloadCommand );
}

// $deBlob: FUNCTION 005b5d30
TBOOL AFXCompositionManager::Load( const TCHAR* a_szFileName )
{
	TASSERT( TNULL == m_pFXDataList );

	m_pLoadJob = AAssetStreaming::GetSingleton()->GetFreeJob();
	m_pLoadJob->Init( new TTRB(), a_szFileName );
	AAssetStreaming::GetSingleton()->AddMainThreadJob2( m_pLoadJob );
	m_bOwnsTRB    = TTRUE;
	m_strFileName = a_szFileName;

	return TTRUE;
}

// $deBlob: FUNCTION 005b5e00
TBOOL AFXCompositionManager::LoadFromLevel( TINT a_iDataBlock )
{
	m_pTRB        = ACurrentLevelInformation::GetSingleton()->Stream( a_iDataBlock );
	m_bOwnsTRB    = TFALSE;
	m_pFXDataList = m_pTRB->GetSymbolAddress( "FXData" );

	if ( m_pFXDataList == TNULL )
		return TFALSE;

	TCHAR* szPath = TStringManager::GetTempString8();
	ACurrentLevelInformation::GetSingleton()->GetDataBlockPath( a_iDataBlock, szPath, 0x400 );
	m_strFileName = szPath;

	return TTRUE;
}

// $deBlob: FUNCTION 005b5e70
TBOOL AFXCompositionManager::IsDoneLoading()
{
	if ( m_pLoadJob == TNULL )
		return TTRUE;

	if ( m_pLoadJob->m_bFinished )
	{
		m_pTRB        = m_pLoadJob->m_pTRB;
		m_pFXDataList = m_pTRB->GetSymbolAddress( "FXData" );

		if ( m_pFXDataList != TNULL )
		{
			AAssetStreaming::GetSingleton()->ReleaseJob( m_pLoadJob );
			m_pLoadJob = TNULL;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005b5ec0
TBOOL AFXCompositionManager::HasFXData( const TCHAR* a_szName )
{
	return m_pTRB != TNULL && m_pTRB->GetSymbolAddress( a_szName ) != TNULL;
}

// $deBlob: FUNCTION 005b5ef0
void AFXCompositionManager::UnLoad()
{
	m_pFXDataList = TNULL;

	if ( m_bOwnsTRB && m_pTRB != TNULL )
		delete m_pTRB;

	m_pTRB     = TNULL;
	m_bOwnsTRB = TFALSE;
	m_oInstances.DeleteAll();
	m_strFileName = "";
}

// $deBlob: FUNCTION 005b5f40
void AFXCompositionManager::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bReloading == TTRUE )
	{
		if ( m_pLoadJob != TNULL )
		{
			if ( !m_pLoadJob->m_bFinished )
				return;

			m_pTRB        = m_pLoadJob->m_pTRB;
			m_pFXDataList = m_pTRB->GetSymbolAddress( "FXData" );

			if ( m_pFXDataList == TNULL )
				return;

			AAssetStreaming::GetSingleton()->ReleaseJob( m_pLoadJob );
			m_pLoadJob = TNULL;
		}

		ReinitializeInstances();
		m_bReloading = TFALSE;
	}
	else if ( m_pLoadJob == TNULL && m_strFileName.Length() != 0 && m_bReloadRequested )
	{
		for ( auto it = m_oInstances.Begin(); it != m_oInstances.End(); it++ )
			it->GetInstance()->Destroy();

		m_pFXDataList = TNULL;

		if ( m_bOwnsTRB && m_pTRB != TNULL )
			delete m_pTRB;

		m_pTRB     = TNULL;
		m_bOwnsTRB = TFALSE;
		Load( m_strFileName );
		m_bReloading       = TTRUE;
		m_bReloadRequested = TFALSE;
	}
}

// $deBlob: FUNCTION 005b60f0
void AFXCompositionManager::AddInstance( AFXCompositionInstance* a_pInstance )
{
	RemoveInstance( a_pInstance );
	m_oInstances.PushBack( new AFXInstanceNode( a_pInstance ) );
}

// $deBlob: FUNCTION 005b6160
void AFXCompositionManager::RemoveInstance( AFXCompositionInstance* a_pInstance )
{
	for ( auto it = m_oInstances.Begin(); it != m_oInstances.End(); it++ )
	{
		if ( it->GetInstance() == a_pInstance )
		{
			AFXInstanceNode* pNode = it;
			m_oInstances.Erase( it );
			delete pNode;
			return;
		}
	}
}

// $deBlob: FUNCTION 005b6250
void AFXCompositionManager::ReinitializeInstances()
{
	for ( auto it = m_oInstances.Begin(); it != m_oInstances.End(); it++ )
		it->GetInstance()->Reinitialize();
}

// $deBlob: FUNCTION 005b6370
AFXCompositionManager::AFXInstanceNode::AFXInstanceNode( AFXCompositionInstance* a_pInstance )
    : m_pInstance( a_pInstance )
{
}
