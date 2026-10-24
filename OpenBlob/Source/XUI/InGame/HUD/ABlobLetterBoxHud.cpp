#include "pch.h"
#include "ABlobLetterBoxHud.h"
#include "ABlobHelpHud.h"
#include "Entities/APropTriggerHelp.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"

#include <XUI/TXUIControl.h>
#include <XUI/TXUIElement.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ABlobLetterBoxHud* ABlobLetterBoxHud::ms_pInstance;

TSTATICASSERT( sizeof( ABlobLetterBoxHud ) == 0x13C );

// AGameStatus+0x3C: set while the letter box is up (0 otherwise); the field is not in AGameStatus yet
static TUINT8& GetLetterBoxFlag( AGameStatus* a_pStatus )
{
	return *( (TUINT8*)a_pStatus + 0x3C );
}

// Gives every player's blob back to the physics: a blob keeps its m_fUntargetableTime when it's above 2.0, otherwise it gets 2.0
static void RestorePlayerBlobs()
{
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	if ( pPlayerManager == TNULL )
		return;

	for ( TINT i = 0; i < pPlayerManager->GetNumPlayers(); i++ )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( TUINT8( i ) ) );
		pBlob->SetLevelStartAttached( TFALSE );

		if ( !( pBlob->GetUntargetableTime() > 2.0f ) )
			pBlob->SetUntargetableTime( 2.0f );
	}
}

// $deBlob: FUNCTION 006568f0
// $deBlob: FUNCTION 00656900
// $deBlob: FUNCTION 006569a0
// $deBlob: FUNCTION 00656a30
// $deBlob: FUNCTION 00656a40
// $deBlob: FUNCTION 0053e340
// $deBlob: FUNCTION 00858c40
TDEFINE_CLASS( ABlobLetterBoxHud );

// $deBlob: FUNCTION 00656d70
ABlobLetterBoxHud::ABlobLetterBoxHud()
    : m_pBars( TNULL )
    , m_pTop( TNULL )
    , m_pBottom( TNULL )
{
}

// $deBlob: FUNCTION 00656a50
// $deBlob: FUNCTION 00656db0
// $deBlob: FUNCTION 00656da7
ABlobLetterBoxHud::~ABlobLetterBoxHud()
{
	APropTriggerHelp::SetLetterBoxHelpHud( TNULL );
	ms_pInstance = TNULL;

	RestorePlayerBlobs();

	AGameStatus* pStatus = AGameStatus::GetSingleton();
	if ( pStatus != TNULL )
		GetLetterBoxFlag( pStatus ) = 0;
}

// $deBlob: FUNCTION 00656b40
void ABlobLetterBoxHud::OnCreated()
{
	FindChildXUIElement( 1 )->SetVisible( TFALSE );

	ABlobHelpHud* pHelpHud = TSTATICCAST( ABlobHelpHud, FindChildXUIControl( 4 ) );

	// NOTE: retail asserts this once
	static TBOOL s_bHelpHudChecked = TFALSE;
	if ( !s_bHelpHudChecked && !pHelpHud->IsA( &TGetClass( ABlobHelpHud ) ) )
	{
		TASSERT( pHelpHud->IsA( &TGetClass( ABlobHelpHud ) ) );
		s_bHelpHudChecked = TTRUE;
	}

	APropTriggerHelp::SetLetterBoxHelpHud( pHelpHud );
	ms_pInstance = this;

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	if ( pPlayerManager != TNULL )
	{
		for ( TINT i = 0; i < pPlayerManager->GetNumPlayers(); i++ )
		{
			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( TUINT8( i ) ) );
			pBlob->SetLevelStartAttached( TTRUE );
		}
	}

	AGameStatus* pStatus = AGameStatus::GetSingleton();
	if ( pStatus != TNULL )
	{
		// NOTE: AGameStatus+0x7F4 (a pointer, its +0x30 is compared with 0) and +0x18 are not named yet
		if ( *(TINT*)( *(TUINT8**)( (TUINT8*)pStatus + 0x7F4 ) + 0x30 ) == 0 )
			GetLetterBoxFlag( pStatus ) = 1;
	}

	m_pBars = FindChildXUIControl( 5 );

	// NOTE: AGameStatus+0x18 is not named yet
	if ( pStatus != TNULL && *(TINT*)( (TUINT8*)pStatus + 0x18 ) == -1 && APlayerManager::GetSingleton()->GetBlobData( 0 )->IsKeyFrameCameraActive() )
		m_pBars->SetVisible( TTRUE );
	else
		m_pBars->SetVisible( TFALSE );

	m_pTop    = FindChildXUIControl( 10 );
	m_pBottom = FindChildXUIElement( 35 );

	m_uiSceneFlags &= ~SceneFlags_AcceptsInput;
}

// $deBlob: FUNCTION 00656c80
void ABlobLetterBoxHud::TransBackFrom()
{
	APropTriggerHelp::ResetHelpState();
	TXUIScene::TransBackFrom();
}

// $deBlob: FUNCTION 00656c90
void ABlobLetterBoxHud::EndTransBackFrom()
{
	RestorePlayerBlobs();

	AGameStatus* pStatus = AGameStatus::GetSingleton();
	if ( pStatus != TNULL )
		GetLetterBoxFlag( pStatus ) = 0;

	TXUIScene::EndTransBackFrom();
}

// $deBlob: FUNCTION 00656c70
void ABlobLetterBoxHud::SetBarsVisible( TBOOL a_bVisible )
{
	m_pBars->SetVisible( a_bVisible );
}

// $deBlob: FUNCTION 00656d30
void ABlobLetterBoxHud::PlayTopDisplay()
{
	if ( m_pTop != TNULL )
		m_pTop->PlayAnimation( L"Display", L"EndDisplay" );
}

// $deBlob: FUNCTION 00656d50
void ABlobLetterBoxHud::PlayBottomDisplay()
{
	if ( m_pBottom != TNULL )
		m_pBottom->PlayAnimation( L"Display", L"EndDisplay" );
}
