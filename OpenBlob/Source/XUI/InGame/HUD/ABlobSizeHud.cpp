#include "pch.h"
#include "ABlobSizeHud.h"
#include "ABlobHud.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "GameInterface/APlayerManager.h"

#include <Math/TMathInline.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobSizeHud ) == 0x1B0 );

// Splits an ARGB colour into its components
// $deBlob: FUNCTION 00580880
static TUINT32* SplitColour( TUINT32 a_uiColour, TUINT32* a_pRed, TUINT32* a_pGreen, TUINT32* a_pBlue, TUINT32* a_pAlpha )
{
	*a_pBlue  = a_uiColour & 0xFF;
	*a_pGreen = ( a_uiColour >> 8 ) & 0xFF;
	*a_pRed   = ( a_uiColour >> 16 ) & 0xFF;
	*a_pAlpha = a_uiColour >> 24;
	return a_pAlpha;
}

static TUINT32 JoinColour( TUINT32 a_uiRed, TUINT32 a_uiGreen, TUINT32 a_uiBlue, TUINT32 a_uiAlpha )
{
	return ( ( ( a_uiAlpha << 8 | a_uiRed ) << 8 | a_uiGreen ) << 8 ) | a_uiBlue;
}

// $deBlob: FUNCTION 0065a160
// $deBlob: FUNCTION 0065a170
// $deBlob: FUNCTION 0065a1e0
// $deBlob: FUNCTION 0065a250
// $deBlob: FUNCTION 0065a260
// $deBlob: FUNCTION 0053e580
// $deBlob: FUNCTION 00858cd0
TDEFINE_CLASS( ABlobSizeHud );

// $deBlob: FUNCTION 0065ac20
ABlobSizeHud::ABlobSizeHud()
    : m_iSize( -1 )
    , m_uiColour( 0 )
    , m_pBlob( TNULL )
    , m_pTop( TNULL )
    , m_pBody( TNULL )
    , m_pRainbowBlob( TNULL )
    , m_pUnk168( TNULL )
    , m_pPaintLeftText( TNULL )
    , m_uiPlayer( 0 )
    , m_iPaintLeft( -1 )
    , m_pLandmarkShake( TNULL )
    , m_pProgressBody( TNULL )
    , m_pPaint( TNULL )
    , m_fProgressEmpty( 0.0f )
    , m_fProgressFull( 0.0f )
    , m_bExitLandmarkPending( TFALSE )
    , m_pPaintMeter( TNULL )
    , m_pBlob1( TNULL )
{
	for ( TINT i = 0; i < NUM_COLOURED; i++ )
		m_apColoured[ i ] = TNULL;
}

// $deBlob: FUNCTION 0065a270
// $deBlob: FUNCTION 0065ad20
// $deBlob: FUNCTION 0065ad0f
ABlobSizeHud::~ABlobSizeHud()
{
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( m_uiPlayer ) );

	if ( pBlob != TNULL )
		pBlob->SetSizeHud( TNULL );
}

// $deBlob: FUNCTION 0065a2f0
void ABlobSizeHud::OnCreated()
{
	TXUIVisual* pVisual = m_pVisual;

	if ( pVisual != TNULL )
	{
		m_pBlob        = TSTATICCAST( TXUIFigure, pVisual->FindChildXUIElement( L"Blob" ) );
		m_pRainbowBlob = pVisual->FindChildXUIElement( L"RainbowBlob" );
		m_pBlob->SetVisible( TTRUE );
		m_pRainbowBlob->SetVisible( TFALSE );

		m_pPaintMeter = TSTATICCAST( TXUIGroup, pVisual->FindChildXUIElement( L"BlobPaintMeter" ) );
		m_pPaintMeter->StopAnimation();
		m_pBlob1 = TSTATICCAST( TXUIGroup, m_pPaintMeter->FindChildXUIElement( L"Blob1" ) );
		m_pBlob1->StopAnimation();
		m_pTop  = TSTATICCAST( TXUIFigure, m_pBlob1->FindChildXUIElement( L"top" ) );
		m_pBody = TSTATICCAST( TXUIFigure, m_pBlob1->FindChildXUIElement( L"body" ) );

		m_fMeterEmpty = m_pPaintMeter->GetTimelineControl()->GetNamedFrameTime( L"Empty" );
		m_fMeterFull  = m_pPaintMeter->GetTimelineControl()->GetNamedFrameTime( L"Full" );
	}

	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
	{
		m_pUnk168 = FindChildXUIElement( 0xB9 );
		m_pUnk168->SetVisible( TFALSE );
		m_pPaintLeftText = TSTATICCAST( TXUIText, FindChildXUIElement( 0xBD ) );

		m_pLandmarkShake = FindChildXUIControl( L"LandmarkShake" );
		m_pProgressBody  = TSTATICCAST( TXUIGroup, m_pLandmarkShake->FindChildXUIElement( L"ProgressBody" ) );
		m_pProgressBody->StopAnimation();
		m_pPaint = TSTATICCAST( TXUIFigure, m_pProgressBody->FindChildXUIElement( L"Paint" ) );

		m_fProgressEmpty = m_pProgressBody->GetTimelineControl()->GetNamedFrameTime( L"Empty" );
		m_fProgressFull  = m_pProgressBody->GetTimelineControl()->GetNamedFrameTime( L"Full" );
	}
}

// $deBlob: FUNCTION 0065a490
void ABlobSizeHud::SetSize( TINT a_iSize )
{
	if ( a_iSize == m_iSize )
		return;

	m_iSize = a_iSize;
	TStringManager::String16Format( m_wszSize, 8, L"%d", a_iSize );
	SetPresenterText( m_wszSize, 0, -1 );

	m_pPaintMeter->GetTimelineControl()->SetTimeLineTime( ( m_fMeterFull - m_fMeterEmpty ) * ( TFLOAT( a_iSize ) / 100.0f ) + m_fMeterEmpty, TTRUE );
	m_pBlob1->GetTimelineControl()->SetTimeLineTime( ( m_fMeterFull - m_fMeterEmpty ) * ( TFLOAT( a_iSize ) / 100.0f ) + m_fMeterEmpty, TTRUE );
}

// $deBlob: FUNCTION 0065a560
void ABlobSizeHud::SetColour( TUINT32 a_uiColour, TBOOL a_bLandmark, TFLOAT a_fPaintLeft )
{
	TUINT32 uiRed, uiGreen, uiBlue, uiAlpha;

	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
	{
		if ( m_pBlob == TNULL )
			return;

		if ( a_bLandmark == TTRUE )
		{
			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

			if ( m_pPaint != TNULL && pBlob->GetBlobState() == ABlobEntity::STATE_LMARK )
			{
				SplitColour( a_uiColour, &uiRed, &uiGreen, &uiBlue, &uiAlpha );
				m_pPaint->SetFillColour( JoinColour( uiRed, uiGreen, uiBlue, uiAlpha ) );
			}

			m_uiColour = 0;

			if ( !m_pRainbowBlob->IsVisible() )
			{
				m_pRainbowBlob->SetVisible( TTRUE );
				m_pBlob->SetVisible( TFALSE );
				m_pTop->SetVisible( TFALSE );
				m_pBody->SetVisible( TFALSE );
				m_pUnk168->SetVisible( TTRUE );
			}

			TINT iPaintLeft = TINT( a_fPaintLeft );

			if ( iPaintLeft != m_iPaintLeft )
			{
				m_iPaintLeft = iPaintLeft;
				TStringManager::String16Format( m_wszPaintLeft, L"%d", iPaintLeft );
				m_pPaintLeftText->SetText( m_wszPaintLeft );
			}
		}
		else if ( a_uiColour != m_uiColour )
		{
			if ( !m_pBlob->IsVisible() )
			{
				m_pBlob->SetVisible( TTRUE );
				m_pTop->SetVisible( TTRUE );
				m_pBody->SetVisible( TTRUE );
				m_pRainbowBlob->SetVisible( TFALSE );
				m_pUnk168->SetVisible( TFALSE );
			}

			m_uiColour = a_uiColour;
			SplitColour( a_uiColour, &uiRed, &uiGreen, &uiBlue, &uiAlpha );
			TUINT32 uiColour = JoinColour( uiRed, uiGreen, uiBlue, uiAlpha );

			m_pBlob->SetFillColour( uiColour );
			m_pTop->SetFillColour( uiColour );
			m_pBody->SetFillColour( uiColour );

			for ( TINT i = 0; i < NUM_COLOURED; i++ )
			{
				if ( m_apColoured[ i ] != TNULL )
					m_apColoured[ i ]->SetColour( uiColour );
			}

			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

			if ( m_pPaint != TNULL && pBlob->GetBlobState() == ABlobEntity::STATE_LMARK )
				m_pPaint->SetFillColour( uiColour );
		}
	}
	else if ( m_pBlob != TNULL && a_uiColour != m_uiColour )
	{
		m_uiColour = a_uiColour;

		// NOTE: retail splits the colour inline here
		TUINT32 uiColour = JoinColour( ( a_uiColour >> 16 ) & 0xFF, ( a_uiColour >> 8 ) & 0xFF, a_uiColour & 0xFF, a_uiColour >> 24 );

		m_pBlob->SetFillColour( uiColour );
		m_pTop->SetFillColour( uiColour );
		m_pBody->SetFillColour( uiColour );

		for ( TINT i = 0; i < NUM_COLOURED; i++ )
		{
			if ( m_apColoured[ i ] != TNULL )
				m_apColoured[ i ]->SetColour( uiColour );
		}
	}
}

// $deBlob: FUNCTION 0065a8b0
void ABlobSizeHud::SetPlayer( TUINT8 a_uiPlayer )
{
	TASSERT( a_uiPlayer < APlayerManager::MAX_PLAYERS );
	m_uiPlayer = a_uiPlayer;

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( a_uiPlayer ) );

	if ( pBlob != TNULL )
		pBlob->SetSizeHud( this );
}

// $deBlob: FUNCTION 0065a910
void ABlobSizeHud::Reset()
{
	m_pBlob->SetVisible( TTRUE );
	m_pTop->SetVisible( TTRUE );
	m_pBody->SetVisible( TTRUE );
	m_pRainbowBlob->SetVisible( TFALSE );

	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
	{
		if ( m_pUnk168 != TNULL )
			m_pUnk168->SetVisible( TFALSE );

		if ( m_pLandmarkShake != TNULL )
			m_pLandmarkShake->PlayAnimation( L"Normal", L"EndNormal" );
	}

	m_bExitLandmarkPending = TFALSE;

	if ( m_pPaintMeter != TNULL )
		m_pPaintMeter->GetTimelineControl()->SetTimeLineTime( m_fMeterEmpty, TTRUE );

	if ( m_pBlob1 != TNULL )
		m_pBlob1->GetTimelineControl()->SetTimeLineTime( m_fMeterEmpty, TTRUE );
}

// $deBlob: FUNCTION 0065a9d0
void ABlobSizeHud::GrowLandmarkMeter( TUINT32 a_uiColour, TINT a_iPaintLeft, TINT a_iTotal )
{
	if ( m_pLandmarkShake == TNULL )
		return;

	// NOTE: retail splits the colour inline here
	m_pPaint->SetFillColour( JoinColour( ( a_uiColour >> 16 ) & 0xFF, ( a_uiColour >> 8 ) & 0xFF, a_uiColour & 0xFF, a_uiColour >> 24 ) );

	TFLOAT fProgress = 1.0f - TFLOAT( a_iPaintLeft ) / TFLOAT( a_iTotal );
	TMath::Clip( fProgress, 0.0f, 1.0f );
	m_pProgressBody->GetTimelineControl()->SetTimeLineTime( ( m_fProgressFull - m_fProgressEmpty ) * fProgress + m_fProgressEmpty, TTRUE );

	m_pLandmarkShake->PlayAnimation( L"MeterGrow", L"EndMeterGrow" );
}

// $deBlob: FUNCTION 0065aae0
void ABlobSizeHud::ExitLandmark()
{
	if ( m_pLandmarkShake != TNULL )
		m_pLandmarkShake->PlayAnimation( L"ExitLandmark", L"EndExitLandmark" );
}

// $deBlob: FUNCTION 0065ab00
void ABlobSizeHud::SetLandmarkProgress( TINT a_iPaintLeft, TINT a_iTotal )
{
	if ( m_pProgressBody == TNULL )
		return;

	TFLOAT fProgress = 1.0f - TFLOAT( a_iPaintLeft ) / TFLOAT( a_iTotal );
	TMath::Clip( fProgress, 0.0f, 1.0f );
	m_pProgressBody->GetTimelineControl()->SetTimeLineTime( ( m_fProgressFull - m_fProgressEmpty ) * fProgress + m_fProgressEmpty, TTRUE );

	m_pLandmarkShake->PlayAnimation( L"Paint", L"EndPaint" );
}

// $deBlob: FUNCTION 0065aba0
void ABlobSizeHud::Tick( TFLOAT a_fDeltaTime )
{

	if ( m_pLandmarkShake != TNULL && m_bExitLandmarkPending && TXUI::GetSingleton()->GetTransToScene() == TNULL &&
	     TXUI::GetSingleton()->GetTransFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() && TXUI::GetSingleton()->GetActiveScene() == ABlobHud::GetSingleton() )
	{
		m_bExitLandmarkPending = TFALSE;
		m_pLandmarkShake->PlayAnimation( L"ExitLandmark", L"EndExitLandmark" );
	}

	TXUIScene::Tick( a_fDeltaTime );
}
