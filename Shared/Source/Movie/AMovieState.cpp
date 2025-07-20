#include "pch.h"
#include "AMovieState.h"
#include "Locale/ALocaleManager.h"
#include <Movie/AMoviePlayer.h>
#include "AAssetStreaming.h"
#include <AppBoot.h>

using namespace Toshi;

AMovieState::AMovieState(const char* name, TBOOL a_bUseSoundChannels, TBOOL bUseLocalePostFix, TBOOL unk3, AGameState* gameState)
    : AGameState(), m_stringVector(GetGlobalAllocator(), 1, 0)
{
	m_bUseSoundChannels = a_bUseSoundChannels;
	m_bUseLocalePostFix = bUseLocalePostFix;
	m_flags             = 0;
	m_fDeltaTime        = 0.0f;

	m_stringVector.PushBack(new TString8(name));
}

AGameState::UpdateResult AMovieState::OnUpdate(float deltaTime)
{
	TIMPLEMENT();
	m_InputHelper.Update(deltaTime);
	m_fDeltaTime += deltaTime;

	switch (m_flags)
	{
		case 0:
			if (!AAssetStreaming::GetSingleton()->HasActiveJobs())
			{
				m_bRenderWorld = AApplication::g_oTheApp.m_bRenderWorld;
				AApplication::g_oTheApp.SetRenderWorld(TFALSE);
				TIMPLEMENT_D("AMovieState::PauseSounds(1,0)");
				PlayMovie(0);
			}
			break;
		case 1:

		default:
			break;
	}

	return UpdateResult();
}

void AMovieState::OnInsertion()
{
	auto inputMng = AInputManager2::GetSingleton();
	inputMng->SetContext(AInputMap::INPUTCONTEXT_UNK4);
	m_InputHelper.AddMapping(AInputMap::INPUTBUTTON_SPACE, TFALSE, -1.0f);
	m_InputHelper.Update();
}

void AMovieState::PlayMovie(TUINT8 unk)
{
	TString8        str       = **m_stringVector.Begin();
	ALocaleManager& localeMgr = ALocaleManager::Instance();

	if (m_bUseLocalePostFix)
	{
		str.Concat("_");
		str.Concat(localeMgr.GetCurLocaleCode());
	}

	int soundChannel = 0;

	if (m_bUseSoundChannels)
	{
		soundChannel = localeMgr.GetCurSoundChannel();
	}

	TBOOL bRes   = AMoviePlayer::GetSingleton()->PlayMovie(str, soundChannel, unk);
	m_fDeltaTime = 0.0f;
	m_flags      = !bRes ? 3 : 1;
}
