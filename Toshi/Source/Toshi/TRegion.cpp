#include "ToshiPCH.h"
#include "TRegion.h"

TOSHI_NAMESPACE_START

static TREGION s_ToshiRegion;
static TBOOL   s_bRegionLocked;

// $deBlob: FUNCTION 00662d20
TREGION TRegion_GetRegion()
{
	return s_ToshiRegion;
}

// $deBlob: FUNCTION 00662cd0
void TRegion_SetRegion( TREGION a_eRegion )
{
	TASSERT( !s_bRegionLocked );
	s_ToshiRegion = a_eRegion;
}

// $deBlob: FUNCTION 00662d30
void TRegion_LockRegion()
{
	s_bRegionLocked = TTRUE;
}

void TRegion_UnlockRegion()
{
	s_bRegionLocked = TFALSE;
}

TOSHI_NAMESPACE_END
