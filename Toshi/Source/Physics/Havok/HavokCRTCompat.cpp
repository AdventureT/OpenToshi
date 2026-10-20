#include "ToshiPCH.h"
#include <cstdio>

// NOTE: only for the Universal CRT (VS2015+); VS2013 (the matching build) has the old CRT
#ifndef TOSHI_COMPILER_VS2013

// NOTE: not retail code. The Havok 6.5 libs were built with VS2008, whose stdin/stdout/stderr macros call the CRT's
// __iob_func; the Universal CRT (VS2015+) has __acrt_iob_func instead. The only user is hkStdioStreamWriter (Havok's
// std stream writer, used by hkcout/hkcerr and the default error report, none of which we use: TPhysicsManager
// passes its own error report). The old FILE array stride (32 bytes) doesn't match the UCRT's, so only stdin
// resolves to the right stream through this shim.
extern "C" FILE* __cdecl HavokCRTCompat_iob_func()
{
	return __acrt_iob_func( 0 );
}

// The import thunk pointer the VS2008 objects reference (__imp____iob_func)
extern "C" void* _imp____iob_func = (void*)&HavokCRTCompat_iob_func;

// NOTE: the release Havok libs (hkString::sprintf/vsnprintf) import vsprintf and _vsnprintf from the VS2008 DLL CRT
// (__imp__vsprintf, __imp___vsnprintf). The UCRT implements them inline in <stdio.h>, so there's nothing to import:
// provide the import thunk pointers, pointing at wrappers around the UCRT's functions.
extern "C" int __cdecl HavokCRTCompat_vsprintf( char* a_szBuffer, const char* a_szFormat, va_list a_Args )
{
	return vsprintf( a_szBuffer, a_szFormat, a_Args );
}

extern "C" int __cdecl HavokCRTCompat_vsnprintf( char* a_szBuffer, size_t a_uiCount, const char* a_szFormat, va_list a_Args )
{
	return _vsnprintf( a_szBuffer, a_uiCount, a_szFormat, a_Args );
}

extern "C" void* _imp__vsprintf   = (void*)&HavokCRTCompat_vsprintf;
extern "C" void* _imp___vsnprintf = (void*)&HavokCRTCompat_vsnprintf;

#endif // !TOSHI_COMPILER_VS2013
