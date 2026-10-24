#pragma once
#include "Input/AInputManager2.h"

#include <Input/TVibrator.h>
#include <Input/TInputDeviceController.h>
#include <Toshi/T2Array.h>

//-----------------------------------------------------------------------------
// Purpose: controller vibration. On Windows the queued vibrations (VibrateRef) and the TVibrators are kept but never
// run: Update only checks IsAllVibrationsDisabled, the controllers vibrate through VibrateOnce. Most methods aren't
// called in retail (they're linked but unreferenced).
//-----------------------------------------------------------------------------
class AVibrationManager : public Toshi::TSingleton<AVibrationManager>
{
public:
	static TCONSTEXPR_VAR TINT MAX_VIBRATORS      = 4;
	static TCONSTEXPR_VAR TINT MAX_VIBRATIONS     = 32;
	static TCONSTEXPR_VAR TINT VibrationIdInvalid = -1;

	// The vibration scripts (ScriptLowShort...); VibrateOnce takes them as its strength
	enum SCRIPT : TINT
	{
		SCRIPT_Test         = 0,
		SCRIPT_LowShort     = 1,
		SCRIPT_LowMedium    = 2,
		SCRIPT_LowLong      = 3,
		SCRIPT_MediumShort  = 4,
		SCRIPT_MediumMedium = 5,
		SCRIPT_MediumLong   = 6,
		SCRIPT_HighShort    = 7,
		SCRIPT_HighMedium   = 8,
		SCRIPT_HighLong     = 9,
		SCRIPT_ComboDance   = 10,
		SCRIPT_Invalid      = -1,
	};

	enum VIBRATE_STRENGTH : TINT
	{
		VIBRATE_STRENGTH_High   = 0,
		VIBRATE_STRENGTH_Medium = 1,
		VIBRATE_STRENGTH_Low    = 2,
	};

	enum VIBRATE_DURATION : TINT
	{
		VIBRATE_DURATION_Long   = 0,
		VIBRATE_DURATION_Medium = 1,
		VIBRATE_DURATION_Short  = 2,
	};

	// Lower is higher; VIBRATE_PRIORITY_None (999) is lower than everything
	typedef TINT                  VIBRATE_PRIORITY;
	static const VIBRATE_PRIORITY VIBRATE_PRIORITY_None;

	enum FLAGS : TUINT32
	{
		FLAGS_ENABLED = 1 << 0,
		FLAGS_PAUSED  = 1 << 1,
	};

	// A queued vibration, free while m_fTime < 0
	struct VibrateRef
	{
		// $deBlob: FUNCTION 005e1310
		VibrateRef()
		{
			m_eInputDevice = AInputManager2::INPUTDEVICE_Invalid;
			m_fTime        = -1.0f;
			m_eScript      = 0;
			m_ePriority    = 0;
			m_bFlag        = TFALSE;
			m_iPlayer      = -1;
			m_iUnk         = 0;
		}

		// the same as the constructor without the return value
		// $deBlob: FUNCTION 005e14f0
		void Reset()
		{
			m_eInputDevice = AInputManager2::INPUTDEVICE_Invalid;
			m_fTime        = -1.0f;
			m_eScript      = 0;
			m_ePriority    = 0;
			m_bFlag        = TFALSE;
			m_iPlayer      = -1;
			m_iUnk         = 0;
		}

		// $deBlob: FUNCTION 005e1390
		VibrateRef& operator=( const VibrateRef& a_rOther )
		{
			m_eInputDevice = a_rOther.m_eInputDevice;
			m_fTime        = a_rOther.m_fTime;
			m_eScript      = a_rOther.m_eScript;
			m_ePriority    = a_rOther.m_ePriority;
			m_bFlag        = a_rOther.m_bFlag;
			m_iPlayer      = a_rOther.m_iPlayer;
			m_iUnk         = a_rOther.m_iUnk;
			return *this;
		}

		AInputManager2::INPUTDEVICE m_eInputDevice; // 0x00
		TFLOAT                      m_fTime;        // 0x04
		TINT                        m_eScript;      // 0x08
		VIBRATE_PRIORITY            m_ePriority;    // 0x0C
		TBOOL                       m_bFlag;        // 0x10
		TINT                        m_iPlayer;      // 0x14
		TINT                        m_iUnk;         // 0x18
	};

	TSTATICASSERT( sizeof( VibrateRef ) == 0x1C );

public:
	AVibrationManager();
	virtual ~AVibrationManager(); // +0x00

	void   Update( TFLOAT a_fDeltaTime );
	void   PauseImmediate();
	void   ResetAll();
	void   StopVibration( TINT a_iVibrationId );
	TBOOL  CheckHigherPriority( VIBRATE_PRIORITY a_eNew, VIBRATE_PRIORITY a_eCurrent );
	TINT   AddActiveVibration( VibrateRef& a_rVibration );
	TINT   GetNewVibrationId();
	SCRIPT GetScriptId( VIBRATE_STRENGTH a_eStrength, VIBRATE_DURATION a_eDuration );
	TFLOAT GetScriptTime( SCRIPT a_eScript );
	void   Create();
	void   EnableAllVibrators( TBOOL a_bEnable );
	TBOOL  IsVibratorEnabled( TINT a_iVibratorID );
	TBOOL  IsVibratorEnabled( const AInputManager2::AInputDeviceHandle& a_rHandle );
	TBOOL  EnableVibrator( TBOOL a_bEnable, TINT a_iVibratorID );
	TBOOL  EnableVibrator( TBOOL a_bEnable, const AInputManager2::AInputDeviceHandle& a_rHandle );
	TINT   FindVibratorID( const AInputManager2::AInputDeviceHandle& a_rHandle );

	Toshi::TVibrator* Alloc( Toshi::TInputDeviceController* a_pController );
	void              Free( Toshi::TVibrator* a_pVibrator );
	Toshi::TVibrator* GetVibrator( Toshi::TInputDeviceController* a_pController );
	Toshi::TVibrator* GetVibrator( AInputManager2::INPUTDEVICE a_eInputDevice );

	TBOOL IsAllVibrationsDisabled();
	TBOOL IsVibrationsDisabled( AInputManager2::INPUTDEVICE a_eInputDevice );

	// $deBlob: FUNCTION 005e14d0
	TBOOL IsEnabled() const { return m_uiFlags & FLAGS_ENABLED; }

	// Vibrates the device of a_iPlayer once: a_iStrength (a SCRIPT) 1-9 picks the duration (200/400/600 ms,
	// a_iStrength - 1 modulo 3) and, on XInput, the force (0x5555/0xAAAA/0xFFFF by thirds); anything else stops it
	// (0 ms)
	// retail is __thiscall (the callers pass s_Instance in ECX) but never uses this
	static void VibrateOnce( TINT a_iPlayer, TINT a_iStrength );

private:
	Toshi::T2Array<VibrateRef, MAX_VIBRATIONS> m_aVibrations;      // 0x004
	TINT                                       m_iLastVibrationId; // 0x384
	Toshi::TVibrator*                          m_pVibrators;       // 0x388
	TINT                                       m_iNumVibrators;    // 0x38C
	TUINT32                                    m_uiFlags;          // 0x390
};

TSTATICASSERT( sizeof( AVibrationManager ) == 0x394 );
