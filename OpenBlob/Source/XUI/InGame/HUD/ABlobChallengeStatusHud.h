#pragma once
#include <XUI/TXUIScene.h>

namespace Toshi
{
class TXUIControl;
class TXUIFigure;
} // namespace Toshi

class AChallenge;
class ALandmarkInstance;

//-----------------------------------------------------------------------------
// Purpose: the status of the current challenge on the single player HUD (ABlobHud's control 0x45,
// AChallengeManager's HUD element): the generic
// counter, or for the paint and landmark challenges one paint droplet per colour still to paint ("ChallengePaint1"
// to "ChallengePaint6" groups of 1 to 6 "Paint%d" figures with a "PaintNumber%d" count).
//-----------------------------------------------------------------------------
class ABlobChallengeStatusHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobChallengeStatusHud, Toshi::TXUIScene );

	static TCONSTEXPR_VAR TINT iNUMCOLOURS  = 8;
	static TCONSTEXPR_VAR TINT MAX_COLOURS  = 6;  // the "ChallengePaint%d" groups
	static TCONSTEXPR_VAR TINT NUM_DROPLETS = 21; // 1 + 2 + ... + 6

	// The colours of the counts, -1: any colour (the white of the colour table)
	static const TINT s_aiColours[ iNUMCOLOURS ];

public:
	ABlobChallengeStatusHud();
	~ABlobChallengeStatusHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	// A target of a paint challenge was painted (or unpainted with a_bUncomplete) in its colour
	void UpdateColourCount( const void* a_pTarget, TBOOL a_bUncomplete );

	// The droplets of the paint challenge's targets
	void SetupPaintChallenge( AChallenge* a_pChallenge );

	// The droplets of the landmark challenge's triggers
	void SetupLandmarkChallenge( AChallenge* a_pChallenge );

	// The paint left of a landmark trigger changed
	void SetTriggerPaintLeft( ALandmarkInstance* a_pLandmark, TINT a_iTrigger, TINT a_iPaintLeft );

	// The current challenge starts: its animation and its droplets
	void OnChallengeStart( AChallenge* a_pChallenge );

	// The text of the counter (the paint number of the single droplet in a civilian challenge)
	void SetCounterText( const TWCHAR* a_wszText );

private:
	// Shows the droplet group of m_iNumColours and the counts in their colours
	void UpdateDroplets();

	Toshi::TXUIFigure*  GetDroplet( TINT a_iNumColours, TINT a_iIndex ) const { return m_apDroplets[ ( a_iNumColours * ( a_iNumColours - 1 ) ) / 2 + a_iIndex ]; }
	Toshi::TXUIControl* GetDropletNumber( TINT a_iNumColours, TINT a_iIndex ) const { return m_apDropletNumbers[ ( a_iNumColours * ( a_iNumColours - 1 ) ) / 2 + a_iIndex ]; }

private:
	TWCHAR              m_awszColourCount[ iNUMCOLOURS ][ 8 ]; // 0x130
	TINT                m_aiColourCount[ iNUMCOLOURS ];        // 0x1B0 -1: not in the challenge
	TUINT8              m_aUnk1D0[ 0x20 ];                     // 0x1D0
	Toshi::TXUIControl* m_pType;                               // 0x1F0 control 0x9D, its visual plays the challenge type
	Toshi::TXUIElement* m_pPaints;                             // 0x1F4 element 0x46, plays "Paint1"..."Paint6"
	Toshi::TXUIFigure*  m_apDroplets[ NUM_DROPLETS ];          // 0x1F8 "Paint%d" of "ChallengePaint1"..."ChallengePaint6"
	Toshi::TXUIControl* m_apDropletNumbers[ NUM_DROPLETS ];    // 0x24C "PaintNumber%d"
	Toshi::TXUIControl* m_pGenericCounter;                     // 0x2A0 "GenericCounter"
	TINT                m_iNumColours;                         // 0x2A4
};
