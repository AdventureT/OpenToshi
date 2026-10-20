#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the placing text of a player of the multiplayer HUD (scene "PlacingHud"): it shows the text
// of string 0xC0 while the player is first in the results (AGameStatus::GetRank) and nothing otherwise.
// The player is set by SetPlayer (ABlobHud's per player HUDs).
//-----------------------------------------------------------------------------
class ABlobPlacingHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobPlacingHud, Toshi::TXUIScene );

public:
	ABlobPlacingHud();
	~ABlobPlacingHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	// Sets the player whose placing is shown
	void SetPlayer( TUINT8 a_uiPlayer );

private:
	TUINT8 m_uiPlayer; // 0x130 the player
	TINT8  m_iRank;    // 0x131 the rank shown last (AGameStatus::GetRank), -1: none
};
