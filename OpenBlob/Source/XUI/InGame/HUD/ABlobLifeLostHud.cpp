#include "pch.h"
#include "ABlobLifeLostHud.h"
#include "ABlobHud.h"
#include "ABlobTimerHud.h"
#include "Cameras/ACamera.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobSoundComponent.h"
#include "GameController/AGameStatus.h"
#include "Challenges/AChallengeManager.h"
#include "GameInterface/APlayerManager.h"
#include "Locale/ALocaleManager.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include "Terrain2/ATerrain2Interface.h"

#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIControl.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobLifeLostHud ) == 0x164 );

// The text: the life lost, or the time ran out
static TCONSTEXPR_VAR TINT s_iLifeLostString = 0x199;
static TCONSTEXPR_VAR TINT s_iTimeOutString  = 0x171;

// $deBlob: FUNCTION 006571d0
// $deBlob: FUNCTION 006571e0
// $deBlob: FUNCTION 00657290
// $deBlob: FUNCTION 00657330
// $deBlob: FUNCTION 00657340
// $deBlob: FUNCTION 0053e3c0
// $deBlob: FUNCTION 00858c60
TDEFINE_CLASS( ABlobLifeLostHud );

// $deBlob: FUNCTION 006578b0
ABlobLifeLostHud::ABlobLifeLostHud()
    : m_bRespawnReady( TFALSE )
    , m_pBlob( TNULL )
    , m_pWipe( TNULL )
    , m_pLives( TNULL )
    , m_pText( TNULL )
{
}

// $deBlob: FUNCTION 00657900
// $deBlob: FUNCTION 00657350
// $deBlob: FUNCTION 006578f8
ABlobLifeLostHud::~ABlobLifeLostHud()
{
}

// Projects a_pBlob's position onto the canvas, relative to the centre of the wipe; returns TFALSE when it's off the
// scene (or a_pBlob's camera has no viewport)
static TBOOL GetWipeCentre( ABlobLifeLostHud* a_pScene, TXUIControl* a_pWipe, ABlobEntity* a_pBlob, TFLOAT& a_rfOutX, TFLOAT& a_rfOutY, TBOOL a_bCheckLevelStart )
{
	TVector4   vPosition = a_pBlob->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
	TViewport* pViewport = ARenderer::GetSingleton()->GetViewportManager()->GetCameraTViewport( a_pBlob->GetViewCamera() );

	if ( pViewport == TNULL || ( a_bCheckLevelStart && AGameStatus::GetSingleton()->IsLevelStart() ) )
		return TFALSE;

	ARenderer::GetSingleton()->ProjectToCanvas( vPosition, pViewport );

	a_rfOutX = vPosition.x - a_pWipe->GetWidth() * 0.5f;
	a_rfOutY = vPosition.y - a_pWipe->GetHeight() * 0.5f;

	return TMath::Abs( a_rfOutX ) < a_pScene->GetWidth() * 0.5f && TMath::Abs( a_rfOutY ) < a_pScene->GetHeight() * 0.5f;
}

// $deBlob: FUNCTION 00657420
void ABlobLifeLostHud::Tick( TFLOAT a_fDeltaTime )
{

	if ( m_bRespawnReady == TTRUE && ATerrain2Interface::GetInstance()->IsCurrentCellReady() && !ATerrain2Interface::GetInstance()->IsStreaming() &&
	     TXUI::GetSingleton()->GetTransToScene() == TNULL && TXUI::GetSingleton()->GetTransFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() )
	{
		m_pBlob->Respawn( TFALSE, TTRUE, TNULL );

		TFLOAT fX, fY;

		if ( GetWipeCentre( this, m_pWipe, m_pBlob, fX, fY, TTRUE ) )
			m_pWipe->SetPosition( fX, fY );
		else
			m_pWipe->SetPosition( 0.0f, 0.0f );

		m_pWipe->PlayAnimation( L"Show", L"EndShow" );


		if ( TINT( APlayerManager::GetSingleton()->GetBlobData( 0 )->m_uiLives ) > 0 && AGameStatus::GetSingleton()->GetTimeLeft() < 300.0f )
		{
			AGameStatus::GetSingleton()->SetTimeLeft( 300.0f );
			ABlobHud::GetSingleton()->GetLevelTimer()->ResetWarning();
		}

		TXUI::GetSingleton()->TransitionBack( this );
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00657640
void ABlobLifeLostHud::SetBlob( ABlobEntity* a_pBlob )
{
	m_pBlob = a_pBlob;

	TFLOAT fX, fY;

	// NOTE: retail writes the fixed point position and the dirty flag directly, which is SetPosition
	if ( GetWipeCentre( this, m_pWipe, a_pBlob, fX, fY, TFALSE ) )
		m_pWipe->SetPosition( fX, fY );
	else
		m_pWipe->SetPosition( 0.0f, 0.0f );

	TStringManager::String16Format( m_wszLives, 16, L"x %d", APlayerManager::GetSingleton()->GetBlobDataOf( m_pBlob )->m_uiLives );
	m_pLives->SetText( m_wszLives, -1 );
	m_pWipe->PlayAnimation( L"Hide", L"EndHide" );

	ALocaleManager* pLocale = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );
	m_pText->SetText( pLocale->GetString( ( AGameStatus::GetSingleton()->GetTimeLeft() > 0.0f ) ? s_iLifeLostString : s_iTimeOutString ), -1 );

	FindChildXUIControl( 7 )->SetOpacity( ( a_pBlob->GetUnkB68() != 0 ) ? 1.0f : 0.0f );
}

// $deBlob: FUNCTION 00657850
TBOOL ABlobLifeLostHud::SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam )
{
	if ( a_eMessage == XUIMessage_NamedFrame && TStringManager::String16Compare( a_wszParam, L"RadialWipeComplete", -1 ) == 0 && m_pBlob != TNULL && m_pBlob->GetUnkB68() != 0 )
	{
		m_pBlob->SetUnkB68( 0 );
		AChallengeManager::GetSingleton()->FailCurrentChallenge();
		return TTRUE;
	}

	return TXUIControl::SendMessage( a_eMessage, a_wszParam );
}

// $deBlob: FUNCTION 00657370
void ABlobLifeLostHud::OnCreated()
{
	m_pWipe  = FindChildXUIControl( 1 );
	m_pLives = FindChildXUIControl( 5 );
	m_pText  = FindChildXUIControl( 4 );
}

// $deBlob: FUNCTION 006573a0
void ABlobLifeLostHud::EndTransTo()
{
	if ( TINT( APlayerManager::GetSingleton()->GetBlobData( 0 )->m_uiLives ) > 0 )
	{
		m_pBlob->PrepareRespawn( TNULL, TNULL, TNULL );
		m_bRespawnReady = TTRUE;
		TXUIScene::EndTransTo();
		return;
	}

	// The last life: game over
	if ( m_pBlob != TNULL )
		m_pBlob->GetSoundComponent()->OnRespawn();

	AGameStatus::GetSingleton()->OnGameOver();
	TXUIScene::EndTransTo();
}

// $deBlob: FUNCTION 00657400
void ABlobLifeLostHud::EndTransBackFrom()
{
	m_pBlob->SetLifeLostShown( TFALSE );
	TXUIScene::EndTransBackFrom();
}
