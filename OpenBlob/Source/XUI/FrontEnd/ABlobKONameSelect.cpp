#include "pch.h"
#include "ABlobKONameSelect.h"

#include <T2Locale/T2Locale.h>
#include <Toshi/T2String16.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobKONameSelect ) == 0x488 );

// $deBlob: FUNCTION 006325c0
// $deBlob: FUNCTION 006325d0
// $deBlob: FUNCTION 00632660
// $deBlob: FUNCTION 006326e0
// $deBlob: FUNCTION 006326f0
// $deBlob: FUNCTION 0053d590
// $deBlob: FUNCTION 008588e0
TDEFINE_CLASS( ABlobKONameSelect );

// $deBlob: FUNCTION 00632700
ABlobKONameSelect::ABlobKONameSelect()
{
	m_iProfileNumber = -1;
}

// $deBlob: FUNCTION 00632890
// $deBlob: FUNCTION 006328a2
// $deBlob: FUNCTION 006328b0
ABlobKONameSelect::~ABlobKONameSelect()
{
}

// $deBlob: FUNCTION 00632730
void ABlobKONameSelect::OnCreated()
{
	ABlobMenu::OnCreated();

	m_pOK          = FindChildXUIElement( L"btnOK" );
	m_pSaveDisplay = FindChildXUIElement( L"lblSaveDisplay" );
	m_pBack        = FindChildXUIElement( L"XuiBackButton1" );
}

// $deBlob: FUNCTION 00632770
TBOOL ABlobKONameSelect::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( a_pSource == m_pOK )
	{
		ApplyName();
		TSTATICCAST( TXUIControl, m_pBack )->SetEnabled( TFALSE );
		CreateProfile();
	}

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}

// $deBlob: FUNCTION 006327d0
void ABlobKONameSelect::SetProfileNumber( TINT a_iProfileNumber )
{
	m_iProfileNumber = a_iProfileNumber;
}

// $deBlob: FUNCTION 006327e0
void ABlobKONameSelect::EndTransTo()
{
	ABlobMenu::EndTransTo();

	TASSERT( m_iProfileNumber != -1 );

	TWCHAR wszName[ 12 ];
	T2String16::Format( wszName, T2Locale::GetSingleton()->GetString( 472 ), m_iProfileNumber );
	SetName( wszName );

	TSTATICCAST( TXUIControl, m_pSaveDisplay )->SetText( m_wszName, -1 );
}
