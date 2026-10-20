#pragma once

TOSHI_NAMESPACE_START

class TXUIVideo;

//-----------------------------------------------------------------------------
// Movie interface used by TXUIVideo, implemented by the game (AXUIBinkMovie
// vtable) and created through TXUI::CreateMovie.
//-----------------------------------------------------------------------------
class TXUIMovie
{
public:
	// $deBlob: FUNCTION 006bc870
	TXUIMovie() {}

	virtual ~TXUIMovie() = default; // +0x00

	// Opens a movie (AXUIBinkMovie converts the names to 8-bit strings), closes the current one first and applies
	// SetPaused( a_bPaused ) and SetVolume( a_fVolume )
	virtual TBOOL  Open( const TWCHAR* a_wszFileName, const TWCHAR* a_wszAudioFileName, TBOOL a_bPaused, TFLOAT a_fVolume ) = 0; // +0x04
	virtual void   Close()                                                                                                   = 0; // +0x08
	virtual void   SetPaused( TBOOL a_bPaused )                                                                              = 0; // +0x0C
	virtual void   SetVolume( TFLOAT a_fVolume )                                                                             = 0; // +0x10
	virtual TBOOL  IsClosed()                                                                                                = 0; // +0x14
	virtual TFLOAT GetWidth()                                                                                                = 0; // +0x18
	virtual TFLOAT GetHeight()                                                                                               = 0; // +0x1C
	virtual void   Update( TFLOAT a_fDeltaTime )                                                                             = 0; // +0x20 calls TXUIVideo::OnMovieFinished at the end
	virtual void   Render( TFLOAT a_fWidth, TFLOAT a_fHeight )                                                               = 0; // +0x24

	// NOTE: unreferenced COMDAT in TXUIVideo.cpp's range (`this[1] = a2`), most likely this setter
	// $deBlob: FUNCTION 006bc860
	void SetVideo( TXUIVideo* a_pVideo ) { m_pVideo = a_pVideo; }

	TXUIVideo* GetVideo() const { return m_pVideo; }

protected:
	TXUIVideo* m_pVideo; // 0x04 set by TXUIVideo::Create
};

TOSHI_NAMESPACE_END
