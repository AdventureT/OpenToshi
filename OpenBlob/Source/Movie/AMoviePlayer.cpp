#include "pch.h"
#include "AMoviePlayer.h"

#ifdef DEBLOB_COMMUNITY_PATCH
#include <File/TFile.h>
#endif // DEBLOB_COMMUNITY_PATCH

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005e7320
AMoviePlayer::AMoviePlayer()
    : m_Emitter( this )
{
	m_bInitialised  = TFALSE;
	m_bIsFullscreen = TTRUE;
	m_bIsMuted      = TFALSE;
	m_bIsHidden     = TTRUE;
}

// $deBlob: FUNCTION 005e73c0
// $deBlob: FUNCTION 005e7620
AMoviePlayer::~AMoviePlayer()
{
}

// $deBlob: FUNCTION 005e7460
TBOOL AMoviePlayer::OnUpdate( TFLOAT a_fDeltaTime )
{
	return TTRUE;
}

// $deBlob: FUNCTION 005e7470
void AMoviePlayer::OnCreate()
{
	TASSERT( TFALSE == m_bInitialised );
	m_bInitialised = TTRUE;
}

// $deBlob: FUNCTION 005e74b0
void AMoviePlayer::OnDestroy()
{
	TASSERT( TTRUE == m_bInitialised );
	m_bInitialised = TFALSE;
}

// $deBlob: FUNCTION 005e74f0
void AMoviePlayer::OnActivate()
{
}

// $deBlob: FUNCTION 005e7500
void AMoviePlayer::OnDeactivate()
{
}

// $deBlob: FUNCTION 005e7510
void AMoviePlayer::ThrowEvent( AMovieEvent::Type a_eEvent )
{
	m_Emitter.Throw( &a_eEvent );
}

#ifdef DEBLOB_COMMUNITY_PATCH
// NOTE: retail opens the movies with fopen relative to the working directory, which is the game folder. With
// -gamedir (dev runs from another folder) a file that isn't found there is opened read-only from the game folder
FILE* AMoviePlayer::OpenFile( const TCHAR* a_szFileName )
{
	FILE* pFile = fopen( a_szFileName, "rb" );

	if ( pFile == TNULL && TFileManager::GetSingleton() != TNULL )
	{
		TFileSystem* pGameDir = TFileManager::GetSingleton()->FindFileSystem( "gamedir" );

		if ( pGameDir != TNULL )
		{
			TString8 strPath = pGameDir->GetPrefix();

			if ( strPath.Length() != 0 && strPath[ strPath.Length() - 1 ] != '\\' && strPath[ strPath.Length() - 1 ] != '/' )
				strPath += "\\";

			strPath += a_szFileName;
			pFile = fopen( strPath.GetString(), "rb" );
		}
	}

	return pFile;
}
#endif // DEBLOB_COMMUNITY_PATCH
