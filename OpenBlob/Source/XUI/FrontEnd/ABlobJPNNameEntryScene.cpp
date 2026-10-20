#include "pch.h"
#include "ABlobJPNNameEntryScene.h"
#include "Lists/ABlobJPNAlphabetList.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobJPNNameEntryScene ) == 0x4A8 );

// The buttons and their labels
static const TWCHAR* s_awszButtonNames[ ABlobJPNNameEntryScene::NUM_BUTTONS ] = {
	L"JPNModTenten", L"JPNModCircle", L"JPNModSmall", L"JPNModHira", L"JPNModKata", L"JPNModEnglish", L"JPNBackSpace", L"JPNCharDash", L"JPNCharSpace"
};
static const TWCHAR* s_awszButtonTexts[ ABlobJPNNameEntryScene::NUM_BUTTONS ] = {
	L"\x309B", L"\x309C", L"\x5C0F\x5B57", L"\x304B\x306A", L"\x30AB\x30CA", L"ABC", L"\x2190", L"\x30FC", L"\x7A7A\x767D"
};

// $deBlob: FUNCTION 00632170
// $deBlob: FUNCTION 00632180
// $deBlob: FUNCTION 00632200
// $deBlob: FUNCTION 00632280
// $deBlob: FUNCTION 00632290
// $deBlob: FUNCTION 0053d550
// $deBlob: FUNCTION 008588d0
TDEFINE_CLASS( ABlobJPNNameEntryScene );

// $deBlob: FUNCTION 006322a0
ABlobJPNNameEntryScene::ABlobJPNNameEntryScene()
{
}

// $deBlob: FUNCTION 00632550
// $deBlob: FUNCTION 00632562
// $deBlob: FUNCTION 00632570
ABlobJPNNameEntryScene::~ABlobJPNNameEntryScene()
{
}

// $deBlob: FUNCTION 006322c0
void ABlobJPNNameEntryScene::OnCreated()
{
	ABlobMenu::OnCreated();

	m_pAlphabet = TSTATICCAST( ABlobJPNAlphabetList, FindChildXUIElement( L"JPNAlphabet" ) );

	for ( TINT i = 0; i < NUM_BUTTONS; i++ )
	{
		m_apButtons[ i ] = FindChildXUIElement( s_awszButtonNames[ i ] );

		if ( m_apButtons[ i ] != TNULL )
			TSTATICCAST( TXUIControl, m_apButtons[ i ] )->SetText( s_awszButtonTexts[ i ], -1 );
	}

	m_pDone = FindChildXUIElement( L"DoneEnterName" );
	m_pName = FindChildXUIElement( L"Name" );

	ClearName();
	TSTATICCAST( TXUIControl, m_pName )->SetText( m_wszName, -1 );
}

// $deBlob: FUNCTION 00632370
TBOOL ABlobJPNNameEntryScene::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( a_pSource == TNULL )
		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	if ( a_pSource == m_apButtons[ BUTTON_Tenten ] )
	{
		m_pAlphabet->ModifyLastChar( ABlobJPNAlphabetList::MODIFIER_Dakuten );
	}
	else if ( a_pSource == m_apButtons[ BUTTON_Circle ] )
	{
		m_pAlphabet->ModifyLastChar( ABlobJPNAlphabetList::MODIFIER_Handakuten );
	}
	else if ( a_pSource == m_apButtons[ BUTTON_Small ] )
	{
		TINT eMode = m_pAlphabet->GetMode();

		if ( eMode == ABlobJPNAlphabetList::MODE_UpperCase )
			m_pAlphabet->ChangeMode( ABlobJPNAlphabetList::MODIFIER_Small );
		else if ( eMode == ABlobJPNAlphabetList::MODE_LowerCase )
			m_pAlphabet->ChangeMode( ABlobJPNAlphabetList::MODIFIER_English );
		else
			m_pAlphabet->ModifyLastChar( ABlobJPNAlphabetList::MODIFIER_Small );
	}
	else if ( a_pSource == m_apButtons[ BUTTON_English ] )
	{
		m_pAlphabet->ChangeMode( ABlobJPNAlphabetList::MODIFIER_English );
	}
	else if ( a_pSource == m_apButtons[ BUTTON_Hira ] )
	{
		m_pAlphabet->ChangeMode( ABlobJPNAlphabetList::MODIFIER_Hiragana );
	}
	else if ( a_pSource == m_apButtons[ BUTTON_Kata ] )
	{
		m_pAlphabet->ChangeMode( ABlobJPNAlphabetList::MODIFIER_Katakana );
	}
	else if ( a_pSource == m_apButtons[ BUTTON_BackSpace ] )
	{
		RemoveChar();
	}
	else if ( a_pSource == m_apButtons[ BUTTON_Dash ] )
	{
		AddChar( 0x30FC );
	}
	else if ( a_pSource == m_apButtons[ BUTTON_Space ] )
	{
		AddChar( L' ' );
	}
	else if ( a_pSource == m_pDone )
	{
		if ( !HasName() )
		{
			PlayUnfocusSound( a_pSource );
			return TTRUE;
		}

		ApplyName();
		CreateProfile();
	}

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}
