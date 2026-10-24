#include "pch.h"
#include "ALevelMemStatsScreen.h"
#include "GUI/AGUIManager.h"
#include "GUI/AFontManager.h"

#include <Toshi/TUtil.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ALevelMemStatsScreen::Stats ) == 0x44 );
TSTATICASSERT( sizeof( ALevelMemStatsScreen::Panel ) == 0x134 );
TSTATICASSERT( sizeof( ALevelMemStatsScreen ) == 0x9E4 );

ALevelMemStatsScreen::Stats ALevelMemStatsScreen::ms_oStats;

// $deBlob: FUNCTION 005e42c0
ALevelMemStatsScreen::ALevelMemStatsScreen()
{
	m_iMode    = 0;
	m_bVisible = TFALSE;
	TUtil::MemClear( &ms_oStats, sizeof( ms_oStats ) );
	Create();
}

// $deBlob: FUNCTION 005e4390
// $deBlob: FUNCTION 005e5620
ALevelMemStatsScreen::~ALevelMemStatsScreen()
{
	for ( TINT i = 0; i < NUM_LINES; i++ )
	{
		GetPanel( i ).oFreeBar.RemoveSelf();
		GetPanel( i ).oUsedBar.RemoveSelf();
		GetPanel( i ).oText.RemoveSelf();
		GetPanel( i ).strText = L"";
	}

	RemoveSelf();
}

// $deBlob: FUNCTION 005e45f0
void ALevelMemStatsScreen::Toggle()
{
	m_bVisible = !m_bVisible;

	if ( m_bVisible )
		AGUIManager::GetSingleton()->GetSafeArea()->AddChildTail( this );
	else
		RemoveSelf();
}

// $deBlob: FUNCTION 005e4640
void ALevelMemStatsScreen::Refresh()
{
	TINT iSize = ms_oStats.iInstancesAsset[ 1 ] + ms_oStats.iInstancesAsset[ 0 ];
	GetPanel( 0 ).strText.Format( L"Instances-ASSET: %d bytes (%d+%d)", iSize, ms_oStats.iInstancesAsset[ 0 ], ms_oStats.iInstancesAsset[ 1 ] );
	SetBar( 0, iSize, 0x500000 );

	iSize = ms_oStats.iEntitiesAsset[ 1 ] + ms_oStats.iEntitiesAsset[ 0 ];
	GetPanel( 1 ).strText.Format( L"Entities-ASSET: %d bytes (%d+%d)", iSize, ms_oStats.iEntitiesAsset[ 0 ], ms_oStats.iEntitiesAsset[ 1 ] );
	SetBar( 1, iSize, 838860 );

	iSize = ms_oStats.iFXAsset[ 1 ] + ms_oStats.iFXAsset[ 0 ];
	GetPanel( 2 ).strText.Format( L"FX-ASSET: %d bytes (%d+%d)", iSize, ms_oStats.iFXAsset[ 0 ], ms_oStats.iFXAsset[ 1 ] );
	SetBar( 2, iSize, 0x140000 );

	iSize = ms_oStats.iBlobAsset[ 1 ] + ms_oStats.iBlobAsset[ 0 ];
	GetPanel( 3 ).strText.Format( L"Blob-ASSET: %d bytes (%d+%d)", iSize, ms_oStats.iBlobAsset[ 0 ], ms_oStats.iBlobAsset[ 1 ] );
	SetBar( 3, iSize, 0x80000 );

	GetPanel( 4 ).strText.Format( L"Num Instances: %d (%d lmk, %d tr, %d bld, %d othr)", ms_oStats.iNumInstances, ms_oStats.iNumLandmarks, ms_oStats.iNumTrees, ms_oStats.iNumBuildings, ms_oStats.iNumInstances - ms_oStats.iNumBuildings - ms_oStats.iNumTrees - ms_oStats.iNumLandmarks );
	SetBar( 4, ms_oStats.iNumInstances, 1600 );

	iSize = ms_oStats.iInstancesPlaced[ 1 ] + ms_oStats.iInstancesPlaced[ 0 ];
	GetPanel( 5 ).strText.Format( L"Instances-PLACED: %d bytes (%d+%d)", iSize, ms_oStats.iInstancesPlaced[ 0 ], ms_oStats.iInstancesPlaced[ 1 ] );
	SetBar( 5, iSize, 0x400000 );

	GetPanel( 6 ).strText.Format( L"Num Entities: %d", ms_oStats.iNumEntities );
	SetBar( 6, ms_oStats.iNumEntities, 230 );

	iSize = ms_oStats.iEntitiesPlaced[ 1 ] + ms_oStats.iEntitiesPlaced[ 0 ];
	GetPanel( 7 ).strText.Format( L"Entities-PLACED: %d bytes (%d+%d)", iSize, ms_oStats.iEntitiesPlaced[ 0 ], ms_oStats.iEntitiesPlaced[ 1 ] );
	SetBar( 7, iSize, 0x180000 );

	for ( TINT i = 0; i < NUM_LINES; i++ )
	{
		const TWCHAR* wszText = GetPanel( i ).strText.GetString( 0 );
		GetPanel( i ).oText.SetText( wszText );
		GetPanel( i ).oText.SetMaxChars( -1 );
		GetPanel( i ).oText.SetWidth( AFontManager::sm_pMsgFont->GetStringWidth( wszText, 1.0f, TNULL ) );
	}
}

// $deBlob: FUNCTION 005e4d70
void ALevelMemStatsScreen::ResetStats()
{
	TUtil::MemClear( &ms_oStats, sizeof( ms_oStats ) );
}

// $deBlob: FUNCTION 005e4d90
TBOOL __stdcall ALevelMemStatsScreen::OnToggle( void* a_pUnused )
{
	ALevelMemStatsScreen* pScreen = GetSingleton();

	if ( pScreen != TNULL )
	{
		pScreen->m_bVisible = !pScreen->m_bVisible;

		if ( pScreen->m_bVisible )
			AGUIManager::GetSingleton()->GetSafeArea()->AddChildTail( pScreen );
		else
			pScreen->RemoveSelf();
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005e4df0
void ALevelMemStatsScreen::Create()
{
	TFLOAT fY = 40.0f;

	TFLOAT fWidth, fHeight;
	AGUIManager::GetSingleton()->GetSafeArea()->GetDimensions( fWidth, fHeight );
	SetDimensions( fWidth, fHeight );

	for ( TINT i = 0; i < NUM_LINES; i++ )
	{
		GetPanel( i ).oText.Create( AFontManager::GetFont( AFontManager::FONTS( 0 ) ), 0.0f );
		GetPanel( i ).oText.GetTransform().PreMultiply( T2GUITransform( 0.8f, 0.8f ) );
		GetPanel( i ).oText.SetAttachment( T2GUIATTACHMENT( m_iMode ), T2GUIATTACHMENT( m_iMode ) );
		GetPanel( i ).oText.GetTransform().SetTranslation( 0.0f, fY );
		AddChildTail( &GetPanel( i ).oText );

		GetPanel( i ).oUsedBar.Create( 0.0f, 10.0f );
		GetPanel( i ).oUsedBar.SetColour( 0xFF0000FF );
		GetPanel( i ).oUsedBar.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
		GetPanel( i ).oText.AddChildTail( &GetPanel( i ).oUsedBar );

		GetPanel( i ).oFreeBar.Create( GetWidth(), 10.0f );
		GetPanel( i ).oFreeBar.SetColour( 0xFF00FF00 );
		GetPanel( i ).oFreeBar.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPLEFT );
		GetPanel( i ).oUsedBar.AddChildTail( &GetPanel( i ).oFreeBar );

		fY += 33.0f;
	}
}

// $deBlob: FUNCTION 005e5250
void ALevelMemStatsScreen::SetBar( TINT a_iIndex, TINT a_iValue, TINT a_iMax )
{
	TFLOAT fWidth = GetWidth();
	TFLOAT fRatio = TFLOAT( a_iValue ) / TFLOAT( a_iMax );

	if ( fRatio > 1.0f )
		fRatio = 1.0f;

	GetPanel( a_iIndex ).oUsedBar.SetWidth( fRatio * fWidth );

	if ( fRatio <= 0.9f )
		GetPanel( a_iIndex ).oUsedBar.SetColour( 0xFF0000FF );
	else
		GetPanel( a_iIndex ).oUsedBar.SetColour( 0xFFFF0000 );

	GetPanel( a_iIndex ).oFreeBar.SetWidth( ( 1.0f - fRatio ) * fWidth );
}

// $deBlob: FUNCTION 005e5470
ALevelMemStatsScreen::Panel::Panel()
{
}

// $deBlob: FUNCTION 005e5550
ALevelMemStatsScreen::Panel::~Panel()
{
}

ALevelMemStatsScreen::Panel& ALevelMemStatsScreen::GetPanel( TINT a_iIndex )
{
	TASSERT( a_iIndex >= 0 && a_iIndex < NUM_LINES );
	return m_aPanels[ a_iIndex ];
}
