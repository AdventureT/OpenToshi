/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <Common/Base/hkBase.h>
#include <Common/Base/Memory/StackTracer/hkStackTracer.h>
#include <Common/Base/Fwd/hkcstdio.h>
#include <Common/Base/Fwd/hkwindows.h>

// dbghelp.h has an incorrect version of this block
#ifndef __specstrings
#define __specstrings
#	define __in
#	define __out
#	define __inout
#	define __in_opt
#	define __out_opt
#	define __inout_opt
#	define __in_ecount
#	define __out_ecount(A)
#	define __inout_ecount
#	define __in_bcount
#	define __out_bcount(A)
#	define __inout_bcount
#	define __deref_opt_out
#	define __deref_out
#	define __out_xcount(A)
#endif
typedef hkUlong ULONG_PTR, *PULONG_PTR;

// VC7 and higher:
#include <imagehlp.h>

typedef BOOL    (__stdcall *tSymInitialize)( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
typedef DWORD   (__stdcall *tSymGetOptions)( VOID );
typedef DWORD   (__stdcall *tSymSetOptions)( IN DWORD SymOptions );
typedef BOOL    (__stdcall *tSymGetSymFromAddr)( IN HANDLE hProcess, IN DWORD dwAddr, OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_SYMBOL Symbol );
typedef BOOL    (__stdcall *tStackWalk64)( DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress );
typedef PVOID   (__stdcall *tSymFunctionTableAccess64)( HANDLE hProcess, DWORD64 AddrBase );
typedef DWORD64 (__stdcall *tSymGetModuleBase64)( IN HANDLE hProcess, IN DWORD64 dwAddr );
typedef BOOL    (__stdcall *tSymGetLineFromAddr64)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Line );
typedef BOOL    (__stdcall *tSymCleanup)( IN HANDLE hProcess );

static HINSTANCE hDbgHelpDll;
static int s_DllReferences;

static tSymInitialize pSymInitialize;
static tSymGetOptions pSymGetOptions;
static tSymSetOptions pSymSetOptions;
static tSymGetSymFromAddr pSymGetSymFromAddr;
static tStackWalk64 pStackWalk64;
static tSymFunctionTableAccess64 pSymFunctionTableAccess64;
static tSymGetModuleBase64 pSymGetModuleBase64;
static tSymGetLineFromAddr64 pSymGetLineFromAddr64;
static tSymCleanup pSymCleanup;

#define LOAD_FUNCTION(A) if(1) { p##A = (t##A) GetProcAddress(hDbgHelpDll, #A); HK_ASSERT2(0x7ff2db1c, p##A, "Could not load symbol from imghelp.dll"); } else
extern const char* hkStackTracerDbghelpPath;
const char* hkStackTracerDbghelpPath = "dbghelp.dll";

hkStackTracer::hkStackTracer()
{
	if( hDbgHelpDll == HK_NULL )
	{
		// If you do not get symbols in the stack trace, it could be because the version of dbghelp.dll
		// in your path is too old. You can download a new one from
		// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/dbghelp_versions.asp
		// And set an explicit path for the dll to load
		// extern const char* hkStackTracerDbghelpPath;
		// hkStackTracerDbghelpPath = "C:\\Program Files\\Debugging Tools for Windows\\dbghelp.dll";
		hDbgHelpDll = LoadLibrary(hkStackTracerDbghelpPath);
		HK_ASSERT2(0x6427e415, hDbgHelpDll != HK_NULL, "Unable to load dbghelp.dll");

		s_DllReferences = 1;
		LOAD_FUNCTION(SymInitialize);
		LOAD_FUNCTION(SymGetOptions);
		LOAD_FUNCTION(SymSetOptions);
		LOAD_FUNCTION(SymGetSymFromAddr);
		LOAD_FUNCTION(StackWalk64);
		LOAD_FUNCTION(SymFunctionTableAccess64);
		LOAD_FUNCTION(SymGetModuleBase64);
		LOAD_FUNCTION(SymGetLineFromAddr64);
		LOAD_FUNCTION(SymCleanup);

		DWORD symOptions = pSymGetOptions();
		symOptions |= SYMOPT_LOAD_LINES | SYMOPT_DEBUG;
		pSymSetOptions( symOptions );

		HK_ON_DEBUG( BOOL initsymbols = ) pSymInitialize( GetCurrentProcess(), HK_NULL, TRUE);
		HK_ASSERT(0x44b27798, initsymbols);
	}
	else
	{
		s_DllReferences += 1;
	}
}

hkStackTracer::~hkStackTracer()
{
	s_DllReferences -= 1;
	if( s_DllReferences == 0 )
	{
		pSymCleanup( GetCurrentProcess() );
		pSymInitialize = 0;
		pSymGetOptions = 0;
		pSymSetOptions = 0;
		pSymGetSymFromAddr = 0;
		pStackWalk64 = 0;
		pSymFunctionTableAccess64 = 0;
		pSymGetModuleBase64 = 0;
		pSymGetLineFromAddr64 = 0;
		pSymCleanup = 0;
		FreeLibrary( hDbgHelpDll );
		hDbgHelpDll = 0;
	}
}

void hkStackTracer::dumpStackTrace( const hkUlong* trace, int maxtrace, printFunc pfunc, void* context )
{
	char buf[2048];
	HANDLE curproc = GetCurrentProcess();

	for( int i = 0; i < maxtrace; ++i)
	{
		hkUlong symaddress = trace[i];

		IMAGEHLP_SYMBOL& sym = *(IMAGEHLP_SYMBOL*)buf;
		memset(&sym, 0, sizeof(IMAGEHLP_SYMBOL));
		sym.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
		sym.MaxNameLength = 2048 - sym.SizeOfStruct;

		DWORD displacement = 0;
		BOOL ok_symbol = pSymGetSymFromAddr( 
			curproc, 
			DWORD(symaddress),
			&displacement,
			&sym);

		if(! ok_symbol)
		{
			static bool haveWarned;
			if( ! haveWarned )
			{
				pfunc(	"**************************************************************\n" \
						"* Cannot find symbol for an address\n" \
						"* Either debug information was not found or your version of\n" \
						"* dbghelp.dll may be too old to understand the debug format.\n" \
						"* See the comments in hkStackTracer::hkStackTracer()\n" \
						"* " __FILE__ "\n" \
						"**************************************************************\n", context);
				haveWarned = 1;
			}
			
			::strcpy(sym.Name, "(unknown)");
		}
		else if (::strcmp(sym.Name, "WinMain") == 0 )
		{
			break;
		}

		IMAGEHLP_LINE64 line;
		::memset(&line, 0, sizeof(line));
		line.SizeOfStruct = sizeof(line);

		pSymGetLineFromAddr64(
			curproc,
			DWORD(symaddress),
			&displacement,
			&line);

		char trace[2048];
		_snprintf(trace, 2048, "%s(%i):'%s'\n", line.FileName, line.LineNumber, sym.Name);
		pfunc(trace, context);
	}
	pfunc("-------------------------------------------------------------------\n\n", context);
}

//
// Hardcoded 386
//
#if 1 
int hkStackTracer::getStackTrace( hkUlong* trace, int maxtrace )
{
#if defined(HK_ARCH_X64) || !defined(HK_FULLDEBUG)
	return 0; // Hardcoded walker only works when full debug information (not just line numbers etc) are used.
#else	

	struct frame
	{
		frame* next;
		hkUlong retaddr;
	};

	frame* stackPtr;
	frame* framePtr;

	__asm
	{
		mov stackPtr, esp
		mov framePtr, ebp
	}

	int count = 0;
	if( stackPtr && framePtr )
	{
		frame* current = framePtr;

		while( (count < maxtrace)
			&& (stackPtr < current)
			&& (current != 0)
			&& (current->retaddr != 0) )
		{
			trace[count++] = current->retaddr;
			current = current->next;
		}
	}

	return count;
#endif
}
#else
// 
// Generic (32 / 64bit compat, but has issues..)
//
int hkStackTracer::getStackTrace( hkUlong* trace, int maxtrace )
{
	HANDLE curproc   = GetCurrentProcess();
	HANDLE curthread = GetCurrentThread();

	CONTEXT context;
	::memset(&context, 0, sizeof(context));
/*
	__asm
	{
		call GetEIP
		jmp Done
	GetEIP:
		pop eax
		mov context.Eip, eax
		mov context.Ebp, ebp
		push eax
		ret
	Done:
	}
*/
	context.ContextFlags = CONTEXT_FULL;
	HK_ON_DEBUG( BOOL ok_context = ) GetThreadContext(curthread, &context);
	HK_ASSERT(0x62047d0a, ok_context);

	STACKFRAME64 sf;
	::memset(&sf, 0, sizeof(sf));
	sf.AddrPC.Offset    = context.Eip;
	sf.AddrPC.Mode      = AddrModeFlat;
	sf.AddrFrame.Offset = context.Ebp;
	sf.AddrFrame.Mode   = AddrModeFlat;

	int frame = -2;
	for( ; frame < maxtrace; ++frame )
	{
		if( pStackWalk64(
			IMAGE_FILE_MACHINE_I386,
			curproc, 
			curthread,
			&sf,
			&context,
			HK_NULL,
			pSymFunctionTableAccess64,
			pSymGetModuleBase64,
			HK_NULL) )
		{
			if(frame >= 0)
			{
				trace[frame] = (hkUlong)sf.AddrPC.Offset;
			}
		}
		else
		{
			break;
		}
	}
	return frame >= 0 ? frame : 0;
}
#endif

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090216)
* 
* Confidential Information of Havok.  (C) Copyright 1999-2009
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
* 
* Use of this software for evaluation purposes is subject to and indicates
* acceptance of the End User licence Agreement for this product. A copy of
* the license is included with this software and is also available at www.havok.com/tryhavok.
* 
*/
