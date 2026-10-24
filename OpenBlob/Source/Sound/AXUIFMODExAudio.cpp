#include "pch.h"
#include "AXUIFMODExAudio.h"
#include "Options/AOptions.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00617f70
AXUIFMODExAudio::AXUIFMODExAudio()
{
	m_iNumSubSounds = 0;
	m_pSound        = TNULL;
}

// $deBlob: FUNCTION 00617f90
// $deBlob: FUNCTION 006182d0
AXUIFMODExAudio::~AXUIFMODExAudio()
{
	if ( m_pSound != TNULL )
	{
		m_pSound->release();
		m_pSound = TNULL;
	}
}

// $deBlob: FUNCTION 00618000
TBOOL AXUIFMODExAudio::Create( const TCHAR* a_szSoundBankName )
{
	if ( TSound::GetSingleton() == TNULL || a_szSoundBankName == TNULL )
		return TFALSE;

	Release();

	if ( TSound::ErrorCheck( TSound::GetSingleton()->GetSystem()->createSound( a_szSoundBankName, FMOD_DEFAULT, TNULL, &m_pSound ) ) )
		return TFALSE;

	return !TSound::ErrorCheck( m_pSound->getNumSubSounds( &m_iNumSubSounds ) );
}

// $deBlob: FUNCTION 00618070
TBOOL AXUIFMODExAudio::Release()
{
	if ( m_pSound != TNULL )
	{
		FMOD_RESULT eResult = m_pSound->release();
		TASSERT( eResult == FMOD_OK );

		if ( eResult == FMOD_OK )
		{
			m_pSound = TNULL;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006180d0
void* AXUIFMODExAudio::GetSubSound( const TCHAR* a_szSubSoundName )
{
	if ( TSound::GetSingleton() == TNULL )
		return TNULL;

	if ( m_pSound != TNULL )
	{
		for ( TINT i = 0; i < m_iNumSubSounds; i++ )
		{
			FMOD::Sound* pSubSound;

			if ( TSound::ErrorCheck( m_pSound->getSubSound( i, &pSubSound ) ) )
				break;

			TCHAR       szName[ 256 ];
			FMOD_RESULT result = pSubSound->getName( szName, sizeof( szName ) );
			TASSERT( !TSound::ErrorCheck( result ) );

			if ( TStringManager::String8Compare( a_szSubSoundName, szName ) == 0 )
				return pSubSound;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 006181d0
TBOOL AXUIFMODExAudio::PlaySound( void* a_pSound, TFLOAT a_fVolume, void** a_ppOutChannel )
{
	if ( TSound::GetSingleton() == TNULL )
		return TFALSE;

	FMOD::Channel** ppChannel = TREINTERPRETCAST( FMOD::Channel**, a_ppOutChannel );

	if ( TSound::ErrorCheck( TSound::GetSingleton()->GetSystem()->playSound( FMOD_CHANNEL_FREE, TSTATICCAST( FMOD::Sound, a_pSound ), true, ppChannel ) ) )
		return TFALSE;

	if ( *ppChannel == TNULL )
		return TTRUE;

	if ( TSound::ErrorCheck( ( *ppChannel )->setVolume( AOptions::GetSingleton()->GetSFXVolume() * a_fVolume ) ) )
		return TFALSE;

	return !TSound::ErrorCheck( ( *ppChannel )->setPaused( false ) );
}

// $deBlob: FUNCTION 00618260
void AXUIFMODExAudio::Stop( void* a_pChannel )
{
	if ( a_pChannel != TNULL )
		TSTATICCAST( FMOD::Channel, a_pChannel )->stop();
}

// $deBlob: FUNCTION 00618280
void AXUIFMODExAudio::SetVolume( void* a_pChannel, TFLOAT a_fVolume )
{
	if ( a_pChannel != TNULL )
		TSTATICCAST( FMOD::Channel, a_pChannel )->setVolume( AOptions::GetSingleton()->GetSFXVolume() * a_fVolume );
}
