#include "pch.h"
#include "ATheoraIo.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: not in retail (see ATheoraIo.h): the vendored theoraplay's THEORAPLAY_Io built from retail's one
struct ATheoraIoAdapter : THEORAPLAY_Io
{
	void ( *fnClose )( THEORAPLAY_Io* a_pIo ); // the player's close callback
};

static void ATheoraIoAdapter_Close( THEORAPLAY_Io* a_pIo )
{
	ATheoraIoAdapter* pAdapter = TSTATICCAST( ATheoraIoAdapter, a_pIo );
	pAdapter->fnClose( a_pIo );
	delete pAdapter;
}

THEORAPLAY_Decoder* ATheoraIo_StartDecode( const ATheoraIo* a_pIo, unsigned int a_uiMaxFrames, THEORAPLAY_VideoFormat a_eVideoFormat )
{
	ATheoraIoAdapter* pAdapter = new ATheoraIoAdapter;
	pAdapter->read             = a_pIo->read;
	pAdapter->streamlen        = TNULL;
	pAdapter->seek             = TNULL;
	pAdapter->close            = ATheoraIoAdapter_Close;
	pAdapter->userdata         = a_pIo->userdata;
	pAdapter->fnClose          = a_pIo->close;

	return THEORAPLAY_startDecode( pAdapter, a_uiMaxFrames, a_eVideoFormat );
}
