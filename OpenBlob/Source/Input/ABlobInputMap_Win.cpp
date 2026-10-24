#include "pch.h"
#include "ABlobInputMap.h"

#include "GameInterface/APlayerManager.h"

#include <XUI/TXUI.h>

#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceMouse.h>
#include <Input/TInputDeviceController.h>
#include <Platform/Windows/Input/TInputDeviceKeyboard_Win.h>
#include <Platform/Windows/Input/TInputDeviceMouse_Win.h>
#include <Platform/Windows/Input/TInputDeviceController_XInput.h>
#include <Platform/Windows/Input/TInputDeviceController_Win.h>
#include <Platform/Windows/Input/TInputDeviceController_Wiin.h>
#include <Render/TRender.h>
#include "Cameras/ABlobFollowCamera.h"
#include <File/TFile.h>
#include <File/TIniFile.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ABlobInputMap*         ABlobInputMap::ms_pSingleton;
ABlobInputMap::Binding ABlobInputMap::ms_aKeyboardBindings[ NUM_BINDINGS ];
ABlobInputMap::Binding ABlobInputMap::ms_aXbox360Bindings[ NUM_BINDINGS ];
ABlobInputMap::Binding ABlobInputMap::ms_aPS4Bindings[ NUM_BINDINGS ];
ABlobInputMap::Binding ABlobInputMap::ms_aWiiBindings[ NUM_BINDINGS ];
ABlobInputMap::Binding ABlobInputMap::ms_aWiiExtensionBindings[ NUM_BINDINGS ];

static void ABlobInputMap_SetBinding( ABlobInputMap::Binding* a_pBindings, TINT a_iIndex, const TCHAR* a_szName, TINT a_iButton )
{
	a_pBindings[ a_iIndex ].m_szName  = a_szName;
	a_pBindings[ a_iIndex ].m_iButton = a_iButton;
}

// The default bindings
struct ABlobInputMap_DefaultBindings
{
	// $deBlob: FUNCTION 005d90d0
	ABlobInputMap_DefaultBindings()
	{
		// Keyboard
		ABlobInputMap::Binding* pBindings = ABlobInputMap::ms_aKeyboardBindings;
		TUtil::MemSet( pBindings, 0, sizeof( ABlobInputMap::ms_aKeyboardBindings ) );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_NAVRING, "NavRing", TInputDeviceKeyboard::KEY_TAB );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP, "Jump", TInputDeviceKeyboard::KEY_SPACE );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_INTERACT, "Interact", TInputDeviceKeyboard::KEY_F );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_BRAKE, "Brake", TInputDeviceKeyboard::KEY_RCONTROL );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_BLOBVISION, "BlobVision", TInputDeviceKeyboard::KEY_RCONTROL );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CENTERCAMERA, "CenterCamera", TInputDeviceKeyboard::KEY_R );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_ZLOCK, "ZLock", TInputDeviceKeyboard::KEY_LCONTROL );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_SKIP, "Skip", TInputDeviceKeyboard::KEY_V );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CAMLEFT, "CamLeft", TInputDeviceKeyboard::KEY_Q );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CAMRIGHT, "CamRight", TInputDeviceKeyboard::KEY_E );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CAMUP, "CamUp", TInputDeviceKeyboard::KEY_Z );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CAMDOWN, "CamDown", TInputDeviceKeyboard::KEY_C );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP2, "Jump", TInputDeviceKeyboard::KEY_SPACE );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP3, "Jump", TInputDeviceKeyboard::KEY_SPACE );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_LEFT, "Left", TInputDeviceKeyboard::KEY_A );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_RIGHT, "Right", TInputDeviceKeyboard::KEY_D );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_UP, "Up", TInputDeviceKeyboard::KEY_W );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_DOWN, "Down", TInputDeviceKeyboard::KEY_S );

		// Xbox 360 (XInput button bits)
		pBindings = ABlobInputMap::ms_aXbox360Bindings;
		TUtil::MemSet( pBindings, 0, sizeof( ABlobInputMap::ms_aXbox360Bindings ) );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_NAVRING, "ButtonNavRing", 3 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP, "ButtonJump", 0 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_INTERACT, "ButtonInteract", 2 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_BRAKE, "ButtonBrake", 11 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_BLOBVISION, "ButtonBlobVision", 4 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CENTERCAMERA, "ButtonCenterCamera", 9 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_ZLOCK, "ButtonZLock", 10 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_SKIP, "ButtonSkip", 6 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP2, "ButtonJump", 0 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP3, "ButtonJump", 0 );

		// PS4 (DirectInput button bits)
		pBindings = ABlobInputMap::ms_aPS4Bindings;
		TUtil::MemSet( pBindings, 0, sizeof( ABlobInputMap::ms_aPS4Bindings ) );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_NAVRING, "ButtonNavRing", 3 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP, "ButtonJump", 1 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_INTERACT, "ButtonInteract", 0 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_BRAKE, "ButtonBrake", 2 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_BLOBVISION, "ButtonBlobVision", 4 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CENTERCAMERA, "ButtonCenterCamera", 5 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_ZLOCK, "ButtonZLock", 6 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_SKIP, "ButtonSkip", 5 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP2, "ButtonJump", 1 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP3, "ButtonJump", 1 );

		// Wii remote
		pBindings = ABlobInputMap::ms_aWiiBindings;
		TUtil::MemSet( pBindings, 0, sizeof( ABlobInputMap::ms_aWiiBindings ) );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_NAVRING, "Default", 0 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_INTERACT, "Default", 0 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_BRAKE, "Default", 1 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_BLOBVISION, "Default", 8 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CENTERCAMERA, "Default", 12 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_ZLOCK, "Default", 13 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_SKIP, "Default", 3 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CAMLEFT, "Default", 9 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CAMRIGHT, "Default", 10 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CAMUP, "Default", 7 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CAMDOWN, "Default", 8 );

		pBindings = ABlobInputMap::ms_aWiiExtensionBindings;
		TUtil::MemSet( pBindings, 0, sizeof( ABlobInputMap::ms_aWiiExtensionBindings ) );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_NAVRING, "Default", 2 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP, "Default", 1 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_INTERACT, "Default", 3 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_BRAKE, "Default", 0 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_BLOBVISION, "Default", 0 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_CENTERCAMERA, "Default", 7 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_ZLOCK, "Default", 8 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_SKIP, "Default", 5 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP2, "Default", 1 );
		ABlobInputMap_SetBinding( pBindings, ABlobInputMap::BINDING_JUMP3, "Default", 1 );
	}
};

// $deBlob: FUNCTION 0053c950
static ABlobInputMap_DefaultBindings s_oDefaultBindings;

// NOTE: retail's TXUIScene reads the tables directly, and their initializer runs before the one of TXUIScene's glyphs;
// the engine can't know the game's tables, so the game installs them and rebuilds the glyphs (the static initializers
// of two libraries run in no defined order)
void ABlobInputMap::InstallXUIBindings()
{
	TXUIScene::SetGameHooks( ms_aKeyboardBindings, ms_aXbox360Bindings, ms_aPS4Bindings );
	TXUIScene::InitPlatformGlyphs();
}

// $deBlob: FUNCTION 005d7700
ABlobInputSwitch::ABlobInputSwitch( TFLOAT a_fOnThreshold, TFLOAT a_fOffThreshold, TINT a_iNumSamples, TFLOAT a_fMinTime, TFLOAT a_fInterval )
{
	m_fOnThreshold  = a_fOnThreshold;
	m_fOffThreshold = a_fOffThreshold;
	m_fMinTime      = a_fMinTime;
	m_fInterval     = a_fInterval;
	m_iState        = 0;
	m_fStateTime    = 0.0f;
	m_uiHistory     = 0;
	m_fTimer        = 0.0f;
	m_iNumSamples   = a_iNumSamples;
	m_pSamples      = TNULL;
	m_fSampleSum    = 0.0f;
	m_iSampleIndex  = -1;

	if ( a_iNumSamples > 0 )
	{
		m_pSamples = new TFLOAT[ a_iNumSamples ];

		for ( TINT i = 0; i < m_iNumSamples; i++ )
			m_pSamples[ i ] = 0.0f;
	}
}

// $deBlob: FUNCTION 005d77b0
ABlobInputSwitch::~ABlobInputSwitch()
{
	if ( m_pSamples != TNULL )
	{
		delete[] m_pSamples;
		m_pSamples = TNULL;
	}
}

// $deBlob: FUNCTION 005d77d0
TFLOAT ABlobInputSwitch::Update( TFLOAT a_fInput, TFLOAT a_fDeltaTime )
{
	m_fTimer += a_fDeltaTime;
	m_fStateTime += a_fDeltaTime;

	TFLOAT fValue = a_fInput;

	if ( m_pSamples != TNULL )
	{
		m_iSampleIndex = ( m_iSampleIndex + 1 ) % m_iNumSamples;
		m_fSampleSum -= m_pSamples[ m_iSampleIndex ];
		m_pSamples[ m_iSampleIndex ] = a_fInput;
		m_fSampleSum                 = a_fInput + m_fSampleSum;
		fValue                       = m_fSampleSum / TFLOAT( m_iNumSamples );
	}

	if ( m_fInterval <= m_fTimer )
	{
		if ( TINT( m_uiHistory & 1 ) != m_iState && m_fStateTime > m_fMinTime )
		{
			m_fTimer    = 0.0f;
			m_uiHistory = m_iState | ( m_uiHistory << 1 );
		}

		if ( m_uiHistory & 1 )
		{
			if ( m_iState != 0 && m_fOffThreshold > fValue )
			{
				m_iState     = 0;
				m_fStateTime = 0.0f;
			}
		}
		else if ( m_iState != 1 && fValue > m_fOnThreshold )
		{
			m_iState     = 1;
			m_fStateTime = 0.0f;
		}
	}

	return fValue;
}

// $deBlob: FUNCTION 005d78c0
void ABlobInputSwitch::Reset()
{
	if ( m_pSamples != TNULL )
	{
		for ( TINT i = 0; i < m_iNumSamples; i++ )
			m_pSamples[ i ] = 0.0f;
	}

	m_iState       = 0;
	m_uiHistory    = 0;
	m_fStateTime   = 0.0f;
	m_fTimer       = 0.0f;
	m_fSampleSum   = 0.0f;
	m_iSampleIndex = -1;
}

// $deBlob: FUNCTION 005d7910
ABlobInputSwitch3::ABlobInputSwitch3( TFLOAT a_fOnThreshold, TFLOAT a_fOffThreshold, TFLOAT a_fLowThreshold, TFLOAT a_fLowOffThreshold, TINT a_iNumSamples, TFLOAT a_fMinTime, TFLOAT a_fInterval )
{
	m_fOnThreshold     = a_fOnThreshold;
	m_fOffThreshold    = a_fOffThreshold;
	m_fLowThreshold    = a_fLowThreshold;
	m_fLowOffThreshold = a_fLowOffThreshold;
	m_fMinTime         = a_fMinTime;
	m_fInterval        = a_fInterval;
	m_iState           = 0;
	m_fStateTime       = 0.0f;
	m_uiHistory        = 0;
	m_fTimer           = 0.0f;
	m_iNumSamples      = a_iNumSamples;
	m_pSamples         = TNULL;
	m_fSampleSum       = 0.0f;
	m_iSampleIndex     = -1;

	if ( a_iNumSamples > 0 )
	{
		m_pSamples = new TFLOAT[ a_iNumSamples ];

		for ( TINT i = 0; i < m_iNumSamples; i++ )
			m_pSamples[ i ] = 0.0f;
	}
}

// $deBlob: FUNCTION 005d79d0
ABlobInputSwitch3::~ABlobInputSwitch3()
{
	if ( m_pSamples != TNULL )
	{
		delete[] m_pSamples;
		m_pSamples = TNULL;
	}
}

// $deBlob: FUNCTION 005d79f0
TFLOAT ABlobInputSwitch3::Update( TFLOAT a_fInput, TFLOAT a_fDeltaTime )
{
	m_fTimer += a_fDeltaTime;
	m_fStateTime += a_fDeltaTime;

	TFLOAT fValue = a_fInput;

	if ( m_pSamples != TNULL )
	{
		m_iSampleIndex = ( m_iSampleIndex + 1 ) % m_iNumSamples;
		m_fSampleSum -= m_pSamples[ m_iSampleIndex ];
		m_pSamples[ m_iSampleIndex ] = a_fInput;
		m_fSampleSum                 = a_fInput + m_fSampleSum;
		fValue                       = m_fSampleSum / TFLOAT( m_iNumSamples );
	}

	if ( m_fInterval > m_fTimer )
		return fValue;

	if ( TINT( m_uiHistory & 3 ) != m_iState && m_fStateTime > m_fMinTime )
	{
		m_fTimer    = 0.0f;
		m_uiHistory = m_iState | ( m_uiHistory << 2 );
	}

	switch ( m_uiHistory & 3 )
	{
		case 0:
			if ( m_iState != 1 && fValue > m_fOnThreshold )
			{
				m_iState     = 1;
				m_fStateTime = 0.0f;
			}
			else if ( m_iState != 3 && !( m_fLowThreshold <= fValue ) )
			{
				m_iState     = 3;
				m_fStateTime = 0.0f;
			}
			break;
		case 1:
			if ( m_iState != 3 && m_fLowThreshold > fValue )
			{
				m_iState     = 3;
				m_fStateTime = 0.0f;
			}
			else if ( m_iState != 0 && m_fOffThreshold > fValue )
			{
				m_iState     = 0;
				m_fStateTime = 0.0f;
			}
			break;
		case 3:
			if ( m_iState != 1 && fValue > m_fOnThreshold )
			{
				m_iState     = 1;
				m_fStateTime = 0.0f;
			}
			else if ( m_iState != 0 && fValue > m_fLowOffThreshold )
			{
				m_iState     = 0;
				m_fStateTime = 0.0f;
			}
			break;
	}

	return fValue;
}

// $deBlob: FUNCTION 005d7b50
void ABlobInputSwitch3::Reset()
{
	if ( m_pSamples != TNULL )
	{
		for ( TINT i = 0; i < m_iNumSamples; i++ )
			m_pSamples[ i ] = 0.0f;
	}

	m_iState       = 0;
	m_uiHistory    = 0;
	m_fStateTime   = 0.0f;
	m_fTimer       = 0.0f;
	m_fSampleSum   = 0.0f;
	m_iSampleIndex = -1;
}

// $deBlob: FUNCTION 005d7ba0
ABlobInputMap::ABlobInputMap()
{
	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		m_aPlayers[ i ].m_vMove         = TVector2::VEC_ZERO;
		m_aPlayers[ i ].m_vCamera       = TVector2::VEC_ZERO;
		m_aPlayers[ i ].m_vCursor       = TVector2::VEC_ZERO;
		m_aPlayers[ i ].m_uiButtons     = 0;
		m_aPlayers[ i ].m_uiPrevButtons = 0;
		m_aPlayers[ i ].m_bCursorActive = TFALSE;
	}

	m_bDisabled     = TFALSE;
	m_fDisabledTime = -1.0f;
	LoadBindings();
	ms_pSingleton = this;
}

// $deBlob: FUNCTION 005d7ce0
// $deBlob: FUNCTION 005c2480
ABlobInputMap::~ABlobInputMap()
{
	Deinitialise();
	ms_pSingleton = TNULL;
}

// Microseconds since the first call (QueryPerformanceCounter)
// $deBlob: FUNCTION 006faf40
static TUINT64 ABlobInputMap_GetMicroseconds()
{
	static LARGE_INTEGER s_oFrequency;
	static LARGE_INTEGER s_oStart;
	static TBOOL         s_bInitialised;

	if ( !s_bInitialised )
	{
		QueryPerformanceFrequency( &s_oFrequency );
		QueryPerformanceCounter( &s_oStart );
		s_bInitialised = TTRUE;
	}

	LARGE_INTEGER oNow;
	QueryPerformanceCounter( &oNow );

	return TUINT64( TDOUBLE( oNow.QuadPart - s_oStart.QuadPart ) / TDOUBLE( s_oFrequency.QuadPart ) * 1000000.0 );
}

static TINT   s_iLastCursorX;
static TINT   s_iLastCursorY;
static TFLOAT s_fLastCursorMoveTime;

#ifdef DEBLOB_COMMUNITY_PATCH
// The camera buttons of the mouse follow its motion (the DirectInput deltas), not the cursor's position: a motion of more
// than CURSOR_CAMERA_THRESHOLD units in one direction of an axis turns the camera that way for CURSOR_CAMERA_TIME ms;
// motion the other way stops it at once
static const TINT   CURSOR_CAMERA_THRESHOLD = 2;
static const TFLOAT CURSOR_CAMERA_TIME      = 100.0f;

// XInput's recommended dead zone of the right stick (XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE), for the camera axes
static const TFLOAT CAMERA_STICK_DEADZONE = 8689.0f / 32767.0f;

static TINT   s_aiCursorMotionDir[ 2 ];
static TINT   s_aiCursorMotionSum[ 2 ];
static TFLOAT s_afCursorMotionTime[ 2 ]    = { -1.0e9f, -1.0e9f };
static TFLOAT s_afCursorLastDeltaTime[ 2 ] = { -1.0e9f, -1.0e9f };

// The free look's cursor: the motion since free look (blob vision) started, -1..1 like retail's offset from the centre
static TFLOAT s_afFreeLookCursor[ 2 ];

static const TUINT32 CAMERA_BUTTONS_X = ABlobInputMap::BUTTON_CAMLEFT | ABlobInputMap::BUTTON_CAMRIGHT;
static const TUINT32 CAMERA_BUTTONS_Y = ABlobInputMap::BUTTON_CAMUP | ABlobInputMap::BUTTON_CAMDOWN;

// The direction (-1, 0, 1) the mouse currently moves in on an axis (0: x, 1: y)
static TINT ABlobInputMap_UpdateCursorMotion( TINT a_iAxis, TINT a_iDelta, TFLOAT a_fTime )
{
	if ( a_iDelta != 0 )
	{
		TINT iDir = ( a_iDelta > 0 ) ? 1 : -1;

		// A reversal (or a new motion after a stop) starts over: the old direction stops right away
		if ( iDir != s_aiCursorMotionDir[ a_iAxis ] || a_fTime - s_afCursorLastDeltaTime[ a_iAxis ] > CURSOR_CAMERA_TIME )
		{
			s_aiCursorMotionDir[ a_iAxis ]  = iDir;
			s_aiCursorMotionSum[ a_iAxis ]  = 0;
			s_afCursorMotionTime[ a_iAxis ] = -1.0e9f;
		}

		s_aiCursorMotionSum[ a_iAxis ] += TMath::Abs( a_iDelta );
		s_afCursorLastDeltaTime[ a_iAxis ] = a_fTime;

		if ( s_aiCursorMotionSum[ a_iAxis ] > CURSOR_CAMERA_THRESHOLD )
		{
			s_aiCursorMotionSum[ a_iAxis ]  = 0;
			s_afCursorMotionTime[ a_iAxis ] = a_fTime;
		}
	}

	return ( a_fTime - s_afCursorMotionTime[ a_iAxis ] < CURSOR_CAMERA_TIME ) ? s_aiCursorMotionDir[ a_iAxis ] : 0;
}

// Moves the free look's cursor by the mouse's motion (or resets it to the centre)
static void ABlobInputMap_UpdateFreeLookCursor( TINT a_iAxis, TBOOL a_bFreeLook, TINT a_iDelta, TFLOAT a_fHalfSize )
{
	if ( !a_bFreeLook )
	{
		s_afFreeLookCursor[ a_iAxis ] = 0.0f;
		return;
	}

	s_afFreeLookCursor[ a_iAxis ] += TFLOAT( a_iDelta ) / a_fHalfSize;
	TMath::Clip( s_afFreeLookCursor[ a_iAxis ], -1.0f, 1.0f );
}

// Adds the camera buttons of the cursor: explicit camera input (keys, sticks) of an axis overrides the cursor's, and
// explicit opposite directions cancel each other
static void ABlobInputMap_AddCursorCamera( TUINT32& a_ruiButtons, TUINT32 a_uiCursorButtons )
{
	if ( ( a_ruiButtons & CAMERA_BUTTONS_X ) == CAMERA_BUTTONS_X )
		a_ruiButtons &= ~CAMERA_BUTTONS_X;
	else if ( ( a_ruiButtons & CAMERA_BUTTONS_X ) == 0 )
		a_ruiButtons |= a_uiCursorButtons & CAMERA_BUTTONS_X;

	if ( ( a_ruiButtons & CAMERA_BUTTONS_Y ) == CAMERA_BUTTONS_Y )
		a_ruiButtons &= ~CAMERA_BUTTONS_Y;
	else if ( ( a_ruiButtons & CAMERA_BUTTONS_Y ) == 0 )
		a_ruiButtons |= a_uiCursorButtons & CAMERA_BUTTONS_Y;
}
#endif

static TBOOL ABlobInputMap_IsBitDown( TUINT32 a_uiBits, const ABlobInputMap::Binding& a_rBinding )
{
	return ( a_uiBits & ( 1 << a_rBinding.m_iButton ) ) != 0;
}

// The buttons and sticks of an XInput or a DirectInput (PS4) pad
static void ABlobInputMap_ReadPad( ABlobInputMap::PlayerInput& a_rInput, const ABlobInputMap::Binding* a_pBindings, TUINT32 a_uiButtons, TUINT32 a_uiOldButtons, TFLOAT a_fMoveX, TFLOAT a_fMoveY, TFLOAT a_fCameraX, TFLOAT a_fCameraY, TBOOL a_bAddMove )
{
	if ( a_bAddMove )
	{
		a_rInput.m_vMove.x += a_fMoveX;
		a_rInput.m_vMove.y -= a_fMoveY;
	}
	else
	{
		a_rInput.m_vMove.x = a_fMoveX;
		a_rInput.m_vMove.y = -a_fMoveY;
	}

	TUINT32  uiPressed = a_uiButtons & ( a_uiButtons ^ a_uiOldButtons );
	TUINT32& rButtons  = a_rInput.m_uiButtons;

	if ( ABlobInputMap_IsBitDown( a_uiButtons, a_pBindings[ ABlobInputMap::BINDING_BRAKE ] ) ) rButtons |= ABlobInputMap::BUTTON_BRAKE;
	if ( ABlobInputMap_IsBitDown( a_uiButtons, a_pBindings[ ABlobInputMap::BINDING_BLOBVISION ] ) ) rButtons |= ABlobInputMap::BUTTON_BLOBVISION;
	if ( ABlobInputMap_IsBitDown( a_uiButtons, a_pBindings[ ABlobInputMap::BINDING_BLOBVISION ] ) ) rButtons |= ABlobInputMap::BUTTON_BRAKE;
	if ( ABlobInputMap_IsBitDown( a_uiButtons, a_pBindings[ 3 ] ) ) rButtons |= ABlobInputMap::BUTTON_UNK8;
	if ( ABlobInputMap_IsBitDown( a_uiButtons, a_pBindings[ ABlobInputMap::BINDING_ZLOCK ] ) ) rButtons |= ABlobInputMap::BUTTON_ZLOCK;
	if ( ABlobInputMap_IsBitDown( a_uiButtons, a_pBindings[ ABlobInputMap::BINDING_SKIP ] ) ) rButtons |= ABlobInputMap::BUTTON_SKIP;
	if ( ABlobInputMap_IsBitDown( a_uiButtons, a_pBindings[ ABlobInputMap::BINDING_INTERACT ] ) ) rButtons |= ABlobInputMap::BUTTON_INTERACT;

	// The camera stick
#ifdef DEBLOB_COMMUNITY_PATCH
	// Retail's dead zone (7500, ~0.229) is barely above the 0.2 threshold, so a slightly drifting stick turned the camera
	if ( TMath::Abs( a_fCameraX ) < CAMERA_STICK_DEADZONE ) a_fCameraX = 0.0f;
	if ( TMath::Abs( a_fCameraY ) < CAMERA_STICK_DEADZONE ) a_fCameraY = 0.0f;
#endif

	if ( a_fCameraX > 0.2f )
		rButtons |= ABlobInputMap::BUTTON_CAMLEFT;
	else if ( a_fCameraX < -0.2f )
		rButtons |= ABlobInputMap::BUTTON_CAMRIGHT;

	if ( a_fCameraY > 0.2f )
		rButtons |= ABlobInputMap::BUTTON_CAMUP;
	else if ( a_fCameraY < -0.2f )
		rButtons |= ABlobInputMap::BUTTON_CAMDOWN;

	// Pressed this frame
	if ( ABlobInputMap_IsBitDown( uiPressed, a_pBindings[ ABlobInputMap::BINDING_JUMP2 ] ) ) rButtons |= ABlobInputMap::BUTTON_JUMP2;
	if ( ABlobInputMap_IsBitDown( uiPressed, a_pBindings[ ABlobInputMap::BINDING_JUMP3 ] ) ) rButtons |= ABlobInputMap::BUTTON_JUMP3;
	if ( ABlobInputMap_IsBitDown( a_uiButtons, a_pBindings[ ABlobInputMap::BINDING_NAVRING ] ) ) rButtons |= ABlobInputMap::BUTTON_NAVRING;
	if ( ABlobInputMap_IsBitDown( uiPressed, a_pBindings[ ABlobInputMap::BINDING_JUMP ] ) ) rButtons |= ABlobInputMap::BUTTON_JUMP;
	if ( ABlobInputMap_IsBitDown( a_uiButtons, a_pBindings[ ABlobInputMap::BINDING_NAVRING ] ) ) rButtons |= ABlobInputMap::BUTTON_NAVRING;
	if ( ABlobInputMap_IsBitDown( a_uiButtons, a_pBindings[ ABlobInputMap::BINDING_CENTERCAMERA ] ) ) rButtons |= ABlobInputMap::BUTTON_CENTERCAMERA;
}

// $deBlob: FUNCTION 005d7eb0
void ABlobInputMap::Update( TFLOAT a_fDeltaTime )
{
	TXUI* pXUI = TXUI::GetSingleton();

	if ( pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked() )
		return;

	// The player of the keyboard (and mouse): the first one, or in multiplayer the one the keyboard is assigned to
	TINT  iKeyboardPlayer = 0;
	TBOOL bMultiplayer    = APlayerManager::GetSingleton()->GetNumPlayers() > 1;

	if ( bMultiplayer )
	{
		TInputDevice* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );
		iKeyboardPlayer         = pKeyboard->GetPlayer();
	}

	TInputDeviceMouse* pMouse = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceMouse>( 0 );

#ifdef DEBLOB_COMMUNITY_PATCH
	// The camera buttons of the mouse's motion
	TUINT32 uiMouseCameraButtons = 0;
#endif

	if ( pMouse != TNULL && iKeyboardPlayer >= 0 )
	{
		// The cursor of the mouse, -1..1 from the window's centre; it's active for a second after it moved
		TASSERT( pMouse->IsA( &TGetClass( TInputDXDeviceMouse ) ) );
		TInputDXDeviceMouse*    pDXMouse       = TSTATICCAST( TInputDXDeviceMouse, pMouse );
		TINT                    iX             = pDXMouse->GetPositionX();
		TINT                    iY             = pDXMouse->GetPositionY();
		TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

		TFLOAT fHalfWidth                        = TFLOAT( TINT( pDisplayParams->uiWidth ) ) * 0.5f;
		TFLOAT fHalfHeight                        = TFLOAT( TINT( pDisplayParams->uiHeight ) ) * 0.5f;
		m_aPlayers[ iKeyboardPlayer ].m_vCursor.x = ( TFLOAT( iX ) - fHalfWidth ) / fHalfWidth;
		m_aPlayers[ iKeyboardPlayer ].m_vCursor.y = ( TFLOAT( iY ) - fHalfHeight ) / fHalfHeight;

		TBOOL bMoved   = iX != s_iLastCursorX || iY != s_iLastCursorY;
		s_iLastCursorX = iX;
		s_iLastCursorY = iY;

		TFLOAT fTime = TFLOAT( TINT64( ABlobInputMap_GetMicroseconds() ) ) / 1000.0f;

		if ( bMoved )
			s_fLastCursorMoveTime = fTime;

		m_aPlayers[ iKeyboardPlayer ].m_bCursorActive = fTime - s_fLastCursorMoveTime < 1000.0f;

#ifdef DEBLOB_COMMUNITY_PATCH
		// The motion of this frame (unclipped, also while the cursor rests at an edge or outside the window)
		TINT iDeltaX = 0;
		TINT iDeltaY = 0;

		if ( pMouse->IsAcquired() )
		{
			iDeltaX = pMouse->GetAxisInt( TInputDeviceMouse::AXIS_CURSOR, 0 );
			iDeltaY = pMouse->GetAxisInt( TInputDeviceMouse::AXIS_CURSOR, 1 );
		}

		if ( iDeltaX != 0 || iDeltaY != 0 )
		{
			s_fLastCursorMoveTime                         = fTime;
			m_aPlayers[ iKeyboardPlayer ].m_bCursorActive = TTRUE;
		}


		// The buttons of retail's offsets from the centre: right CamRight, left CamLeft, up CamDown, down CamUp
		TINT iDirX = ABlobInputMap_UpdateCursorMotion( 0, iDeltaX, fTime );
		TINT iDirY = ABlobInputMap_UpdateCursorMotion( 1, iDeltaY, fTime );

		if ( iDirX > 0 ) uiMouseCameraButtons |= ABlobInputMap::BUTTON_CAMRIGHT;
		if ( iDirX < 0 ) uiMouseCameraButtons |= ABlobInputMap::BUTTON_CAMLEFT;
		if ( iDirY > 0 ) uiMouseCameraButtons |= ABlobInputMap::BUTTON_CAMUP;
		if ( iDirY < 0 ) uiMouseCameraButtons |= ABlobInputMap::BUTTON_CAMDOWN;

		// The free look's offset counts from where it started (blob vision held in the last frame)
		TBOOL bFreeLook = ( m_aPlayers[ iKeyboardPlayer ].m_uiButtons & ABlobInputMap::BUTTON_BLOBVISION ) != 0;
		ABlobInputMap_UpdateFreeLookCursor( 0, bFreeLook, iDeltaX, fHalfWidth );
		ABlobInputMap_UpdateFreeLookCursor( 1, bFreeLook, iDeltaY, fHalfHeight );
		m_aPlayers[ iKeyboardPlayer ].m_vCursor.x = s_afFreeLookCursor[ 0 ];
		m_aPlayers[ iKeyboardPlayer ].m_vCursor.y = s_afFreeLookCursor[ 1 ];
#endif
	}

	if ( m_bDisabled == TTRUE )
	{
		if ( m_fDisabledTime <= 0.0f )
			return;

		m_fDisabledTime -= a_fDeltaTime;

		if ( 0.0f < m_fDisabledTime )
			return;

		m_bDisabled     = TFALSE;
		m_fDisabledTime = -1.0f;
		return;
	}

	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		m_aPlayers[ i ].m_uiPrevButtons = m_aPlayers[ i ].m_uiButtons;
		m_aPlayers[ i ].m_uiButtons     = 0;
		m_aPlayers[ i ].m_vCamera       = TVector2::VEC_ZERO;
	}

	TINT iPlayer = bMultiplayer ? iKeyboardPlayer : 0;

	if ( m_bKeyboardEnabled && iPlayer >= 0 )
	{
		TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );
		PlayerInput&          rInput    = m_aPlayers[ iPlayer ];

		if ( pKeyboard == TNULL || pMouse == TNULL )
		{
			rInput.m_vMove.x       = 0.0f;
			rInput.m_vMove.y       = 0.0f;
			rInput.m_bCursorActive = TFALSE;
		}
		else
		{
			// The mouse look: the cursor outside a dead zone of 0.1 around the window's centre
			TFLOAT fCursorX = TMath::Min( TMath::Abs( rInput.m_vCursor.x ), 1.0f ) - 0.1f;
			fCursorX        = ( 0.0f <= fCursorX ) ? ( TMath::Sign( rInput.m_vCursor.x ) * fCursorX ) / 0.9f : 0.0f;

			TFLOAT fCursorY = TMath::Min( TMath::Abs( rInput.m_vCursor.y ), 1.0f ) - 0.1f;
			fCursorY        = ( 0.0f <= fCursorY ) ? ( TMath::Sign( rInput.m_vCursor.y ) * fCursorY ) / 0.9f : 0.0f;

			if ( iKeyboardPlayer == iPlayer )
			{
				if ( pMouse->IsDown( TInputDeviceMouse::BUTTON_1 ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_JUMP;
				if ( pMouse->IsDown( TInputDeviceMouse::BUTTON_2 ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_ZLOCK;
				if ( pMouse->IsDown( TInputDeviceMouse::BUTTON_3 ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_CENTERCAMERA;
			}

			// The actions 0-16 set 1 << index
			TUINT32 uiBit = 1;

			for ( TINT i = 0; i < 17; i++ )
			{
				if ( pKeyboard->IsDown( ms_aKeyboardBindings[ i ].m_iButton ) )
					rInput.m_uiButtons |= uiBit;

				uiBit = ( uiBit << 1 ) | ( uiBit >> 31 );
			}

			if ( pKeyboard->IsDown( ms_aKeyboardBindings[ BINDING_BLOBVISION ].m_iButton ) )
				rInput.m_uiButtons |= BUTTON_BRAKE;

			if ( rInput.m_uiButtons & BUTTON_BRAKE )
			{
				rInput.m_vCamera.x += fCursorX;
				rInput.m_vCamera.y += fCursorY;
			}

			// The movement keys ramp the stick up at 2 / s (and reset it when the direction changes)
			if ( pKeyboard->IsDown( ms_aKeyboardBindings[ BINDING_UP ].m_iButton ) )
			{
				rInput.m_vMove.y = TMath::Max( rInput.m_vMove.y, 0.0f ) + a_fDeltaTime * 2.0f;
				TMath::Clip( rInput.m_vMove.y, -1.0f, 1.0f );
			}
			else if ( pKeyboard->IsDown( ms_aKeyboardBindings[ BINDING_DOWN ].m_iButton ) )
			{
				rInput.m_vMove.y = TMath::Min( rInput.m_vMove.y, 0.0f ) - a_fDeltaTime * 2.0f;
				TMath::Clip( rInput.m_vMove.y, -1.0f, 1.0f );
			}
			else
			{
				rInput.m_vMove.y = 0.0f;
			}

			if ( pKeyboard->IsDown( ms_aKeyboardBindings[ BINDING_LEFT ].m_iButton ) )
			{
				rInput.m_vMove.x = TMath::Min( rInput.m_vMove.x, 0.0f ) - a_fDeltaTime * 2.0f;
				TMath::Clip( rInput.m_vMove.x, -1.0f, 1.0f );
			}
			else if ( pKeyboard->IsDown( ms_aKeyboardBindings[ BINDING_RIGHT ].m_iButton ) )
			{
				rInput.m_vMove.x = TMath::Max( rInput.m_vMove.x, 0.0f ) + a_fDeltaTime * 2.0f;
				TMath::Clip( rInput.m_vMove.x, -1.0f, 1.0f );
			}
			else
			{
				rInput.m_vMove.x = 0.0f;
			}
		}
	}

	// The controllers (and the mouse cursor's directions)
	for ( iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++ )
	{
		PlayerInput& rInput = m_aPlayers[ iPlayer ];

#ifdef DEBLOB_COMMUNITY_PATCH
		// The mouse's directions follow its motion; they're added after the explicit camera input (keys, sticks) that
		// overrides them
		TUINT32 uiCursorButtons = 0;

		if ( iPlayer == iKeyboardPlayer && pMouse != TNULL )
		{
			uiCursorButtons = uiMouseCameraButtons;
		}
		else if ( rInput.m_bCursorActive )
		{
			if ( rInput.m_vCursor.y < -0.2f ) uiCursorButtons |= ABlobInputMap::BUTTON_CAMDOWN;
			if ( 0.2f < rInput.m_vCursor.y ) uiCursorButtons |= ABlobInputMap::BUTTON_CAMUP;
			if ( 0.2f < rInput.m_vCursor.x ) uiCursorButtons |= ABlobInputMap::BUTTON_CAMRIGHT;
			if ( rInput.m_vCursor.x < -0.2f ) uiCursorButtons |= ABlobInputMap::BUTTON_CAMLEFT;
		}
#else
		// NOTE: the hidden cursor is never recentred: while it rests off-centre, any mouse
		// movement turns the camera for a second, and as the follow camera prefers CamLeft over CamRight, the opposite key
		// doesn't stop a turn to the left
		if ( rInput.m_bCursorActive )
		{
			if ( rInput.m_vCursor.y < -0.2f ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_CAMDOWN;
			if ( 0.2f < rInput.m_vCursor.y ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_CAMUP;
			if ( 0.2f < rInput.m_vCursor.x ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_CAMRIGHT;
			if ( rInput.m_vCursor.x < -0.2f ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_CAMLEFT;
		}
#endif

		TInputDevice* pDevice = TInputDevice::GetPlayerDevice( iPlayer );

		if ( pDevice == TNULL || !pDevice->IsA( &TGetClass( TInputDeviceController ) ) || !pDevice->IsEnabled() )
		{
			if ( iPlayer != iKeyboardPlayer )
			{
				rInput.m_vMove.x       = 0.0f;
				rInput.m_vMove.y       = 0.0f;
				rInput.m_bCursorActive = TFALSE;
			}

#ifdef DEBLOB_COMMUNITY_PATCH
			ABlobInputMap_AddCursorCamera( rInput.m_uiButtons, uiCursorButtons );
#endif
			continue;
		}

		// The first player adds the stick to the keyboard's movement (when the keyboard is the first player's)
		TBOOL bAddMove = iPlayer == 0 && iKeyboardPlayer == 0;

		if ( pDevice->IsA( &TGetClass( TInputDeviceController_XInput ) ) )
		{
			TInputDeviceController_XInput* pPad = TSTATICCAST( TInputDeviceController_XInput, pDevice );
			ABlobInputMap_ReadPad( rInput, ms_aXbox360Bindings, pPad->GetButtonBits(), pPad->GetOldButtonBits(), pPad->GetAxis( 0 ), pPad->GetAxis( 1 ), pPad->GetAxis( 3 ), pPad->GetAxis( 4 ), bAddMove );
		}
		else if ( pDevice->IsA( &TGetClass( TInputDXDeviceController ) ) )
		{
			TASSERT( pDevice->GetGamepadType() == TInputDevice::Gamepad_PS4, "pInputDevice->GetGamepadType() == Gamepad_PS4" );
			TInputDXDeviceController* pPad = TSTATICCAST( TInputDXDeviceController, pDevice );
			ABlobInputMap_ReadPad( rInput, ms_aPS4Bindings, pPad->GetButtonBits(), pPad->GetOldButtonBits(), pPad->GetAxis( 0 ), pPad->GetAxis( 1 ), pPad->GetAxis( 2 ), pPad->GetAxis( 5 ), bAddMove );
		}
		else if ( pDevice->IsA( &TGetClass( TInputWiinDeviceController ) ) )
		{
			TASSERT( pDevice->GetGamepadType() == TInputDevice::Gamepad_Wii, "pInputDevice->GetGamepadType() == Gamepad_Wii" );
			TInputWiinDeviceController* pRemote = TSTATICCAST( TInputWiinDeviceController, pDevice );
			const wiimote_state&        rState  = pRemote->GetState();

			if ( rState.ExtensionType == wiimote_state::NUNCHUK || rState.ExtensionType == wiimote_state::CLASSIC )
			{
				// The Nunchuk's stick (not negated)
				if ( bAddMove )
				{
					rInput.m_vMove.x += rState.Nunchuk.Joystick.X;
					rInput.m_vMove.y += rState.Nunchuk.Joystick.Y;
				}
				else
				{
					rInput.m_vMove.x = rState.Nunchuk.Joystick.X;
					rInput.m_vMove.y = rState.Nunchuk.Joystick.Y;
				}

				// NOTE: retail reads the remote's buttons unpacked in its wiimote_state (+0x5e) and the bindings 11 and 12
				// (the Nunchuk's C and Z) from +0x152; ours are all in the remote's button states
				const Binding* pBindings = ms_aWiiBindings;
				if ( pRemote->IsButtonStateDown( pBindings[ 5 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_BRAKE;
				if ( pRemote->IsButtonStateDown( pBindings[ 3 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_UNK8;
				if ( pRemote->IsButtonStateDown( pBindings[ 16 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_SKIP;
				if ( pRemote->IsButtonStateDown( pBindings[ 2 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_INTERACT;
				if ( pRemote->IsButtonStateDown( pBindings[ 1 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_NAVRING;
				if ( pRemote->IsButtonStateDown( pBindings[ 10 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_BLOBVISION;
				if ( pRemote->IsButtonStateDown( pBindings[ 10 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_BRAKE;
				if ( pRemote->IsButtonStateDown( pBindings[ 6 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_CAMLEFT;
				if ( pRemote->IsButtonStateDown( pBindings[ 7 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_CAMRIGHT;
				if ( pRemote->IsButtonStateDown( pBindings[ 8 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_CAMUP;
				if ( pRemote->IsButtonStateDown( pBindings[ 9 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_CAMDOWN;
				if ( pRemote->IsButtonStateDown( pBindings[ 12 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_ZLOCK;
				if ( pRemote->IsButtonStateDown( pBindings[ 11 ].m_iButton ) ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_CENTERCAMERA;

				// Shaking the remote jumps
				if ( TMath::Abs( rState.Acceleration.Z ) > 1.0f ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_JUMP2;
				if ( rState.Acceleration.Z < -1.0f ) rInput.m_uiButtons |= ABlobInputMap::BUTTON_JUMP;

				// Shaking the Nunchuk
				if ( TMath::Abs( rState.Nunchuk.Acceleration.Y ) > 0.8f && TMath::Abs( rState.Nunchuk.Acceleration.X ) < 1.0f )
					rInput.m_uiButtons |= ABlobInputMap::BUTTON_JUMP3;

				// Pointing at the sensor bar turns the camera
				if ( ( rInput.m_uiButtons & 0x20 ) && ( rState.IR.Dot[ 0 ].bVisible || rState.IR.Dot[ 1 ].bVisible ) )
				{
					TFLOAT fX = ( rState.IR.Dot[ 0 ].X + 0.0f ) + rState.IR.Dot[ 1 ].X;
					TFLOAT fY = ( rState.IR.Dot[ 0 ].Y + 0.0f ) + rState.IR.Dot[ 1 ].Y;

					fX = ABlobFollowCamera::GetCamSpeedOption() * ( fX - 1.0f );
					fY = ABlobFollowCamera::GetInvertY() * ABlobFollowCamera::GetCamSpeedOption() * ( fY - 1.0f );

					rInput.m_bCursorActive = TTRUE;
					rInput.m_vCursor.x     = fX;
					rInput.m_vCamera.x     = fX;
					rInput.m_vCursor.y     = fY;
					rInput.m_vCamera.y     = fY;
				}
			}
			else
			{
				rInput.m_vMove.x       = 0.0f;
				rInput.m_vMove.y       = 0.0f;
				rInput.m_bCursorActive = TFALSE;
			}
		}

#ifdef DEBLOB_COMMUNITY_PATCH
		ABlobInputMap_AddCursorCamera( rInput.m_uiButtons, uiCursorButtons );
#endif
	}
}

// $deBlob: FUNCTION 005d8ed0
void ABlobInputMap::LoadBindings()
{
	m_bKeyboardEnabled = TTRUE;

	TNativeFileInfo oNativeInfo;

	if ( TFile::ToNative( "setup.ini", oNativeInfo ) )
	{
		TIniFile oIniFile;
		oIniFile.Open( oNativeInfo.InternalPath );

		for ( TINT i = 0; i < NUM_BINDINGS; i++ )
		{
			TINT iValue;

			if ( ms_aKeyboardBindings[ i ].m_szName != TNULL && oIniFile.ReadLong( "binding", ms_aKeyboardBindings[ i ].m_szName, &iValue, 0 ) == TIniFile::RESULT_OK )
				ms_aKeyboardBindings[ i ].m_iButton = TInputDXDeviceKeyboard::TranslateDXToDoodad( iValue );

			if ( ms_aXbox360Bindings[ i ].m_szName != TNULL && oIniFile.ReadLong( "binding", ms_aXbox360Bindings[ i ].m_szName, &iValue, 0 ) == TIniFile::RESULT_OK )
				ms_aXbox360Bindings[ i ].m_iButton = TInputDeviceController::RemapButtonIndex( iValue, TFALSE );

			if ( ms_aPS4Bindings[ i ].m_szName != TNULL && oIniFile.ReadLong( "binding", ms_aPS4Bindings[ i ].m_szName, &iValue, 0 ) == TIniFile::RESULT_OK )
				ms_aPS4Bindings[ i ].m_iButton = TInputDeviceController::RemapButtonIndex( iValue, TTRUE );
		}
	}

	TXUIScene::InitPlatformGlyphs();
}
