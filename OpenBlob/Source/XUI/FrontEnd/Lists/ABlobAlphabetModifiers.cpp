#include "pch.h"
#include "ABlobAlphabetModifiers.h"
#include "XUI/ABlobMenu.h"
#include "XUI/FrontEnd/ABlobGermanicNameEntry.h"

#include <T2Locale/T2Locale.h>
#include <XUI/TXUIListItem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobAlphabetModifiers ) == 0x104 );

// The labels of the case item
static const TWCHAR* s_wszLowerCase = L"abc";
static const TWCHAR* s_wszUpperCase = L"ABC";

// $deBlob: FUNCTION 0063c2c0
// $deBlob: FUNCTION 0063c2d0
// $deBlob: FUNCTION 0063c370
// $deBlob: FUNCTION 0063c410
// $deBlob: FUNCTION 0063c420
// $deBlob: FUNCTION 0053d890
// $deBlob: FUNCTION 008589a0
TDEFINE_CLASS( ABlobAlphabetModifiers );

// $deBlob: FUNCTION 0063c5f0
ABlobAlphabetModifiers::ABlobAlphabetModifiers()
{
	m_pNameEntry = TNULL;
	m_pCase      = TNULL;
	m_pDelete    = TNULL;
	m_bUpper     = TTRUE;
}

// $deBlob: FUNCTION 0063c630
// $deBlob: FUNCTION 0063c635
// $deBlob: FUNCTION 0063c640
ABlobAlphabetModifiers::~ABlobAlphabetModifiers()
{
}

// $deBlob: FUNCTION 0063c430
void ABlobAlphabetModifiers::OnCreated()
{
	TXUIList::OnCreated();

	TXUIScene* pParent = FindParentScene();
	TASSERT( pParent->IsA( &TGetClass( ABlobGermanicNameEntry ) ) );

	m_pNameEntry = TSTATICCAST( ABlobGermanicNameEntry, pParent );
	m_pCase      = &m_pItems[ 0 ];
	m_pDelete    = &m_pItems[ 1 ];
	m_pDone      = &m_pItems[ 2 ];

	const TWCHAR* wszDone   = T2Locale::GetSingleton()->GetString( 350 );
	const TWCHAR* wszCase   = s_wszLowerCase;
	const TWCHAR* wszDelete = T2Locale::GetSingleton()->GetString( 473 );

	m_pDelete->SetText( wszDelete, -1 );
	m_pDone->SetText( wszDone, -1 );
	m_pCase->SetText( wszCase, -1 );
}

// $deBlob: FUNCTION 0063c510
void ABlobAlphabetModifiers::OnItemPressed( TXUIListItem* a_pItem )
{
	if ( a_pItem == m_pCase )
	{
		m_bUpper = !m_bUpper;
		m_pNameEntry->SetUpperCase( m_bUpper );
		m_pCase->SetText( m_bUpper ? s_wszLowerCase : s_wszUpperCase, -1 );
	}
	else if ( a_pItem == m_pDelete )
	{
		m_pNameEntry->RemoveChar();
	}
	else if ( a_pItem == m_pDone )
	{
		if ( !m_pNameEntry->HasName() )
		{
			ABlobMenu::PlayUnfocusSound( a_pItem );
			TXUICommonList::OnItemPressed( a_pItem );
			return;
		}

		m_pNameEntry->ApplyName();
	}

	TXUICommonList::OnItemPressed( a_pItem );
}
