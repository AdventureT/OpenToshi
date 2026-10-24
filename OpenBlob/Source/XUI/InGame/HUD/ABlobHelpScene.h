#pragma once
#include <XUI/TXUIScene.h>

class APropTriggerHelp;
class ABlobTextReveal;

//-----------------------------------------------------------------------------
// Purpose: the help message that pauses the game (helphud.xur):
// APropTriggerHelp creates it (HelpType 0 and 4), it transitions in once the HUD is the active scene,
// shows its text page by page ('*' separates the pages) with the face of its speaker, and unpauses the game when
// it's closed.
//-----------------------------------------------------------------------------
class ABlobHelpScene : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobHelpScene, Toshi::TXUIScene );

	static TCONSTEXPR_VAR TINT MAXPAGES  = 8;
	static TCONSTEXPR_VAR TINT iMAXFACES = 4;

public:
	ABlobHelpScene();
	~ABlobHelpScene();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void OnCreated() override;                 // +0x94

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransBackFrom() override; // +0x120
	virtual void EndTransTo() override;    // +0x124

	// Sets the text: the string a_iStringId or a_wszText
	void SetHelpText( TINT a_iStringId, const TWCHAR* a_wszText );

	// Shows the count of the found collectables (the help of HelpType 4) instead of a face
	void ShowCollectableCount();

	// Shows the face of a_iSpeaker
	void SetSpeaker( TINT a_iSpeaker );

	// $deBlob: FUNCTION 00762790
	void SetTrigger( APropTriggerHelp* a_pTrigger ) { m_pTrigger = a_pTrigger; }

private:
	// Unpauses the game once the scene is closed
	void Close();

public:
	// Splits a_wszText at each a_wszSeparator into newly allocated strings (also used by ABlobChallengeScene)
	static void SplitString( const TWCHAR* a_wszText, const TWCHAR* a_wszSeparator, TWCHAR** a_ppOutStrings );

private:
	APropTriggerHelp*   m_pTrigger;                  // 0x130 the trigger that showed the message
	Toshi::TXUIElement* m_pSpeech;                   // 0x134 element 1 ("ShowSpeech"/"HideSpeech")
	Toshi::TXUIControl* m_pContinue;                 // 0x138 control 0xC (strings 0xB4/0xB5)
	ABlobTextReveal*    m_pText;                     // 0x13C control 5
	TBOOL               m_bPaged;                    // 0x140 the text has several pages
	TWCHAR*             m_apPages[ MAXPAGES ];       // 0x144
	TINT16              m_aiPageOffsets[ MAXPAGES ]; // 0x164 the start of each page in the string (the format's too)
	TINT                m_iNumPages;                 // 0x174
	TINT                m_iCurrentPage;              // 0x178
	TINT                m_iStringId;                 // 0x17C -1: a text
	TWCHAR              m_wszCount[ 32 ];            // 0x180 "%d/%d"
};
