#include "pch.h"
#include "AGameState.h"
#include "AGameStateController.h"
#include "AAssetStreaming.h"

#include <Toshi/Input/TInputInterface.h>
#include <Toshi/Input/TInputDeviceKeyboard.h>

#include <Toshi2/T2GUI/T2GUI.h>

// $deBlob: FUNCTION 005c3e30
AGameState::AGameState()
    : m_InputHelperSimple(1, Toshi::TInputInterface::GetSingleton()->GetDeviceByIndex<Toshi::TInputDeviceKeyboard>())
{
	m_InputHelperSimple.AddDoodad(Toshi::TInputDeviceKeyboard::KEY_GRAVE);
	m_State = State_Null;
}

// $deBlob: FUNCTION 005c3ed0
AGameState::~AGameState()
{
	TIMPLEMENT();
}

void AGameState::Unk1()
{
}

TBOOL AGameState::Unk2()
{
	return TFALSE;
}

void AGameState::Unk3(void*, void*)
{
}

// $deBlob: FUNCTION 005c4140
AGameState::UpdateResult AGameState::OnUpdate(float deltaTime)
{
	m_InputHelperSimple.Update();

	if (!AAssetStreaming::GetSingleton()->HasActiveJobs())
	{
		sm_pLoadIconRect->SetVisible(TFALSE);
	}

	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnUpdate(deltaTime);
	}

	return UpdateResult_OK;
}

// $deBlob: FUNCTION 005c4240
void AGameState::OnInsertion()
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnInsertion();
	}
}

// $deBlob: FUNCTION 005c4300
void AGameState::OnRemoval()
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnRemoval();
	}

	m_GameStates.DeleteAll();
}

// $deBlob: FUNCTION 005c43d0
void AGameState::OnSuspend()
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnDeactivate();
	}
}

// $deBlob: FUNCTION 005c4490
void AGameState::OnResume(AGameState* pOldState)
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnActivate();
	}
}

// $deBlob: FUNCTION 005c4560
void AGameState::OnActivate()
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnActivate();
	}
}

// $deBlob: FUNCTION 005c4620
void AGameState::OnDeactivate()
{
	for (auto state = m_GameStates.Begin(); state != m_GameStates.End(); state++)
	{
		state->OnDeactivate();
	}
}

// $deBlob: FUNCTION 005c1c90
TBOOL AGameState::CanActivateConsoleState()
{
	return TTRUE;
}

// $deBlob: FUNCTION 005b0400
TBOOL AGameState::CheckForControllerRemoval()
{
	return TFALSE;
}

// $deBlob: FUNCTION 005b0a20
TBOOL AGameState::PauseOnControllerReinserted()
{
	return TFALSE;
}

#ifdef TOSHI_DEBUG
void AGameState::DEBUG_RenderImGui()
{
}
#endif // TOSHI_DEBUG

// $deBlob: FUNCTION 005c4030
void AGameState::SetupLoadIcon()
{
	TASSERT(TNULL == sm_pLoadIconRect);
	sm_pLoadIconRect = new Toshi::T2GUIRectangle();
	sm_pLoadIconRect->Create(50.0f, 50.0f);
	sm_pLoadIconRect->SetColour(-1);
	sm_pLoadIconMat = Toshi::T2GUI::GetSingleton()->CreateMaterial("loadicon.tga");
	sm_pLoadIconRect->SetMaterial(sm_pLoadIconMat);
	sm_pLoadIconRect->SetAnchor(Toshi::T2GUIElement::Anchor::TopLeft);
	sm_pLoadIconRect->SetPivot(Toshi::T2GUIElement::Pivot::TopLeft);
}

// $deBlob: FUNCTION 005c40e0
void AGameState::DestroyLoadIcon()
{
}

// $deBlob: FUNCTION 005c46f0
void AGameState::RemoveSelf()
{
	TASSERT(AGameStateController::GetSingleton()->GetCurrentGameState() == this);
	AGameStateController::GetSingleton()->PopCurrentState();
}
