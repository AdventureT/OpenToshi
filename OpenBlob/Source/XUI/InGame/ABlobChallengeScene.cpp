#include "pch.h"
#include "ABlobChallengeScene.h"
#include "Sound/AStreamingSoundPlayer.h"
#include "XUI/ABlobConfirmationMenu.h"
#include "ARandom.h"
#include "ARevolutionary/ARevolutionary.h"
#include "Challenges/AChallengeManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobRibbonTrailComponent.h"
#include "Entities/ABlobEntity/ABlobSoundComponent.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/APlayerManager.h"
#include "GUI/AGUIManager.h"
#include "Input/ABlobInputMap.h"
#include "Locale/ALocaleManager.h"
#include "Render/PaintParticleSystem/APaintParticleSystem.h"
#include "Sound/AMusicManager.h"
#include "XUI/ABlobControllerPopup.h"
#include "XUI/ElementTypes/ABlobTextReveal.h"
#include "XUI/InGame/HUD/ABlobHelpScene.h"
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIBackButton.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The controller popup of the in-game state ignores the XUI input lock while the prompt waits for the marker
// $deBlob: FUNCTION 00644260
static void ABlobChallengeScene_SetPopupIgnoreInputLock( TBOOL a_bIgnore )
{
	AInGameState* pInGameState = TSTATICCAST( AInGameState, AGameStateController::GetSingleton()->FindState( &TGetClass( AInGameState ) ) );

	if ( pInGameState != TNULL && pInGameState->GetControllerPopup() != TNULL )
		TSTATICCAST( ABlobControllerPopup, pInGameState->GetControllerPopup() )->SetIgnoreInputLock( a_bIgnore );
}

// $deBlob: FUNCTION 00642d90
// $deBlob: FUNCTION 00642da0
// $deBlob: FUNCTION 00642e10
// $deBlob: FUNCTION 00642e80
// $deBlob: FUNCTION 00642e90
// $deBlob: FUNCTION 0053dc10
// $deBlob: FUNCTION 00858a80
TDEFINE_CLASS( ABlobChallengeScene );

// $deBlob: FUNCTION 00642ea0
ABlobChallengeScene::ABlobChallengeScene()
    : ABlobMenu( -1 )
{
	m_pYesButton        = TNULL;
	m_pNoButton         = TNULL;
	m_pOkButton         = TNULL;
	m_pText             = TNULL;
	m_iNumPages         = 0;
	m_iCurrentPage      = 0;
	m_bConfirmed        = TFALSE;
	m_bHasPages         = TFALSE;
	m_bCompleteOnAccept = TFALSE;
	m_bTextDone         = TFALSE;
	m_bWaitingForCamera = TFALSE;
	m_pElement9         = TNULL;
	m_pDetails          = TNULL;
	m_pButtons          = TNULL;
}

// $deBlob: FUNCTION 00642f20
// $deBlob: FUNCTION 00644490
// $deBlob: FUNCTION 00644486
ABlobChallengeScene::~ABlobChallengeScene()
{
	ABlobChallengeScene_SetPopupIgnoreInputLock( TFALSE );

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

	if ( !( pBlob->m_uiFlags & 0x200000 ) )
		AMusicManager::GetSingleton()->ReleaseSnapshot();
	else
		AMusicManager::GetSingleton()->PlaySnapshot( "rainbow" );

	// Stops the revolutionaries' speech
	AStreamingSoundPlayer::GetSingleton()->StopAll();

	AChallengeManager::GetSingleton()->m_bWaitingForPrompt = TFALSE;
	AChallengeManager::GetSingleton()->m_pChallengeScene   = TNULL;

	pBlob->SetVisible( TTRUE );
	AChallengeManager::GetSingleton()->m_bPromptShown = TFALSE;

	for ( TINT i = 0; i < m_iNumPages; i++ )
		TFree( m_apPages[ i ] );

	APaintParticleSystem::s_Instance->SetEnabled( TTRUE );
	AGUIManager::GetSingleton()->SetCursorVisible( TFALSE, TTRUE );
}

// $deBlob: FUNCTION 00643060
void ABlobChallengeScene::OnCreated()
{
	AMusicManager::GetSingleton()->PlayConversation();

	ALocaleManager* pLocale = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );

	m_pYesButton = FindChildXUIControl( 5 );
	m_pNoButton  = FindChildXUIControl( 4 );
	m_pOkButton  = FindChildXUIControl( 6 );
	m_pOkButton->SetText( pLocale->GetString( 3 ), -1 );
	m_pText      = TSTATICCAST( ABlobTextReveal, FindChildXUIControl( 10 ) );
	m_pElement11 = FindChildXUIElement( 11 );
	m_pElement9  = FindChildXUIElement( 9 );
	m_pDetails   = FindChildXUIElement( 20 );
	m_pDetails->SetVisible( TFALSE );
	m_pButtons = FindChildXUIElement( 3 );
	m_pButtons->SetVisible( TFALSE );

	AChallenge*       pChallenge = AChallengeManager::GetSingleton()->m_pNearbyChallenge;
	AChallenge::Data* pData      = pChallenge->GetData();

	// The icon of the challenge's type
	TXUIControl*  pIcon   = FindChildXUIControl( 22 );
	const TWCHAR* wszIcon = TNULL;

	switch ( pChallenge->GetType() )
	{
		case AChallenge::Type_Paint:
		case AChallenge::Type_Civilian:
		case AChallenge::Type_Car:
			wszIcon = L"Paint";
			break;
		case AChallenge::Type_Landmark:
			wszIcon = L"Landmark";
			break;
		case AChallenge::Type_Combat:
			wszIcon = L"Combat";
			break;
		case AChallenge::Type_Race:
			wszIcon = L"Race";
			break;
		default:
			break;
	}

	if ( wszIcon != TNULL )
		pIcon->GetVisual()->PlayAnimation( wszIcon, wszIcon );

	// The stars of its difficulty
	TXUIControl* pStars = FindChildXUIControl( 21 );
	TUINT32      uiStars;

	if ( pData->m_uiFlags & AChallenge::Data::Flags_DifficultyEasy )
		uiStars = 0;
	else if ( pData->m_uiFlags & AChallenge::Data::Flags_DifficultyMedium )
		uiStars = 1;
	else
		uiStars = ( ( pData->m_uiFlags & 0xFF ) >> 2 ) & 2;

	if ( uiStars == 0 )
		pStars->GetVisual()->PlayAnimation( L"Show1Star", L"EndShow1Star" );
	else if ( uiStars == 1 )
		pStars->GetVisual()->PlayAnimation( L"Show2Stars", L"EndShow2Stars" );
	else if ( uiStars == 2 )
		pStars->GetVisual()->PlayAnimation( L"Show3Stars", L"EndShow3Stars" );

	// The time limit
	TXUIControl* pTime = FindChildXUIControl( 23 );

	if ( pData->m_iTimeLimit == -1 )
		TStringManager::String16Format( m_wszTimeLimit, 0x80, pLocale->GetString( 0xBC ) );
	else
		TStringManager::String16Format( m_wszTimeLimit, 0x80, pLocale->GetString( 0xBD ), TINT( pData->m_iTimeLimit ) );

	pTime->SetText( m_wszTimeLimit, -1 );

	// The revolutionary's text
	if ( pChallenge->IsAlreadyCompleted() == TTRUE )
	{
		m_bCompleteOnAccept = TTRUE;
		TINT iStringID      = TINT( pData->m_auiStringIDs[ 1 ] );

		if ( iStringID == -1 )
		{
			TINT iRandom = ARandom::GetSingleton()->GetRandom().GetInt( 8 );

			if ( pChallenge->GetType() == AChallenge::Type_Combat )
			{
				TINT aiStrings[ 8 ] = { 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB };
				iStringID           = aiStrings[ iRandom ];
			}
			else
			{
				TINT aiStrings[ 8 ] = { 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67 };
				iStringID           = aiStrings[ iRandom ];
			}
		}

		m_pText->SetText( pLocale->GetString( iStringID ), -1 );
		ALocaleManager::StringFormat* pFormat = pLocale->GetStringFormat( iStringID );

		AStreamingSoundPlayer::GetSingleton()->PlayVO( iStringID, 1, pChallenge->GetRevolutionary(), "Talk_Overlay_1" ); // the speech

		if ( pFormat == TNULL )
			m_pText->SetText2( TNULL, 0 );
		else
			m_pText->SetText2( pLocale->GetOverwrittenFormatString( iStringID, pFormat->m_wszFormat ), pLocale->GetFormatUnk04() );
	}
	else if ( pData->m_iStringID == -1 )
	{
		if ( pChallenge->GetType() == AChallenge::Type_Null || !( pData->m_uiFlags & AChallenge::Data::Flags_CanQuit ) )
			ShowOkButton();
		else
			ShowYesNoButtons();
	}
	else
	{
		const TWCHAR* wszText = pLocale->GetString( pData->m_iStringID );
		m_bHasPages           = TStringManager::String16FindChar( wszText, L'*' ) != TNULL;

		AStreamingSoundPlayer::GetSingleton()->PlayVO( pData->m_iStringID, 1, pChallenge->GetRevolutionary(), "Talk_Overlay_1" ); // the speech

		if ( m_bHasPages )
		{
			// The pages: the text between the '*'
			m_iNumPages = 1;

			for ( const TWCHAR* wszSeparator = TStringManager::String16FindChar( wszText, L'*' ); wszSeparator != TNULL; wszSeparator = TStringManager::String16FindChar( wszSeparator + 1, L'*' ) )
			{
				m_aiPageOffsets[ m_iNumPages ] = TINT16( ( wszSeparator - wszText ) + 1 );
				m_iNumPages++;
			}

			TASSERT( m_iNumPages < MAXPAGES );

			ABlobHelpScene::SplitString( wszText, L"*", m_apPages );
			m_iCurrentPage = 0;
			m_pText->SetText( m_apPages[ 0 ], -1 );

			ALocaleManager::StringFormat* pFormat = pLocale->GetStringFormat( pData->m_iStringID );

			if ( pFormat == TNULL )
				m_pText->SetText2( TNULL, 0 );
			else
				m_pText->SetText2( pLocale->GetOverwrittenFormatString( pData->m_iStringID, pFormat->m_wszFormat ), pLocale->GetFormatUnk04() );

			m_pElement11->SetVisible( TTRUE );
		}
		else
		{
			m_pText->SetText( wszText, -1 );
			ALocaleManager::StringFormat* pFormat = pLocale->GetStringFormat( pData->m_iStringID );

			if ( pFormat == TNULL )
				m_pText->SetText2( TNULL, 0 );
			else
				m_pText->SetText2( pLocale->GetOverwrittenFormatString( pData->m_iStringID, pFormat->m_wszFormat ), pLocale->GetFormatUnk04() );
		}
	}

	// The game waits behind the prompt
	AChallengeManager::GetSingleton()->m_bWaitingForPrompt = TTRUE;
	AChallengeManager::GetSingleton()->m_pChallengeScene   = this;

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	pBlob->SetVisible( TFALSE );
	pBlob->m_pRibbonTrail->m_uiFlags &= ~ABlobRibbonTrailComponent::FLAGS_VISIBLE;
	pBlob->m_pSoundComponent->SetSoundsEnabled( TFALSE );
	pBlob->SetLevelStartAttached( TTRUE );

	APaintParticleSystem::s_Instance->SetEnabled( TFALSE );
	AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TTRUE;
	AChallengeManager::GetSingleton()->m_bPromptShown     = TTRUE;
	m_iChoice                                             = Choice_None;
	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );

	// No back button if the challenge can't be refused
	if ( !( pData->m_uiFlags & AChallenge::Data::Flags_CanQuit ) && m_pBackButton != TNULL )
		m_pBackButton->SetEnabled( TFALSE );
}

// $deBlob: FUNCTION 00643670
TBOOL ABlobChallengeScene::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	AChallengeManager* pManager   = AChallengeManager::GetSingleton();
	AChallenge*        pChallenge = pManager->m_pNearbyChallenge;

	if ( m_pButtons->IsVisible() )
		PlayPressSound( a_pSource );

	TXUIControl* pSource = TSTATICCAST( TXUIControl, a_pSource );

	if ( ( a_pSource == m_pYesButton || a_pSource == m_pOkButton ) && m_iChoice == Choice_None )
	{
		if ( m_bCompleteOnAccept )
		{
			m_iChoice = Choice_Complete;
		}
		else if ( pManager->m_pCurrentChallenge == TNULL || pChallenge->GetType() == AChallenge::Type_Null )
		{
			m_iChoice = Choice( pManager->m_pNearbyChallenge->GetType() != AChallenge::Type_Null );
		}
		else
		{
			// Another challenge runs: quit it first
			if ( pChallenge->GetData()->m_uiFlags & AChallenge::Data::Flags_CanQuit )
			{
				TXUI*                  pXUI          = TXUI::GetSingleton();
				ABlobConfirmationMenu* pConfirmation = TSTATICCAST( ABlobConfirmationMenu, pXUI->CreateScene( pXUI->FindResource( "confirmation.xur" ), L"Confirmation", TFALSE ) );
				TXUIScene*             pParent       = FindParentScene();

				if ( pParent != TNULL )
				{
					pConfirmation->RemoveSelf();
					pParent->AddChildTail( pConfirmation );
				}

				pConfirmation->SetParentMenu( this );
				pConfirmation->SetTitle( T2Locale::GetSingleton()->GetString( 0xB7 ) );
				pConfirmation->SetText( T2Locale::GetSingleton()->GetString( 0xBB ) );
				pConfirmation->SetPosition( pConfirmation->GetPositionX() * 2.0f, pConfirmation->GetPositionY() * 2.0f );

				AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
				TASSERT( pState->IsA( &TGetClass( AInGameState ) ) );
				TSTATICCAST( AInGameState, AGameStateController::GetSingleton()->GetCurrentGameState() )->SetPaused( TTRUE );

				pXUI->TransitionToPopup( this, pConfirmation );
				return TTRUE;
			}

			pManager->QuitChallenge( TFALSE );
			m_iChoice = Choice_Start;
		}

		SetEnabled( TFALSE );
		pChallenge->GetRevolutionary()->HideStartPrompt();
		PlayAnimation( L"SlideOut", L"EndSlideOut" );
		pSource->SetEnabled( TFALSE );
		return TTRUE;
	}

	if ( a_pSource != m_pNoButton || m_iChoice != Choice_None )
		return TFALSE;

	m_iChoice = Choice_Cancel;
	SetEnabled( TFALSE );
	pChallenge->GetRevolutionary()->HideStartPrompt();
	PlayAnimation( L"SlideOut", L"EndSlideOut" );
	pSource->SetEnabled( TFALSE );
	return TTRUE;
}

// $deBlob: FUNCTION 006439d0
void ABlobChallengeScene::Tick( TFLOAT a_fDeltaTime )
{
	TXUI::GetSingleton()->TickInput( a_fDeltaTime );

	AChallengeManager* pManager = AChallengeManager::GetSingleton();

	if ( !m_bWaitingForCamera )
	{
		if ( m_iChoice != Choice_None )
		{
			ABlobChallengeScene_SetPopupIgnoreInputLock( TTRUE );

			AChallenge* pChallenge = pManager->m_pNearbyChallenge;

			// Once the marker's prompt is gone
			if ( pChallenge->GetRevolutionary()->IsIdle() )
			{
				if ( m_iChoice == Choice_Complete )
				{
					pManager->OnChallengeCompleted( pChallenge, m_bCompleteOnAccept );
					pManager->EndChallenge( pChallenge, TFALSE, TFALSE );
				}
				else if ( m_iChoice == Choice_Replace )
				{
					pManager->QuitChallenge( TTRUE );
				}
				else if ( m_iChoice == Choice_Cancel )
				{
					pManager->HideNearbyPrompt();
				}

				ABlobChallengeScene_SetPopupIgnoreInputLock( TFALSE );
				ResumeGame();

				if ( m_iChoice == Choice_Start || m_iChoice == Choice_Replace )
				{
					// The challenge's camera first
					APlayerManager::GetSingleton()->PlaySSCamera( 0, pChallenge->GetData()->m_iSSCamera );
					if ( APlayerManager::GetSingleton()->GetBlobData( 0 )->IsKeyFrameCameraActive() )
					{
						m_bWaitingForCamera = TTRUE;
						pChallenge->GetRevolutionary()->HideMarker();
						TXUIScene::Tick( a_fDeltaTime );
						return;
					}

					pManager->StartChallenge();
				}
				else if ( m_iChoice == Choice_Complete )
				{
					if ( APlayerManager::GetSingleton()->GetBlobData( 0 )->IsKeyFrameCameraActive() )
					{
						m_bWaitingForCamera = TTRUE;
						TXUIScene::Tick( a_fDeltaTime );
						return;
					}
				}

				pChallenge->GetRevolutionary()->Refresh( TFALSE );
				TXUI::GetSingleton()->TransitionBack( this );
			}

			TXUIScene::Tick( a_fDeltaTime );
			return;
		}
	}
	else if ( APlayerManager::GetSingleton()->GetBlobData( 0 )->m_iCameraState == 5 )
	{
		// The camera is back: the letterbox replaces the prompt
		AChallenge* pChallenge = pManager->m_pNearbyChallenge;

		if ( m_iChoice == Choice_Start || m_iChoice == Choice_Replace )
			pManager->StartChallenge();
		else if ( m_iChoice == Choice_Complete )
			pManager->m_pNearbyChallenge = TNULL;

		pChallenge->GetRevolutionary()->Refresh( TFALSE );

		TXUI*      pXUI       = TXUI::GetSingleton();
		TXUIScene* pLetterBox = pXUI->CreateScene( pXUI->FindResource( "letterbox.xur" ), 0u );
		pLetterBox->SetPrevScene( m_pPrevScene );
		delete this;
		pXUI->SetFocussedScene( pLetterBox );
		return;
	}

	// The buttons once the text is shown to its end
	if ( !m_pText->IsRevealing() && !m_bTextDone && ( !m_bHasPages || m_iCurrentPage == m_iNumPages - 1 ) )
	{
		m_bTextDone = TTRUE;

		if ( m_bCompleteOnAccept || pManager->m_pNearbyChallenge->GetType() == AChallenge::Type_Null )
			ShowOkButton();
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00643c70
TBOOL ABlobChallengeScene::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_eButton == XUIButton_Accept )
	{
		if ( m_iChoice == Choice_None )
		{
			// The text: shown at once, then the details, or the next page
			if ( m_pText->IsRevealing() )
			{
				m_pText->FinishReveal( TTRUE );
				return TTRUE;
			}

			if ( m_bTextDone && m_pElement11->IsVisible() && AChallengeManager::GetSingleton()->m_pNearbyChallenge->GetType() != AChallenge::Type_Null )
			{
				m_pDetails->SetVisible( TTRUE );
				AChallenge* pChallenge = AChallengeManager::GetSingleton()->m_pNearbyChallenge;
				m_pText->SetOpacity( 0.0f );
				m_pElement11->SetVisible( TFALSE );
				m_pButtons->SetVisible( TTRUE );

				if ( !( pChallenge->GetData()->m_uiFlags & AChallenge::Data::Flags_CanQuit ) )
					ShowOkButton();
				else
					ShowYesNoButtons();

				return TTRUE;
			}

			if ( m_bHasPages && m_iCurrentPage < m_iNumPages - 1 )
			{
				m_iCurrentPage++;
				AChallenge*     pChallenge = AChallengeManager::GetSingleton()->m_pNearbyChallenge;
				ALocaleManager* pLocale    = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );
				TINT            iStringID  = pChallenge->GetData()->m_iStringID;

				m_pText->SetText( m_apPages[ m_iCurrentPage ], -1 );
				ALocaleManager::StringFormat* pFormat = pLocale->GetStringFormat( iStringID );

				// NOTE: the colour indices are a TUINT8 per character
				if ( pFormat != TNULL && pFormat->m_wszFormat != TNULL )
				{
					const TUINT8* pIndices = TREINTERPRETCAST( const TUINT8*, pLocale->GetOverwrittenFormatString( iStringID, pFormat->m_wszFormat ) );
					m_pText->SetText2( TREINTERPRETCAST( const TWCHAR*, pIndices + m_aiPageOffsets[ m_iCurrentPage ] ), pLocale->GetFormatUnk04() );
				}

				if ( !pChallenge->GetRevolutionary()->IsStarting() )
					pChallenge->GetRevolutionary()->OnChallengeStarting();

				AStreamingSoundPlayer::GetSingleton()->PlayVO( iStringID, m_iCurrentPage + 1, pChallenge->GetRevolutionary(), "Talk_Overlay_1" ); // the speech
			}

			return TTRUE;
		}
	}
	else if ( ( a_eButton == XUIButton_Up || a_eButton == XUIButton_Down ) && m_pButtons->IsVisible() && ( m_pOkButton->GetTimelineState() == TimelineState_Focus || m_pOkButton->GetTimelineState() == TimelineState_TransTab ) && !m_pNoButton->IsVisible() )
	{
		// Only the "Ok" button: nothing to navigate to
		return TTRUE;
	}

	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// $deBlob: FUNCTION 00643eb0
void ABlobChallengeScene::OnConfirmationClosed( TBOOL a_bYes )
{
	TASSERT( AGameStateController::GetSingleton()->GetCurrentGameState()->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, AGameStateController::GetSingleton()->GetCurrentGameState() )->SetPaused( TFALSE );

	m_bConfirmed = a_bYes;
}

// $deBlob: FUNCTION 00643f30
void ABlobChallengeScene::EndTransBackTo()
{
	ABlobMenu::EndTransBackTo();

	// Back from the confirmation: its yes quits the current challenge for this one
	if ( m_bConfirmed && m_iChoice == Choice_None )
	{
		m_iChoice = Choice_Replace;
		SetEnabled( TFALSE );
		AChallengeManager::GetSingleton()->m_pNearbyChallenge->GetRevolutionary()->HideStartPrompt();
		PlayAnimation( L"SlideOut", L"EndSlideOut" );
	}
}

// $deBlob: FUNCTION 00644070
void ABlobChallengeScene::MirrorLayout()
{
	SetScale( -GetScaleX(), GetScaleY() );
	m_pElement9->SetScale( -m_pElement9->GetScaleX(), m_pElement9->GetScaleY() );
	m_pButtons->SetScale( -m_pButtons->GetScaleX(), m_pButtons->GetScaleY() );
	m_pDetails->SetPosition( m_pDetails->GetPositionX() + 20.0f, m_pDetails->GetPositionY() );
}

// $deBlob: FUNCTION 006441c0
void ABlobChallengeScene::ResumeGame()
{
	ABlobInputMap::GetSingleton()->DisableFor( 0.5f );
	AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

	if ( !APlayerManager::GetSingleton()->GetBlobDataOf( pBlob )->IsKeyFrameCameraActive() )
		pBlob->SetLevelStartAttached( TFALSE );

	pBlob->m_pSoundComponent->SetSoundsEnabled( TTRUE );
	pBlob->m_pRibbonTrail->m_uiFlags |= ABlobRibbonTrailComponent::FLAGS_VISIBLE;
	pBlob->SetUntargetableTime( TMath::Max( pBlob->GetUntargetableTime(), 1.0f ) );
}

// $deBlob: FUNCTION 00644290
void ABlobChallengeScene::ShowOkButton()
{
	m_pElement11->SetVisible( TFALSE );
	m_pButtons->SetVisible( TTRUE );

	TINT iController = TXUI::GetSingleton()->GetDefaultController();

	if ( GetFocus( iController ) != TNULL )
		GetFocus( iController )->SetFocus( TFALSE, TFALSE, TTRUE );

	SetFocus( m_pOkButton, iController );
	m_pOkButton->SetFocus( TTRUE, TFALSE, TTRUE );
	m_pButtons->PlayAnimation( L"Ok", L"EndOk" );
}

// $deBlob: FUNCTION 00644390
void ABlobChallengeScene::ShowYesNoButtons()
{
	m_pElement11->SetVisible( TFALSE );
	m_pButtons->SetVisible( TTRUE );

	TINT iController = TXUI::GetSingleton()->GetDefaultController();

	if ( GetFocus( iController ) != TNULL )
		GetFocus( iController )->SetFocus( TFALSE, TFALSE, TTRUE );

	SetFocus( m_pYesButton, iController );
	m_pYesButton->SetFocus( TTRUE, TFALSE, TTRUE );
	m_pButtons->PlayAnimation( L"YesNo", L"EndYesNo" );
}
