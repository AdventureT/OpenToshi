#pragma once
#include <XUI/TXUIScene.h>

class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: the life lost screen (lifelost.xur): ABlobEntity's STATE_DEAD opens it over ABlobHud with a
// radial wipe centred on the blob and the lives left; once it's in, the blob is moved to its respawn point (or the
// game is over), and once the terrain is streamed in the blob respawns and the wipe opens again.
//-----------------------------------------------------------------------------
class ABlobLifeLostHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobLifeLostHud, Toshi::TXUIScene );

public:
	ABlobLifeLostHud();
	~ABlobLifeLostHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                              // +0x04
	virtual TBOOL SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam ) override; // +0x78
	virtual void  OnCreated() override;                                              // +0x94

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void EndTransTo() override;       // +0x124
	virtual void EndTransBackFrom() override; // +0x130

	// Sets the dead blob: centres the wipe on it and shows its lives
	void SetBlob( ABlobEntity* a_pBlob );

private:
	TBOOL               m_bRespawnReady;  // 0x130 the blob was moved to its respawn point (EndTransTo)
	TWCHAR              m_wszLives[ 16 ]; // 0x132 "x %d"
	ABlobEntity*        m_pBlob;          // 0x154
	Toshi::TXUIControl* m_pWipe;          // 0x158 element 1 ("Hide"/"Show")
	Toshi::TXUIControl* m_pLives;         // 0x15C control 5
	Toshi::TXUIControl* m_pText;          // 0x160 control 4 (string 0x199, or 0x171 when the time ran out)
};
