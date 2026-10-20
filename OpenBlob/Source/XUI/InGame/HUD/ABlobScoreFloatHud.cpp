#include "pch.h"
#include "ABlobScoreFloatHud.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"

#include <Toshi/TStringManager.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The share of the display time over which the popup fades out
static TCONSTEXPR_VAR TFLOAT s_fFadeOutShare = 0.2f;

// $deBlob: FUNCTION 00658470
// $deBlob: FUNCTION 00658480
// $deBlob: FUNCTION 00658520
// $deBlob: FUNCTION 006585c0
// $deBlob: FUNCTION 006585d0
// $deBlob: FUNCTION 0053e4c0
// $deBlob: FUNCTION 00858ca0
TDEFINE_CLASS( ABlobScoreFloatHud );

// $deBlob: FUNCTION 006585e0
ABlobScoreFloatHud::ABlobScoreFloatHud()
    : m_fTimeLeft( 0.0f )
    , m_iPlayer( 0 )
    , m_vPosition( TVector4::VEC_ZERO )
{
}

// $deBlob: FUNCTION 00658a50
// $deBlob: FUNCTION 00658620
// $deBlob: FUNCTION 00658a3d
ABlobScoreFloatHud::~ABlobScoreFloatHud()
{
}

// $deBlob: FUNCTION 006588a0
void ABlobScoreFloatHud::Tick( TFLOAT a_fDeltaTime )
{
	// The position on the screen of the player's camera (split screen: the player's quarter of 1920x1080)
	ACamera*   pCamera   = APlayerManager::GetSingleton()->GetPlayerCamera( m_iPlayer );
	TViewport* pViewport = ARenderer::GetSingleton()->GetViewportManager()->GetCameraTViewport( pCamera );

	TVector4 vScreen = m_vPosition;
	ARenderer::GetSingleton()->ProjectToCanvas( vScreen, pViewport );

	TFLOAT fOffsetX = 0.0f;
	TFLOAT fOffsetY = 0.0f;

	if ( m_iPlayer == 1 )
	{
		fOffsetX = 960.0f;
	}
	else if ( m_iPlayer == 2 )
	{
		fOffsetY = 540.0f;
	}
	else if ( m_iPlayer == 3 )
	{
		fOffsetY = 540.0f;
		fOffsetX = 960.0f;
	}

	TFLOAT fX = ( vScreen.x + fOffsetX ) - GetWidth() * 0.5f;
	TFLOAT fY = ( vScreen.y + fOffsetY ) - GetHeight() * 0.5f;
	SetPosition( fX, fY );

	if ( m_fTimeLeft > 0.0f )
	{
		m_fTimeLeft -= a_fDeltaTime;

		if ( m_fTimeLeft < 0.0f )
			m_fTimeLeft = 0.0f;

		TFLOAT fShare = m_fTimeLeft / *TREINTERPRETCAST( TFLOAT*, AGameStatus::GetBlobScoringProperties() );

		if ( fShare < s_fFadeOutShare )
		{
			TFLOAT fOpacity = fShare / s_fFadeOutShare;
			TMath::Clip( fOpacity, 0.0f, 1.0f );
			SetOpacity( fOpacity );
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00658640
void ABlobScoreFloatHud::ShowScore( TINT a_iPoints, TINT a_iMultiplier, const TWCHAR* a_wszName, const TVector4& a_rPosition, const TWCHAR* a_wszBonusName, TINT a_iBonus )
{
	if ( AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel != 0 )
		return;

	TASSERT( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint );

	m_vPosition = a_rPosition;

	const TWCHAR* wszSign = ( a_iPoints > 0 ) ? L"+" : L"";

	if ( a_iMultiplier <= 1 )
	{
		if ( a_iBonus != 0 )
			TStringManager::String16Format( m_wszText, 0x100, L"%s%d\n%d %ls", wszSign, a_iPoints, a_iBonus, a_wszBonusName );
		else
			TStringManager::String16Format( m_wszText, 0x100, L"%s%d\n%ls", wszSign, a_iPoints, a_wszBonusName );
	}
	else if ( a_iBonus != 0 )
	{
		TStringManager::String16Format( m_wszText, 0x100, L"%s%d x%d\n%ls\n%d %ls", wszSign, a_iPoints, a_iMultiplier, a_wszName, a_iBonus, a_wszBonusName );
	}
	else
	{
		TStringManager::String16Format( m_wszText, 0x100, L"%s%d x%d\n%ls", wszSign, a_iPoints, a_iMultiplier, a_wszName );
	}

	SetPresenterText( m_wszText, 0, -1 );
	StopAnimation();

	if ( a_iPoints <= 0 )
		m_pVisual->PlayAnimation( L"DisplayBad", L"EndDisplayBad" );
	else
		m_pVisual->PlayAnimation( L"Display", L"EndDisplay" );

	m_fTimeLeft = *TREINTERPRETCAST( TFLOAT*, AGameStatus::GetBlobScoringProperties() );
	SetOpacity( 1.0f );
}

// $deBlob: FUNCTION 006587c0
void ABlobScoreFloatHud::ShowText( const TWCHAR* a_wszText, const TVector4& a_rPosition, TBOOL a_bTimePickup )
{
	if ( AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel != 0 )
		return;

	TASSERT( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint );

	m_vPosition = a_rPosition;
	TStringManager::String16Copy( m_wszText, a_wszText );
	SetPresenterText( m_wszText, 0, -1 );
	StopAnimation();
	SetOpacity( 1.0f );

	if ( a_bTimePickup )
		m_pVisual->PlayAnimation( L"TimePickup", L"EndTimePickup" );
	else
		m_pVisual->PlayAnimation( L"Display", L"EndDisplay" );

	m_fTimeLeft = -1.0f;
}
