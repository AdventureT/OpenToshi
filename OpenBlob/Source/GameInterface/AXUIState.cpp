#include "pch.h"
#include "Cameras/ACameraManager.h"
#include "AXUIState.h"
#include "Sound/AMusicManager.h"
#include "GUI/AGUIManager.h"
#include "AAssetStreaming.h"
#include "Locale/ALocaleManager.h"
#include "Memory/AMemory.h"
#include "AGameStateController.h"
#include "GameController/AGameControllerHelper.h"
#include "ASaveLoadStartAppState.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include "XUI/FrontEnd/ABlobMainMenu.h"
#include "XUI/FrontEnd/ABlobLevelSelection.h"
#include "APlayerProfileManager.h"
#include "XUI/ABlobControllerPopup.h"
#include "Movie/AMoviePlayer.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIScene.h>
#include <XUI/TXUICanvas.h>
#include <XUI/TXUIInput.h>
#include <Render/TAssetInit.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TCHAR AXUIState::s_AssetLoadQueue[ BUFFSIZE ];
// $deBlob: FUNCTION 0053c890
// $deBlob: FUNCTION 00858580
Toshi::TTRB AXUIState::s_FontTRB;
TBOOL       AXUIState::s_bFontLoaded;
// $deBlob: FUNCTION 0053c870
// $deBlob: FUNCTION 00858570
Toshi::TTRB AXUIState::s_CommonTRB;
TBOOL       AXUIState::s_bCommonLoaded;
// $deBlob: FUNCTION 0053c8b0
// $deBlob: FUNCTION 00858590
Toshi::TTRB  AXUIState::s_SkinTRB;
TBOOL        AXUIState::s_bSkinLoaded;
TMemoryHeap* AXUIState::s_pSkinOldTRBHeap;
// $deBlob: FUNCTION 0053c830
// $deBlob: FUNCTION 00858550
Toshi::TTRB AXUIState::s_StreamedTRB;
TBOOL       AXUIState::m_bLoading;
TBOOL       AXUIState::s_bStreamedTRBLoaded;
// $deBlob: FUNCTION 0053c850
// $deBlob: FUNCTION 00858560
Toshi::TTRB AXUIState::s_InGameTRB;
TBOOL       AXUIState::s_bInGameLoaded;

// NOTE: no CreateObject (the state takes its assets)
// $deBlob: FUNCTION 005d3040
// $deBlob: FUNCTION 005d3050
// $deBlob: FUNCTION 005d3060
// $deBlob: FUNCTION 005d3070
// $deBlob: FUNCTION 005d3080
// $deBlob: FUNCTION 0053c7f0
// $deBlob: FUNCTION 00858540
TDEFINE_CLASS_NORUNTIME( AXUIState );

// $deBlob: FUNCTION 005b9d20
AXUIState::Assets::Assets()
{
	m_uiUnk00          = 0;
	m_uiUnk04          = 0;
	m_uiUnk08           = 0;
	m_szSoundFile       = TNULL;
	m_sResourceName     = TNULL;
	m_wszSceneName      = TNULL;
	m_fSoundVolume      = 1.0f;
	m_bCacheScene       = TFALSE;
	m_bActivatesCameras = TFALSE;
	m_pTargetParent     = TNULL;
	m_bDelayed          = TFALSE;
	m_iTransitionIndex  = -1;
	m_bUnk2C            = TFALSE;
	m_bBackFromLevel    = TFALSE;
}

// $deBlob: FUNCTION 005d3090
AXUIState::AXUIState( const Assets& a_rAssets )
{
	m_pSound                  = TNULL;
	m_pChannel                = TNULL;
	m_uiUnk38                 = 0;
	m_eNextState              = 0;
	m_bActivatesCameras       = TFALSE;
	m_pPreviousScene          = TNULL;
	m_szNextLevel             = TNULL;
	m_wszNextLevelName        = TNULL;
	m_iLevelIndex             = -1;
	m_pScene                  = TNULL;
	m_bFadeToNextState        = TFALSE;
	m_bFadedOut               = TFALSE;
	m_bTransitioned           = TFALSE;
	m_pTargetParent           = TNULL;
	m_bDelayedCacheScene      = TFALSE;
	m_iDelayedTransitionIndex = -1;
	m_bPendingTransition      = TFALSE;
	m_bBackFromLevel          = TFALSE;
	m_pControllerPopup        = TNULL;

	TXUI* pXUI          = TXUI::GetSingleton();
	m_bActivatesCameras = a_rAssets.m_bActivatesCameras;

	if ( a_rAssets.m_bDelayed == TFALSE )
	{
		TASSERT( TNULL != a_rAssets.m_sResourceName );
		TXUIResource* pResource = pXUI->FindResource( a_rAssets.m_sResourceName );

		if ( a_rAssets.m_wszSceneName == TNULL )
			m_pScene = pXUI->CreateScene( pResource, TUINT32( 0 ) );
		else
			m_pScene = pXUI->CreateScene( pResource, a_rAssets.m_wszSceneName, TFALSE );

		if ( a_rAssets.m_pTargetParent != TNULL )
			a_rAssets.m_pTargetParent->AddChildTail( m_pScene );

		if ( a_rAssets.m_bCacheScene == TTRUE )
			m_pScene->SetCached();

		if ( a_rAssets.m_iTransitionIndex != -1 )
			m_pScene->SetTransitionIndex( a_rAssets.m_iTransitionIndex );
	}
	else
	{
		if ( a_rAssets.m_wszSceneName == TNULL )
			TStringManager::String16Format( m_wszSceneName, L"", 256 );
		else
			TStringManager::String16Copy( m_wszSceneName, a_rAssets.m_wszSceneName, 256 );

		TStringManager::String8Copy( m_szResourceName, a_rAssets.m_sResourceName, 256 );
		m_bDelayedCacheScene      = a_rAssets.m_bCacheScene;
		m_iDelayedTransitionIndex = a_rAssets.m_iTransitionIndex;
		m_pTargetParent           = a_rAssets.m_pTargetParent;
		m_bBackFromLevel          = a_rAssets.m_bBackFromLevel;
	}

	// The sound of the menu (a stream that plays while the state is on)
	if ( a_rAssets.m_szSoundFile != TNULL && TSound::GetSingleton() != TNULL )
	{
		FMOD_RESULT result = TSound::GetSingleton()->GetSystem()->createStream( a_rAssets.m_szSoundFile, FMOD_LOOP_NORMAL | FMOD_2D | FMOD_HARDWARE, TNULL, &m_pSound );
		TASSERT( !TSound::ErrorCheck( result ) );

		TSound::GetSingleton()->GetSystem()->playSound( FMOD_CHANNEL_FREE, m_pSound, true, &m_pChannel );
		m_pChannel->setVolume( a_rAssets.m_fSoundVolume );
		m_pChannel->setPaused( false );
	}

	if ( m_bLoading == TFALSE && m_pScene != TNULL )
	{
		pXUI->TransitionTo( TNULL, m_pScene );
		m_bTransitioned = TTRUE;
	}
}

// $deBlob: FUNCTION 005d4590
// $deBlob: FUNCTION 005d3340
AXUIState::~AXUIState()
{
	UnloadStreamedTRB();
}

// $deBlob: FUNCTION 005d3ca0
void AXUIState::UnloadStreamedTRB()
{
	TASSERT( TFALSE == m_bLoading );

	if ( s_bStreamedTRBLoaded )
	{
		TAssetInit::DeinitAssets( s_StreamedTRB );
		s_StreamedTRB.Close();
		s_bStreamedTRBLoaded = TFALSE;
	}
}

// $deBlob: FUNCTION 005d3fa0
AGameState::UpdateResult AXUIState::OnUpdate( TFLOAT a_fDeltaTime )
{
	// The streamed TRB is loaded: creates the delayed scene
	if ( m_bLoading && s_bStreamedTRBLoaded )
	{
		m_bLoading = TFALSE;

		TXUI*         pXUI      = TXUI::GetSingleton();
		TXUIResource* pResource = pXUI->FindResource( m_szResourceName );

		if ( TStringManager::String16Length( m_wszSceneName ) < 1 )
			m_pScene = pXUI->CreateScene( pResource, TUINT32( 0 ) );
		else
			m_pScene = pXUI->CreateScene( pResource, m_wszSceneName, TFALSE );

		if ( m_pTargetParent != TNULL )
			m_pTargetParent->AddChildTail( m_pScene );

		if ( m_bDelayedCacheScene == TTRUE )
			m_pScene->SetCached();

		if ( m_iDelayedTransitionIndex != -1 )
			m_pScene->SetTransitionIndex( m_iDelayedTransitionIndex );

		m_bPendingTransition = TTRUE;
	}

	if ( m_bPendingTransition )
	{
		TXUI::GetSingleton()->TransitionTo( TNULL, m_pScene );

		if ( m_bBackFromLevel )
		{
			// Back from a level: the scene is ABlobMainMenu or ABlobLevelSelection (over the main menu) of the front end
			TXUIElement* pFrontEndElement = TSTATICCAST( TXUIElement, m_pTargetParent );
			TASSERT( pFrontEndElement->IsA( &TGetClass( ABlobFrontEnd ) ) );
			ABlobFrontEnd*    pFrontEnd  = TSTATICCAST( ABlobFrontEnd, pFrontEndElement );
			Toshi::TXUIScene* pMenuScene = TNULL; // v7

			if ( m_pScene->IsA( &TGetClass( ABlobMainMenu ) ) )
			{
				pMenuScene = m_pScene;
				pMenuScene->SetTransitionVisual( Toshi::TXUIScene::Transition_To, L"TransTo" );
			}
			else if ( m_pScene->IsA( &TGetClass( ABlobLevelSelection ) ) )
			{
				// The main menu stays hidden behind the level selection, the background movie of the worlds unlocked
				pMenuScene = TSTATICCAST( Toshi::TXUIScene, pFrontEnd->FindChildXUIControl( L"MainMenu" ) );
				m_pScene->SetScale( 1.0f, 1.0f );
				m_pScene->SetTransitionVisual( Toshi::TXUIScene::Transition_To, L"TransTo" );
				pMenuScene->SetXUIStateBits( Toshi::TXUIScene::State_Hidden, 0xF, Toshi::TXUIElement::XUIState_TimelineStateShift );
				pMenuScene->SetVisible( TFALSE );
				m_pScene->SetPrevScene( pMenuScene );

				TUINT32 uiWorlds   = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData()->m_iUnlockedWorlds;
				TINT    iNumWorlds = 0;

				for ( TINT i = 0; i < 10; i++ )
				{
					if ( uiWorlds & ( 1 << i ) )
						iNumWorlds++;
				}

				pFrontEnd->PlayBackgroundMovie( iNumWorlds + 37, -1, TTRUE, TFALSE, TFALSE );
			}

			// NOTE: retail has no TNULL check (the scene is always one of the two)
			pMenuScene->SetPrevScene( TSTATICCAST( Toshi::TXUIScene, pFrontEnd->FindChildXUIControl( L"Splash" ) ) );
		}

		AGameStateController::GetSingleton()->FadeFromBlack( 0.5f, 0 );
		m_bPendingTransition = TFALSE;
		m_bTransitioned      = TTRUE;

		TASSERT( m_pTargetParent && TSTATICCAST( TXUIElement, m_pTargetParent )->IsA( &TGetClass( ABlobFrontEnd ) ) );
		TSTATICCAST( ABlobFrontEnd, m_pTargetParent )->SetCursorTrailsVisible( TTRUE );

		AMusicManager::GetSingleton()->StartMusic( TFALSE );

		TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
	}

	if ( !m_bTransitioned )
		return AGameState::OnUpdate( a_fDeltaTime );

	if ( m_eNextState == NextState_None )
	{
		TXUI* pXUI = TXUI::GetSingleton();

		if ( pXUI->GetActiveScene() != TNULL && pXUI->GetTransToScene() == TNULL && pXUI->GetTransBackFromScene() == TNULL && !pXUI->IsInputLocked() && pXUI->GetActiveScene()->GetPrevScene() == TNULL )
		{
			m_InputHelper.Update( a_fDeltaTime );
		}
	}
	else if ( m_bFadeToNextState )
	{
		if ( !AGameStateController::GetSingleton()->IsFading( 0 ) )
		{
			if ( m_bFadedOut )
				return GoToNextState();

			m_bFadedOut = TTRUE;
		}
	}
	else
	{
		if ( m_eNextState == NextState_LoadLevel )
		{
			AGameControllerHelper::LoadLevel( m_szNextLevel, m_wszNextLevelName );
			return UpdateResult_OK;
		}

		if ( m_eNextState == NextState_FrontEnd )
		{
			AGameControllerHelper::ReturnToFrontEnd( TFALSE );
			return UpdateResult_OK;
		}

		return UpdateResult_Remove;
	}

	if ( !AGameStateController::GetSingleton()->IsFading( 0 ) && AMoviePlayer::GetSingleton()->IsMoviePlaying() == TFALSE )
		TXUI::GetSingleton()->TickInput( a_fDeltaTime );

	m_pControllerPopup->UpdateFrontEnd( a_fDeltaTime );

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $deBlob: FUNCTION 005d4430
AGameState::UpdateResult AXUIState::GoToNextState()
{
	if ( m_eNextState == NextState_LoadLevel )
	{
		AGameControllerHelper::LoadLevel( m_szNextLevel, m_wszNextLevelName );
		return UpdateResult_OK;
	}

	if ( m_eNextState == NextState_FrontEnd )
	{
		AGameControllerHelper::ReturnToFrontEnd( TFALSE );
		return UpdateResult_OK;
	}

	return UpdateResult_Remove;
}

// $deBlob: FUNCTION 005d33b0
void AXUIState::SetNextState( TINT a_eNextState )
{
	m_eNextState       = a_eNextState;
	m_bFadeToNextState = a_eNextState == NextState_LoadLevel;

	if ( a_eNextState == NextState_LoadLevel )
		AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
}

// $deBlob: FUNCTION 005d3e00
void AXUIState::OnInsertion()
{
	TXUIResource* pResource = TXUI::GetSingleton()->FindResource( "controllerscreen.xur" );
	TXUIScene*    pScene    = TXUI::GetSingleton()->CreateScene( pResource, L"ControllerPopup", TTRUE );

	TASSERT( pScene == TNULL || pScene->IsA( &TGetClass( ABlobControllerPopup ) ) );

	// Not shown until it's needed
	m_pControllerPopup = TSTATICCAST( ABlobControllerPopup, pScene );
	m_pControllerPopup->RemoveSelf();
}

// $deBlob: FUNCTION 005d3eb0
void AXUIState::OnRemoval()
{
	m_pControllerPopup = TNULL;

	if ( m_pPreviousScene != TNULL )
		m_pPreviousScene->SetVisible( TTRUE );

	if ( m_eNextState == NextState_LoadLevel || m_eNextState == NextState_FrontEnd || m_eNextState == NextState_SaveLoadStart )
	{
		TXUI::GetSingleton()->Reset();

		if ( m_eNextState == NextState_SaveLoadStart )
			AGameStateController::GetSingleton()->PushState( new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) ASaveLoadStartAppState() );
	}
	else
	{
		TXUI::GetSingleton()->TransitionBack( m_pPreviousScene );
	}

	// The sound of the menu
	if ( m_pChannel != TNULL )
		m_pChannel->stop();

	if ( m_pSound != TNULL )
		m_pSound->release();

	AGameState::OnRemoval();
}

// $deBlob: FUNCTION 005d3d00
void AXUIState::OnActivate()
{
	AInputManager2::GetSingleton()->SetContext( AInputMap::INPUTCONTEXT_Menu );
	m_InputHelper.AddMapping( 0, TFALSE, -1.0f );
	m_InputHelper.ResetJustDown();

	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );

	if ( m_pPreviousScene != TNULL )
	{
		// Shows the topmost scene above the previous one
		TXUIElement*  pScene  = m_pPreviousScene;
		T2GUIElement* pParent = m_pPreviousScene->GetParent();

		if ( pParent != TNULL )
		{
			// NOTE: retail reads the flags of the parent before checking it for TNULL (the chain always ends at
			// T2GUI's root, which isn't a XUI element)
			do
			{
				TXUIElement* pXUIParent = TSTATICCAST( TXUIElement, pParent );

				if ( pXUIParent->IsA( &TGetClass( TXUIScene ) ) )
					pScene = pXUIParent;

				pParent = pParent->GetParent();
			} while ( pParent->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) && pParent != TNULL );
		}

		pScene->SetVisible( TTRUE );
		TXUI::GetSingleton()->SetFocussedScene( m_pPreviousScene );
		m_pPreviousScene = TNULL;
	}

	if ( m_bActivatesCameras )
	{
		ACameraManager::GetSingleton()->Activate();
	}

	AGameState::OnActivate();
}

// $deBlob: FUNCTION 005d3db0
void AXUIState::OnDeactivate()
{
	m_InputHelper.ResetJustDown();
	AInputManager2::GetSingleton()->RemoveCurrentContext();

	m_pPreviousScene = TXUI::GetSingleton()->GetActiveScene();

	if ( m_pPreviousScene != TNULL )
		m_pPreviousScene->SetVisible( TFALSE );

	if ( m_bActivatesCameras )
	{
		ACameraManager::GetSingleton()->Deactivate();
	}

	AGameState::OnDeactivate();
}

void AXUIState::MakeLocalisedPath( const TCHAR* a_szFileName )
{
	TStringManager::String8Copy( s_AssetLoadQueue, "data/xui/", BUFFSIZE );
	TINT iLen = TStringManager::String8Length( s_AssetLoadQueue );
	TStringManager::String8Copy( s_AssetLoadQueue + iLen, TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() )->GetCurLocaleCode(), BUFFSIZE - iLen );

	iLen = TStringManager::String8Length( s_AssetLoadQueue );
	TASSERT( iLen < BUFFSIZE );
	s_AssetLoadQueue[ iLen ] = '/';
	TStringManager::String8Copy( s_AssetLoadQueue + iLen + 1, a_szFileName, BUFFSIZE - ( iLen + 1 ) );
}

// $deBlob: FUNCTION 005d33f0
void AXUIState::SetSkin1( const TCHAR* a_szTRBFileName, const TCHAR* a_szXURName )
{
	MakeLocalisedPath( a_szTRBFileName );
	TXUI::GetSingleton()->SetSkin1( s_AssetLoadQueue, a_szXURName );
}

// $deBlob: FUNCTION 005d34b0
void AXUIState::SetSkin2( const TCHAR* a_szTRBFileName, const TCHAR* a_szXURName )
{
	MakeLocalisedPath( a_szTRBFileName );
	TXUI::GetSingleton()->SetSkin2( s_AssetLoadQueue, a_szXURName );
}

// $deBlob: FUNCTION 005d3570
TBOOL AXUIState::InitFont( const TCHAR* a_szTRBFileName )
{
	s_FontTRB.SetMemoryFunctions( AMemory::TemporaryTRBAllocator, AMemory::TemporaryTRBDeallocator, TNULL );
	TMemoryHeap* pOldTRBHeap = AMemory::SetTRBHeap( TMemory_GetGlobalHeap() );

	if ( s_bFontLoaded == TTRUE )
	{
		TAssetInit::DeinitAssets( s_FontTRB );
		s_FontTRB.Close();
		s_bFontLoaded = TFALSE;
	}

	TTRB::ERROR error = s_FontTRB.Load( a_szTRBFileName );
	TASSERT( error == TTRB::ERROR_OK );

	TAssetInit::InitAssets( s_FontTRB, TTRUE, TFALSE );
	s_bFontLoaded = TTRUE;
	AMemory::SetTRBHeap( pOldTRBHeap );

	return TTRUE;
}

// $deBlob: FUNCTION 005d3620
TBOOL AXUIState::SetSkinWithFont( const TCHAR* a_szTRBFileName, TBOOL a_bStreamed )
{
	if ( s_bSkinLoaded == TTRUE )
	{
		TAssetInit::DeinitAssets( s_SkinTRB );
		s_SkinTRB.Close();
		s_bSkinLoaded = TFALSE;
	}

	MakeLocalisedPath( a_szTRBFileName );

	s_SkinTRB.SetMemoryFunctions( AMemory::TemporaryTRBAllocator, AMemory::TemporaryTRBDeallocator, TNULL );
	s_pSkinOldTRBHeap = AMemory::SetTRBHeap( TXUI::ms_pXUIMemoryBlock );

	if ( a_bStreamed )
	{
		AAssetStreaming* pAssetStreaming = AAssetStreaming::GetSingleton();
		ATRBLoaderJob*   pJob            = pAssetStreaming->GetFreeJob();
		pJob->Init( &s_SkinTRB, s_AssetLoadQueue );
		pJob->m_fnOnFinished        = SkinWithFontLoaded;
		pJob->m_pOnFinishedUserData = TNULL;
		pAssetStreaming->AddMainThreadJob2( pJob );

		return TTRUE;
	}

	TTRB::ERROR error = s_SkinTRB.Load( s_AssetLoadQueue );
	TASSERT( error == TTRB::ERROR_OK );

	TAssetInit::InitAssets( s_SkinTRB, TTRUE, TFALSE );
	s_bSkinLoaded = TTRUE;
	AMemory::SetTRBHeap( s_pSkinOldTRBHeap );
	s_pSkinOldTRBHeap = TNULL;

	return TTRUE;
}

// The end of the streamed SetSkinWithFont
// $deBlob: FUNCTION 005d43c0
void AXUIState::SkinWithFontLoaded( ATRBLoaderJob* a_pJob, void* a_pUserData )
{
	TAssetInit::InitAssets( s_SkinTRB, TTRUE, TFALSE );
	s_bSkinLoaded = TTRUE;
	AMemory::SetTRBHeap( s_pSkinOldTRBHeap );
	s_pSkinOldTRBHeap = TNULL;
	AAssetStreaming::GetSingleton()->ReleaseJob( a_pJob );
}

// $deBlob: FUNCTION 005d37d0
TBOOL AXUIState::SetCommonSkin( const TCHAR* a_szTRBFileName )
{
	MakeLocalisedPath( a_szTRBFileName );

	s_CommonTRB.SetMemoryFunctions( AMemory::TemporaryTRBAllocator, AMemory::TemporaryTRBDeallocator, TNULL );
	TMemoryHeap* pOldTRBHeap = AMemory::SetTRBHeap( TXUI::ms_pXUIMemoryBlock );

	if ( s_bCommonLoaded == TTRUE )
	{
		TAssetInit::DeinitAssets( s_CommonTRB );
		s_CommonTRB.Close();
		s_bCommonLoaded = TFALSE;
	}

	TTRB::ERROR error = s_CommonTRB.Load( s_AssetLoadQueue );
	TASSERT( error == TTRB::ERROR_OK );

	TAssetInit::InitAssets( s_CommonTRB, TTRUE, TFALSE );
	s_bCommonLoaded = TTRUE;
	AMemory::SetTRBHeap( pOldTRBHeap );

	return TTRUE;
}

// $deBlob: FUNCTION 005d3920
TBOOL AXUIState::SetInGameSkin( const TCHAR* a_szTRBFileName )
{
	MakeLocalisedPath( a_szTRBFileName );

	s_InGameTRB.SetMemoryFunctions( AMemory::TemporaryTRBAllocator, AMemory::TemporaryTRBDeallocator, TNULL );
	TMemoryHeap* pOldTRBHeap = AMemory::SetTRBHeap( TXUI::ms_pXUIMemoryBlock );

	if ( s_bInGameLoaded == TTRUE )
	{
		TAssetInit::DeinitAssets( s_InGameTRB );
		s_InGameTRB.Close();
		s_bInGameLoaded = TFALSE;
	}

	TTRB::ERROR error = s_InGameTRB.Load( s_AssetLoadQueue );
	TASSERT( error == TTRB::ERROR_OK );

	TAssetInit::InitAssets( s_InGameTRB, TTRUE, TFALSE );
	s_bInGameLoaded = TTRUE;
	AMemory::SetTRBHeap( pOldTRBHeap );

	return TTRUE;
}

// $deBlob: FUNCTION 005d3a70
void AXUIState::UnloadSkin()
{
	if ( s_bSkinLoaded == TTRUE )
	{
		TAssetInit::DeinitAssets( s_SkinTRB );
		s_SkinTRB.Close();
		s_bSkinLoaded = TFALSE;
	}
}

// $deBlob: FUNCTION 005d3aa0
void AXUIState::UnloadCommonSkin()
{
	if ( s_bCommonLoaded == TTRUE )
	{
		TAssetInit::DeinitAssets( s_CommonTRB );
		s_CommonTRB.Close();
		s_bCommonLoaded = TFALSE;
	}
}

// $deBlob: FUNCTION 005d3ad0
void AXUIState::UnloadInGameSkin()
{
	if ( s_bInGameLoaded == TTRUE )
	{
		TAssetInit::DeinitAssets( s_InGameTRB );
		s_InGameTRB.Close();
		s_bInGameLoaded = TFALSE;
	}
}

// $deBlob: FUNCTION 005d3b00
void AXUIState::UnloadFontSkin()
{
	if ( s_bFontLoaded == TTRUE )
	{
		TAssetInit::DeinitAssets( s_FontTRB );
		s_FontTRB.Close();
		s_bFontLoaded = TFALSE;
	}
}

// $deBlob: FUNCTION 005d3b30
void AXUIState::ResetXUI()
{
	TXUI::GetSingleton()->Reset();
}

// $deBlob: FUNCTION 005d3bb0
void AXUIState::LoadFrontendCarousel()
{
	MakeLocalisedPath( "frontendcarousel.trb" );

	AAssetStreaming* pAssetStreaming = AAssetStreaming::GetSingleton();
	ATRBLoaderJob*   pJob            = pAssetStreaming->GetFreeJob();
	pJob->Init( &s_StreamedTRB, s_AssetLoadQueue );
	pJob->m_fnOnFinished        = FrontendCarouselLoaded;
	pJob->m_pOnFinishedUserData = TNULL;
	pAssetStreaming->AddMainThreadJob2( pJob );

	m_bLoading = TTRUE;
}

// The end of LoadFrontendCarousel: creates the cached level selection scene
// $deBlob: FUNCTION 005d3b40
void AXUIState::FrontendCarouselLoaded( ATRBLoaderJob* a_pJob, void* a_pUserData )
{
	TAssetInit::InitAssets( s_StreamedTRB, TTRUE, TFALSE );
	s_bStreamedTRBLoaded = TTRUE;

	TXUIScene* pScene = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "levelselection.xur" ), L"LevelSelection", TTRUE );
	pScene->RemoveSelf();

	AAssetStreaming::GetSingleton()->ReleaseJob( a_pJob );
}

// $deBlob: FUNCTION 00635eb0
void AXUIState::SetNextLevel( const TCHAR* a_szLevel, TINT a_iNameId, TINT a_iLevelIndex )
{
	m_szNextLevel = a_szLevel;

	if ( a_iNameId == -1 )
	{
		m_wszNextLevelName = L"";
		m_iLevelIndex      = a_iLevelIndex;
	}
	else
	{
		m_wszNextLevelName = T2Locale::GetSingleton()->GetString( a_iNameId );
		m_iLevelIndex      = a_iLevelIndex;
	}
}
