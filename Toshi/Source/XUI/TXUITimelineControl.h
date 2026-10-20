#pragma once
#include "TXUITimeline.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Plays the timelines and named frames of an element (0x50 bytes, a member of TXUICanvas, TXUIGroup, TXUIVisual,
// TXUIScene...). Source\Xui\TXUITimelineControl.cpp
//-----------------------------------------------------------------------------
class TXUITimelineControl
{
public:
	// The standard named frames (mas_StandardNamedFrames), -1 for a custom named frame
	using StdNamedFrame = TINT;
	enum StdNamedFrame_ : StdNamedFrame
	{
		StdNamedFrame_None                     = -1,
		StdNamedFrame_Normal                   = 0,
		StdNamedFrame_EndNormal                = 1,
		StdNamedFrame_NormalDisable            = 2,
		StdNamedFrame_EndNormalDisable         = 3,
		StdNamedFrame_Focus                    = 4,
		StdNamedFrame_EndFocus                 = 5,
		StdNamedFrame_FocusLoop                = 6,
		StdNamedFrame_FocusDisable             = 7,
		StdNamedFrame_EndFocusDisable          = 8,
		StdNamedFrame_InactiveFocusLoop        = 9,
		StdNamedFrame_InitFocus                = 10,
		StdNamedFrame_EndInitFocus             = 11,
		StdNamedFrame_InitFocusDisable         = 12,
		StdNamedFrame_EndInitFocusDisable      = 13,
		StdNamedFrame_KillFocus                = 14,
		StdNamedFrame_Press                    = 15,
		StdNamedFrame_EndPress                 = 16,
		StdNamedFrame_NormalPress              = 17,
		StdNamedFrame_EndNormalPress           = 18,
		StdNamedFrame_PressDisable             = 19,
		StdNamedFrame_EndPressDisable          = 20,
		StdNamedFrame_NormalCheck              = 21,
		StdNamedFrame_EndNormalCheck           = 22,
		StdNamedFrame_FocusCheck               = 23,
		StdNamedFrame_EndFocusCheck            = 24,
		StdNamedFrame_FocusCheckLoop           = 25, // "FocusLoop" again
		StdNamedFrame_InitFocusCheck           = 26,
		StdNamedFrame_EndInitFocusCheck        = 27,
		StdNamedFrame_NormalCheckDisable       = 28,
		StdNamedFrame_EndNormalCheckDisable    = 29,
		StdNamedFrame_FocusCheckDisable        = 30,
		StdNamedFrame_EndFocusCheckDisable     = 31,
		StdNamedFrame_InitFocusCheckDisable    = 32,
		StdNamedFrame_EndInitFocusCheckDisable = 33,
		StdNamedFrame_NormalSel                = 34,
		StdNamedFrame_EndNormalSel             = 35,
		StdNamedFrame_NormalSelCheck           = 36,
		StdNamedFrame_EndNormalSelCheck        = 37,
		StdNamedFrame_NormalSelDisable         = 38,
		StdNamedFrame_EndNormalSelDisable      = 39,
		StdNamedFrame_NormalSelCheckDisable    = 40,
		StdNamedFrame_EndNormalSelCheckDisable = 41,
		StdNamedFrame_ScrollMore               = 42,
		StdNamedFrame_EndScrollMore            = 43,
		StdNamedFrame_Scrolling                = 44,
		StdNamedFrame_EndScrolling             = 45,
		StdNamedFrame_ScrollMoreDisable        = 46,
		StdNamedFrame_EndScrollMoreDisable     = 47,
		StdNamedFrame_TransTo                  = 48,
		StdNamedFrame_EndTransTo               = 49,
		StdNamedFrame_TransFrom                = 50,
		StdNamedFrame_EndTransFrom             = 51,
		StdNamedFrame_TransBackTo              = 52,
		StdNamedFrame_EndTransBackTo           = 53,
		StdNamedFrame_TransBackFrom            = 54,
		StdNamedFrame_EndTransBackFrom         = 55,
		StdNamedFrame_NUMOF                    = 56,
	};

	// A named frame of the element (0x18 bytes), created from its XURXUINamedFrameData
	class NamedFrame : public T2DList<NamedFrame>::Node
	{
	public:
		// NOTE: retail only links the node, the other members are set by CreateNamedFrames
		NamedFrame()
		{
#ifdef DEBLOB_COMMUNITY_PATCH
			// NOTE: CreateNamedFrames reads m_wszName even when the data has no name
			m_eStdNamedFrame = StdNamedFrame_None;
			m_wszName        = TNULL;
			m_wszTarget      = TNULL;
			m_uiTime         = 0;
			m_eCommand       = 0;
#endif
		}

		// $deBlob: FUNCTION 006c51e0
		~NamedFrame() = default;

		TFLOAT GetTime() const { return TFLOAT( m_uiTime ) * sm_fFrameTime; }

	public:
		StdNamedFrame m_eStdNamedFrame; // 0x08
		const TWCHAR* m_wszName;        // 0x0C
		const TWCHAR* m_wszTarget;      // 0x10 the named frame Command_GoTo* jumps to
		TUINT16       m_uiTime;         // 0x14 in frames
		TUINT16       m_eCommand;       // 0x16 XURXUINamedFrameData::Command
	};

public:
	TXUITimelineControl( TXUIElement* a_pElement );
	~TXUITimelineControl();

	TBOOL Create( TXUIResource& a_rResource, XURXUIObjectData* a_pData );
	TBOOL CreateNamedFrames( TXUIResource& a_rResource, XURXUINamedFrameData* a_pNamedFrames, TUINT32 a_uiNumNamedFrames );
	TBOOL CreateTimelines( TXUIResource& a_rResource, XURXUITimelineData* a_pTimelines, TUINT32 a_uiNumTimelines );

	void Tick( TFLOAT a_fDeltaTime );

	void PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd );
	void PlayAnimation( const TWCHAR* a_wszStart, const TWCHAR* a_wszEnd );
	void PlayAnimation( StdNamedFrame a_eStart, StdNamedFrame a_eEnd, TINT a_iIndex );
	void PlayAnimation_Imp( NamedFrame* a_pStart, NamedFrame* a_pEnd, TBOOL a_bContinue );
	void ContinueAnimation();
	void StopAnimation();
	void SetTimeLineTime( TFLOAT a_fTime, TBOOL a_bLoop );
	void ResetAnimation();
	void SetPropertyThief( TXUIElement* a_pThief );

	static StdNamedFrame GetStdNamedFrame( const TWCHAR* a_wszName );

	NamedFrame*   FindNamedFrame( const TWCHAR* a_wszName );
	NamedFrame*   GetNamedFrame( TFLOAT a_fTime, TFLOAT a_fFrameTime );
	NamedFrame*   FindNextNamedFrame( NamedFrame* a_pNamedFrame );
	TXUITimeline* FindTimeline( const TWCHAR* a_wszName );
	TFLOAT        GetNamedFrameTime( const TWCHAR* a_wszName );

	void Reset();
	void SendMessage( StdNamedFrame a_eFrame, const TWCHAR* a_wszName );

	TXUIElement* GetElement() const { return m_pElement; }
	TFLOAT       GetTimeLineTime() const { return m_fTimeLineTime; }
	// $deBlob: FUNCTION 0062cfa0
	TFLOAT       GetTimelineLength() const { return m_fTimelineLength; }
	// $deBlob: FUNCTION 005c0a10
	TBOOL        IsPlaying() const { return m_bPlaying; }

	// $deBlob: FUNCTION 006cf470
	void SetLoop( TBOOL a_bLoop ) { m_bLoop = a_bLoop; }

public:
	static const TWCHAR* mas_StandardNamedFrames[ StdNamedFrame_NUMOF ];
	static const TFLOAT  sm_fFrameTime;

public:
	NamedFrame*           m_pCurrentNamedFrame;  // 0x00
	NamedFrame*           m_pNextNamedFrame;     // 0x04
	TUINT32               m_uiUnknown8;          // 0x08 unknown (never set by retail)
	NamedFrame*           m_pEndNamedFrame;      // 0x0C
	TFLOAT                m_fStartTime;          // 0x10
	TFLOAT                m_fTimeLineTime;       // 0x14 -1 when stopped
	TFLOAT                m_fNextNamedFrameTime; // 0x18
	TFLOAT                m_fEndNamedFrameTime;  // 0x1C
	TFLOAT                m_fTimelineRangeBegin; // 0x20 -1 if no range is played
	TFLOAT                m_fTimelineRangeEnd;   // 0x24
	TXUIElement*          m_pElement;            // 0x28
	TXUIElement*          m_pPropertyThief;      // 0x2C
	NamedFrame*           m_aNamedFrames;        // 0x30
	TXUITimeline*         m_aTimelines;          // 0x34
	T2DList<NamedFrame>   m_oNamedFrames;        // 0x38
	T2DList<TXUITimeline> m_oTimelines;          // 0x40
	TFLOAT                m_fTimelineLength;     // 0x48 the end time of the longest timeline
	TBOOL                 m_bLoop;               // 0x4C
	TBOOL                 m_bEnabled;            // 0x4D PlayAnimation_Imp does nothing when not set
	TBOOL                 m_bPlaying;            // 0x4E
};

TOSHI_NAMESPACE_END
