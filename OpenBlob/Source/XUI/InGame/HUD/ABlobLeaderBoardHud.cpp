#include "pch.h"
#include "ABlobLeaderBoardHud.h"
#include "GameController/AGameStatus.h"
#include "Locale/ALocaleManager.h"

#include <Toshi/TStringManager.h>
#include <T2Locale/T2Locale.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The locale strings of a place (1st, 2nd...) by the rank's m_iUnk0 (entry 0 holds the bits
// of 5.0f and is not a string)
static const TINT s_aiPlaceStrings[] = { 0x40A00000, 0xC0, 0xC1, 0xC2, 0xAD, 0xAE };

// The locale strings of the players by the rank's m_iPlayer (entry 3 holds the bits of 3.0f)
static const TINT s_aiPlayerStrings[] = { 0xAD, 0xAE, 0xAF, 0x40400000 };

// The locale string of an empty place
static const TINT s_iEmptyPlaceString = 0xC0;

static const TWCHAR* GetLocaleString( TINT a_iStringId )
{
	return TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() )->GetString( a_iStringId );
}

// $deBlob: FUNCTION 00656520
// $deBlob: FUNCTION 00656530
// $deBlob: FUNCTION 006565b0
// $deBlob: FUNCTION 00656630
// $deBlob: FUNCTION 00656640
// $deBlob: FUNCTION 0053e300
// $deBlob: FUNCTION 00858c30
TDEFINE_CLASS( ABlobLeaderBoardHud );

// $deBlob: FUNCTION 00656870
ABlobLeaderBoardHud::ABlobLeaderBoardHud()
{
#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail leaves the members to OnCreated
	for ( TINT i = 0; i < NUM_ROWS; i++ )
	{
		m_apName[ i ]  = TNULL;
		m_apPlace[ i ] = TNULL;
		m_apScore[ i ] = TNULL;
	}
#endif
}

// $deBlob: FUNCTION 00656650
// $deBlob: FUNCTION 006568a0
// $deBlob: FUNCTION 00656889
ABlobLeaderBoardHud::~ABlobLeaderBoardHud()
{
	AGameStatus::GetSingleton()->SetLeaderBoardHud( TNULL );
}

// $deBlob: FUNCTION 00656680
void ABlobLeaderBoardHud::OnCreated()
{
	for ( TINT i = 0; i < NUM_ROWS; i++ )
	{
		m_apName[ i ]  = FindChildXUIControl( 12 + i );
		m_apPlace[ i ] = FindChildXUIControl( 9 + i );
		m_apScore[ i ] = FindChildXUIControl( 15 + i );

		m_apScore[ i ]->SetText( L"0", -1 );
	}

	AGameStatus::GetSingleton()->SetLeaderBoardHud( this );
}

// $deBlob: FUNCTION 00656700
void ABlobLeaderBoardHud::UpdateRanks()
{
	for ( TINT i = 0; i < NUM_ROWS; i++ )
	{
		const AGameStatus::Rank& rRank = AGameStatus::GetSingleton()->GetRankResult( i );

		m_apPlace[ i ]->SetText( GetLocaleString( s_aiPlaceStrings[ rRank.m_iRank ] ), -1 );
		m_apName[ i ]->SetText( GetLocaleString( s_aiPlayerStrings[ rRank.m_iPlayer ] ), -1 );

		TStringManager::String16Format( m_awszScore[ i ], TARRAYSIZE( m_awszScore[ i ] ), L"%d", rRank.m_iScore );
		m_apScore[ i ]->SetText( m_awszScore[ i ], -1 );
	}
}

// $deBlob: FUNCTION 006567d0
void ABlobLeaderBoardHud::ResetRanks()
{
	for ( TINT i = 0; i < NUM_ROWS; i++ )
	{
		m_apPlace[ i ]->SetText( GetLocaleString( s_iEmptyPlaceString ), -1 );
		m_apName[ i ]->SetText( GetLocaleString( s_aiPlayerStrings[ i ] ), -1 );

		TStringManager::String16Copy( m_awszScore[ i ], L"0", -1 );
		m_apScore[ i ]->SetText( m_awszScore[ i ], -1 );
	}
}
