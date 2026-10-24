#include "pch.h"
#include "ATRBResource.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0078ce10
void ATRBResource::Reload()
{
	if ( m_bLoaded )
		m_oTRB.Close();

	// NOTE: retail's file name is the path + "/" + the extension, without the resource's name
	TString8 strFileName = m_strPath;
	strFileName += "/";
	strFileName += GetExtension();

	TTRB::ERROR eRes = m_oTRB.Load( strFileName.GetString() );
	TASSERT( TTRB::ERROR_OK == eRes );

	m_bLoaded = TTRUE;
	m_pMain   = m_oTRB.GetSymbolAddress( "Main" );
}

// $deBlob: FUNCTION 0078cde0
void* ATRBResource::GetTRBData( const TCHAR* a_szSymbol )
{
	if ( !m_bLoaded )
		return TNULL;

	void* pData = m_oTRB.GetSymbolAddress( a_szSymbol );

	if ( pData == TNULL )
		return m_pMain;

	return pData;
}
