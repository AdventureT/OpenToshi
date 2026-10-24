#include "pch.h"
#include "ADebugGameStateHelper.h"
#include "Console/AStaticCVars.h"
#include "Input/AInputManager2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ADebugGameStateHelper ) == 0x30 );

// $deBlob: FUNCTION 005c2530
ADebugGameStateHelper::ADebugGameStateHelper()
    : m_bDebugPickups( TFALSE )
    , m_bUnk29( TTRUE )
    , m_iUnk2C( -1 )
{
}

// $deBlob: FUNCTION 005c25d0
// $deBlob: FUNCTION 005c28e0
ADebugGameStateHelper::~ADebugGameStateHelper()
{
}

// $deBlob: FUNCTION 005c2680
void ADebugGameStateHelper::Init()
{
	AInputManager2* pInputManager = AInputManager2::GetSingleton();
	pInputManager->SetContext( AInputMap::INPUTCONTEXT_InGame );

	m_oInputHelper.AddMapping( 21, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( 26, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( 27, TFALSE, -1.0f );
	m_oInputHelper.Update( 0.0f );

	pInputManager->RemoveCurrentContext();
}

// $deBlob: FUNCTION 005c26f0
TBOOL ADebugGameStateHelper::Update( TFLOAT a_fDeltaTime )
{
	m_oInputHelper.Update( a_fDeltaTime );

	TBOOL bJustDown = m_oInputHelper.IsJustDown( 21, AInputManager2::INPUTDEVICE_Keyboard );

	if ( bJustDown == TTRUE )
	{
		AInputManager2* pInputManager = AInputManager2::GetSingleton();
		return pInputManager->GetButtonDown( 22, pInputManager->GetControllerHandle( AInputManager2::INPUTDEVICE_Controller1 ) );
	}

	return bJustDown;
}

// $deBlob: FUNCTION 005c2750
void ADebugGameStateHelper::ToggleDebugPickups()
{
	m_bDebugPickups = !m_bDebugPickups;
	AStaticCVars::cvDebugPickups.SetBool( m_bDebugPickups );
}

// $deBlob: FUNCTION 005c27b0
void ADebugGameStateHelper::SyncDebugPickups()
{
	if ( AStaticCVars::cvDebugPickups.GetBool() != m_bDebugPickups )
	{
		m_bDebugPickups = !m_bDebugPickups;
		AStaticCVars::cvDebugPickups.SetBool( m_bDebugPickups );
	}
}
