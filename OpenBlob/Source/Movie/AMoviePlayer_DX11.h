#pragma once
#include "AMoviePlayer.h"

#include <Platform/DX11/TRender_DX11.h>
#include "ATheoraIo.h"
#include <Sound/TSound.h>

//-----------------------------------------------------------------------------
// Purpose: the DX11 full-screen movie player: plays Data\Movies\<name>.bik.ogv (the PC port converted the Bink movies
// to Ogg Theora) with theoraplay, uploads each IYUV frame into three A8 textures (Y, U, V) and draws them over the
// screen with a YUV to RGB pixel shader. The audio is a FMOD stream fed with the theora audio packets
// (pcmreadcallback/ReadBuffer); without it the video is timed by the elapsed time.
//-----------------------------------------------------------------------------
class ADX11MoviePlayer : public AMoviePlayer
{
public:
	static TCONSTEXPR_VAR TINT MAX_FILE_NAME = 256;

public:
	ADX11MoviePlayer();
	~ADX11MoviePlayer();

	static ADX11MoviePlayer* Create();

	//-----------------------------------------------------------------------------
	// AMoviePlayer
	//-----------------------------------------------------------------------------
	virtual TBOOL PlayMovie( const TCHAR* a_szFileName, TUINT32 a_uiSoundChannel, PlayFlags a_uiFlags ) override; // +0x04
	virtual void  StopMovie() override;                                                                           // +0x08
	virtual void  PauseMovie( TBOOL a_bPause ) override;                                                          // +0x0C
	virtual void  Mute( TBOOL a_bMute ) override;                                                                 // +0x10
	// $deBlob: FUNCTION 00552090
	virtual TBOOL IsMoviePlaying() override { return m_bIsPlaying; } // +0x14
	// $deBlob: FUNCTION 00552080
	virtual TBOOL IsMoviePaused() override { return m_bIsPaused; } // +0x18
	virtual void  OnRender( TFLOAT a_fDeltaTime ) override;        // +0x34
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;        // +0x38
	virtual void  OnCreate() override;                             // +0x3C
	virtual void  OnDestroy() override;                            // +0x40

	//-----------------------------------------------------------------------------
	// ADX11MoviePlayer
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 00552070
	virtual TBOOL IsMovieLooping() { return m_bIsMovieLooping; } // +0x4C

	// Fills the FMOD stream with the theora audio (called by pcmreadcallback)
	void ReadBuffer( void* a_pData, TUINT32 a_uiDataLength );

private:
	void StopMovieImpl();

	void CreateTextures( TUINT32 a_uiWidth, TUINT32 a_uiHeight );
	void ReleaseTextures();
	void CompileShader();
	void ReleaseShaders();

private:
	TUINT32                       m_uiUnk14;                            // 0x014
	ID3D11ShaderResourceView*     m_apTextures[ 6 ];                    // 0x018 Y (2), U (2), V (2)
	TINT                          m_iTexIndex;                          // 0x030 always 0
	ID3D11PixelShader*            m_pPixelShader;                       // 0x034
	ID3D11VertexShader*           m_pVertexShader;                      // 0x038
	ID3D11Buffer*                 m_pVertexBuffer;                      // 0x03C
	ID3D11InputLayout*            m_pInputLayout;                       // 0x040
	TBOOL                         m_bIsPlaying;                         // 0x044
	TBOOL                         m_bIsPaused;                          // 0x045
	TBOOL                         m_bIsMovieLooping;                    // 0x046
	TCHAR                         m_szCurrentFileName[ MAX_FILE_NAME ]; // 0x047
	TUINT32                       m_uiUnk148;                           // 0x148 cleared by PlayMovie
	TUINT8                        m_aUnk14C[ 0xC ];                     // 0x14C
	TINT                          m_iFrameMS;                           // 0x158
	TUINT32                       m_uiTexturesWidth;                    // 0x15C
	TUINT32                       m_uiTexturesHeight;                   // 0x160
	ATheoraIo                     m_oIo;                                // 0x164 userdata: the FILE
	THEORAPLAY_Decoder*           m_pTheoraDecoder;                     // 0x170
	const THEORAPLAY_VideoFrame*  m_pTheoraVideo;                       // 0x174
	TBOOL                         m_bHasAudioStream;                    // 0x178
	FMOD::Channel*                m_pChannel;                           // 0x17C
	const THEORAPLAY_AudioPacket* m_pTheoraAudio;                       // 0x180
	TINT                          m_iAudioOffset;                       // 0x184 frames of m_pTheoraAudio already read
	TDOUBLE                       m_dPosition;                          // 0x188 ms
};
