#pragma once
#include "XUI/ABlobMenu.h"

namespace Toshi
{
class TInputDevice;
}

//-----------------------------------------------------------------------------
// Purpose: the Blob Party engagement menu:
// four player slots; a keyboard or a controller joins a free slot by pressing accept (the player
// devices are locked meanwhile, TInputDevice::ms_bPlayerDevicesLocked), chooses one of seven colours (a colour
// another slot took is dimmed and can't be taken) and is ready; once the first player and every other joined
// player are ready (two at least), the start button bounces in and opens the multiplayer level selection with the
// players' colours and count in APlayerManager. TXUIInput sends the buttons of every device to the scene's
// "MPEngagement" control with the device index as the controller.
//-----------------------------------------------------------------------------
class ABlobMultiplayerEngagementMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobMultiplayerEngagementMenu, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_SLOTS   = 4;
	static TCONSTEXPR_VAR TINT NUM_COLOURS = 7;

	// The state of a slot (m_aiState, the printf of HandleButton)
	enum SlotState
	{
		SlotState_Engagement, // free, or joined and waiting for the next accept
		SlotState_Colour,     // choosing a colour
		SlotState_Ready,      // colour chosen
	};

public:
	ABlobMultiplayerEngagementMenu();
	~ABlobMultiplayerEngagementMenu();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                 // +0x04
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0
	virtual TBOOL OnNotifySetFocus( TXUIElement* a_pSource ) override;                  // +0xB4

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;        // +0x114
	virtual void TransFrom() override;      // +0x118
	virtual void TransBackTo() override;    // +0x11C
	virtual void TransBackFrom() override;  // +0x120
	virtual void EndTransTo() override;     // +0x124
	virtual void EndTransBackTo() override; // +0x12C

private:
	// Locks the player devices and frees every slot (OnCreated, EndTransBackTo)
	void Reset();

	// No slot has a device
	// $deBlob: FUNCTION 00641770
	TBOOL AreAllSlotsFree() const { return m_aiDevice[ 0 ] < 0 && m_aiDevice[ 1 ] < 0 && m_aiDevice[ 2 ] < 0 && m_aiDevice[ 3 ] < 0; }

	// a_pDevice takes the first free slot (or updates the prompt's platform if it has slot 0)
	void AddDevice( Toshi::TInputDevice* a_pDevice );

	// Frees the slot of a_pDevice and moves the slots after it down
	void RemoveDevice( Toshi::TInputDevice* a_pDevice );

	// Frees the slot a_iSlot
	void ResetSlot( TINT a_iSlot );

	// Moves the slot a_iFrom to a_iTo (RemoveDevice)
	void MoveSlot( TINT a_iFrom, TINT a_iTo );

	// Dims the colours other slots took
	void UpdateTakenColours();

	// Focuses the colour a_iColour of the slot a_iSlot
	void FocusColour( TINT a_iColour, TINT a_iSlot );

private:
	TINT                m_aiState[ NUM_SLOTS ];                          // 0x140 SlotState
	Toshi::TXUIControl* m_pStart;                                        // 0x150 control 79, shown when the players are ready
	TWCHAR              m_wszStartText[ 256 ];                           // 0x154
	Toshi::TXUIControl* m_apColours[ NUM_SLOTS ][ NUM_COLOURS ];         // 0x354 the colour buttons
	Toshi::TXUIControl* m_apSelectedColours[ NUM_SLOTS ][ NUM_COLOURS ]; // 0x3C4 the chosen colour, shown instead
	TINT                m_aiDevice[ NUM_SLOTS ];                         // 0x434 device index, -1: free
	TINT                m_aiColour[ NUM_SLOTS ];                         // 0x444 the chosen colour, -1: none
	TINT                m_aiFocus[ NUM_SLOTS ];                          // 0x454 the focused colour
	TINT                m_iEntryDevice;                                  // 0x464 player 0's device when the menu was made
	TWCHAR              m_awszSlotText[ NUM_SLOTS ][ 256 ];              // 0x468
	TFLOAT              m_fBlinkAlpha;                                   // 0xC68 the "join" texts, 0.5 to 1
	TBOOL               m_bBlinkIn;                                      // 0xC6C
	TFLOAT              m_fStartY;                                       // 0xC70 the start button's rest position, -9999: not set yet
	TFLOAT              m_fStartBounceDelay;                             // 0xC74
	TFLOAT              m_fStartBounceSpeed;                             // 0xC78
	TBOOL               m_bStartBouncing;                                // 0xC7C
	Toshi::TXUIElement* m_apTakenWarnings[ NUM_SLOTS ];                  // 0xC80 shown on the colour another slot has
	TFLOAT              m_afTakenWarningTime[ NUM_SLOTS ];               // 0xC90 before the warning fades
};
