#include "pch.h"
#include "Buildings/ABuildingManager.h"
#include "ACheatManager.h"
#include "Input/AInputManager2.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/AXUIState.h"
#include "GameInterface/APlayerManager.h"
#include "GameInterface/APlayerProfileManager.h"
#include "GameController/AGameStatus.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include "XUI/FrontEnd/ABlobMainMenu.h"
#include "Sound/ASoundInterface.h"
#include "GUI/AGUIManager.h"
#include "Memory/AMemory.h"
#include "MemoryCard/SaveLoad.h"

#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputInterface.h>
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUITextBox.h>
#include <GUI/T2GUIFontManager.h>
#include <T2Locale/T2Locale.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIScene.h>
#include <XUI/TXUIControl.h>
#include <Toshi/TStringManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// A cheat sequence of the tables (0x30 bytes): the indices into ACheatManager's doodads (a keyboard doodad or its
// gamepad twin, +8), oldest first, and the function that runs
struct ACheatSequence
{
	TINT                           m_aiDoodads[ 10 ]; // 0x00
	TINT                           m_iLength;         // 0x28
	ACheatManager::t_CheatCallback m_pfnCallback;     // 0x2C
};

TSTATICASSERT( sizeof( ACheatSequence ) == 0x30 );
TSTATICASSERT( sizeof( ACheatManager ) == 0x3C0 );

// The doodad ids of the keyboard keys and the gamepad buttons: the keys 0x20015, 0x20022-24 and 0x20030-33, the pad
// 0x10001-04 and 0x10009-0C
static TCONSTEXPR_VAR TINT s_aiCheatDoodads[ ACheatManager::NUM_DOODADS ] = {
	0x20015, 0x20023, 0x20022, 0x20024, 0x20030, 0x20031, 0x20032, 0x20033,
	0x10001, 0x10003, 0x10004, 0x10002, 0x10009, 0x1000A, 0x1000B, 0x1000C
};

static TCONSTEXPR_VAR TINT s_aiShoulderDoodads[ ACheatManager::NUM_SHOULDERDOODADS ] = { 0x10005, 0x20052 };

// $deBlob: FUNCTION 005a0780
ACheatManager::ACheatManager()
{
	m_iQueue0Index = 0;
	m_iQueue1Index = 0;
	m_bEnabled     = TFALSE;
	m_bInGame      = TTRUE;
	m_bSequence    = TFALSE;

	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );
	m_oKeyboardHelper.~AInputHelperSimple();
	new ( &m_oKeyboardHelper ) AInputHelperSimple( 16, pKeyboard );
	m_oKeyboardShoulderHelper.~AInputHelperSimple();
	new ( &m_oKeyboardShoulderHelper ) AInputHelperSimple( 2, pKeyboard );

	m_pCheatDevice      = TNULL;
	m_iCheatDeviceId    = 0;
	m_pMessageBox       = TNULL;
	m_fMessageTime      = 0.0f;
	m_bScoreJumpPending = TFALSE;
	memset( &m_abShoulderHeld, 0, sizeof( m_abShoulderHeld ) );

	for ( TINT i = 0; i < NUM_PADS; i++ )
	{
		TInputDeviceKeyboard* pDevice = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );
		m_aPadHelpers[ i ].Initialise( 16, pDevice );

		pDevice = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );
		m_aPadShoulderHelpers[ i ].Initialise( 2, pDevice );
	}

	ClearInputQueue();
	SetupInputHelper();
	SetupInputHelperShoulderButtons();
	ClearCheatsEnabled();
	m_wszMessage[ 0 ] = 0;

	// The message text box, centred at the bottom of the safe area
	T2GUIFont*    pFont     = T2GUIFontManager::GetFont( "SHAG_LOUNGE_30" );
	T2GUIElement* pSafeArea = AGUIManager::GetSingleton()->GetSafeArea();
	m_pMessageBox           = new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) T2GUITextBox();
	m_pMessageBox->Create( pFont, pSafeArea->GetWidth() );
	m_pMessageBox->SetText( m_wszMessage );
	m_pMessageBox->SetMaxChars( -1 );
	m_pMessageBox->SetVisible( TFALSE );
	m_pMessageBox->SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_pMessageBox->SetJustify( T2GUIFont::JUSTIFY_CENTRE );
	m_pMessageBox->SetShadow( TTRUE, 4.0f, 4.0f );
	m_pMessageBox->SetShadowColour( 0x7F404040 );
	pSafeArea->AddChildTail( m_pMessageBox );
}

// $deBlob: FUNCTION 005a0b40
// $deBlob: FUNCTION 0056bef0
ACheatManager::~ACheatManager()
{
	if ( m_pMessageBox != TNULL )
	{
		m_pMessageBox->Unlink();

		if ( m_pMessageBox != TNULL )
			delete m_pMessageBox;

		m_pMessageBox = TNULL;
	}
}

// $deBlob: FUNCTION 005a0c50
void ACheatManager::SetEnabled( TBOOL a_bEnabled, TBOOL a_bInGame )
{
	if ( !m_bEnabled )
	{
		if ( a_bEnabled )
			ClearInputQueue();

		if ( !m_bEnabled )
		{
			m_fMessageTime = 0.0f;
			m_pMessageBox->SetVisible( TFALSE );
		}
	}

	m_bEnabled  = a_bEnabled;
	m_bInGame   = a_bInGame;
	m_bSequence = TFALSE;
}

// $deBlob: FUNCTION 005a0cc0
void ACheatManager::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bEnabled )
		return;

	if ( 0.0f < m_fMessageTime )
	{
		m_fMessageTime -= a_fDeltaTime;

		if ( m_fMessageTime <= 0.0f )
			m_pMessageBox->SetVisible( TFALSE );
	}

	for ( TINT i = 0; i < NUM_PADS; i++ )
	{
		m_aPadHelpers[ i ].Update();
		m_aPadShoulderHelpers[ i ].Update();
	}

	m_oKeyboardHelper.Update();
	m_oKeyboardShoulderHelper.Update();

	if ( !IsAllShoulderButtonsDown() )
	{
		if ( m_bSequence )
		{
			ClearInputQueue();
			m_bSequence = TFALSE;
		}

		return;
	}

	m_bSequence = TTRUE;

	TBOOL                 bPadPressed = TFALSE;
	TBOOL                 bPressed    = TFALSE;
	TInputDeviceKeyboard* pKeyboard   = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

	if ( pKeyboard != TNULL )
	{
		m_oKeyboardHelper.SetInputDevice( TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 ) );

		for ( TINT i = 0; i < NUM_DOODADS; i++ )
		{
			TINT iDoodad = m_aDoodads[ i ];

			if ( m_oKeyboardHelper.WasPressed( iDoodad ) )
			{
				TINT iIndex         = m_iQueue0Index++;
				m_aQueue0[ iIndex ] = iDoodad;
				TMath::Wrap( m_iQueue0Index, 0, QUEUE_SIZE - 1 );
				bPressed = TTRUE;
			}
		}

		if ( bPressed )
		{
			CheckSequences( m_aQueue0, m_iQueue0Index, 0 );
			CheckSequences( m_aQueue1, m_iQueue1Index, 1 );
			return;
		}
	}

	AInputManager2* pInputManager = AInputManager2::GetSingleton();
	TINT            iNumPads      = pInputManager->GetConnectedDeviceCount();

	for ( TINT iPad = 0; iPad < iNumPads; iPad++ )
	{
		if ( !m_abShoulderHeld[ iPad ] )
			continue;

		m_aPadHelpers[ iPad ].SetInputDevice( pInputManager->GetControllerHandle( iPad ).m_pDevice );

		for ( TINT i = 0; i < NUM_DOODADS; i++ )
		{
			TINT iDoodad = m_aDoodads[ i ];

			if ( m_aPadHelpers[ iPad ].WasPressed( iDoodad ) )
			{
				if ( iPad == 0 )
				{
					TINT iIndex         = m_iQueue0Index++;
					m_aQueue0[ iIndex ] = iDoodad;
					TMath::Wrap( m_iQueue0Index, 0, QUEUE_SIZE - 1 );
				}
				else if ( iPad == 1 )
				{
					TINT iIndex         = m_iQueue1Index++;
					m_aQueue1[ iIndex ] = iDoodad;
					TMath::Wrap( m_iQueue1Index, 0, QUEUE_SIZE - 1 );
				}

				bPadPressed = TTRUE;
			}
		}
	}

	if ( !bPadPressed )
		return;

	CheckSequences( m_aQueue0, m_iQueue0Index, 0 );
	CheckSequences( m_aQueue1, m_iQueue1Index, 1 );
}

// Sets the enabled flags of the five cheats from the bits of a_uiMask
// $deBlob: FUNCTION 005a11c0
void ACheatManager::SetCheatsEnabled( TUINT a_uiMask )
{
	TUINT uiBit = 1;

	for ( TINT i = 0; i < CHEATS_NUMOF; i++ )
	{
		m_aCheats[ i ].m_bEnabled = ( uiBit & a_uiMask & 0xFF ) != 0;
		uiBit                     = ( uiBit << 1 ) | TUINT( TINT( uiBit ) < 0 );
	}
}

// $deBlob: FUNCTION 005a1240
void ACheatManager::SetCheatEnabledFlag( TINT a_iCheat, TBOOL a_bEnabled )
{
	TASSERT( 0 <= a_iCheat );
	TASSERT( CHEATS_NUMOF > a_iCheat );
	m_aCheats[ a_iCheat ].m_bEnabled = a_bEnabled;
}

// $deBlob: FUNCTION 005a1300
void ACheatManager::SetCheatEnabled( TINT a_iCheat, TBOOL a_bEnabled )
{
	TASSERT( 0 <= a_iCheat );
	TASSERT( CHEATS_NUMOF > a_iCheat );

	Cheat& rCheat = m_aCheats[ a_iCheat ];

	if ( TUINT8( rCheat.m_pfnGet( a_iCheat ) ) != TUINT8( a_bEnabled ) )
		m_aCheats[ a_iCheat ].m_pfnSet( a_bEnabled, a_iCheat );
}

// $deBlob: FUNCTION 005a1410
TINT ACheatManager::GetCode( TINT a_iCheat )
{
	TASSERT( 0 <= a_iCheat );
	TASSERT( CHEATS_NUMOF > a_iCheat );
	return m_aCheats[ a_iCheat ].m_iCode;
}

// $deBlob: FUNCTION 005a14d0
TINT ACheatManager::GetLocaleId( TINT a_iCheat )
{
	TASSERT( 0 <= a_iCheat );
	TASSERT( CHEATS_NUMOF > a_iCheat );
	return m_aCheats[ a_iCheat ].m_iLocaleId;
}

// $deBlob: FUNCTION 005a1590
TBOOL ACheatManager::IsCheatEnabled( TINT a_iCheat )
{
	TASSERT( 0 <= a_iCheat );
	TASSERT( CHEATS_NUMOF > a_iCheat );
	return m_aCheats[ a_iCheat ].m_bEnabled;
}

// Runs the cheat's getter (the result is ignored)
// $deBlob: FUNCTION 005a1650
void ACheatManager::UpdateCheat( TINT a_iCheat )
{
	TASSERT( 0 <= a_iCheat );
	TASSERT( CHEATS_NUMOF > a_iCheat );
	m_aCheats[ a_iCheat ].m_pfnGet( a_iCheat );
}

// $deBlob: FUNCTION 005a1710
TBOOL ACheatManager::IsAllShoulderButtonsDown()
{
	AInputManager2* pInputManager = AInputManager2::GetSingleton();
	TINT            iNumPads      = pInputManager->GetConnectedDeviceCount();
	TBOOL           bKeyboardHeld = TFALSE;

	if ( TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 ) != TNULL )
	{
		m_oKeyboardShoulderHelper.SetInputDevice( TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 ) );
		bKeyboardHeld = m_oKeyboardShoulderHelper.IsHeld( m_aShoulderDoodads[ 1 ] );
	}

	for ( TINT iPad = 0; iPad < iNumPads; iPad++ )
	{
		TInputDevice* pDevice = pInputManager->GetControllerHandle( iPad ).m_pDevice;

		m_abShoulderHeld[ iPad ] = TTRUE;
		m_aPadShoulderHelpers[ iPad ].SetInputDevice( pDevice );

		for ( TINT i = 0; i < NUM_SHOULDERDOODADS; i++ )
		{
			if ( !m_aPadShoulderHelpers[ iPad ].IsHeld( m_aShoulderDoodads[ i ] ) )
			{
				m_abShoulderHeld[ iPad ] = TFALSE;
				break;
			}
		}
	}

	if ( !m_abShoulderHeld[ 0 ] && !m_abShoulderHeld[ 1 ] && !bKeyboardHeld )
		return TFALSE;

	return TTRUE;
}

// $deBlob: FUNCTION 005a1930
TBOOL ACheatManager::SetMessageVisible( TBOOL a_bVisible )
{
	if ( m_pMessageBox == TNULL )
		return TFALSE;

	if ( m_pMessageBox->IsVisible() )
	{
		m_pMessageBox->SetVisible( a_bVisible );
		return TTRUE;
	}

	m_pMessageBox->SetVisible( a_bVisible );
	return TFALSE;
}

// $deBlob: FUNCTION 005a1980
void ACheatManager::SetupInputHelper()
{
	for ( TINT i = 0; i < NUM_DOODADS; i++ )
	{
		m_aDoodads[ i ] = s_aiCheatDoodads[ i ];
	}

	for ( TINT iDoodad = 0; iDoodad < NUM_DOODADS; iDoodad++ )
	{
		for ( TINT iPad = 0; iPad < NUM_PADS; iPad++ )
			m_aPadHelpers[ iPad ].AddDoodad( m_aDoodads[ iDoodad ] );
	}

	for ( TINT iDoodad = 0; iDoodad < 8; iDoodad++ )
		m_oKeyboardHelper.AddDoodad( m_aDoodads[ iDoodad ] );
}

// $deBlob: FUNCTION 005a1b50
void ACheatManager::SetupInputHelperShoulderButtons()
{
	m_aShoulderDoodads[ 0 ] = s_aiShoulderDoodads[ 0 ];
	m_aShoulderDoodads[ 1 ] = s_aiShoulderDoodads[ 1 ];

	for ( TINT iDoodad = 0; iDoodad < NUM_SHOULDERDOODADS; iDoodad++ )
	{
		for ( TINT iPad = 0; iPad < NUM_PADS; iPad++ )
			m_aPadShoulderHelpers[ iPad ].AddDoodad( m_aShoulderDoodads[ iDoodad ] );
	}

	m_oKeyboardShoulderHelper.AddDoodad( m_aShoulderDoodads[ 1 ] );
}

// The cheat sequences of the in-game table (0x87CA98)
static const ACheatSequence s_aInGameSequences[ 4 ] = {
	{ { 4, 4, 4, 4, 0, 0, 0, 0, 0, 0 }, 4, ACheatManager::Cheat_ToggleControlsLock },
	{ { 4, 7, 4, 7, 0, 0, 0, 0, 0, 0 }, 4, ACheatManager::Cheat_AddTime },
	{ { 4, 4, 7, 7, 0, 0, 0, 0, 0, 0 }, 4, ACheatManager::Cheat_AddLife },
	{ { 7, 7, 7, 0, 0, 0, 0, 0, 0, 0 }, 3, ACheatManager::DummyCheat },
};

// The cheat sequences of ABlobMainMenu (0x87C970)
static const ACheatSequence s_aMainMenuSequences[ 4 ] = {
	{ { 4, 7, 5, 4, 7, 5, 4, 7, 0, 0 }, 8, ACheatManager::Cheat_UnlockAll },
	{ { 7, 7, 5, 5, 4, 4, 5, 5, 0, 0 }, 8, ACheatManager::Cheat_UnlockBlobParty },
	{ { 5, 4, 5, 7, 5, 4, 5, 7, 0, 0 }, 8, ACheatManager::Cheat_UnlockExtras },
	{ { 5, 5, 4, 7, 4, 7, 5, 5, 0, 0 }, 8, ACheatManager::Cheat_UnlockTracks },
};

// The cheat sequence of the other menus (0x87CA34)
static const ACheatSequence s_aMenuSequences[ 1 ] = {
	{ { 7, 7, 7, 0, 0, 0, 0, 0, 0, 0 }, 3, ACheatManager::DummyCheat },
};

// $deBlob: FUNCTION 005a1c40
void ACheatManager::CheckSequences( T2Array<TINT, QUEUE_SIZE>& a_rQueue, TINT a_iCount, TINT a_iController )
{
	const ACheatSequence* pSequences    = TNULL;
	TINT                  iNumSequences = 0;
	AGameState*           pState        = AGameStateController::GetSingleton()->GetCurrentGameState();

	if ( m_bInGame && pState->GetClass() == &TGetClass( AInGameState ) )
	{
		pSequences    = s_aInGameSequences;
		iNumSequences = 4;
	}
	else
	{
		TXUI* pXUI = TXUI::GetSingleton();

		if ( !pState->GetClass()->IsA( &TGetClass( AXUIState ) ) ||
		     !TSTATICCAST( AXUIState, pState )->m_bTransitioned ||
		     pXUI->GetTransToScene() != TNULL ||
		     pXUI->GetTransFromScene() != TNULL ||
		     pXUI->IsInputLocked() )
		{
			return;
		}

		// The assert is once per process
		static TBOOL s_bAssertedFrontEnd = TFALSE;

		if ( !s_bAssertedFrontEnd && !pXUI->GetActiveScene()->IsA( &TGetClass( ABlobFrontEnd ) ) )
		{
			TASSERT( pXUI->GetActiveScene()->IsA( &TGetClass( ABlobFrontEnd ) ) );
			s_bAssertedFrontEnd = TTRUE;
		}

		TXUIControl* pFocus    = pXUI->GetActiveScene()->GetFocus( pXUI->GetDefaultController() );
		TBOOL        bMainMenu = TFALSE;

		if ( pFocus != TNULL )
		{
			TXUIScene* pScene = pFocus->IsA( &TGetClass( TXUIScene ) ) ? TREINTERPRETCAST( TXUIScene*, pFocus ) : pFocus->FindParentScene();

			if ( pScene != TNULL && pScene->IsA( &TGetClass( ABlobMainMenu ) ) )
				bMainMenu = TTRUE;
		}

		if ( bMainMenu )
		{
			pSequences    = s_aMainMenuSequences;
			iNumSequences = 4;
		}
		else
		{
			pSequences    = s_aMenuSequences;
			iNumSequences = 1;
		}
	}

	for ( TINT iSequence = 0; iSequence < iNumSequences; iSequence++ )
	{
		const ACheatSequence& rSequence = pSequences[ iSequence ];
		TINT                  iDoodad   = rSequence.m_iLength;
		TINT                  iQueue    = a_iCount;

		for ( ;; )
		{
			iQueue--;

			if ( iQueue == a_iCount || ( iDoodad--, iDoodad < 0 ) )
			{
				// The whole sequence matches: remember the device and run the cheat
				AInputManager2::AInputDeviceHandle& rHandle = AInputManager2::GetSingleton()->GetControllerHandle( a_iController );
				m_pCheatDevice                              = rHandle.m_pDevice;
				m_iCheatDeviceId                            = rHandle.m_iId;

				if ( rSequence.m_pfnCallback != TNULL )
					rSequence.m_pfnCallback();

				ClearInputQueue();
				m_pCheatDevice   = TNULL;
				m_iCheatDeviceId = 0;
				return;
			}

			if ( iQueue < 0 )
				iQueue = QUEUE_SIZE - 1;

			TINT iIndex = rSequence.m_aiDoodads[ iDoodad ];

			if ( a_rQueue[ iQueue ] != m_aDoodads[ iIndex ] && a_rQueue[ iQueue ] != m_aDoodads[ iIndex + 8 ] )
				break;
		}
	}
}

// $deBlob: FUNCTION 005a1fc0
void ACheatManager::ClearInputQueue()
{
	TUtil::MemSet( &m_aQueue0, 0, sizeof( m_aQueue0 ) );
	TUtil::MemSet( &m_aQueue1, 0, sizeof( m_aQueue1 ) );
	m_iQueue0Index = 0;
	m_iQueue1Index = 0;
}

// $deBlob: FUNCTION 005a1ff0
void ACheatManager::ClearCheatsEnabled()
{
	for ( TINT i = 0; i < CHEATS_NUMOF; i++ )
	{
		m_aCheats[ i ].m_bEnabled = TFALSE;
	}
}

// $deBlob: FUNCTION 005a2050
void ACheatManager::ShowMessage( TFLOAT a_fTime )
{
	m_pMessageBox->SetVisible( TTRUE );
	m_fMessageTime = a_fTime;
}

// Copies the locale string a_iString into the message of the manager and shows it for a_fTime seconds (a macro: the
// string id is evaluated inside the copy)
#define CHEAT_MESSAGE( a_iString, a_fTime )                                                                                  	{                                                                                                                        		ACheatManager* pCheatManager = ACheatManager::GetSingleton();                                                        		T2String16::CopySafe( pCheatManager->m_wszMessage, T2Locale::GetSingleton()->GetString( a_iString ), 256 );          		pCheatManager->ShowMessage( a_fTime );                                                                               	}

// Toggles the controls lock of the blobs (message 239/240)
// $deBlob: FUNCTION 005a2080
void ACheatManager::Cheat_ToggleControlsLock()
{
	if ( AGameStateController::GetSingleton()->GetCurrentGameState()->GetClass() == &TGetClass( AInGameState ) )
	{
		AGameStatus* pStatus = AGameStatus::GetSingleton();

		if ( !pStatus->IsMultiplayer() && pStatus->GetLevelProperties()->m_sprintLevel == 0 )
		{
			ABlobEntity::ms_bControlsLocked = !ABlobEntity::ms_bControlsLocked;
			CHEAT_MESSAGE( ABlobEntity::ms_bControlsLocked ? 239 : 240, 1.0f );
		}
	}
}

// Adds ten minutes to the level's time (up to 5401 seconds; not in the free paint mode; message 241)
// $deBlob: FUNCTION 005a2110
void ACheatManager::Cheat_AddTime()
{
	AGameStatus* pStatus = AGameStatus::GetSingleton();

	if ( pStatus != TNULL )
	{
		if ( AGameStateController::GetSingleton()->GetCurrentGameState()->GetClass() == &TGetClass( AInGameState ) && !pStatus->IsMultiplayer() )
		{
			TFLOAT fTimeLeft = pStatus->GetTimeLeft();

			if ( fTimeLeft > FLT_MIN && pStatus->GetLevelMode() != AGameStatus::Mode_FreePaint && pStatus->GetLevelProperties()->m_sprintLevel == 0 )
			{
				TFLOAT fNewTime = fTimeLeft + 600.0f;
				TMath::Clip( fNewTime, 0.0f, 5401.0f );
				pStatus->SetTimeLeft( fNewTime );

				CHEAT_MESSAGE( 241, 0.5f );
			}
		}
	}
}

// A sound at the current blob, message 248
// $deBlob: FUNCTION 005a21e0
void ACheatManager::Cheat_AddLife()
{
	if ( AGameStateController::GetSingleton()->GetCurrentGameState()->GetClass() == &TGetClass( AInGameState ) )
	{
		AGameStatus* pStatus = AGameStatus::GetSingleton();

		if ( !pStatus->IsMultiplayer() && pStatus->GetLevelProperties()->m_sprintLevel == 0 )
		{
			ABaseEntity* pBlob = APlayerManager::GetSingleton()->GetCurrentPlayerBlob();

			if ( pBlob != TNULL )
			{
				ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 39 ), TSTATICCAST( ABlobEntity, pBlob )->GetBodyPosition(), 0 );

				APlayerManager::GetSingleton()->GetCurrentBlobData()->AddLife();

				CHEAT_MESSAGE( 248, 0.5f );
			}
		}
	}
}

// Sets the status' flag at 0x7AC, message 242
// $deBlob: FUNCTION 005a2290
void ACheatManager::Cheat_SkipGoals()
{
	if ( AGameStateController::GetSingleton()->GetCurrentGameState()->GetClass() == &TGetClass( AInGameState ) && !AGameStatus::GetSingleton()->IsMultiplayer() )
	{
		if ( AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel == 0 )
		{
			AGameStatus::GetSingleton()->GetLevelData().m_bGoalCheat = TTRUE;
			CHEAT_MESSAGE( 242, 1.0f );
		}
	}
}

// Main menu cheat, message 244
// $deBlob: FUNCTION 005a2310
void ACheatManager::Cheat_UnlockAll()
{
	if ( AGameStateController::GetSingleton()->GetCurrentGameState()->GetClass() == &TGetClass( AXUIState ) )
	{
		APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();

		if ( pProfile != TNULL )
		{
			pProfile->UnlockAllWorlds();
			pProfile->UnlockWorldAndModes( 0 );
			pProfile->UnlockWorldAndModes( 1 );
			pProfile->UnlockWorldAndModes( 2 );
			pProfile->UnlockWorldAndModes( 3 );
			pProfile->UnlockWorldAndModes( 4 );
			pProfile->UnlockWorldAndModes( 5 );
			pProfile->UnlockWorldAndModes( 6 );
			pProfile->UnlockWorldAndModes( 8 );
			pProfile->UnlockWorldAndModes( 7 );
			pProfile->UnlockWorldAndModes( 9 );
			SaveLoad::GetInstance()->SetCaller( TXUI::GetSingleton()->GetActiveScene() );
			SaveLoad::GetInstance()->SaveProfile( 0, 0 );
			SaveLoad::GetInstance()->Update( 0.0f );
			CHEAT_MESSAGE( 244, 1.0f );
		}
	}
}

// Main menu cheat, message 246
// $deBlob: FUNCTION 005a2420
void ACheatManager::Cheat_UnlockBlobParty()
{
	if ( AGameStateController::GetSingleton()->GetCurrentGameState()->GetClass() == &TGetClass( AXUIState ) )
	{
		APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();

		if ( pProfile != TNULL )
		{
			pProfile->UnlockAllBlobPartyItems();
			SaveLoad::GetInstance()->SetCaller( TXUI::GetSingleton()->GetActiveScene() );
			SaveLoad::GetInstance()->SaveProfile( 0, 0 );
			SaveLoad::GetInstance()->Update( 0.0f );
			CHEAT_MESSAGE( 246, 1.0f );
		}
	}
}

// Main menu cheat, message 247
// $deBlob: FUNCTION 005a24d0
void ACheatManager::Cheat_UnlockExtras()
{
	if ( AGameStateController::GetSingleton()->GetCurrentGameState()->GetClass() == &TGetClass( AXUIState ) )
	{
		APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();

		if ( pProfile != TNULL )
		{
			pProfile->UnlockAllMovies();
			pProfile->UnlockAllGalleryItems();
			pProfile->UnlockAllVideos();
			SaveLoad::GetInstance()->SetCaller( TXUI::GetSingleton()->GetActiveScene() );
			SaveLoad::GetInstance()->SaveProfile( 0, 0 );
			SaveLoad::GetInstance()->Update( 0.0f );
			CHEAT_MESSAGE( 247, 1.0f );
		}
	}
}

// Main menu cheat, message 282
// $deBlob: FUNCTION 005a2590
void ACheatManager::Cheat_UnlockTracks()
{
	if ( AGameStateController::GetSingleton()->GetCurrentGameState()->GetClass() == &TGetClass( AXUIState ) )
	{
		APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();

		if ( pProfile != TNULL )
		{
			pProfile->UnlockAllTracks();
			SaveLoad::GetInstance()->SetCaller( TXUI::GetSingleton()->GetActiveScene() );
			SaveLoad::GetInstance()->SaveProfile( 0, 0 );
			SaveLoad::GetInstance()->Update( 0.0f );
			CHEAT_MESSAGE( 282, 0.5f );
		}
	}
}

// Not referenced by the tables
// $deBlob: FUNCTION 005a2660
void ACheatManager::Cheat_ToggleSafeZone()
{
	if ( AGUIManager::GetSingleton() != TNULL )
		AGUIManager::ToggleSafeZone( TNULL );
}

// Not referenced by the tables
// $deBlob: FUNCTION 005a2680
void ACheatManager::Cheat_AddScore()
{
	AGameStatus::GetSingleton()->GetLevelData().AddScore( 0, 10000, -1 );
	GetSingleton()->m_bScoreJumpPending = TTRUE;
}

// Not referenced by the tables; shows the position of the next civilian block
// $deBlob: FUNCTION 005a26b0
void ACheatManager::Cheat_ShowCivBlockPosition()
{
	if ( ABuildingManager::GetSingleton() == TNULL )
		return;

	ACheatManager* pManager = ACheatManager::GetSingleton();
	TVector4       vPosition;

	// NOTE: retail prints the Y of the position as "Z"
	if ( ABuildingManager::GetSingleton()->GetNearestCivBlockPosition( vPosition ) )
		T2String16::Format( pManager->m_wszMessage, 256, L"X: %0.0f, Z: %0.0f", vPosition.x, vPosition.y );
	else
		T2String16::Format( pManager->m_wszMessage, 256, L"All civblks done" );

	pManager->ShowMessage( 1.0f );
}

// DOES_NOTHING
void ACheatManager::DummyCheat()
{
}
