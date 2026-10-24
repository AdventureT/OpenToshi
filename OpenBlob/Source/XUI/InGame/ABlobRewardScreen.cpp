#include "pch.h"
#include "ABlobRewardScreen.h"
#include "ABlobInGame.h"
#include "GameInterface/ABossEndSeqState.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameController/AGameStatus.h"
#include "Sound/ASoundInterface.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIGroup.h>
#include <XUI/TXUIVisual.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The locale strings of the rewards: the worlds (the first entry is never read), the
// sprint challenges, two per world, the gallery items, the Blob party items and the
// reward types
static const TINT s_aiWorldNames[ 11 ]     = { 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x9A };
static const TINT s_aiChallengeNames[ 20 ] = { 0x86, 0x87, 0x89, 0x88, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99 };
static const TINT s_aiGalleryNames[ 10 ]   = { 0x310, 0x317, 0x314, 0x313, 0x318, 0x31A, 0x311, 0x31E, 0x319, 0x315 };
static const TINT s_aiBlobPartyNames[ 9 ]  = { 0x323, 0x324, 0x325, 0x326, 0x327, 0x328, 0x329, 0x32A, 0x32B };
static const TINT s_aiRewardTypeNames[ 7 ] = { 0xD7, 0xDA, 0xD8, 0xD4, 0xD5, 0xD6, 0x244 };

// The movies of the RewardType_Video rewards (the locale string and the movie)
struct VideoReward
{
	TINT         m_iString;
	const TCHAR* m_szMovie;
};

static const VideoReward s_aVideoRewards[ 10 ] = {
	{ 0x305, "Intro_De_Blob_M" },
	{ 0x307, "Intro_M" },
	{ 0x304, "inky_wii_M" },
	{ 0x353, "zepphead" },
	{ 0x351, "paintribbon" },
	{ 0x356, "wakeywakey" },
	{ 0x357, "BlobTest_ZAttack_Cam2" },
	{ 0x354, "BlobTest_Grapple_Scenario" },
	{ 0x355, "Blob Slam Rear Cam" },
	{ 0x358, "00_ChromaCity" },
};

// $deBlob: FUNCTION 0064a270
// $deBlob: FUNCTION 0064a280
// $deBlob: FUNCTION 0064a3d0
// $deBlob: FUNCTION 0064a3e0
// $deBlob: FUNCTION 0053de50
// $deBlob: FUNCTION 00858b10
TDEFINE_CLASS( ABlobRewardScreen );

// $deBlob: FUNCTION 0064a330
// $deBlob: FUNCTION 0064a3f0
ABlobRewardScreen::ABlobRewardScreen()
    : ABlobMenu( -1 )
{
	m_iNumRewards    = 0;
	m_iCurrentReward = 0;
	m_pElement9      = TNULL;
	m_bLeaving       = TFALSE;
	m_bExit          = TFALSE;
}

// $deBlob: FUNCTION 0064ac20
// $deBlob: FUNCTION 0064ac00
// $deBlob: FUNCTION 0064ac12
ABlobRewardScreen::~ABlobRewardScreen()
{
}

// $deBlob: FUNCTION 0064a680
void ABlobRewardScreen::AddReward( TINT a_eType, TINT a_iIndex )
{
	TASSERT( m_iNumRewards < MAXREWARDS );

	m_aRewards[ m_iNumRewards ].m_eType  = a_eType;
	m_aRewards[ m_iNumRewards ].m_iIndex = a_iIndex;

	switch ( a_eType )
	{
		case RewardType_World:
		case RewardType_Challenge:
			m_aRewards[ m_iNumRewards ].m_eDisplay = RewardDisplay_Normal;
			break;
		case RewardType_WorldMode:
			m_aRewards[ m_iNumRewards ].m_eDisplay = RewardDisplay_Mode;
			break;
		case RewardType_Movie:
		case RewardType_Video:
		case RewardType_Gallery:
			m_aRewards[ m_iNumRewards ].m_eDisplay = RewardDisplay_Movie;
			break;
		case RewardType_BlobParty:
			m_aRewards[ m_iNumRewards ].m_eDisplay = RewardDisplay_BlobParty;
			break;
	}

	m_iNumRewards++;
}

// $deBlob: FUNCTION 0064a790
void ABlobRewardScreen::Tick( TFLOAT a_fDeltaTime )
{

	if ( m_bLeaving && !AGameStateController::GetSingleton()->IsFading( 0 ) && AGameStateController::GetSingleton()->GetFade( 0 ) == 1.0f )
	{
		m_bLeaving = TFALSE;

		if ( !m_bExit )
		{
			// The level restarts and the screen goes back
			AInGameState* pInGameState = TSTATICCAST( AInGameState, AGameStateController::GetSingleton()->FindState( &TGetClass( AInGameState ) ) );
			pInGameState->RestartLevel();

			TXUIScene* pScene = m_pPrevScene;

			while ( pScene != TNULL )
			{
				TXUIScene* pPrev = pScene->GetPrevScene();
				delete pScene;
				pScene = pPrev;
			}

			m_pPrevScene = TNULL;
			TXUI::GetSingleton()->TransitionBack( FindParentScene() );
			m_uiSceneFlags |= 1;
		}
		else
		{
			// The level ends
			AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();

			if ( pState->IsA( &TGetClass( ABossEndSeqState ) ) )
				TSTATICCAST( ABossEndSeqState, pState )->Finish();
			else
			{
				TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
				TSTATICCAST( AInGameState, pState )->EndLevelFromResults();
			}
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0064a8f0
TBOOL ABlobRewardScreen::SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam )
{
	// Message 9 with the name of a bling: the bling and its sparkles play their "Bling" animation
	if ( a_eMessage == XUIMessage_NamedFrame && a_wszParam != TNULL )
	{
		TINT iBling;

		if ( TStringManager::String16Compare( a_wszParam, L"Bling1" ) == 0 )
			iBling = 0;
		else if ( TStringManager::String16Compare( a_wszParam, L"Bling2" ) == 0 )
			iBling = 1;
		else if ( TStringManager::String16Compare( a_wszParam, L"Bling3" ) == 0 )
			iBling = 2;
		else
			return TXUIControl::SendMessage( a_eMessage, a_wszParam );

		m_apBlingElements[ iBling ]->StopAnimation();
		m_apBlingElements[ iBling ]->PlayAnimation( L"Bling", L"EndBling" );

		for ( TINT i = 0; i < MAXSPARKLES; i++ )
			m_apSparkles[ iBling ][ i ]->GetVisual()->PlayAnimation( L"Bling", L"EndBling" );

		return TTRUE;
	}

	return TXUIControl::SendMessage( a_eMessage, a_wszParam );
}

// $deBlob: FUNCTION 0064a440
void ABlobRewardScreen::OnCreated()
{
	ABlobMenu::OnCreated();

	m_pElement9 = FindChildXUIElement( 9 );

	for ( TINT i = 0; i < MAXBLINGS; i++ )
	{
		m_apBlings[ i ]        = FindChildXUIControl( 10 + i );
		m_apItemTypes[ i ]     = m_apBlings[ i ]->GetVisual()->FindChildXUIControl( L"ItemType" );
		m_apDescriptions[ i ]  = m_apBlings[ i ]->GetVisual()->FindChildXUIControl( L"Description" );
		m_apBlingElements[ i ] = m_apBlings[ i ]->GetVisual()->FindChildXUIElement( L"Bling" );

		TWCHAR* wszName = TStringManager::GetTempString16();

		for ( TINT j = 0; j < MAXSPARKLES; j++ )
		{
			if ( j == 0 )
			{
				m_apSparkles[ i ][ j ] = m_apBlingElements[ i ]->FindChildXUIControl( L"Sparkle" );
			}
			else
			{
				TStringManager::String16Format( wszName, L"Sparkle%d", j );
				m_apSparkles[ i ][ j ] = m_apBlingElements[ i ]->FindChildXUIControl( wszName );
			}
		}
	}
}

// $deBlob: FUNCTION 0064a5a0
TBOOL ABlobRewardScreen::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_eButton == XUIButton_Accept && !TSTATICCAST( TXUIGroup, m_pElement9 )->GetTimelineControl()->IsPlaying() )
	{
		// More rewards to show
		if ( m_iCurrentReward < m_iNumRewards )
		{
			ShowNextRewards();
			return TXUIScene::HandleButton( XUIButton_Accept, a_iController );
		}

		// A sprint goes on to its game over screen
		if ( AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel != 0 )
		{
			TXUI*        pXUI      = TXUI::GetSingleton();
			TXUIScene*   pGameOver = pXUI->CreateScene( pXUI->FindResource( "sprintgameover.xur" ), TUINT32( 0 ) );
			TXUIElement* pParent   = FindParentControl();

			if ( pParent != TNULL )
				pParent->AddChildTail( pGameOver );

			pXUI->TransitionTo( this, pGameOver );
			return TTRUE;
		}

		// Fades out and ends the level
		AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TTRUE );
		AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
		m_bLeaving = TTRUE;
		m_bExit    = TTRUE;
	}

	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// $deBlob: FUNCTION 0064a540
void ABlobRewardScreen::EndTransTo()
{
	// NOTE: the parent scene is the in-game scene
	TXUIScene* pParent = FindParentScene();

	if ( pParent != TNULL )
		TSTATICCAST( ABlobInGame, pParent )->PlayBackgroundMovie( L"POSTGAME_REWARDS_LOOP.bik", TTRUE );

	TASSERT( m_iNumRewards > 0 );

	ShowNextRewards();
	ABlobMenu::EndTransTo();
}

// $deBlob: FUNCTION 0064a9c0
void ABlobRewardScreen::ShowNextRewards()
{

	for ( TINT i = 0; i < MAXBLINGS; i++ )
	{
		if ( m_iNumRewards <= m_iCurrentReward )
		{
			m_apBlings[ i ]->SetVisible( TFALSE );
			continue;
		}

		m_apBlings[ i ]->SetVisible( TTRUE );

		const TWCHAR* wszName  = TNULL;
		const TWCHAR* wszStart = TNULL;
		const TWCHAR* wszEnd   = TNULL;
		Reward&       rReward  = m_aRewards[ m_iCurrentReward ];

		switch ( rReward.m_eType )
		{
			case RewardType_World:
			case RewardType_WorldMode:
				wszName  = T2Locale::GetSingleton()->GetString( s_aiWorldNames[ rReward.m_iIndex ] );
				wszStart = L"Normal";
				wszEnd   = L"EndNormal";
				break;
			case RewardType_Challenge:
				wszName  = T2Locale::GetSingleton()->GetString( s_aiChallengeNames[ rReward.m_iIndex ] );
				wszStart = L"Normal";
				wszEnd   = L"EndNormal";
				break;
			case RewardType_Movie:
			{
				// The movies of the worlds (0: the intro, 11: the ending)
				TINT iString;

				if ( rReward.m_iIndex == 0 )
					iString = 0x335;
				else if ( rReward.m_iIndex == 11 )
					iString = 0x336;
				else
					iString = s_aiWorldNames[ rReward.m_iIndex - 1 ];

				wszName  = T2Locale::GetSingleton()->GetString( iString );
				wszStart = L"Movie";
				wszEnd   = L"Movie";
				break;
			}
			case RewardType_Video:
				wszName  = T2Locale::GetSingleton()->GetString( s_aVideoRewards[ rReward.m_iIndex ].m_iString );
				wszStart = L"Movie";
				wszEnd   = L"Movie";
				break;
			case RewardType_Gallery:
				wszName  = T2Locale::GetSingleton()->GetString( s_aiGalleryNames[ rReward.m_iIndex ] );
				wszStart = L"Gallery";
				wszEnd   = L"Gallery";
				break;
			case RewardType_BlobParty:
				wszName = T2Locale::GetSingleton()->GetString( s_aiBlobPartyNames[ rReward.m_iIndex ] );
				m_apBlings[ i ]->GetVisual()->PlayAnimation( L"BlobParty", L"BlobParty" );
				break;
		}

		if ( wszStart != TNULL )
			m_apBlings[ i ]->GetVisual()->PlayAnimation( wszStart, wszEnd );

		m_apItemTypes[ i ]->SetText( T2Locale::GetSingleton()->GetString( s_aiRewardTypeNames[ rReward.m_eType ] ), -1 );
		TStringManager::String16Format( m_awszDescriptions[ i ], 0x100, L"%ls", wszName );
		m_apDescriptions[ i ]->SetText( m_awszDescriptions[ i ], -1 );

		ASoundInterface::PlayEvent( ASoundMap::GROUPID( 4 ), ASoundMap::EVENTID( 0x54 ), FMOD_EVENT_DEFAULT, TTRUE );
		m_iCurrentReward++;
	}

	m_pElement9->PlayAnimation( L"Display", L"EndDisplay" );
}
