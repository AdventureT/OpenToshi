#include "ToshiPCH.h"
#include "TUtil.h"
#include "TRegion.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/Core/TError.h"

TOSHI_NAMESPACE_START

void TUtil::LogInitialise()
{
	TLogFile* logfile                         = new TLogFile();
	Toshi::TUtil::GetSingleton()->m_pLogFile1 = logfile;
	Toshi::TUtil::GetSingleton()->m_pLogFile2 = logfile;

	TrimLog("*.log", 9);
	time_t seconds;
	time(&seconds);

	tm* time = gmtime(&seconds);

	TCHAR filename[256];
	T2String8::Format(filename, "deblob_%d%02d%02d_%02d_%02d_%02d.log", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);
	GetLog()->Create(filename, "Toshi 2.0", TFALSE);
	GetLog()->AllowIndentation(TTRUE);
	GetLog()->SetSimpleMode(TTRUE);
}

void TUtil::Log(const TCHAR* format, ...)
{
	if (GetLog() != TNULL)
	{
		TCHAR str[2048];

		va_list args;
		va_start(args, format);
		TINT size = T2String8::FormatV(str, sizeof(str), format, args);
		va_end(args);

#ifdef TOSHI_DEBUG
		str[size]     = '\n';
		str[size + 1] = '\0';
		OutputDebugStringA(str);
		str[size] = '\0';
		TOSHI_CORE_TRACE(str);
#endif

		GetLog()->Log(TLogFile::Type_Info, "Toshi", "Kernel", str);
		GetSingleton()->m_Emitter.Throw(GetLog());
	}
}

void TUtil::Log(TLogFile::Type logtype, const TCHAR* format, ...)
{

	if (GetLog() != TNULL)
	{
		va_list args;
		va_start(args, format);

		TCHAR str[2048];
		TINT  size = T2String8::FormatV(str, sizeof(str), format, args);

#ifdef TOSHI_DEBUG
		str[size]     = '\n';
		str[size + 1] = '\0';
		OutputDebugStringA(str);
		str[size] = '\0';
		TOSHI_CORE_TRACE(str);
#endif

		va_end(args);

		GetLog()->Log(logtype, "Toshi", "Kernel", format, str);
		GetSingleton()->m_Emitter.Throw(GetLog());
	}
}

void TUtil::LogConsole(const TCHAR* format, ...)
{
	if (GetLog() != TNULL)
	{
		va_list vargs;
		va_start(vargs, format);

		TCHAR str[1024];
		TINT  size    = T2String8::FormatV(str, sizeof(str), format, vargs);
		str[size]     = '\n';
		str[size + 1] = '\0';
		OutputDebugStringA(str);
		str[size] = '\0';
		TOSHI_CORE_TRACE(str);
	}
}

void TUtil::LogSet(TLogFile* a_logFile)
{
	Log("Changing log file.");
	TUtil* util = Toshi::TUtil::GetSingletonSafe();
	GetLog()->Close();
	util->m_pLogFile2 = a_logFile == TNULL ? util->m_pLogFile1 : a_logFile;
}

TBOOL TUtil::ToshiCreate(TINT argc, TCHAR** argv, TMemory& memorySettings)
{
	TRegion::LockRegion();

	// Initialize TMemory and dlmalloc
	memorySettings.Init();

	// Initialize other systems
	TLog::Create();
	TSystemManager::Create();
	TFileManager::Create();

	Create();
	TError::CreateSingleton(0x1800, 0x100);

	return TTRUE;
}

void TUtil::MemSet(void* ptr, size_t value, size_t size)
{
	if (size >= sizeof(void*))
	{
		size_t* pos       = static_cast<size_t*>(ptr);
		size_t  stepCount = size / sizeof(void*);
		size -= stepCount * sizeof(void*);

		for (; stepCount != 0; stepCount--) *(pos++) = value;

		ptr = pos;
	}

	while (size > 0)
	{
		TUINT8 stepSize = size & 0b11;
		if (stepSize == 0) stepSize = 4;

		if (size == 1)
		{
			*(TUINT8*)ptr = (TUINT8)value;
		}
		else
		{
			if (size == 2)
			{
				*(TUINT16*)ptr = (TUINT16)value;
			}
			else if (size == 3)
			{
				*(TUINT16*)ptr                               = (TUINT16)value;
				*(TUINT8*)((uintptr_t)ptr + sizeof(TUINT16)) = (TUINT8)value;
			}
			else if (size == 4)
			{
				*(TUINT32*)ptr = (TUINT32)value;
			}
		}

		ptr = reinterpret_cast<void*>((uintptr_t)ptr + stepSize);
		size -= stepSize;
	}
}

// Source: https://lentz.com.au/blog/tag/crc-table-generator
void TUtil::CRCInitialise()
{
	TUINT32 crc;

	for (TINT i = 0; i < CRC_TABSIZE; i++)
	{
		crc = i;
		for (TINT j = 8; j > 0; j--)
		{
			if (crc & 1)
				crc = (crc >> 1) ^ CRC32POLY;
			else
				crc >>= 1;
		}
		s_aiCRC32LUT[i] = crc;
	}
}

// Source: https://lentz.com.au/blog/tag/crc-table-generator
TUINT32 TUtil::CRC32(TUINT8* buffer, TUINT32 len)
{
	TUINT32 crc = 0;

	while (len--) crc = crc32upd(s_aiCRC32LUT, crc, *buffer++);

	return CRC32POST(crc);
}

TOSHI_NAMESPACE_END
