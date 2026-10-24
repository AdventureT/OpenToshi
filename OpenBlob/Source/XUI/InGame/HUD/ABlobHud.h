#pragma once
#include <XUI/TXUIScene.h>

class ABlobTimerHud;
class ABlobMultiPlayerHud;
class ABlobScoreHud;

//-----------------------------------------------------------------------------
// Purpose: the single player in-game HUD scene (hud.xur "HUD",
// created by APlayerManager::CreatePlayers): finds the sub HUDs (score, score floats, size, goals, the targeting
// reticle, the achievement banners...) and hands them to the game.
//-----------------------------------------------------------------------------
class ABlobHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobHud, Toshi::TXUIScene );

	static TCONSTEXPR_VAR TINT MAXBANNERS = 5;

	// The showcase animations of m_pMain (SetPendingShowcase, ABlobScoreHud shows the energy and the gate)
	enum SHOWCASE : TUINT32
	{
		SHOWCASE_Timer  = 0x1,
		SHOWCASE_Energy = 0x2,
		SHOWCASE_Size   = 0x4,
		SHOWCASE_Gate   = 0x8,
	};

	// The bits of m_uiPendingAchievements (AGameStatus::Achievements_Bronze/Silver/Gold), the others are the
	// achievements' bits
	enum ACHIEVEMENT : TUINT32
	{
		Achievements_Bronze = 0x1,
		Achievements_Silver = 0x2,
		Achievements_Gold   = 0x4,
		Achievements_Medals = Achievements_Bronze | Achievements_Silver | Achievements_Gold,
	};

public:
	ABlobHud();
	~ABlobHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                              // +0x04
	virtual TBOOL SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam ) override; // +0x78
	virtual void  OnCreated() override;                                              // +0x94

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransFrom() override;   // +0x118
	virtual void TransBackTo() override; // +0x11C

	//-----------------------------------------------------------------------------
	// ABlobHud (the multiplayer HUDs override them)
	//-----------------------------------------------------------------------------
	virtual void                 StartReadySetGo();                            // +0x134 the match start (AInGameState::OnFadeFinished)
	virtual void                 ShowWinner( TINT a_iPlayer );                 // +0x138 multiplayer: the winner a_iPlayer
	virtual Toshi::TXUIElement*  GetMessageElement( void** a_ppMessageOwner ); // +0x13C TNULL: none
	virtual void                 Reset();                                      // +0x140
	virtual ABlobMultiPlayerHud* GetPlayer1Hud();                              // +0x144
	virtual ABlobMultiPlayerHud* GetPlayer2Hud();                              // +0x148
	virtual ABlobMultiPlayerHud* GetPlayer3Hud();                              // +0x14C
	virtual ABlobMultiPlayerHud* GetPlayer4Hud();                              // +0x150

	// Shows the message element (GetMessageElement) with the text a_wszMessage, hides it for TNULL
	void ShowMessage( const TWCHAR* a_wszMessage );

	// Queues the banners of control 0xFD and 0xE4
	void QueueUnk25CBanner();
	void QueueUnk260Banner();

	// Queues the banner of achievements (a medal: the medal banner, others: the achievement banner)
	void AddPendingAchievements( TUINT32 a_uiAchievements );

	// Shows/hides the splats
	void ShowSplats( TUINT32 a_uiUnused = 0 ); // NOTE: the callers pass the colour (ARGB), which is not used
	void HideSplats();

	// Plays a random splat in the blob's colour (half of the times, when it isn't already playing)
	void ShowRandomSplat();

	// The reticle flashes red when the blob fires at its target
	void OnFire();

	// Queues a banner
	void QueueBanner( Toshi::TXUIElement* a_pBanner );

	// $deBlob: FUNCTION 00646d10
	Toshi::TXUIElement* GetGoalHud() const { return m_pGoalHud; }
	// $deBlob: FUNCTION 0075d430
	Toshi::TXUIControl* GetMaskParent() const { return m_pMaskParent; }
	Toshi::TXUIControl* GetLevelTimerControl() const { return m_pLevelTimer; }

	// Control 0xCB (AGameStatus::LevelData+0x7B8)
	// $deBlob: FUNCTION 006571b0
	Toshi::TXUIControl* GetMessageHud() const { return m_pMessageHud; }
	// Element 0xCC (its visual plays "Normal" with the messages of AGameStatus::LevelData::ShowMessage)
	// $deBlob: FUNCTION 005c08e0
	Toshi::TXUIElement* GetLifeHud() const { return m_pLifeHud; }

	// The shake prompt (ABlobShakeHud, control 0xD9)
	// $deBlob: FUNCTION 007519b0
	Toshi::TXUIControl* GetShakeHud() const { return m_pShakeHud; }
	void                SetPendingShowcase( TUINT32 a_uiShowcase ) { m_uiPendingShowcases = a_uiShowcase; }
	// $deBlob: FUNCTION 006513e0
	TUINT32 GetShownShowcases() const { return m_uiShownShowcases; }

	static ABlobHud* GetSingleton() { return sm_pSingleton; }

	// The hold timeline of the Z-jump (element 0x154, a TXUIVisual) and its "Stop" frame (ABlobZJumpComponent)
	// $deBlob: FUNCTION 0076aaa0
	Toshi::TXUIElement* GetHoldTimeline() const { return m_pHoldTimeline; }
	// $deBlob: FUNCTION 0076aab0
	TFLOAT GetHoldTimelineStopTime() const { return m_fStopTime; }

	// The challenge timer control (0x9E)
	Toshi::TXUIControl* GetChallengeTimerControl() const { return m_pChallengeTimer; }

	// The level timer (control 5) and the challenge timer (control 0x9E), both ABlobTimerHud scenes
	// $deBlob: FUNCTION 005d1370
	TINT GetBannerQueueSize() const { return m_iBannerQueueSize; }

	ABlobTimerHud* GetLevelTimer() const { return (ABlobTimerHud*)m_pLevelTimer; }
	ABlobTimerHud* GetChallengeTimer() const { return (ABlobTimerHud*)m_pChallengeTimer; }

protected:
	// The split screen HUDs (ABlobHud_2P/3P/4P::OnCreated): the panel a_iPanel of a_iPlayer, its
	// score (a_iScore), size (a_iSize, coloured by the HUD's element a_iSizeColoured), placing and score float HUDs;
	// returns the score HUD
	ABlobScoreHud* SetupPlayerPanel( TINT a_iPlayer, ABlobMultiPlayerHud*& a_rpPanel, TINT a_iPanel, TINT a_iScore, TINT a_iSizeColoured, TINT a_iSize, TINT a_iPlacing, TINT a_iScoreFloat );

	// The split screen HUDs: a_iPlayer's Z-jump lock element is the control a_iLock of its panel
	void SetPlayerLockElement( TINT a_iPlayer, ABlobMultiPlayerHud* a_pPanel, TINT a_iLock );

	// The split screen HUDs: the end of the match start (ReadySetGo stopped): the input, the GUI, the level timer and the
	// music go on
	void OnReadySetGoEnded();

private:
	// Plays the first banner of the queue and removes the finished ones
	void UpdateBanners( TFLOAT a_fDeltaTime );

	// Plays the first banner of the queue
	void ShowNextBanner();

	// Plays the showcase animation of m_pMain once
	void ShowShowcase( TUINT32 a_uiShowcase );

protected:
	Toshi::TXUIControl* m_pLevelTimer;                // 0x130 control 5
	TBOOL               m_bUnk134;                    // 0x134
	Toshi::TXUIControl* m_pChallengeTimer;            // 0x138 control 0x9E
	Toshi::TXUIControl* m_pChallengeHudElement;       // 0x13C control 0x45
	TWCHAR              m_wszGoalCount[ 128 ];        // 0x140 the level's goal count (LevelProperties+0x38)
	TUINT32             m_uiUnk240;                   // 0x240
	Toshi::TXUIControl* m_pTargetReticle;             // 0x244 control 0xE1 ("Targeted"/"StopTargeted")
	Toshi::TXUIElement* m_pMain;                      // 0x248 element 1 ("FreePaintHud", "StartSPLevel"...)
	Toshi::TXUIControl* m_pMaskParent;                // 0x24C control 2 (its "Mask" is set black)
	Toshi::TXUIElement* m_pGoalHud;                   // 0x250 element 0x19 (AGameStatus::LevelData+0x7B0)
	TUINT32             m_uiPendingShowcases;         // 0x254 SHOWCASE
	TUINT32             m_uiShownShowcases;           // 0x258 SHOWCASE
	Toshi::TXUIControl* m_pUnk25C;                    // 0x25C control 0xFD
	Toshi::TXUIControl* m_pUnk260;                    // 0x260 control 0xE4
	Toshi::TXUIElement* m_pAchievementBanner;         // 0x264 element 0x122 (has "AchievementName")
	Toshi::TXUIControl* m_pSplats;                    // 0x268 control 0xCE
	Toshi::TXUIControl* m_apSplats[ 10 ];             // 0x26C "Splat%d"
	Toshi::TXUIElement* m_pHoldTimeline;              // 0x294 element 0x154 (a TXUIVisual)
	TFLOAT              m_fStopTime;                  // 0x298 the "Stop" frame
	TBOOL               m_bTargeted;                  // 0x29C
	TFLOAT              m_fTargetedTime;              // 0x2A0
	Toshi::TXUIElement* m_apDrops[ 4 ];               // 0x2A4 "drop1"-"drop4" of the reticle
	Toshi::TXUIElement* m_pCircle;                    // 0x2B4 "Circle" of the reticle
	TUINT32             m_uiPendingAchievements;      // 0x2B8 ACHIEVEMENT
	Toshi::TXUIElement* m_pGp;                        // 0x2BC "gp"
	Toshi::TXUIElement* m_pAchievementName;           // 0x2C0 "AchievementName"
	Toshi::TXUIControl* m_pShakeHud;                  // 0x2C4 control 0xD9
	Toshi::TXUIElement* m_pMedalsBanner;              // 0x2C8 element 0x142
	Toshi::TXUIElement* m_aBannerQueue[ MAXBANNERS ]; // 0x2CC
	TINT                m_iBannerQueueSize;           // 0x2E0
	TBOOL               m_bBannerShowing;             // 0x2E4
	TFLOAT              m_fMedalBannerTime;           // 0x2E8
	Toshi::TXUIElement* m_pLifeHud;                   // 0x2EC element 0xCC
	Toshi::TXUIControl* m_pMessageHud;                // 0x2F0 control 0xCB (AGameStatus::LevelData+0x7B8)

	static ABlobHud* sm_pSingleton;
};
