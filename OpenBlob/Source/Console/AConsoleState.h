#pragma once
#include "AConsoleVar.h"
#include "AConsoleAllocator.h"
#include "GameInterface/AGameState.h"
#include "Input/AInputHelper.h"
#include "Input/AInputHelperSimple.h"

#include <Toshi/TSingleton.h>
#include <Toshi/TString8.h>
#include <ToshiTools/T2DynamicVector.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextBox.h>

//-----------------------------------------------------------------------------
// Purpose: the developer console (an AGameState, singleton): a half transparent rectangle over the screen with
// the typed line and the
// last lines of text, keyboard input (typed characters, backspace, tab to complete, up/down for the history, enter
// executes a command or shows/sets a console variable). Its statics are the lists of console commands and console
// variables the game registers, the console's text and the history of the executed lines.
// NOTE: retail never creates it: nothing pushes it (only its TClass factory references the constructor), so de Blob PC
// has no way to open the console.
//-----------------------------------------------------------------------------
class AConsoleState
    : public AGameState
    , public Toshi::TSingleton<AConsoleState>
{
public:
	TDECLARE_CLASS( AConsoleState, AGameState );

	// A console command: arguments (a_pArgs[ 0 ] is the command name) and their count
	using t_CommandFunc = void( __stdcall* )( Toshi::TString8* a_pArgs, TINT a_iNumArgs );

	struct Command
	{
		Toshi::TString8 strName;   // 0x00
		t_CommandFunc   fnCommand; // 0x0C
	};                             // 0x10

public:
	AConsoleState();
	~AConsoleState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 005b0dc0
	virtual TBOOL        WasControllerRemoved() override { return TTRUE; } // +0x10
	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override;         // +0x18
	virtual void         OnInsertion() override;                           // +0x1C
	virtual void         OnRemoval() override;                             // +0x20
	virtual void         OnActivate() override;                            // +0x2C
	virtual void         OnDeactivate() override;                          // +0x30
	// $deBlob: FUNCTION 005b03d0
	virtual TBOOL CanActivateConsoleState() override { return TFALSE; } // +0x34

	// Adds a console command unless one with the same name (case-insensitive) exists
	static void RegisterCommand( const TCHAR* a_szName, t_CommandFunc a_fnCommand );
	static void UnregisterCommand( t_CommandFunc a_fnCommand );

	// Adds a console variable unless one with the same name (case-insensitive) exists
	static void RegisterCVar( AConsoleVar* a_pVar );
	static void UnregisterCVar( AConsoleVar* a_pVar );

	// Adds a line to the console's text (ms_oLines)
	static void Printf( const TCHAR* a_szFormat, ... );

	// Forgets the console's text and the history
	static void ClearText();

	// Frees the lists (the TClass' DeinitialiseClass)
	static void DeinitialiseStatic();

	// The allocator of the console's strings (TString8's default allocator while the console is active)
	static Toshi::T2Allocator* GetConsoleAllocator();

#if defined( DEBLOB_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
	// NOTE: retail has no opener (see above). The console key (AGameState::OnUpdate) asks for the console and
	// AGameStateController::Update opens it after the current state's update, when that state allows it
	static void RequestOpen() { ms_bOpenRequested = TTRUE; }
	static void OpenIfRequested();

private:
	static TBOOL ms_bOpenRequested;
#endif

private:
	// The a_iIndex-th console variable or command (variables first) whose name starts with a_rPrefix (case-insensitive)
	// or else the last one that does ("" if none); the result is a static buffer
	const TCHAR* AutoComplete( const Toshi::TString8& a_rPrefix, TINT a_iIndex );

	TBOOL StartsWith( const Toshi::TString8& a_rName, const Toshi::TString8& a_rPrefix, TINT a_iPrefixLength );

	// Copies a_iCount characters of a_szSource from a_iStart into a_szDest and terminates it
	void SubString( TCHAR* a_szDest, TINT a_iDestLen, const TCHAR* a_szSource, TINT a_iStart, TINT a_iCount );

	// Splits the line into arguments and runs it: shows/sets a console variable or calls a command
	void ExecuteLine( const Toshi::TString8& a_rLine );

	AConsoleVar* FindCVar( const TCHAR* a_szName );
	Command*     FindCommand( const TCHAR* a_szName );
	void         SetCVar( AConsoleVar* a_pVar, Toshi::T2DynamicVector<Toshi::TString8>& a_rArgs );
	void         PrintCVar( AConsoleVar* a_pVar );

	// Draws the typed line and the console's text (the rectangle's post render callback)
	static void RenderCallback();
	void        Render();

private:
	Toshi::TString8                          m_strInput;           // 0x2C the typed line
	TINT                                     m_iAutoCompleteIndex; // 0x38
	Toshi::TString8                          m_strAutoComplete;    // 0x3C the prefix being completed
	TINT                                     m_iHistoryIndex;      // 0x48
	TFLOAT                                   m_fCursorTime;        // 0x4C the cursor shows while it's over 0.25 s
	Toshi::T2GUIRectangle                    m_oBackground;        // 0x50
	Toshi::T2GUITextBox                      m_oTextBox;           // 0xB0 a line of text (its dimensions)
	TINT                                     m_iNumTextBoxes;      // 0x118 the lines that fit in half the screen
	AInputHelper                             m_oInputHelper;       // 0x11C
	AInputHelperSimple                       m_oKeyboard;          // 0x140 the typed keys
	TBOOL                                    m_bCameraLocked;      // 0x150 the camera's lock before the console
	TBOOL                                    m_bGamePaused;        // 0x151 the pause before the console
	Toshi::TString8::func_DefaultAllocatorCB m_pfnOldAllocatorCB;  // 0x154

	// NOTE: retail allocates 0x400 elements in the static initialisers; the memory isn't up then here, so the first
	// Grow allocates them
	static Toshi::T2DynamicVector<Command>         ms_oCommands;
	static Toshi::T2DynamicVector<Toshi::TString8> ms_oHistory; // the executed lines
	static Toshi::T2DynamicVector<AConsoleVar*>    ms_oVars;
	static Toshi::T2DynamicVector<Toshi::TString8> ms_oLines; // the console's text
};                                                            // 0x158

TSTATICASSERT( sizeof( AConsoleState ) == 0x158 );
