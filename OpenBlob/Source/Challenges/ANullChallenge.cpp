#include "pch.h"
#include "ANullChallenge.h"
#include "AChallengeManager.h"
#include "XUI/InGame/HUD/ABlobHud.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0059c7c0
ANullChallenge::ANullChallenge()
    : AChallenge( Type_Null )
{

}

// $deBlob: FUNCTION 005994f0
void ANullChallenge::Complete()
{
	// The Professor's intro challenge (string 0x3ae) brings up the first HUD showcase
	if ( ABlobHud::GetSingleton() != TNULL && m_pData->m_iStringID == 0x3ae )
		ABlobHud::GetSingleton()->SetPendingShowcase( 1 );

	AChallenge::Complete();
}
