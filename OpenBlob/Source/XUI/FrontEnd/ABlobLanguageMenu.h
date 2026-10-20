#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the language menu (language.xur).
// NOTE: on PC the menu chooses Steam's language by itself in its first Tick and leaves (AXUIState::SetNextState(1)
// pushes ASaveLoadStartAppState), so it shows for one frame; its buttons never get their language names (the
// buttons of sm_aLanguages are never set: only the press handler reads them).
//-----------------------------------------------------------------------------
class ABlobLanguageMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobLanguageMenu, ABlobMenu );

	// A language button (0x10 bytes)
	struct Language
	{
		TINT                m_iLanguage; // 0x0
		TINT                m_iUnk04;    // 0x4
		TINT                m_iStringId; // 0x8 its localised name
		Toshi::TXUIElement* m_pButton;   // 0xC
	};

	static TCONSTEXPR_VAR TINT NUM_LANGUAGES = 3;

	static Language sm_aLanguages[ NUM_LANGUAGES ];

public:
	ABlobLanguageMenu();
	~ABlobLanguageMenu();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	// $deBlob: FUNCTION 00632b40
	virtual void Render() override {} // +0x14

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

private:
	TINT m_iSelectedLanguage; // 0x140 index in sm_aLanguages, -1: none
};
