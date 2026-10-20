#include "pch.h"
#include "AMusicGameBotTest.h"
#include "AMusicManager.h"
#include "Debug/ADebugLog.h"

#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The names of the tests (TESTTYPE)
static const TCHAR* s_aszTestNames[] = {
	"All Feature level, all sprint levels, all music",
	"Load all levels with all music",
	"Load all sprint levels",
	"All music for current level",
	"All levels for current music",
	"All Feature levels No Music",
};

// $deBlob: FUNCTION 00604f90
AMusicGameBotTest::AMusicGameBotTest()
{
	for ( TINT i = MAX_NUM_MUSIC_TEST - 1; i >= 0; i-- )
	{
		m_aTests[ i ].m_szLevelName       = TNULL;
		m_aTests[ i ].m_szTrackName       = TNULL;
		m_aTests[ i ].m_iMemoryUsed       = -1;
		m_aTests[ i ].m_bMemoryFSBSuccess = TFALSE;
		m_aTests[ i ].m_bStreamSuccess    = TFALSE;
		m_aTests[ i ].m_bMusicLoadSuccess = TFALSE;
	}

	m_pLogMusicTestSummery  = new ADebugLog( "MTS", "csv", -1 );
	m_pLogMusicTestDetailed = new ADebugLog( "MTD", "csv", -1 );

	m_iUnkE24         = 0;
	m_szTestName      = TNULL;
	m_aiUnkE28[ 0 ]   = 0;
	m_aiUnkE28[ 1 ]   = 0;
	m_aiUnkE28[ 2 ]   = 0;
	m_aiUnkE28[ 3 ]   = 0;
	m_iNumTests       = -1;
	m_iCurrentTestNum = -1;
	m_iNumFailed      = -1;
}

// $deBlob: FUNCTION 006050d0
AMusicGameBotTest::~AMusicGameBotTest()
{
	if ( m_pLogMusicTestSummery )
		delete m_pLogMusicTestSummery;

	if ( m_pLogMusicTestDetailed )
		delete m_pLogMusicTestDetailed;
}

// $deBlob: FUNCTION 00605110
void AMusicGameBotTest::Reset()
{
	m_iUnkE24         = 0;
	m_szTestName      = TNULL;
	m_aiUnkE28[ 0 ]   = 0;
	m_aiUnkE28[ 1 ]   = 0;
	m_aiUnkE28[ 2 ]   = 0;
	m_aiUnkE28[ 3 ]   = 0;
	m_iNumTests       = -1;
	m_iCurrentTestNum = -1;
	m_iNumFailed      = -1;
}

// $deBlob: FUNCTION 00605170
AMusicGameBotTest::Test* AMusicGameBotTest::GetCurrentTest()
{
	TASSERT( m_iCurrentTestNum >= 0 && m_iCurrentTestNum <= MAX_NUM_MUSIC_TEST );
	return &m_aTests[ m_iCurrentTestNum ];
}

// $deBlob: FUNCTION 006051c0
void AMusicGameBotTest::WriteSummaryStart()
{
	TCHAR szDate[ 256 ];
	FormatDate( szDate, sizeof( szDate ) );

	m_pLogMusicTestSummery->Write( "Summery Test Run %s\n", m_szTestName );
	m_pLogMusicTestSummery->Write( "Test started %s\n", szDate );
}

// $deBlob: FUNCTION 00605230
void AMusicGameBotTest::WriteSummaryEnd()
{
	TASSERT( m_pLogMusicTestSummery );

	TCHAR szDate[ 256 ];
	FormatDate( szDate, sizeof( szDate ) );

	// NOTE: retail passes "Num tests failed\n" as an argument the format doesn't use
	m_pLogMusicTestSummery->Write( "Test finished %s\n", szDate );
	m_pLogMusicTestSummery->Write( "Num tests Total, Num tests run \n", "Num tests failed\n" );
	m_pLogMusicTestSummery->Write( "%d,%d,%d\n", m_iNumTests, m_iCurrentTestNum, m_iNumFailed );

	// The successful test that used the most memory
	TINT  iMaxMemory = -1;
	Test* pMaxTest   = TNULL;

	for ( TINT i = 0; i < m_iCurrentTestNum; i++ )
	{
		Test* pTest = &m_aTests[ i ];

		if ( pTest->m_bMemoryFSBSuccess && pTest->m_bStreamSuccess && pTest->m_bMusicLoadSuccess && iMaxMemory < pTest->m_iMemoryUsed )
		{
			iMaxMemory = pTest->m_iMemoryUsed;
			pMaxTest   = pTest;
		}
	}

	m_pLogMusicTestSummery->Write( "Max level, Max music, Mood name, Memory used, Peak memory used\n" );

	if ( pMaxTest != TNULL )
		m_pLogMusicTestSummery->Write( "%s, %s, %ls, %d, %d\n", pMaxTest->m_szLevelName, pMaxTest->m_szTrackName, AMusicManager::GetTrackDisplayName( pMaxTest->m_szTrackName ), pMaxTest->m_iMemoryUsed, pMaxTest->m_iPeakMemoryUsed );
	else
		m_pLogMusicTestSummery->Write( "NONE, NONE, NONE, NONE, NONE\n\n" );

	m_pLogMusicTestSummery->Write( "Failed List, Num Test Failed, %d \n", m_iNumFailed );
	m_pLogMusicTestSummery->Write( "LevelName, TrackName, MoodName, Pass Memory FSB Load, Pass Stream Player Load, Pass Music Load, Fail Memory Point\n" );
	TDebug_FinalPrintf( "Failed List, Num Test Failed, %d \n", m_iNumFailed );
	TDebug_FinalPrintf( "LevelName, TrackName, MoodName, Pass Memory FSB Load, Pass Stream Player Load, Pass Music Load, Fail Memory Point\n" );

	// The failed tests
	for ( TINT i = 0; i < m_iCurrentTestNum; i++ )
	{
		Test* pTest = &m_aTests[ i ];

		if ( !pTest->m_bMemoryFSBSuccess || !pTest->m_bStreamSuccess || !pTest->m_bMusicLoadSuccess )
			WriteTestResult( i, m_pLogMusicTestSummery );
	}

	m_pLogMusicTestSummery->Flush();
}

// $deBlob: FUNCTION 00605410
void AMusicGameBotTest::WriteDetailsStart( ADebugLog* a_pLog )
{
	if ( a_pLog == TNULL )
		a_pLog = m_pLogMusicTestDetailed;

	TCHAR szDate[ 256 ];
	FormatDate( szDate, sizeof( szDate ) );

	a_pLog->Write( "Detailed test Run:, %s, Time started, %s \n", m_szTestName, szDate );
	a_pLog->Write( "Level name, Music track, Mood Name ,MemoryFSBs Success, Streamed audio success, Music load success, Memory Used, Peak Memory Used\n" );
	TDebug_FinalPrintf( "Detailed test Run:, %s, Time started, %s \n", m_szTestName, szDate );
	TDebug_FinalPrintf( "Level name, Music track, Mood Name ,MemoryFSBs Success, Streamed audio success, Music load success, Memory Used, Peak Memory Used\n" );
	a_pLog->Flush();
}

// $deBlob: FUNCTION 006054b0
void AMusicGameBotTest::SetTestType( TINT a_eTestType )
{
	m_szTestName = s_aszTestNames[ a_eTestType ];
}

// $deBlob: FUNCTION 006054d0
TBOOL AMusicGameBotTest::NextTest()
{
	WriteTestResult( -1, TNULL );

	TASSERT( m_iCurrentTestNum >= 0 && m_iCurrentTestNum <= MAX_NUM_MUSIC_TEST );
	Test* pCurrentTest = &m_aTests[ m_iCurrentTestNum ];

	if ( !pCurrentTest->m_bMusicLoadSuccess || !pCurrentTest->m_bStreamSuccess || !pCurrentTest->m_bMemoryFSBSuccess )
		m_iNumFailed++;

	m_iCurrentTestNum++;

	if ( m_iCurrentTestNum == m_iNumTests )
	{
		WriteSummaryEnd();
		return TTRUE;
	}

	TASSERT( m_iCurrentTestNum >= 0 && m_iCurrentTestNum <= MAX_NUM_MUSIC_TEST );
	Test* pNextItem = &m_aTests[ m_iCurrentTestNum ];
	TASSERT( pNextItem != pCurrentTest );

	return TFALSE;
}

// $deBlob: FUNCTION 006055e0
void AMusicGameBotTest::SetUnkE14( TINT a_iUnk )
{
	m_iUnkE24 = 0;
	m_iUnkE14 = a_iUnk;
}

// NOTE: retail passes a_iSize as an argument the format doesn't use
// $deBlob: FUNCTION 00605600
void AMusicGameBotTest::FormatDate( TCHAR* a_szBuffer, TINT a_iSize )
{
	T2String8::Format( a_szBuffer, "yyyy/mm/dd hh:mm Not on win", a_iSize );
}

// NOTE: retail passes m_iPeakMemoryUsed as an argument neither format uses
// $deBlob: FUNCTION 00605620
void AMusicGameBotTest::WriteTestResult( TINT a_iTest, ADebugLog* a_pLog )
{
	if ( a_pLog == TNULL )
		a_pLog = m_pLogMusicTestDetailed;

	if ( a_iTest < 0 )
		a_iTest = m_iCurrentTestNum;

	Test* pTest = &m_aTests[ a_iTest ];

	a_pLog->Write( "%s, %s, %ls, %s, %s, %s, %d \n", pTest->m_szLevelName, pTest->m_szTrackName, AMusicManager::GetTrackDisplayName( pTest->m_szTrackName ), pTest->m_bMemoryFSBSuccess ? "TRUE" : "FALSE", pTest->m_bStreamSuccess ? "TRUE" : "FALSE", pTest->m_bMusicLoadSuccess ? "TRUE" : "FALSE", pTest->m_iMemoryUsed, pTest->m_iPeakMemoryUsed );
	TDebug_FinalPrintf( " %s, %s, %s, %s, %s, %d \n", pTest->m_szLevelName, pTest->m_szTrackName, pTest->m_bMemoryFSBSuccess ? "TRUE" : "FALSE", pTest->m_bStreamSuccess ? "TRUE" : "FALSE", pTest->m_bMusicLoadSuccess ? "TRUE" : "FALSE", pTest->m_iMemoryUsed, pTest->m_iPeakMemoryUsed );
	a_pLog->Flush();
}
