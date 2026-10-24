#include "pch.h"
#include "ABlobPlacingHud.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Locale/ALocaleManager.h"

#include <T2Locale/T2Locale.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 006581e0
// $deBlob: FUNCTION 006581f0
// $deBlob: FUNCTION 00658280
// $deBlob: FUNCTION 00658300
// $deBlob: FUNCTION 00658310
// $deBlob: FUNCTION 0053e480
// $deBlob: FUNCTION 00858c90
TDEFINE_CLASS( ABlobPlacingHud );

// $deBlob: FUNCTION 00658400
ABlobPlacingHud::ABlobPlacingHud()
    : m_uiPlayer( 0 )
    , m_iRank( -1 )
{
}

// $deBlob: FUNCTION 00658440
// $deBlob: FUNCTION 00658435
ABlobPlacingHud::~ABlobPlacingHud()
{
}

// $deBlob: FUNCTION 00658320
void ABlobPlacingHud::SetPlayer( TUINT8 a_uiPlayer )
{
	TASSERT( a_uiPlayer < APlayerManager::MAX_PLAYERS );

	m_uiPlayer = a_uiPlayer;
}

// $deBlob: FUNCTION 00658380
void ABlobPlacingHud::Tick( TFLOAT a_fDeltaTime )
{
	TINT iRank = AGameStatus::GetSingleton()->GetRank( m_uiPlayer );

	if ( iRank != m_iRank )
	{
		m_iRank = TINT8( iRank );

		if ( m_iRank == 1 )
			SetPresenterText( TSTATICCAST( ALocaleManager, Toshi::T2Locale::GetSingleton() )->GetString( 0xC0 ), 0, -1 );
		else
			SetPresenterText( L"", 0, -1 );
	}

	TXUIScene::Tick( a_fDeltaTime );
}
