#include "pch.h"
#include "AppBoot.h"
#include "AAppInitState.h"
#include "Render/ARenderer.h"
#include "Render/ModelLoading/AModelLoader.h"
#include "Movie/AMoviePlayer.h"
#include "GameInterface/AFrontEndMovieState.h"
#include "GameInterface/ATestState.h"
#include "AAssetStreaming.h"
#include "ALevelInformation.h"
#include "Options/AOptions.h"
#include "ARandom.h"

#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/Movie/ADX11MoviePlayer_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/Movie/ASDLMoviePlayer_SDL.h"
#endif // TOSHI_RENDERER_OPENGL

#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi2/T2GUI/T2GUIRectangle.h>

using namespace Toshi;

AGameState::UpdateResult AAppInitState::OnUpdate(TFLOAT deltaTime)
{
	TIMPLEMENT();

	if (AAssetStreaming::GetSingleton()->HasActiveJobs())
	{
		ARenderer::GetSingleton()->SetBackgroundColour(0, 0, 0);
		return AGameState::OnUpdate(deltaTime);
	}

	TString8 str;
	str = str.Format("data/%s.trb", AApplication::g_oTheApp.GetLevelInformationFileName());

	ALevelInformation* levelInformation = ALevelInformation::CreateSingleton();
	levelInformation->Create(str);

	AOptions::CreateSingleton();
	ARandom::CreateSingleton();
	AModelLoader::CreateSingleton();
	ADX11MoviePlayer::CreateSingleton<ADX11MoviePlayer>()->OnCreate();

	return UpdateResult_Remove;
}

void AAppInitState::OnInsertion()
{
	TIMPLEMENT();
	AGameState::OnInsertion();
}

void AAppInitState::OnRemoval()
{
	AGameState::OnRemoval();

	if (AApplication::g_oTheApp.ShouldLoadModelViewState())
	{
		TIMPLEMENT_D("AModelViewState");
	}
	else
	{
		AGameStateController::GetSingletonSafe()->PushState(new AFrontEndMovieState);
		TIMPLEMENT_D("The game");
	}
}
