#include "ToshiPCH.h"
#include "TXUIStringTable.h"
#include "TXUI.h"

#include "File/TFile.h"
#include "File/TTRB.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006eb0d0
TXUIStringTable::TXUIStringTable()
{
	m_oXUSHeader.m_uiFileID     = 0;
	m_oXUSHeader.m_uiNumStrings = 0;
	m_oXUSHeader.m_uiVersion1   = 0;
	m_oXUSHeader.m_uiVersion2   = 0;
	m_oXUSHeader.m_uiSize       = 0;
	m_aStringTable              = TNULL;
}

// $deBlob: FUNCTION 006eb0f0
// $deBlob: FUNCTION 006d9c90
TXUIStringTable::~TXUIStringTable()
{
	for ( TINT i = 0; i < m_oXUSHeader.m_uiNumStrings; i++ )
	{
		TFree( m_aStringTable[ i ].m_wszID );
		TFree( m_aStringTable[ i ].m_wszString );
	}

	TFree( m_aStringTable );
}

// $deBlob: FUNCTION 006eb130
TBOOL TXUIStringTable::Load( const TCHAR* a_szFileName, TBOOL a_bTRB, TTRB* a_pTRB )
{
	if ( TStringManager::String8FindString( a_szFileName, a_bTRB ? ".trb" : ".xus" ) == TNULL )
		return TFALSE;

	for ( TINT i = 0; i < m_oXUSHeader.m_uiNumStrings; i++ )
	{
		TFree( m_aStringTable[ i ].m_wszID );
		TFree( m_aStringTable[ i ].m_wszString );
	}

	TFree( m_aStringTable );

	TTRB    oTRB;
	TUINT8* pData = TNULL;

	if ( a_bTRB == TFALSE )
	{
		TFile* pFile = TFileManager::GetSingleton()->CreateFile( a_szFileName, TFILEMODE_READ );

		if ( pFile == TNULL )
		{
			oTRB.Close();
			return TFALSE;
		}

		TUINT32 uiSize = pFile->GetSize();
		pData          = TSTATICCAST( TUINT8, TMalloc( uiSize ) );
		pFile->Read( pData, uiSize );
		pFile->Destroy();
	}
	else
	{
		// The symbol of the string table is "<name>_<name>" in a_pTRB or "<name>" in a_szFileName's .trb
		TINT iNameStart = 0;
		TINT iLength    = TStringManager::String8Length( a_szFileName );

		for ( TINT i = iLength - 1; i >= 0; i-- )
		{
			if ( a_szFileName[ i ] == '/' || a_szFileName[ i ] == '\\' )
			{
				iNameStart = i + 1;
				break;
			}
		}

		TINT iFileNameLen = iLength - 4 - iNameStart;
		TASSERT( ( ( iFileNameLen << 1 ) + 5 ) < 128 );

		TCHAR szSymbol[ 128 ];
		TStringManager::String8Copy( szSymbol, a_szFileName + iNameStart, iFileNameLen );
		szSymbol[ iFileNameLen ]     = '_';
		szSymbol[ iFileNameLen + 1 ] = '\0';
		TStringManager::String8Copy( szSymbol + iFileNameLen + 1, a_szFileName + iNameStart, iFileNameLen );
		szSymbol[ iFileNameLen * 2 + 1 ] = '\0';
		TStringManager::String8ToLowerCase( szSymbol );

		if ( a_pTRB != TNULL )
			pData = TSTATICCAST( TUINT8, a_pTRB->GetSymbolAddress( szSymbol ) );

		if ( pData == TNULL )
		{
			// NOTE: copies 4 characters more than the name, like TXUIResource::Load does for "_xur" (only zeros here)
			TStringManager::String8Copy( szSymbol, szSymbol + iFileNameLen + 1, iFileNameLen + 4 );
			szSymbol[ iFileNameLen + 4 ] = '\0';

			if ( oTRB.Load( a_szFileName ) != TTRB::ERROR_OK || ( pData = TSTATICCAST( TUINT8, oTRB.GetSymbolAddress( szSymbol ) ) ) == TNULL )
			{
				oTRB.Close();
				return TFALSE;
			}
		}
	}

	if ( ReadHeader( pData ) )
	{
		ReadStringTable( pData + 12 );

		if ( a_bTRB == TFALSE )
			TFree( pData );

		oTRB.Close();
		return TTRUE;
	}

	oTRB.Close();
	return TFALSE;
}

// $deBlob: FUNCTION 006eb480
TBOOL TXUIStringTable::ReadHeader( TUINT8* a_pData )
{
	m_oXUSHeader.m_uiFileID = PARSEDWORD_BIG( a_pData );
	TASSERT( m_oXUSHeader.m_uiFileID == IDXUS );

	m_oXUSHeader.m_uiVersion1   = a_pData[ 4 ];
	m_oXUSHeader.m_uiVersion2   = a_pData[ 5 ];
	m_oXUSHeader.m_uiSize       = PARSEDWORD_BIG( a_pData + 6 );
	m_oXUSHeader.m_uiNumStrings = PARSEWORD_BIG( a_pData + 10 );

	return TTRUE;
}

// Copies a big endian string of the .xus into a new buffer
static TWCHAR* TXUIStringTable_ReadString( TUINT8*& a_rData )
{
	TUINT16 uiLength = PARSEWORD_BIG( a_rData );
	TWCHAR* wszString = TSTATICCAST( TWCHAR, TMalloc( uiLength * sizeof( TWCHAR ) + sizeof( TWCHAR ) ) );
	TStringManager::String16Copy( wszString, TREINTERPRETCAST( TWCHAR*, a_rData + 2 ), uiLength );

	for ( TUINT32 i = 0; i < uiLength; i++ )
		wszString[ i ] = PARSEWORD_BIG( TREINTERPRETCAST( TUINT8*, &wszString[ i ] ) );

	wszString[ uiLength ] = L'\0';
	a_rData += ( uiLength + 1 ) * sizeof( TWCHAR );

	return wszString;
}

// $deBlob: FUNCTION 006eb530
TBOOL TXUIStringTable::ReadStringTable( TUINT8* a_pData )
{
	TASSERT( m_oXUSHeader.m_uiFileID == IDXUS );
	TASSERT( TNULL == m_aStringTable );

	m_aStringTable = TSTATICCAST( XUSString, TMalloc( m_oXUSHeader.m_uiNumStrings * sizeof( XUSString ), TXUI::ms_pXUIMemoryBlock ) );

	for ( TINT i = 0; i < m_oXUSHeader.m_uiNumStrings; i++ )
	{
		m_aStringTable[ i ].m_wszString = TXUIStringTable_ReadString( a_pData );
		m_aStringTable[ i ].m_wszID     = TXUIStringTable_ReadString( a_pData );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006eb430
const TWCHAR* TXUIStringTable::Lookup( const TWCHAR* a_wszID )
{
	for ( TINT i = 0; i < m_oXUSHeader.m_uiNumStrings; i++ )
	{
		if ( TStringManager::String16Compare( a_wszID, m_aStringTable[ i ].m_wszID, -1 ) == 0 )
			return m_aStringTable[ i ].m_wszString;
	}

	return TNULL;
}

TOSHI_NAMESPACE_END
