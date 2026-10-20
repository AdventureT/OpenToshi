#include "ToshiPCH.h"
#include "Toshi/TDebug.h"
#include "Core/T2Mutex_Win.h"

#include <Windows.h>
#include <DbgHelp.h>
#include <TlHelp32.h>
#include <vector>

#pragma comment( lib, "dbghelp.lib" )

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TCHAR  TDebug_ScratchMem[ 4096 ];
BOOL   TDebug_IsMutexCreated;
T2MutexHandle TDebug_Mutex;

// $deBlob: FUNCTION 006fc2c0
void TDebug_AcquireMutex()
{
	if ( TDebug_IsMutexCreated == FALSE )
	{
		T2Mutex_Create( &TDebug_Mutex );
		TDebug_IsMutexCreated = TRUE;
	}

	T2Mutex_Lock( &TDebug_Mutex, 0 );
}

// $deBlob: FUNCTION 006fc2f0
void TDebug_ReleaseMutex()
{
	T2Mutex_Unlock( &TDebug_Mutex );
}

// $deBlob: FUNCTION 006fad90
TBOOL TDebug_IsValidAddress( const void* a_pPtr )
{
	if ( a_pPtr != TNULL &&
	     a_pPtr != (void*)0xcdcdcdcd &&
	     a_pPtr != (void*)0xdeaddead &&
	     a_pPtr != (void*)0xbeefbeef &&
	     a_pPtr >= (void*)0x50 )
	{
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006fbfe0
void TDebug_FinalPrintf( const TCHAR* a_szFormat, ... )
{
	va_list args;
	va_start( args, a_szFormat );
	TDebug_FinalVPrintf( a_szFormat, args );
	va_end( args );
}

// Prints only if a_bCondition is set
// $deBlob: FUNCTION 006fc000
void TDebug_FinalCPrintf( TINT a_iCondition, const TCHAR* a_szFormat, ... )
{
	if ( a_iCondition )
	{
		va_list args;
		va_start( args, a_szFormat );
		TDebug_FinalVPrintf( a_szFormat, args );
		va_end( args );
	}
}

// $deBlob: FUNCTION 006facf0
void TDebug_FinalVPrintf( const TCHAR* a_szFormat, va_list a_Args )
{
	TDebug_AcquireMutex();
	vsprintf( TDebug_ScratchMem, a_szFormat, a_Args );
	TDebug_OutputDebugString( TDebug_ScratchMem );
	TDebug_ReleaseMutex();
}

void TDebug_FinalSPrintf( const TCHAR* a_szFormat, ... )
{
	va_list args;
	va_start( args, a_szFormat );
	TDebug_FinalVSPrintf( a_szFormat, args );
	va_end( args );
}

void TDebug_FinalVSPrintf( const TCHAR* a_szFormat, va_list a_Args )
{
	TDebug_AcquireMutex();
	vsprintf( TDebug_ScratchMem, a_szFormat, a_Args );
	TDebug_ReleaseMutex();
}

// $deBlob: FUNCTION 006fad50
void TDebug_OutputDebugString( const TCHAR* a_szString )
{
	TDebug_AcquireMutex();
	OutputDebugStringA( a_szString );
	printf( "%s", a_szString );

	TDebug_BroadcastDebugString( a_szString );
	TDebug_ReleaseMutex();
}

// A slot of the debug string callbacks (a free or a used one)
struct TDebug_DebugStringCallbackNode
{
	TDebug_DebugStringCallbackNode* pNext;
	TDebug_DebugStringCallbackNode* pPrev;
	t_DebugStringCallback           fnCallback;
};

static TDebug_DebugStringCallbackNode s_aDebugStringCallbacks[ 8 ];
static TDebug_DebugStringCallbackNode s_oFreeDebugStringCallbacks; // the root of the free nodes
static TDebug_DebugStringCallbackNode s_oUsedDebugStringCallbacks; // the root of the used nodes
static BOOL                           s_bDebugStringCallbacksCreated;

// Moves a_pNode from its list to the end of the list of a_pRoot
static void TDebug_MoveDebugStringCallbackNode( TDebug_DebugStringCallbackNode* a_pNode, TDebug_DebugStringCallbackNode* a_pRoot )
{
	a_pNode->pNext->pPrev = a_pNode->pPrev;
	a_pNode->pPrev->pNext = a_pNode->pNext;

	TDebug_DebugStringCallbackNode* pLast = a_pRoot->pPrev;
	a_pNode->pPrev                        = pLast;
	a_pNode->pNext                        = a_pRoot;
	pLast->pNext                          = a_pNode;
	a_pNode->pNext->pPrev                 = a_pNode;
}

// $deBlob: FUNCTION 006fc020
TBOOL __stdcall TDebug_AddDebugStringCallback( t_DebugStringCallback a_fnCallback )
{
	TDebug_DebugStringCallbackNode* pNode;

	if ( s_bDebugStringCallbacksCreated )
	{
		pNode = s_oFreeDebugStringCallbacks.pNext;
	}
	else
	{
		// The 8 nodes go to the free list
		TDebug_DebugStringCallbackNode* pPrev = &s_oFreeDebugStringCallbacks;

		for ( TINT i = 0; i < 8; i++ )
		{
			pPrev->pNext                       = &s_aDebugStringCallbacks[ i ];
			s_aDebugStringCallbacks[ i ].pPrev = pPrev;
			pPrev                              = &s_aDebugStringCallbacks[ i ];
		}

		pPrev->pNext                          = &s_oFreeDebugStringCallbacks;
		s_oFreeDebugStringCallbacks.pPrev     = pPrev;
		s_oUsedDebugStringCallbacks.pNext     = &s_oUsedDebugStringCallbacks;
		s_oUsedDebugStringCallbacks.pPrev     = &s_oUsedDebugStringCallbacks;
		s_bDebugStringCallbacksCreated        = TTRUE;
		pNode                                 = &s_aDebugStringCallbacks[ 0 ];
	}

	if ( pNode == &s_oFreeDebugStringCallbacks )
		return TFALSE;

	TDebug_MoveDebugStringCallbackNode( pNode, &s_oUsedDebugStringCallbacks );
	pNode->fnCallback = a_fnCallback;
	return TTRUE;
}

// $deBlob: FUNCTION 006fc150
TBOOL __stdcall TDebug_RemoveDebugStringCallback( t_DebugStringCallback a_fnCallback )
{
	if ( !s_bDebugStringCallbacksCreated )
		return TFALSE;

	TDebug_DebugStringCallbackNode* pNode = s_oUsedDebugStringCallbacks.pNext;

	while ( pNode != &s_oUsedDebugStringCallbacks )
	{
		if ( pNode->fnCallback == a_fnCallback )
		{
			TDebug_MoveDebugStringCallbackNode( pNode, &s_oFreeDebugStringCallbacks );
			pNode->fnCallback = TNULL;
			return TTRUE;
		}

		pNode = pNode->pNext;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006fc280
void TDebug_BroadcastDebugString( const TCHAR* a_szString )
{
	if ( s_bDebugStringCallbacksCreated )
	{
		for ( TDebug_DebugStringCallbackNode* pNode = s_oUsedDebugStringCallbacks.pNext; pNode != &s_oUsedDebugStringCallbacks; pNode = pNode->pNext )
			pNode->fnCallback( a_szString );
	}
}

// $deBlob: FUNCTION 006fadc0
void TDebug_GetCallStack( TINT a_iMaxFrames, void** a_ppFrames )
{
	CONTEXT oContext;
	RtlCaptureContext( &oContext );
	GetCurrentProcess();
	GetCurrentThread();
	RtlCaptureStackBackTrace( 0, a_iMaxFrames, a_ppFrames, TNULL );
}

static BOOL s_bSymInitialised;

// $deBlob: FUNCTION 006fae10
void TDebug_DumpCallStack( TINT a_iNumFrames, void** a_ppFrames, TINT a_iFirstFrame )
{
	HANDLE hProcess = GetCurrentProcess();
	GetCurrentThread();

	DWORD   dwLineDisplacement = 0;
	DWORD64 dwSymDisplacement  = 0;
	TCHAR   szUndecorated[ 1024 ];

	IMAGEHLP_SYMBOL64 aSymbol[ 64 ];
	memset( aSymbol, 0, sizeof( aSymbol ) );
	aSymbol[ 0 ].SizeOfStruct  = sizeof( IMAGEHLP_SYMBOL64 );
	aSymbol[ 0 ].MaxNameLength = 1024;

	IMAGEHLP_LINE64 oLine;
	// NOTE: retail sets SizeOfStruct to 20 (the 32-bit IMAGEHLP_LINE size)
	oLine.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

	if ( s_bSymInitialised == FALSE )
	{
		s_bSymInitialised = TRUE;
		SymInitialize( hProcess, TNULL, TRUE );
	}

	// NOTE: retail resolves every frame but prints nothing (the output is compiled out)
	for ( TINT i = a_iFirstFrame; i < a_iNumFrames && *a_ppFrames != TNULL; i++, a_ppFrames++ )
	{
		SymGetSymFromAddr64( hProcess, DWORD64( TREINTERPRETCAST( uintptr_t, *a_ppFrames ) ), &dwSymDisplacement, aSymbol );
		UnDecorateSymbolName( aSymbol[ 0 ].Name, szUndecorated, 0x400, 0 );

		if ( !SymGetLineFromAddr64( hProcess, DWORD64( TREINTERPRETCAST( uintptr_t, *a_ppFrames ) ), &dwLineDisplacement, &oLine ) )
		{
			oLine.FileName   = (PCHAR) "unknown";
			oLine.LineNumber = 0;
		}
	}
}

#include "resource.h"

const TCHAR* TDebug_szAssertFilename;
const TCHAR* TDebug_szAssertExpression;
TUINT        TDebug_uiAssertLineNumber;

// The controls of the assert dialog
enum ASSERTDIALOG_ : TINT
{
	ASSERTDIALOG_QUIT       = 8200,
	ASSERTDIALOG_DEBUG      = 8201,
	ASSERTDIALOG_IGNORE     = 8202,
	ASSERTDIALOG_FILENAME   = 8204,
	ASSERTDIALOG_LINE       = 8205,
	ASSERTDIALOG_EXPRESSION = 8206,
};

BOOL CALLBACK TDebug_AssertionDlgProc( HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam )
{
	switch ( Message )
	{
		case WM_INITDIALOG:
		{
			ShowCursor( TRUE );
			SetActiveWindow( hwnd );
			SetForegroundWindow( hwnd );
			SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );

			if ( !TDebug_szAssertFilename )
				TDebug_szAssertFilename = "***INVALID***";

			if ( !TDebug_szAssertExpression )
				TDebug_szAssertExpression = "***INVALID***";

			SetDlgItemTextA( hwnd, ASSERTDIALOG_FILENAME, TDebug_szAssertFilename );
			SetDlgItemInt( hwnd, ASSERTDIALOG_LINE, TDebug_uiAssertLineNumber, TRUE );
			SetDlgItemTextA( hwnd, ASSERTDIALOG_EXPRESSION, TDebug_szAssertExpression );
			return TRUE;
		}
		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
				case ASSERTDIALOG_QUIT:
					ExitProcess( 1 );
					break;
				case ASSERTDIALOG_DEBUG:
					EndDialog( hwnd, 1 );
					break;
				case ASSERTDIALOG_IGNORE:
					ShowCursor( FALSE );
					EndDialog( hwnd, 0 );
					break;
			}
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

// The lock of TDebug_ShowAssertMessageBox, a static object
class TDebugAssertLock
{
public:
	// $deBlob: FUNCTION 006fb4d0
	TDebugAssertLock()
	{
		InitializeCriticalSection( &m_oCriticalSection );
		m_bCreated = TTRUE;
	}

	// $deBlob: FUNCTION 006fb520
	~TDebugAssertLock()
	{
		m_bCreated = TFALSE;
		DeleteCriticalSection( &m_oCriticalSection );
	}

public:
	CRITICAL_SECTION m_oCriticalSection;
	TBOOL            m_bCreated;
};

// $deBlob: FUNCTION 0053f9f0
// $deBlob: FUNCTION 008594d0
static TDebugAssertLock s_oAssertLock;

// Shows a_szText in an "Assert Failed" message box (abort/retry/ignore) with the other threads of the process
// suspended; returns the button (IDABORT, IDRETRY, IDIGNORE), 0 before the lock exists
// $deBlob: FUNCTION 006fafd0
static TINT TDebug_ShowAssertMessageBox( const TCHAR* a_szText )
{
	if ( !s_oAssertLock.m_bCreated )
		return 0;

	EnterCriticalSection( &s_oAssertLock.m_oCriticalSection );

	DWORD  uiThreadId  = GetCurrentThreadId();
	DWORD  uiProcessId = GetCurrentProcessId();
	HANDLE hSnapshot   = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );

	std::vector<HANDLE> vecThreads;
	vecThreads.reserve( 1024 );

	// NOTE: retail also holds the CRT heap lock (_lock( 4 )/_unlock( 4 )) while it suspends the threads; the UCRT
	// doesn't export it
	if ( hSnapshot != INVALID_HANDLE_VALUE )
	{
		THREADENTRY32 oEntry;
		oEntry.dwSize = sizeof( oEntry );

		if ( Thread32First( hSnapshot, &oEntry ) )
		{
			do
			{
				if ( oEntry.th32ThreadID != uiThreadId && oEntry.th32OwnerProcessID == uiProcessId && oEntry.dwSize >= 0x10 )
				{
					HANDLE hThread = OpenThread( THREAD_ALL_ACCESS, FALSE, oEntry.th32ThreadID );
					SuspendThread( hThread );
					vecThreads.push_back( hThread );
				}

				oEntry.dwSize = sizeof( oEntry );
			} while ( Thread32Next( hSnapshot, &oEntry ) );
		}

		CloseHandle( hSnapshot );
	}

	TINT iResult = MessageBoxA( NULL, a_szText, "Assert Failed", MB_SYSTEMMODAL | MB_ICONASTERISK | MB_ABORTRETRYIGNORE );

	for ( TSIZE i = 0; i < vecThreads.size(); i++ )
	{
		ResumeThread( vecThreads[ i ] );
		CloseHandle( vecThreads[ i ] );
	}

	LeaveCriticalSection( &s_oAssertLock.m_oCriticalSection );
	return iResult;
}

// A message box version of the assert dialog: 1 Debug (abort), 2 Ignore this time (retry), 0 Ignore Always
// $deBlob: FUNCTION 006fb140
static TINT TDebug_ShowAssertDialog( const TCHAR* a_szExpression, const TCHAR* a_szFileName, TINT a_iLine, const TCHAR* a_szFormat, ... )
{
	static TCHAR s_szDescription[ 1024 ];
	static TCHAR s_szText[ 2048 ];

	va_list args;
	va_start( args, a_szFormat );
	ShowCursor( TRUE );
	vsprintf( s_szDescription, a_szFormat, args );
	va_end( args );

	TINT iLength = sprintf( s_szText, "%s:\n%s\n\n", s_szDescription, a_szExpression );
	iLength += sprintf( s_szText + iLength, "%s:%d\n\n", a_szFileName, a_iLine );
	sprintf( s_szText + iLength, "Debug // Ignore this time // Ignore Always" );

	switch ( TDebug_ShowAssertMessageBox( s_szText ) )
	{
		case IDABORT: return 1;
		case IDRETRY: return 2;
		default: return 0;
	}
}

void __cdecl TDebug_AssertHandler( const TCHAR* a_szExpression, const TCHAR* a_szFileName, TUINT a_uiLineNumber, const TCHAR* a_szDescription, ... )
{
	TDebug_szAssertFilename   = a_szFileName;
	TDebug_szAssertExpression = a_szExpression;
	TDebug_uiAssertLineNumber = a_uiLineNumber;

	TERROR( "Assertion (%s) failed ('%s', at line %u)\n", a_szExpression, a_szFileName, a_uiLineNumber );

	// Let an attached debugger know about the assertion (it can capture the callstack at this point)
	if ( IsDebuggerPresent() )
	{
		TCHAR szDebugMessage[ 1024 ];
		T2String8::Format( szDebugMessage, sizeof( szDebugMessage ), "Assertion (%s) failed ('%s', at line %u)\n", a_szExpression, a_szFileName, a_uiLineNumber );
		OutputDebugStringA( szDebugMessage );
	}

	// Unattended runs (e.g. automated tests) can't answer the dialog, act as if Ignore was pressed
	if ( T2String8::FindString( GetCommandLineA(), "-noassertdialog" ) )
		return;

	HWND hWnd = GetActiveWindow();

	if ( hWnd != NULL )
		hWnd = GetLastActivePopup( hWnd );

	INT_PTR result = DialogBoxParamA(
	    GetModuleHandleA( NULL ),
	    MAKEINTRESOURCEA( IDD_ASSERT ),
	    hWnd,
	    TDebug_AssertionDlgProc,
	    NULL
	);

	// Debug (or the dialog failed): break here, the caller is the assert site
	if ( result == -1 || result == 1 )
		TBREAK();
}
