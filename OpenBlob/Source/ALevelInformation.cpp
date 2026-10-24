#include "pch.h"
#include "ALevelInformation.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ALevelInformation* ALevelInformation::sm_singleton;

// $deBlob: FUNCTION 0056da50
ALevelInformation::ALevelInformation()
{
	m_pData = TNULL;

	TASSERT( !sm_singleton );
	sm_singleton = this;
}

// $deBlob: FUNCTION 0056daf0
// $deBlob: FUNCTION 0056c0a0
ALevelInformation::~ALevelInformation()
{
	m_oHashTable.Destroy();

	TASSERT( sm_singleton == this );
	sm_singleton = TNULL;
}

// $deBlob: FUNCTION 0056dba0
void ALevelInformation::Create( const TCHAR* a_szFileName )
{
	m_pData = TNULL;
	Close();

	if ( Load( a_szFileName ) == ERROR_OK )
		m_pData = TSTATICCAST( LevelHeader, GetSymbolAddress( "LevelsMain" ) );

	MakeStringToIDHashTable();
}

// $deBlob: FUNCTION 0056dbf0
TINT ALevelInformation::GetLevelIndex( const LevelProperties* a_pProperties )
{
	for ( TINT i = 0; i < m_pData->m_levelCount; i++ )
	{
		if ( &m_pData->m_pLevelProps[ i ] == a_pProperties )
			return i;
	}

	return -1;
}

// $deBlob: FUNCTION 0056dc20
TINT ALevelInformation::GetLevelIndex( const TCHAR* a_szName )
{
	TCHAR szName[ 128 ];
	TStringManager::String8Copy( szName, a_szName, 128 );

	THashTable::HashNode* pNode = m_oHashTable.Find( szName );

	if ( pNode != TNULL )
		return pNode->iItemIndex;

	return -1;
}

// $deBlob: FUNCTION 0056dc90
ALevelInformation::LevelProperties* ALevelInformation::GetLevelProperties( TINT a_iLevelIndex )
{
	TASSERT( a_iLevelIndex >= 0 && a_iLevelIndex < GetLevelHeader()->m_levelCount );
	return &GetLevelHeader()->m_pLevelProps[ a_iLevelIndex ];
}

// $deBlob: FUNCTION 0056dce0
ALevelInformation::LevelProperties* ALevelInformation::FindLevelProperties( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < m_pData->m_levelCount; i++ )
	{
		LevelProperties* pProperties = &m_pData->m_pLevelProps[ i ];

		// The name is compared up to its extension
		const TCHAR* pExtension = T2String8::FindString( pProperties->m_szName, "." );
		TINT         iLength    = -1;

		if ( pExtension != TNULL )
			iLength = TINT( T2String8::Length( pProperties->m_szName ) - T2String8::Length( pExtension ) );

		if ( T2String8::CompareNoCase( a_szName, pProperties->m_szName, iLength ) == 0 )
			return pProperties;
	}

	return TNULL;
}

// $deBlob: FUNCTION 0056dd70
void ALevelInformation::MakeStringToIDHashTable()
{
	TINT iLevelCount = m_pData->m_levelCount;
	m_oHashTable.Create( iLevelCount, 128, iLevelCount, iLevelCount );
	m_oHashTable.SetItemCompareFunction( LevelNameCompare );

	for ( TINT i = 0; i < m_pData->m_levelCount; i++ )
	{
		TCHAR szName[ 128 ];
		TStringManager::String8Copy( szName, m_pData->m_pLevelProps[ i ].m_szName, 128 );
		m_oHashTable.Insert( szName );
	}
}

// $deBlob: FUNCTION 0056de00
TBOOL ALevelInformation::LevelNameCompare( void* a_pMem1, void* a_pMem2, TINT a_iSize )
{
	if ( TStringManager::String8Compare( TSTATICCAST( TCHAR, a_pMem1 ), TSTATICCAST( TCHAR, a_pMem2 ) ) == 0 )
		return TTRUE;

	return TFALSE;
}
