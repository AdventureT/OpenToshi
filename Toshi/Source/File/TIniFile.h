#pragma once
#include "Toshi/TString8.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: reads values of a Windows .ini file (GetPrivateProfileString). de Blob reads setup.ini with it.
//-----------------------------------------------------------------------------
class TIniFile
{
public:
	enum RESULT
	{
		RESULT_OK       = 0,
		RESULT_NOTFOUND = 2,
		RESULT_NOTBOOL  = 4, // ReadBool: neither "true" nor "false"
	};

public:
	TIniFile() {}

	// Sets the (native) path of the file, made absolute. Returns 0
	TINT Open( const TCHAR* a_szFileName );

	// Reads a number of a section, a_iDefault if missing (RESULT_NOTFOUND)
	TINT ReadLong( const TCHAR* a_szSection, const TCHAR* a_szKey, TINT* a_pOutNumber, TINT a_iDefault );

	// The string readers ignore a_szDefault
	TINT ReadString( const TCHAR* a_szSection, const TCHAR* a_szKey, TCHAR* a_szOutString, TUINT a_uiSize, const TCHAR* a_szDefault );
	TINT ReadString( const TCHAR* a_szSection, const TCHAR* a_szKey, TString8* a_pOutString, const TCHAR* a_szDefault );
	TINT ReadInt( const TCHAR* a_szSection, const TCHAR* a_szKey, TINT* a_pOutNumber, TINT a_iDefault );
	TINT ReadFloat( const TCHAR* a_szSection, const TCHAR* a_szKey, TFLOAT* a_pOutNumber, TFLOAT a_fDefault );
	TINT ReadBool( const TCHAR* a_szSection, const TCHAR* a_szKey, TBOOL* a_pOutBool, TBOOL a_bDefault );

private:
	TString8 m_strFileName; // 0x00
};

TOSHI_NAMESPACE_END
