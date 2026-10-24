#include "pch.h"
#include "AAudioMemoryHudElement.h"
#include "GUI/AGUIManager.h"
#include "GUI/AFontManager.h"

#include <Toshi/TStringManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TFLOAT s_fTextScale   = 1.0f;
static TFLOAT s_fColumnWidth = 250.0f;

// $deBlob: FUNCTION 005b1130
AAudioMemoryHudElement::AAudioMemoryHudElement()
    : m_bVisible( TFALSE )
{
	m_iAvailable = 0xB00000;
	Reset();
	CreateUsageText();
	SetHudVisible( m_bVisible );
}

// $deBlob: FUNCTION 005b1860
// $deBlob: FUNCTION 005b1950
AAudioMemoryHudElement::~AAudioMemoryHudElement()
{
}

// $deBlob: FUNCTION 005b11e0
void AAudioMemoryHudElement::AddEntry( const TCHAR* a_szName, TINT a_iUsage, TINT a_iSize )
{
	T2GUIElement* pParent = ( m_iNumEntries != 0 ) ? TSTATICCAST( T2GUIElement, &m_aEntries[ m_iNumEntries - 1 ].m_oNameText ) : &m_oUsageText;

	if ( m_iNumEntries == MAX_NUM_ENTRIES )
		return;

	Entry& rEntry = m_aEntries[ m_iNumEntries++ ];

	rEntry.m_oPercentageText.SetScale( s_fTextScale );
	rEntry.m_oSizeText.SetScale( s_fTextScale );
	rEntry.m_oNameText.SetScale( s_fTextScale );

	rEntry.m_oPercentageText.Create( AFontManager::sm_pMsgFont, s_fColumnWidth );
	rEntry.m_oSizeText.Create( AFontManager::sm_pMsgFont, s_fColumnWidth );
	rEntry.m_oNameText.Create( AFontManager::sm_pMsgFont, s_fColumnWidth );

	rEntry.m_oPercentageText.SetJustify( T2GUIFont::JUSTIFY_LEFT );
	rEntry.m_oSizeText.SetJustify( T2GUIFont::JUSTIFY_LEFT );
	rEntry.m_oNameText.SetJustify( T2GUIFont::JUSTIFY_LEFT );

	rEntry.m_oPercentageText.SetColour( 0xFFFFFF00 );
	rEntry.m_oSizeText.SetColour( 0xFFFFFF00 );
	rEntry.m_oNameText.SetColour( 0xFFFFFF00 );

	rEntry.m_oPercentageText.SetDimensions( 50.0f, s_fTextScale * 20.0f, TTRUE );
	rEntry.m_oSizeText.SetDimensions( 150.0f, s_fTextScale * 20.0f, TTRUE );
	rEntry.m_oNameText.SetDimensions( 350.0f, s_fTextScale * 20.0f, TTRUE );

	rEntry.m_oPercentageText.SetVisible( m_bVisible );
	rEntry.m_oSizeText.SetVisible( m_bVisible );
	rEntry.m_oNameText.SetVisible( m_bVisible );

	rEntry.m_oPercentageText.SetMaxChars( -1 );
	rEntry.m_oPercentageText.SetText( rEntry.m_wszPercentage );
	rEntry.m_oSizeText.SetText( rEntry.m_wszSize );
	rEntry.m_oSizeText.SetMaxChars( -1 );
	rEntry.m_oNameText.SetText( rEntry.m_wszName );
	rEntry.m_oNameText.SetMaxChars( -1 );

	rEntry.m_oNameText.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
	rEntry.m_oSizeText.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	rEntry.m_oPercentageText.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );

	pParent->AddChildTail( &rEntry.m_oNameText );
	rEntry.m_oNameText.AddChildTail( &rEntry.m_oSizeText );
	rEntry.m_oSizeText.AddChildTail( &rEntry.m_oPercentageText );

	TINT iSize = a_iUsage - m_iUsage;

	if ( a_iSize > 0 )
		iSize = a_iSize;

	rEntry.m_iSize = iSize;
	TStringManager::String16Format( rEntry.m_wszSize, 64, L"Size %d", iSize );
	TStringManager::StringCharToUnicode( rEntry.m_wszName, a_szName, 256 );

	m_iUsage = a_iUsage;
	UpdateUsageText();
	UpdatePercentages();
}

// $deBlob: FUNCTION 005b14e0
void AAudioMemoryHudElement::Reset()
{
	m_iUsage      = 0;
	m_iNumEntries = 0;

	for ( TINT i = 0; i < MAX_NUM_ENTRIES; i++ )
	{
		m_aEntries[ i ].m_oNameText.RemoveSelf();
		m_aEntries[ i ].m_oSizeText.RemoveSelf();
		m_aEntries[ i ].m_oPercentageText.RemoveSelf();
	}

	m_oUsageText.RemoveSelf();
	CreateUsageText();
	SetVisible( TFALSE );
}

// $deBlob: FUNCTION 005b15d0
void AAudioMemoryHudElement::SetHudVisible( TBOOL a_bVisible )
{
	m_bVisible = a_bVisible;

	for ( TINT i = 0; i < m_iNumEntries; i++ )
	{
		m_aEntries[ i ].m_oNameText.SetVisible( a_bVisible );
		m_aEntries[ i ].m_oSizeText.SetVisible( a_bVisible );
		m_aEntries[ i ].m_oPercentageText.SetVisible( a_bVisible );
	}

	m_oUsageText.SetVisible( a_bVisible );
}

// $deBlob: FUNCTION 005b1640
void AAudioMemoryHudElement::UpdateUsageText()
{
	TStringManager::String16Format( m_wszUsage, 256, L"Current usage = %d Available %d Percentage used %d", m_iUsage, m_iAvailable, TMath::CeilToInt( ( TFLOAT( m_iUsage ) / TFLOAT( m_iAvailable ) ) * 100.0f ) );
}

// $deBlob: FUNCTION 005b16a0
void AAudioMemoryHudElement::CreateUsageText()
{
	m_oUsageText.Create( AFontManager::sm_pMsgFont, 400.0f );
	m_oUsageText.SetJustify( T2GUIFont::JUSTIFY_LEFT );
	m_oUsageText.SetColour( 0xFFFFFF00 );
	m_oUsageText.SetMaxChars( -1 );
	m_oUsageText.SetText( m_wszUsage );
	m_oUsageText.SetDimensions( 400.0f, 20.0f, TTRUE );

	T2GUIElement* pSafeArea = AGUIManager::GetSingleton()->GetSafeArea();
	m_oUsageText.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	pSafeArea->AddChildTail( &m_oUsageText );

	m_oUsageText.SetScale( s_fTextScale );
	UpdateUsageText();
}

// $deBlob: FUNCTION 005b1780
void AAudioMemoryHudElement::UpdatePercentages()
{
	for ( TINT i = 0; i < m_iNumEntries; i++ )
		TStringManager::String16Format( m_aEntries[ i ].m_wszPercentage, 64, L" %f Percentage of used", ( TFLOAT( m_aEntries[ i ].m_iSize ) / TFLOAT( m_iUsage ) ) * 100.0f );
}
