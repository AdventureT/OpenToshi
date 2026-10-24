#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the lives counter of the single player HUD (ABlobHud's element 0xCC).
// APlayerManager's BlobData+0x24 points to it (SetPlayer).
//-----------------------------------------------------------------------------
class ABlobLifeHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobLifeHud, Toshi::TXUIScene );

public:
	ABlobLifeHud();
	~ABlobLifeHud();

	// Attaches the HUD to a player and shows the player's lives
	void SetPlayer( TUINT8 a_uiPlayer );

	// Shows a_iLives (a_bAnimate: plays "ChangeLives" unless a life is pending)
	void SetLives( TINT a_iLives, TBOOL a_bAnimate );

	// A life is pending (plays "StartLifePending") or not any more
	void SetLifePending( TBOOL a_bPending );

	TBOOL IsLifePending() const { return m_bLifePending; }

private:
	TWCHAR m_wszLives[ 8 ]; // 0x130
	TUINT8 m_uiPlayer;      // 0x140
	TBOOL  m_bLifePending;  // 0x141
};
