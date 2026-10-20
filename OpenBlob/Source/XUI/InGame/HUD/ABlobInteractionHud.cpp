#include "pch.h"
#include "ABlobInteractionHud.h"
#include "ABlobHud.h"
#include "Challenges/AChallengeManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobZJumpComponent.h"
#include "Entities/APropDoor.h"
#include "Entities/APropTriggerHudMessage.h"
#include "Entities/APropWindCannon/APropWindCannon.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Locale/ALocaleManager.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"

#include <Input/TInputInterface.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUIControl.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00655910
// $deBlob: FUNCTION 00655920
// $deBlob: FUNCTION 00655990
// $deBlob: FUNCTION 00655a00
// $deBlob: FUNCTION 00655a10
// $deBlob: FUNCTION 0053e2c0
// $deBlob: FUNCTION 00858c20
TDEFINE_CLASS( ABlobInteractionHud );

// $deBlob: FUNCTION 006563c0
ABlobInteractionHud::ABlobInteractionHud()
    : m_iState( STATE_None )
    , m_bUnk134( TFALSE )
    , m_pScaleGroup( TNULL )
    , m_vPosition( 0.0f, 0.0f, 0.0f, 1.0f )
    , m_pSwingScene( TNULL )
{
}

// $deBlob: FUNCTION 00656450
// $deBlob: FUNCTION 00656448
ABlobInteractionHud::~ABlobInteractionHud()
{
}

// $deBlob: FUNCTION 00656340
void ABlobInteractionHud::OnCreated()
{
	if ( TStringManager::String16Compare( GetID(), L"InteractionHud2", -1 ) == 0 )
	{
		m_bUnk134 = TFALSE;
	}
	else
	{
		TASSERT( 0 == TStringManager::String16Compare( GetID(), L"InteractZJumpAttack", -1 ) );
		m_bUnk134 = TTRUE;
	}

	m_pScaleGroup = m_pVisual->FindChildXUIElement( L"ScaleGroup" );
}

// $deBlob: FUNCTION 00655a20
void ABlobInteractionHud::Tick( TFLOAT a_fDeltaTime )
{
	ABlobEntity*         pBlob       = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	ABlobZJumpComponent* pZJump      = pBlob->m_pZJump;
	TINT                 iZJumpState = pZJump->GetState();
	AZJumpData::Point*   pPoint      = pZJump->FindPoint();

	// Follow the point of interaction on screen
	if ( !m_vPosition.IsEqual( TVector4::VEC_ZERO ) )
	{
		AXYZViewportManager* pViewportManager = ARenderer::GetSingleton()->GetViewportManager();
		TViewport*           pViewport        = pViewportManager->GetCameraTViewport( TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) )->GetViewCamera() );

		if ( pViewport != TNULL )
		{
			TVector4 vPosition = m_vPosition;
			ARenderer::GetSingleton()->ProjectToCanvas( vPosition, pViewport );

			TFLOAT fX = vPosition.x - GetWidth() * 0.5f;
			TFLOAT fY = vPosition.y - GetHeight() * 0.5f;
			SetPosition( fX, fY );
		}
	}

	if ( !m_bUnk134 )
	{
		// "InteractionHud2": the A button of a nearby challenge or wind cannon

		if ( AChallengeManager::GetSingleton()->CanStartNearbyChallenge() )
		{
			m_vPosition = AChallengeManager::GetSingleton()->GetNearbyChallenge()->GetData()->m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION );
			m_vPosition.y -= 5.0f;

			if ( m_iState != STATE_Challenge )
			{
				m_iState = STATE_Challenge;
				m_pVisual->PlayAnimation( L"Show", L"EndShow" );
				m_pScaleGroup->PlayAnimation( L"AButton", L"EndAButton" );
			}
		}
		else if ( pBlob->m_pWindCannon != TNULL && pBlob->m_eState != ABlobEntity::STATE_CANNON )
		{
			m_vPosition = pBlob->m_pWindCannon->GetTarget()->GetTranslation();
			m_vPosition.y -= 5.0f;

			if ( m_iState != STATE_WindCannon )
			{
				m_iState = STATE_WindCannon;
				m_pVisual->PlayAnimation( L"Show", L"EndShow" );
				m_pScaleGroup->PlayAnimation( L"AButton", L"EndAButton" );
			}
		}
		else if ( m_iState != STATE_None )
		{
			if ( m_iState == STATE_WindCannon || m_iState == STATE_Challenge )
			{
				m_pVisual->PlayAnimation( L"Hide", L"EndHide" );
				m_pScaleGroup->StopAnimation();
			}

			m_iState = STATE_None;
		}
	}
	else if ( m_bUnk134 == TTRUE )
	{
		// "InteractZJumpAttack": the Z button of a Z-jump target or of the open sprint gate, or the swing message
		ABlobEntity* pPlayerBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
		TFLOAT       fBlobX      = pPlayerBlob->GetTranslation().x;
		TFLOAT       fBlobZ      = pPlayerBlob->GetTranslation().z;
		APropDoor*   pGate       = AGameStatus::GetSingleton()->GetSprintGate();
		TVector4     vGate       = ( pGate != TNULL ) ? pGate->GetTranslation() : TVector4::VEC_ZERO;

		if ( iZJumpState == ABlobZJumpComponent::STATE_LOCK && pZJump->GetCurrentPriorityLock() != TNULL && pZJump->HasValidPriorityLock() )
		{
			if ( m_iState != STATE_SwingLock )
			{
				if ( m_iState == STATE_ZButton || m_iState == STATE_WindCannon || m_iState == STATE_Challenge )
				{
					m_pVisual->PlayAnimation( L"Hide", L"EndHide" );
					m_pScaleGroup->StopAnimation();
				}

				m_iState = STATE_SwingLock;

				static TWCHAR s_wszSwingText[ 0x100 ];
				ShowSwingMessage( 0x1EC, s_wszSwingText );
			}
		}
		else if ( iZJumpState == ABlobZJumpComponent::STATE_POINT || iZJumpState == ABlobZJumpComponent::STATE_AIMNODE || ( iZJumpState == ABlobZJumpComponent::STATE_LOCKLMARK && pZJump->GetCurrentPriorityLock() != TNULL && pZJump->CanJumpToLandmark() ) )
		{
			if ( m_iState != STATE_Swing )
			{
				if ( m_iState == STATE_ZButton )
				{
					m_pVisual->PlayAnimation( L"Hide", L"EndHide" );
					m_pScaleGroup->StopAnimation();
				}

				m_iState = STATE_Swing;

				static TWCHAR s_wszSwingText[ 0x100 ];
				ShowSwingMessage( 0x206, s_wszSwingText );
			}
		}
		else
		{
			TBOOL bZTarget = TFALSE;

			if ( iZJumpState == ABlobZJumpComponent::STATE_AIM )
			{
				m_vPosition = pZJump->m_pTargetNode->GetPosition();
				bZTarget    = TTRUE;
			}
			else if ( ( pPoint != TNULL && iZJumpState != ABlobZJumpComponent::STATE_NODEFLIGHT ) || ( pBlob->m_pHudMessageTrigger != TNULL && pBlob->m_pHudMessageTrigger->GetInteractionType() == 0 ) )
			{
				m_vPosition = ( pPoint != TNULL ) ? pPoint->m_pNode->GetPosition() : pBlob->GetTranslation();
				m_vPosition.y -= 2.0f;
				bZTarget = TTRUE;
			}
			else
			{
				// The sprint gate, while it's open and Blob is close to it
				TBOOL bGate = TFALSE;

				if ( pGate != TNULL && pGate->GetDoorState() == APropDoor::DOORSTATE_Open &&
				     ( iZJumpState == ABlobZJumpComponent::STATE_LOCK ? ( pZJump->GetCurrentPriorityLock() == TNULL ) : ( iZJumpState != ABlobZJumpComponent::STATE_ATTACKFLIGHT && iZJumpState != ABlobZJumpComponent::STATE_ATTACKSLAM ) ) )
				{
					TFLOAT fRadius = ABlobEntity::GetCommonProperties()->m_afUnk38[ 1 ]; // 0x3C

					if ( ( fBlobZ - vGate.z ) * ( fBlobZ - vGate.z ) + ( fBlobX - vGate.x ) * ( fBlobX - vGate.x ) < fRadius * fRadius &&
					     ( ( pPlayerBlob->m_uiFlags >> 30 ) & 1 ) == 0 )
					{
						m_vPosition = vGate;
						m_vPosition.y -= 5.0f;
						bGate = TTRUE;
					}
				}

				if ( bGate )
				{
					if ( m_iState != STATE_ZButton )
						ShowZPrompt();
				}
				else if ( m_iState != STATE_None )
				{
					if ( m_iState == STATE_ZButton )
					{
						m_pVisual->PlayAnimation( L"Hide", L"EndHide" );
						m_pScaleGroup->StopAnimation();
					}

					TXUIScene* pSwingScene = GetSwingScene();

					if ( ( m_iState == STATE_Swing || m_iState == STATE_SwingLock ) && pPlayerBlob->m_eState != ABlobEntity::STATE_LMARK )
						pSwingScene->GetVisual()->PlayAnimation( L"Hide", L"EndHide" );

					m_iState = STATE_None;
				}
			}

			if ( bZTarget && m_iState != STATE_ZButton )
				ShowZPrompt();
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
	TXUIScene::UpdatePlatformElements( m_pScaleGroup, TTRUE );
}

TXUIScene* ABlobInteractionHud::GetSwingScene()
{
	if ( m_pSwingScene == TNULL )
		m_pSwingScene = TSTATICCAST( TXUIScene, ABlobHud::GetSingleton()->FindChildXUIElement( 0xCD ) );

	return m_pSwingScene;
}

void ABlobInteractionHud::ShowZPrompt()
{
	TXUIScene* pSwingScene = GetSwingScene();

	if ( m_iState == STATE_Swing || m_iState == STATE_SwingLock )
		pSwingScene->GetVisual()->PlayAnimation( L"Hide", L"EndHide" );

	m_iState = STATE_ZButton;
	m_pVisual->PlayAnimation( L"Show", L"EndShow" );
	m_pScaleGroup->PlayAnimation( L"ZButton", L"EndZButton" );
}

// The swing message of STATE_SwingLock and STATE_Swing, each with its own static buffer
void ABlobInteractionHud::ShowSwingMessage( TINT a_iNameString, TWCHAR* a_wszBuffer )
{
	TXUIVisual* pVisual = GetSwingScene()->GetVisual();

	pVisual->FindChildXUIElement( L"Drum" )->SetVisible( TFALSE );
	pVisual->FindChildXUIElement( L"raydianswing" )->SetVisible( TInputDevice::GetLastGamepadType() == TInputDevice::Gamepad_Wii );
	pVisual->FindChildXUIElement( L"swing_pc" )->SetVisible( TInputDevice::GetLastGamepadType() != TInputDevice::Gamepad_Wii );

	TXUIControl*    pMessage = TSTATICCAST( TXUIControl, pVisual->FindChildXUIElement( L"Message" ) );
	ALocaleManager* pLocale  = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );

	if ( TInputDevice::GetLastGamepadType() == TInputDevice::Gamepad_Wii )
	{
		pMessage->SetText( pLocale->GetString( 0x32D ), -1 );
	}
	else
	{
		const TWCHAR* wszName = pLocale->GetString( a_iNameString );
		const TWCHAR* wszKey  = pLocale->GetString( 0x36D );
		TStringManager::String16Format( a_wszBuffer, 0x100, L"%ls %ls", wszKey, wszName );
		pMessage->SetText( a_wszBuffer, -1 );
	}

	pVisual->PlayAnimation( L"Display", L"EndDisplay" );
}
