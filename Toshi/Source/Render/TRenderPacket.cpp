#include "ToshiPCH.h"
#include "TRenderPacket.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: de Blob's (no callers: the packets come from TOrderTable's pool without construction) sets no ambient or light,
// unlike Barnyard's
// $deBlob: FUNCTION 006ae830
TRenderPacket::TRenderPacket()
{
	m_pNextPacket       = TNULL;
	m_pUnk              = TNULL;
	m_pSkeletonInstance = TNULL;
	m_ui8Fade           = 255;
	m_aUnk80[ 0 ]       = 255;
	m_fnRenderCallback  = TNULL;
	m_pRenderUserData   = TNULL;
	*TREINTERPRETCAST( TUINT32*, &m_aUnk80[ 0x0C ] ) = 0;
	*TREINTERPRETCAST( TUINT32*, &m_aUnk80[ 0x04 ] ) = 0;
	*TREINTERPRETCAST( TUINT32*, &m_aUnk80[ 0x10 ] ) = 0;
	*TREINTERPRETCAST( TUINT32*, &m_aUnk80[ 0x08 ] ) = 0;
}

TRenderPacket::~TRenderPacket()
{
}

TOSHI_NAMESPACE_END
