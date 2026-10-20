#pragma once
#include "ABlobNameEntryCommon.h"

class ABlobGermanicAlphabetList;

TOSHI_NAMESPACE_START
class TXUIListItem;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: the name entry of the latin alphabet languages: the "Alphabet" list types the letters of the
// language, the modifiers list (ABlobAlphabetModifiers) switches the case, deletes and confirms; the last item of
// list 3 confirms too. At most 11 characters.
//-----------------------------------------------------------------------------
class ABlobGermanicNameEntry : public ABlobNameEntryCommon
{
public:
	TDECLARE_CLASS( ABlobGermanicNameEntry, ABlobMenu );

public:
	ABlobGermanicNameEntry();
	~ABlobGermanicNameEntry();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// ABlobNameEntryCommon
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 00632160
	virtual TINT GetMaxNameLength() override { return 11; } // +0x144

	// Switches the alphabet's case (ABlobAlphabetModifiers)
	void SetUpperCase( TBOOL a_bUpper );

private:
	Toshi::TXUIElement*        m_pName;     // 0x478 element 7, shows the name
	ABlobGermanicAlphabetList* m_pAlphabet; // 0x47C "Alphabet"
	Toshi::TXUIControl*        m_pList3;    // 0x480 control 3
	Toshi::TXUIListItem*       m_pDone;     // 0x484 the last item of m_pList3
};
