#include "pch.h"
#include "AConsoleState.h"
#include "AStaticCVars.h"
#include "AApplication.h"
#include "Cameras/ACameraManager.h"
#include "GUI/AFontManager.h"
#include "GUI/AGUIManager.h"
#include "Input/AInputManager2.h"
#include "GameInterface/AGameStateController.h"
#include "Memory/AMemory.h"

#include <GUI/T2GUI.h>
#include <Input/TInputInterface.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0053bbf0
// $deBlob: FUNCTION 00858080
Toshi::T2DynamicVector<AConsoleState::Command> AConsoleState::ms_oCommands( Toshi::GetGlobalAllocator(), 0, 0x400 );
// $deBlob: FUNCTION 0053bc20
// $deBlob: FUNCTION 00858090
Toshi::T2DynamicVector<Toshi::TString8> AConsoleState::ms_oHistory( Toshi::GetGlobalAllocator(), 0, 0x400 );
// $deBlob: FUNCTION 00857fe0
// $deBlob: FUNCTION 0053bbc0
Toshi::T2DynamicVector<AConsoleVar*> AConsoleState::ms_oVars( Toshi::GetGlobalAllocator(), 0, 0x400 );
// $deBlob: FUNCTION 0053bc50
// $deBlob: FUNCTION 008580a0
Toshi::T2DynamicVector<Toshi::TString8> AConsoleState::ms_oLines( Toshi::GetGlobalAllocator(), 0, 0x400 );

// $deBlob: FUNCTION 005ad2f0
// $deBlob: FUNCTION 005ad300
// $deBlob: FUNCTION 005ad370
// $deBlob: FUNCTION 0053bb80
// $deBlob: FUNCTION 00857fd0
TDEFINE_CLASS_CORE( AConsoleState );
Toshi::TClass AConsoleState::TClassObjectName = Toshi::TClass( "AConsoleState", AConsoleState::GetParentClassStatic(), AConsoleState::CreateTObject, AConsoleState::CreateTObjectInPlace, AConsoleState::InitialiseClass, AConsoleState::DeinitialiseClass, 0, 1, sizeof( AConsoleState ), TALIGNOF( AConsoleState ) );

#if defined( DEBLOB_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )

TBOOL AConsoleState::ms_bOpenRequested = TFALSE;

void AConsoleState::OpenIfRequested()
{
	if ( !ms_bOpenRequested )
		return;

	ms_bOpenRequested = TFALSE;

	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( !pController->GetCurrentGameState()->CanActivateConsoleState() || pController->FindState( &TGetClass( AConsoleState ) ) )
		return;

	// The console locks the current camera and draws with the message font
	if ( !ACameraManager::GetSingleton() || !ACameraManager::GetSingleton()->GetCurrentCamera() || !AFontManager::sm_pMsgFont )
		return;

	pController->PushState( new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) AConsoleState() );
}

#endif

// $deBlob: FUNCTION 005ad3d0
void AConsoleState::InitialiseClass()
{
}

// $deBlob: FUNCTION 005ad3e0
void AConsoleState::DeinitialiseClass()
{
	DeinitialiseStatic();
}

// $deBlob: FUNCTION 005ad3f0
AConsoleState::AConsoleState()
    : m_strInput( &AConsoleAllocator::s_Instance )
    , m_strAutoComplete( &AConsoleAllocator::s_Instance )
    , m_oKeyboard( 128, TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 ) )
{
	m_iAutoCompleteIndex = 0;
	m_iHistoryIndex      = ms_oHistory.Size();
	m_fCursorTime        = 0.0f;

	// Every key of the keyboard can be typed
	for ( TINT i = TInputDeviceKeyboard::VIRTKEY; i < TInputDeviceKeyboard::DOODAD_END; i++ )
		m_oKeyboard.AddDoodad( i );

	// The background covers 95% of the screen's height
	T2GUIElement* pRoot = T2GUI::GetSingleton()->GetRootElement();

	TFLOAT fWidth, fHeight;
	pRoot->GetDimensions( fWidth, fHeight );
	fHeight *= 0.95f;

	m_oBackground.SetDimensions( fWidth, fHeight );
#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: the PC's GUI projection (A2GUIRenderer) has Y up while T2GUI's attachments and fonts assume Y down, so
	// retail would draw the console upside down at the bottom: the background flips Y for itself and its contents and
	// is attached by its (flipped) top left corner to the screen's top left corner
	m_oBackground.GetTransform().PreMultiply( 1.0f, -1.0f );
	m_oBackground.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
#else
	m_oBackground.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
#endif
	m_oBackground.SetColour( 0x7F000000 );
	pRoot->AddChildTail( &m_oBackground );
	m_oBackground.SetPostRenderCallback( RenderCallback );

	// The text uses half the safe area's height, a line per text box
	TFLOAT fSafeWidth, fSafeHeight;
	AGUIManager::GetSingleton()->GetSafeArea()->GetDimensions( fSafeWidth, fSafeHeight );

	T2GUIFont* pFont       = AFontManager::sm_pMsgFont;
	TFLOAT     fLineHeight = pFont->GetHeight() + pFont->GetLineSpacing();

	m_iNumTextBoxes = TMath::FloorToInt( fSafeHeight * 0.5f / fLineHeight );
	TASSERT( 1 < m_iNumTextBoxes );

	m_oTextBox.SetDimensions( fSafeWidth, fLineHeight, TTRUE );
	m_oTextBox.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_oTextBox.SetJustify( T2GUIFont::JUSTIFY_LEFT );
	m_oTextBox.SetFont( pFont );
	m_oTextBox.SetColour( 0xFFFFFF00 );
	m_oBackground.AddChildTail( &m_oTextBox );
}

// $deBlob: FUNCTION 005ad680
// $deBlob: FUNCTION 005b0300
AConsoleState::~AConsoleState()
{
	m_oTextBox.RemoveSelf();
	m_oBackground.RemoveSelf();
}

// NOTE: retail's callback loads the singleton and jumps into the member
// $deBlob: FUNCTION 005ad7c0
void AConsoleState::RenderCallback()
{
	GetSingleton()->Render();
}

void AConsoleState::Render()
{
	TFLOAT fX = m_oTextBox.GetTransform().GetPos().x;
	TFLOAT fY = m_oTextBox.GetTransform().GetPos().y;

	TFLOAT fWidth, fHeight;
	m_oTextBox.GetDimensions( fWidth, fHeight );

	T2GUIFont* pFont = AFontManager::sm_pMsgFont;

	fX -= fWidth * 0.5f;
	fY -= fHeight * 0.5f;

	m_oTextBox.PreRender();

	// The typed line, with the blinking cursor
	TCHAR  szLine[ 0x400 ];
	TWCHAR wszLine[ 0x400 ];

	if ( m_fCursorTime > 0.25f )
		TStringManager::String8Format( szLine, 0x400, ">%s|", m_strInput.GetString() );
	else
		TStringManager::String8Format( szLine, 0x400, ">%s", m_strInput.GetString() );

	TStringManager::StringCharToUnicode( wszLine, szLine, 0x400 );
	pFont->DrawTextBox( wszLine, fX, fY, fWidth, fHeight, 0xFFFFFF00, 1.0f, T2GUIFont::JUSTIFY_LEFT, -1, TTRUE, 0, TFALSE, TNULL, TNULL, TNULL );

	// The last lines of text above it
	TINT iTextBox = 1;

	for ( TINT i = ms_oLines.Size() - 1; i >= 0; i-- )
	{
		if ( iTextBox >= m_iNumTextBoxes )
			break;

		TStringManager::StringCharToUnicode( wszLine, ms_oLines[ i ].GetString(), 0x400 );
		fY -= fHeight;
		pFont->DrawTextBox( wszLine, fX, fY, fWidth, fHeight, 0xFFFFFF00, 1.0f, T2GUIFont::JUSTIFY_LEFT, -1, TTRUE, 0, TFALSE, TNULL, TNULL, TNULL );

		iTextBox++;
	}

	m_oTextBox.PostRender();
}

// $deBlob: FUNCTION 005adb00
void AConsoleState::RegisterCVar( AConsoleVar* a_pVar )
{
	for ( auto it = ms_oVars.Begin(); it != ms_oVars.End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it.Value()->GetName(), a_pVar->GetName() ) == 0 )
			return;
	}

	ms_oVars.PushBack( a_pVar );
}

// $deBlob: FUNCTION 005adc70
void AConsoleState::UnregisterCVar( AConsoleVar* a_pVar )
{
	for ( auto it = ms_oVars.Begin(); it != ms_oVars.End(); it++ )
	{
		if ( it.Value() == a_pVar )
		{
			ms_oVars.Erase( it );
			return;
		}
	}
}

// $deBlob: FUNCTION 005addb0
void AConsoleState::RegisterCommand( const TCHAR* a_szName, t_CommandFunc a_fnCommand )
{
	for ( auto it = ms_oCommands.Begin(); it != ms_oCommands.End(); it++ )
	{
		if ( it.Value().strName.CompareNoCase( a_szName ) == 0 )
			return;
	}

	Command& rCommand  = ms_oCommands.PushBack();
	rCommand.fnCommand = a_fnCommand;
	rCommand.strName   = a_szName;
}

// $deBlob: FUNCTION 005adf70
void AConsoleState::UnregisterCommand( t_CommandFunc a_fnCommand )
{
	for ( auto it = ms_oCommands.Begin(); it != ms_oCommands.End(); it++ )
	{
		if ( it.Value().fnCommand == a_fnCommand )
		{
			ms_oCommands.Erase( it );
			return;
		}
	}
}

// $deBlob: FUNCTION 005ae0b0
void AConsoleState::ClearText()
{
	ms_oLines.Clear();
	ms_oHistory.Clear();

	// NOTE: retail then erases the (empty) lists and reallocates them to nothing, as FreeMemory does
	ms_oLines.FreeMemory();
	ms_oHistory.FreeMemory();
}

// $deBlob: FUNCTION 005ae1c0
void AConsoleState::DeinitialiseStatic()
{
	// NOTE: retail erases every element and reallocates each list to nothing, as FreeMemory does
	ms_oVars.FreeMemory();
	ms_oLines.FreeMemory();
	ms_oCommands.FreeMemory();
	ms_oHistory.FreeMemory();
}

// NOTE: AConsole's Printf passes the scratch string it formatted into as a_szFormat, so this formats the scratch
// string into itself (vsprintf with an overlapping buffer); harmless as long as the text has no '%'
// $deBlob: FUNCTION 005ae2f0
void AConsoleState::Printf( const TCHAR* a_szFormat, ... )
{
	va_list args;
	va_start( args, a_szFormat );
	vsprintf( TStringManager::GetTempString8(), a_szFormat, args );
	va_end( args );

	TString8 strLine;
	strLine = TStringManager::GetTempString8();
	ms_oLines.PushBack( strLine );
}

// $deBlob: FUNCTION 005ae3b0
T2Allocator* AConsoleState::GetConsoleAllocator()
{
	return &AConsoleAllocator::s_Instance;
}

// $deBlob: FUNCTION 005ae3c0
AGameState::UpdateResult AConsoleState::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_oInputHelper.Update( a_fDeltaTime );
	m_oKeyboard.Update();

	// Closes the console
	if ( m_oInputHelper.IsJustDown( 20 ) )
		return UpdateResult_Remove;

	// Backspace
	if ( m_oInputHelper.IsJustDown( 24 ) )
	{
		TINT iLength = m_strInput.Length();

		if ( iLength > 0 )
		{
			if ( iLength <= 1 )
			{
				m_strInput = "";
			}
			else
			{
				TCHAR szInput[ 0x400 ];
				SubString( szInput, 0x400, m_strInput.GetString(), 0, iLength - 1 );
				m_strInput = szInput;
			}
		}
	}

	// The previous line of the history
	if ( m_oInputHelper.IsJustDown( 2 ) )
	{
		m_iHistoryIndex--;
		m_iHistoryIndex = TMath::Min( m_iHistoryIndex, ms_oHistory.Size() - 1 );
		m_iHistoryIndex = TMath::Max( m_iHistoryIndex, 0 );

		if ( m_iHistoryIndex < ms_oHistory.Size() )
			m_strInput = ms_oHistory[ m_iHistoryIndex ];
	}

	// The next line of the history
	if ( m_oInputHelper.IsJustDown( 3 ) )
	{
		m_iHistoryIndex++;
		m_iHistoryIndex = TMath::Min( m_iHistoryIndex, ms_oHistory.Size() - 1 );
		m_iHistoryIndex = TMath::Max( m_iHistoryIndex, 0 );

		if ( m_iHistoryIndex < ms_oHistory.Size() )
			m_strInput = ms_oHistory[ m_iHistoryIndex ];
	}

	// Tab: completes the typed prefix with the next console variable or command
	if ( m_oInputHelper.IsJustDown( 25 ) )
	{
		if ( m_iAutoCompleteIndex <= 0 )
			m_strAutoComplete = m_strInput;

		TString8 strCompleted( AutoComplete( m_strAutoComplete, m_iAutoCompleteIndex ), &AConsoleAllocator::s_Instance );

		if ( strCompleted.Length() > 0 )
		{
			m_strInput = strCompleted;
			m_iAutoCompleteIndex++;
		}
		else
		{
			m_iAutoCompleteIndex = 0;
		}
	}

	// Enter: executes the line
	if ( m_oInputHelper.IsJustDown( 28 ) )
	{
		TString8 strLine( m_strInput, &AConsoleAllocator::s_Instance );
		m_strInput           = "";
		m_iAutoCompleteIndex = 0;
		m_iHistoryIndex      = ms_oHistory.Size();
		ExecuteLine( strLine );

		return UpdateResult_OK;
	}

	// The typed characters (not tab nor the console key)
	TInputDeviceKeyboard* pKeyboard = TSTATICCAST( TObject, m_oKeyboard.GetInputDevice() )->CastTo<TInputDeviceKeyboard>();

	for ( TINT iDoodad = TInputDeviceKeyboard::VIRTKEY; iDoodad - TInputDeviceKeyboard::VIRTKEY < m_oKeyboard.GetNumDoodads(); iDoodad++ )
	{
		if ( iDoodad != TInputDeviceKeyboard::KEY_GRAVE && iDoodad != TInputDeviceKeyboard::KEY_TAB && m_oKeyboard.WasPressed( iDoodad ) )
		{
			TString8 strChar( pKeyboard->TranslateDoodadToCharacter( iDoodad ), &AConsoleAllocator::s_Instance );
			m_strInput += strChar;
			m_iAutoCompleteIndex = 0;
		}
	}

	m_fCursorTime += a_fDeltaTime;

	if ( m_fCursorTime >= 0.5f )
		m_fCursorTime = 0.0f;

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $deBlob: FUNCTION 005ae720
void AConsoleState::OnInsertion()
{
	AGameState::OnInsertion();

	// The camera is locked and the game paused while the console is open
	m_bCameraLocked = ACameraManager::GetSingleton()->GetCurrentCamera()->IsLocked();
	ACameraManager::GetSingleton()->GetCurrentCamera()->Lock( TTRUE );

	m_bGamePaused = g_oTheApp.IsGamePaused();
	g_oTheApp.SetGamePaused( TTRUE );

	AInputManager2::GetSingleton()->SetContext( AInputMap::INPUTCONTEXT_1 );

	m_oInputHelper.AddMapping( 20, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( 24, TTRUE, 0.1f );
	m_oInputHelper.AddMapping( 2, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( 3, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( 25, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( 28, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( 29, TTRUE, 0.2f );
	m_oInputHelper.Update( 0.0f );
}

// $deBlob: FUNCTION 005ae820
void AConsoleState::OnRemoval()
{
	AInputManager2::GetSingleton()->RemoveCurrentContext();
	ACameraManager::GetSingleton()->GetCurrentCamera()->Lock( m_bCameraLocked );

	if ( !AStaticCVars::cvPauseGame.GetBool() )
		g_oTheApp.SetGamePaused( m_bGamePaused );

	AGameState::OnRemoval();
}

// $deBlob: FUNCTION 005ae8b0
void AConsoleState::OnActivate()
{
	// The strings made while the console is active use its allocator
	m_pfnOldAllocatorCB = TString8::GetDefaultAllocatorCallback();
	TString8::SetDefaultAllocatorCB( GetConsoleAllocator );

	AGameState::OnActivate();
}

// $deBlob: FUNCTION 005ae8d0
void AConsoleState::OnDeactivate()
{
	TString8::SetDefaultAllocatorCB( m_pfnOldAllocatorCB );

	AGameState::OnDeactivate();
}

// $deBlob: FUNCTION 005ae8e0
AConsoleVar* AConsoleState::FindCVar( const TCHAR* a_szName )
{
	for ( auto it = ms_oVars.Begin(); it != ms_oVars.End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it.Value()->GetName(), a_szName ) == 0 )
			return it.Value();
	}

	return TNULL;
}

// $deBlob: FUNCTION 005aea80
AConsoleState::Command* AConsoleState::FindCommand( const TCHAR* a_szName )
{
	for ( auto it = ms_oCommands.Begin(); it != ms_oCommands.End(); it++ )
	{
		if ( it.Value().strName.CompareNoCase( a_szName ) == 0 )
			return &it.Value();
	}

	return TNULL;
}

// $deBlob: FUNCTION 005aec20
void AConsoleState::ExecuteLine( const TString8& a_rLine )
{
	if ( a_rLine.Length() <= 0 )
		return;

	const TCHAR* szLine  = a_rLine;
	TINT         iLength = a_rLine.Length();

	// Skips the leading spaces
	TINT i = 0;
	while ( i < iLength && szLine[ i ] == ' ' )
		i++;

	TString8                  strToken( &AConsoleAllocator::s_Instance );
	TString8                  strChar( &AConsoleAllocator::s_Instance );
	T2DynamicVector<TString8> vecArgs( &AConsoleAllocator::s_Instance, 64, 64 );
	TString8                  strArg( &AConsoleAllocator::s_Instance );

	// Splits the line at the spaces
	// NOTE: strArg is always empty when it's tested, so every space adds an argument (two spaces add an empty one)
	for ( ; i < iLength; i++ )
	{
		if ( szLine[ i ] == ' ' )
		{
			if ( strArg.Length() <= 0 )
			{
				strArg = strToken;
				vecArgs.PushBack( strArg );
				strArg = "";
			}

			strToken = "";
		}
		else
		{
			strChar.Format( "%c", szLine[ i ] );
			strToken += strChar;

			if ( i + 1 == iLength )
			{
				strArg = strToken;
				vecArgs.PushBack( strArg );
				strArg = "";
			}
		}
	}

	if ( vecArgs.Size() > 0 )
	{
		AConsoleVar* pVar = FindCVar( vecArgs[ 0 ] );

		if ( pVar != TNULL )
		{
			// A variable: shows it, after setting it to the argument if there's one
			if ( vecArgs.Size() > 1 )
				SetCVar( pVar, vecArgs );

			PrintCVar( pVar );
		}
		else
		{
			// A command: echoes its name and calls it
			ms_oLines.PushBack( vecArgs[ 0 ] );

			Command* pCommand = FindCommand( vecArgs[ 0 ] );

			if ( pCommand != TNULL )
				pCommand->fnCommand( &vecArgs[ 0 ], vecArgs.Size() );
		}

		// The arguments, each followed by a space, go to the history
		TString8 strHistory( &AConsoleAllocator::s_Instance );

		for ( TINT j = 0; j < vecArgs.Size(); j++ )
		{
			strHistory += vecArgs[ j ];
			strHistory += " ";
		}

		ms_oHistory.PushBack( strHistory );
	}
}

// $deBlob: FUNCTION 005aefd0
void AConsoleState::PrintCVar( AConsoleVar* a_pVar )
{
	TString8 strLine( &AConsoleAllocator::s_Instance );

	switch ( a_pVar->GetType() )
	{
		case AConsoleVar::TYPE_INT:
			strLine.Format( "%s %d", a_pVar->GetName(), a_pVar->GetInt() );
			break;
		case AConsoleVar::TYPE_FLOAT:
			strLine.Format( "%s %f", a_pVar->GetName(), a_pVar->GetFloat() );
			break;
		case AConsoleVar::TYPE_BOOL:
			strLine.Format( "%s %s", a_pVar->GetName(), a_pVar->GetBool() ? "true" : "false" );
			break;
		case AConsoleVar::TYPE_STRING:
			strLine.Format( "%s %s", a_pVar->GetName(), a_pVar->GetString() );
			break;
	}

	ms_oLines.PushBack( strLine );
}

// $deBlob: FUNCTION 005af100
void AConsoleState::SetCVar( AConsoleVar* a_pVar, T2DynamicVector<TString8>& a_rArgs )
{
	if ( a_rArgs.Size() < 2 )
		return;

	switch ( a_pVar->GetType() )
	{
		case AConsoleVar::TYPE_INT:
			a_pVar->SetInt( a_rArgs[ 1 ].ToInt() );
			break;
		case AConsoleVar::TYPE_FLOAT:
			a_pVar->SetFloat( a_rArgs[ 1 ].ToFloat() );
			break;
		case AConsoleVar::TYPE_BOOL:
			a_pVar->SetBool( a_rArgs[ 1 ].ToBool() );
			break;
		case AConsoleVar::TYPE_STRING:
			a_pVar->SetString( a_rArgs[ 1 ] );
			break;
	}
}

// AutoComplete's result
static TCHAR s_szAutoComplete[ 0x400 ];

// Compares one character at a time (case-insensitive)
TBOOL AConsoleState::StartsWith( const TString8& a_rName, const TString8& a_rPrefix, TINT a_iPrefixLength )
{
	TINT  iNameLength = TStringManager::String8Length( a_rName.GetString() );
	TBOOL bMatch      = TTRUE;
	TINT  i           = 0;

	TString8 strNameChar( &AConsoleAllocator::s_Instance );
	TString8 strPrefixChar( &AConsoleAllocator::s_Instance );

	for ( ; i < iNameLength && i < a_iPrefixLength; i++ )
	{
		TCHAR szNameChar[ 0x400 ];
		TCHAR szPrefixChar[ 0x400 ];
		SubString( szNameChar, 0x400, a_rName.GetString(), i, 1 );
		SubString( szPrefixChar, 0x400, a_rPrefix.GetString(), i, 1 );

		strNameChar.Copy( szNameChar );
		strPrefixChar.Copy( szPrefixChar );

		if ( strNameChar.CompareNoCase( strPrefixChar ) != 0 )
		{
			bMatch = TFALSE;
			break;
		}
	}

	return i == a_iPrefixLength && bMatch;
}

// $deBlob: FUNCTION 005af1a0
const TCHAR* AConsoleState::AutoComplete( const TString8& a_rPrefix, TINT a_iIndex )
{
	TString8 strLastMatch( &AConsoleAllocator::s_Instance );
	TINT     iPrefixLength = TStringManager::String8Length( a_rPrefix.GetString() );
	TINT     iNumMatches   = 0;

	for ( auto it = ms_oVars.Begin(); it != ms_oVars.End(); it++ )
	{
		const TCHAR* szName = it.Value()->GetName();
		TString8     strName( szName, &AConsoleAllocator::s_Instance );

		if ( StartsWith( strName, a_rPrefix, iPrefixLength ) )
		{
			strLastMatch.Copy( szName );

			if ( a_iIndex <= iNumMatches++ )
			{
				TStringManager::String8CopySafe( s_szAutoComplete, strLastMatch.GetString(), 0x400 );
				return s_szAutoComplete;
			}
		}
	}

	for ( auto it = ms_oCommands.Begin(); it != ms_oCommands.End(); it++ )
	{
		const TCHAR* szName = it.Value().strName.GetString();
		TString8     strName( szName, &AConsoleAllocator::s_Instance );

		if ( StartsWith( strName, a_rPrefix, iPrefixLength ) )
		{
			strLastMatch.Copy( szName );

			if ( a_iIndex <= iNumMatches++ )
			{
				TStringManager::String8CopySafe( s_szAutoComplete, strLastMatch.GetString(), 0x400 );
				return s_szAutoComplete;
			}
		}
	}

	TStringManager::String8CopySafe( s_szAutoComplete, strLastMatch.GetString(), 0x400 );
	return s_szAutoComplete;
}

// $deBlob: FUNCTION 005afa70
void AConsoleState::SubString( TCHAR* a_szDest, TINT a_iDestLen, const TCHAR* a_szSource, TINT a_iStart, TINT a_iCount )
{
	TINT iStrLen = TINT( TStringManager::String8Length( a_szSource ) );

	TASSERT( a_iDestLen > a_iCount );
	TASSERT( a_iStart <= iStrLen );
	TASSERT( a_iStart + a_iCount <= iStrLen );

	TUtil::MemCopy( a_szDest, a_szSource + a_iStart, a_iCount );
	a_szDest[ a_iCount ] = '\0';
}
