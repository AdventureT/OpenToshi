#pragma once
#include <theoraplay/theoraplay.h>

//-----------------------------------------------------------------------------
// Purpose: retail's THEORAPLAY_Io.
//
// NOTE: retail links an older theoraplay whose THEORAPLAY_Io is { read, close, userdata } (0xC bytes); the movie
// players embed it (ADX11MoviePlayer at 0x164, AXUIBinkMovie at 0x13C) and pass it to THEORAPLAY_startDecode. The
// vendored theoraplay's THEORAPLAY_Io is bigger (streamlen and seek too), so the players keep retail's layout with
// this struct and ATheoraIo_StartDecode hands the decoder an allocated THEORAPLAY_Io copy of it, freed after the
// player's close callback has run (theoraplay closes the io once, when the decoder stops or fails to start).
//-----------------------------------------------------------------------------
struct ATheoraIo
{
	long ( *read )( THEORAPLAY_Io* a_pIo, void* a_pBuffer, long a_iBufferLength ); // 0x0
	void ( *close )( THEORAPLAY_Io* a_pIo );                                         // 0x4
	void* userdata;                                                                  // 0x8
};

// THEORAPLAY_startDecode( a_pIo, ... ) of retail's theoraplay
THEORAPLAY_Decoder* ATheoraIo_StartDecode( const ATheoraIo* a_pIo, unsigned int a_uiMaxFrames, THEORAPLAY_VideoFormat a_eVideoFormat );
