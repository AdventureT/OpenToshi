#pragma once
#include "ABlobHud.h"

class ABlobMultiPlayerHud;
class ABlobScoreHud;

//-----------------------------------------------------------------------------
// Purpose: the HUD of a three player match (hud_3p.xur "HUD_3P",
// created by APlayerManager::CreatePlayers): a panel per player (ABlobMultiPlayerHud with the score, size, placing
// and score float HUDs), the level timer, the "ReadySetGo" of the match start (the input is disabled until it
// ends) and the winner's animation.
//-----------------------------------------------------------------------------
class ABlobHud_3P : public ABlobHud
{
public:
	TDECLARE_CLASS( ABlobHud_3P, ABlobHud );

	static TCONSTEXPR_VAR TINT NUM_PLAYERS = 3;

public:
	ABlobHud_3P();
	~ABlobHud_3P();

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
	virtual ABlobMultiPlayerHud* GetPlayer3Hud() override;                              // +0x14C

private:
	ABlobMultiPlayerHud* m_apPanels[ NUM_PLAYERS ];    // 0x2F4 elements 0x26, 0x1C, 0x12
	Toshi::TXUIElement*  m_pReadySetGo;                // 0x300 element 0x43 (an XuiGroup)
	Toshi::TXUIControl*  m_pWinner;                    // 0x304 control 0x34
	ABlobScoreHud*       m_apScoreHuds[ NUM_PLAYERS ]; // 0x308
};
