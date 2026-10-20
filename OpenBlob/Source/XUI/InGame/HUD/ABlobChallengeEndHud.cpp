#include "pch.h"
#include "ABlobChallengeEndHud.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ABlobFollowCamera.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "Entities/Enemies/AEnemyGrappler.h"
#include "Locale/ALocaleManager.h"
#include "Sound/AMusicManager.h"
#include "Entities/APropTriggerHelp.h"
#include "XUI/InGame/HUD/ABlobHelpHud.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "XUI/InGame/HUD/ABlobGoalHud.h"
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUIText.h>
#include "Challenges/AChallenge.h"
#include "Challenges/AChallengeManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/APlayerManager.h"
#include "GUI/AGUIManager.h"
#include "Input/ABlobInputMap.h"
#include "Terrain2/ATerrain2Interface.h"
#include "XUI/ABlobMenu.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0064e130
// $deBlob: FUNCTION 0064e140
// $deBlob: FUNCTION 0064e1b0
// $deBlob: FUNCTION 0064e220
// $deBlob: FUNCTION 0064e230
// $deBlob: FUNCTION 0053e050
// $deBlob: FUNCTION 00858b90
TDEFINE_CLASS( ABlobChallengeEndHud );

// $deBlob: FUNCTION 0064ec70
ABlobChallengeEndHud::ABlobChallengeEndHud()
    : m_bShown( TFALSE )
    , m_fTimer( -1.0f )
    , m_pResult( TNULL )
    , m_pGate( TNULL )
    , m_pTitle( TNULL )
    , m_bFailed( TFALSE )
    , m_bUnk149( TFALSE )
    , m_pChallenge( TNULL )
    , m_iCount( -1 )
    , m_bWaitingForBlob( TFALSE )
    , m_pRetry( TNULL )
    , m_pBack( TNULL )
    , m_bRetrying( TFALSE )
    , m_bRespawnPending( TFALSE )
    , m_pUnk264( TNULL )
    , m_pUnk268( TNULL )
    , m_pHint( TNULL )
{
}

// $deBlob: FUNCTION 0064e240
// $deBlob: FUNCTION 0064ed20
// $deBlob: FUNCTION 0064ed17
ABlobChallengeEndHud::~ABlobChallengeEndHud()
{
	AGUIManager::GetSingleton()->SetCursorEnabled( TFALSE );
	AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TFALSE );
}

// $deBlob: FUNCTION 0064e2c0
void ABlobChallengeEndHud::OnCreated()
{
	AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TTRUE;

	ABlobInputMap* pInputMap = ABlobInputMap::GetSingleton();
	pInputMap->DisableFor( -1.0f );

	m_pResult = FindChildXUIControl( 3 );
	m_pGate   = FindChildXUIElement( 77 );
	m_pGate->SetVisible( TFALSE );
	m_pResult->SetVisible( TFALSE );

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	TPhysicsManager::StopBody( pBlob->m_pBody );
	pBlob->SetLevelStartAttached( TTRUE );
	pBlob->ResetMotion();

	m_oTimelineControl.PlayAnimation( L"Normal", L"EndNormal" );

	TXUIElement* pElement = FindChildXUIElement( 72 );
	TASSERT( pElement->IsA( &TGetClass( TXUIText ) ) );
	m_pTitle = TSTATICCAST( TXUIText, pElement );

	pElement = FindChildXUIElement( 109 );
	TASSERT( pElement->IsA( &TGetClass( TXUIText ) ) );
	m_pCount = TSTATICCAST( TXUIText, pElement );

	m_pRetry  = FindChildXUIControl( 74 );
	m_pBack   = FindChildXUIControl( 75 );
	m_pHint   = TSTATICCAST( TXUIText, FindChildXUIElement( 71 ) );
	m_pUnk264 = FindChildXUIElement( 5 );
	m_pUnk268 = FindChildXUIElement( 35 );
}

// $deBlob: FUNCTION 0064e500
void ABlobChallengeEndHud::EndTransBackFrom()
{
	Finish();
	AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;

	// A gate: the goal counter decays
	if ( m_pChallenge == TNULL && ABlobHud::GetSingleton() != TNULL )
		TSTATICCAST( ABlobGoalHud, ABlobHud::GetSingleton()->GetGoalHud() )->StartDecay();

	TXUIScene::EndTransBackFrom();
}

// $deBlob: FUNCTION 0064e540
void ABlobChallengeEndHud::Tick( TFLOAT a_fDeltaTime )
{
	ACamera*     pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
	ABlobEntity* pBlob   = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

	TXUI::GetSingleton()->TickInput( a_fDeltaTime );

	if ( TXUI::GetSingleton()->GetTransToScene() == TNULL && TXUI::GetSingleton()->GetTransFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() )
	{
		if ( !m_bRetrying )
		{
			if ( !m_bWaitingForBlob )
			{
				if ( m_bShown == TTRUE )
				{
					m_fTimer -= a_fDeltaTime;

					if ( m_fTimer < 0.0f )
					{
						if ( m_pChallenge != TNULL )
						{
							if ( m_bFailed )
							{
								AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );
								TXUIScene::Tick( a_fDeltaTime );
								return;
							}

							// The camera of the challenge's end (a sub challenge with one): the HUD waits for Blob
							AChallenge::Data* pData = m_pChallenge->GetData();

							if ( ( pData->m_uiFlags & AChallenge::Data::Flags_CompletedCameraOnRequest ) && pData->m_iCompletedSSCamera != -1 )
							{
								APlayerManager::GetSingleton()->PlaySSCamera( 0, pData->m_iCompletedSSCamera );

								if ( APlayerManager::GetSingleton()->GetBlobData( 0 )->IsKeyFrameCameraActive() )
								{
									m_bWaitingForBlob                                     = TTRUE;
									AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;
									TXUIScene::Tick( a_fDeltaTime );
									return;
								}
							}
						}

						TXUI::GetSingleton()->TransitionBack( this );
					}
				}
				else if ( pCamera != TNULL && pCamera->GetCameraType() == ACamera::CT_BLOBFOLLOW && ( TSTATICCAST( ABlobFollowCamera, pCamera )->IsAtIdealPos() || TSTATICCAST( ABlobFollowCamera, pCamera )->IsOnFlaggedNedFixedCam() ) )
				{
					// The camera is back on Blob: the result
					m_bShown                = TTRUE;
					ALocaleManager* pLocale = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );

					if ( m_pChallenge == TNULL )
					{
						// A gate opened (APropSwitchPressure)
						m_fTimer = 3.0f;
						m_pResult->SetVisible( TFALSE );
						m_pGate->SetVisible( TTRUE );
						AEnemyGrappler::ReleaseAllGrapples( pBlob );
						pBlob->GetAnimationComponent()->SetState( ABlobAnimationComponent::STATE_CHALLENGE_VICTORY );
						m_pGate->PlayAnimation( L"GateOpened", L"EndGateOpened" );

						if ( m_iCount == -1 )
						{
							m_pCount->SetText( L"" );
						}
						else
						{
							TStringManager::String16Format( m_wszCount, 0x80, L"%d", m_iCount );
							m_pCount->SetText( m_wszCount );
						}
					}
					else
					{
						m_fTimer = 2.0f;
						m_pResult->SetVisible( TTRUE );
						m_pGate->SetVisible( TFALSE );
						TINT iStringID;

						if ( m_bFailed == TTRUE )
						{
							AMusicManager::GetSingleton()->PlayMusicSFX( "challenge_fail", 1, &TVector4::VEC_ZERO, TNULL );
							AEnemyGrappler::ReleaseAllGrapples( pBlob );
							pBlob->GetAnimationComponent()->SetState( ABlobAnimationComponent::STATE_CHALLENGE_DEFEAT );
							m_pResult->PlayAnimation( L"ChallengeFailed", L"EndChallengeFailed" );
							iStringID = 0xC9;
						}
						else
						{
							if ( m_pChallenge->GetType() != AChallenge::Type_Null )
								AMusicManager::GetSingleton()->PlayMusicSFX( "challenge_complete", 0x2041, &TVector4::VEC_ZERO, TNULL );

							AEnemyGrappler::ReleaseAllGrapples( pBlob );
							pBlob->GetAnimationComponent()->SetState( ABlobAnimationComponent::STATE_CHALLENGE_VICTORY );
							m_pResult->PlayAnimation( L"ChallengeComplete", L"EndChallengeComplete" );
							iStringID = 200;
						}

						m_pTitle->SetText( pLocale->GetString( iStringID ) );

						if ( !( m_pChallenge->GetFlags() & AChallenge::Flags_Restarted ) )
						{
							m_pUnk264->SetVisible( TFALSE );
							m_pUnk268->SetVisible( TTRUE );
							m_pHint->SetVisible( TTRUE );
							m_pHint->SetText( pLocale->GetString( 0xCE ) );
						}
						else
						{
							m_pUnk264->SetVisible( TTRUE );
							m_pUnk268->SetVisible( TFALSE );
							m_pHint->SetVisible( TFALSE );
						}
					}
				}
			}
			else
			{
				// The camera is back: the letterbox replaces the scene, with the help about the next challenges
				if ( APlayerManager::GetSingleton()->GetBlobData( 0 )->m_iCameraState == 5 )
				{
					Finish();

					TXUIScene* pLetterBox = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "letterbox.xur" ), 0u );
					pLetterBox->SetPrevScene( m_pPrevScene );

					TINT   iStringID   = -1;
					TUINT8 uiNumLinked = m_pChallenge->GetData()->m_uiNumLinkedChallenges;

					if ( uiNumLinked != 0 )
						iStringID = ( uiNumLinked != 1 ) + 0xC5;

					if ( APropTriggerHelp::IsHelpShown() && APropTriggerHelp::IsHelpHudShown() )
					{
						APropTriggerHelp::GetHelpHud()->Hide();
						APropTriggerHelp::ResetHelpState();
					}

					APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_LetterBox, iStringID, -1, TTRUE, 5.0f, 0, TNULL, TFALSE );
					delete this;
					TXUI::GetSingleton()->SetFocussedScene( pLetterBox );
					return;
				}
			}
		}
		else if ( !m_bRespawnPending )
		{

			if ( !AGameStateController::GetSingleton()->IsFading( 0 ) && AGameStateController::GetSingleton()->GetFade( 0 ) == 1.0f )
			{
				AChallengeManager::RestartInfo& rRestart = AChallengeManager::GetSingleton()->GetRestartInfo();
				pBlob->PrepareRespawn( &rRestart.m_vBlobPosition, &rRestart.m_iBlobPoints, (const TINT*)&rRestart.m_uiBlobColour );
				m_bRespawnPending = TTRUE;
			}
		}
		else if ( ATerrain2Interface::GetInstance()->IsCurrentCellReady() && !ATerrain2Interface::GetInstance()->IsStreaming() && TXUI::GetSingleton()->GetTransToScene() == TNULL && TXUI::GetSingleton()->GetTransFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() )
		{
			m_bRespawnPending = TFALSE;
			pBlob->Respawn( TTRUE, TTRUE, &AChallengeManager::GetSingleton()->GetRestartInfo().m_vBlobPosition );
			AGameStateController::GetSingleton()->FadeFromBlack( 0.5f, 0 );
			TXUI::GetSingleton()->TransitionBack( this );
			AChallengeManager::GetSingleton()->RestartChallenge();
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0064eaa0
void ABlobChallengeEndHud::SetChallenge( AChallenge* a_pChallenge, TBOOL a_bFailed )
{
	m_pChallenge = a_pChallenge;
	m_bFailed    = a_bFailed;
}

// $deBlob: FUNCTION 0064eac0
void ABlobChallengeEndHud::Finish()
{
	ABlobInputMap::GetSingleton()->Enable();
	ABlobInputMap::GetSingleton()->ClearPlayers();

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	pBlob->SetLevelStartAttached( TFALSE );

	if ( m_pChallenge != TNULL )
	{
		AChallengeManager::GetSingleton()->EndChallenge( m_pChallenge, m_bFailed, m_bRetrying );
		return;
	}

	pBlob->m_uiFlags &= ~0x10000000;
}

// $deBlob: FUNCTION 0064eb40
TBOOL ABlobChallengeEndHud::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	ABlobMenu::PlayPressSound( a_pSource );

	if ( a_pSource == m_pRetry )
	{
		m_bRetrying = TTRUE;
		AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TTRUE );
		m_pResult->PlayAnimation( L"Retry", L"EndRetry" );
		AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
		TSTATICCAST( TXUIControl, a_pSource )->SetEnabled( TFALSE );
		return TTRUE;
	}
	else if ( a_pSource == m_pBack )
	{
		TXUI::GetSingleton()->TransitionBack( this );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0064ebf0
TBOOL ABlobChallengeEndHud::OnNotifySetFocus( TXUIElement* a_pSource )
{
	ABlobMenu::PlayFocusSound( a_pSource );
	return TFALSE;
}
