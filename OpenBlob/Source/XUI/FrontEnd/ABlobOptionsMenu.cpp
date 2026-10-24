#include "pch.h"
#include "ABlobOptionsMenu.h"
#include "ABlobFrontEnd.h"
#include "Cameras/ABlobFollowCamera.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "Options/AOptions.h"
#include "MemoryCard/SaveLoad.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundLoadHelper.h"
#include "XUI/ABlobMenu.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIBackButton.h>
#include <XUI/TXUIInput.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Points a_pFrom's navigation a_eNav at a_pTo, with the Id of a_pTo
static void SetNavigation( Toshi::TXUIControl* a_pFrom, Toshi::TXUIControl::Nav a_eNav, Toshi::TXUIControl* a_pTo )
{
	a_pFrom->SetNav( a_eNav, a_pTo );
	a_pFrom->SetNavId( a_eNav, ( a_pTo != TNULL ) ? a_pTo->GetID() : TNULL );
}

// $deBlob: FUNCTION 00639400
// $deBlob: FUNCTION 00639410
// $deBlob: FUNCTION 00639480
// $deBlob: FUNCTION 006394f0
// $deBlob: FUNCTION 00639500
// $deBlob: FUNCTION 0053d750
// $deBlob: FUNCTION 00858950
TDEFINE_CLASS( ABlobOptionsMenu );

// $deBlob: FUNCTION 00639510
ABlobOptionsMenu::ABlobOptionsMenu()
    : ABlobMenu( -1 )
    , m_pMusicSlider( TNULL )
    , m_pSFXSlider( TNULL )
    , m_pCameraSpeed( TNULL )
    , m_pCameraInvert( TNULL )
    , m_pHelp( TNULL )
    , m_uiUnk154( 0 )
    , m_pUID24( TNULL )
    , m_bSaving( TFALSE )
    , m_uiUnk15D( 0 )
    , m_bUnk15E( TFALSE )
    , m_iLastSFX( 0 )
    , m_iLastMusic( 0 )
    , m_iLastCameraSpeed( 0 )
    , m_pTabs( TNULL )
    , m_pTabButton1( TNULL )
    , m_pTabButton2( TNULL )
    , m_pTabButton3( TNULL )
{
}

// $deBlob: FUNCTION 0063a380
// $deBlob: FUNCTION 0063a360
// $deBlob: FUNCTION 0063a372
ABlobOptionsMenu::~ABlobOptionsMenu()
{
}

// $deBlob: FUNCTION 006395a0
void ABlobOptionsMenu::OnCreated()
{
	ABlobMenu::OnCreated();

	m_pMusicSlider  = TSTATICCAST( Toshi::TXUISliderBar, FindChildXUIControl( 7 ) );
	m_pSFXSlider    = TSTATICCAST( Toshi::TXUISliderBar, FindChildXUIControl( 8 ) );
	m_pCameraSpeed  = TSTATICCAST( Toshi::TXUISliderBar, FindChildXUIControl( 12 ) );
	m_pCameraInvert = TSTATICCAST( Toshi::TXUICheckBox, FindChildXUIControl( 15 ) );
	m_pHelp         = TSTATICCAST( Toshi::TXUICheckBox, FindChildXUIControl( 20 ) );
	m_pUID24        = FindChildXUIControl( 24 );
	m_pTabs         = TSTATICCAST( Toshi::TXUITabScene, FindChildXUIControl( 3 ) );
	m_pTabButton1   = FindChildXUIControl( 21 );
	m_pTabButton2   = FindChildXUIControl( 22 );
	m_pTabButton3   = FindChildXUIControl( 23 );

	// Each tab loses its default focus and the focus of controller 0
	for ( TUINT32 i = 0; i < m_pTabs->GetNumTabs(); i++ )
	{
		TXUIScene* pTab = m_pTabs->GetTab( i );
		pTab->SetDefaultFocus( TNULL );

		if ( pTab->GetFocus( 0 ) != TNULL )
		{
			pTab->GetFocus( 0 )->SetFocus( TFALSE, TFALSE, TTRUE );
			pTab->SetFocus( TNULL, 0 );
		}
	}

	SyncFromOptions();

	// Sliders and checkboxes: the tab buttons go down to the controls, the Apply button goes up to the first one
	SetNavigation( m_pMusicSlider, Toshi::TXUIControl::Nav_Up, m_pTabButton1 );
	SetNavigation( m_pSFXSlider, Toshi::TXUIControl::Nav_Down, m_pUID24 );
	SetNavigation( m_pCameraSpeed, Toshi::TXUIControl::Nav_Up, m_pTabButton2 );
	SetNavigation( m_pCameraInvert, Toshi::TXUIControl::Nav_Down, m_pUID24 );
	SetNavigation( m_pHelp, Toshi::TXUIControl::Nav_Up, m_pTabButton3 );
	SetNavigation( m_pHelp, Toshi::TXUIControl::Nav_Down, m_pUID24 );
	SetNavigation( m_pTabButton1, Toshi::TXUIControl::Nav_Down, m_pMusicSlider );
	SetNavigation( m_pTabButton2, Toshi::TXUIControl::Nav_Down, m_pMusicSlider );
	SetNavigation( m_pTabButton3, Toshi::TXUIControl::Nav_Down, m_pMusicSlider );
	SetNavigation( m_pUID24, Toshi::TXUIControl::Nav_Up, m_pSFXSlider );

	// Only in a level (the pause menu): the transitions of the options menu are the in-game ones
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	if ( pState != TNULL && pState->IsA( &TGetClass( AInGameState ) ) )
	{
		SetTransitionVisual( TXUIScene::Transition_To, L"TransTo" );
		SetTransitionVisual( TXUIScene::Transition_BackFrom, L"TransBackFrom" );
	}
}

// $deBlob: FUNCTION 00639870
TBOOL ABlobOptionsMenu::OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController )
{
	ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	TUINT32 uiTab = m_pTabs->GetCurrentTab();

	if ( a_pSource == m_pTabButton1 || a_pSource == m_pTabButton2 || a_pSource == m_pTabButton3 )
	{
		// The focused control of the current tab loses the focus before the tab changes
		TXUIScene* pTab = m_pTabs->GetTab( uiTab );

		if ( pTab->GetFocus( a_iController ) != TNULL )
		{
			pTab->GetFocus( a_iController )->SetFocus( TFALSE, TFALSE, TTRUE );
			pTab->SetFocus( TNULL, a_iController );
		}
	}

	if ( a_pSource == m_pTabButton1 )
	{
		if ( uiTab == 1 || uiTab == 2 )
		{
			SetNavigation( m_pTabButton2, Toshi::TXUIControl::Nav_Down, m_pUID24 );
			SetNavigation( m_pTabButton3, Toshi::TXUIControl::Nav_Down, m_pUID24 );

			m_pTabs->SetForwardButtons( TFALSE );
			TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );
			return m_pTabs->HandleButton( ( uiTab != 1 ) ? XUIButton_Right : XUIButton_Left, a_iController );
		}
	}
	else if ( a_pSource == m_pTabButton2 )
	{
		if ( uiTab == 0 || uiTab == 2 )
		{
			SetNavigation( m_pTabButton1, Toshi::TXUIControl::Nav_Down, m_pUID24 );
			SetNavigation( m_pTabButton3, Toshi::TXUIControl::Nav_Down, m_pUID24 );

			m_pTabs->SetForwardButtons( TFALSE );
			TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );
			return m_pTabs->HandleButton( XUIButton_Right - ( uiTab != 0 ), a_iController );
		}
	}
	else if ( a_pSource == m_pTabButton3 )
	{
		if ( uiTab == 0 || uiTab == 1 )
		{
			SetNavigation( m_pTabButton1, Toshi::TXUIControl::Nav_Down, m_pUID24 );
			SetNavigation( m_pTabButton2, Toshi::TXUIControl::Nav_Down, m_pUID24 );

			m_pTabs->SetForwardButtons( TFALSE );
			TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );
			return m_pTabs->HandleButton( ( uiTab != 0 ) + XUIButton_Left, a_iController );
		}
	}
	else if ( a_pSource == m_pUID24 )
	{
		// Apply: the sliders and checkboxes go to the options, then they are saved
		TINT iMusic = m_pMusicSlider->GetValue();
		TINT iSFX   = m_pSFXSlider->GetValue();

		AOptions* pOptions = AOptions::GetSingleton();
		pOptions->SetMusicVolume( iMusic );
		pOptions->SetSFXVolume( iSFX );

		// Camera speed: the slider's position back to the 0.5..1.0 range of the options
		TFLOAT fCameraMin = ABlobFollowCamera::GetMINCamSpeedOption();
		TFLOAT fCameraMax = ABlobFollowCamera::GetMAXCamSpeedOption();
		TINT   iCameraMin = m_pCameraSpeed->GetRangeMin();
		TINT   iCameraMax = m_pCameraSpeed->GetRangeMax();
		TINT   iCamera    = m_pCameraSpeed->GetValue();
		pOptions->SetCameraSpeed( ( TFLOAT( iCamera - iCameraMin ) / TFLOAT( iCameraMax - iCameraMin ) ) * ( fCameraMax - fCameraMin ) + fCameraMin );

		pOptions->SetCameraInverted( m_pCameraInvert->IsChecked() );
		// The help checkbox is checked when the help is off
		pOptions->SetHelpEnabled( !m_pHelp->IsChecked() );

		pOptions->Apply( TFALSE );

		m_bSaving = TTRUE;

		if ( m_pBackButton != TNULL )
			m_pBackButton->SetEnabled( TFALSE );

		// The save finishes in Tick
		if ( SaveLoad::GetInstance()->IsEnabled() )
		{
			SaveLoad::GetInstance()->SetCaller( this );
			SaveLoad::GetInstance()->SaveProfile( 0, 0 );
			return TFALSE;
		}

		FinishSaving( SaveLoad::ERROR_OK );
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00639f80
TBOOL ABlobOptionsMenu::OnNotifyValueChanged( Toshi::TXUIElement* a_pSource )
{
	if ( a_pSource != m_pTabs )
		return TFALSE;

	m_pTabs->SetForwardButtons( TTRUE );
	TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );

	// The tab buttons go down to the first control of the tab, the Apply button up to the one of the tab's last control
	switch ( m_pTabs->GetCurrentTab() )
	{
		case 0:
			SetNavigation( m_pTabButton1, Toshi::TXUIControl::Nav_Down, m_pMusicSlider );
			SetNavigation( m_pTabButton2, Toshi::TXUIControl::Nav_Down, m_pMusicSlider );
			SetNavigation( m_pTabButton3, Toshi::TXUIControl::Nav_Down, m_pMusicSlider );
			SetNavigation( m_pUID24, Toshi::TXUIControl::Nav_Up, m_pSFXSlider );
			break;

		case 1:
			SetNavigation( m_pTabButton1, Toshi::TXUIControl::Nav_Down, m_pCameraSpeed );
			SetNavigation( m_pTabButton2, Toshi::TXUIControl::Nav_Down, m_pCameraSpeed );
			SetNavigation( m_pTabButton3, Toshi::TXUIControl::Nav_Down, m_pCameraSpeed );
			SetNavigation( m_pUID24, Toshi::TXUIControl::Nav_Up, m_pCameraInvert );
			break;

		case 2:
			SetNavigation( m_pTabButton1, Toshi::TXUIControl::Nav_Down, m_pHelp );
			SetNavigation( m_pTabButton2, Toshi::TXUIControl::Nav_Down, m_pHelp );
			SetNavigation( m_pTabButton3, Toshi::TXUIControl::Nav_Down, m_pHelp );
			SetNavigation( m_pUID24, Toshi::TXUIControl::Nav_Up, m_pHelp );
			break;

		default:
			break;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00639ca0
void ABlobOptionsMenu::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bSaving )
	{
		SaveLoad::GetInstance()->Update( a_fDeltaTime );

		if ( SaveLoad::GetInstance()->GetError() != SaveLoad::ERROR_NONE )
			FinishSaving( SaveLoad::GetInstance()->GetError() );
	}

	// The sliders play a sound when they change and the volumes are applied when one of them moved
	if ( m_pSFXSlider != TNULL && m_pMusicSlider != TNULL )
	{
		TINT iSFX   = m_pSFXSlider->GetValue();
		TINT iMusic = m_pMusicSlider->GetValue();

		TBOOL bSFXSame = iSFX == m_iLastSFX;
		if ( !bSFXSame )
		{
			ASoundInterface::PlayEvent( (ASoundMap::GROUPID)4, (ASoundMap::EVENTID)83, FMOD_EVENT_DEFAULT, TTRUE );
			m_iLastSFX = iSFX;
		}

		TBOOL bUpdate = !bSFXSame;
		if ( iMusic != m_iLastMusic )
		{
			m_iLastMusic = iMusic;
			bUpdate      = TTRUE;
		}

		if ( bUpdate )
			AOptions::GetSingleton()->UpdateVolumes( TTRUE, (TFLOAT)iSFX / 10.0f, (TFLOAT)iMusic / 10.0f );
	}

	if ( m_pCameraSpeed != TNULL )
	{
		TINT iCameraSpeed = m_pCameraSpeed->GetValue();
		if ( iCameraSpeed != m_iLastCameraSpeed )
		{
			ASoundInterface::PlayEvent( (ASoundMap::GROUPID)4, (ASoundMap::EVENTID)83, FMOD_EVENT_DEFAULT, TTRUE );
			m_iLastCameraSpeed = iCameraSpeed;
		}
	}

	Toshi::TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00639ed0
void ABlobOptionsMenu::TransTo()
{
	Toshi::TXUIElement* pParent = FindParentScene();
	if ( pParent->IsA( &TGetClass( ABlobFrontEnd ) ) )
	{
		TSTATICCAST( ABlobFrontEnd, pParent )->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransMainMenuOptions, ABlobFrontEnd::BGMOVIE_LoopOptions, TFALSE, TFALSE, TFALSE );
		TSTATICCAST( ABlobFrontEnd, pParent )->SlideDown();
		AMusicManager::GetSingleton()->PauseMusic( TTRUE, 0.5f );
	}

	Toshi::TXUIScene::TransTo();
}

// $deBlob: FUNCTION 00639e20
void ABlobOptionsMenu::TransBackFrom()
{
	Toshi::TXUIElement* pParent = FindParentScene();
	if ( pParent->IsA( &TGetClass( ABlobFrontEnd ) ) )
	{
		TSTATICCAST( ABlobFrontEnd, pParent )->SlideUp();
		TSTATICCAST( ABlobFrontEnd, pParent )->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_BackOptionsMainMenu, ABlobFrontEnd::BGMOVIE_LoopMainMenu, TFALSE, TFALSE, TFALSE );
		AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.5f );
	}

	TFLOAT fMusic = AOptions::GetSingleton()->GetMusicVolume();
	TFLOAT fSFX   = AOptions::GetSingleton()->GetSFXVolume();
	AOptions::GetSingleton()->UpdateVolumes( TTRUE, fSFX, fMusic );

	ASoundLoadHelper::GetSingleton()->StopLoadingLoop();

	ABlobMenu::TransBackFrom();
}

// $deBlob: FUNCTION 00639f30
void ABlobOptionsMenu::EndTransTo()
{
	if ( ASoundLoadHelper::GetSingleton()->IsLoadingLoopLoaded() )
		ASoundLoadHelper::GetSingleton()->PlayLoadingLoop();

	Toshi::TXUIElement* pParent = FindParentScene();
	if ( pParent->IsA( &TGetClass( ABlobFrontEnd ) ) )
		TSTATICCAST( ABlobFrontEnd, pParent )->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_LoopOptions, ABlobFrontEnd::BGMOVIE_None, TTRUE, TFALSE, TFALSE );

	ABlobMenu::EndTransTo();
}

// $deBlob: FUNCTION 00639df0
void ABlobOptionsMenu::EndTransBackTo()
{
	ABlobMenu::EndTransBackTo();

	if ( m_bSaving )
		FinishSaving( SaveLoad::GetInstance()->GetError() );
}

// $deBlob: FUNCTION 0063a200
void ABlobOptionsMenu::SyncFromOptions()
{

	// The volumes (0..1) go to the slider's range, truncated
	TFLOAT fSFX    = AOptions::GetSingleton()->GetSFXVolume();
	TINT   iMinSFX = m_pSFXSlider->GetRangeMin();
	TINT   iSFX    = TINT( TFLOAT( m_pSFXSlider->GetRangeMax() - iMinSFX ) * fSFX ) + iMinSFX;
	m_pSFXSlider->SetValue( iSFX );
	m_iLastSFX = iSFX;

	TFLOAT fMusic    = AOptions::GetSingleton()->GetMusicVolume();
	TINT   iMinMusic = m_pMusicSlider->GetRangeMin();
	TINT   iMusic    = TINT( TFLOAT( m_pMusicSlider->GetRangeMax() - iMinMusic ) * fMusic ) + iMinMusic;
	m_pMusicSlider->SetValue( iMusic );
	m_iLastMusic = iMusic;

	// The camera speed is clipped to 0.5..1.0
	// NOTE: the camera speed global ABlobFollowCamera::ms_fCamSpeedOption is read directly
	TFLOAT fCameraSpeed = ABlobFollowCamera::ms_fCamSpeedOption;
	TFLOAT fCameraMin   = ABlobFollowCamera::GetMINCamSpeedOption();
	TFLOAT fCameraMax   = ABlobFollowCamera::GetMAXCamSpeedOption();
	Toshi::TMath::Clip( fCameraSpeed, fCameraMin, fCameraMax );

	TINT iMinCamera = m_pCameraSpeed->GetRangeMin();
	TINT iCamera    = TINT( ( ( fCameraSpeed - fCameraMin ) / ( fCameraMax - fCameraMin ) ) * TFLOAT( m_pCameraSpeed->GetRangeMax() - iMinCamera ) ) + iMinCamera;
	m_pCameraSpeed->SetValue( iCamera );
	m_iLastCameraSpeed = iCamera;

	m_pCameraInvert->SetChecked( ABlobFollowCamera::IsInvertCamera() );

	// The help checkbox is checked when the help is off
	m_pHelp->SetChecked( !AOptions::GetSingleton()->IsHelpEnabled() );
}

// $deBlob: FUNCTION 0063a170
void ABlobOptionsMenu::FinishSaving( TINT a_iError )
{
	TASSERT( TTRUE == m_bSaving );
	m_bSaving = TFALSE;

	if ( m_pBackButton != TNULL )
		m_pBackButton->SetEnabled( TTRUE );

	AOptions::GetSingleton()->Apply( TFALSE );

	if ( m_pBackButton != TNULL )
		m_pBackButton->Press();
}
