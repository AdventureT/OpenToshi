/**
 * @file TLogFile.h
 * Logging system for the Toshi engine
 * 
 * This file provides logging functionality including:
 * - Log file creation and management
 * - Log message formatting and writing
 * - Log level control
 * - Log file rotation and cleanup
 */

#pragma once

TOSHI_NAMESPACE_START

class TFile;

class TLogFile
{
private:
	static const TINT cLevelMax = 30;

	// NOTE: de Blob's TLogFile knows 3 log types (Info, Warning, Error): no Critical (Barnyard's 4th) in its counters
	static TCONSTEXPR_VAR TINT cTypeCount = 3;
	static const TCHAR* const cTypeStrings[ cTypeCount ]; // 0x981b60

public:
	enum class Error
	{
		OK,
		OpenFile,
		AlreadyCreated
	};

public:
	TLogFile();
	~TLogFile();

	Error Create( const TCHAR* fileName, const TCHAR* str2, TBOOL writeExisting );
	void  Close();

	void RecalcLevel();

	void Print( const TCHAR* format, ... );
	void Log( TUtil::LogType type, const TCHAR* str1, const TCHAR* str2, const TCHAR* format, ... );
	void LogSimple( TUtil::LogType type, const TCHAR* format, ... );

	void Flush();

	void Down();
	void Up();
	void Indent();
	void UnIndent();

	// $deBlob: FUNCTION 00687950
	TBOOL AllowIndentation( TBOOL a_bAllow )
	{
		TBOOL bOld          = m_bAllowIndentation;
		m_bAllowIndentation = a_bAllow;
		return bOld;
	}
	TBOOL GetAllowIndentation() { return m_bAllowIndentation; }

	// $deBlob: FUNCTION 00687940
	TBOOL SetSimpleMode( TBOOL a_bEnable )
	{
		TBOOL bOld      = m_bIsSimpleMode;
		m_bIsSimpleMode = a_bEnable;
		return bOld;
	}
	TBOOL IsSimpleMode() { return m_bIsSimpleMode; }

	// $deBlob: FUNCTION 00687b70
	TINT GetTypeCount( TUtil::LogType a_eType ) const { return m_typeCounts[ a_eType ]; }

	// $deBlob: FUNCTION 00687b90
	TINT GetTotalLogCount() const { return m_iTotalLogCount; }

	// $deBlob: FUNCTION 00687b80
	const TCHAR* GetLevelString() const { return m_LevelString; }

private:
	TFile*  m_pFile;                              // 0x00
	TINT    m_iTotalLogCount;                     // 0x04
	TBOOL   m_bIsSimpleMode;                      // 0x08
	TBOOL   m_bAllowIndentation;                  // 0x09
	TCHAR   m_LevelString[ cLevelMax + 1 ];       // 0x0A
	TINT    m_curLevel;                           // 0x2C
	TINT    m_typeCounts[ cTypeCount ];           // 0x30
};

TOSHI_NAMESPACE_END
