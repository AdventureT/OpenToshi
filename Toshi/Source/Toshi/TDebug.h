#pragma once
#include "Toshi/Typedefs.h"

TBOOL TDebug_IsValidAddress( const void* a_pPtr );

void TDebug_AcquireMutex();
void TDebug_ReleaseMutex();

void TDebug_FinalPrintf( const TCHAR* a_szFormat, ... );
void TDebug_FinalVPrintf( const TCHAR* a_szFormat, va_list a_Args );
void TDebug_FinalCPrintf( TINT a_iCondition, const TCHAR* a_szFormat, ... );

void TDebug_FinalSPrintf( const TCHAR* a_szFormat, ... );
void TDebug_FinalVSPrintf( const TCHAR* a_szFormat, va_list a_Args );

void TDebug_OutputDebugString( const TCHAR* a_szString );
void TDebug_BroadcastDebugString( const TCHAR* a_szString );

// The callbacks TDebug_BroadcastDebugString passes the debug output to (at most 8)
typedef void( __stdcall* t_DebugStringCallback )( const TCHAR* a_szString );
TBOOL __stdcall TDebug_AddDebugStringCallback( t_DebugStringCallback a_fnCallback );
TBOOL __stdcall TDebug_RemoveDebugStringCallback( t_DebugStringCallback a_fnCallback );

void TDebug_GetCallStack( TINT a_iMaxFrames, void** a_ppFrames );
void TDebug_DumpCallStack( TINT a_iNumFrames, void** a_ppFrames, TINT a_iFirstFrame );

// The TASSERT handler: expression, file, line, "TASSERT" or a printf-style message with its arguments (a few retail
// sites pass them, e.g. "Missing animation [%s]"). NOTE: retail's handler is an empty stub; ours logs and shows the
// assert dialog ("Debug" breaks inside the handler)
void __cdecl TDebug_AssertHandler( const TCHAR* a_szExpression, const TCHAR* a_szFileName, TUINT a_uiLineNumber, const TCHAR* a_szDescription = "TASSERT", ... );
