#pragma once
#include "XUI/ABlobMenu.h"

class ABlobTextReveal;

//-----------------------------------------------------------------------------
// Purpose: the start prompt of a challenge (challenge.xur),
// opened by AChallengeManager::ShowChallengeScreen when Blob touches a marker:
// the type icon, the stars, the time limit and the revolutionary's text (in pages separated by '*'), then the
// "Ok" or "Yes/No" buttons. Once the marker's prompt is idle again, Tick starts the challenge (or quits the current
// one first), completes a talk-only challenge or just closes.
//-----------------------------------------------------------------------------
class ABlobChallengeScene : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobChallengeScene, ABlobMenu );

	static TCONSTEXPR_VAR TINT MAXPAGES = 8;

	// m_iChoice: what Tick does once the marker's prompt is idle
	enum Choice : TINT8
	{
		Choice_None     = -1,
		Choice_Complete = 0, // a challenge without play (IsAlreadyCompleted, the null challenges): completed at once
		Choice_Start    = 1,
		Choice_Replace  = 2, // quits the current challenge and starts this one (the confirmation's yes)
		Choice_Cancel   = 3,
	};

public:
	ABlobChallengeScene();
	~ABlobChallengeScene();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                        // +0x04
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 00643fa0
	virtual void EndTransFrom() override { TXUIScene::EndTransFrom(); } // +0x128
	virtual void EndTransBackTo() override;                             // +0x12C

	//-----------------------------------------------------------------------------
	// ABlobMenu
	//-----------------------------------------------------------------------------
	virtual void OnConfirmationClosed( TBOOL a_bYes ) override; // +0x138

	// Mirrors the layout (the revolutionary on the other side), for the challenges with Data::Flags_MirrorLayout
	void MirrorLayout();

private:
	// Gives the game back to the player (the prompt is closed)
	static void ResumeGame();

	// Shows the "Ok" button, or the "Yes"/"No" buttons
	void ShowOkButton();
	void ShowYesNoButtons();

private:
	Toshi::TXUIControl* m_pYesButton;                // 0x140 UID 5
	Toshi::TXUIControl* m_pNoButton;                 // 0x144 UID 4
	Toshi::TXUIControl* m_pOkButton;                 // 0x148 UID 6
	ABlobTextReveal*    m_pText;                     // 0x14C UID 10
	TWCHAR*             m_apPages[ MAXPAGES ];       // 0x150
	TINT16              m_aiPageOffsets[ MAXPAGES ]; // 0x170 the character index of each page in the text
	TINT8               m_iNumPages;                 // 0x180
	TINT8               m_iCurrentPage;              // 0x181
	Choice              m_iChoice;                   // 0x182
	TBOOL               m_bConfirmed;                // 0x183 the yes of the confirmation popup
	TBOOL               m_bHasPages;                 // 0x184
	TBOOL               m_bCompleteOnAccept;         // 0x185 AChallenge::IsAlreadyCompleted
	TBOOL               m_bTextDone;                 // 0x186 the text was shown to its end
	TBOOL               m_bWaitingForCamera;         // 0x187 the challenge's camera plays before it starts
	Toshi::TXUIElement* m_pElement9;                 // 0x188 UID 9
	Toshi::TXUIElement* m_pDetails;                  // 0x18C UID 20, the icon, stars and time (hidden at first)
	Toshi::TXUIElement* m_pButtons;                  // 0x190 UID 3, the buttons (hidden at first)
	TWCHAR              m_wszTimeLimit[ 0x80 ];      // 0x194
	Toshi::TXUIElement* m_pElement11;                // 0x294 UID 11
};

TSTATICASSERT( sizeof( ABlobChallengeScene ) == 0x298 );
