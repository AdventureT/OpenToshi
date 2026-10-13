#include "ToshiPCH.h"
#include "TNativeFile_Win.h"
#include "File/TNullFile.h"

#include <windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

#ifdef DEBLOB_COMMUNITY_PATCH

//-----------------------------------------------------------------------------
// Purpose: -gamedir <dir> (dev switch). The retail data is read from <dir> while
// the working directory stays the place where everything is written (logs,
// setup.ini...). The "gamedir" file system follows "local" in the system path,
// so reads fall back to it, and it refuses anything that would write.
//-----------------------------------------------------------------------------
class TReadOnlyNativeFileSystem : public TNativeFileSystem
{
public:
	TReadOnlyNativeFileSystem( const TCHAR* a_szName )
	    : TNativeFileSystem( a_szName )
	{}

	virtual TFile* CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode ) OVERRIDE
	{
		if ( a_eFileMode & ( TFILEMODE_WRITE | TFILEMODE_READWRITE | TFILEMODE_CREATENEW ) )
			return TNULL;

		return TNativeFileSystem::CreateFile( a_rcFileName, a_eFileMode );
	}

	virtual TBOOL RemoveFile( const TString8& a_rcFileName ) OVERRIDE { return TFALSE; }
	virtual TBOOL MakeDirectory( const TString8& a_rcDirectory ) OVERRIDE { return TFALSE; }
};

// Returns TTRUE and the folder if the command line has -gamedir <dir> (the folder may be quoted)
static TBOOL GetGameDirFromCommandLine( TCHAR* a_pOutDir, TINT a_iMaxLength )
{
	const TCHAR* pszCmdLine = GetCommandLineA();
	const TCHAR* pszSwitch  = T2String8::FindString( pszCmdLine, "-gamedir" );

	if ( !pszSwitch ) return TFALSE;

	const TCHAR* pchCursor = pszSwitch + 8;
	while ( *pchCursor == ' ' || *pchCursor == '\t' ) pchCursor++;

	TINT iLength = 0;

	if ( *pchCursor == '"' )
	{
		pchCursor++;

		while ( *pchCursor != '\0' && *pchCursor != '"' && iLength < a_iMaxLength - 1 )
			a_pOutDir[ iLength++ ] = *( pchCursor++ );
	}
	else
	{
		// Unquoted (the test runner doesn't quote it): the folder may contain spaces, so it ends at the next switch
		while ( *pchCursor != '\0' && !( pchCursor[ 0 ] == ' ' && pchCursor[ 1 ] == '-' ) && iLength < a_iMaxLength - 1 )
			a_pOutDir[ iLength++ ] = *( pchCursor++ );

		while ( iLength > 0 && ( a_pOutDir[ iLength - 1 ] == ' ' || a_pOutDir[ iLength - 1 ] == '\t' ) )
			iLength--;
	}

	a_pOutDir[ iLength ] = '\0';
	return iLength > 0;
}

#endif // DEBLOB_COMMUNITY_PATCH

// $deBlob: FUNCTION 00687430
// $Barnyard: FUNCTION 006d90f0
TBOOL TFileManager::Create()
{
	TASSERT( TFALSE == IsSingletonCreated() );
	CreateCommon();

	// NOTE: de Blob resolves every relative data path through the "local" file system: its prefix is the process'
	// current directory, so the retail game reads Data\... next to where it was started from.
	TString8 strCurrentDir;
	CHAR     szCurrentDir[ MAX_PATH + 1 ];
	TUtil::MemClear( szCurrentDir, sizeof( szCurrentDir ) );
	DWORD iCharCount = GetCurrentDirectoryA( MAX_PATH, szCurrentDir );
	TASSERT( iCharCount );
	strCurrentDir = szCurrentDir;

	( new TNativeFileSystem( "local" ) )->SetPrefix( strCurrentDir );
	( new TNativeFileSystem( "abs" ) )->SetPrefix( "" );
	new TNullFileSystem( "null" );

#ifdef DEBLOB_COMMUNITY_PATCH
	TCHAR szGameDir[ MAX_PATH + 1 ];

	if ( GetGameDirFromCommandLine( szGameDir, TARRAYSIZE( szGameDir ) ) )
	{
		( new TReadOnlyNativeFileSystem( "gamedir" ) )->SetPrefix( szGameDir );
		TFileManager::GetSingletonSafe()->SetSystemPath( "local;gamedir" );
		OutputDebugStringA( "TFileManager::Create - reading the game data from -gamedir\n" );

		return TTRUE;
	}
#endif // DEBLOB_COMMUNITY_PATCH

	TFileManager::GetSingletonSafe()->SetSystemPath( "local" );

	return TTRUE;
}

TOSHI_NAMESPACE_END
