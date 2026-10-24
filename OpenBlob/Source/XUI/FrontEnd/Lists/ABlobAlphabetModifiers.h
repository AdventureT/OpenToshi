#pragma once
#include <XUI/TXUICommonList.h>

class ABlobGermanicNameEntry;

//-----------------------------------------------------------------------------
// Purpose: the modifiers of ABlobGermanicNameEntry: item 0 switches the case ("abc"/"ABC"), item 1 deletes a
// character, item 2 confirms the name.
//-----------------------------------------------------------------------------
class ABlobAlphabetModifiers : public Toshi::TXUICommonList
{
public:
	TDECLARE_CLASS( ABlobAlphabetModifiers, Toshi::TXUICommonList );

public:
	ABlobAlphabetModifiers();
	~ABlobAlphabetModifiers();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	//-----------------------------------------------------------------------------
	// TXUIList
	//-----------------------------------------------------------------------------
	virtual void OnItemPressed( Toshi::TXUIListItem* a_pItem ) override; // +0x158

private:
	ABlobGermanicNameEntry* m_pNameEntry; // 0xF0
	Toshi::TXUIListItem*    m_pCase;      // 0xF4 item 0
	Toshi::TXUIListItem*    m_pDelete;    // 0xF8 item 1
	Toshi::TXUIListItem*    m_pDone;      // 0xFC item 2
	TBOOL                   m_bUpper;     // 0x100
};
