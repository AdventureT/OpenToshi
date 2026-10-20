#include "ToshiPCH.h"
#include "TIniFile.h"

#ifdef TOSHI_SKU_WINDOWS
#  include <windows.h>
#endif // TOSHI_SKU_WINDOWS

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

#ifdef TOSHI_SKU_WINDOWS

// File/TLogFile.cpp
TCHAR* CanonicalizePath( const TCHAR* a_szPath, const TCHAR* a_szBase );

// $deBlob: FUNCTION 006f3370
TINT TIniFile::Open( const TCHAR* a_szFileName )
{
	m_strFileName = CanonicalizePath( a_szFileName, "" );
	return RESULT_OK;
}

// $deBlob: FUNCTION 006f33a0
TINT TIniFile::ReadString( const TCHAR* a_szSection, const TCHAR* a_szKey, TCHAR* a_szOutString, TUINT a_uiSize, const TCHAR* a_szDefault )
{
	return ( GetPrivateProfileStringA( a_szSection, a_szKey, "", a_szOutString, a_uiSize, m_strFileName.GetString() ) != 0 ) ? RESULT_OK : RESULT_NOTFOUND;
}

// $deBlob: FUNCTION 006f3410
TINT TIniFile::ReadString( const TCHAR* a_szSection, const TCHAR* a_szKey, TString8* a_pOutString, const TCHAR* a_szDefault )
{
	TCHAR szValue[ 1024 ];

	if ( GetPrivateProfileStringA( a_szSection, a_szKey, "", szValue, sizeof( szValue ), m_strFileName.GetString() ) != 0 )
	{
		if ( a_pOutString != TNULL )
			a_pOutString->Copy( szValue, -1 );

		return RESULT_OK;
	}

	return RESULT_NOTFOUND;
}

// $deBlob: FUNCTION 006f34e0
TINT TIniFile::ReadLong( const TCHAR* a_szSection, const TCHAR* a_szKey, TINT* a_pOutNumber, TINT a_iDefault )
{
	TCHAR szValue[ 1024 ];

	*a_pOutNumber = a_iDefault;

	if ( GetPrivateProfileStringA( a_szSection, a_szKey, "", szValue, sizeof( szValue ), m_strFileName.GetString() ) != 0 )
	{
		*a_pOutNumber = atoi( szValue );
		return RESULT_OK;
	}

	return RESULT_NOTFOUND;
}

// $deBlob: FUNCTION 006f35b0
TINT TIniFile::ReadInt( const TCHAR* a_szSection, const TCHAR* a_szKey, TINT* a_pOutNumber, TINT a_iDefault )
{
	TCHAR szValue[ 1024 ];

	*a_pOutNumber = a_iDefault;

	if ( GetPrivateProfileStringA( a_szSection, a_szKey, "", szValue, sizeof( szValue ), m_strFileName.GetString() ) != 0 )
	{
		*a_pOutNumber = atoi( szValue );
		return RESULT_OK;
	}

	return RESULT_NOTFOUND;
}

// $deBlob: FUNCTION 006f3680
TINT TIniFile::ReadFloat( const TCHAR* a_szSection, const TCHAR* a_szKey, TFLOAT* a_pOutNumber, TFLOAT a_fDefault )
{
	TCHAR szValue[ 1024 ];

	*a_pOutNumber = a_fDefault;

	if ( GetPrivateProfileStringA( a_szSection, a_szKey, "", szValue, sizeof( szValue ), m_strFileName.GetString() ) != 0 )
	{
		*a_pOutNumber = TFLOAT( atof( szValue ) );
		return RESULT_OK;
	}

	return RESULT_NOTFOUND;
}

// $deBlob: FUNCTION 006f3750
TINT TIniFile::ReadBool( const TCHAR* a_szSection, const TCHAR* a_szKey, TBOOL* a_pOutBool, TBOOL a_bDefault )
{
	TCHAR szValue[ 1024 ];

	*a_pOutBool = a_bDefault;

	if ( GetPrivateProfileStringA( a_szSection, a_szKey, "", szValue, sizeof( szValue ), m_strFileName.GetString() ) != 0 )
	{
		if ( TStringManager::String8CompareNoCase( "true", szValue, -1 ) == 0 )
		{
			*a_pOutBool = TTRUE;
			return RESULT_OK;
		}

		if ( TStringManager::String8CompareNoCase( "false", szValue, -1 ) == 0 )
		{
			*a_pOutBool = TFALSE;
			return RESULT_OK;
		}

		return RESULT_NOTBOOL;
	}

	return RESULT_NOTFOUND;
}

#endif // TOSHI_SKU_WINDOWS

TOSHI_NAMESPACE_END
