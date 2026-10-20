#include "pch.h"
#include "ABlobGermanicNameEntry.h"
#include "Lists/ABlobGermanicAlphabetList.h"

#include <XUI/TXUIList.h>
#include <XUI/TXUIListItem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobGermanicNameEntry ) == 0x488 );

// $deBlob: FUNCTION 00631e50
// $deBlob: FUNCTION 00631e60
// $deBlob: FUNCTION 00631f00
// $deBlob: FUNCTION 00631f90
// $deBlob: FUNCTION 00631fa0
// $deBlob: FUNCTION 0053d510
// $deBlob: FUNCTION 008588c0
TDEFINE_CLASS( ABlobGermanicNameEntry );

// $deBlob: FUNCTION 00631fb0
ABlobGermanicNameEntry::ABlobGermanicNameEntry()
{
	m_pName     = TNULL;
	m_pAlphabet = TNULL;
	m_pDone     = TNULL;
}

// $deBlob: FUNCTION 006320e0
// $deBlob: FUNCTION 00632100
// $deBlob: FUNCTION 00632112
// $deBlob: FUNCTION 00632120
ABlobGermanicNameEntry::~ABlobGermanicNameEntry()
{
}

// $deBlob: FUNCTION 00631ff0
void ABlobGermanicNameEntry::OnCreated()
{
	ABlobMenu::OnCreated();

	m_pName = FindChildXUIElement( 7 );
	TSTATICCAST( TXUIControl, m_pName )->SetText( m_wszName, -1 );

	m_pAlphabet = TSTATICCAST( ABlobGermanicAlphabetList, FindChildXUIControl( L"Alphabet" ) );
	m_pList3    = FindChildXUIControl( 3 );

	m_pDone = TSTATICCAST( TXUIList, m_pList3 )->GetListItem( TSTATICCAST( TXUIList, m_pList3 )->GetNumItems() - 1 );
}

// $deBlob: FUNCTION 00632060
void ABlobGermanicNameEntry::SetUpperCase( TBOOL a_bUpper )
{
	if ( a_bUpper )
		m_pAlphabet->SetCase( 1 );
	else
		m_pAlphabet->SetCase( 0 );
}

// $deBlob: FUNCTION 00632090
TBOOL ABlobGermanicNameEntry::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( a_pSource == m_pDone )
	{
		if ( !HasName() )
		{
			PlayUnfocusSound( a_pSource );
			return TTRUE;
		}

		CreateProfile();
	}

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}
