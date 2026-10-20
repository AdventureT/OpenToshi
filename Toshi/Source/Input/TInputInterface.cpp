#include "ToshiPCH.h"
#include "TInputInterface.h"
#include "TInputDeviceKeyboard.h"
#include "TInputDeviceMouse.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TBOOL                     TInputDevice::ms_bPlayerDevicesLocked;
TINT                      TInputDevice::ms_iLastXUIDeviceIndex;
TInputDevice*             TInputDevice::s_aDevices[ MAX_DEVICE_COUNT ];
TINT                      TInputDevice::s_uiDeviceCount;
TINT                      TInputDevice::ms_aPlayerDevices[ MAX_DEVICE_COUNT ]      = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
TINT                      TInputDevice::ms_aSavedPlayerDevices[ MAX_DEVICE_COUNT ] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
TInputDevice::GAMEPADTYPE TInputDevice::ms_eLastGamepadType                        = Gamepad_Keyboard;

// NOTE: TDEFINE_CLASS generates GetClass, CreateObject, CreateObjectInPlace (abstract: both return TNULL)
// $deBlob: FUNCTION 006772c0
// $deBlob: FUNCTION 006772d0
// $deBlob: FUNCTION 006772e0
// $deBlob: FUNCTION 006772f0
// $deBlob: FUNCTION 00677300
// $deBlob: FUNCTION 0053e810
// $deBlob: FUNCTION 00858df0
TDEFINE_CLASS_NORUNTIME( TInputInterface );

// $deBlob: FUNCTION 00677720
// $Barnyard: FUNCTION 006c3fd0
TInputDevice* TInputInterface::GetDeviceByIndex( const TClass* a_pClass, TUINT a_uiIndex )
{
	TInputDevice* pDevice = m_DeviceList.Begin().Get();

	for ( TInputDevice* pEnd = m_DeviceList.End().Get(); pDevice != pEnd; pDevice = pDevice->Next() )
	{
		if ( pDevice->GetClass()->IsA( a_pClass ) )
		{
			if ( a_uiIndex-- == 0 )
			{
				return pDevice;
			}
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 006776c0
// $Barnyard: FUNCTION 006c3f80
void TInputInterface::AddDevice( TInputDevice* device )
{
	if ( device != TNULL )
	{
		m_DeviceList.InsertTail( device );
		device->SetInputInterface( this );
	}
}

// $Barnyard: FUNCTION 006c3d10
void TInputInterface::SendEvent( TInputDevice* a_pSource, TINT a_iDoodad, EVENT_TYPE a_eEventType )
{
	m_InputEventEmitter.Throw( InputEvent( a_pSource, a_iDoodad, a_eEventType ) );
}

// $deBlob: FUNCTION 006776f0
// $Barnyard: FUNCTION 006c3d50
void TInputInterface::RemoveDevice( TInputDevice* device )
{
	if ( device != TNULL )
	{
		m_DeviceList.Remove( device );
		device->SetInputInterface( TNULL );
	}
}

// $deBlob: FUNCTION 00677510
// $Barnyard: FUNCTION 006c3e10
TBOOL TInputInterface::AcquireAll()
{
	TInputDevice* pDevice = m_DeviceList.Begin().Get();
	TBOOL         bResult = TTRUE;

	for ( TInputDevice* pEnd = m_DeviceList.End().Get(); pDevice != pEnd; pDevice = pDevice->Next() )
	{
		bResult &= pDevice->Acquire();
	}

	return bResult;
}

// $deBlob: FUNCTION 00677550
// $Barnyard: FUNCTION 006c3e50
TBOOL TInputInterface::UnacquireAll()
{
	TInputDevice* pDevice = m_DeviceList.Begin().Get();
	TBOOL         bResult = TTRUE;

	for ( TInputDevice* pEnd = m_DeviceList.End().Get(); pDevice != pEnd; pDevice = pDevice->Next() )
	{
		bResult &= pDevice->Unacquire();
	}

	return bResult;
}

// $deBlob: FUNCTION 00677590
// $Barnyard: FUNCTION 006c3e90
TBOOL TInputInterface::FlushAll()
{
	TInputDevice* pDevice = m_DeviceList.Begin().Get();
	TBOOL         bResult = TTRUE;

	for ( TInputDevice* pEnd = m_DeviceList.End().Get(); pDevice != pEnd; pDevice = pDevice->Next() )
	{
		bResult &= pDevice->Flush();
	}

	return bResult;
}

// $deBlob: FUNCTION 006775d0
// $Barnyard: FUNCTION 006c3cb0
void TInputInterface::SetExclusiveMode( TBOOL a_bIsExclusive )
{
	m_bIsExclusiveMode = a_bIsExclusive;
}

// $deBlob: FUNCTION 006775e0
// $Barnyard: FUNCTION 006c3cc0
TBOOL TInputInterface::GetExclusiveMode() const
{
	return m_bIsExclusiveMode;
}

// $deBlob: FUNCTION 006775f0
// $Barnyard: FUNCTION 006c3ed0
TINT TInputInterface::ProcessEvents( TFLOAT a_fDeltaTime )
{
	TInputDevice* pDevice        = m_DeviceList.Begin().Get();
	TINT          iNumProcessed = 0;

	for ( TInputDevice* pEnd = m_DeviceList.End().Get(); pDevice != pEnd; pDevice = pDevice->Next() )
	{
		if ( pDevice->IsAcquired() )
		{
			pDevice->Update( a_fDeltaTime );
		}
	}

	pDevice = m_DeviceList.Begin().Get();

	for ( TInputDevice* pEnd = m_DeviceList.End().Get(); pDevice != pEnd; pDevice = pDevice->Next() )
	{
#if defined( DEBLOB_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
		// Development: keys injected with -devkeys come in while the keyboard isn't acquired (background runs)
		if ( pDevice->IsAcquired() || ( TInputDeviceKeyboard::Dev_HasInjectedKeys() && pDevice->IsA( &TGetClass( TInputDeviceKeyboard ) ) ) )
#else
		if ( pDevice->IsAcquired() )
#endif
		{
			iNumProcessed += pDevice->ProcessEvents( m_InputEventEmitter, a_fDeltaTime );
		}
	}

	return iNumProcessed;
}

// $deBlob: FUNCTION 00677690
// $Barnyard: FUNCTION 006c3f50
void TInputInterface::StopAllRepeats()
{
	TInputDevice* pDevice = m_DeviceList.Begin().Get();

	for ( TInputDevice* pEnd = m_DeviceList.End().Get(); pDevice != pEnd; pDevice = pDevice->Next() )
	{
		pDevice->StopAllRepeats();
	}
}

// $deBlob: FUNCTION 00677460
// $Barnyard: FUNCTION 006c3d90
TBOOL TInputInterface::ReleaseAllDevices()
{
	TNodeList<TInputDevice>::Iterator itEnd = m_DeviceList.End();
	TNodeList<TInputDevice>::Iterator it    = m_DeviceList.Begin();

	while ( it != itEnd )
	{
		// Take the next device before this one gets deleted
		TNodeList<TInputDevice>::Iterator itNext = it;
		itNext++;

		TInputDevice* pDevice = it.Get();
		pDevice->Unacquire();
		pDevice->Release();
		pDevice->Remove();
		delete pDevice;

		it = itNext;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00677310
// $Barnyard: FUNCTION 006c4030
TInputInterface::TInputInterface()
    : m_InputEventEmitter( this )
    , m_DeviceStatusEmitter( this )
{
	m_bIsExclusiveMode = TFALSE;
}

// $deBlob: FUNCTION 006773a0
// $Barnyard: FUNCTION 006c4080
TInputInterface::~TInputInterface()
{
}

// $deBlob: FUNCTION 00677780
// $Barnyard: FUNCTION 006c3ba0
TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType )
{
	m_pSource           = a_pDevice;
	m_iDoodad           = a_iDoodad;
	m_eEventType        = a_eEventType;
	m_bIsMagnitudeFloat = TFALSE;
	m_iAxisCount        = 0;
	m_wszString[ 0 ]    = L'\0';
}

// $deBlob: FUNCTION 006777b0
// $Barnyard: FUNCTION 006c3bd0
TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TWCHAR* a_wszString )
{
	m_pSource           = a_pDevice;
	m_iDoodad           = a_iDoodad;
	m_eEventType        = a_eEventType;
	m_bIsMagnitudeFloat = TFALSE;
	m_iAxisCount        = 0;
	TStringManager::String16Copy( m_wszString, a_wszString, 3 );
	m_Magnitude.Ints[ 0 ] = 0;
}

// $deBlob: FUNCTION 00677820
TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1 )
{
	m_pSource             = a_pDevice;
	m_iDoodad             = a_iDoodad;
	m_eEventType          = a_eEventType;
	m_Magnitude.Ints[ 0 ] = a_iMagnitude1;
	m_bIsMagnitudeFloat   = TFALSE;
	m_iAxisCount          = 1;
	m_wszString[ 0 ]      = L'\0';
}

// $deBlob: FUNCTION 00677890
// $Barnyard: FUNCTION 006c3c50
TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1, TINT a_iMagnitude2 )
{
	m_pSource             = a_pDevice;
	m_iDoodad             = a_iDoodad;
	m_eEventType          = a_eEventType;
	m_Magnitude.Ints[ 0 ] = a_iMagnitude1;
	m_Magnitude.Ints[ 1 ] = a_iMagnitude2;
	m_bIsMagnitudeFloat   = TFALSE;
	m_iAxisCount          = 2;
	m_wszString[ 0 ]      = L'\0';
}

// $deBlob: FUNCTION 006777f0
TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1 )
{
	m_pSource               = a_pDevice;
	m_iDoodad               = a_iDoodad;
	m_eEventType            = a_eEventType;
	m_Magnitude.Floats[ 0 ] = a_fMagnitude1;
	m_bIsMagnitudeFloat     = TTRUE;
	m_iAxisCount            = 1;
	m_wszString[ 0 ]        = L'\0';
}

// $deBlob: FUNCTION 00677850
// $Barnyard: FUNCTION 006c3c10
TInputInterface::InputEvent::InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1, TFLOAT a_fMagnitude2 )
{
	m_pSource               = a_pDevice;
	m_iDoodad               = a_iDoodad;
	m_eEventType            = a_eEventType;
	m_Magnitude.Floats[ 0 ] = a_fMagnitude1;
	m_Magnitude.Floats[ 1 ] = a_fMagnitude2;
	m_bIsMagnitudeFloat     = TTRUE;
	m_iAxisCount            = 2;
	m_wszString[ 0 ]        = L'\0';
}

// $deBlob: FUNCTION 006778d0
TINT TInputInterface::InputEvent::GetMagnitudeInt( TINT a_iAxis ) const
{
	TASSERT( a_iAxis >= 0 && a_iAxis < m_iAxisCount );

	if ( IsMagnitudeInt() )
	{
		return m_Magnitude.Ints[ a_iAxis ];
	}
	else
	{
		if ( m_Magnitude.Floats[ a_iAxis ] < -0.5f )
		{
			return -1;
		}

		return 0.5f < m_Magnitude.Floats[ a_iAxis ];
	}
}

// $deBlob: FUNCTION 00677950
// $Barnyard: FUNCTION 006c3c90
TFLOAT TInputInterface::InputEvent::GetMagnitudeFloat( TINT a_iAxis ) const
{
	TASSERT( a_iAxis >= 0 && a_iAxis < m_iAxisCount );

	if ( IsMagnitudeFloat() )
	{
		return m_Magnitude.Floats[ a_iAxis ];
	}

	return (TFLOAT)m_Magnitude.Ints[ a_iAxis ];
}

// $deBlob: FUNCTION 006779c0
// $Barnyard: FUNCTION 006c3cd0
TBOOL TInputInterface::InputEvent::StartRepeat( TFLOAT a_fFirstRepeatTime, TFLOAT a_fRepeatTime ) const
{
	return m_pSource->StartRepeat( m_iDoodad, a_fFirstRepeatTime, a_fRepeatTime );
}

// $deBlob: FUNCTION 006779f0
// $Barnyard: FUNCTION 006c3cf0
TBOOL TInputInterface::InputEvent::StartRepeat() const
{
	return m_pSource->StartRepeat( m_iDoodad, 0.0f, 0.0f );
}

// $deBlob: FUNCTION 00677a20
TString8 TInputInterface::InputEvent::GetString() const
{
	TString8 strResult;
	TString8 strMagnitude;

	strResult.Format( "%s - %s", m_pSource->GetButtonFromDoodad( m_iDoodad ), GetEventTypeName( m_eEventType ) );

	for ( TINT i = 0; i < m_iAxisCount; i++ )
	{
		if ( i == 0 )
			strResult += " (";

		if ( m_bIsMagnitudeFloat )
			strResult += strMagnitude.Format( "%f", m_Magnitude.Floats[ i ] );
		else
			strResult += strMagnitude.Format( "%d", m_Magnitude.Ints[ i ] );

		if ( i == m_iAxisCount - 1 )
			strResult += ")";
		else
			strResult += ", ";
	}

	return strResult;
}

// $deBlob: FUNCTION 00677b20
const TCHAR* TInputInterface::InputEvent::GetEventTypeName( EVENT_TYPE a_eEventType )
{
	switch ( a_eEventType )
	{
		case EVENT_TYPE_GONE_DOWN: return "GONEDOWN";
		case EVENT_TYPE_GONE_UP: return "GONEUP";
		case EVENT_TYPE_REPEAT: return "REPEAT";
		case EVENT_TYPE_VIRTUAL: return "MOVEDABS";
		case EVENT_TYPE_MOVED: return "MOVEDREL";
		default: return "UNKNOWN";
	}
}

TOSHI_NAMESPACE_END
