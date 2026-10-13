#include "ToshiPCH.h"
#include "TApplication.h"
#include "Toshi/Typedefs.h"
#include "Toshi/TString8.h"
#include "Toshi/TSystem.h"
#include "File/TFile.h"

#include <mmsystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006ef060
// $Barnyard: FUNCTION 006c16b0
TApplication::TApplication()
{
	m_Flags        = 0;
	m_bVerbose     = TTRUE;
	m_bShowConsole = TTRUE;
	m_hConsole     = NULL;
}

// $deBlob: FUNCTION 006ef0b0
// $deBlob: FUNCTION 006ef560
// $Barnyard: FUNCTION 006c17c0
// $Barnyard: FUNCTION 006c16e0
TApplication::~TApplication()
{
	// NOTE: retail doesn't call Destroy here, the listener disconnects itself
	m_oExitListener.Disconnect();

#ifdef DEBLOB_COMMUNITY_PATCH
	delete m_pDebugConsole;
#endif
}

// $deBlob: FUNCTION 006ef100
// $Barnyard: FUNCTION 006c16f0
TBOOL TApplication::Execute()
{
	TASSERT( TTRUE == TApplication::IsCreated() );

	TSystemManager::GetSingleton()->Update();

	while ( OnUpdate( TSystemManager::GetSingleton()->GetTimer().GetDelta() ) )
	{
		if ( IsDestroyed() )
			break;

		TSystemManager::GetSingleton()->Update();
	}

	return OnDestroy();
}

// $deBlob: FUNCTION 006ef1a0
// $Barnyard: FUNCTION 006c17f0
TBOOL TApplication::Create( const TCHAR* appName, TINT argc, TCHAR** argv )
{
	m_hConsole = FindConsole();

	if ( AllowedToHideConsole() )
	{
		ShowWindow( m_hConsole, SW_HIDE );
		UpdateWindow( m_hConsole );
	}

	m_bShowConsole = TFALSE;
	m_oExitListener.Connect( this, OnApplicationExitEvent );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: Barnyard's console log mirror, kept for development
	m_pDebugConsole = new TDebugConsole;
#endif

	m_Name = appName;

	return OnCreate( argc, argv );
}

// $deBlob: FUNCTION 006ef210
// $Barnyard: FUNCTION 006c1760
void TApplication::Destroy()
{
	m_Flags |= TApplicationFlag_Destroyed;
}

// $deBlob: FUNCTION 006ef220
TBOOL TApplication::IsCreated() const
{
	return m_Flags & TApplicationFlag_Created;
}

// $deBlob: FUNCTION 006ef230
// $Barnyard: FUNCTION 006c1770
TBOOL TApplication::OnCreate( TINT argc, TCHAR** argv )
{
	m_Flags |= TApplicationFlag_Created;
	return TTRUE;
}

// $deBlob: FUNCTION 006ef240
// $Barnyard: FUNCTION 006c1780
TBOOL TApplication::OnUpdate( TFLOAT deltaTime )
{
	return ( m_Flags & TApplicationFlag_Destroyed ) == 0;
}

// $deBlob: FUNCTION 006ef250
// $Barnyard: FUNCTION 006c1790
TBOOL TApplication::OnDestroy()
{
	m_Name = TString8();
	m_oExitListener.Disconnect();

	return TTRUE;
}

// $deBlob: FUNCTION 006ef2b0
TBOOL TApplication::ShowConsole( TBOOL a_bShow )
{
	TBOOL bOldState = m_bShowConsole;

	if ( a_bShow || AllowedToHideConsole() )
	{
		ShowWindow( m_hConsole, a_bShow == TTRUE );
		UpdateWindow( m_hConsole );
	}

	m_bShowConsole = a_bShow;
	return bOldState;
}

// $deBlob: FUNCTION 006ef300
TBOOL TApplication::ToggleConsole()
{
	TBOOL bShow = m_bShowConsole != TTRUE;

	if ( bShow || AllowedToHideConsole() )
	{
		ShowWindow( m_hConsole, bShow );
		UpdateWindow( m_hConsole );
	}

	m_bShowConsole = bShow;
	return bShow;
}

// $deBlob: FUNCTION 006ef340
void TApplication::DisplayErrorMessageBox( TString8 a_strMessage )
{
	MessageBoxA( m_hConsole, a_strMessage.GetString( 0 ), "Error", MB_OK );
}

// $deBlob: FUNCTION 006ef3c0
// Finds the console window: gives it a unique title for a moment
HWND TApplication::FindConsole()
{
	TCHAR szTimeHex[ 20 ];
	_ultoa( timeGetTime(), szTimeHex, 16 );

	TCHAR szUniqueTitle[ 200 ];
	strcpy( szUniqueTitle, "{1B5D8052-529A-4c88-9DDE-EE523D440B00}" );
	strcat( szUniqueTitle, szTimeHex );

	TCHAR szOldTitle[ 512 ];

	if ( GetConsoleTitleA( szOldTitle, sizeof( szOldTitle ) ) )
	{
		SetConsoleTitleA( szUniqueTitle );
		HWND hWindow = FindWindowA( NULL, szUniqueTitle );
		SetConsoleTitleA( szOldTitle );

		return hWindow;
	}

	return NULL;
}

TOSHI_NAMESPACE_END
