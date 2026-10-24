#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the paint colour indicator of a player: its
// visual's figures are filled with the blob's colour.
// NOTE: retail never calls SetPlayer, so no blob ever gets an ammo HUD and the
// ABlobColourComponent calls are dead in retail.
//-----------------------------------------------------------------------------
class ABlobAmmoHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobAmmoHud, Toshi::TXUIScene );

public:
	ABlobAmmoHud();
	~ABlobAmmoHud();

	// Attaches the HUD to a player's blob
	void SetPlayer( TUINT8 a_uiPlayer );

	// Shows the colour (ABlobColourComponent::COLOUR_*), hides the HUD for 0 (white)
	void SetColour( TUINT32 a_uiColour );

private:
	TUINT8 m_uiPlayer; // 0x130
};
