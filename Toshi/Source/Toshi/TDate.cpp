#include "ToshiPCH.h"
#include "TDate.h"

#include "Toshi/T2String8.h"

#include <time.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00661440
// $Barnyard: FUNCTION 006bba20
void TDate::Pack( TUINT32& a_rOutDate, TUINT32& a_rOutTime ) const
{
	a_rOutDate = uiDay | ( ( uiMonth | ( TUINT32( uiYear ) << 8 ) ) << 8 );
	a_rOutTime = ( uiSecond | ( ( ( TUINT32( uiHour ) << 8 ) | uiMinute ) << 8 ) ) << 8;
}

// $deBlob: FUNCTION 00661490
// $Barnyard: FUNCTION 006bba60
void TDate::UnPack( const TUINT32& a_rcDate, const TUINT32& a_rcTime )
{
	uiYear   = TUINT16( a_rcDate >> 16 );
	uiMonth  = TUINT8( a_rcDate >> 8 );
	uiDay    = TUINT8( a_rcDate );
	uiHour   = TUINT8( a_rcTime >> 24 );
	uiMinute = TUINT8( a_rcTime >> 16 );
	uiSecond = TUINT8( a_rcTime >> 8 );
}

// $deBlob: FUNCTION 006614d0
TINT TDate::Format( TCHAR* a_pszOutput, TINT a_iOutputLength ) const
{
	TASSERT( a_iOutputLength >= 0 );
	return T2String8::Format( a_pszOutput, a_iOutputLength, "%2d:%02d:%02d %d/%d/%d", uiHour, uiMinute, uiSecond, uiDay, uiMonth, uiYear );
}

TOSHI_NAMESPACE_END
