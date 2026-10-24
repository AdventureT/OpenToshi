#include "pch.h"
#include "ABlobDebugNumberPlayers.h"
#include "GameInterface/APlayerManager.h"

#include <XUI/TXUIListItem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobDebugNumberPlayers ) == 0xF0 );

// $deBlob: FUNCTION 0063cdd0
// $deBlob: FUNCTION 0063cde0
// $deBlob: FUNCTION 0063ce60
// $deBlob: FUNCTION 0063cee0
// $deBlob: FUNCTION 0063cef0
// $deBlob: FUNCTION 0053d950
// $deBlob: FUNCTION 008589d0
TDEFINE_CLASS( ABlobDebugNumberPlayers );

// $deBlob: FUNCTION 0063cfe0
ABlobDebugNumberPlayers::ABlobDebugNumberPlayers()
{
}

// $deBlob: FUNCTION 0063d000
// $deBlob: FUNCTION 0063d005
// $deBlob: FUNCTION 0063d010
ABlobDebugNumberPlayers::~ABlobDebugNumberPlayers()
{
}

// $deBlob: FUNCTION 0063cf00
void ABlobDebugNumberPlayers::OnItemPressed( TXUIListItem* a_pItem )
{
	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		if ( a_pItem != &m_pItems[ i ] )
			continue;

		TINT iNumPlayers = TINT( i + 1 );
		TASSERT( iNumPlayers >= 0 && iNumPlayers <= APlayerManager::MAX_PLAYERS );
		APlayerManager::GetSingleton()->SetNumPlayers( iNumPlayers );
	}

	TXUICommonList::OnItemPressed( a_pItem );
}
