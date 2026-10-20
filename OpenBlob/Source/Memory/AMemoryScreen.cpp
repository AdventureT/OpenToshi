#include "pch.h"
#include "AMemoryScreen.h"
#include "AMemory.h"
#include "GUI/AGUIManager.h"
#include "GUI/AFontManager.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Common/Base/Memory/Memory/hkMemory.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AMemoryScreen::Panel ) == 0x194 );
TSTATICASSERT( sizeof( AMemoryScreen ) == 0x69C );

// $deBlob: FUNCTION 005e60e0
AMemoryScreen::AMemoryScreen()
{
	m_fUpdateTimer = 0.1f;
	m_eFont        = 0;
	m_iMode        = 0;
	m_bVisible     = TFALSE;
}

// $deBlob: FUNCTION 005e61a0
// $deBlob: FUNCTION 005e72c0
AMemoryScreen::~AMemoryScreen()
{
	for ( TINT i = 0; i < NUM_POOLS; i++ )
	{
		GetPanel( i ).oUnk134.RemoveSelf();
		GetPanel( i ).oFreeBar.RemoveSelf();
		GetPanel( i ).oUsedBar.RemoveSelf();
		GetPanel( i ).oText.RemoveSelf();
	}

	ClearTexts();
}

// $deBlob: FUNCTION 005e6420
void AMemoryScreen::Create()
{
	TFLOAT fWidth, fHeight;
	AGUIManager::GetSingleton()->GetSafeArea()->GetDimensions( fWidth, fHeight );
	SetDimensions( fWidth, fHeight );

	for ( TINT i = 0; i < NUM_POOLS; i++ )
	{
		GetPanel( i ).oText.Create( AFontManager::GetFont( AFontManager::FONTS( m_eFont ) ), 0.0f );
		GetPanel( i ).oText.GetTransform().PreMultiply( T2GUITransform( 0.8f, 0.8f ) );
		AddChildTail( &GetPanel( i ).oText );

		GetPanel( i ).oUsedBar.Create( 0.0f, 10.0f );
		GetPanel( i ).oUsedBar.SetColour( 0xFF0000FF );
		GetPanel( i ).oUsedBar.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
		GetPanel( i ).oText.AddChildTail( &GetPanel( i ).oUsedBar );

		GetPanel( i ).oFreeBar.Create( GetWidth(), 10.0f );
		GetPanel( i ).oFreeBar.SetColour( 0xFF00FF00 );
		GetPanel( i ).oFreeBar.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPLEFT );
		GetPanel( i ).oUsedBar.AddChildTail( &GetPanel( i ).oFreeBar );

		GetPanel( i ).oUnk134.Create( 8.0f, 16.0f );
		GetPanel( i ).oUnk134.SetColour( 0xFFFFFFFF );
		GetPanel( i ).oUnk134.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
	}
}

// $deBlob: FUNCTION 005e68c0
void AMemoryScreen::Tick( TFLOAT a_fDeltaTime )
{
	m_fUpdateTimer -= a_fDeltaTime;

	if ( m_fUpdateTimer < 0.0f )
	{
		m_fUpdateTimer += 0.1f;

		if ( m_bVisible == TTRUE )
			UpdatePools();
	}
}

// $deBlob: FUNCTION 005e6910
void AMemoryScreen::ClearTexts()
{
	for ( TINT i = 0; i < NUM_POOLS; i++ )
		GetPanel( i ).strText = L"";
}

// $deBlob: FUNCTION 005e6980
void AMemoryScreen::SetMode( TINT a_iMode )
{
	m_iMode = a_iMode;
	UpdateLayout();
}

// $deBlob: FUNCTION 005e69a0
void AMemoryScreen::Toggle()
{
	m_bVisible = !m_bVisible;

	if ( m_bVisible )
	{
		UpdateLayout();
		AGUIManager::GetSingleton()->GetSafeArea()->AddChildTail( this );
	}
	else
	{
		RemoveSelf();
	}
}

// $deBlob: FUNCTION 005e69f0
TBOOL __stdcall AMemoryScreen::OnToggle( void* a_pUnused )
{
	AMemoryScreen* pScreen = GetSingleton();
	pScreen->m_bVisible    = !pScreen->m_bVisible;

	if ( pScreen->m_bVisible )
	{
		pScreen->UpdateLayout();
		AGUIManager::GetSingleton()->GetSafeArea()->AddChildTail( pScreen );
	}
	else
	{
		pScreen->RemoveSelf();
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005e6a50
TBOOL __stdcall AMemoryScreen::OnTopLeft( void* a_pUnused )
{
	AMemoryScreen* pScreen = GetSingleton();
	pScreen->m_iMode       = T2GUIATTACHMENT_TOPLEFT;
	pScreen->UpdateLayout();
	return TTRUE;
}

// $deBlob: FUNCTION 005e6a70
TBOOL __stdcall AMemoryScreen::OnTopRight( void* a_pUnused )
{
	AMemoryScreen* pScreen = GetSingleton();
	pScreen->m_iMode       = T2GUIATTACHMENT_TOPRIGHT;
	pScreen->UpdateLayout();
	return TTRUE;
}

// $deBlob: FUNCTION 005e6a90
TBOOL __stdcall AMemoryScreen::OnBottomLeft( void* a_pUnused )
{
	AMemoryScreen* pScreen = GetSingleton();
	pScreen->m_iMode       = T2GUIATTACHMENT_BOTTOMLEFT;
	pScreen->UpdateLayout();
	return TTRUE;
}

// $deBlob: FUNCTION 005e6ab0
TBOOL __stdcall AMemoryScreen::OnBottomRight( void* a_pUnused )
{
	AMemoryScreen* pScreen = GetSingleton();
	pScreen->m_iMode       = T2GUIATTACHMENT_BOTTOMRIGHT;
	pScreen->UpdateLayout();
	return TTRUE;
}

// $deBlob: FUNCTION 005e6ad0
TBOOL __stdcall AMemoryScreen::OnCentre( void* a_pUnused )
{
	AMemoryScreen* pScreen = GetSingleton();
	pScreen->m_iMode       = T2GUIATTACHMENT_MIDDLECENTER;
	pScreen->UpdateLayout();
	return TTRUE;
}

// $deBlob: FUNCTION 005e6af0
void AMemoryScreen::UpdatePools()
{
	TMemoryHeapInfo oInfo;

	for ( TINT i = 0; i < AMemory::POOL_NumOf; i++ )
	{
		TMemoryHeap_GetInfo( AMemory::ms_apMemoryBlocks[ i ], &oInfo );
		SetPoolInfo( i, AMemory::ms_aPoolDefinitionInfo[ i ].m_szName, oInfo );
	}

	TMemoryHeap_GetInfo( TMemory_GetGlobalHeap(), &oInfo );
	SetPoolInfo( 2, "Mem1", oInfo );

	if ( TPhysicsManager::GetSingleton()->IsInitialised() )
	{
		hkMemoryStatistics oStats;
		TPhysicsManager::GetSingleton()->GetMemoryStatistics( oStats );

		oInfo.uiHeapSize       = TUINT( oStats.m_allocated );
		oInfo.uiFree           = TUINT( oStats.m_available );
		oInfo.uiUsed           = TUINT( oStats.m_used );
		oInfo.uiNumAllocations = 0;
		SetPoolInfo( 3, "PHYSICS", oInfo );
	}

	UpdateTextWidths();
}

// $deBlob: FUNCTION 005e6bc0
void AMemoryScreen::UpdateTextWidths()
{
	for ( TINT i = 0; i < NUM_POOLS; i++ )
	{
		const TWCHAR* wszText = GetPanel( i ).strText.GetString( 0 );
		GetPanel( i ).oText.SetText( wszText );
		GetPanel( i ).oText.SetMaxChars( -1 );
		GetPanel( i ).oText.SetWidth( AFontManager::sm_pMsgFont->GetStringWidth( wszText, 1.0f, TNULL ) );
	}
}

// $deBlob: FUNCTION 005e6ce0
void AMemoryScreen::UpdateLayout()
{
	TFLOAT fY = 0.0f;

	for ( TINT i = NUM_POOLS - 1; i >= 0; i-- )
	{
		GetPanel( i ).oText.SetAttachment( T2GUIATTACHMENT( m_iMode ), T2GUIATTACHMENT( m_iMode ) );
		GetPanel( i ).oText.GetTransform().SetTranslation( 0.0f, fY );

		// The bottom modes stack the lines upwards
		if ( m_iMode == T2GUIATTACHMENT_BOTTOMLEFT || m_iMode == T2GUIATTACHMENT_BOTTOMRIGHT )
			fY -= 33.0f;
		else
			fY += 33.0f;
	}
}

// $deBlob: FUNCTION 005e6dd0
void AMemoryScreen::SetPoolInfo( TINT a_iIndex, const TCHAR* a_szName, const TMemoryHeapInfo& a_rInfo )
{
	TWCHAR wszName[ 32 ];
	TStringManager::StringCharToUnicode( wszName, a_szName, -1 );

	GetPanel( a_iIndex ).strText.Format( L"Pool %ls: Free=%d, Used=%d, Total=%d, Alloc=%d", wszName, a_rInfo.uiFree, a_rInfo.uiUsed, a_rInfo.uiHeapSize, a_rInfo.uiNumAllocations );
	GetPanel( a_iIndex ).oUsedBar.SetWidth( ( GetWidth() * TFLOAT( a_rInfo.uiUsed ) ) / TFLOAT( a_rInfo.uiHeapSize ) );

	if ( TFLOAT( a_rInfo.uiUsed ) / TFLOAT( a_rInfo.uiHeapSize ) <= 0.9f )
		GetPanel( a_iIndex ).oUsedBar.SetColour( 0xFF0000FF );
	else
		GetPanel( a_iIndex ).oUsedBar.SetColour( 0xFFFF0000 );

	GetPanel( a_iIndex ).oFreeBar.SetWidth( ( GetWidth() * TFLOAT( a_rInfo.uiFree ) ) / TFLOAT( a_rInfo.uiHeapSize ) );
}

// $deBlob: FUNCTION 005e7100
AMemoryScreen::Panel::Panel()
{
}

// $deBlob: FUNCTION 005e71f0
AMemoryScreen::Panel::~Panel()
{
}

AMemoryScreen::Panel& AMemoryScreen::GetPanel( TINT a_iIndex )
{
	TASSERT( a_iIndex >= 0 && a_iIndex < NUM_POOLS );
	return m_aPanels[ a_iIndex ];
}
