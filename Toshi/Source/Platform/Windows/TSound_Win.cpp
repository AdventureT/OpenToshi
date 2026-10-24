#include "ToshiPCH.h"
#include "Sound/TSound.h"

#ifdef DEBLOB_COMMUNITY_PATCH
#  include "File/TFile.h"
#  include <windows.h>
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00700830
TBOOL TSound::InitialiseMemory( void* a_pPoolMem, TINT a_iPoolSize )
{
	if ( a_pPoolMem == TNULL )
	{
		if ( a_iPoolSize < 0 )
			a_iPoolSize = 0x400000;

		a_pPoolMem = malloc( a_iPoolSize );
	}

	if ( ErrorCheck( FMOD::Memory_Initialize( a_pPoolMem, a_iPoolSize, TNULL, TNULL, TNULL ) ) == TTRUE )
	{
		TUtil::Log( "FMOD: Failed to Initialise Memory" );
		return TFALSE;
	}

	return TTRUE;
}

// The device loss callback of the FMOD system
// $deBlob: FUNCTION 00700a60
static FMOD_RESULT F_CALLBACK SoundCB( FMOD_SYSTEM* a_pSystem, FMOD_SYSTEM_CALLBACKTYPE a_eType, void* a_pCommandData1, void* a_pCommandData2 )
{
	if ( TUINT32( a_eType ) <= FMOD_SYSTEM_CALLBACKTYPE_DEVICELOST )
	{
		TUtil::Log( "FMOD reported a device loss. Setting system as uninitialized" );
		TSound::GetSingleton()->SetInitialised( TFALSE );
	}

	return FMOD_OK;
}

// $deBlob: FUNCTION 00700890
TBOOL TSound::Initialise( TINT a_iMaxChannels, SpeakerType a_eSpeakerType )
{
	TASSERT( !m_bInitialised );

	if ( a_eSpeakerType == SpeakerType_Mono )
	{
		GetSystem()->setSpeakerMode( FMOD_SPEAKERMODE_MONO );
	}
	else if ( a_eSpeakerType == SpeakerType_Stereo )
	{
		GetSystem()->setSpeakerMode( FMOD_SPEAKERMODE_STEREO );
	}
	else if ( a_eSpeakerType == SpeakerType_Driver )
	{
		FMOD_SPEAKERMODE eSpeakerMode;
		GetSystem()->getDriverCaps( 0, TNULL, TNULL, &eSpeakerMode );
		GetSystem()->setSpeakerMode( eSpeakerMode );

		TINT        iNumDrivers = 0;
		FMOD_RESULT eResult     = m_pSystem->getNumDrivers( &iNumDrivers );
		TASSERT( !ErrorCheck( eResult ) );

		if ( iNumDrivers < 1 )
		{
			TUtil::Log( "No soundcard, initializing to FMOD_OUTPUTTYPE_NOSOUND " );
			m_pSystem->setOutput( FMOD_OUTPUTTYPE_NOSOUND );
		}
	}

#ifdef DEBLOB_COMMUNITY_PATCH
	if ( IsMuteMode() )
	{
		TUtil::Log( "TSound::Initialise() - -mute/-background: FMOD_OUTPUTTYPE_NOSOUND\n" );
		m_pSystem->setOutput( FMOD_OUTPUTTYPE_NOSOUND );
	}
#endif

	if ( a_iMaxChannels < 0 )
		a_iMaxChannels = 256;

	GetSystem()->setCallback( SoundCB );

	TASSERT( TNULL != m_pEventSystem );
	FMOD_RESULT eResult = m_pEventSystem->init( a_iMaxChannels, FMOD_INIT_3D_RIGHTHANDED, TNULL, FMOD_EVENT_INIT_NORMAL );

	if ( ErrorCheck( eResult ) == TTRUE )
	{
		TUtil::Log( "TSound::Initialise() - Failed to initialise FMOD Event System with error code %d", eResult );
		return TFALSE;
	}

	m_bInitialised = TTRUE;
	return TTRUE;
}

// $deBlob: FUNCTION 00700a20
TBOOL TSound::InitialiseFileSystem()
{
	return TTRUE;
}

// $deBlob: FUNCTION 00700a30
TBOOL TSound::InitialiseNetwork()
{
	return TTRUE;
}

// $deBlob: FUNCTION 00700a40
TBOOL TSound::UpdateNetwork()
{
	return TTRUE;
}

// $deBlob: FUNCTION 00700a50
TBOOL TSound::DestroyNetwork()
{
	return TTRUE;
}

// $deBlob: FUNCTION 00700aa0
void TSound::SetInitialised( TBOOL a_bInitialised )
{
	m_bInitialised = a_bInitialised;
}

#ifdef DEBLOB_COMMUNITY_PATCH

TBOOL TSound::IsMuteMode()
{
	static const TBOOL s_bMute =
	    ( T2String8::FindString( GetCommandLineA(), "-mute" ) || T2String8::FindString( GetCommandLineA(), "-background" ) ) &&
	    !T2String8::FindString( GetCommandLineA(), "-devaudio" );

	return s_bMute;
}

//-----------------------------------------------------------------------------
// The FMOD file callbacks of -gamedir: every FMOD file goes through TFile, so the
// "gamedir" file system of TFileManager is used as the fallback of "local".
//-----------------------------------------------------------------------------
static FMOD_RESULT F_CALLBACK TFileOpenCB( const char* a_szName, int a_iUnicode, unsigned int* a_pFileSize, void** a_ppHandle, void** a_ppUserData )
{
	if ( a_szName == TNULL || a_iUnicode != 0 )
		return FMOD_ERR_FILE_NOTFOUND;

	TFile* pFile = TFile::Create( a_szName, TFILEMODE_READ );

	if ( pFile == TNULL )
		return FMOD_ERR_FILE_NOTFOUND;

	*a_pFileSize  = TUINT( pFile->GetSize() );
	*a_ppHandle   = pFile;
	*a_ppUserData = TNULL;

	return FMOD_OK;
}

static FMOD_RESULT F_CALLBACK TFileCloseCB( void* a_pHandle, void* a_pUserData )
{
	if ( a_pHandle == TNULL )
		return FMOD_ERR_INVALID_PARAM;

	TFile::Destroy( TSTATICCAST( TFile, a_pHandle ) );
	return FMOD_OK;
}

static FMOD_RESULT F_CALLBACK TFileReadCB( void* a_pHandle, void* a_pBuffer, unsigned int a_uiSizeBytes, unsigned int* a_pBytesRead, void* a_pUserData )
{
	if ( a_pHandle == TNULL )
		return FMOD_ERR_INVALID_PARAM;

	*a_pBytesRead = TUINT( TSTATICCAST( TFile, a_pHandle )->Read( a_pBuffer, a_uiSizeBytes ) );

	return ( *a_pBytesRead < a_uiSizeBytes ) ? FMOD_ERR_FILE_EOF : FMOD_OK;
}

static FMOD_RESULT F_CALLBACK TFileSeekCB( void* a_pHandle, unsigned int a_uiPos, void* a_pUserData )
{
	if ( a_pHandle == TNULL )
		return FMOD_ERR_INVALID_PARAM;

	return TSTATICCAST( TFile, a_pHandle )->Seek( TINT( a_uiPos ), TSEEK_SET ) ? FMOD_OK : FMOD_ERR_FILE_COULDNOTSEEK;
}

FMOD_RESULT TSound::SetTFileSystem( TINT a_iBlockAlign )
{
	return GetSystem()->setFileSystem( TFileOpenCB, TFileCloseCB, TFileReadCB, TFileSeekCB, TNULL, TNULL, a_iBlockAlign );
}

#endif // DEBLOB_COMMUNITY_PATCH

TOSHI_NAMESPACE_END
