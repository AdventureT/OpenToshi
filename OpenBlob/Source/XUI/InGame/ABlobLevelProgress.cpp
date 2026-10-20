#include "pch.h"
#include "ABlobLevelProgress.h"
#include "Sound/ASoundInterface.h"
#include "GameController/AGameStatus.h"
#include "MemoryCard/EnSaveData.h"
#include "Challenges/AChallengeManager.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "XUI/InGame/HUD/ABlobGoalHud.h"

#include <XUI/TXUIVisual.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00646090
// $deBlob: FUNCTION 006460a0
// $deBlob: FUNCTION 006461a0
// $deBlob: FUNCTION 006461b0
// $deBlob: FUNCTION 0053dd10
// $deBlob: FUNCTION 00858ac0
TDEFINE_CLASS( ABlobLevelProgress );

// $deBlob: FUNCTION 00646ca0
// $deBlob: FUNCTION 00646120
ABlobLevelProgress::ABlobLevelProgress()
    : ABlobMenu( -1 )
{
}

// $deBlob: FUNCTION 00646ce0
// $deBlob: FUNCTION 00646cc0
// $deBlob: FUNCTION 00646cd2
ABlobLevelProgress::~ABlobLevelProgress()
{
}

// The names of the worlds by SPLevel (the same table as ABlobLevelSelection's)
static const TINT s_aiWorldNames[ 10 ] = { 124, 125, 126, 127, 128, 129, 130, 131, 132, 133 };

// Sets the title (the locale string a_iTitle) of the goal control a_pControl and returns its medal element "gp"
static TXUIElement* SetupGoalControl( TXUIControl* a_pControl, TINT a_iTitle )
{
	TXUIControl* pTitle = a_pControl->GetVisual()->FindChildXUIControl( L"Title" );
	pTitle->SetText( T2Locale::GetSingleton()->GetString( a_iTitle ), -1 );

	return a_pControl->GetVisual()->FindChildXUIElement( L"gp" );
}

// Sets the counter of the goal control a_pControl: "a_iCount/a_iTotal" in a_wszBuffer, "-" when the level has none
static void SetGoalCounter( TXUIControl* a_pControl, TWCHAR* a_wszBuffer, TINT a_iCount, TINT a_iTotal )
{
	TXUIControl* pCounter = a_pControl->GetVisual()->FindChildXUIControl( L"100percent" );

	if ( a_iTotal <= 0 )
	{
		pCounter->SetText( L"-", -1 );
	}
	else
	{
		TStringManager::String16Format( a_wszBuffer, L"%d/%d", a_iCount, a_iTotal );
		pCounter->SetText( a_wszBuffer, -1 );
	}
}

// $deBlob: FUNCTION 006461c0
void ABlobLevelProgress::OnCreated()
{
	// The award controls and their awarded flags
	for ( TINT i = 0; i < MAXAWARDS; i++ )
	{
		m_abAwarded[ i ] = TFALSE;

		TWCHAR wszName[ 0x40 ];
		T2String16::Format( wszName, 0x40, L"Award%d", i + 1 );
		m_apAwards[ i ] = FindChildXUIControl( wszName );
	}

	ABlobMenu::OnCreated();

	TASSERT( -1 != AGameStatus::GetSingleton()->GetSPLevel() );

	TXUIControl* pLevelName = FindChildXUIControl( 8 );

	if ( AGameStatus::GetSingleton()->GetSPLevel() != AGameStatus::SPLevel_Invalid )
		pLevelName->SetText( T2Locale::GetSingleton()->GetString( s_aiWorldNames[ AGameStatus::GetSingleton()->GetSPLevel() ] ), -1 );

	AGameStatus::LevelData&  rLevelData  = AGameStatus::GetSingleton()->GetLevelData();
	AGameStatus::PlayerData& rPlayerData = rLevelData.m_aPlayerData[ 0 ];
	TUINT32                  uiAwards    = rPlayerData.m_auiUnk3C[ 0 ];

	// Paint: the medal of the first paint award found, the paint percentage
	TXUIControl* pControl = FindChildXUIControl( 0x13 );
	TXUIElement* pMedal   = SetupGoalControl( pControl, 0x47 );

	if ( ( uiAwards & EnSaveData::AWARD_Paint25 ) != 0 )
	{
		m_abAwarded[ 0 ] = TTRUE;
		pMedal->PlayAnimation( L"MedalPaint25On", L"MedalPaint25On" );
	}
	else if ( ( uiAwards & EnSaveData::AWARD_Paint50 ) != 0 )
	{
		m_abAwarded[ 0 ] = TTRUE;
		pMedal->PlayAnimation( L"MedalPaint50On", L"MedalPaint50On" );
	}
	else if ( ( uiAwards & EnSaveData::AWARD_Paint75 ) != 0 )
	{
		m_abAwarded[ 0 ] = TTRUE;
		pMedal->PlayAnimation( L"MedalPaint75On", L"MedalPaint75On" );
	}
	else if ( ( uiAwards & EnSaveData::AWARD_Paint100 ) == 0 )
	{
		pMedal->PlayAnimation( L"MedalPaintOff", L"MedalPaintOff" );
	}
	else
	{
		m_abAwarded[ 0 ] = TTRUE;
		pMedal->PlayAnimation( L"MedalPaint100On", L"MedalPaint100On" );
	}

	TXUIControl* pPercent = pControl->GetVisual()->FindChildXUIControl( L"100percent" );

	if ( TINT( rLevelData.m_auiLevelCounts[ 0 ] ) <= 0 )
	{
		pPercent->SetText( L"-", -1 );
	}
	else
	{
		TStringManager::String16Format( m_wszPercent, L"%d%%", TINT( rLevelData.GetPaintPercentage( 0 ) ) );
		pPercent->SetText( m_wszPercent, -1 );
	}

	// Trees
	pControl = FindChildXUIControl( 0x14 );
	pMedal   = SetupGoalControl( pControl, 0x77 );

	if ( ( uiAwards & EnSaveData::AWARD_Trees ) != 0 )
	{
		m_abAwarded[ 1 ] = TTRUE;
		pMedal->PlayAnimation( L"MedalTreesOn", L"MedalTreesOn" );
	}
	else
	{
		pMedal->PlayAnimation( L"MedalTreesOff", L"MedalTreesOff" );
	}

	SetGoalCounter( pControl, m_wszTrees, rPlayerData.m_auiProgress[ 0 ], rLevelData.m_auiLevelCounts[ 1 ] );

	// Raydians (the liberated civilians)
	pControl = FindChildXUIControl( 0x15 );
	pMedal   = SetupGoalControl( pControl, 0xAC );

	if ( ( uiAwards & EnSaveData::AWARD_Raydian ) != 0 )
	{
		m_abAwarded[ 2 ] = TTRUE;
		pMedal->PlayAnimation( L"MedalRaydianOn", L"MedalRaydianOn" );
	}
	else
	{
		pMedal->PlayAnimation( L"MedalRaydianOff", L"MedalRaydianOff" );
	}

	SetGoalCounter( pControl, m_wszRaydian, rPlayerData.m_auiProgress[ 2 ], rLevelData.m_auiLevelCounts[ 6 ] );

	// Bill boards
	pControl = FindChildXUIControl( 0x16 );
	pMedal   = SetupGoalControl( pControl, 0x7A );

	if ( ( uiAwards & EnSaveData::AWARD_Bill ) != 0 )
	{
		pMedal->PlayAnimation( L"MedalBillOn", L"MedalBillOn" );
		m_abAwarded[ 3 ] = TTRUE;
	}
	else
	{
		pMedal->PlayAnimation( L"MedalBillOff", L"MedalBillOff" );
	}

	SetGoalCounter( pControl, m_wszBill, rPlayerData.m_auiProgress[ 5 ], rLevelData.m_auiLevelCounts[ 3 ] );

	// Landmarks
	pControl = FindChildXUIControl( 0x10 );
	pMedal   = SetupGoalControl( pControl, 0x79 );

	if ( ( uiAwards & EnSaveData::AWARD_Landmarks ) != 0 )
	{
		pMedal->PlayAnimation( L"MedalLandmarksOn", L"MedalLandmarksOn" );
		m_abAwarded[ 4 ] = TTRUE;
	}
	else
	{
		pMedal->PlayAnimation( L"MedalLandmarksOff", L"MedalLandmarksOff" );
	}

	SetGoalCounter( pControl, m_wszLandmarks, rPlayerData.m_auiProgress[ 4 ], rLevelData.m_auiLevelCounts[ 2 ] );

	// Transforms (the grass bombs)
	pControl = FindChildXUIControl( 0x0F );
	pMedal   = SetupGoalControl( pControl, 0x7B );

	if ( ( uiAwards & EnSaveData::AWARD_Transform ) != 0 )
	{
		m_abAwarded[ 5 ] = TTRUE;
		pMedal->PlayAnimation( L"MedalTransformOn", L"MedalTransformOn" );
	}
	else
	{
		pMedal->PlayAnimation( L"MedalTransformOff", L"MedalTransformOff" );
	}

	SetGoalCounter( pControl, m_wszTransform, rPlayerData.m_auiProgress[ 6 ], rLevelData.m_auiLevelCounts[ 4 ] );

	// Challenges (without the null and the sub challenges)
	pControl = FindChildXUIControl( 0x11 );
	pMedal   = SetupGoalControl( pControl, 0x3C );

	if ( ( uiAwards & EnSaveData::AWARD_Challenge ) != 0 )
	{
		pMedal->PlayAnimation( L"MedalChallengeOn", L"MedalChallengeOn" );
		m_abAwarded[ 6 ] = TTRUE;
	}
	else
	{
		pMedal->PlayAnimation( L"MedalChallengeOff", L"MedalChallengeOff" );
	}

	SetGoalCounter(
	    pControl,
	    m_wszCounter140,
	    AChallengeManager::GetSingleton()->GetTotalCompletedChallenges(),
	    AChallengeManager::GetSingleton()->GetTotalNumChallenges() - AChallengeManager::GetSingleton()->GetChallengesData()->m_uiNumNull - AChallengeManager::GetSingleton()->GetNumSubChallenges()
	);

	// Styles
	pControl = FindChildXUIControl( 0x12 );
	pMedal   = SetupGoalControl( pControl, 0x3D );

	if ( ( uiAwards & EnSaveData::AWARD_Styles ) != 0 )
	{
		pMedal->PlayAnimation( L"MedalStylesOn", L"MedalStylesOn" );
		m_abAwarded[ 7 ] = TTRUE;
	}
	else
	{
		pMedal->PlayAnimation( L"MedalStylesOff", L"MedalStylesOff" );
	}

	SetGoalCounter( pControl, m_wszStyles, rPlayerData.m_auiUnk3C[ 2 ], rLevelData.m_auiUnk160[ 0 ] );

	// The time left (minutes:seconds)
	TXUIControl* pTime    = FindChildXUIControl( 0x18 );
	TINT         iMinutes = TINT( TFLOAT( rLevelData.m_fTimeLeft / 60.0 ) );
	TINT         iSeconds = TINT( TFLOAT( fmod( TDOUBLE( rLevelData.m_fTimeLeft ), 60.0 ) ) );

	if ( iMinutes != 0 )
		TStringManager::String16Format( m_wszTime, 0x10, ( iSeconds >= 10 ) ? L"%d:%d" : L"%d:0%d", iMinutes, iSeconds );
	else
		TStringManager::String16Format( m_wszTime, 0x10, ( iSeconds >= 10 ) ? L"0:%d" : L"0:0%d", iSeconds );

	pTime->SetText( m_wszTime, -1 );

	// The points
	TXUIControl* pPoints = FindChildXUIControl( 0x19 );
	TStringManager::String16Format( m_wszNumber2A0, L"%d", rLevelData.GetPoints( 0 ) );
	pPoints->SetText( m_wszNumber2A0, -1 );

	// The gates left: the goal HUD's gate still counts while its rise hasn't ended
	TXUIControl* pGates     = FindChildXUIControl( 0x17 );
	TINT         iGatesLeft = TINT( rLevelData.m_uiUnk780 - rLevelData.m_uiUnk774 ) - 1;
	TASSERT( iGatesLeft >= 0 );

	if ( ABlobHud::GetSingleton() != TNULL )
	{
		ABlobGoalHud* pGoalHud = TSTATICCAST( ABlobGoalHud, ABlobHud::GetSingleton()->GetGoalHud() );

		if ( pGoalHud != TNULL && !pGoalHud->GetUnk280() )
			iGatesLeft++;
	}

	TASSERT( iGatesLeft >= 0 );
	TStringManager::String16Format( m_wszGatesLeft, L"%d", iGatesLeft );
	pGates->SetText( m_wszGatesLeft, -1 );

	// The level's medal
	TXUIElement* pMedals = FindChildXUIControl( 0x1B )->GetVisual()->FindChildXUIElement( L"Medals" );

	if ( ( uiAwards & EnSaveData::AWARD_Bronze ) != 0 )
		pMedals->PlayAnimation( L"Bronze", L"Bronze" );
	else if ( ( uiAwards & EnSaveData::AWARD_Silver ) != 0 )
		pMedals->PlayAnimation( L"Silver", L"Silver" );
	else if ( ( uiAwards & EnSaveData::AWARD_Gold ) != 0 )
		pMedals->PlayAnimation( L"Gold", L"Gold" );
}

// Writes a time as "m:ss" to a_wszBuffer
// $deBlob: FUNCTION 00646c40
static TINT FormatTime( TINT a_iMinutes, TINT a_iSeconds, TWCHAR* a_wszBuffer, TSIZE a_uiBufferCount )
{
	if ( a_iMinutes != 0 )
		return TStringManager::String16Format( a_wszBuffer, TINT( a_uiBufferCount ), ( a_iSeconds >= 10 ) ? L"%d:%d" : L"%d:0%d", a_iMinutes, a_iSeconds );

	return TStringManager::String16Format( a_wszBuffer, TINT( a_uiBufferCount ), ( a_iSeconds >= 10 ) ? L"0:%d" : L"0:0%d", a_iSeconds );
}

// $deBlob: FUNCTION 00646be0
TBOOL ABlobLevelProgress::OnNotifySetFocus( Toshi::TXUIElement* a_pSource )
{
	// The award controls: the sound of a flagged award (group 4, event 0x4D) or of an unflagged one (event 0x4F)
	for ( TINT i = 0; i < MAXAWARDS; i++ )
	{
		if ( a_pSource == m_apAwards[ i ] )
		{
			if ( m_abAwarded[ i ] )
				ASoundInterface::PlayEvent( ASoundMap::GROUPID_GUI, ASoundMap::EVENTID( 0x4D ), FMOD_EVENT_DEFAULT, TTRUE );
			else
				ASoundInterface::PlayEvent( ASoundMap::GROUPID_GUI, ASoundMap::EVENTID( 0x4F ), FMOD_EVENT_DEFAULT, TTRUE );

			return TFALSE;
		}
	}

	return ABlobMenu::OnNotifySetFocus( a_pSource );
}
