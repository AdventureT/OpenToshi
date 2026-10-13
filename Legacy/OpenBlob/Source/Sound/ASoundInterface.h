#pragma once

#include "Toshi/Sound/TSound.h"

class ASoundInterface : public Toshi::TSingleton<ASoundInterface>
{
	// $deBlob: FUNCTION 00611610
	void GetEventCategory(TINT index)
	{
		auto sound  = Toshi::TSound::GetSingleton();
		auto system = sound->GetSystem();
	}
};
