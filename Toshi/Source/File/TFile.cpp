#include "ToshiPCH.h"
#include "TFile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00686370
// $Barnyard: FUNCTION 006bddc0
TFile::TFile( TFileSystem* pFS )
{
	m_pFileSystem = pFS;
}

TFile::TFile( const TFile& other )
{
	m_pFileSystem = other.GetFileSystem();
}

// $deBlob: FUNCTION 00686390
// $Barnyard: FUNCTION 006bddf0
TFile::~TFile()
{
}

// $deBlob: FUNCTION 00685fd0
// $Barnyard: FUNCTION 006bde00
TINT TFile::CPrintf( const TCHAR* a_szFormat, ... )
{
	va_list args;
	va_start( args, a_szFormat );
	TINT iResult = VCPrintf( a_szFormat, args );
	va_end( args );

	return iResult;
}

// $deBlob: FUNCTION 00685e40
// $Barnyard: FUNCTION 006be700
TFile* TFile::Create( const TString8& filename, TFILEMODE mode )
{
	TPROFILER_SCOPE();

	TFile* pFile = TFileManager::GetSingletonSafe()->CreateFile( filename, mode );

	// NOTE: de Blob only traces while a global trace flag (byte 009a557c) is set
#ifndef TOSHI_FINAL
	if ( pFile != TNULL )
	{
		TTRACE( "TFile::Create - Created file %s with mode %d\n", filename.GetString(), mode );
	}
	else
	{
		TTRACE( "TFile::Create - Failed to create file %s with mode %d\n", filename.GetString(), mode );
	}
#endif

	return pFile;
}

// $deBlob: FUNCTION 00685f20
void TFile::Destroy( TFile* a_pFile )
{
	if ( a_pFile )
		a_pFile->GetFileSystem()->DestroyFile( a_pFile );
}

// $deBlob: FUNCTION 00685f40
TBOOL TFile::ToNative( const TString8& a_rcFileName, TNativeFileInfo& a_rNativeInfo )
{
	return TFileManager::GetSingleton()->ToNative( a_rcFileName, a_rNativeInfo );
}

// NOTE: does nothing in de Blob (the trace of TFile::Create has its own flag)
// $deBlob: FUNCTION 00685f50
void TFile::PrintFileAccess( TBOOL a_bPrint )
{
}

// $deBlob: FUNCTION 00685f70
TINT TFile::ReadLine( TCHAR* a_pBuffer, TINT a_iSize )
{
	TCHAR* pEnd    = a_pBuffer + a_iSize;
	TCHAR* pCursor = a_pBuffer;
	TCHAR  cChar;

	while ( Read( &cChar, 1 ) != 0 )
	{
		if ( cChar == '\n' || cChar == '\r' )
		{
			// Skip empty lines
			if ( pCursor != a_pBuffer )
				break;
		}
		else
		{
			*( pCursor++ ) = cChar;

			// NOTE: retail writes the terminator at a_pBuffer[ a_iSize ] (one past the given size)
			if ( pCursor == pEnd )
				break;
		}
	}

	*pCursor = '\0';
	return TINT( pCursor - a_pBuffer );
}

// $deBlob: FUNCTION 006861d0
// $Barnyard: FUNCTION 006bdff0
TString8 TFile::ConcatPath( const TString8& a_rcPath1, const TString8& a_rcPath2 )
{
	TString8 strResult;

	TINT iPath1StartPos = 0;
	TINT iPath2StartPos = 0;

	// Copy filesystem/volume name from a_rcPath1 or a_rcPath2
	// prioritizing the one in a_rcPath2
	TINT iColonPos = a_rcPath2.FindReverse( ':' );

	if ( iColonPos == -1 )
	{
		iColonPos = a_rcPath1.FindReverse( ':' );

		if ( iColonPos > -1 )
		{
			// Copy volume/filesystem name from the first path
			iPath1StartPos = iColonPos + 1;
			strResult.Concat( a_rcPath1, iPath1StartPos );
		}
	}
	else
	{
		// Copy volume/filesystem name from the second path
		iPath2StartPos = iColonPos + 1;
		strResult.Concat( a_rcPath2, iPath2StartPos );

		// Ignore everything before the ':' character in the first path
		iPath1StartPos = a_rcPath1.FindReverse( ':' ) + 1;
	}

	TINT iPath1Length = a_rcPath1.Length();
	TINT iPath2Length = a_rcPath2.Length();

	if ( iPath2StartPos < iPath2Length &&
	     ( a_rcPath2[ iPath2StartPos ] == '/' || a_rcPath2[ iPath2StartPos ] == '\\' ) )
	{
		// The second path starts from a filesystem/volume root so ignore the first path
		strResult += a_rcPath2.GetString( iPath2StartPos );
	}
	else
	{
		// Concat the first path with the second one trimming parts like 'C:' or 'native:' and etc.
		strResult += a_rcPath1.GetString( iPath1StartPos );
		if ( iPath1StartPos < iPath1Length && iPath2StartPos < iPath2Length &&
		     a_rcPath1[ iPath1Length - 1 ] != '/' && a_rcPath1[ iPath1Length - 1 ] != '\\' )
		{
			strResult += "/";
		}

		strResult += a_rcPath2.GetString( iPath2StartPos );
	}

	return SimplifyPath( strResult );
}

// $deBlob: FUNCTION 00686010
// $Barnyard: FUNCTION 006bde20
TString8 TFile::SimplifyPath( const TString8& a_rcPath )
{
	TString8 strPath = a_rcPath;
	FixPathSlashesBack( strPath );

	TINT     iPos = 0;
	TString8 strResult;

	// Keep filesystem/volume/protocol name in the simplified path and leave it as it is
	TINT iColonPos = strPath.FindReverse( ':' );
	if ( iColonPos > -1 )
	{
		iPos = iColonPos + 1;
		strResult.Concat( strPath, iPos );
	}

	if ( strPath[ iPos ] == '/' )
	{
		iPos += 1;
		strResult += "/";
	}

	TINT iInitialResultLength = strResult.Length();
	TINT iResultLength        = iInitialResultLength;

	strPath += "/";
	TINT iSlashPos = strPath.Find( '/', iPos );

	do
	{
		// Is the path over?
		if ( iSlashPos < 0 )
		{
			if ( iInitialResultLength < strResult.Length() )
			{
				strResult.Truncate( strResult.Length() - 1 );
			}

			return strResult;
		}

		if ( !strncmp( strPath.GetString( iPos ), "../", 3 ) )
		{
			if ( iResultLength == strResult.Length() )
			{
				// Can't go back to a previous dir since it's the only one
				strResult += "../";
				iResultLength += 3;
			}
			else
			{
				// Truncate the path removing the current dir
				TINT iTruncateTo;

				if ( strResult.Length() > 1 && ( iTruncateTo = strResult.FindReverse( '/', strResult.Length() - 2 ), iResultLength <= iTruncateTo ) )
				{
					strResult.Truncate( iTruncateTo + 1 );
				}
				else
				{
					strResult.Truncate( iResultLength );
				}
			}
		}
		// Add anything except './'
		else if ( strncmp( strPath.GetString( iPos ), "./", 2 ) != 0 )
		{
			strResult.Concat( strPath.GetString( iPos ), ( iSlashPos - iPos ) + 1 );
		}

		iPos      = iSlashPos + 1;
		iSlashPos = strPath.Find( '/', iPos );

	} while ( TTRUE );
}

// $deBlob: FUNCTION 00686300
void TFile::SplitPath( const TString8& a_rcPath, TString8& a_rDirectory, TString8& a_rFileName )
{
	for ( TINT i = a_rcPath.Length() - 1; i >= 0; i-- )
	{
		const TCHAR* pChar = a_rcPath.GetString( i );

		if ( *pChar == ':' || *pChar == '/' || *pChar == '\\' )
		{
			a_rDirectory.Copy( a_rcPath, i + 1 );
			a_rFileName.Copy( pChar + 1 );
			return;
		}
	}

	a_rDirectory.Copy( "" );
	a_rFileName = a_rcPath;
}

// $deBlob: FUNCTION 00685f60
// $Barnyard: FUNCTION 006be310
// $Barnyard: FUNCTION 006bdcf0
void TFile::Destroy()
{
	// NOTE: retail checks this != TNULL
	TVALIDPTR( m_pFileSystem );
	m_pFileSystem->DestroyFile( this );
}

TOSHI_NAMESPACE_END
