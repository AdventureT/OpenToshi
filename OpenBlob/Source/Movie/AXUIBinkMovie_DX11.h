#pragma once
#include <XUI/TXUIMovie.h>
#include <Platform/DX11/TRender_DX11.h>
#include "ATheoraIo.h"
#include <Sound/TSound.h>

//-----------------------------------------------------------------------------
// Purpose: the game's XUI movie, created by TXUI::CreateMovie for each TXUIVideo element: the frontend's background
// movies (ABlobFrontEnd). Plays data/movies/<name>.ogv (the PC port converted the Bink movies to Ogg Theora) with
// theoraplay, uploads each IYUV frame into three A8 textures (Y, U, V) and draws them as a quad of the element's size
// with a YUV to RGB pixel shader. TXUIVideo::OnMovieFinished is called at the end of the movie; looping videos restart
// it.
//-----------------------------------------------------------------------------
class AXUIBinkMovie : public Toshi::TXUIMovie
{
public:
	static TCONSTEXPR_VAR TINT MAX_FILE_NAME = 128;

public:
	AXUIBinkMovie();
	~AXUIBinkMovie();

	//-----------------------------------------------------------------------------
	// Toshi::TXUIMovie
	//-----------------------------------------------------------------------------
	virtual TBOOL  Open( const TWCHAR* a_wszFileName, const TWCHAR* a_wszAudioFileName, TBOOL a_bPaused, TFLOAT a_fVolume ) override; // +0x04
	virtual void   Close() override;                                                                                                  // +0x08
	virtual void   SetPaused( TBOOL a_bPaused ) override;                                                                             // +0x0C
	virtual void   SetVolume( TFLOAT a_fVolume ) override;                                                                            // +0x10
	virtual TBOOL  IsClosed() override;                                                                                               // +0x14
	virtual TFLOAT GetWidth() override;                                                                                               // +0x18
	virtual TFLOAT GetHeight() override;                                                                                              // +0x1C
	virtual void   Update( TFLOAT a_fDeltaTime ) override;                                                                            // +0x20
	virtual void   Render( TFLOAT a_fWidth, TFLOAT a_fHeight ) override;                                                              // +0x24

	//-----------------------------------------------------------------------------
	// AXUIBinkMovie
	//-----------------------------------------------------------------------------
	// Asserts and returns TTRUE (unsupported)
	virtual TBOOL Unknown28(); // +0x28

	// $deBlob: FUNCTION 00553350
	virtual TBOOL IsPaused() { return m_bPaused; } // +0x2C

	// Empty functions called by ABlobFrontEnd when it's created and destroyed
	// $deBlob: FUNCTION 00552bc0
	void OnFrontEndCreated() {}
	// $deBlob: FUNCTION 00552bd0
	void OnFrontEndDestroyed() {}

	// Set by ABlobFrontEnd::PlayBackgroundMovie
	// $deBlob: FUNCTION 006310d0
	void SetPauseAtEnd( TBOOL a_bPauseAtEnd ) { m_bPauseAtEnd = a_bPauseAtEnd; }

private:
	// Opens the 8-bit file names (Open converts them)
	TBOOL OpenMovie( const TCHAR* a_szFileName, const TCHAR* a_szAudioFileName, TBOOL a_bPaused, TFLOAT a_fVolume );

	// Reopens the movie when the looping video ends
	TBOOL Restart( TINT a_iUnk );

	void CreateTextures();
	void UploadFrame();

private:
	TCHAR                         m_szFileName[ MAX_FILE_NAME ];      // 0x008
	TCHAR                         m_szAudioFileName[ MAX_FILE_NAME ]; // 0x088 not used by OpenMovie
	ID3D11ShaderResourceView*     m_apTextures[ 6 ];                  // 0x108 Y (2), U (2), V (2)
	TINT                          m_iTexIndex;                        // 0x120 always 0
	ID3D11PixelShader*            m_pPixelShader;                     // 0x124
	ID3D11VertexShader*           m_pVertexShader;                    // 0x128
	ID3D11InputLayout*            m_pInputLayout;                     // 0x12C
	TINT                          m_iFrameMS;                         // 0x130
	TINT                          m_iTexturesWidth;                   // 0x134
	TINT                          m_iTexturesHeight;                  // 0x138
	ATheoraIo                     m_oIo;                              // 0x13C userdata: the FILE
	THEORAPLAY_Decoder*           m_pTheoraDecoder;                   // 0x148
	const THEORAPLAY_VideoFrame*  m_pTheoraVideo;                     // 0x14C
	TBOOL                         m_bHasAudioStream;                  // 0x150
	FMOD::Channel*                m_pChannel;                         // 0x154 never created
	const THEORAPLAY_AudioPacket* m_pTheoraAudio;                     // 0x158
	TINT                          m_iAudioOffset;                     // 0x15C
	TDOUBLE                       m_dPosition;                        // 0x160 ms
	TUINT32                       m_uiUnk168;                         // 0x168 cleared on open, close and at the end
	TINT                          m_iUnk16C;                          // 0x16C -1, Restart's argument
	TFLOAT                        m_fWidth;                           // 0x170 never set (0)
	TFLOAT                        m_fHeight;                          // 0x174 never set (0)
	TBOOL                         m_bFrameReady;                      // 0x178 a frame was uploaded
	TBOOL                         m_bPaused;                          // 0x179
	TBOOL                         m_bPauseAtEnd;                      // 0x17A pauses the movie at its end
	TUINT32                       m_auiUnk17C[ 4 ];                   // 0x17C
	TUINT8                        m_aUnk18C[ 0xC ];                   // 0x18C
};
