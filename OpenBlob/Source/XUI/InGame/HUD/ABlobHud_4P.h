#pragma once
#include "ABlobHud.h"

class ABlobMultiPlayerHud;
class ABlobScoreHud;

//-----------------------------------------------------------------------------
// Purpose: the HUD of a four player match (hud_4p.xur "HUD_4P",
// created by APlayerManager::CreatePlayers): a panel per player (ABlobMultiPlayerHud with the score, size, placing
// and score float HUDs), the level timer, the "ReadySetGo" of the match start (the input is disabled until it
// ends) and the winner's animation.
//-----------------------------------------------------------------------------
class ABlobHud_4P : public ABlobHud
{
public:
	TDECLARE_CLASS( ABlobHud_4P, ABlobHud );

	static TCONSTEXPR_VAR TINT NUM_PLAYERS = 4;

public:
	ABlobHud_4P();
	~ABlobHud_4P();

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
	virtual ABlobMultiPlayerHud* GetPlayer4Hud() override;                              // +0x150

private:
	ABlobMultiPlayerHud* m_apPanels[ NUM_PLAYERS ];    // 0x2F4 elements 0x1F, 0x15, 0xB, 1
	Toshi::TXUIElement*  m_pReadySetGo;                // 0x304 element 0x3C (an XuiGroup)
	Toshi::TXUIControl*  m_pWinner;                    // 0x308 control 0x2D
	ABlobScoreHud*       m_apScoreHuds[ NUM_PLAYERS ]; // 0x30C
};
