#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the HUD panel of a player in the split screen (TClass "ABlobMultiPlayerHud";
// ABlobHud_2P/3P/4P find one per player): renders inside its own viewport, shows
// "Threat" over the blob while another player's Z-jump locks on it and the "PlayerMarker" for a while after Display
// (the match start).
//-----------------------------------------------------------------------------
class ABlobMultiPlayerHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobMultiPlayerHud, Toshi::TXUIScene );

public:
	ABlobMultiPlayerHud();
	~ABlobMultiPlayerHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void Render() override;                    // +0x14
	virtual void OnCreated() override;                 // +0x94

	// Shows the player marker over the blob for a while (the match start, ABlobHud_2P/3P/4P::Tick)
	void ShowPlayerMarker();

	void SetPlayer( TUINT8 a_uiPlayer ) { m_uiPlayer = a_uiPlayer; }

private:
	TUINT8              m_uiPlayer;          // 0x130
	Toshi::TXUIControl* m_pThreat;           // 0x134 "Threat"
	Toshi::TXUIControl* m_pPlayerMarker;     // 0x138 "PlayerMarker"
	TFLOAT              m_fPlayerMarkerTime; // 0x13C before the marker hides
	TBOOL               m_bThreatened;       // 0x140 another blob's Z-jump locks on this one
	TFLOAT              m_fThreatTime;       // 0x144 the threat still follows the blob after the lock is gone
};
