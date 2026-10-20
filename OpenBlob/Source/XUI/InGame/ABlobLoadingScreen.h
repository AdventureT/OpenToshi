#pragma once
#include <XUI/TXUIScene.h>

class AGameState;
class ABlobCursorTrail;

//-----------------------------------------------------------------------------
// Purpose: the loading screen of a level (loadingscreen*.xur):
// loadingscreen*.xur): the level's name, the tips (layers) and the "Continue" prompt once the level is loaded.
//-----------------------------------------------------------------------------
class ABlobLoadingScreen : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobLoadingScreen, Toshi::TXUIScene );

public:
	ABlobLoadingScreen();
	~ABlobLoadingScreen();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                        // +0x04
	virtual TBOOL SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam ) override;           // +0x78 "EndIntro" of a challenge's tip enables its button
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0 a layer's button: on to the next tip
	virtual TBOOL OnNotifySetFocus( Toshi::TXUIElement* a_pSource ) override;                  // +0xB4 a focused button: the loading sound, moved to the end of the children
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override;                 // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override; // +0x114

	// The text of "LevelName"
	void SetLevelName( const TWCHAR* a_wszName );

private:
	// Shows a_iStringId in the speech bubble of a_iLayer
	void SetLayerTip( TINT a_iLayer, TINT a_iStringId );

	static void HideButtonPrompts( Toshi::TXUIScene* a_pScene );

private:
	Toshi::TXUIControl** m_apLayerButtons;      // 0x130 m_iNumLayers ("Button", the sprint/free paint ones "Button_Arty"...)
	Toshi::TXUIControl** m_apSpeechBubbles;     // 0x134 m_iNumLayers ("SpeechBubble")
	Toshi::TXUIControl** m_apLayers;            // 0x138 m_iNumLayers ("Layer%d")
	Toshi::TXUIElement*  m_pContinue;           // 0x13C "Continue"
	Toshi::TXUIElement*  m_pPointRemote;        // 0x140 "PointRemote"
	Toshi::TXUIElement*  m_pLoadingIndicator;   // 0x144 "LoadingIndicator"
	Toshi::TXUIControl*  m_pLevelName;          // 0x148 "LevelName"
	TBOOL                m_bContinueShown;      // 0x14C
	AGameState*          m_pLoadLevelState;     // 0x150 the current state if it's ALoadLevelState
	ABlobCursorTrail*    m_apCursorTrails[ 4 ]; // 0x154 "CursorTrail", "CursorTrail%d"
	TUINT32              m_uiUnk164;            // 0x164
	TINT                 m_iNumLayers;          // 0x168
	TINT                 m_iCurLayer;           // 0x16C the focused layer
	TINT                 m_iLayerOrder;         // 0x170 index in the layer order tables
	Toshi::TXUIElement*  m_apPulses[ 4 ];       // 0x174 "pulse_arty"... (sprint/free paint)
	TUINT16              m_uiUnk184;            // 0x184
	TFLOAT               m_fTipAnimTime;        // 0x188 0.75
	TBOOL                m_bUnk18C;             // 0x18C
};
