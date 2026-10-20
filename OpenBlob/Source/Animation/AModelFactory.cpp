#include "pch.h"
#include "AModelFactory.h"
#include "AModelInstance2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00570160
AModelFactory::AModelFactory()
{
}

// $deBlob: FUNCTION 005701f0
// $deBlob: FUNCTION 0056c100
AModelFactory::~AModelFactory()
{
	TASSERT( m_Models.IsEmpty() );
	TASSERT( m_UnmanagedModels.IsEmpty() );
}

// $deBlob: FUNCTION 005702f0
AModelInstance2* AModelFactory::CreateInstance( const TCHAR* a_szFileName, TBOOL a_bCreateSoundBreakpoints, TBOOL a_bCreateAnimInterface )
{
	TString8 strFileName = GetFilename( a_szFileName );
	AModel2* pModel      = TNULL;

	for ( auto it = m_Models.Begin(); it != m_Models.End(); it++ )
	{
		if ( it->m_FileName.CompareNoCase( strFileName ) == 0 )
		{
			pModel = it;
			break;
		}
	}

	if ( pModel == TNULL )
	{
		for ( auto it = m_UnmanagedModels.Begin(); it != m_UnmanagedModels.End(); it++ )
		{
			if ( it->m_FileName.CompareNoCase( strFileName ) == 0 )
			{
				pModel = it;
				pModel->Remove();
				m_Models.PushBack( pModel );
				break;
			}
		}
	}

	if ( pModel == TNULL )
	{
		pModel = new AModel2();
		m_Models.PushBack( pModel );
		pModel->m_FileName = strFileName;
		pModel->Load();
	}

	return pModel->CreateInstance( a_bCreateSoundBreakpoints, 0, a_bCreateAnimInterface );
}

// $deBlob: FUNCTION 00570680
AModelInstance2* AModelFactory::CreateInstanceByName( TUINT a_uiUnused, const TCHAR* a_szFileName, TBOOL a_bCreateSoundBreakpoints, TBOOL a_bCreateAnimInterface )
{
	AModel2* pModel = TNULL;

	for ( auto it = m_Models.Begin(); it != m_Models.End(); it++ )
	{
		if ( it->m_FileName.CompareNoCase( a_szFileName ) == 0 )
		{
			pModel = it;
			break;
		}
	}

	if ( pModel == TNULL )
	{
		for ( auto it = m_UnmanagedModels.Begin(); it != m_UnmanagedModels.End(); it++ )
		{
			if ( it->m_FileName.CompareNoCase( a_szFileName ) == 0 )
			{
				pModel = it;
				pModel->Remove();
				m_Models.PushBack( pModel );
				break;
			}
		}
	}

	if ( pModel == TNULL )
	{
		pModel = new AModel2();
		m_Models.PushBack( pModel );
		pModel->m_FileName.Copy( a_szFileName );
		pModel->Load();
	}

	return pModel->CreateInstance( a_bCreateSoundBreakpoints, 0, a_bCreateAnimInterface );
}

// $deBlob: FUNCTION 00570960
AModelInstance2* AModelFactory::CreateUnmanagedInstance( const TCHAR* a_szFileName, TBOOL a_bCreateSoundBreakpoints, TUINT a_eType, TBOOL a_bCreateAnimInterface )
{
	TString8 strFileName = GetFilename( a_szFileName );
	AModel2* pModel      = TNULL;

	for ( auto it = m_UnmanagedModels.Begin(); it != m_UnmanagedModels.End(); it++ )
	{
		if ( it->m_FileName.CompareNoCase( strFileName ) == 0 )
		{
			pModel = it;
			break;
		}
	}

	if ( pModel == TNULL )
	{
		for ( auto it = m_Models.Begin(); it != m_Models.End(); it++ )
		{
			if ( it->m_FileName.CompareNoCase( strFileName ) == 0 )
			{
				pModel = it;
				break;
			}
		}
	}

	if ( pModel == TNULL )
	{
		pModel = new AModel2();
		m_UnmanagedModels.PushBack( pModel );
		pModel->m_FileName = strFileName;
		pModel->Load();
	}

	return pModel->CreateInstance( a_bCreateSoundBreakpoints, a_eType, a_bCreateAnimInterface );
}

// $deBlob: FUNCTION 00570cb0
void AModelFactory::DestroyInstance( AModelInstance2* a_pInstance )
{
	AModel2* pModel         = a_pInstance->GetModel();
	TBOOL    bFoundInstance = TFALSE;
	TBOOL    bFoundModel    = TFALSE;

	for ( auto it = m_Models.Begin(); it != m_Models.End(); it++ )
	{
		if ( pModel->m_FileName.CompareNoCase( it->m_FileName ) == 0 )
		{
			TASSERT( pModel->HasInstances() );
			bFoundModel = TTRUE;
			break;
		}
	}

	if ( !bFoundModel )
	{
		for ( auto it = m_UnmanagedModels.Begin(); it != m_UnmanagedModels.End(); it++ )
		{
			if ( pModel->m_FileName.CompareNoCase( it->m_FileName ) == 0 )
			{
				TASSERT( pModel->HasInstances() );
				bFoundModel = TTRUE;
				break;
			}
		}
	}

	if ( bFoundModel )
	{
		pModel->DestroyInstance( a_pInstance );

		if ( !pModel->HasInstances() )
		{
			pModel->Remove();
			delete pModel;
		}

		bFoundInstance = TTRUE;
	}

	TASSERT( bFoundInstance, "Model instance not found" );
}

// $deBlob: FUNCTION 00571300
void AModelFactory::Render()
{
	for ( auto it = m_Models.Begin(); it != m_Models.End(); it++ )
	{
		if ( it->IsRendered() )
			it->RenderAllInstances();
	}
}

// $deBlob: FUNCTION 00571420
void AModelFactory::RenderLoadingScreenInstances()
{
	for ( auto it = m_Models.Begin(); it != m_Models.End(); it++ )
		it->RenderLoadingScreenInstances();
}

// $deBlob: FUNCTION 00571880
TString8 AModelFactory::GetFilename( const TCHAR* a_szPath )
{
	TINT i = TStringManager::String8Length( a_szPath );

	while ( --i >= 0 )
	{
		if ( a_szPath[ i ] == '/' || a_szPath[ i ] == '\\' )
			return TString8( a_szPath + i + 1 );
	}

	return TString8( a_szPath );
}

// $deBlob: FUNCTION 00570fc0
TBOOL AModelFactory::HasModel( const TCHAR* a_szFileName )
{
	TString8 strFileName = GetFilename( a_szFileName );

	for ( auto it = m_Models.Begin(); it != m_Models.End(); it++ )
	{
		if ( it->m_FileName.CompareNoCase( strFileName ) == 0 )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00571160
TBOOL AModelFactory::HasUnmanagedModel( const TCHAR* a_szFileName )
{
	TString8 strFileName = GetFilename( a_szFileName );

	for ( auto it = m_UnmanagedModels.Begin(); it != m_UnmanagedModels.End(); it++ )
	{
		if ( it->m_FileName.CompareNoCase( strFileName ) == 0 )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005714d0
AModel2* AModelFactory::FindModel( const TCHAR* a_szFileName )
{
	TString8 strFileName = GetFilename( a_szFileName );

	for ( auto it = m_Models.Begin(); it != m_Models.End(); it++ )
	{
		if ( it->m_FileName.CompareNoCase( strFileName ) == 0 )
			return it;
	}

	return TNULL;
}

// $deBlob: FUNCTION 005716a0
AModel2* AModelFactory::FindUnmanagedModel( const TCHAR* a_szFileName )
{
	TString8 strFileName = GetFilename( a_szFileName );

	for ( auto it = m_UnmanagedModels.Begin(); it != m_UnmanagedModels.End(); it++ )
	{
		if ( it->m_FileName.CompareNoCase( strFileName ) == 0 )
			return it;
	}

	return TNULL;
}
