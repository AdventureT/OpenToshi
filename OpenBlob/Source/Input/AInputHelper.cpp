#include "pch.h"
#include "AInputHelper.h"

#include "Memory/AMemory.h"

#include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005d9770
void AInputHelper::AddMapping( AInputMap::INPUTBUTTON a_eButton, AInputManager2::INPUTDEVICE a_eDevice, TBOOL a_bRepeat, TFLOAT a_fRepeatTime )
{
	if ( a_bRepeat )
		TASSERT( 0.01666f <= a_fRepeatTime );

	AInputManager2*  pInputManager = AInputManager2::GetSingleton();
	ButtonInfoVector vecButtons;

	if ( m_eInputContext == AInputMap::INPUTCONTEXT_None )
		m_eInputContext = pInputManager->GetContext();

	AInputMap::ButtonMap* pButtonMap = pInputManager->GetInputMap().GetButtonMap( pInputManager->GetContext() );
	auto                  itBinds    = pButtonMap->Find( a_eButton );

	if ( itBinds != pButtonMap->End() )
	{
		AInputMap::BindVector oBinds = itBinds->GetSecond();

		for ( auto it = oBinds.Begin(); it != oBinds.End(); it++ )
		{
			ButtonInfo oInfo;
			oInfo.m_iDoodad        = *it;
			oInfo.m_uiFlags        = a_bRepeat ? ( FLAGS_Repeat | FLAGS_Down ) : FLAGS_Down;
			oInfo.m_fRepeatTime    = a_fRepeatTime;
			oInfo.m_fCurRepeatTime = 0.0f;
			vecButtons.PushBack( oInfo );
		}
	}

	m_oButtonMap.Insert( MakeButtonDevice( a_eButton, a_eDevice ), vecButtons );
}

// $deBlob: FUNCTION 005d9a90
TBOOL AInputHelper::IsJustDown( AInputMap::INPUTBUTTON a_eButton, AInputManager2::INPUTDEVICE a_eDevice )
{
	AInputManager2* pInputManager = AInputManager2::GetSingleton();

	if ( a_eDevice != AInputManager2::INPUTDEVICE_Keyboard && !pInputManager->CheckIfValidDevice( pInputManager->GetControllerHandle( a_eDevice ) ) )
		return TFALSE;

	auto itButton = m_oButtonMap.Find( MakeButtonDevice( a_eButton, a_eDevice ) );

	if ( itButton == m_oButtonMap.End() )
		return TFALSE;

	ButtonInfoVector vecButtons = itButton->GetSecond();

	for ( TINT i = 0; i < vecButtons.Size(); i++ )
	{
		if ( vecButtons[ i ].m_uiFlags & FLAGS_JustDown )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005d9b50
void AInputHelper::Update( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_oButtonMap.Begin(); it != m_oButtonMap.End(); it++ )
	{
		AInputMap::INPUTBUTTON      eButton;
		AInputManager2::INPUTDEVICE eDevice;
		GetInputButtonDevice( it->GetFirst(), eButton, eDevice );

		ButtonInfoVector& rButtons = it->GetSecond();

		for ( TINT i = 0; i < rButtons.Size(); i++ )
		{
			rButtons[ i ].m_fCurRepeatTime += a_fDeltaTime;
			UpdateButtonInfo( &rButtons[ i ], eDevice );
		}
	}
}

// $deBlob: FUNCTION 005d9be0
// $deBlob: FUNCTION 005c2670
void AInputHelper::ResetJustDown()
{
	for ( auto it = m_oButtonMap.Begin(); it != m_oButtonMap.End(); it++ )
	{
		ButtonInfoVector& rButtons = it->GetSecond();

		for ( TINT i = 0; i < rButtons.Size(); i++ )
			rButtons[ i ].m_uiFlags = ( rButtons[ i ].m_uiFlags & ~FLAGS_JustDown ) | FLAGS_Down;
	}
}

// $deBlob: FUNCTION 005d9c50
// $deBlob: FUNCTION 005c2740
void AInputHelper::Destroy()
{
	m_oButtonMap.Clear();
}

// $deBlob: FUNCTION 005d9cc0
AInputHelper::AInputHelper()
    : m_eInputContext( AInputMap::INPUTCONTEXT_None )
    , m_oButtonMap( AMemory::GetAllocator( AMemory::POOL_FrequentAllocations ) )
{
}

// $deBlob: FUNCTION 005d9cf0
// $deBlob: FUNCTION 005da490
AInputHelper::~AInputHelper()
{
	m_oButtonMap.Clear();
}

// $deBlob: FUNCTION 005d9d70
TUINT32 AInputHelper::MakeButtonDevice( AInputMap::INPUTBUTTON a_eButton, AInputManager2::INPUTDEVICE a_eDevice )
{
	TASSERT( 0 == ( 0xFFFF0000 & a_eButton ) );
	TASSERT( 0 == ( 0xFFFF0000 & a_eDevice ) );
	return ( a_eButton << 16 ) | a_eDevice;
}

// $deBlob: FUNCTION 005d9df0
void AInputHelper::GetInputButtonDevice( TUINT32 a_uiButtonDevice, AInputMap::INPUTBUTTON& a_rButton, AInputManager2::INPUTDEVICE& a_rDevice )
{
	a_rButton = a_uiButtonDevice >> 16;
	a_rDevice = a_uiButtonDevice & 0xFFFF;
}

// $deBlob: FUNCTION 005d9e10
void AInputHelper::UpdateButtonInfo( TInputDevice* a_pDevice, ButtonInfo* a_pButtonInfo )
{
	TBOOL bIsDown = a_pDevice->IsDown( a_pButtonInfo->m_iDoodad );
	FLAGS uiFlags = a_pButtonInfo->m_uiFlags;

	if ( m_eInputContext != AInputManager2::GetSingleton()->GetContext() )
		bIsDown = TFALSE;

	if ( !( uiFlags & FLAGS_Down ) && bIsDown == TTRUE )
	{
		a_pButtonInfo->m_uiFlags = uiFlags | FLAGS_JustDown;
		a_pButtonInfo->m_uiFlags |= FLAGS_Down;
	}
	else
	{
		a_pButtonInfo->m_uiFlags = uiFlags & ~FLAGS_JustDown;

		if ( !bIsDown )
			a_pButtonInfo->m_uiFlags = uiFlags & ~( FLAGS_JustDown | FLAGS_Down );
		else
			a_pButtonInfo->m_uiFlags |= FLAGS_Down;
	}

	uiFlags = a_pButtonInfo->m_uiFlags;

	if ( !( uiFlags & FLAGS_Repeat ) || bIsDown != TTRUE )
	{
		a_pButtonInfo->m_fCurRepeatTime = 0.0f;
	}
	else
	{
		TFLOAT fRepeatTime = a_pButtonInfo->m_fRepeatTime;

		if ( fRepeatTime <= a_pButtonInfo->m_fCurRepeatTime )
		{
			do
			{
				uiFlags |= FLAGS_JustDown;
				a_pButtonInfo->m_fCurRepeatTime -= fRepeatTime;
			} while ( fRepeatTime <= a_pButtonInfo->m_fCurRepeatTime );

			a_pButtonInfo->m_uiFlags = uiFlags;
		}
	}
}

// $deBlob: FUNCTION 005dad60
void AInputHelper::AddMapping( AInputMap::INPUTBUTTON a_eButton, TBOOL a_bRepeat, TFLOAT a_fRepeatTime )
{
	for ( TINT i = 0; i < AInputManager2::INPUTDEVICE_Count; i++ )
		AddMapping( a_eButton, i, a_bRepeat, a_fRepeatTime );

	AddMapping( a_eButton, AInputManager2::INPUTDEVICE_Keyboard, a_bRepeat, a_fRepeatTime );
}

// $deBlob: FUNCTION 005dadb0
TBOOL AInputHelper::IsJustDown( AInputMap::INPUTBUTTON a_eButton )
{
	for ( TINT i = 0; i < AInputManager2::INPUTDEVICE_Count; i++ )
	{
		if ( IsJustDown( a_eButton, i ) )
			return TTRUE;
	}

	return IsJustDown( a_eButton, AInputManager2::INPUTDEVICE_Keyboard );
}

// $deBlob: FUNCTION 005dae00
AInputHelper::ButtonResult AInputHelper::IsJustDownWithResult( AInputMap::INPUTBUTTON a_eButton )
{
	ButtonResult oResult;
	oResult.m_bDown              = TFALSE;
	oResult.m_Handle.m_pDevice   = TNULL;
	oResult.m_Handle.m_iId       = 0;

	for ( TINT i = 0; i < AInputManager2::INPUTDEVICE_Count; i++ )
	{
		if ( IsJustDown( a_eButton, i ) )
		{
			oResult.m_bDown  = TTRUE;
			oResult.m_Handle = AInputManager2::GetSingleton()->GetControllerHandle( i );
			return oResult;
		}
	}

	if ( IsJustDown( a_eButton, AInputManager2::INPUTDEVICE_Keyboard ) )
		oResult.m_bDown = TTRUE;

	return oResult;
}

// $deBlob: FUNCTION 005dae90
void AInputHelper::UpdateButtonInfo( ButtonInfo* a_pButtonInfo, AInputManager2::INPUTDEVICE a_eDevice )
{
	TInputDevice* pDevice;

	if ( a_eDevice == AInputManager2::INPUTDEVICE_Keyboard )
		pDevice = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );
	else
		pDevice = AInputManager2::GetSingleton()->GetControllerHandle( a_eDevice ).m_pDevice;

	if ( pDevice != TNULL )
		UpdateButtonInfo( pDevice, a_pButtonInfo );
}
