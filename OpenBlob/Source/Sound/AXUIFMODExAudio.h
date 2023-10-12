#pragma once

#include "Toshi/Xui/TXUIAudio.h"
#include "Options/AOptions.h"

class AXUIFMODExAudio : public Toshi::TXUIAudio
{
public:
	AXUIFMODExAudio()
	{
		m_iNumSubSounds = 0;
		m_pSound = TNULL;
	}

	virtual TBOOL Create(char* a_szSoundName);
	virtual TBOOL Release();
	virtual FMOD::Sound* GetSubSound(char* a_pSubSoundName);
	virtual TBOOL PlaySound(FMOD::Sound* a_pSound, TFLOAT a_fVolume, FMOD::Channel* a_pChannel);

	virtual void Stop(FMOD::Channel* a_pChannel) { if (a_pChannel) a_pChannel->stop(); }
	virtual void SetVolume(FMOD::Channel* a_pChannel, TFLOAT a_fVolume) { if (a_pChannel) a_pChannel->setVolume(AOptions::GetSingleton()->GetPitch() * a_fVolume); }

	int m_iNumSubSounds;            // 0x4
	FMOD::Sound* m_pSound; // 0x8
};

