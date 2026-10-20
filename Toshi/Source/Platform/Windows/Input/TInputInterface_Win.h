#pragma once
#include "Includes.h"
#include "Input/TInputInterface.h"
#include "Thread/TThread.h"
#include "Thread/TMutex.h"
#include "Toshi/THPTimer.h"

TOSHI_NAMESPACE_START

// DirectInput keyboard, mouse and PlayStation controllers, four XInput controllers and four Wii remotes
class TInputDXInterface : public TInputInterface
{
public:
	TDECLARE_CLASS( TInputDXInterface, TInputInterface );

	static TCONSTEXPR_VAR TINT MAXDEVICESNUM = 127;

	// NOTE: the Wii branch's input interface updates the Wii remotes on this thread (TInputRevInterface::KPADThread);
	// on Windows nothing creates it, but its code is linked
	class KPADThread : public TThread
	{
	public:
		KPADThread( TINT a_iSleepTime );
		~KPADThread();

		virtual void Main() override;

	private:
		TINT     m_iSleepTime; // 0x14 milliseconds between the updates, at least 16
		TBOOL    m_bUnknown;   // 0x18
		THPTimer m_oTimer;     // 0x20
		TMutex   m_oMutex;     // 0x48
	};

public:
	TInputDXInterface();
	~TInputDXInterface();

	virtual TBOOL Initialise() override;
	virtual TBOOL Deinitialise() override;
	virtual void  SetExclusiveMode( TBOOL a_bIsExclusive ) override;
	// $deBlob: FUNCTION 00680520
	virtual void  RefreshDirect() {} // +0x30

	TBOOL LostDevice();

	// Like SetExclusiveMode without telling the base class
	void SetExclusive( TBOOL a_bIsExclusive );

	// $deBlob: FUNCTION 00680500
	HWND GetMainWindow() const { return m_hMainWindow; }

	// $deBlob: FUNCTION 00680530
	void SetMainWindow( HWND a_hMainWindow ) { m_hMainWindow = a_hMainWindow; }

	// $deBlob: FUNCTION 00680510
	TBOOL IsExclusive() const { return m_bExclusive; }

	// $deBlob: FUNCTION 00680060
	LPDIRECTINPUT8A GetDirectInput() const { return m_poDirectInput8; }

	static TInputDXInterface* GetInterface() { return ms_pTheInterface; }

	// $deBlob: FUNCTION 00680000
	static TBOOL Unknown1( TINT a_iUnused ) { return TFALSE; }
	// $deBlob: FUNCTION 00680010
	static TBOOL Unknown2( TINT a_iUnused ) { return TFALSE; }
	// $deBlob: FUNCTION 00680020
	static TINT  Unknown3( TINT a_iUnused ) { return 100; }

private:
	static BOOL CALLBACK EnumerateDeviceCallback( LPCDIDEVICEINSTANCEA a_poDeviceInstance, LPVOID poDXInputInterface );

public:
	static TInputDXInterface* ms_pTheInterface;
	static GUID               ms_aRegisteredDevices[ MAXDEVICESNUM ]; // instance GUIDs EnumerateDeviceCallback already saw
	static TINT               ms_iNumRegisteredDevices;

private:
	LPDIRECTINPUT8A m_poDirectInput8; // 0x30
	HWND            m_hMainWindow;    // 0x34
	TBOOL           m_bExclusive;     // 0x38 cooperative level of the mouse
};

TSTATICASSERT( sizeof( TInputDXInterface ) == 0x3C );
TSTATICASSERT( sizeof( TInputDXInterface::KPADThread ) == 0x50 );

TOSHI_NAMESPACE_END
