#pragma once
#include <XUI/TXUIScene.h>

class ABlobFrontEnd;

//-----------------------------------------------------------------------------
// Purpose: the base of de Blob's menu scenes: the button sounds, the
// background movie of ABlobFrontEnd and the music beat scene while the menu is shown, the localised text of the
// back buttons and the attract mode (the intro movie after a while without input, see UpdateAttractMode).
//-----------------------------------------------------------------------------
class ABlobMenu : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobMenu, Toshi::TXUIScene );

	// The time without input before the attract movie and the fade before it
	static const TFLOAT sm_fAttractModeTime;
	static const TFLOAT sm_fAttractModeFadeTime;

public:
	ABlobMenu();
	ABlobMenu( TINT8 a_iBackgroundMovie );
	~ABlobMenu();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0
	virtual TBOOL OnNotifySetFocus( TXUIElement* a_pSource ) override;                  // +0xB4
	virtual TBOOL OnNotifyKillFocus( TXUIElement* a_pSource ) override;                 // +0xB8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransFrom() override;      // +0x118
	virtual void TransBackFrom() override;  // +0x120
	virtual void EndTransTo() override;     // +0x124
	virtual void EndTransBackTo() override; // +0x12C

	//-----------------------------------------------------------------------------
	// ABlobMenu
	//-----------------------------------------------------------------------------
	virtual void OnAttractModeUpdate(); // +0x134
	// $deBlob: FUNCTION 00626030
	virtual void OnConfirmationClosed( TBOOL a_bYes ) {} // +0x138, the answer of a confirmation popup
	virtual void StartAttractMode();                     // +0x13C
	virtual void EndAttractMode();                       // +0x140

	// Plays the attract movie after sm_fAttractModeTime without input (called by the Tick of the frontend menus)
	void UpdateAttractMode( TFLOAT a_fDeltaTime );

	// The sounds of the buttons (TXUIButton, TXUIListItem, TXUIRadioButton)
	static void PlayPressSound( TXUIElement* a_pSource );
	static void PlayFocusSound( TXUIElement* a_pSource );
	static void PlayUnfocusSound( TXUIElement* a_pSource );

	// The scene a_wszName (case-insensitive) below the "FrontEnd" scene this menu is in
	Toshi::TXUIScene* FindFrontEndScene( const TWCHAR* a_wszName );

	// The scene a_wszName (case-insensitive) below a_pParent (depth first)
	static Toshi::TXUIScene* FindChildScene( Toshi::T2GUIElement* a_pParent, const TWCHAR* a_wszName );

	// Puts the localised "Back" in the XuiText1 of the XuiBackButton visuals of the scenes below a_pElement
	static void SetBackButtonText( TXUIElement* a_pElement );

	// The "New" labels of the current profile's new flags: shown while the flag is new and seen, revealed (played
	// "Reveal") when an unseen flag gets seen
	void UpdateNewFlagLabel( Toshi::TXUIControl* a_pControl, TUINT32 a_uiNewFlag );
	void RevealNewFlagLabel( Toshi::TXUIControl* a_pControl, TUINT32 a_uiNewFlag );
	void UpdateNewFlagLabel_extended( Toshi::TXUIControl* a_pControl, TUINT32 a_uiNewFlag );
	void RevealNewFlagLabel_extended( Toshi::TXUIControl* a_pControl, TUINT32 a_uiNewFlag );

protected:
	TFLOAT         m_fIdleTime;        // 0x130 time without input (attract mode)
	TFLOAT         m_fAttractFadeTime; // 0x134
	TBOOL          m_bAttractMovie;    // 0x138 the attract movie is playing
	TBOOL          m_bAttractFading;   // 0x139 fading out before the attract movie
	TINT8          m_iBackgroundMovie; // 0x13A ABlobFrontEnd's background movie of the menu, -1: none
	ABlobFrontEnd* m_pFrontEnd;        // 0x13C the parent scene (set by EndTransTo/EndTransBackTo)
};
