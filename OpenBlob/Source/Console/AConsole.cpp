#include "pch.h"
#include "AConsole.h"
#include "AConsoleState.h"
#include "Memory/AMemory.h"
#include "GameInterface/APlayerManager.h"
#include "GameController/AGameStatus.h"
#include "Challenges/AChallengeManager.h"
#include "Challenges/AChallenge.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"

#include <Toshi/TStringManager.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: the handlers are never called in retail (the console can't be opened)

// Toggled by phys_show_collision, never read
static TBOOL s_bPhysShowCollision = TFALSE;

// Prints to the console
// NOTE: passes the formatted scratch string as the format of AConsoleState::Printf (see there)
// $deBlob: FUNCTION 005ad030
static void Console_Printf( const TCHAR* a_szFormat, ... )
{
	va_list args;
	va_start( args, a_szFormat );
	vsprintf( TStringManager::GetTempString8(), a_szFormat, args );
	AConsoleState::Printf( TStringManager::GetTempString8(), args );
	va_end( args );
}

// The blob of the player whose number is the second character of the command's name (pN_...), or TNULL
static ABlobEntity* GetCommandBlob( TString8* a_pArgs )
{
	TINT iPlayer = a_pArgs[ 0 ][ 1 ] - '1';

	if ( iPlayer < 0 || iPlayer >= APlayerManager::GetSingleton()->GetNumPlayers() )
		return TNULL;

	return TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( iPlayer ) ) );
}

// pN_colour [colour]: prints or sets the colour of player N's blob (0: none)
// $deBlob: FUNCTION 005acbe0
static void __stdcall Command_Colour( TString8* a_pArgs, TINT a_iNumArgs )
{
	ABlobEntity* pBlob = GetCommandBlob( a_pArgs );

	if ( pBlob == TNULL )
		return;

	if ( a_iNumArgs < 2 )
	{
		TUINT32 uiColour = pBlob->GetColourComponent()->GetCurrentColour();

		if ( uiColour & 0x10 )
			uiColour = 0x10;

		Console_Printf( "%d\n", uiColour );
		return;
	}

	TINT iColour = atoi( a_pArgs[ 1 ].GetString() );
	TMath::Clip( iColour, 0, 0x10 );

	if ( iColour == 0 )
		pBlob->WashOff();
	else
		pBlob->ChangeColour( iColour );
}

// pN_mp_colour [colour]: prints or sets the multiplayer colour of player N
// $deBlob: FUNCTION 005acc90
static void __stdcall Command_MPColour( TString8* a_pArgs, TINT a_iNumArgs )
{
	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
	{
		Console_Printf( "Not multiplayer!\n" );
		return;
	}

	ABlobEntity* pBlob = GetCommandBlob( a_pArgs );

	if ( pBlob == TNULL )
		return;

	if ( a_iNumArgs < 2 )
	{
		Console_Printf( "%d\n", pBlob->GetColourComponent()->GetStartColour() );
		return;
	}

	TINT iColour = atoi( a_pArgs[ 1 ].GetString() );
	TMath::Clip( iColour, 0, 0x10 );

	TUINT32 uiCurrentColour = pBlob->GetColourComponent()->GetCurrentColour();

	if ( !( uiCurrentColour & 0x10 ) && uiCurrentColour != 0 )
		pBlob->GetColourComponent()->SetColour( iColour );

	pBlob->GetColourComponent()->SetStartColour( iColour );
	pBlob->GetColourComponent()->SetUnk20( iColour );
}

// pN_points [points]: prints or sets the points of player N's blob
// $deBlob: FUNCTION 005acd70
static void __stdcall Command_Points( TString8* a_pArgs, TINT a_iNumArgs )
{
	ABlobEntity* pBlob = GetCommandBlob( a_pArgs );

	if ( pBlob == TNULL )
		return;

	if ( a_iNumArgs < 2 )
	{
		Console_Printf( "%d\n", pBlob->GetPaintPoints() );
		return;
	}

	TINT iPoints = atoi( a_pArgs[ 1 ].GetString() );
	TMath::Clip( iPoints, 0, 100 );
	pBlob->SetPaintPoints( iPoints );
}

// pN_energy [energy]: prints or sets the energy of player N
// $deBlob: FUNCTION 005ace00
static void __stdcall Command_Energy( TString8* a_pArgs, TINT a_iNumArgs )
{
	TINT iPlayer = a_pArgs[ 0 ][ 1 ] - '1';

	if ( iPlayer < 0 || iPlayer >= APlayerManager::GetSingleton()->GetNumPlayers() )
		return;

	if ( APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( iPlayer ) ) == TNULL )
		return;

	AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();

	if ( a_iNumArgs < 2 )
	{
		Console_Printf( "%d\n", rLevelData.GetPoints( iPlayer ) );
		return;
	}

	TINT iEnergy = atoi( a_pArgs[ 1 ].GetString() );
	rLevelData.AddScore( iPlayer, iEnergy - rLevelData.GetPoints( iPlayer ), -1 );
}

// lvl_timeleft [seconds]: prints or sets the level's time left
// $deBlob: FUNCTION 005acea0
static void __stdcall Command_LevelTimeLeft( TString8* a_pArgs, TINT a_iNumArgs )
{
	if ( a_iNumArgs < 2 )
	{
		Console_Printf( "%f seconds\n", AGameStatus::GetSingleton()->GetTimeLeft() );
		return;
	}

	AGameStatus::GetSingleton()->SetTimeLeft( TFLOAT( atof( a_pArgs[ 1 ].GetString() ) ) );
}

// chal_timeleft [seconds]: prints or sets the current challenge's time left
// $deBlob: FUNCTION 005acf30
static void __stdcall Command_ChallengeTimeLeft( TString8* a_pArgs, TINT a_iNumArgs )
{
	AChallenge* pChallenge;

	if ( AChallengeManager::GetSingleton() != TNULL &&
	     ( pChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge() ) != TNULL )
	{
		if ( a_iNumArgs < 2 )
		{
			Console_Printf( "%f seconds\n", pChallenge->GetTimeLeft() );
			return;
		}

		pChallenge->SetTimeLeft( TFLOAT( atof( a_pArgs[ 1 ].GetString() ) ) );
		return;
	}

	Console_Printf( "No challenge active.\n" );
}

// phys_show_collision: toggles the collision display flag
// $deBlob: FUNCTION 005acfa0
static void __stdcall Command_PhysShowCollision( TString8* a_pArgs, TINT a_iNumArgs )
{
	s_bPhysShowCollision = !s_bPhysShowCollision;
}

// $deBlob: FUNCTION 005ad0a0
void AConsole::RegisterCommands()
{
	AConsoleState::RegisterCommand( "mem_show_stats", AMemory::Console_DumpMemoryStats );
	AConsoleState::RegisterCommand( "mem_show_stats_simple", AMemory::Console_DumpMemoryStatsSimple );
	AConsoleState::RegisterCommand( "mem_mark_point", AMemory::Console_MarkMemoryPoint );
	AConsoleState::RegisterCommand( "mem_show_point_delta", AMemory::Console_ShowMemoryPointDelta );
	AConsoleState::RegisterCommand( "p1_colour", Command_Colour );
	AConsoleState::RegisterCommand( "p2_colour", Command_Colour );
	AConsoleState::RegisterCommand( "p3_colour", Command_Colour );
	AConsoleState::RegisterCommand( "p4_colour", Command_Colour );
	AConsoleState::RegisterCommand( "p1_mp_colour", Command_MPColour );
	AConsoleState::RegisterCommand( "p2_mp_colour", Command_MPColour );
	AConsoleState::RegisterCommand( "p3_mp_colour", Command_MPColour );
	AConsoleState::RegisterCommand( "p4_mp_colour", Command_MPColour );
	AConsoleState::RegisterCommand( "p1_points", Command_Points );
	AConsoleState::RegisterCommand( "p2_points", Command_Points );
	AConsoleState::RegisterCommand( "p3_points", Command_Points );
	AConsoleState::RegisterCommand( "p4_points", Command_Points );
	AConsoleState::RegisterCommand( "p1_energy", Command_Energy );
	AConsoleState::RegisterCommand( "p2_energy", Command_Energy );
	AConsoleState::RegisterCommand( "p3_energy", Command_Energy );
	AConsoleState::RegisterCommand( "p4_energy", Command_Energy );
	AConsoleState::RegisterCommand( "lvl_timeleft", Command_LevelTimeLeft );
	AConsoleState::RegisterCommand( "chal_timeleft", Command_ChallengeTimeLeft );
	AConsoleState::RegisterCommand( "phys_show_collision", Command_PhysShowCollision );
}

// $deBlob: FUNCTION 005ad200
void AConsole::UnregisterCommands()
{
	AConsoleState::UnregisterCommand( AMemory::Console_DumpMemoryStats );
	AConsoleState::UnregisterCommand( AMemory::Console_DumpMemoryStatsSimple );
	AConsoleState::UnregisterCommand( AMemory::Console_MarkMemoryPoint );
	AConsoleState::UnregisterCommand( AMemory::Console_ShowMemoryPointDelta );
	AConsoleState::UnregisterCommand( Command_Colour );
	AConsoleState::UnregisterCommand( Command_Colour );
	AConsoleState::UnregisterCommand( Command_Colour );
	AConsoleState::UnregisterCommand( Command_Colour );
	AConsoleState::UnregisterCommand( Command_MPColour );
	AConsoleState::UnregisterCommand( Command_MPColour );
	AConsoleState::UnregisterCommand( Command_MPColour );
	AConsoleState::UnregisterCommand( Command_MPColour );
	AConsoleState::UnregisterCommand( Command_Points );
	AConsoleState::UnregisterCommand( Command_Points );
	AConsoleState::UnregisterCommand( Command_Points );
	AConsoleState::UnregisterCommand( Command_Points );
	AConsoleState::UnregisterCommand( Command_Energy );
	AConsoleState::UnregisterCommand( Command_Energy );
	AConsoleState::UnregisterCommand( Command_Energy );
	AConsoleState::UnregisterCommand( Command_Energy );
	AConsoleState::UnregisterCommand( Command_LevelTimeLeft );
	AConsoleState::UnregisterCommand( Command_ChallengeTimeLeft );
	AConsoleState::UnregisterCommand( Command_PhysShowCollision );
}
