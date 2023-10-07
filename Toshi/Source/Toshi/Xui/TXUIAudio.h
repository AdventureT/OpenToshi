#pragma once

#include "Toshi/Sound/TSound.h"

TOSHI_NAMESPACE_BEGIN

class TXUIAudio
{
	virtual TBOOL Create(char* a_szSoundName) = 0;
	virtual TBOOL Release() = 0;
	virtual FMOD::Sound* FindSubSound(char* a_pSubSoundName) = 0;
	virtual TBOOL PlaySound(FMOD::Sound* a_pSound, TFLOAT a_fVolume, FMOD::Channel* a_pChannel) = 0;

	virtual void Stop(FMOD::Channel* a_pChannel) = 0;
	virtual void SetVolume(FMOD::Channel* a_pChannel, TFLOAT a_fVolume) = 0;
};

TOSHI_NAMESPACE_END