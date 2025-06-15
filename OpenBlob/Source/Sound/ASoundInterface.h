#pragma once

#include "Toshi/Sound/TSound.h"

class ASoundInterface : public Toshi::TSingleton<ASoundInterface>
{
	void GetEventCategory(TINT index)
	{
		auto sound  = Toshi::TSound::GetSingleton();
		auto system = sound->GetSystem();
	}
};
