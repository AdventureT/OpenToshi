#pragma once
#include "Toshi/Sound/TSound.h"

TOSHI_NAMESPACE_START

class TSound_Win : public TSound
{
public:
	TBOOL Initialise(TINT maxchannels, TINT unk);

	static FMOD_SYSTEM_CALLBACK SoundCB;
};


TOSHI_NAMESPACE_END
