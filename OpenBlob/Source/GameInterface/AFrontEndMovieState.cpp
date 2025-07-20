#include "pch.h"
#include "AFrontEndMovieState.h"
#include "Movie/AMoviePlayer.h"
#include "ImGui/AImGui.h"
#include "AppBoot.h"

#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi/Shaders/SysShader/TSysShaderHAL.h>

#ifdef TOSHI_DEBUG
// Used to output debug info

#  ifdef TOSHI_RENDERER_DX11
#	include "Platform/DX11/Movie/ADX11MoviePlayer_DX11.h"
#  elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#	include "Platform/SDL/Movie/ASDLMoviePlayer_SDL.h"
#  endif // TOSHI_RENDERER_OPENGL

#endif // TOSHI_DEBUG

#include "Input/AInputHelper.h"
#include "Toshi/Xui/TXUI.h"
#include "XUI/Frontend/ABlobFrontEnd.h"

AGameState::UpdateResult AFrontEndMovieState::OnUpdate(TFLOAT deltaTime)
{
	TIMPLEMENT();
	m_InputHelper.Update(deltaTime);

	if (m_iAssetId != Asset_Legal)
	{
		if (m_InputHelper.IsJustDown(0x2A))
		{
			// Commenting this cause handling input is not finished atm and it would skip all instantly
			m_bSkip = true;
		}
	}

	AGameState::OnUpdate(deltaTime);
	AMoviePlayer* pMoviePlayer = AMoviePlayer::GetSingleton();

	if (m_iAssetId == Asset_Legal || (m_iAssetId < 3 && pMoviePlayer->IsMoviePlaying()))
	{
		m_fBackgroundLeftTime -= deltaTime;
	}

	switch (m_iAssetId)
	{
		case Asset_THQLogo:
		case Asset_LogoMovie:
		case Asset_Intro:
			if (m_bSkip)
			{
				if (AApplication::g_oTheApp.m_bUnk4)
				{
					StopMovieIfPlaying();
					TIMPLEMENT_D("Start Rendering Frontend menu");
				}
			}
			else
			{
				if (m_fBackgroundLeftTime <= 0.0f)
				{
					if (AApplication::g_oTheApp.m_bUnk4)
					{
						StopMovieIfPlaying();
						TIMPLEMENT_D("Start Rendering Frontend menu");
					}
				}
			}
			break;
		default:
			if (m_fBackgroundLeftTime <= 0.0f)
			{
				if (AApplication::g_oTheApp.m_bUnk4)
				{
					StopMovieIfPlaying();
					TIMPLEMENT_D("Start Rendering Frontend menu");
				}
			}
			break;
	}

	switch (m_iAssetId)
	{
		case Asset_Legal:
			if (HasBackgroundStoppedShowing())
			{
				StartMovie(Asset_THQLogo);
			}
			break;
		case Asset_THQLogo:
			if (HasMovieStopped())
			{
				StartMovie(Asset_LogoMovie);
			}
			break;
		case Asset_LogoMovie:
			if (HasMovieStopped())
			{
				StartMovie(Asset_Intro);
			}
			break;
		case Asset_Intro:
			if (HasMovieStopped())
			{
				StopMovieIfPlaying();
				m_bSkip = TFALSE;
				TIMPLEMENT_D("Start Rendering Frontend menu");
				StartBlobFrontendMenu();
			}
			break;
	}

	return UpdateResult_OK;
}

void AFrontEndMovieState::OnInsertion()
{
	auto pGUI         = Toshi::T2GUI::GetSingletonSafe();
	auto pGUIRenderer = pGUI->GetRenderer();
	auto pRootElement = pGUI->GetRootElement();

	AInputManager2::GetSingleton()->SetContext(AInputMap::INPUTCONTEXT_UNK8);
	m_InputHelper.AddMapping(0x2A, TFALSE, -1.0f); // SPACE
	m_InputHelper.AddMapping(0x2B, TFALSE, -1.0f); // RETURN

	m_Background.Create(s_Assets[m_iAssetId]);
	m_Background.SetVisible(TTRUE);

	TIMPLEMENT();
	AGameState::OnInsertion();
}

void AFrontEndMovieState::OnRemoval()
{
	TIMPLEMENT();
	AGameState::OnRemoval();
}

void AFrontEndMovieState::OnActivate()
{
	AGameState::OnActivate();

	auto pRootElement = Toshi::T2GUI::GetSingleton()->GetRootElement();
	pRootElement->AddChildHead(&m_Background);

	TFLOAT fWidth, fHeight;
	pRootElement->GetDimensions(fWidth, fHeight);
	m_Background.SetDimensions(fWidth, fHeight);

	AApplication::g_oTheApp.SetRenderWorld(TFALSE);
}

void AFrontEndMovieState::OnDeactivate()
{
	AGameState::OnDeactivate();
	AMoviePlayer* pPlayer = AMoviePlayer::GetSingletonSafe();

	if (pPlayer->IsMoviePlaying())
	{
		pPlayer->StopMovie();
	}

	m_Background.Unlink();
	AApplication::g_oTheApp.SetRenderWorld(TTRUE);
}

TBOOL AFrontEndMovieState::HasBackgroundStoppedShowing()
{
	switch (m_iAssetId)
	{
		case Asset_THQLogo:
		case Asset_LogoMovie:
		case Asset_Intro:
			if (m_bSkip)
			{
				return TTRUE;
			}
	}

	return m_fBackgroundLeftTime <= 0.0f;
}

TBOOL AFrontEndMovieState::HasMovieStopped()
{
	switch (m_iAssetId)
	{
		case Asset_THQLogo:
		case Asset_LogoMovie:
		case Asset_Intro:
			if (m_bSkip)
			{
				return TTRUE;
			}
	}
	return !AMoviePlayer::GetSingleton()->IsMoviePlaying();
}

void AFrontEndMovieState::StopMovieIfPlaying()
{
	AMoviePlayer* pPlayer = AMoviePlayer::GetSingleton();
	if (pPlayer->IsMoviePlaying())
	{
		pPlayer->StopMovie();
	}
}

void AFrontEndMovieState::StartMovie(Asset assetId)
{
	AMoviePlayer* pPlayer = AMoviePlayer::GetSingleton();

	if (pPlayer->IsMoviePlaying())
	{
		pPlayer->StopMovie();
	}

	m_iAssetId            = assetId;
	m_fBackgroundLeftTime = 5.0f;
	m_bSkip               = TFALSE;
	m_Background.SetVisible(TFALSE);
	pPlayer->PlayMovie(s_Assets[assetId], 0, 0);
}

void AFrontEndMovieState::StartBlobFrontendMenu()
{
	TIMPLEMENT();
#if 0
    AGameStateController* pGameStateController = AGameStateController::GetSingleton();
    Toshi::TXUI* pXui = Toshi::TXUI::GetSingleton();

    if (AApplication::g_oTheApp.m_bUnk3) {
        pGameStateController->ResetStack();
        return;
    }
    else if (AApplication::g_oTheApp.m_bUnk4) {
        return;
    }
    pGameStateController->ResetStack();
    Toshi::TXUIScene* pFrontend = pXui->CreateScene(pXui->FindResource("frontend.xur"), 0);
    TASSERT(pFrontend->IsA(TGetClass(ABlobFrontEnd)));
#endif
}

#ifdef TOSHI_DEBUG
void AFrontEndMovieState::DEBUG_RenderImGui()
{
	auto pMoviePlayer = TSTATICCAST(ADX11MoviePlayer, AMoviePlayer::GetSingleton());

	TUINT audioPosition = 0;

	if (pMoviePlayer->m_pChannel)
		pMoviePlayer->m_pChannel->getPosition(&audioPosition, FMOD_TIMEUNIT_MS);

	AIMGUI_FORMAT("Video file: %s", pMoviePlayer->m_CurrentFileName);
	AIMGUI_FORMAT("Current asset: %s", s_Assets[m_iAssetId]);
	AIMGUI_FORMAT("Video position: %.0lfms", pMoviePlayer->m_Position);
	AIMGUI_FORMAT("Audio position: %ums", audioPosition);
	AIMGUI_FORMAT("Audio delay: %dms", TINT(pMoviePlayer->m_Position - audioPosition));
	AIMGUI_FORMAT("Background image left time: %.2fs", m_fBackgroundLeftTime);

	if (ImGui::Button("Reset state"))
	{
		m_iAssetId            = Asset_Legal;
		m_fBackgroundLeftTime = 5.0f;
		m_Background.SetVisible(TTRUE);
		StopMovieIfPlaying();
	}
}
#endif // TOSHI_DEBUG
