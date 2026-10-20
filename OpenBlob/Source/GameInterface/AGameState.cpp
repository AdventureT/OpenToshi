#include "pch.h"
#include <GUI/T2GUIMaterial.h>
#include <GUI/T2GUI.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUIElement.h>
#include "AGameState.h"
#include "AGameStateController.h"
#include "AAssetStreaming.h"
#include "Console/AConsoleState.h"

#include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void* AGameState::sm_pLoadIconRect;
void* AGameState::sm_pLoadIconMat;

// $deBlob: FUNCTION 005c3d30
// $deBlob: FUNCTION 005c3d40
// $deBlob: FUNCTION 005c3db0
// $deBlob: FUNCTION 005c3e10
// $deBlob: FUNCTION 005c3e20
// $deBlob: FUNCTION 0053c5d0
// $deBlob: FUNCTION 008584c0
TDEFINE_CLASS( AGameState );

// $deBlob: FUNCTION 005c4030
void AGameState::SetupLoadIcon()
{
	TASSERT( TNULL == sm_pLoadIconRect );

	T2GUIRectangle* pRect = new T2GUIRectangle();
	sm_pLoadIconRect      = pRect;
	pRect->Create( 50.0f, 50.0f );
	pRect->SetColour( 0xFFFFFFFF );
	sm_pLoadIconMat = T2GUI::CreateMaterial( "loadicon.tga" );
	pRect->SetMaterial( TSTATICCAST( T2GUIMaterial, sm_pLoadIconMat ) );
	pRect->SetAnchor( T2GUIATTACHMENT_BOTTOMLEFT );
	pRect->SetPivot( T2GUIATTACHMENT_BOTTOMLEFT );
}

// $deBlob: FUNCTION 005c40e0
void AGameState::DestroyLoadIcon()
{
	T2GUIRectangle* pRect = TSTATICCAST( T2GUIRectangle, sm_pLoadIconRect );
	pRect->RemoveSelf();

	if ( sm_pLoadIconRect != TNULL )
		delete pRect;

	sm_pLoadIconRect = TNULL;
	T2GUI::DestroyMaterial( TSTATICCAST( TMaterial, sm_pLoadIconMat ) );
	sm_pLoadIconMat = TNULL;
}

// $deBlob: FUNCTION 005c3e30
AGameState::AGameState()
    : m_InputHelper( 1, TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 ) )
{
	m_uiParent = 0;
	m_State    = State_Null;

	m_InputHelper.AddDoodad( TInputDeviceKeyboard::KEY_GRAVE );
}

// $deBlob: FUNCTION 005c3ed0
// $deBlob: FUNCTION 005c48f0
AGameState::~AGameState()
{
	m_GameStates.DeleteAll();
}

// $deBlob: FUNCTION 005c4140
AGameState::UpdateResult AGameState::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_InputHelper.Update();

#if defined( DEBLOB_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
	// The console key (the only key of m_InputHelper) opens the developer console
	if ( m_InputHelper.WasPressed( TInputDeviceKeyboard::KEY_GRAVE ) && CanActivateConsoleState() )
		AConsoleState::RequestOpen();
#endif

	if ( !AAssetStreaming::GetSingleton()->HasActiveJobs() )
	{
		TSTATICCAST( T2GUIElement, sm_pLoadIconRect )->SetVisible( TFALSE );
	}

	for ( auto it = m_GameStates.Begin(); it != m_GameStates.End(); )
	{
		AGameState* pState = it.Get();
		++it;
		pState->OnUpdate( a_fDeltaTime );
	}

	return UpdateResult_OK;
}

// $deBlob: FUNCTION 005c4240
void AGameState::OnInsertion()
{
	T2_FOREACH( m_GameStates, it )
	{
		it->OnInsertion();
	}
}

// $deBlob: FUNCTION 005c4300
void AGameState::OnRemoval()
{
	T2_FOREACH( m_GameStates, it )
	{
		it->OnRemoval();
	}

	m_GameStates.DeleteAll();
}

// NOTE: retail calls OnDeactivate (+0x30) of the child states, not OnSuspend
// $deBlob: FUNCTION 005c43d0
void AGameState::OnSuspend()
{
	T2_FOREACH( m_GameStates, it )
	{
		it->OnDeactivate();
	}
}

// $deBlob: FUNCTION 005c4490
void AGameState::OnResume( AGameState* a_pPreviousState )
{
	T2_FOREACH( m_GameStates, it )
	{
		it->OnResume( a_pPreviousState );
	}
}

// $deBlob: FUNCTION 005c4560
void AGameState::OnActivate()
{
	T2_FOREACH( m_GameStates, it )
	{
		it->OnActivate();
	}
}

// $deBlob: FUNCTION 005c4620
void AGameState::OnDeactivate()
{
	T2_FOREACH( m_GameStates, it )
	{
		it->OnDeactivate();
	}
}

// Adds a child state (at the end, m_uiParent is its parent) and inserts it
// $deBlob: FUNCTION 005c3f90
void AGameState::AddChildState( AGameState* a_pState )
{
	a_pState->InsertBefore( m_GameStates.End() );
	a_pState->m_uiParent = TREINTERPRETCAST( TUINT32, this );
	a_pState->OnInsertion();
}

// $deBlob: FUNCTION 005c46f0
void AGameState::RemoveSelf()
{
	TASSERT( AGameStateController::GetSingleton()->GetCurrentGameState() == this );
	AGameStateController::GetSingleton()->PopCurrentState();
}
