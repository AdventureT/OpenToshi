#pragma once
#include "Toshi/TDebugConsole.h"
#include "Toshi/TString8.h"
#include "TGlobalEmitter.h"

TOSHI_NAMESPACE_START

class TDebugConsole;

enum TApplicationFlag
{
	TApplicationFlag_Created   = BITFLAG( 0 ),
	TApplicationFlag_Destroyed = BITFLAG( 1 ),
};

struct TApplicationExitEvent
{
	TBOOL m_bLockedCursor;
};

//-----------------------------------------------------------------------------
// Purpose: the application's main loop (Create, Execute, Destroy).
// NOTE: in de Blob's branch TApplication finds and shows/hides the console window itself (Barnyard: TDebugConsole)
// and listens to TApplicationExitEvent with a member listener.
//-----------------------------------------------------------------------------
class TApplication
{
public:
	TApplication();
	virtual ~TApplication();

	virtual TBOOL OnCreate( TINT argc, TCHAR** argv );   // +0x04
	virtual TBOOL OnUpdate( TFLOAT deltaTime );          // +0x08
	virtual TBOOL OnDestroy();                           // +0x0C
	// NOTE: emitted in AApplication.cpp (de Blob: AApplication's vftable)
	// $deBlob: FUNCTION 0056c3b0
	virtual TBOOL AllowedToHideConsole() { return TTRUE; } // +0x10

	// $deBlob: FUNCTION 006ef3b0
	static void OnApplicationExitEvent( void* a_pApp, const TApplicationExitEvent& a_rEvent )
	{
		TSTATICCAST( TApplication, a_pApp )->Destroy();
	}

	// Returns TTRUE if success
	TBOOL Create( const TCHAR* appName, TINT argc, TCHAR** argv );

	// Sets the destroyed flag: Execute stops after the current frame
	void Destroy();

	// Returns TTRUE if the application is created
	TBOOL IsCreated() const;

	// Returns TTRUE if the application is destroyed
	TBOOL IsDestroyed() const { return m_Flags & TApplicationFlag_Destroyed; }

	// Sets new state
	void SetVerbose( TBOOL state ) { m_bVerbose = state; }

	// Executes the application and starts the loop
	TBOOL Execute();

	// Shows or hides the console window, returns the previous state
	TBOOL ShowConsole( TBOOL a_bShow );

	// Toggles the console window, returns the new state
	TBOOL ToggleConsole();

	void DisplayErrorMessageBox( TString8 a_strMessage );

private:
	static HWND FindConsole();

private:
	TString8                                             m_Name;          // 0x04
	TUINT32                                              m_Flags;         // 0x10
	TBOOL                                                m_bVerbose;      // 0x14
	TBOOL                                                m_bShowConsole;  // 0x15
	HWND                                                 m_hConsole;      // 0x18
	TGlobalListener<TApplication, TApplicationExitEvent> m_oExitListener; // 0x1C

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: not in retail, mirrors the log to the console
	TDebugConsole* m_pDebugConsole = TNULL;
#endif
}; // 0x2C

TOSHI_NAMESPACE_END
