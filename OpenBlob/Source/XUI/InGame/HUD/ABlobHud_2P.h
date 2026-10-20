#pragma once
#include "ABlobHud.h"

class ABlobMultiPlayerHud;
class ABlobScoreHud;

//-----------------------------------------------------------------------------
// Purpose: the HUD of a two player match (hud_2p.xur "HUD_2P",
// created by APlayerManager::CreatePlayers): a panel per player (ABlobMultiPlayerHud with the score, size, placing
// and score float HUDs), the level timer, the "ReadySetGo" of the match start (the input is disabled until it
// ends) and the winner's animation.
//-----------------------------------------------------------------------------
class ABlobHud_2P : public ABlobHud
{
public:
	TDECLARE_CLASS( ABlobHud_2P, ABlobHud );

	static TCONSTEXPR_VAR TINT NUM_PLAYERS = 2;

public:
	ABlobHud_2P();
	~ABlobHud_2P();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void OnCreated() override;                 // +0x94

	//-----------------------------------------------------------------------------
	// ABlobHud
	//-----------------------------------------------------------------------------
	virtual void                 StartReadySetGo() override;                            // +0x134
	virtual void                 ShowWinner( TINT a_iPlayer ) override;                 // +0x138
	virtual Toshi::TXUIElement*  GetMessageElement( void** a_ppMessageOwner ) override; // +0x13C
	virtual void                 Reset() override;                                      // +0x140
	virtual ABlobMultiPlayerHud* GetPlayer1Hud() override;                              // +0x144
	virtual ABlobMultiPlayerHud* GetPlayer2Hud() override;                              // +0x148

private:
	ABlobMultiPlayerHud* m_apPanels[ NUM_PLAYERS ];    // 0x2F4 elements 0xB, 1
	Toshi::TXUIElement*  m_pReadySetGo;                // 0x2FC element 0x27 (an XuiGroup)
	Toshi::TXUIControl*  m_pWinner;                    // 0x300 control 0x18
	ABlobScoreHud*       m_apScoreHuds[ NUM_PLAYERS ]; // 0x304
};
