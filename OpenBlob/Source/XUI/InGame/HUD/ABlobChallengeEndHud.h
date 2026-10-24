#pragma once
#include <XUI/TXUIScene.h>
#include <XUI/TXUIText.h>

class AChallenge;

//-----------------------------------------------------------------------------
// Purpose: the "challenge complete/failed" scene (challengeend.xur):
// created by AChallengeManager::Update and APropSwitchPressure once a challenge is over; waits for
// the challenge camera, plays Blob's cheer, shows the result and offers Retry/Back after a failure.
//-----------------------------------------------------------------------------
class ABlobChallengeEndHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobChallengeEndHud, Toshi::TXUIScene );

public:
	ABlobChallengeEndHud();
	~ABlobChallengeEndHud();

	// The challenge failed and Retry is shown: the pause is blocked meanwhile
	// $deBlob: FUNCTION 0064eb30
	TBOOL IsRetrying() const { return m_bRetrying; }

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                        // +0x04
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0
	virtual TBOOL OnNotifySetFocus( Toshi::TXUIElement* a_pSource ) override;                  // +0xB4

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void EndTransBackFrom() override; // +0x130

	// The challenge that is over and whether it failed
	void SetChallenge( AChallenge* a_pChallenge, TBOOL a_bFailed );

	// The count shown on the gate (APropSwitchPressure), -1: none
	// $deBlob: FUNCTION 0079f4c0
	void SetCount( TINT a_iCount ) { m_iCount = a_iCount; }

private:
	// Gives the input back to Blob and tells AChallengeManager the result
	void Finish();

private:
	TBOOL               m_bShown;          // 0x130 the result is shown
	TFLOAT              m_fTimer;          // 0x134 -1
	Toshi::TXUIControl* m_pResult;         // 0x138 control 3
	Toshi::TXUIElement* m_pGate;           // 0x13C element 77
	Toshi::TXUIText*    m_pTitle;          // 0x140 element 72
	Toshi::TXUIText*    m_pCount;          // 0x144 element 109
	TBOOL               m_bFailed;         // 0x148
	TBOOL               m_bUnk149;         // 0x149
	AChallenge*         m_pChallenge;      // 0x14C
	TINT                m_iCount;          // 0x150 -1
	TWCHAR              m_wszCount[ 128 ]; // 0x154
	TBOOL               m_bWaitingForBlob; // 0x254 waits for Blob's state 5 to show the letterbox
	Toshi::TXUIControl* m_pRetry;          // 0x258 control 74
	Toshi::TXUIControl* m_pBack;           // 0x25C control 75
	TBOOL               m_bRetrying;       // 0x260
	TBOOL               m_bRespawnPending; // 0x261
	Toshi::TXUIElement* m_pUnk264;         // 0x264 element 5
	Toshi::TXUIElement* m_pUnk268;         // 0x268 element 35
	Toshi::TXUIText*    m_pHint;           // 0x26C element 71
};
