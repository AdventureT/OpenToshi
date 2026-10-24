#pragma once
#include <XUI/TXUIScene.h>
#include <Toshi/T2DList.h>

//-----------------------------------------------------------------------------
// Purpose: the score counter of the single player HUD:
// counts the displayed score up/down to the player's points, queues bonus texts that scroll into the score and
// spins the "ColourEnergy" meter on each score. AGameStatus::LevelData keeps it (+0x7B4).
//-----------------------------------------------------------------------------
class ABlobScoreHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobScoreHud, Toshi::TXUIScene );

	static TCONSTEXPR_VAR TINT MAX_BONUSES = 8;

	// A bonus text waiting to be added to the score (0x24 bytes)
	struct Bonus : public Toshi::T2GenericDList::Node
	{
		Bonus()
		    : m_pControl( TNULL ), m_iPoints( -1 ), m_bUnk10( TFALSE ) {}

		Toshi::TXUIControl* m_pControl;     // 0x08 "161"-"168"
		TINT                m_iPoints;      // 0x0C
		TBOOL               m_bUnk10;       // 0x10
		TWCHAR              m_wszText[ 8 ]; // 0x12
	};

	enum State
	{
		State_Idle       = 0,
		State_Collecting = 2, // the queued bonuses scroll into the score
	};

public:
	ABlobScoreHud();
	~ABlobScoreHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                              // +0x04
	virtual TBOOL SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam ) override; // +0x78
	virtual void  OnCreated() override;                                              // +0x94

	// Resets the counter for a new level
	void Reset();

	// $deBlob: FUNCTION 00654670
	void SetPlayer( TUINT8 a_uiPlayer )
	{
		TASSERT( a_uiPlayer < 4 );
		m_uiPlayer = a_uiPlayer;
	}

	// Adds a_iPoints to the player (and shows/queues a_iBonus, added after a_fDelay)
	void AddScore( TINT a_iPoints, TINT a_iBonus, TBOOL a_bCombo, TFLOAT a_fDelay );

	// Adds the oldest queued bonus to the score. Public: AGameStatus::LevelData::EndCombo calls it
	void CollectBonus();

private:
	TUINT8                m_uiPlayer;                // 0x130
	TINT                  m_iDisplayedScore;         // 0x134
	TINT                  m_iScore;                  // 0x138
	TWCHAR                m_wszScore[ 16 ];          // 0x13C
	TFLOAT                m_fCountTime;              // 0x15C
	TFLOAT                m_fBonusY;                 // 0x160 the y of the first bonus text
	TINT                  m_eState;                  // 0x164
	TFLOAT                m_fBonusDelay;             // 0x168
	Toshi::T2DList<Bonus> m_FreeBonuses;             // 0x16C
	Toshi::T2DList<Bonus> m_QueuedBonuses;           // 0x174
	Bonus                 m_aBonuses[ MAX_BONUSES ]; // 0x17C
	TBOOL                 m_bCountingDown;           // 0x29C
	Toshi::TXUIControl*   m_pColourEnergy;           // 0x2A0
	TFLOAT                m_fSpinTime;               // 0x2A4
	TBOOL                 m_bSpinning;               // 0x2A8
};
