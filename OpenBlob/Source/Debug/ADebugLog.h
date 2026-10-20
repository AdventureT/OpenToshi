#pragma once

//-----------------------------------------------------------------------------
// Purpose: a buffered text log written to "<base>.<secondary>" in the working directory. Write formats into the buffer
// and appends it to the file when it's full (Flush); the destructor flushes.
// NOTE: only debug code that retail never runs uses it (AStreamingDataHudElement and AMusicGameBotTest).
//-----------------------------------------------------------------------------
class ADebugLog
{
public:
	static TCONSTEXPR_VAR TINT DEBUGLOG_FMTSTR_BUFLEN = 0x400;

public:
	// a_iBufSize < 0: 0x4000 bytes
	ADebugLog( const TCHAR* a_szBaseName, const TCHAR* a_szSecondaryName, TINT a_iBufSize );
	~ADebugLog();

	void Write( const TCHAR* a_szFormat, ... );
	void Flush();

	// Creates (empties) the file
	void CreateLogFile();

private:
	TCHAR* m_szBaseName;      // 0x00 (at most 8 characters)
	TCHAR* m_szSecondaryName; // 0x04 (the extension)
	TCHAR* m_pBuffer;         // 0x08
	TINT   m_iBufSize;        // 0x0C
	TINT   m_iBufUsedSize;    // 0x10
	TUINT8 m_aUnk14[ 4 ];     // 0x14 (not used)
};                            // 0x18
