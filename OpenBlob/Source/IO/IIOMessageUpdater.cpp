#include "pch.h"
#include "IIOMessageUpdater.h"
#include "IO/AIOEntity.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void IIOMessageUpdater::SendFinishedMessage( TINT a_iFinished )
{
	switch ( a_iFinished )
	{
		case 1: m_pOwner->SendInputMessage( 0xC, TNULL ); return;
		case 2: m_pOwner->SendInputMessage( 0xD, TNULL ); return;
		case 3: m_pOwner->SendInputMessage( 0xE, TNULL ); return;
		case 4: m_pOwner->SendInputMessage( 0xF, TNULL ); return;
		case 5: m_pOwner->SendInputMessage( 0x4A, TNULL ); return;
		case 6: m_pOwner->SendInputMessage( 0x4B, TNULL ); return;
		case 7: m_pOwner->SendInputMessage( 0x4C, TNULL ); return;
		case 8: m_pOwner->SendInputMessage( 0x4D, TNULL ); return;
	}
}
