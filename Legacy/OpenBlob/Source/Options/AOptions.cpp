#include "pch.h"
#include "AOptions.h"
#include <Toshi/Sound/TSound.h>

using namespace Toshi;

// $deBlob: FUNCTION 005e8530
void AOptions::SoundThings(TBOOL unk, TFLOAT a_fPitch, TFLOAT a_fVolume)
{
	if (TSound::GetSingleton()->IsInitialised())
	{
		TMath::Clip(m_Options->m_iPitch, 0, 10);
		TMath::Clip(m_Options->m_iVolume, 0, 10);
		TFLOAT fUnk2   = 0.0f <= a_fPitch ? a_fPitch : m_Options->m_iPitch / 10.0f;
		TFLOAT fVolume = 0.0f <= a_fVolume ? a_fVolume : m_Options->m_iVolume / 10.0f;
	}
}

TFLOAT AOptions::GetPitch()
{
	TIMPLEMENT();
	return 0.5F;
}
