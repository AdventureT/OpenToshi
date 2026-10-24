#include "pch.h"
#include "ABlobTextReveal.h"

#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIScene.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobTextReveal ) == 0xA8 );

// $deBlob: FUNCTION 0062d0f0
// $deBlob: FUNCTION 0062d100
// $deBlob: FUNCTION 0062d1b0
// $deBlob: FUNCTION 0062d250
// $deBlob: FUNCTION 0062d260
// $deBlob: FUNCTION 0053d210
// $deBlob: FUNCTION 00858800
TDEFINE_CLASS( ABlobTextReveal );

// $deBlob: FUNCTION 0062d420
ABlobTextReveal::ABlobTextReveal()
    : m_fRevealInterval( 0.025f )
    , m_fRevealTimer( 0.025f )
    , m_iNumRevealed( 0 )
    , m_iLength( 0 )
{
#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail leaves m_bRevealing uninitialised
	m_bRevealing = TFALSE;
#endif
}

// $deBlob: FUNCTION 0062d4e0
// $deBlob: FUNCTION 0062d4c2
ABlobTextReveal::~ABlobTextReveal()
{
}

// $deBlob: FUNCTION 0062d270
void ABlobTextReveal::Tick( TFLOAT a_fDeltaTime )
{
	TXUI* pXUI = TXUI::GetSingleton();

	if ( pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() &&
	     m_bRevealing && m_iNumRevealed < m_iLength )
	{
		m_fRevealTimer -= a_fDeltaTime;

		if ( m_fRevealTimer <= 0.0f )
		{
			TINT iNumRevealed = m_iNumRevealed + 1;
			m_iNumRevealed    = iNumRevealed;
			m_wszText         = GetText();

			if ( m_pVisual != TNULL )
				m_pVisual->SetPresenterText( m_wszText, 0, iNumRevealed );

			m_fRevealTimer = m_fRevealInterval;
		}
	}

	TXUILabel::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0062d320
void ABlobTextReveal::SetText( const TWCHAR* a_wszText, TINT a_iParam )
{
	m_wszText = a_wszText;

	if ( m_pVisual != TNULL )
		m_pVisual->SetPresenterText( a_wszText, 0, 0 );

	m_iLength      = TStringManager::String16Length( a_wszText );
	m_fRevealTimer = m_fRevealInterval;
	m_iNumRevealed = 0;
	m_bRevealing   = TTRUE;
	TXUIScene::UpdatePlatformElements( this, TFALSE );
}

// $deBlob: FUNCTION 0062d380
void ABlobTextReveal::FinishReveal( TBOOL a_bShowAll )
{
	if ( a_bShowAll )
		m_iNumRevealed = m_iLength;
	else
		m_iNumRevealed = 0;

	TINT iNumRevealed = m_iNumRevealed;
	m_wszText         = GetText();

	if ( m_pVisual != TNULL )
		m_pVisual->SetPresenterText( m_wszText, 0, iNumRevealed );

	m_bRevealing = TFALSE;
}

// $deBlob: FUNCTION 0062d3e0
void ABlobTextReveal::RestartReveal()
{
	m_bRevealing = TTRUE;
	m_wszText    = GetText();

	if ( m_pVisual != TNULL )
		m_pVisual->SetPresenterText( m_wszText, 0, 0 );

	m_iNumRevealed = 0;
}
