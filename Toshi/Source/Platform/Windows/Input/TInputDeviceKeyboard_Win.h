#pragma once
#include "Input/TInputDeviceKeyboard.h"
#include "Toshi/T2Array.h"

#include "Includes.h"

TOSHI_NAMESPACE_START

class TInputDXDeviceKeyboard : public TInputDeviceKeyboard
{
public:
	TDECLARE_CLASS( TInputDXDeviceKeyboard, TInputDeviceKeyboard );

public:
	struct KeyInfo
	{
		TWCHAR* wszKeyName;    // the object name of the key (EnumObjectCallback)
		DWORD   dwKeyScanCode; // DIPROP_SCANCODE, -1 if it has none
	};

public:
	TInputDXDeviceKeyboard();
	~TInputDXDeviceKeyboard();

	virtual TBOOL Acquire() override;
	virtual TBOOL Unacquire() override;
	virtual void  Release() override;
	virtual void  Update( TFLOAT deltaTime ) override;
	virtual TBOOL Flush() override;
	virtual TINT  ProcessEvents( EventEmitter& emitter, TFLOAT deltaTime ) override;
	// $deBlob: FUNCTION 00678650
	virtual TINT  GetButtonCount() const override { return 0; }
	virtual TBOOL IsDown( TINT doodad ) const override;
	// $deBlob: FUNCTION 00679a90
	virtual TBOOL   IsEnabled() const override { return TTRUE; }
	virtual TBOOL   IsShiftDown() const override;
	virtual TBOOL   IsControlDown() const override;
	virtual TBOOL   IsAltDown() const override;
	virtual TBOOL   WasDown( TINT doodad ) const override;
	virtual TWCHAR* TranslateDoodadToCharacter( TINT doodad ) const override;
	virtual TBOOL   Initialise();    // +0x74
	virtual TBOOL   Deinitialise();  // +0x78
	virtual void    RefreshDirect(); // +0x7C

	// Name of the key (as DirectInput calls it), TNULL if the doodad isn't a key
	const TWCHAR* GetKeyName( Doodad a_iDoodad ) const;

	// Changes the state of a key without throwing events (a_iKey is a DIK_ code)
	void ClearKey( TINT a_iKey );
	void ClearAllKeys();
	void SetKeyDown( TINT a_iKey );
	void SetKeyUp( TINT a_iKey );

public:
	static TINT TranslateDXToDoodad( TINT dxkey );

protected:
	TBOOL       HandleKeyChange( EventEmitter& a_Emitter, TINT a_iKeyIndex, TUINT8 a_ui8KeyState );
	static TINT TranslateDoodadToDX( TINT doodad );

public:
	const TBOOL          BindToDIDevice( HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice );
	static BOOL CALLBACK EnumObjectCallback( LPCDIDEVICEOBJECTINSTANCEA a_poObjectInstance, LPVOID a_pvRef );

private:
	TINT                                   m_iSomeNum;          // 0x070
	TUINT8*                                m_pKeyStates1;       // 0x074 current states
	TUINT8*                                m_pKeyStates2;       // 0x078 previous states
	TUINT8                                 m_aBuffer[ 512 ];    // 0x07C
	T2Array<KeyInfo, KEYBOARD_NUM_DOODADS> m_aKeys;             // 0x27C
	TBOOL                                  m_bIsUpdating;       // 0x67C
	DIDEVICEINSTANCEA                      m_oDeviceInstance;   // 0x680
	DIDEVCAPS                              m_DIDevCaps;         // 0x8C4
	IDirectInputDevice8A*                  m_poDXInputDevice;   // 0x8F0
	TBOOL                                  m_bFlag;             // 0x8F4 the states got refreshed: throw the changes
};

TSTATICASSERT( sizeof( TInputDXDeviceKeyboard ) == 0x8F8 );

TOSHI_NAMESPACE_END
