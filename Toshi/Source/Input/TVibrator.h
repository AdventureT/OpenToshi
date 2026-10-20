#pragma once

TOSHI_NAMESPACE_START

class TInputDevice;

// A command of a vibration script (TVibrator::RunScript), not used on Windows
struct TVibratorCommand;

// The vibration of a controller (AVibrationManager). on Windows it only keeps its state, the controllers vibrate
// through their own functions (TInputDeviceController_XInput::Vibrate)
class TVibrator
{
public:
	enum FLAGS : TUINT32
	{
		FLAGS_ENABLED = 1 << 0,
		FLAGS_PAUSED  = 1 << 1,
	};

public:
	TVibrator();
	~TVibrator();

	TBOOL Create( TInputDevice* a_pDevice );
	void  Destroy();
	void  Reset();
	void  Enable( TBOOL a_bEnable );
	void  Update( TFLOAT a_fDeltaTime );
	void  Pause( TBOOL a_bPause );
	TBOOL RunScript( TVibratorCommand* a_pCommands, TINT a_iNumCommands );

	// $deBlob: FUNCTION 005e14c0
	TInputDevice* GetDevice() const { return m_pDevice; }
	// $deBlob: FUNCTION 005e14e0
	TBOOL IsEnabled() const { return m_uiFlags & FLAGS_ENABLED; }

private:
	TUINT32       m_uiFlags; // 0x00
	TInputDevice* m_pDevice; // 0x04
	TUINT32       m_uiUnk1;  // 0x08 state of the script
	TUINT32       m_uiUnk2;  // 0x0C
	TUINT32       m_uiUnk3;  // 0x10
	TUINT32       m_uiUnk4;  // 0x14
};

TSTATICASSERT( sizeof( TVibrator ) == 0x18 );

TOSHI_NAMESPACE_END
