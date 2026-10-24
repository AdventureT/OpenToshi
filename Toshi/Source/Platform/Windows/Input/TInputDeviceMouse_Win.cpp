#include "ToshiPCH.h"
#include "TInputDeviceMouse_Win.h"
#include "TInputInterface_Win.h"
#include "Platform/DX11/TRender_DX11.h"
#include "Platform/DX11/TMSWindow_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0067ef90
// $deBlob: FUNCTION 0067efa0
// $deBlob: FUNCTION 0067efe0
// $deBlob: FUNCTION 0067f010
// $deBlob: FUNCTION 0067f020
// $deBlob: FUNCTION 0053ea80
// $deBlob: FUNCTION 00858e80
TDEFINE_CLASS( TInputDXDeviceMouse );

// $deBlob: FUNCTION 0067f070
// $Barnyard: FUNCTION 006d3c50
TBOOL TInputDXDeviceMouse::Initialise()
{
	m_bReverseButtons  = TFALSE;
	m_dwButtonCurrent  = 0;
	m_dwButtonPrevious = 0;

	POINT cursorPos;
	GetCursorPos( &cursorPos );

	m_iPositionX      = cursorPos.x;
	m_iPositionY      = cursorPos.y;
	m_iPositionZ      = 0;
	m_iDeltaPositionX = 0;
	m_iDeltaPositionY = 0;
	m_iDeltaPositionZ = 0;
	m_iUnk1           = 0;
	m_iUnk2           = 0;
	m_iUnk3           = 0;
	m_iUnk4           = 0;
	m_iUnk5           = 0;
	m_iUnk6           = -1;
	m_iUnk7           = -1;

	DIPROPDWORD dwordProperty;
	dwordProperty.diph.dwSize       = sizeof( dwordProperty );
	dwordProperty.diph.dwHeaderSize = sizeof( DIPROPHEADER );
	dwordProperty.diph.dwObj        = DIMOFS_Z;
	dwordProperty.diph.dwHow        = DIPH_BYOFFSET;

	HRESULT hr = m_poDXInputDevice->GetProperty( DIPROP_GRANULARITY, &dwordProperty.diph );

	if ( hr != DIERR_OBJECTNOTFOUND )
	{
		m_iWheelGranularity = ( hr == DI_OK ) ? dwordProperty.dwData : 0;
		return TTRUE;
	}

	m_iWheelGranularity = 0;
	return TTRUE;
}

// $deBlob: FUNCTION 0067f170
TBOOL TInputDXDeviceMouse::Deinitialise()
{
	Release();
	return TTRUE;
}

// $deBlob: FUNCTION 0067f180
// $Barnyard: FUNCTION 006d3dd0
TBOOL TInputDXDeviceMouse::Acquire()
{
#ifdef DEBLOB_COMMUNITY_PATCH
	// Background test runs never read the user's mouse
	if ( TMSWindow::IsBackgroundMode() )
	{
		SetAcquired( TFALSE );
		return TFALSE;
	}
#endif

	HRESULT hr = m_poDXInputDevice->Acquire();

	if ( FAILED( hr ) )
	{
		SetAcquired( TFALSE );
		return TFALSE;
	}

	SetAcquired( TTRUE );

	if ( hr != S_FALSE )
	{
		RefreshDirect();
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0067f1b0
// $Barnyard: FUNCTION 006d3e00
TBOOL TInputDXDeviceMouse::Unacquire()
{
	SetAcquired( TFALSE );
	HRESULT hr = m_poDXInputDevice->Unacquire();
	return SUCCEEDED( hr );
}

// $deBlob: FUNCTION 0067f1d0
// $Barnyard: FUNCTION 006d3e20
void TInputDXDeviceMouse::Release()
{
	if ( m_poDXInputDevice )
	{
		m_poDXInputDevice->Release();
		m_poDXInputDevice = NULL;
	}
}

// $deBlob: FUNCTION 0067f1f0
// $Barnyard: FUNCTION 006d3e40
TBOOL TInputDXDeviceMouse::Flush()
{
	DWORD dwItems = INFINITE;

	if ( IsAcquired() )
	{
		HRESULT hr = m_poDXInputDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), NULL, &dwItems, 0 );

		if ( SUCCEEDED( hr ) )
		{
			RefreshDirect();
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0067f240
// $Barnyard: FUNCTION 006d3e80
void TInputDXDeviceMouse::RefreshDirect()
{
	if ( IsAcquired() )
	{
		m_dwButtonPrevious = m_dwButtonCurrent;
		m_dwButtonCurrent  = 0;

		DIMOUSESTATE mouseState = {};

		if ( SUCCEEDED( m_poDXInputDevice->Poll() ) &&
		     SUCCEEDED( m_poDXInputDevice->GetDeviceState( sizeof( DIMOUSESTATE ), &mouseState ) ) )
		{
			if ( mouseState.rgbButtons[ 0 ] & 0x80 ) // left button
				m_dwButtonCurrent |= m_bReverseButtons ? 2 : 1;

			if ( mouseState.rgbButtons[ 1 ] & 0x80 ) // right button
				m_dwButtonCurrent |= m_bReverseButtons ? 1 : 2;

			if ( mouseState.rgbButtons[ 2 ] & 0x80 ) // wheel button
				m_dwButtonCurrent |= INPUT_DEVICE_MOUSE_WHEEL;
		}
	}
}

// $deBlob: FUNCTION 0067f2e0
void TInputDXDeviceMouse::Update( TFLOAT a_fDeltaTime )
{
	// NOTE: unlike Barnyard the position comes from the buffered data of ProcessEvents
	if ( IsAcquired() )
	{
		m_iDeltaPositionX = 0;
		m_iDeltaPositionY = 0;
		m_iDeltaPositionZ = 0;

		TRenderDX11* pRender = TRenderDX11::Interface();

		if ( pRender != TNULL )
		{
			TRender::DISPLAYPARAMS* pDisplayParams = pRender->GetCurrentDisplayParams();

			if ( pDisplayParams->bWindowed )
			{
				// The cursor position of the window (WM_MOUSEMOVE)
				TMSWindow* pWindow = pRender->GetMSWindow();
				pWindow->GetMousePosition( m_iPositionX, m_iPositionY );
			}
			else
			{
				TMath::Clip( m_iPositionX, 0, TINT( pDisplayParams->uiWidth ) );
				TMath::Clip( m_iPositionY, 0, TINT( pDisplayParams->uiHeight ) );
			}
		}
	}
}

// $deBlob: FUNCTION 0067f360
// $Barnyard: FUNCTION 006d4200
TINT TInputDXDeviceMouse::ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	m_dwButtonPrevious = m_dwButtonCurrent;
	TINT iNumProcessed = ProcessRepeats( a_rEmitter, a_fDeltaTime );

	DWORD              dwItems = 32;
	DIDEVICEOBJECTDATA dod[ 32 ];
	TUtil::MemClear( dod, sizeof( dod ) );

	HRESULT hRes = m_poDXInputDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), dod, &dwItems, 0 );

	if ( hRes == DIERR_INPUTLOST )
	{
		Unacquire();
		Acquire();

		hRes = m_poDXInputDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), dod, &dwItems, 0 );

		if ( hRes != DI_OK )
		{
			return 0;
		}
	}

	TRenderDX11* pRender = TRenderDX11::Interface();

	for ( DWORD i = 0; i < dwItems; i++ )
	{
		switch ( dod[ i ].dwOfs )
		{
			case DIMOFS_X:
				m_iDeltaPositionX += dod[ i ].dwData;

				// Windowed the position comes from the window (Update)
				if ( pRender == TNULL || !pRender->GetCurrentDisplayParams()->bWindowed )
					m_iPositionX += dod[ i ].dwData;

				a_rEmitter.Throw( TInputInterface::InputEvent( this, AXIS_CURSOR, TInputInterface::EVENT_TYPE_MOVED, m_iDeltaPositionX, m_iDeltaPositionY ) );
				iNumProcessed += 1;
				break;
			case DIMOFS_Y:
				m_iDeltaPositionY += dod[ i ].dwData;

				if ( pRender == TNULL || !pRender->GetCurrentDisplayParams()->bWindowed )
					m_iPositionY += dod[ i ].dwData;

				a_rEmitter.Throw( TInputInterface::InputEvent( this, AXIS_CURSOR, TInputInterface::EVENT_TYPE_MOVED, m_iDeltaPositionX, m_iDeltaPositionY ) );
				iNumProcessed += 1;
				break;
			case DIMOFS_Z:
			{
				m_iDeltaPositionZ += dod[ i ].dwData;
				m_iPositionZ += dod[ i ].dwData;

				// One wheel notch is 120 (WHEEL_DELTA)
				a_rEmitter.Throw( TInputInterface::InputEvent( this, AXIS_WHEEL, TInputInterface::EVENT_TYPE_MOVED, 0.0f, m_iDeltaPositionZ / 120.0f ) );
				iNumProcessed += 1;

#ifdef DEBLOB_COMMUNITY_PATCH
				// Retail divides by zero if the device reports no wheel granularity
				TINT iNumSteps = ( m_iWheelGranularity != 0 ) ? m_iDeltaPositionZ / m_iWheelGranularity : 0;
#else
				TINT iNumSteps = m_iDeltaPositionZ / m_iWheelGranularity;
#endif

				// Every step of the wheel is a press of the wheel button (held while the further steps repeat)
				if ( iNumSteps < 0 )
				{
					a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_BACKWARD, TInputInterface::EVENT_TYPE_GONE_DOWN ) );

					for ( TINT j = iNumSteps + 1; j < 0; j++ )
						a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_BACKWARD, TInputInterface::EVENT_TYPE_REPEAT ) );

					a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_BACKWARD, TInputInterface::EVENT_TYPE_GONE_UP ) );
					iNumProcessed += 1 - iNumSteps;
				}
				else if ( iNumSteps > 0 )
				{
					a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_FORWARD, TInputInterface::EVENT_TYPE_GONE_DOWN ) );

					for ( TINT j = 1; j < iNumSteps; j++ )
						a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_FORWARD, TInputInterface::EVENT_TYPE_REPEAT ) );

					a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_FORWARD, TInputInterface::EVENT_TYPE_GONE_UP ) );
					iNumProcessed += iNumSteps + 1;
				}

				break;
			}
			case DIMOFS_BUTTON0:
			case DIMOFS_BUTTON1:
			case DIMOFS_BUTTON2:
			{
				TINT iButton =
				    ( dod[ i ].dwOfs == DIMOFS_BUTTON0 ) ? ( !m_bReverseButtons ? 0 : 1 ) :
				    ( dod[ i ].dwOfs == DIMOFS_BUTTON1 ) ? ( !m_bReverseButtons ? 1 : 0 ) :
				                                           2;

				TUINT uiButtonFlag = ( 1 << iButton );

				TASSERT( iButton < INPUT_DEVICE_MOUSE_BUTTONS );

				if ( dod[ i ].dwData & 0x80 )
				{
					if ( ( m_dwButtonCurrent & uiButtonFlag ) == 0 )
					{
						a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_1 + iButton, TInputInterface::EVENT_TYPE_GONE_DOWN ) );
						iNumProcessed += 1;
						m_dwButtonCurrent |= uiButtonFlag;
					}
					else
					{
						TASSERT( ( m_dwButtonCurrent & ( 1 << iButton ) ) != 0 );
					}
				}
				else
				{
					if ( ( m_dwButtonCurrent & uiButtonFlag ) != 0 )
					{
						a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_1 + iButton, TInputInterface::EVENT_TYPE_GONE_UP ) );
						iNumProcessed += 1;
						m_dwButtonCurrent &= ~uiButtonFlag;
					}
					else
					{
						TASSERT( ( m_dwButtonCurrent & ( 1 << iButton ) ) == 0 );
					}
				}

				break;
			}
		}
	}

	return iNumProcessed;
}

// $deBlob: FUNCTION 0067f890
// $Barnyard: FUNCTION 006d4000
TINT TInputDXDeviceMouse::GetButtonCount() const
{
	return m_DIDevCaps.dwButtons;
}

// $deBlob: FUNCTION 0067f8a0
// $Barnyard: FUNCTION 006d4010
TINT TInputDXDeviceMouse::GetAxisCount() const
{
	return m_DIDevCaps.dwAxes;
}

// $deBlob: FUNCTION 0067f8b0
// $Barnyard: FUNCTION 006d4020
TBOOL TInputDXDeviceMouse::IsDown( TINT a_iDoodad ) const
{
	return TUINT( a_iDoodad - BUTTON_1 ) <= 7 && ( m_dwButtonCurrent & ( 1 << ( ( a_iDoodad - 1 ) & 0x1F ) ) ) != 0;
}

// $deBlob: FUNCTION 0067f8f0
// $Barnyard: FUNCTION 006d4060
TBOOL TInputDXDeviceMouse::WasDown( TINT a_iDoodad ) const
{
	return TUINT( a_iDoodad - BUTTON_1 ) <= 7 && ( m_dwButtonPrevious & ( 1 << ( ( a_iDoodad - 1 ) & 0x1F ) ) ) != 0;
}

// $deBlob: FUNCTION 0067f930
// $Barnyard: FUNCTION 006d40a0
TINT TInputDXDeviceMouse::GetAxisInt( Doodad a_iDoodad, TINT coord ) const
{
	if ( a_iDoodad == AXIS_CURSOR )
	{
		if ( coord == 0 )
		{
			return m_iDeltaPositionX;
		}

		if ( coord == 1 )
		{
			return m_iDeltaPositionY;
		}
	}

	return 0;
}

// $deBlob: FUNCTION 0067f960
// $Barnyard: FUNCTION 006d40d0
TFLOAT TInputDXDeviceMouse::GetAxisFloat( Doodad a_iDoodad, TINT coord ) const
{
	if ( a_iDoodad == AXIS_CURSOR )
	{
		if ( coord == 0 )
		{
			return TFLOAT( m_iDeltaPositionX );
		}

		if ( coord == 1 )
		{
			return TFLOAT( m_iDeltaPositionY );
		}
	}
	else if ( a_iDoodad == AXIS_WHEEL )
	{
		return TFLOAT( m_iDeltaPositionZ );
	}

	return 0.0f;
}

// $deBlob: FUNCTION 0067f9b0
// $Barnyard: FUNCTION 006d4110
void TInputDXDeviceMouse::SetExclusive( HWND a_hWindow, TBOOL a_bExclusive )
{
	m_poDXInputDevice->SetCooperativeLevel(
	    a_hWindow,
	    a_bExclusive == TTRUE ? ( DISCL_EXCLUSIVE | DISCL_FOREGROUND ) : ( DISCL_NONEXCLUSIVE | DISCL_FOREGROUND )
	);
}

// $deBlob: FUNCTION 0067f9e0
// $Barnyard: FUNCTION 006d4140
void TInputDXDeviceMouse::SetCurrentPosition( TINT a_iX, TINT a_iY )
{
	auto pDisplayParams = TRenderDX11::Interface()->GetCurrentDisplayParams();

	TMath::Clip( a_iX, 0, TINT( pDisplayParams->uiWidth ) );
	TMath::Clip( a_iY, 0, TINT( pDisplayParams->uiHeight ) );

	m_iPositionX = a_iX;
	m_iPositionY = a_iY;
}

// $deBlob: FUNCTION 0067fa30
// $Barnyard: FUNCTION 006d3d00
TBOOL const TInputDXDeviceMouse::BindToDIDevice( HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDXDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice, TBOOL a_bExclusive )
{
	TASSERT( a_poDXDeviceInstance != NULL );
	TASSERT( a_poDXInputDevice != NULL );

	Release();
	TUtil::MemCopy( &m_oDeviceInstance, a_poDXDeviceInstance, sizeof( DIDEVICEINSTANCEA ) );

	m_poDXInputDevice = a_poDXInputDevice;

	m_DIDevCaps.dwSize = sizeof( DIDEVCAPS );
	m_poDXInputDevice->GetCapabilities( &m_DIDevCaps );

	if ( m_poDXInputDevice->SetDataFormat( &c_dfDIMouse ) != DI_OK )
		return TFALSE;

	if ( a_hMainWindow )
	{
		m_poDXInputDevice->SetCooperativeLevel( a_hMainWindow, DISCL_FOREGROUND | ( a_bExclusive == TTRUE ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE ) );
	}

	DIPROPDWORD dipdw{};
	dipdw.diph.dwSize       = sizeof( DIPROPDWORD );
	dipdw.diph.dwHeaderSize = sizeof( DIPROPHEADER );
	dipdw.diph.dwObj        = 0;
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = DIPROPAXISMODE_REL;

	return m_poDXInputDevice->SetProperty( DIPROP_AXISMODE, &dipdw.diph ) == DI_OK;
}

TOSHI_NAMESPACE_END
