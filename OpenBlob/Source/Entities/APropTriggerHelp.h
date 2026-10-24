#pragma once
#include "Entities/APropTrigger.h"

#include <File/TTRB.h>

class ABlobHelpHud;

TOSHI_NAMESPACE_START
class TXUIScene;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: a trigger that shows a help message or a full screen help topic.
//-----------------------------------------------------------------------------
class APropTriggerHelp
    : public APropTrigger
{
public:
	TDECLARE_CLASS( APropTriggerHelp, APropTrigger );

	enum HELPFLAG : TUINT8
	{
		HELPFLAG_Unk1          = 0x1,
		HELPFLAG_Unk2          = 0x2,
		HELPFLAG_Counted       = 0x4,
		HELPFLAG_NoTriggerType = 0x8,
	};

	// "HelpType"
	enum HELPTYPE : TINT8
	{
		HELPTYPE_Scene       = 0, // helphud.xur
		HELPTYPE_FullScreen  = 1,
		HELPTYPE_Hud         = 2,
		HELPTYPE_LetterBox   = 3,
		HELPTYPE_Collectable = 4,
	};

public:
	APropTriggerHelp();
	virtual ~APropTriggerHelp() override; // +0x08

	virtual void  PostCreate() override;                                                               // +0x20
	virtual void  ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void  SetBBox( Toshi::TVector4 a_vBBox ) override;                                         // +0x7C
	virtual void  Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual TBOOL CanObjectTrigger( Toshi::TObject* a_pObject ) override;                              // +0xE0
	virtual void  Trigger() override;                                                                  // +0xE4

	// ATriggerUtil
	virtual void TriggerAllTargets( ABaseEntity* a_pCaller ) override; // +0x0C

	// The blob pressed the read button in the trigger: triggers it (once, TRIGGERFLAG 0x100) and leaves the blob's
	// help trigger (ABlobEntity 0xB40)
	void Activate();

	TUINT8 GetHelpFlags() const { return m_uiHelpFlags; }

	// Shows a context help (a_uiHelp: one bit, the HUD's help popup with the matching string) once per level
	static void ShowContextHelp( TUINT32 a_uiHelp );

	// Shows a help: a_iHelpType 0 (paused message), 1 (full screen topic), 2 (HUD popup), 3 (letterbox popup),
	// 4 (collectable message); a_bForce ignores a help already shown
	static void ShowHelp( APropTriggerHelp* a_pTrigger, TINT8 a_iHelpType, TINT a_iStringId, TINT8 a_iFullScreenTopic, TBOOL a_bBlocking, TFLOAT a_fDisplayTime, TINT8 a_iSpeaker, const TWCHAR* a_wszText, TBOOL a_bForce );

	// Clears the state of the shown help
	static void ResetHelpState();

	// The full screen help's TRB is streamed: creates the help ("<topic>.xur") and transitions to it from the help
	// menu (TNULL: the HUD); a_pTrigger is the trigger that showed it
	static void OpenFullscreenHelp( APropTriggerHelp* a_pTrigger );

	// The full screen help is deleted: its TRB is closed and the topic cleared
	static void CloseFullscreenHelp();

	// Counts the help as collected once (HELPFLAG_Counted)
	void SetCollected();

	static const TCHAR* GetFullscreenTopicName( TINT a_iTopic ) { return sm_aszFullscreenTopics[ a_iTopic ]; }
	static void         SetFullscreenTopic( TINT8 a_iTopic ) { sm_iCurrFullscreenTopic = a_iTopic; }
	static Toshi::TTRB* GetFullscreenTRB() { return &sm_oTRB; }

	// Loads a_szFileName into sm_oTRB at once (with the XUI heap) and initialises its assets
	static void LoadFullscreenTRB( const TCHAR* a_szFileName );
	// The help menu the full screen help is opened from (ABlobHelp)
	static void SetHelpMenu( Toshi::TXUIScene* a_pMenu ) { ms_pHelpMenu = a_pMenu; }

	static ABlobHelpHud* GetHelpHud() { return ms_pHelpHud; }
	static void          SetHelpHud( ABlobHelpHud* a_pHelpHud ) { ms_pHelpHud = a_pHelpHud; }
	static ABlobHelpHud* GetLetterBoxHelpHud() { return ms_pLetterBoxHelpHud; }
	static void          SetLetterBoxHelpHud( ABlobHelpHud* a_pHelpHud ) { ms_pLetterBoxHelpHud = a_pHelpHud; }
	static TBOOL         IsHelpShown() { return ms_bHelpShown; }
	static TBOOL         IsHelpBlocking() { return ms_bHelpBlocking; }
	static TBOOL         IsUnk9B720C() { return ms_bUnk9B720C; }
	static void          SetUnk9B720C( TBOOL a_bValue ) { ms_bUnk9B720C = a_bValue; }
	static TBOOL         IsHelpHudShown() { return ms_bHelpHudShown; }
	static void          SetHelpHudShown( TBOOL a_bShown ) { ms_bHelpHudShown = a_bShown; }
	// The full screen help topic on screen (-1: none)
	static TINT8 GetFullscreenTopic() { return sm_iCurrFullscreenTopic; }
	// Set when APlayerManager shows the help of a key frame camera
	static TBOOL IsUnk9B720D() { return ms_bUnk9B720D; }
	static void  SetUnk9B720D( TBOOL a_bValue ) { ms_bUnk9B720D = a_bValue; }

	// $deBlob: FUNCTION 00593350
	TINT GetSSCamID() const { return m_iSSCamID; }
	// $deBlob: FUNCTION 00593330
	TINT8 GetHelpType() const { return m_iHelpType; }

private:
	static TBOOL             ms_bHelpDisabled; // no help while set
	static TBOOL             ms_bHelpShown;
	static TBOOL             ms_bHelpBlocking; // the shown help blocks the other ones
	static TBOOL             ms_bHelpHudShown;
	static TBOOL             ms_bUnk9B720C;
	static TBOOL             ms_bUnk9B720D;
	static ABlobHelpHud*     ms_pHelpHud;            // ABlobHud's control 0x153
	static ABlobHelpHud*     ms_pLetterBoxHelpHud;   // ABlobLetterBoxHud's
	static TUINT32           ms_uiShownContextHelps; // ShowContextHelp's bits
	static Toshi::TXUIScene* ms_pHelpMenu;
	static Toshi::TTRB       sm_oTRB;

	static TCONSTEXPR_VAR TINT NUM_FULLSCREEN_TOPICS = 18;
	static const TCHAR* const  sm_aszFullscreenTopics[ NUM_FULLSCREEN_TOPICS ]; // the TRB/XUR names

	static TINT8 sm_iCurrFullscreenTopic;
	static TINT  sm_iCurrFullscreenMessageID;

protected:
	TINT   m_iLocaleID;    // 0xC0 -1
	TINT   m_iSSCamID;     // 0xC4 -1
	TINT8  m_iTopic;       // 0xC8 -1
	TINT8  m_iHelpType;    // 0xC9 -1
	TINT8  m_iSpeaker;     // 0xCA
	TUINT8 m_uiHelpFlags;  // 0xCB HELPFLAG
	TFLOAT m_fDisplayTime; // 0xCC 5.0f
};
