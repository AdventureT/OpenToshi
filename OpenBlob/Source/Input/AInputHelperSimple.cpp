#include "pch.h"
#include "AInputHelperSimple.h"
#include "Memory/AMemory.h"

#include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The default constructor (array elements, e.g. ACheatManager's pad helpers)
// $deBlob: FUNCTION 005daa20
AInputHelperSimple::AInputHelperSimple()
{
	m_pArray       = TNULL;
	m_iArrayIndex  = 0;
	m_iArraySize   = 0;
	m_pInputDevice = TNULL;
}

// $deBlob: FUNCTION 005daa40
AInputHelperSimple::AInputHelperSimple( TINT a_iArraySize, TInputDevice* a_pInputDevice )
{
	m_iArrayIndex  = 0;
	m_iArraySize   = a_iArraySize;
	m_pInputDevice = a_pInputDevice;
	TASSERT( 0 != m_iArraySize, "ERROR: Initialising with zero array size\n" );

	m_pArray = TSTATICCAST( DoodadInfo, TMalloc( a_iArraySize * sizeof( DoodadInfo ), AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) );
	TUtil::MemSet( m_pArray, 0, a_iArraySize * sizeof( DoodadInfo ) );
}

// $deBlob: FUNCTION 005daad0
AInputHelperSimple::~AInputHelperSimple()
{
	TFree( m_pArray );
}

// $deBlob: FUNCTION 005daae0
void AInputHelperSimple::Initialise( TINT a_iDoodadCount, TInputDevice* a_pInputDevice )
{
	TASSERT( 0 != a_iDoodadCount, "ERROR: Initialising with zero array size\n" );
	m_pInputDevice = a_pInputDevice;
	m_iArraySize   = a_iDoodadCount;

	m_pArray = TSTATICCAST( DoodadInfo, TMalloc( a_iDoodadCount * sizeof( DoodadInfo ), AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) );
	TUtil::MemSet( m_pArray, 0, a_iDoodadCount * sizeof( DoodadInfo ) );
}

// $deBlob: FUNCTION 005dab60
void AInputHelperSimple::AddDoodad( TINT a_iDoodad )
{
	TASSERT( 0 != m_iArraySize, "ERROR: Initialising with zero array size\n" );
	TASSERT( m_iArrayIndex < m_iArraySize );

	m_pArray[ m_iArrayIndex ].m_iDoodad = a_iDoodad;
	m_iArrayIndex++;
}

// $deBlob: FUNCTION 005dabe0
void AInputHelperSimple::Update()
{
	if ( m_pInputDevice == TNULL )
		return;

	// A keyboard only counts while it's enabled
	if ( m_pInputDevice->IsA( &TGetClass( TInputDeviceKeyboard ) ) == TTRUE && !m_pInputDevice->IsEnabled() )
		return;

	for ( TINT i = 0; i < m_iArrayIndex; i++ )
	{
		switch ( m_pArray[ i ].m_eState )
		{
			case STATE_None:
				if ( m_pInputDevice->IsDown( m_pArray[ i ].m_iDoodad ) )
					m_pArray[ i ].m_eState = STATE_Pressed;
				break;
			case STATE_Pressed:
				m_pArray[ i ].m_eState = m_pInputDevice->IsDown( m_pArray[ i ].m_iDoodad ) ? STATE_Held : STATE_Released;
				break;
			case STATE_Held:
				if ( !m_pInputDevice->IsDown( m_pArray[ i ].m_iDoodad ) )
					m_pArray[ i ].m_eState = STATE_Released;
				break;
			case STATE_Released:
				m_pArray[ i ].m_eState = STATE_None;
				break;
		}
	}
}

// $deBlob: FUNCTION 005daca0
TBOOL AInputHelperSimple::WasPressed( TINT a_iDoodad )
{
	for ( TINT i = 0; i < m_iArrayIndex; i++ )
	{
		if ( m_pArray[ i ].m_iDoodad == a_iDoodad )
			return m_pArray[ i ].m_eState == STATE_Pressed;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005dace0
TBOOL AInputHelperSimple::IsHeld( TINT a_iDoodad )
{
	for ( TINT i = 0; i < m_iArrayIndex; i++ )
	{
		if ( m_pArray[ i ].m_iDoodad == a_iDoodad )
			return m_pArray[ i ].m_eState == STATE_Held;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005dad20
TBOOL AInputHelperSimple::WasReleased( TINT a_iDoodad )
{
	for ( TINT i = 0; i < m_iArrayIndex; i++ )
	{
		if ( m_pArray[ i ].m_iDoodad == a_iDoodad )
			return m_pArray[ i ].m_eState == STATE_Released;
	}

	return TFALSE;
}
