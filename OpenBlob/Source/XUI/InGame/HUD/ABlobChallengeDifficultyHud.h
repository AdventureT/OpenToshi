#pragma once
#include <XUI/TXUIScene.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: the difficulty stars over the challenge nearest to Blob: plays "Show1Star", "Show2Stars"
// or "Show3Stars" on its visual, follows the challenge's marker on the screen and fades with the distance (hidden
// beyond 50 units on XZ).
//-----------------------------------------------------------------------------
class ABlobChallengeDifficultyHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobChallengeDifficultyHud, Toshi::TXUIScene );

public:
	ABlobChallengeDifficultyHud();
	~ABlobChallengeDifficultyHud();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	// Plays the animation of 1 to 3 stars (a_iStars 0 to 2)
	void PlayStarsAnimation( TINT a_iStars );

private:
	Toshi::TVector4 m_vPosition; // 0x130 the challenge's marker, then its canvas position
};
