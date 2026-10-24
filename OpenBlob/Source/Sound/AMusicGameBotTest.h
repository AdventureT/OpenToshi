#pragma once

class ADebugLog;

//-----------------------------------------------------------------------------
// Purpose: the music test of the game bot: loads levels with their music, records per test whether the memory FSBs, the
// streamed audio and the music loaded and the memory used, and writes a summary ("MTS.csv") and the details ("MTD.csv")
// with ADebugLog.
// NOTE: retail never creates one nor calls any of its functions (dead code).
//-----------------------------------------------------------------------------
class AMusicGameBotTest
{
public:
	static TCONSTEXPR_VAR TINT MAX_NUM_MUSIC_TEST = 180;

	// One test (0x14 bytes)
	struct Test
	{
		const TCHAR* m_szLevelName;       // 0x00
		const TCHAR* m_szTrackName;       // 0x04
		TINT         m_iMemoryUsed;       // 0x08 -1
		TINT         m_iPeakMemoryUsed;   // 0x0C
		TBOOL        m_bMemoryFSBSuccess; // 0x10
		TBOOL        m_bStreamSuccess;    // 0x11
		TBOOL        m_bMusicLoadSuccess; // 0x12
	};

	// The names of the tests
	enum TESTTYPE
	{
		TESTTYPE_AllLevelsAllMusic,
		TESTTYPE_AllLevelsWithAllMusic,
		TESTTYPE_AllSprintLevels,
		TESTTYPE_AllMusicCurrentLevel,
		TESTTYPE_AllLevelsCurrentMusic,
		TESTTYPE_AllFeatureLevelsNoMusic,
	};

public:
	AMusicGameBotTest();
	~AMusicGameBotTest();

	void Reset();

	Test* GetCurrentTest();

	// The first lines of the summary log
	void WriteSummaryStart();
	// The end of the summary log: the counts, the test that used the most memory and the failed tests
	void WriteSummaryEnd();
	// The first lines of the detailed log (a_pLog TNULL: m_pLogMusicTestDetailed)
	void WriteDetailsStart( ADebugLog* a_pLog );

	void SetTestType( TINT a_eTestType );

	// Logs the current test and moves to the next one; TRUE once every test has run (the summary is written)
	TBOOL NextTest();

	void SetUnkE14( TINT a_iUnk );

	// The date of the logs (not available on Windows)
	static void FormatDate( TCHAR* a_szBuffer, TINT a_iSize );

	// Logs the test a_iTest (< 0: the current one) to a_pLog (TNULL: m_pLogMusicTestDetailed) and the debug output
	void WriteTestResult( TINT a_iTest, ADebugLog* a_pLog );

private:
	Test         m_aTests[ MAX_NUM_MUSIC_TEST ]; // 0x000
	TUINT32      m_uiUnkE10;                     // 0xE10
	TINT         m_iUnkE14;                      // 0xE14
	const TCHAR* m_szTestName;                   // 0xE18
	ADebugLog*   m_pLogMusicTestSummery;         // 0xE1C
	ADebugLog*   m_pLogMusicTestDetailed;        // 0xE20
	TINT         m_iUnkE24;                      // 0xE24
	TINT         m_aiUnkE28[ 4 ];                // 0xE28
	TINT         m_iNumTests;                    // 0xE38
	TINT         m_iCurrentTestNum;              // 0xE3C
	TINT         m_iNumFailed;                   // 0xE40
};
