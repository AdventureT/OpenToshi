#include "pch.h"
#include "AXUIFMODExAudio.h"

TOSHI_NAMESPACE_USING

TBOOL AXUIFMODExAudio::Create(char* a_szSoundName)
{
	auto sound = TSound::GetSingleton();

	if (sound != TNULL && a_szSoundName != TNULL)
	{
		auto system = sound->GetSystem();
		FMOD_RESULT eResult = system->createSound(a_szSoundName, FMOD_DEFAULT, NULL, &m_pSound);
		if (!TSound::ErrorCheck(eResult))
		{
			eResult = m_pSound->getNumSubSounds(&m_iNumSubSounds);
			return !TSound::ErrorCheck(eResult);
		}
	}
	return TFALSE;
}

TBOOL AXUIFMODExAudio::Release()
{
	if (m_pSound != TNULL)
	{
		FMOD_RESULT eResult = m_pSound->release();
		TASSERT(eResult == FMOD_OK);
		if (eResult == FMOD_OK)
		{
			m_pSound = TNULL;
			return TTRUE;
		}
	}
	return TFALSE;
}

FMOD::Sound* AXUIFMODExAudio::GetSubSound(char* a_pSubSoundName)
{
	auto sound = TSound::GetSingleton();
	if (sound == TNULL || m_pSound != TNULL) return TNULL;

	for (size_t i = 0; i < m_iNumSubSounds; i++)
	{
		FMOD::Sound* subSound;
		char subSoundName[256];

		FMOD_RESULT eResult = m_pSound->getSubSound(i, &subSound);
		if (TSound::ErrorCheck(eResult)) return TNULL;
		
		eResult = subSound->getName(subSoundName, sizeof(subSoundName));
		TASSERT(!TSound::ErrorCheck(eResult), "Error retrieving subsound %d\n", i);

		if (TStringManager::String8Compare(a_pSubSoundName, subSoundName) == 0)
		{
			return subSound;
		}
	}
	return TNULL;
}

TBOOL AXUIFMODExAudio::PlaySound(FMOD::Sound* a_pSound, TFLOAT a_fVolume, FMOD::Channel* a_pChannel)
{
	auto sound = TSound::GetSingleton();
	if (a_pSound == TNULL) return TFALSE;
	auto system = sound->GetSystem();

	FMOD_RESULT eResult = system->playSound(a_pSound, TNULL, TTRUE, &a_pChannel);

	if (!TSound::ErrorCheck(eResult))
	{
		eResult = a_pChannel->setVolume(AOptions::GetSingleton()->GetPitch() * a_fVolume);
		if (!TSound::ErrorCheck(eResult))
		{
			a_pChannel->setPaused(TFALSE);
			if (!TSound::ErrorCheck(eResult)) return TTRUE;
		}
	}

	return TFALSE;
}
