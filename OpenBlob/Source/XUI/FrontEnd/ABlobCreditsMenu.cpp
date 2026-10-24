#include "pch.h"
#include "ABlobCreditsMenu.h"
#include "ABlobFrontEnd.h"

#include <T2Locale/T2Locale.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobCreditsMenu ) == 0x210 );

// $deBlob: FUNCTION 0062e6e0
// $deBlob: FUNCTION 0062e6f0
// $deBlob: FUNCTION 0062e760
// $deBlob: FUNCTION 0062e7d0
// $deBlob: FUNCTION 0062e7e0
// $deBlob: FUNCTION 0053d310
// $deBlob: FUNCTION 00858840
TDEFINE_CLASS( ABlobCreditsMenu );

// $deBlob: FUNCTION 0062e7f0
ABlobCreditsMenu::ABlobCreditsMenu()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopExtrasB )
{
	m_pBlocks    = TNULL;
	m_iBlock     = 0;
	m_iLine      = 0;
	m_iNextName  = 0;
	m_iNextTitle = 0;
	m_bEnding    = TFALSE;
	m_bClosed    = TFALSE;
}

// $deBlob: FUNCTION 0062e890
// $deBlob: FUNCTION 0062ef0e
// $deBlob: FUNCTION 0062ef20
ABlobCreditsMenu::~ABlobCreditsMenu()
{
	m_oTRB.Close();

	for ( TINT i = 0; i < NUM_NAME_SCENES; i++ )
	{
		if ( m_apNames[ i ] != TNULL )
			delete m_apNames[ i ];
	}

	for ( TINT i = 0; i < NUM_TITLE_SCENES; i++ )
	{
		if ( m_apTitles[ i ] != TNULL )
			delete m_apTitles[ i ];
	}
}

// $deBlob: FUNCTION 0062ec40
void ABlobCreditsMenu::Tick( TFLOAT a_fDeltaTime )
{

	if ( TXUI::GetSingleton()->GetTransToScene() == TNULL && TXUI::GetSingleton()->GetTransFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() )
	{
		Blocks* pBlocks = m_pBlocks;

		if ( m_iBlock < pBlocks->m_iNumBlocks )
		{
			UpdateLines( a_fDeltaTime );
		}
		else if ( !m_bEnding )
		{
			// Waits for the last names to scroll away
			TBOOL bEnding = TTRUE;

			for ( TINT i = 0; i < NUM_NAME_SCENES; i++ )
			{
				if ( m_apNames[ i ]->GetParent() != TNULL && m_apNames[ i ]->GetTimelineControl().IsPlaying() )
				{
					bEnding = TFALSE;
					break;
				}
			}

			m_bEnding = bEnding;
			m_fTimer  = pBlocks->m_fEndDelay;
		}
		else
		{
			m_fTimer -= a_fDeltaTime;

			if ( m_fTimer <= 0.0f )
			{
				m_bClosed = TTRUE;
				TXUI::GetSingleton()->TransitionBack( this );
			}
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0062e950
void ABlobCreditsMenu::OnCreated()
{
	ABlobMenu::OnCreated();

	TTRB::ERROR result = m_oTRB.Load( "Data\\credits.trb" );
	TASSERT( result == TTRB::ERROR_OK );

	m_pBlocks = TSTATICCAST( Blocks, m_oTRB.GetSymbolAddress( "Blocks" ) );

	for ( TINT i = 0; i < m_pBlocks->m_iNumBlocks; i++ )
	{
		Block* pBlock      = m_pBlocks->m_ppBlocks[ i ];
		pBlock->m_wszTitle = T2Locale::GetSingleton()->GetString( pBlock->m_iTitleId );

		for ( TINT j = 0; j < pBlock->m_iNumLines; j++ )
			pBlock->m_pwszLines[ j ] = T2Locale::GetSingleton()->GetString( pBlock->m_piLineIds[ j ] );
	}

	TXUIResource* pResource = TXUI::GetSingleton()->FindResource( "credits.xur" );

	for ( TINT i = 0; i < NUM_NAME_SCENES; i++ )
	{
		m_apNames[ i ] = TXUI::GetSingleton()->CreateScene( pResource, L"NameScene", TFALSE );
		m_apNames[ i ]->RemoveSelf();
	}

	for ( TINT i = 0; i < NUM_TITLE_SCENES; i++ )
	{
		m_apTitles[ i ] = TXUI::GetSingleton()->CreateScene( pResource, L"TitleScene", TFALSE );
		m_apTitles[ i ]->RemoveSelf();
	}
}

// $deBlob: FUNCTION 0062eb20
void ABlobCreditsMenu::TransTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );

	if ( pFrontEnd != TNULL )
	{
		TASSERT( -1 != m_iBackgroundMovie );
		pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransExtrasAExtrasB, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
	}

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 0062eb90
void ABlobCreditsMenu::EndTransTo()
{
	// The first block's title
	AddLine( m_pBlocks->m_ppBlocks[ m_iBlock ]->m_wszTitle, TTRUE );

	m_iLine  = 0;
	m_fTimer = m_pBlocks->m_fTitleDelay;

	ABlobMenu::EndTransTo();
}

// $deBlob: FUNCTION 0062ed30
void ABlobCreditsMenu::AddLine( const TWCHAR* a_wszText, TBOOL a_bTitle )
{
	TXUIScene* pScene = a_bTitle ? m_apTitles[ m_iNextTitle ] : m_apNames[ m_iNextName ];

	AddChildTail( pScene );
	pScene->FindChildXUIControl( L"Name" )->SetText( a_wszText, -1 );
	pScene->StopAnimation();
	pScene->PlayAnimation( L"Scroll", L"EndScroll" );

	if ( a_bTitle )
		m_iNextTitle++;
	else
		m_iNextName++;

	// NOTE: retail only wraps the title index while the name index is below 32
	if ( m_iNextName < NUM_NAME_SCENES )
	{
		if ( m_iNextTitle >= NUM_TITLE_SCENES )
			m_iNextTitle = 0;
	}
	else
	{
		m_iNextName = 0;
	}
}

// $deBlob: FUNCTION 0062ede0
void ABlobCreditsMenu::UpdateLines( TFLOAT a_fDeltaTime )
{
	m_fTimer -= a_fDeltaTime;

	if ( m_fTimer > 0.0f )
		return;

	Blocks* pBlocks   = m_pBlocks;
	TINT    iNumLines = pBlocks->m_ppBlocks[ m_iBlock ]->m_iNumLines;

	if ( m_iLine == iNumLines )
	{
		// The next block's title
		m_iBlock++;

		if ( m_iBlock < pBlocks->m_iNumBlocks )
		{
			AddLine( pBlocks->m_ppBlocks[ m_iBlock ]->m_wszTitle, TTRUE );
			m_iLine  = 0;
			m_fTimer = m_pBlocks->m_fTitleDelay;
		}
	}
	else
	{
		m_fTimer = ( m_iLine == iNumLines - 1 ) ? pBlocks->m_fLastLineDelay : pBlocks->m_fLineDelay;

		const TWCHAR* wszLine = pBlocks->m_ppBlocks[ m_iBlock ]->m_pwszLines[ m_iLine ];
		m_iLine++;
		AddLine( wszLine, TFALSE );
	}
}
