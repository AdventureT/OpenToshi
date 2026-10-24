#include "pch.h"
#include "ABlobLifeHud.h"
#include "ABlobHud.h"
#include "GameInterface/APlayerManager.h"

#include <Toshi/TStringManager.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobLifeHud ) == 0x144 );

// $deBlob: FUNCTION 00656de0
// $deBlob: FUNCTION 00656df0
// $deBlob: FUNCTION 00656e70
// $deBlob: FUNCTION 00656ef0
// $deBlob: FUNCTION 00656f00
// $deBlob: FUNCTION 0053e380
// $deBlob: FUNCTION 00858c50
TDEFINE_CLASS( ABlobLifeHud );

// $deBlob: FUNCTION 006570f0
ABlobLifeHud::ABlobLifeHud()
    : m_uiPlayer( 0 )
#ifdef DEBLOB_COMMUNITY_PATCH
    , m_bLifePending( TFALSE )
#endif
{
	// NOTE: retail leaves m_bLifePending uninitialised and SetPlayer reads it
}

// $deBlob: FUNCTION 00656f10
// $deBlob: FUNCTION 00657120
// $deBlob: FUNCTION 00657110
ABlobLifeHud::~ABlobLifeHud()
{
	APlayerManager::GetSingleton()->GetBlobData( m_uiPlayer )->m_uiLifeHud = 0;
}

// $deBlob: FUNCTION 00656f90
void ABlobLifeHud::SetLives( TINT a_iLives, TBOOL a_bAnimate )
{
	TStringManager::String16Format( m_wszLives, 8, L"%d", a_iLives );
	SetPresenterText( m_wszLives, 0, -1 );

	if ( !m_bLifePending && a_bAnimate )
	{
		m_pVisual->PlayAnimation( L"ChangeLives", L"EndChangeLives" );
		ABlobHud::GetSingleton()->GetMessageHud()->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );
	}
}

// $deBlob: FUNCTION 00657010
void ABlobLifeHud::SetLifePending( TBOOL a_bPending )
{
	if ( a_bPending != m_bLifePending )
	{
		if ( a_bPending )
		{
			m_pVisual->PlayAnimation( L"StartLifePending", L"EndLifePending" );
			m_bLifePending = a_bPending;
			return;
		}

		m_pVisual->PlayAnimation( L"Normal", L"EndNormal" );
	}

	m_bLifePending = a_bPending;
}

// $deBlob: FUNCTION 00657060
void ABlobLifeHud::SetPlayer( TUINT8 a_uiPlayer )
{
	m_uiPlayer = a_uiPlayer;

	APlayerManager::BlobData* pData = APlayerManager::GetSingleton()->GetBlobData( a_uiPlayer );
	pData->m_uiLifeHud              = TREINTERPRETCAST( TUINT32, this );

	TStringManager::String16Format( m_wszLives, 8, L"%d", pData->m_uiLives );
	SetPresenterText( m_wszLives, 0, -1 );

	if ( !m_bLifePending )
	{
		m_pVisual->PlayAnimation( L"ChangeLives", L"EndChangeLives" );
		ABlobHud::GetSingleton()->GetMessageHud()->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );
	}
}
