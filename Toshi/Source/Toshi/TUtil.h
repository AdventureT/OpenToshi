#pragma once
#include "Toshi/TSingleton.h"
#include "Toshi/TEvent.h"
#include "Thread/TMutex.h"
#include "Core/TMemory.h"

#include <utility>
#include <cstring>

TOSHI_NAMESPACE_START

class TPString8Pool;
class TLogFile;

class TUtil : public TSingleton<TUtil>
{
public:
	//-----------------------------------------------------------------------------
	// Logging
	//-----------------------------------------------------------------------------

	enum LogType
	{
		LogType_Info,
		LogType_Warning,
		LogType_Error,
		LogType_Critical,
		LogType_NUMOF
	};

	static const TCHAR* const kTypeStrings[ LogType_NUMOF ];

	static TCONSTEXPR const TCHAR* LogTypeToString( LogType a_eType )
	{
		TSTATICASSERT( Toshi::TUtil::LogType_NUMOF == TARRAYSIZE( kTypeStrings ) );

		return a_eType >= LogType_Info && a_eType < LogType_NUMOF ?
		    kTypeStrings[ a_eType ] :
		    "UNKNOWN";
	}

	struct LogEvent
	{
		TCONSTEXPR LogEvent( TLogFile* a_pFile, LogType a_eType, const TCHAR* a_szString )
		    : m_pFile( a_pFile ), m_eType( a_eType ), m_szString( a_szString )
		{}

		TLogFile*    m_pFile;
		LogType      m_eType;
		const TCHAR* m_szString;
	};

public:
	//-----------------------------------------------------------------------------
	// Static members
	//-----------------------------------------------------------------------------

	// Creates the region lock, the memory, the system manager, the file manager, TUtil (logs + CRC table) and TError.
	// NOTE: argc and argv are unused
	static TBOOL ToshiCreate( TINT a_iArgc, TCHAR** a_ppArgv, const TMemoryParams* a_pMemoryParams );

	// Destroys the subsystems and the memory
	// NOTE: retail never calls it (main() ends with ExitProcess)
	static void ToshiDestroy();
	static void ToshiDestroySubsystems();
	static void ToshiDestroyMemory();

	// Updates the system manager (the frame timer and the scheduler)
	static void ToshiUpdate();

	static const TCHAR* GetTime();

	static void MemSet( void* ptr, TINT value, TSIZE size );

	// NOTE: de Blob's MemCopy is a memmove
	static void* MemCopy( void* dst, const void* src, TSIZE size );

	static void MemClear( void* ptr, TSIZE size );

	static TINT MemCompare( const void* ptr1, const void* ptr2, TSIZE size );

	static void* MemMove( void* dst, const void* src, TSIZE size );

	template <class T>
	static void Fill( T* a_pStart, T* a_pEnd, const T& a_rcValue = T() )
	{
		for ( ; a_pStart != a_pEnd; a_pStart++ )
			*a_pStart = a_rcValue;
	}

	// Converts a binary-coded decimal number (one decimal digit per nibble) to its value
	static TUINT32 BCDToInteger( TUINT32 a_uiBCD );

	//-----------------------------------------------------------------------------
	// Logging
	//-----------------------------------------------------------------------------
	static void Log( const TCHAR* a_szFormat, ... );
	static void Log( LogType a_eLogType, const TCHAR* a_szFormat, ... );
	static void TrimLog( const TCHAR* fileExtension, TSIZE trimTo );
	static void LogDown();
	static void LogUp();
	static void LogIndent();
	static void LogUnIndent();
	static void LogConsole( const TCHAR* a_szFormat, ... );
	static void LogSet( TLogFile* a_logFile );

	// $deBlob: FUNCTION 0065d5c0
	static TLogFile* GetCurrentLogFile() { return ms_pSingleton->m_pCurrentLogFile; }
	static TLogFile* GetLog() { return GetCurrentLogFile(); }

	// NOTE: de Blob has no TPString8 pool (no trace of TPString8 in retail); kept for the imported engine code
	static TPString8Pool* SetTPStringPool( TPString8Pool* a_pStringPool ) { return TExchange( *ms_poStringPool, a_pStringPool ); }

	// $Barnyard: FUNCTION 006bb580
	static TPString8Pool* GetTPStringPool()
	{
		TASSERT( ms_poStringPool != TNULL );
		return *ms_poStringPool;
	}

private:
	static void Create();
	static void Destroy();
	static void CreateTPStringPool();
	static void DestroyTPStringPool();

private:
	static TPString8Pool** ms_poStringPool;

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
#define CRC32POLY        ( 0xEDB88320L ) /* Generator polynomial number       */
#define CRC32POST( crc ) ( ~( crc ) )    /* CRC Postconditioning before xmit  */

#define crc32upd( crctab, crc, c ) \
	( ( crctab )[ ( (TINT)( crc ) ^ ( c ) ) & 0xff ] ^ ( ( crc ) >> 8 ) )

#define CRC_TABSIZE ( 256 ) /* Normal 256-entry table            */

	//=============================================================================

	static TUINT32 s_aiCRC32LUT[ CRC_TABSIZE ];

	static void    CRCInitialise();
	static TUINT32 CRC32( void* buffer, TUINT32 len );

#pragma endregion

public:
	TUtil();

	// $deBlob: FUNCTION 0065d8b0
	~TUtil() = default;

	// Creates the logs (TrimLog, a new TLogFile named after the local time). Returns TFALSE when the log file can't
	// be created
	TBOOL LogInitialise();

	// Closes the current log and deletes the default one
	void LogUninitialise();

	// $deBlob: FUNCTION 0065d7d0
	TEmitter<TUtil, LogEvent>& GetLogEvent() { return m_LogEmitter; }

private:
	TLogFile*                 m_pDefaultLogFile; // 0x00
	TLogFile*                 m_pCurrentLogFile; // 0x04
	TEmitter<TUtil, LogEvent> m_LogEmitter;      // 0x08
}; // 0x14

TOSHI_NAMESPACE_END
