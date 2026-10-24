#pragma once
#include "XUI/ABlobMenu.h"

class ABlobPopupMusicMixer;

//-----------------------------------------------------------------------------
// Purpose: the music mixer of the extras: plays the
// override track (AMusicManager's music override), A and Z raise and lower the score (the intensity), the colour
// buttons play the colour sync sounds; back opens the popup (ABlobPopupMusicMixer). Leaving it reloads the front end
// music.
//-----------------------------------------------------------------------------
class ABlobMusicMixer : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobMusicMixer, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_COLOURS = 7;

public:
	ABlobMusicMixer();
	~ABlobMusicMixer();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override; // +0x114

private:
	// Plays the colour sync sound of a_iColour 20 units in front of the camera (at most every 0.3 s)
	void PlayColourSound( TINT a_iColour );

	// The track name's text (0x68) and 0x84 of the "TrackName" element
	static void SetTrackNameText( Toshi::TXUIElement* a_pElement, const TWCHAR* a_wszText, TINT a_iUnk84 );

private:
	const TWCHAR*         m_wszTrackName;             // 0x140
	TBOOL                 m_bStartMusic;              // 0x144 the music starts once the sounds are loaded
	TBOOL                 m_bMoviePlaying;            // 0x145 the background movie was started
	Toshi::TXUIElement*   m_pLoadingText;             // 0x148 "LoadingText"
	TUINT32               m_uiUnk14C;                 // 0x14C 0
	Toshi::TXUIElement*   m_pIntensity;               // 0x150 "MusicIntensity"
	ABlobPopupMusicMixer* m_pPopup;                   // 0x154 "MixerPopup" of popupmusicmixer.xur
	TUINT32               m_uiUnk158;                 // 0x158 0
	TUINT32               m_uiUnk15C;                 // 0x15C 8
	TFLOAT                m_fColourSoundTimer;        // 0x160
	TUINT32               m_auiUnk164[ 3 ];           // 0x164
	TINT                  m_aiColours[ NUM_COLOURS ]; // 0x170 the colours of the buttons
	TUINT32               m_auiUnk18C[ 7 ];           // 0x18C
	Toshi::TXUIElement*   m_apButtons[ NUM_COLOURS ]; // 0x1A8 "RedSelect".."BrownSelect"
};
