#pragma once
#include <XUI/TXUIList.h>

class ABlobNameEntryCommon;

//-----------------------------------------------------------------------------
// Purpose: the letters of ABlobGermanicNameEntry: A-Z, plus ÆØÅ (Danish/Norwegian) or ÅÄÖ (Swedish) depending
// on the language, upper or lower case;
// pressing one adds it to the name.
//-----------------------------------------------------------------------------
class ABlobGermanicAlphabetList : public Toshi::TXUIList
{
public:
	TDECLARE_CLASS( ABlobGermanicAlphabetList, Toshi::TXUIList );

	static TCONSTEXPR_VAR TINT MAX_CHARS = 40;

	// SetAlphabet's alphabets
	enum ALPHABET : TINT
	{
		ALPHABET_Upper        = 0, // A-Z
		ALPHABET_Lower        = 1,
		ALPHABET_UpperDanish  = 2, // A-Z ÆØÅ
		ALPHABET_LowerDanish  = 3,
		ALPHABET_UpperSwedish = 4, // A-Z ÅÄÖ
		ALPHABET_LowerSwedish = 5,
	};

public:
	ABlobGermanicAlphabetList();
	~ABlobGermanicAlphabetList();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	//-----------------------------------------------------------------------------
	// TXUIList
	//-----------------------------------------------------------------------------
	virtual const TWCHAR* GetListItemText( TINT a_iItemIndex ) override;          // +0x124
	virtual void          OnItemPressed( Toshi::TXUIListItem* a_pItem ) override; // +0x158

	//-----------------------------------------------------------------------------
	// ABlobGermanicAlphabetList
	//-----------------------------------------------------------------------------
	virtual void SetAlphabet( TINT a_eAlphabet ); // +0x15C

	// The alphabet of the language, upper case for a_iCase 1
	void SetCase( TINT a_iCase );

private:
	ABlobNameEntryCommon* m_pNameEntry;                  // 0xD8
	TWCHAR                m_awszChars[ MAX_CHARS ][ 2 ]; // 0xDC
	TINT                  m_eAlphabet;                   // 0x17C
};
