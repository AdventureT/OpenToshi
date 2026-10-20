#include "pch.h"
#include "AConsoleVar.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005b0dd0
AConsoleVar::AConsoleVar( const TCHAR* a_szName, TFLOAT a_fValue )
    : m_eType( TYPE_FLOAT ), m_szName( a_szName )
{
	m_fValue = a_fValue;
}

// $deBlob: FUNCTION 005b0df0
AConsoleVar::AConsoleVar( const TCHAR* a_szName, TINT a_iValue )
    : m_eType( TYPE_INT ), m_iValue( a_iValue ), m_szName( a_szName )
{
}

// $deBlob: FUNCTION 005b0e10
AConsoleVar::AConsoleVar( const TCHAR* a_szName, TBOOL a_bValue )
    : m_eType( TYPE_BOOL ), m_bValue( a_bValue ), m_szName( a_szName )
{
}

// $deBlob: FUNCTION 005b0e30
AConsoleVar::AConsoleVar( const TCHAR* a_szName, const TCHAR* a_szValue )
    : m_eType( TYPE_STRING ), m_szName( a_szName )
{
	TStringManager::String8Copy( m_szValue, a_szValue, MAX_STRING );
}

AConsoleVar::~AConsoleVar()
{
}

// $deBlob: FUNCTION 005b0e70
void AConsoleVar::SetString( const TCHAR* a_szValue )
{
	TASSERT( TYPE_STRING == m_eType );
	TStringManager::String8Copy( m_szValue, a_szValue, MAX_STRING );
}
