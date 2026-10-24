#include "pch.h"
#include "ABlobMenu.h"
#include "FrontEnd/ABlobFrontEnd.h"
#include "FrontEnd/ABlobChooseProfileMenu.h"
#include "FrontEnd/ABlobDeleteProfileMenu.h"
#include "ABlobConfirmationMenu.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/APlayerProfileManager.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundLoadHelper.h"
#include "Sound/AMusicManager.h"
#include "Sound/AGUIMusicController.h"
#include "Movie/AMoviePlayer.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIInput.h>
#include <XUI/TXUIButton.h>
#include <XUI/TXUIBackButton.h>
#include <XUI/TXUIListItem.h>
#include <XUI/TXUIRadioButton.h>
#include <XUI/TXUIText.h>
#include <XUI/TXUICanvas.h>
#include <XUI/TXUIVisual.h>
#include <Input/TInputInterface.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/T2String16.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TFLOAT ABlobMenu::sm_fAttractModeTime     = 60.0f;
const TFLOAT ABlobMenu::sm_fAttractModeFadeTime = 0.5f;

// $deBlob: FUNCTION 00629f60
// $deBlob: FUNCTION 00629f70
// $deBlob: FUNCTION 0062a020
// $deBlob: FUNCTION 0062a0c0
// $deBlob: FUNCTION 0062a0d0
// $deBlob: FUNCTION 0053d0d0
// $deBlob: FUNCTION 008587b0
TDEFINE_CLASS( ABlobMenu );

ABlobMenu::ABlobMenu()
{
	m_fIdleTime        = 0.0f;
	m_fAttractFadeTime = 0.0f;
	m_bAttractMovie    = TFALSE;
	m_bAttractFading   = TFALSE;
	m_iBackgroundMovie = -1;
	m_pFrontEnd        = TNULL;
}

// $deBlob: FUNCTION 0062a0e0
ABlobMenu::ABlobMenu( TINT8 a_iBackgroundMovie )
{
	m_iBackgroundMovie = a_iBackgroundMovie;
	m_fIdleTime        = 0.0f;
	m_fAttractFadeTime = 0.0f;
	m_bAttractMovie    = TFALSE;
	m_bAttractFading   = TFALSE;
	m_pFrontEnd        = TNULL;
}

// $deBlob: FUNCTION 00625da0
// $deBlob: FUNCTION 00625d40
// $deBlob: FUNCTION 00625d5a
ABlobMenu::~ABlobMenu()
{
}

// $deBlob: FUNCTION 0062a130
void ABlobMenu::OnCreated()
{
	TXUIScene::OnCreated();
	SetBackButtonText( this );
}

// $deBlob: FUNCTION 0062a3f0
TBOOL ABlobMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( m_bAttractMovie != TTRUE )
		PlayPressSound( a_pSource );

	return TFALSE;
}

// $deBlob: FUNCTION 0062a410
TBOOL ABlobMenu::OnNotifySetFocus( TXUIElement* a_pSource )
{
	if ( m_bAttractMovie != TTRUE )
	{
		PlayFocusSound( a_pSource );
		m_fIdleTime = 0.0f;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0062a440
TBOOL ABlobMenu::OnNotifyKillFocus( TXUIElement* a_pSource )
{
	// NOTE: retail plays the press sound here too
	if ( m_bAttractMovie != TTRUE )
		PlayPressSound( a_pSource );

	return TFALSE;
}

// $deBlob: FUNCTION 0062a2d0
void ABlobMenu::TransFrom()
{
	if ( m_pFrontEnd != TNULL && m_pFrontEnd->IsA( &TGetClass( ABlobFrontEnd ) ) )
	{
		TASSERT( AMusicManager::GetSingleton()->GetGUIMusicController()->GetBeatScene() == this );
		AMusicManager::GetSingleton()->GetGUIMusicController()->SetBeatScene( TNULL );
	}

	TXUIScene::TransFrom();
}

// $deBlob: FUNCTION 0062a360
void ABlobMenu::TransBackFrom()
{
	if ( m_pFrontEnd != TNULL && m_pFrontEnd->IsA( &TGetClass( ABlobFrontEnd ) ) )
	{
		TASSERT( AMusicManager::GetSingleton()->GetGUIMusicController()->GetBeatScene() == this );
		AMusicManager::GetSingleton()->GetGUIMusicController()->SetBeatScene( TNULL );
	}

	TXUIScene::TransBackFrom();
}

// $deBlob: FUNCTION 0062a140
void ABlobMenu::EndTransTo()
{
	if ( m_pFrontEnd == TNULL )
		m_pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );

	if ( m_pFrontEnd != TNULL && m_pFrontEnd->IsA( &TGetClass( ABlobFrontEnd ) ) )
	{
		AMusicManager::GetSingleton()->GetGUIMusicController()->SetBeatScene( this );

		ABlobFrontEnd* pFrontEnd = m_pFrontEnd;

		// The popups keep the cursor trails
		TBOOL bIsPopupMenu = IsA( &TGetClass( ABlobConfirmationMenu ) ) || IsA( &TGetClass( ABlobDeleteProfileMenu ) );

		if ( !bIsPopupMenu )
			pFrontEnd->ResetCursorTrails();

		if ( m_iBackgroundMovie != -1 )
			pFrontEnd->PlayBackgroundMovie( m_iBackgroundMovie, ABlobFrontEnd::BGMOVIE_None, TTRUE, 0, TFALSE );
	}

	TXUIScene::EndTransTo();
}

// $deBlob: FUNCTION 0062a200
void ABlobMenu::EndTransBackTo()
{
	if ( m_pFrontEnd == TNULL )
		m_pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );

	if ( m_pFrontEnd != TNULL && m_pFrontEnd->IsA( &TGetClass( ABlobFrontEnd ) ) )
	{
		TBOOL bKeepCursorTrails = IsA( &TGetClass( ABlobDeleteProfileMenu ) ) || ( IsA( &TGetClass( ABlobChooseProfileMenu ) ) && m_iTransitionIndex == 2 );

		if ( !bKeepCursorTrails )
			m_pFrontEnd->ResetCursorTrails();

		AMusicManager::GetSingleton()->GetGUIMusicController()->SetBeatScene( this );

		if ( m_iBackgroundMovie != -1 )
		{
			ABlobFrontEnd* pParentFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
			pParentFrontEnd->PlayBackgroundMovie( m_iBackgroundMovie, ABlobFrontEnd::BGMOVIE_None, TTRUE, 0, TFALSE );
		}
	}

	TXUIScene::EndTransBackTo();
}

// $deBlob: FUNCTION 0062a9c0
void ABlobMenu::StartAttractMode()
{
	TXUI::GetSingleton()->GetCanvas()->SetVisible( TFALSE );

	AMusicManager::GetSingleton()->PauseMusic( TTRUE, 0.0f );

	m_bAttractMovie    = TTRUE;
	m_bAttractFading   = TFALSE;
	m_fAttractFadeTime = -1.0f;
	AGameStateController::GetSingleton()->SetFade( 0.0f, 0 );
	SetVisible( TFALSE );

	TXUIScene* pParent = FindParentScene();

	if ( pParent != TNULL && pParent->IsA( &TGetClass( ABlobFrontEnd ) ) )
		TSTATICCAST( ABlobFrontEnd, pParent )->SetCursorTrailsVisible( TFALSE );
}

// $deBlob: FUNCTION 0062aa50
void ABlobMenu::EndAttractMode()
{
	TXUI::GetSingleton()->GetCanvas()->SetVisible( TTRUE );
	TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
	SetVisible( TTRUE );

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );

	if ( pFrontEnd != TNULL )
	{
		if ( pFrontEnd->GetBackgroundMovie() != -1 )
			pFrontEnd->PlayBackgroundMovie( pFrontEnd->GetBackgroundMovie(), ABlobFrontEnd::BGMOVIE_None, TTRUE, 1, TFALSE );

		TXUI::GetSingleton()->SetInputLocked( TTRUE );
		pFrontEnd->SetInputLockTime( 0.5f );
		pFrontEnd->SetCursorTrailsVisible( TTRUE );
	}

	m_bAttractMovie  = TFALSE;
	m_bAttractFading = TFALSE;

	AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.25f );

	m_fIdleTime = 0.0f;
}

// $deBlob: FUNCTION 0062a6e0
void ABlobMenu::UpdateAttractMode( TFLOAT a_fDeltaTime )
{
	TXUIScene* pParentScene = FindParentScene();

	if ( pParentScene == TNULL || TXUI::GetSingleton()->GetTransToScene() != TNULL || TXUI::GetSingleton()->GetTransBackFromScene() != TNULL || TXUI::GetSingleton()->IsInputLocked() )
		return;


	if ( m_bAttractFading )
	{
		if ( AGameStateController::GetSingleton()->IsFading( 0 ) )
		{
			m_fAttractFadeTime += a_fDeltaTime;
			return;
		}

		TSTATICCAST( ABlobFrontEnd, pParentScene )->StopBackgroundMovie();

		if ( AMoviePlayer::GetSingleton()->PlayMovie( "00_ChromaCity", 0, AMoviePlayer::PlayFlags_None ) )
			StartAttractMode();
	}
	else if ( m_bAttractMovie )
	{
		if ( !AMoviePlayer::GetSingleton()->IsMoviePlaying() )
		{
			EndAttractMode();
			return;
		}

		// Skipped with Return/Escape or a controller's Start/context buttons
		TInputDeviceKeyboard* pKeyboard   = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );
		TBOOL                 bSkip       = pKeyboard->IsDown( TInputDeviceKeyboard::KEY_RETURN ) || pKeyboard->IsDown( TInputDeviceKeyboard::KEY_ESCAPE );
		TINT                  iNumDevices = TInputInterface::GetSingleton()->GetDeviceList().Count();

		for ( TINT i = 0; !bSkip; i++ )
		{
			if ( i >= iNumDevices )
				return;

			TInputDeviceController* pController = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceController>( i );

			if ( pController != TNULL && pController->IsEnabled() &&
			     ( pController->IsDown( TInputDeviceController::BUTTON_CONTEXT_1 ) || pController->IsDown( TInputDeviceController::BUTTON_CONTEXT_2 ) ||
			       pController->IsDown( TInputDeviceController::BUTTON_START ) ) )
			{
				bSkip = TTRUE;
			}
		}

		AMoviePlayer::GetSingleton()->StopMovie();
		EndAttractMode();
	}
	else if ( m_fIdleTime != -1.0f && !AMoviePlayer::GetSingleton()->IsMoviePlaying() )
	{
		TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

		if ( pKeyboard->IsDown( TInputDeviceKeyboard::KEY_RETURN ) || pKeyboard->IsDown( TInputDeviceKeyboard::KEY_ESCAPE ) ||
		     pKeyboard->IsDown( TInputDeviceKeyboard::KEY_LEFT ) || pKeyboard->IsDown( TInputDeviceKeyboard::KEY_RIGHT ) ||
		     pKeyboard->IsDown( TInputDeviceKeyboard::KEY_UP ) || pKeyboard->IsDown( TInputDeviceKeyboard::KEY_DOWN ) )
		{
			m_fIdleTime = 0.0f;
		}

		m_fIdleTime += a_fDeltaTime;

		if ( m_fIdleTime >= 60.0f )
		{
			// A minute without input: fade out, then the attract movie
			m_fIdleTime = 0.0f;
			AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
			m_fAttractFadeTime = 0.0f;
			m_bAttractFading   = TTRUE;
			TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );
		}
	}
}

// $deBlob: FUNCTION 0062a460
void ABlobMenu::PlayPressSound( TXUIElement* a_pSource )
{

	if ( !a_pSource->IsA( &TGetClass( TXUIButton ) ) && !a_pSource->IsA( &TGetClass( TXUIListItem ) ) && !a_pSource->IsA( &TGetClass( TXUIRadioButton ) ) )
		return;

	// NOTE: retail calls a folded "return TTRUE" stub of the game's GUI here, its result isn't used

	// The sounds
	if ( a_pSource->IsA( &TGetClass( TXUIBackButton ) ) )
	{
		if ( ASoundLoadHelper::GetSingleton()->IsFrontEndLoaded() )
			ASoundInterface::PlayEvent( ASoundMap::GROUPID_FRONT_END, ASoundMap::EVENTID( 62 ), FMOD_EVENT_DEFAULT, TTRUE );
	}
	else if ( ASoundLoadHelper::GetSingleton()->IsFrontEndLoaded() )
	{
		ASoundInterface::PlayEvent( ASoundMap::GROUPID_FRONT_END, ASoundMap::EVENTID( 61 ), FMOD_EVENT_DEFAULT, TTRUE );
	}
	else if ( ASoundLoadHelper::GetSingleton()->IsGUILoaded() )
	{
		ASoundInterface::PlayEvent( ASoundMap::GROUPID_GUI, ASoundMap::EVENTID( 82 ), FMOD_EVENT_DEFAULT, TTRUE );
	}
}

// $deBlob: FUNCTION 0062a520
void ABlobMenu::PlayFocusSound( TXUIElement* a_pSource )
{

	if ( !TSTATICCAST( TXUIControl, a_pSource )->IsEnabled() || !a_pSource->IsVisible() )
		return;

	if ( !a_pSource->IsA( &TGetClass( TXUIButton ) ) && !a_pSource->IsA( &TGetClass( TXUIListItem ) ) && !a_pSource->IsA( &TGetClass( TXUIRadioButton ) ) )
		return;

	// The sounds
	if ( ASoundLoadHelper::GetSingleton()->IsFrontEndLoaded() )
		ASoundInterface::PlayEvent( ASoundMap::GROUPID_FRONT_END, ASoundMap::EVENTID( 63 ), FMOD_EVENT_DEFAULT, TTRUE );
	else if ( ASoundLoadHelper::GetSingleton()->IsGUILoaded() )
		ASoundInterface::PlayEvent( ASoundMap::GROUPID_GUI, ASoundMap::EVENTID( 81 ), FMOD_EVENT_DEFAULT, TTRUE );
}

// $deBlob: FUNCTION 0062a5d0
void ABlobMenu::PlayUnfocusSound( TXUIElement* a_pSource )
{

	if ( !a_pSource->IsVisible() )
		return;

	if ( !a_pSource->IsA( &TGetClass( TXUIButton ) ) && !a_pSource->IsA( &TGetClass( TXUIListItem ) ) && !a_pSource->IsA( &TGetClass( TXUIRadioButton ) ) )
		return;

	// The sounds
	if ( ASoundLoadHelper::GetSingleton() != TNULL && ASoundLoadHelper::GetSingleton()->IsFrontEndLoaded() )
		ASoundInterface::PlayEvent( ASoundMap::GROUPID_FRONT_END, ASoundMap::EVENTID( 64 ), FMOD_EVENT_DEFAULT, TTRUE );
}

// $deBlob: FUNCTION 0062a650
void ABlobMenu::OnAttractModeUpdate()
{
}

// $deBlob: FUNCTION 0062a660
TXUIScene* ABlobMenu::FindFrontEndScene( const TWCHAR* a_wszName )
{
	TXUIScene* pParentScene;

	for ( pParentScene = FindParentScene(); pParentScene != TNULL; pParentScene = pParentScene->FindParentScene() )
	{
		if ( TStringManager::String16CompareNoCase( pParentScene->GetID(), L"FrontEnd" ) == 0 )
			break;
	}

	TASSERT( pParentScene != TNULL );
	return FindChildScene( pParentScene, a_wszName );
}

// $deBlob: FUNCTION 0062ad50
TXUIScene* ABlobMenu::FindChildScene( T2GUIElement* a_pParent, const TWCHAR* a_wszName )
{
	const T2GUIElementNode* pEnd = a_pParent->GetChildrenEnd();

	for ( T2GUIElementNode* pNode = pEnd->GetNext(); pNode != pEnd && pNode != TNULL; pNode = pNode->GetNext() )
	{
		T2GUIElement* pChild = TSTATICCAST( T2GUIElement, pNode );

		if ( pChild->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) )
		{
			TXUIElement* pElement = TSTATICCAST( TXUIElement, pChild );

			if ( pElement->IsA( &TGetClass( TXUIScene ) ) )
			{
				if ( TStringManager::String16CompareNoCase( pElement->GetID(), a_wszName ) == 0 )
					return TSTATICCAST( TXUIScene, pElement );

				TXUIScene* pResult = FindChildScene( pElement, a_wszName );

				if ( pResult != TNULL )
					return pResult;
			}
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 0062add0
void ABlobMenu::SetBackButtonText( TXUIElement* a_pElement )
{
	for ( T2GUIElement* pChild = a_pElement->ChildBegin(); pChild != TNULL; pChild = a_pElement->GetNextChild( pChild ) )
	{
		if ( !pChild->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) )
			continue;

		TXUIElement* pXUIChild = TSTATICCAST( TXUIElement, pChild );

		if ( pXUIChild->IsA( &TGetClass( TXUIScene ) ) || pXUIChild->IsA( &TGetClass( TXUIBackButton ) ) )
		{
			TXUIVisual* pVisual = TSTATICCAST( TXUIControl, pXUIChild )->GetVisual();

			if ( pVisual != TNULL && TStringManager::String16Compare( pVisual->GetID(), L"XuiBackButton", -1 ) == 0 )
			{
				TXUIElement* pText = pVisual->FindChildXUIElement( L"XuiText1" );

				if ( pText != TNULL && pText->IsA( &TGetClass( TXUIText ) ) )
					TSTATICCAST( TXUIText, pText )->SetText( T2Locale::GetSingleton()->GetString( 2 ) );
			}
		}

		SetBackButtonText( pXUIChild );
	}
}

// $deBlob: FUNCTION 0062aaf0
void ABlobMenu::UpdateNewFlagLabel( TXUIControl* a_pControl, TUINT32 a_uiNewFlag )
{
	EnSaveData::SlotData* pData = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();

	if ( ( pData->m_uiNewFlagsUnseen & a_uiNewFlag ) == 0 )
		a_pControl->SetVisible( ( pData->m_uiNewFlags & a_uiNewFlag ) != 0 );
	else
		a_pControl->SetVisible( TFALSE );
}

// $deBlob: FUNCTION 0062ab40
void ABlobMenu::RevealNewFlagLabel( TXUIControl* a_pControl, TUINT32 a_uiNewFlag )
{
	APlayerProfile*       pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
	EnSaveData::SlotData* pData    = pProfile->GetData();

	if ( ( pData->m_uiNewFlagsUnseen & a_uiNewFlag ) == 0 )
		return;

	pData->m_uiNewFlagsUnseen &= ~a_uiNewFlag;
	TASSERT( pProfile->GetData()->GetNewFlag( a_uiNewFlag ) );
	TASSERT( TFALSE == a_pControl->IsVisible() );

	a_pControl->SetVisible( TTRUE );
	a_pControl->GetVisual()->FindChildXUIElement( L"Reveal" )->PlayAnimation( L"Reveal", L"EndReveal" );
}

// $deBlob: FUNCTION 0062ac20
void ABlobMenu::UpdateNewFlagLabel_extended( TXUIControl* a_pControl, TUINT32 a_uiNewFlag )
{
	EnSaveData::SlotData* pData = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();

	if ( ( pData->m_uiNewFlagsExtendedUnseen & a_uiNewFlag ) == 0 )
		a_pControl->SetVisible( ( pData->m_uiNewFlagsExtended & a_uiNewFlag ) != 0 );
	else
		a_pControl->SetVisible( TFALSE );
}

// $deBlob: FUNCTION 0062ac70
void ABlobMenu::RevealNewFlagLabel_extended( TXUIControl* a_pControl, TUINT32 a_uiNewFlag )
{
	APlayerProfile*       pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
	EnSaveData::SlotData* pData    = pProfile->GetData();

	if ( ( pData->m_uiNewFlagsExtendedUnseen & a_uiNewFlag ) == 0 )
		return;

	pData->m_uiNewFlagsExtendedUnseen &= ~a_uiNewFlag;
	TASSERT( pProfile->GetData()->GetNewFlag_extended( a_uiNewFlag ) );
	TASSERT( TFALSE == a_pControl->IsVisible() );

	a_pControl->SetVisible( TTRUE );
	a_pControl->GetVisual()->FindChildXUIElement( L"Reveal" )->PlayAnimation( L"Reveal", L"EndReveal" );
}
