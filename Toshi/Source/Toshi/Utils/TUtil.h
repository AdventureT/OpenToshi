#pragma once
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/File/TLogFile.h"
#include "Toshi/Core/TEvent.h"

TOSHI_NAMESPACE_START

class TUtil : public TSingleton<TUtil>
{
public:
	static TBOOL ToshiCreate(TINT argc, TCHAR** argv, TMemory& memorySettings);

	static void ToshiDestroy()
	{
		ToshiDestroySubsystems();
		ToshiDestroyMemory();
	}

	static void Create()
	{
		TUtil::CreateSingleton();
		TUtil::GetSingletonSafe()->LogInitialise();
		TUtil::CRCInitialise();
	}

	static void ToshiDestroySubsystems()
	{
		TLog::Destroy();
		// ....
	}

	static void ToshiDestroyMemory() { TMemory::Shutdown(); }

	static const TCHAR* GetTime();
	static void         TrimLog(const TCHAR* fileExtension, size_t trimTo);
	//static uint64_t GetUnixSeconds(uint64_t* pOut = TNULL);
	static void MemSet(void* ptr, size_t value, size_t size);

	static void* MemCopy(void* dst, const void* src, size_t size) { return memcpy(dst, src, size); }

	static void MemClear(void* ptr, size_t size) { memset(ptr, 0, size); }

	// $deBlob: FUNCTION 0065d4f0
	static TINT MemCompare(void* ptr1, void* ptr2, TINT size) { return memcmp(ptr1, ptr2, size); }

	static void LogInitialise();
	static void Log(const TCHAR* format, ...);
	static void Log(TLogFile::Type logtype, const TCHAR* format, ...);
	static void LogConsole(const TCHAR* format, ...);
	static void LogSet(TLogFile* a_logFile);

	// $deBlob: FUNCTION 0065d5c0
	static TLogFile* GetLog() { return Toshi::TUtil::GetSingleton()->m_pLogFile2; }

	// $deBlob: FUNCTION 0065d770
	static void LogDown() { GetLog()->Down(); }

	static void LogUp() { GetLog()->Up(); }

private:
	TLogFile*          m_pLogFile1;
	TLogFile*          m_pLogFile2;
	TEmitter<TLogFile> m_Emitter;

public:
#pragma region CRC

// Source: https://lentz.com.au/blog/tag/crc-table-generator

/*
			LICENCE

			This package may be freely distributed provided the files remain together,
			in their original unmodified form.
			All files, executables and sourcecode remain the copyrighted property of
			Arjen G. Lentz and LENTZ SOFTWARE-DEVELOPMENT.
			Licence for any use granted, provided this notice & CRC.DOC are included.
			For executable applications, credit should be given in the appropriate
			places in the program and documentation.
			These notices must be retained in any copies of any part of this
			documentation and/or software.

			Any use of, or operation on (including copying/distributing) any of
			the above mentioned files implies full and unconditional acceptance of
			this licence and disclaimer.
		*/

/* ------------------------------------------------------------------------- */
/* CRC-32 CCITT                                                              */
/* ------------------------------------------------------------------------- */
#define CRC32POLY      (0xEDB88320L) /* Generator polynomial number       */
#define CRC32POST(crc) (~(crc))      /* CRC Postconditioning before xmit  */

#define crc32upd(crctab, crc, c) ((crctab)[((TINT)(crc) ^ (c)) & 0xff] ^ ((crc) >> 8))

#define CRC_TABSIZE (256) /* Normal 256-entry table            */

	//=============================================================================

	inline static TUINT32 s_aiCRC32LUT[CRC_TABSIZE] = {};

	static void    CRCInitialise();
	static TUINT32 CRC32(TUINT8* buffer, TUINT32 len);

#pragma endregion
};

TOSHI_NAMESPACE_END
