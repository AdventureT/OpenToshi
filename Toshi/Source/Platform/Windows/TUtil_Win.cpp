#include "ToshiPCH.h"
#include "Toshi/TUtil.h"
#include "Toshi/T2String8.h"

#include <vector>
#include <algorithm>
#include <time.h>
#include <windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// A log file found by TrimLog (0x10C bytes)
struct TrimLogEntry
{
	// $deBlob: FUNCTION 0065dd20
	TrimLogEntry( const TCHAR* a_szName, TUINT32 a_uiTimeLow, TUINT32 a_uiTimeHigh )
	{
		uiTimeLow  = a_uiTimeLow;
		uiTimeHigh = a_uiTimeHigh;
		strcpy( szName, a_szName );
	}

	TUINT64 GetTime() const { return ( TUINT64( uiTimeHigh ) << 32 ) | uiTimeLow; }

	// $deBlob: FUNCTION 0065dd60
	TBOOL operator<( const TrimLogEntry& a_rcOther ) const { return GetTime() < a_rcOther.GetTime(); }

	TCHAR   szName[ MAX_PATH ]; // 0x000
	TUINT32 uiTimeLow;          // 0x104 creation time
	TUINT32 uiTimeHigh;         // 0x108
};

// $deBlob: FUNCTION 0065ddb0
static TBOOL TrimLogEntry_Less( const TrimLogEntry& a_rcLeft, const TrimLogEntry& a_rcRight )
{
	return a_rcLeft < a_rcRight;
}

// $deBlob: FUNCTION 0065db40
// Deletes the oldest (by creation time) files matching a_szFileName until a_uiTrimTo of them are left
void TUtil::TrimLog( const TCHAR* a_szFileName, TSIZE a_uiTrimTo )
{
	std::vector<TrimLogEntry> vecFiles;

	WIN32_FIND_DATAA oFindData;
	HANDLE           hFind = FindFirstFileA( a_szFileName, &oFindData );

	if ( hFind == INVALID_HANDLE_VALUE )
	{
		// NOTE: like retail, closes the invalid handle
		FindClose( hFind );
		return;
	}

	do
	{
		if ( oFindData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE )
			vecFiles.push_back( TrimLogEntry( oFindData.cFileName, oFindData.ftCreationTime.dwLowDateTime, oFindData.ftCreationTime.dwHighDateTime ) );

	} while ( FindNextFileA( hFind, &oFindData ) );

	FindClose( hFind );
	std::sort( vecFiles.begin(), vecFiles.end(), TrimLogEntry_Less );

	TINT iNumToDelete = TINT( vecFiles.size() ) - TINT( a_uiTrimTo );

	for ( TINT i = 0; i < iNumToDelete; i++ )
		DeleteFileA( vecFiles[ i ].szName );
}

// $deBlob: FUNCTION 00828b00
const TCHAR* TUtil::GetTime()
{
	static TCHAR s_szTime[ 32 ];

	__time64_t iTime;
	_time64( &iTime );

	T2String8::Copy( s_szTime, _ctime64( &iTime ), -1 );
	*T2String8::FindChar( s_szTime, '\n' ) = '\0';

	return s_szTime;
}

TOSHI_NAMESPACE_END
