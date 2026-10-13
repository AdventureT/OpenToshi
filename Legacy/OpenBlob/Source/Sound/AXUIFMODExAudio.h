#pragma once

#include "Toshi/Xui/TXUIAudio.h"
#include "Options/AOptions.h"

class AXUIFMODExAudio : public Toshi::TXUIAudio
{
public:
	// $deBlob: FUNCTION 00617f70
	AXUIFMODExAudio()
	{
		m_iNumSubSounds = 0;
		m_pSound        = TNULL;
	}

	virtual TBOOL        Create(TCHAR* a_szSoundName);
	virtual TBOOL        Release();
	virtual FMOD::Sound* GetSubSound(TCHAR* a_pSubSoundName);
	virtual TBOOL        PlaySound(FMOD::Sound* a_pSound, TFLOAT a_fVolume, FMOD::Channel* a_pChannel);

	// $deBlob: FUNCTION 00618260
	virtual void Stop(FMOD::Channel* a_pChannel)
	{
		if (a_pChannel) a_pChannel->stop();
	}
	// $deBlob: FUNCTION 00618280
	virtual void SetVolume(FMOD::Channel* a_pChannel, TFLOAT a_fVolume)
	{
		if (a_pChannel) a_pChannel->setVolume(AOptions::GetSingleton()->GetPitch() * a_fVolume);
	}

	TINT         m_iNumSubSounds; // 0x4
	FMOD::Sound* m_pSound;        // 0x8
};
