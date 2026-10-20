#pragma once
#include <XUI/TXUIAudio.h>
#include <Sound/TSound.h>

//-----------------------------------------------------------------------------
// AXUIFMODExAudio (0xC): the TXUIAudio of the game (TXUI::SetAudio in AppBoot).
// The sound bank is an FMOD sound whose sub sounds are the XUI sounds.
//-----------------------------------------------------------------------------
class AXUIFMODExAudio : public Toshi::TXUIAudio
{
public:
	AXUIFMODExAudio();
	~AXUIFMODExAudio();

	virtual TBOOL Create( const TCHAR* a_szSoundBankName ) OVERRIDE;                       // +0x04
	virtual TBOOL Release() OVERRIDE;                                                      // +0x08
	virtual void* GetSubSound( const TCHAR* a_szSubSoundName ) OVERRIDE;                   // +0x0C
	virtual TBOOL PlaySound( void* a_pSound, TFLOAT a_fVolume, void** a_ppOutChannel ) OVERRIDE; // +0x10
	virtual void  Stop( void* a_pChannel ) OVERRIDE;                                       // +0x14
	virtual void  SetVolume( void* a_pChannel, TFLOAT a_fVolume ) OVERRIDE;                // +0x18

private:
	TINT         m_iNumSubSounds; // 0x04
	FMOD::Sound* m_pSound;        // 0x08
};
