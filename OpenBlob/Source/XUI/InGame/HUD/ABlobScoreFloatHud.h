#pragma once
#include <XUI/TXUIScene.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: the score popup floating over a painted building:
// four of them are in hud.xur (ABlobHud gives them to AGameStatus::LevelData).
//-----------------------------------------------------------------------------
class ABlobScoreFloatHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobScoreFloatHud, Toshi::TXUIScene );

public:
	ABlobScoreFloatHud();
	~ABlobScoreFloatHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	// Shows "+points xmultiplier\nname\nbonus bonusname" at a_rPosition (the bonus line only when a_iBonus != 0)
	void ShowScore( TINT a_iPoints, TINT a_iMultiplier, const TWCHAR* a_wszName, const Toshi::TVector4& a_rPosition, const TWCHAR* a_wszBonusName, TINT a_iBonus );

	// Shows a_wszText at a_rPosition ("TimePickup" animation if a_bTimePickup)
	void ShowText( const TWCHAR* a_wszText, const Toshi::TVector4& a_rPosition, TBOOL a_bTimePickup );

	// $deBlob: FUNCTION 00653eb0
	void SetPlayer( TINT8 a_iPlayer ) { m_iPlayer = a_iPlayer; }

private:
	TWCHAR          m_wszText[ 256 ]; // 0x130
	TFLOAT          m_fTimeLeft;      // 0x330 fades out over the last 20% (BlobScoringProperties+0x00 when shown)
	TINT8           m_iPlayer;        // 0x334
	Toshi::TVector4 m_vPosition;      // 0x338 world position
};
