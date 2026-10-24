#pragma once
#include "ABlobNameEntryCommon.h"

class ABlobJPNAlphabetList;

//-----------------------------------------------------------------------------
// Purpose: the Japanese name entry: the "JPNAlphabet" list types the characters, the "JPNMod..." buttons
// switch the character set or modify the last character, "JPNBackSpace", "JPNCharDash" and "JPNCharSpace" edit the
// name, "DoneEnterName" confirms. At most 10 characters.
//-----------------------------------------------------------------------------
class ABlobJPNNameEntryScene : public ABlobNameEntryCommon
{
public:
	TDECLARE_CLASS( ABlobJPNNameEntryScene, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_BUTTONS = 9;

	enum BUTTON
	{
		BUTTON_Tenten,
		BUTTON_Circle,
		BUTTON_Small,
		BUTTON_Hira,
		BUTTON_Kata,
		BUTTON_English,
		BUTTON_BackSpace,
		BUTTON_Dash,
		BUTTON_Space,
	};

public:
	ABlobJPNNameEntryScene();
	~ABlobJPNNameEntryScene();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// ABlobNameEntryCommon
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 006325b0
	virtual TINT GetMaxNameLength() override { return 10; } // +0x144

private:
	Toshi::TXUIElement*   m_apButtons[ NUM_BUTTONS ]; // 0x478
	ABlobJPNAlphabetList* m_pAlphabet;                // 0x49C "JPNAlphabet"
	Toshi::TXUIElement*   m_pName;                    // 0x4A0 "Name"
	Toshi::TXUIElement*   m_pDone;                    // 0x4A4 "DoneEnterName"
};
