#pragma once
#include <XUI/TXUIScene.h>
#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Purpose: a two state switch of an analog input: the input (averaged over m_iNumSamples samples) turns it on above
// m_fOnThreshold and off below m_fOffThreshold, checked every m_fInterval seconds, and a state has to last m_fMinTime
// before the history (a bit per state change) takes it. Nothing uses it in retail.
//-----------------------------------------------------------------------------
class ABlobInputSwitch
{
public:
	ABlobInputSwitch( TFLOAT a_fOnThreshold, TFLOAT a_fOffThreshold, TINT a_iNumSamples, TFLOAT a_fMinTime, TFLOAT a_fInterval );
	~ABlobInputSwitch();

	// Adds a sample and updates the state; returns the averaged input
	TFLOAT Update( TFLOAT a_fInput, TFLOAT a_fDeltaTime );
	void   Reset();

public:
	TFLOAT  m_fOnThreshold;  // 0x00
	TFLOAT  m_fOffThreshold; // 0x04
	TFLOAT  m_fMinTime;      // 0x08
	TFLOAT  m_fInterval;     // 0x0C
	TINT    m_iState;        // 0x10 0: off, 1: on
	TFLOAT  m_fStateTime;    // 0x14
	TUINT32 m_uiHistory;     // 0x18 the past states, a bit each (the newest in bit 0)
	TFLOAT  m_fTimer;        // 0x1C
	TINT    m_iNumSamples;   // 0x20
	TFLOAT* m_pSamples;      // 0x24
	TFLOAT  m_fSampleSum;    // 0x28
	TINT    m_iSampleIndex;  // 0x2C -1
};

//-----------------------------------------------------------------------------
// Purpose: the same with three states: 1 above m_fOnThreshold, 3 below m_fLowThreshold, 0 back between m_fOffThreshold
// and m_fLowOffThreshold; two history bits per state. Nothing uses it in retail.
//-----------------------------------------------------------------------------
class ABlobInputSwitch3
{
public:
	ABlobInputSwitch3( TFLOAT a_fOnThreshold, TFLOAT a_fOffThreshold, TFLOAT a_fLowThreshold, TFLOAT a_fLowOffThreshold, TINT a_iNumSamples, TFLOAT a_fMinTime, TFLOAT a_fInterval );
	~ABlobInputSwitch3();

	TFLOAT Update( TFLOAT a_fInput, TFLOAT a_fDeltaTime );
	void   Reset();

public:
	TFLOAT  m_fOnThreshold;     // 0x00
	TFLOAT  m_fOffThreshold;    // 0x04
	TFLOAT  m_fLowThreshold;    // 0x08
	TFLOAT  m_fLowOffThreshold; // 0x0C
	TFLOAT  m_fMinTime;         // 0x10
	TFLOAT  m_fInterval;        // 0x14
	TINT    m_iState;           // 0x18 0, 1 (high) or 3 (low)
	TFLOAT  m_fStateTime;       // 0x1C
	TUINT32 m_uiHistory;        // 0x20 the past states, two bits each
	TFLOAT  m_fTimer;           // 0x24
	TINT    m_iNumSamples;      // 0x28
	TFLOAT* m_pSamples;         // 0x2C
	TFLOAT  m_fSampleSum;       // 0x30
	TINT    m_iSampleIndex;     // 0x34 -1
};

//-----------------------------------------------------------------------------
// Purpose: Blob's controls: the bindings of the player actions for each device type (the defaults, setup.ini's
// "binding" section overrides them), which TXUIScene also uses for its glyphs, and the player input read through them
// every frame (created by AInGameState).
// NOTE: the class name is taken from the file name
//-----------------------------------------------------------------------------
class ABlobInputMap
{
public:
	using Binding = Toshi::TXUIScene::InputBinding;

	static TCONSTEXPR_VAR TINT NUM_BINDINGS = Toshi::TXUIScene::NUM_PLATFORM_GLYPHS;
	static TCONSTEXPR_VAR TINT MAX_PLAYERS  = 4;

	// The bindings (setup.ini's "binding" names; the indices are TXUIScene's platform glyphs)
	enum BINDING
	{
		BINDING_JUMP         = 0,
		BINDING_NAVRING      = 1,
		BINDING_INTERACT     = 2,
		BINDING_BRAKE        = 5,
		BINDING_CAMLEFT      = 6,
		BINDING_CAMRIGHT     = 7,
		BINDING_CAMUP        = 8,
		BINDING_CAMDOWN      = 9,
		BINDING_BLOBVISION   = 10,
		BINDING_CENTERCAMERA = 11,
		BINDING_ZLOCK        = 12,
		BINDING_JUMP2        = 14, // also "Jump"
		BINDING_JUMP3        = 15, // also "Jump"
		BINDING_SKIP         = 16,
		BINDING_UP           = 17,
		BINDING_DOWN         = 18,
		BINDING_RIGHT        = 19,
		BINDING_LEFT         = 20,
	};

	// The bits of PlayerInput::m_uiButtons: 1 << the binding
	enum BUTTON : TUINT32
	{
		BUTTON_JUMP         = BITFLAG( BINDING_JUMP ), // pressed this frame (a pad)
		BUTTON_NAVRING      = BITFLAG( BINDING_NAVRING ),
		BUTTON_INTERACT     = BITFLAG( BINDING_INTERACT ),
		BUTTON_UNK8         = BITFLAG( 3 ),
		BUTTON_BRAKE        = BITFLAG( BINDING_BRAKE ), // also BlobVision (the keyboard: the mouse look)
		BUTTON_CAMLEFT      = BITFLAG( BINDING_CAMLEFT ),
		BUTTON_CAMRIGHT     = BITFLAG( BINDING_CAMRIGHT ),
		BUTTON_CAMUP        = BITFLAG( BINDING_CAMUP ),
		BUTTON_CAMDOWN      = BITFLAG( BINDING_CAMDOWN ),
		BUTTON_BLOBVISION   = BITFLAG( BINDING_BLOBVISION ),
		BUTTON_CENTERCAMERA = BITFLAG( BINDING_CENTERCAMERA ),
		BUTTON_ZLOCK        = BITFLAG( BINDING_ZLOCK ),
		BUTTON_JUMP2        = BITFLAG( BINDING_JUMP2 ), // pressed this frame
		BUTTON_JUMP3        = BITFLAG( BINDING_JUMP3 ), // pressed this frame
		BUTTON_SKIP         = BITFLAG( BINDING_SKIP ),
	};

	// One player's input (0x24 bytes)
	struct PlayerInput
	{
		// $deBlob: FUNCTION 005d7e10
		PlayerInput() { Reset(); }

		// $deBlob: FUNCTION 005d7e60
		void Reset()
		{
			m_vMove         = Toshi::TVector2::VEC_ZERO;
			m_vCamera       = Toshi::TVector2::VEC_ZERO;
			m_vCursor       = Toshi::TVector2::VEC_ZERO;
			m_uiButtons     = 0;
			m_uiPrevButtons = 0;
			m_bCursorActive = TFALSE;
		}

		Toshi::TVector2 m_vMove;         // 0x00 the movement stick (keyboard: Up/Down/Left/Right ramp at 2 / s)
		Toshi::TVector2 m_vCamera;       // 0x08 the camera stick
		Toshi::TVector2 m_vCursor;       // 0x10 the mouse cursor, -1..1 from the window's centre
		TUINT32         m_uiButtons;     // 0x18 BUTTON_*
		TUINT32         m_uiPrevButtons; // 0x1C last frame's m_uiButtons
		TBOOL           m_bCursorActive; // 0x20 the mouse moved during the last second
	};

public:
	ABlobInputMap();
	~ABlobInputMap();

	// Reads the input of every player
	void Update( TFLOAT a_fDeltaTime );

	// $deBlob: FUNCTION 0058cbb0
	const Toshi::TVector2& GetMoveStick( TINT a_iPlayer ) const { return m_bDisabled ? Toshi::TVector2::VEC_ZERO : m_aPlayers[ a_iPlayer ].m_vMove; }
	const PlayerInput&     GetPlayerInput( TINT a_iPlayer ) const { return m_aPlayers[ a_iPlayer ]; }

	// $deBlob: FUNCTION 0058c9a0
	TBOOL IsButtonDown( TINT a_iPlayer, TINT a_iButton ) const { return !m_bDisabled && ( ( 1 << a_iButton ) & m_aPlayers[ a_iPlayer ].m_uiButtons ) != 0; }

	// $deBlob: FUNCTION 0058cc00
	TBOOL IsButtonPressed( TINT a_iPlayer, TINT a_iButton ) const { return !m_bDisabled && ( m_aPlayers[ a_iPlayer ].m_uiButtons & ( 1 << a_iButton ) & ( m_aPlayers[ a_iPlayer ].m_uiButtons ^ m_aPlayers[ a_iPlayer ].m_uiPrevButtons ) ) != 0; }

	TUINT32 GetButtons( TINT a_iPlayer ) const { return m_aPlayers[ a_iPlayer ].m_uiButtons; }
	TUINT32 GetPrevButtons( TINT a_iPlayer ) const { return m_aPlayers[ a_iPlayer ].m_uiPrevButtons; }
	TBOOL   IsDisabled() const { return m_bDisabled; }

	// Ignores the input for a_fTime seconds
	void DisableFor( TFLOAT a_fTime )
	{
		m_bDisabled     = TTRUE;
		m_fDisabledTime = a_fTime;
	}

	// Gives the input back at once (ABlobChallengeEndHud)
	void Enable() { m_bDisabled = TFALSE; }

	// Zeroes the input of every player (0x5d7cf0; it first calls the nullsub 0x5d9070, which does nothing)
	// $deBlob: FUNCTION 005d7cf0
	void ClearPlayers()
	{
		for ( TINT i = 0; i < MAX_PLAYERS; i++ )
		{
			PlayerInput& rPlayer    = m_aPlayers[ i ];
			rPlayer.m_vMove         = Toshi::TVector2::VEC_ZERO;
			rPlayer.m_vCamera       = Toshi::TVector2::VEC_ZERO;
			rPlayer.m_vCursor       = Toshi::TVector2::VEC_ZERO;
			rPlayer.m_uiButtons     = 0;
			rPlayer.m_uiPrevButtons = 0;
			rPlayer.m_bCursorActive = TFALSE;
		}
	}

	// Clears buttons of a player (and from last frame's buttons)
	void ClearButtons( TINT a_iPlayer, TUINT32 a_uiButtons )
	{
		m_aPlayers[ a_iPlayer ].m_uiButtons &= ~a_uiButtons;
		m_aPlayers[ a_iPlayer ].m_uiPrevButtons &= ~a_uiButtons;
	}

	// Sets buttons of a player as held since the last frame (so they aren't a new press)
	void SetButtonsHeld( TINT a_iPlayer, TUINT32 a_uiButtons )
	{
		m_aPlayers[ a_iPlayer ].m_uiButtons |= a_uiButtons;
		m_aPlayers[ a_iPlayer ].m_uiPrevButtons |= a_uiButtons;
	}

	static ABlobInputMap* GetSingleton() { return ms_pSingleton; }

	// Installs the bindings in TXUIScene (retail's TXUIScene reads them directly)
	static void InstallXUIBindings();

private:
	// Reads setup.ini's "binding" section
	void LoadBindings();

	// $deBlob: FUNCTION 005d9060
	void Deinitialise() {}

public:
	static Binding ms_aKeyboardBindings[ NUM_BINDINGS ]; // TInputDeviceKeyboard doodads
	static Binding ms_aXbox360Bindings[ NUM_BINDINGS ];  // XInput button bits
	static Binding ms_aPS4Bindings[ NUM_BINDINGS ];      // DirectInput button bits
	static Binding ms_aWiiBindings[ NUM_BINDINGS ];      // Wii remote buttons
	static Binding ms_aWiiExtensionBindings[ NUM_BINDINGS ];

private:
	static ABlobInputMap* ms_pSingleton;

private:
	PlayerInput m_aPlayers[ MAX_PLAYERS ]; // 0x00
	TBOOL       m_bKeyboardEnabled;        // 0x90 set by LoadBindings
	TBOOL       m_bDisabled;               // 0x91 no input until m_fDisabledTime runs out
	TFLOAT      m_fDisabledTime;           // 0x94 -1: none
};

TSTATICASSERT( sizeof( ABlobInputMap ) == 0x98 );
