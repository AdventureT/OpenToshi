#pragma once
#include <XUI/TXUIList.h>

class ABlobNameEntryCommon;

//-----------------------------------------------------------------------------
// Purpose: the 50 characters of the Japanese name entry (ABlobJPNNameEntryScene): hiragana, katakana
// (plain, with dakuten, handakuten or small) or latin letters, digits and symbols; pressing one adds it to the name.
//-----------------------------------------------------------------------------
class ABlobJPNAlphabetList : public Toshi::TXUIList
{
public:
	TDECLARE_CLASS( ABlobJPNAlphabetList, Toshi::TXUIList );

	static TCONSTEXPR_VAR TINT NUM_CHARS = 50;

	// The character sets (SetMode)
	enum MODE : TINT
	{
		MODE_HiraganaSmall      = 0,
		MODE_Hiragana           = 1,
		MODE_HiraganaDakuten    = 2,
		MODE_HiraganaHandakuten = 3,
		MODE_KatakanaSmall      = 4,
		MODE_Katakana           = 5,
		MODE_KatakanaDakuten    = 6,
		MODE_KatakanaHandakuten = 7,
		MODE_UpperCase          = 8,
		MODE_LowerCase          = 9,
	};

	// The buttons of the name entry (ChangeMode, ModifyLastChar)
	enum MODIFIER : TINT
	{
		MODIFIER_Dakuten    = 0,
		MODIFIER_Handakuten = 1,
		MODIFIER_Small      = 2,
		MODIFIER_Hiragana   = 3,
		MODIFIER_Katakana   = 4,
		MODIFIER_English    = 5,
	};

public:
	ABlobJPNAlphabetList();
	~ABlobJPNAlphabetList();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIList
	//-----------------------------------------------------------------------------
	virtual const TWCHAR* GetListItemText( TINT a_iItemIndex ) override; // +0x124

	// Switches the character set with a modifier button
	void ChangeMode( TINT a_eModifier );

	// Shows the characters of a_eMode
	void SetMode( TINT a_eMode );

	// Toggles a_eModifier (dakuten, handakuten or small) on the last character of the name (hiragana or katakana)
	void ModifyLastChar( TINT a_eModifier );

	// $deBlob: FUNCTION 006325a0
	TINT GetMode() const { return m_eMode; }

private:
	ABlobNameEntryCommon* m_pNameEntry;                  // 0x0D8
	TWCHAR                m_awszChars[ NUM_CHARS ][ 2 ]; // 0x0DC
	TINT                  m_eMode;                       // 0x1A4
};
