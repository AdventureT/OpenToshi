#include "ToshiPCH.h"
#include "TStringTable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00828ba0
TINT TStringTable::GetStringID( const TCHAR* a_pString )
{
	TASSERT( T2String8::IsLowerCase( a_pString ) );

	// NOTE: like retail, the indices are 16-bit
	TINT16 iHigh  = TINT16( m_iNumStrings );
	TINT16 iLow   = 0;
	TINT16 iIndex = iHigh >> 1;

	if ( iIndex >= 0 )
	{
		while ( iIndex < iHigh )
		{
			TINT iResult = T2String8::Compare( a_pString, m_pStrings[ iIndex ], -1 );

			if ( iResult == 0 )
				return iIndex;

			if ( iResult >= 0 )
				iLow = iIndex + 1;
			else
				iHigh = iIndex;

			iIndex = iLow + ( ( iHigh - iLow ) >> 1 );

			if ( iLow > iIndex )
				return -1;
		}
	}

	return -1;
}

// $deBlob: FUNCTION 00828c60
const TCHAR* TStringTable::GetStringFromID( TINT a_ID )
{
	TASSERT( a_ID >= 0 && a_ID < m_iNumStrings );
	return m_pStrings[ a_ID ];
}

// $deBlob: FUNCTION 00828cc0
void TStringTable::CheckIfDataValid()
{
	TASSERT( m_iNumStrings > 0 );

	for ( TINT i = 0; i < m_iNumStrings; i++ )
		TASSERT( T2String8::IsLowerCase( m_pStrings[ i ] ) );

	for ( TINT i = 0; i < m_iNumStrings - 1; i++ )
		TASSERT( T2String8::Compare( m_pStrings[ i ], m_pStrings[ i + 1 ] ) < 0 );
}

TOSHI_NAMESPACE_END
