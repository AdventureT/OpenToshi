#include "ToshiPCH.h"
#include "TSound.h"

#include <fmod_errors.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006ffd10
TSound::TSound()
{
	m_pEventSystem = TNULL;
	m_pSystem      = TNULL;
	m_bInitialised = TFALSE;
	m_bUnknown     = TFALSE;
}

TSound::~TSound()
{
}

// $deBlob: FUNCTION 006ffdd0
TBOOL TSound::Create( const SoundInitValues* a_pInitValues )
{
	return Create( a_pInitValues->pPoolMem, a_pInitValues->iPoolSize, a_pInitValues->iMaxChannels, a_pInitValues->iFileBufferSize, a_pInitValues->eSpeakerType );
}

// $deBlob: FUNCTION 006ffdf0
TBOOL TSound::Create( void* a_pPoolMem, TINT a_iPoolSize, TINT a_iMaxChannels, TINT a_iFileBufferSize, SpeakerType a_eSpeakerType )
{
	if ( m_pEventSystem != TNULL )
	{
		TUtil::Log( "TSound::Create() - Event system has already been created. Only one event system can be created at one time.\n" );
		return TFALSE;
	}

	if ( m_pSystem != TNULL )
	{
		TUtil::Log( "TSound::Create() - FMOD system has already been created. Only one FMOD system can be created at one time.\n" );
		return TFALSE;
	}

	m_oInitValues.pPoolMem  = a_pPoolMem;
	m_oInitValues.iPoolSize = a_iPoolSize;

	TBOOL bInitialiseMemoryResult = InitialiseMemory( a_pPoolMem, a_iPoolSize );
	TASSERT( TTRUE == bInitialiseMemoryResult );

	FMOD_RESULT eResult = FMOD::EventSystem_Create( &m_pEventSystem );

	if ( ErrorCheck( eResult ) )
	{
		TUtil::Log( "TSound::Create() - Failed to create FMOD Event System" );
		return TFALSE;
	}

	eResult = m_pEventSystem->getSystemObject( &m_pSystem );

	if ( ErrorCheck( eResult ) )
	{
		TUtil::Log( "TSound::Create() - Failed to get FMOD System object" );
		return TFALSE;
	}

	TUINT uiVersion;

	if ( ErrorCheck( m_pSystem->getVersion( &uiVersion ) ) == TTRUE )
	{
		TUtil::Log( "TSound::Create() - Can't get FMOD version" );
		return TFALSE;
	}

	if ( uiVersion < 0x44447 )
	{
		TUtil::Log( "TSound::Create() - You are using an old version of FMOD %08x.  This program requires %08x", uiVersion, 0x44447 );
		return TFALSE;
	}

	TBOOL bInitialiseFileSystemResult = InitialiseFileSystem();
	TASSERT( TTRUE == bInitialiseFileSystemResult );

	m_oInitValues.iMaxChannels = a_iMaxChannels;
	m_oInitValues.eSpeakerType = a_eSpeakerType;

	TBOOL bInitialiseResult = Initialise( a_iMaxChannels, a_eSpeakerType );
	TASSERT( TTRUE == bInitialiseResult );

	if ( a_iFileBufferSize < 0 )
		a_iFileBufferSize = 0x20000;

	m_oInitValues.iFileBufferSize = a_iFileBufferSize;
	m_pSystem->setStreamBufferSize( a_iFileBufferSize, FMOD_TIMEUNIT_RAWBYTES );

	TBOOL bResult = InitialiseNetwork();
	TASSERT( TTRUE == bResult );

	return TTRUE;
}

// $deBlob: FUNCTION 00700000
void TSound::ResetSystem()
{
	Destroy();
	Create( m_oInitValues.pPoolMem, m_oInitValues.iPoolSize, m_oInitValues.iMaxChannels, m_oInitValues.iFileBufferSize, m_oInitValues.eSpeakerType );
}

// $deBlob: FUNCTION 00700020
void TSound::Destroy()
{
	TASSERT( TNULL != m_pEventSystem );
	TASSERT( TNULL != m_pSystem );

	FMOD_RESULT eResult = m_pEventSystem->release();

	if ( ErrorCheck( eResult ) )
	{
		TUtil::Log( "TSound::Destroy() - Could not release event system" );
	}
	else
	{
		m_pEventSystem = TNULL;
		m_pSystem      = TNULL;
	}

	TBOOL bResult = DestroyNetwork();
	TASSERT( TTRUE == bResult );
}

// $deBlob: FUNCTION 00700100
void TSound::Update()
{
	TASSERT( TNULL != m_pEventSystem );
	TASSERT( TNULL != m_pSystem );

	if ( ErrorCheck( m_pEventSystem->update() ) )
		TUtil::Log( "TSound::Update() - Could not update the FMOD system" );

	TBOOL bResult = UpdateNetwork();
	TASSERT( TTRUE == bResult );
}

// $deBlob: FUNCTION 007001e0
FMOD::EventSystem* TSound::GetEventSystem()
{
	TASSERT( TNULL != m_pEventSystem );
	return m_pEventSystem;
}

// $deBlob: FUNCTION 00700220
FMOD::System* TSound::GetSystem()
{
	TASSERT( TNULL != m_pSystem );
	return m_pSystem;
}

// $deBlob: FUNCTION 00700260
TBOOL TSound::ErrorCheck( FMOD_RESULT a_eResult )
{
	if ( a_eResult != FMOD_OK )
	{
		TUtil::Log( "TSound::ErrorCheck() - FMOD Error: %s", FMOD_ErrorString( a_eResult ) );
		return TTRUE;
	}

	return TFALSE;
}

TOSHI_NAMESPACE_END
