#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Sound interface of XUI, implemented by the game (AXUIFMODExAudio) and given to
// TXUI::SetAudio. TXUISound plays its sounds through it. The handles are FMOD::Sound* and FMOD::Channel*.
//-----------------------------------------------------------------------------
class TXUIAudio
{
public:
	// $deBlob: FUNCTION 006182c0
	TXUIAudio() {}

	// $deBlob: FUNCTION 006cf8e0
	virtual ~TXUIAudio() {} // +0x00

	virtual TBOOL Create( const TCHAR* a_szSoundBankName )                                         = 0; // +0x04
	virtual TBOOL Release()                                                                        = 0; // +0x08
	virtual void* GetSubSound( const TCHAR* a_szSubSoundName )                                     = 0; // +0x0C FMOD::Sound*
	virtual TBOOL PlaySound( void* a_pSound, TFLOAT a_fVolume, void** a_ppOutChannel )             = 0; // +0x10 FMOD::Channel**
	virtual void  Stop( void* a_pChannel )                                                         = 0; // +0x14
	virtual void  SetVolume( void* a_pChannel, TFLOAT a_fVolume )                                  = 0; // +0x18
};

TOSHI_NAMESPACE_END
