#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the medal banner of the single player HUD (ABlobHud's element 0x142, m_pUnk2C8; ABlobHud's medal
// branch of ShowNextBanner). Its element 0x143 (SetMedal) plays "MedalAchieved",
// its text (0x145) shows the medal's name and the "Medals" element of the "Medal" control plays "Bronze", "Silver" or
// "Gold". The class registers itself in AGameStatus's LevelData (m_pMedalsHud) while it exists.
//-----------------------------------------------------------------------------
class ABlobMedalsHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobMedalsHud, Toshi::TXUIScene );

public:
	ABlobMedalsHud();
	~ABlobMedalsHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	// Sets the medal element and finds its text (element 0x145 of it), returns the text
	Toshi::TXUIElement* SetMedal( Toshi::TXUIElement* a_pMedal );

	// Plays the "Normal" animation and clears m_iUnk140 and m_iUnk144
	void ShowNormal();

	// $deBlob: FUNCTION 00653c80
	Toshi::TXUIElement* GetMedal() const { return m_pMedal; }
	// $deBlob: FUNCTION 00653ca0
	Toshi::TXUIControl* GetMedalText() const { return m_pMedalText; }
	// $deBlob: FUNCTION 00653c90
	Toshi::TXUIElement* GetMedals() const { return m_pMedals; }

private:
	Toshi::TXUIElement* m_pMedal;     // 0x130 the medal element, plays "MedalAchieved"
	Toshi::TXUIControl* m_pMedalText; // 0x134 its text (set with SetTextPointer)
	Toshi::TXUIElement* m_pMedals;    // 0x138 "Medals" of the "Medal" control, plays "Bronze", "Silver" or "Gold"
	Toshi::TXUIElement* m_pUnk13C;    // 0x13C
	TINT                m_iUnk140;    // 0x140 -1
	TINT                m_iUnk144;    // 0x144 -1
};
