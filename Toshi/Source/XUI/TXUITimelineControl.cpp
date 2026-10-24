#include "ToshiPCH.h"
#include "TXUITimelineControl.h"
#include "TXUI.h"
#include "TXUIScene.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

const TWCHAR* TXUITimelineControl::mas_StandardNamedFrames[ StdNamedFrame_NUMOF ] = {
	L"Normal",
	L"EndNormal",
	L"NormalDisable",
	L"EndNormalDisable",
	L"Focus",
	L"EndFocus",
	L"FocusLoop",
	L"FocusDisable",
	L"EndFocusDisable",
	L"InactiveFocusLoop",
	L"InitFocus",
	L"EndInitFocus",
	L"InitFocusDisable",
	L"EndInitFocusDisable",
	L"KillFocus",
	L"Press",
	L"EndPress",
	L"NormalPress",
	L"EndNormalPress",
	L"PressDisable",
	L"EndPressDisable",
	L"NormalCheck",
	L"EndNormalCheck",
	L"FocusCheck",
	L"EndFocusCheck",
	L"FocusLoop",
	L"InitFocusCheck",
	L"EndInitFocusCheck",
	L"NormalCheckDisable",
	L"EndNormalCheckDisable",
	L"FocusCheckDisable",
	L"EndFocusCheckDisable",
	L"InitFocusCheckDisable",
	L"EndInitFocusCheckDisable",
	L"NormalSel",
	L"EndNormalSel",
	L"NormalSelCheck",
	L"EndNormalSelCheck",
	L"NormalSelDisable",
	L"EndNormalSelDisable",
	L"NormalSelCheckDisable",
	L"EndNormalSelCheckDisable",
	L"ScrollMore",
	L"EndScrollMore",
	L"Scrolling",
	L"EndScrolling",
	L"ScrollMoreDisable",
	L"EndScrollMoreDisable",
	L"TransTo",
	L"EndTransTo",
	L"TransFrom",
	L"EndTransFrom",
	L"TransBackTo",
	L"EndTransBackTo",
	L"TransBackFrom",
	L"EndTransBackFrom",
};

const TFLOAT TXUITimelineControl::sm_fFrameTime = 1.0f / 60.0f;

// Sends a_eMessage to the element and up its parent controls until one handles it
static void TXUITimelineControl_SendMessageUp( TXUIElement* a_pElement, TINT a_eMessage, const TWCHAR* a_wszParam )
{
	TBOOL bHandled = a_pElement->SendMessage( a_eMessage, a_wszParam );

	for ( T2GUIElement* pParent = a_pElement->GetParent(); !bHandled && pParent != TNULL; pParent = pParent->GetParent() )
	{
		TXUIElement* pXUIParent = TSTATICCAST( TXUIElement, pParent );

		if ( !pXUIParent->IsA( &TGetClass( TXUIControl ) ) )
			return;

		bHandled = pXUIParent->SendMessage( a_eMessage, a_wszParam );
	}
}

// $deBlob: FUNCTION 006c3aa0
TXUITimelineControl::TXUITimelineControl( TXUIElement* a_pElement )
{
	m_pCurrentNamedFrame  = TNULL;
	m_pNextNamedFrame     = TNULL;
	m_pEndNamedFrame      = TNULL;
	m_fStartTime          = 0.0f;
	m_fTimeLineTime       = -1.0f;
	m_fNextNamedFrameTime = 0.0f;
	m_fTimelineRangeBegin = -1.0f;
	m_fTimelineRangeEnd   = -1.0f;
	m_pElement            = a_pElement;
	m_pPropertyThief      = TNULL;
	m_aNamedFrames        = TNULL;
	m_aTimelines          = TNULL;
	m_fTimelineLength     = 0.0f;
	m_bLoop               = TFALSE;
	m_bEnabled            = TTRUE;
	m_bPlaying            = TFALSE;
}

// $deBlob: FUNCTION 006c3b20
// $deBlob: FUNCTION 006c52b0
TXUITimelineControl::~TXUITimelineControl()
{
	m_oNamedFrames.Clear();

	if ( m_aNamedFrames != TNULL )
		delete[] m_aNamedFrames;

	m_oTimelines.Clear();

	if ( m_aTimelines != TNULL )
		delete[] m_aTimelines;
}

// $deBlob: FUNCTION 006c3ba0
TBOOL TXUITimelineControl::Create( TXUIResource& a_rResource, XURXUIObjectData* a_pData )
{
	if ( a_pData->GetNamedFrames() != TNULL )
	{
		TBOOL bRes = CreateNamedFrames( a_rResource, a_pData->GetNamedFrames(), a_pData->GetNumNamedFrames() );
		TASSERT( TTRUE == bRes );
	}

	if ( a_pData->GetTimelines() != TNULL )
	{
		TBOOL bRes = CreateTimelines( a_rResource, a_pData->GetTimelines(), a_pData->GetNumTimelines() );
		TASSERT( TTRUE == bRes );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c4a70
TBOOL TXUITimelineControl::CreateNamedFrames( TXUIResource& a_rResource, XURXUINamedFrameData* a_pNamedFrames, TUINT32 a_uiNumNamedFrames )
{
	m_aNamedFrames = new ( TXUI::ms_pXUIMemoryBlock ) NamedFrame[ a_uiNumNamedFrames ];

	for ( TUINT32 i = 0; i < a_uiNumNamedFrames; i++ )
	{
		NamedFrame*           pNamedFrame = &m_aNamedFrames[ i ];
		XURXUINamedFrameData* pData       = &a_pNamedFrames[ i ];

		if ( pData->m_uiName != 0 )
			pNamedFrame->m_wszName = a_rResource.GetString( pData->m_uiName );

		if ( pData->m_uiTarget != 0 )
			pNamedFrame->m_wszTarget = a_rResource.GetString( pData->m_uiTarget );

		pNamedFrame->m_uiTime   = pData->m_uiTime;
		pNamedFrame->m_eCommand = pData->m_eCommand;

		if ( pNamedFrame->m_wszName == TNULL )
			pNamedFrame->m_eStdNamedFrame = StdNamedFrame_None;
		else
			pNamedFrame->m_eStdNamedFrame = GetStdNamedFrame( pNamedFrame->m_wszName );

		m_oNamedFrames.PushBack( pNamedFrame );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c4bb0
TBOOL TXUITimelineControl::CreateTimelines( TXUIResource& a_rResource, XURXUITimelineData* a_pTimelines, TUINT32 a_uiNumTimelines )
{
	m_aTimelines = new ( TXUI::ms_pXUIMemoryBlock ) TXUITimeline[ a_uiNumTimelines ];

	for ( TUINT16 i = 0; i < a_uiNumTimelines; i++ )
	{
		TXUITimeline* pTimeline = &m_aTimelines[ i ];

		if ( !pTimeline->Create( a_rResource, &a_pTimelines[ i ], m_pElement ) )
			return TFALSE;

		if ( pTimeline->GetEndTime() > m_fTimelineLength )
			m_fTimelineLength = pTimeline->GetEndTime();

		m_oTimelines.PushBack( pTimeline );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c3c50
void TXUITimelineControl::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_fTimeLineTime == -1.0f )
		return;

	if ( !m_bPlaying )
		return;

	if ( m_fTimelineLength == 0.0f )
		return;

	m_fTimeLineTime += a_fDeltaTime;

	if ( m_fTimelineRangeBegin != -1.0f )
	{
		TASSERT( m_fTimelineRangeEnd != 1.0f );
		TASSERT( m_fTimeLineTime >= m_fTimelineRangeBegin );

		if ( m_fTimeLineTime >= m_fTimelineRangeEnd )
			m_fTimeLineTime = m_fTimelineRangeBegin;
	}
	else if ( !m_oNamedFrames.IsEmpty() && m_fTimeLineTime >= m_fNextNamedFrameTime && m_pNextNamedFrame != TNULL )
	{
		NamedFrame* pCurrentFrame = m_pCurrentNamedFrame;
		NamedFrame* pNextFrame    = m_pNextNamedFrame;

		SendMessage( pNextFrame->m_eStdNamedFrame, pNextFrame->m_wszName );

		// The message can start another animation
		if ( pCurrentFrame == m_pCurrentNamedFrame && pNextFrame == m_pNextNamedFrame )
		{
			m_pCurrentNamedFrame = pNextFrame;

			switch ( pNextFrame->m_eCommand )
			{
				case XURXUINamedFrameData::Command_Play:
					if ( pNextFrame->m_eStdNamedFrame != StdNamedFrame_EndNormal )
						break;
					// fallthrough
				case XURXUINamedFrameData::Command_Stop:
					m_fTimeLineTime = m_fNextNamedFrameTime;
					StopAnimation();
					break;
				case XURXUINamedFrameData::Command_GoTo:
				case XURXUINamedFrameData::Command_GoToAndPlay:
				{
					NamedFrame* pTarget  = FindNamedFrame( pNextFrame->m_wszTarget );
					m_pCurrentNamedFrame = pTarget;
					SendMessage( pTarget->m_eStdNamedFrame, pTarget->m_wszName );
					m_fTimeLineTime = TFLOAT( m_pCurrentNamedFrame->m_uiTime ) * sm_fFrameTime + ( m_fTimeLineTime - m_fNextNamedFrameTime );
					break;
				}
			}

			if ( m_bPlaying == TTRUE )
			{
				m_pNextNamedFrame     = FindNextNamedFrame( m_pCurrentNamedFrame );
				m_fNextNamedFrameTime = TFLOAT( m_pNextNamedFrame->m_uiTime ) * sm_fFrameTime;
			}
		}
	}

	if ( m_pElement->IsVisible() && m_fTimeLineTime != -1.0f )
	{
		for ( auto it = m_oTimelines.Begin(); it != m_oTimelines.End(); it++ )
		{
			it->SetTime( m_fTimeLineTime, m_bLoop );
			// NOTE: retail passes a_fDeltaTime to Update
			it->Update();
		}
	}
}

// $deBlob: FUNCTION 006c4580
void TXUITimelineControl::PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd )
{
	if ( 0.0f <= a_fStart && a_fStart < m_fTimelineLength && a_fStart < a_fEnd && a_fEnd <= m_fTimelineLength )
	{
		m_fTimelineRangeBegin = a_fStart;
		m_fTimelineRangeEnd   = a_fEnd;
		m_fTimeLineTime       = a_fStart;
	}
}

// $deBlob: FUNCTION 006c44f0
void TXUITimelineControl::PlayAnimation( const TWCHAR* a_wszStart, const TWCHAR* a_wszEnd )
{
	NamedFrame* pEnd   = FindNamedFrame( a_wszEnd );
	NamedFrame* pStart = FindNamedFrame( a_wszStart );

	PlayAnimation_Imp( pStart, pEnd, TFALSE );
}

// $deBlob: FUNCTION 006c4c90
void TXUITimelineControl::PlayAnimation_Imp( NamedFrame* a_pStart, NamedFrame* a_pEnd, TBOOL a_bContinue )
{
	if ( !m_bEnabled )
		return;

	if ( m_fTimelineLength == 0.0f )
	{
		StopAnimation();
		return;
	}

	NamedFrame* pNextFrame;

	if ( a_pStart == TNULL )
	{
		if ( a_bContinue && m_pNextNamedFrame != TNULL )
		{
			// Continue from the next named frame
			m_pCurrentNamedFrame = m_pNextNamedFrame;
			pNextFrame           = FindNextNamedFrame( m_pNextNamedFrame );
		}
		else
		{
			// Loop the whole timeline
			m_fTimeLineTime       = 0.0f;
			m_bLoop               = TTRUE;
			m_fStartTime          = -1.0f;
			m_fNextNamedFrameTime = -1.0f;
			m_fEndNamedFrameTime  = -1.0f;
			m_pCurrentNamedFrame  = TNULL;
			m_pNextNamedFrame     = TNULL;
			m_pEndNamedFrame      = TNULL;

			if ( m_oNamedFrames.IsEmpty() )
			{
				m_bPlaying = TTRUE;
				Tick( 0.0f );
				return;
			}

			pNextFrame = m_oNamedFrames.Begin();
		}

		m_pNextNamedFrame     = pNextFrame;
		m_fNextNamedFrameTime = TFLOAT( pNextFrame->m_uiTime ) * sm_fFrameTime;

		m_bPlaying = TTRUE;
		Tick( 0.0f );
		return;
	}

	m_bLoop              = TFALSE;
	m_pCurrentNamedFrame = a_pStart;

	switch ( a_pStart->m_eCommand )
	{
		case XURXUINamedFrameData::Command_Play:
		{
			if ( a_pEnd == TNULL )
			{
				m_fTimeLineTime = TFLOAT( a_pStart->m_uiTime ) * sm_fFrameTime;
			}
			else
			{
				m_pEndNamedFrame = a_pEnd;

				if ( !m_bPlaying ||
				     m_fTimeLineTime < TFLOAT( a_pStart->m_uiTime ) * sm_fFrameTime ||
				     TFLOAT( a_pEnd->m_uiTime ) * sm_fFrameTime < m_fTimeLineTime )
				{
					m_fTimeLineTime = TFLOAT( a_pStart->m_uiTime ) * sm_fFrameTime;
				}
			}

			if ( m_fTimelineLength <= m_fTimeLineTime )
			{
				// The start frame is at the end of the timeline
				NamedFrame* pEndFrame = m_pEndNamedFrame;

				if ( pEndFrame != TNULL )
				{
					TINT          eMessage = 0;
					const TWCHAR* wszParam = TNULL;

					switch ( pEndFrame->m_eStdNamedFrame )
					{
						case StdNamedFrame_EndPress: eMessage = TXUIElement::XUIMessage_EndPress; break;
						case StdNamedFrame_EndTransTo: eMessage = TXUIElement::XUIMessage_EndTransTo; break;
						case StdNamedFrame_EndTransFrom: eMessage = TXUIElement::XUIMessage_EndTransFrom; break;
						case StdNamedFrame_EndTransBackTo: eMessage = TXUIElement::XUIMessage_EndTransBackTo; break;
						case StdNamedFrame_EndTransBackFrom: eMessage = TXUIElement::XUIMessage_EndTransBackFrom; break;
						case StdNamedFrame_None:
							wszParam = pEndFrame->m_wszName;
							eMessage = TXUIElement::XUIMessage_NamedFrame;
							break;
					}

					StopAnimation();

					if ( eMessage != 0 )
						TXUITimelineControl_SendMessageUp( m_pElement, eMessage, wszParam );

					return;
				}

				StopAnimation();
				return;
			}

			m_fStartTime      = m_fTimeLineTime;
			pNextFrame        = FindNextNamedFrame( a_pStart );
			m_pNextNamedFrame = pNextFrame;
			break;
		}
		case XURXUINamedFrameData::Command_Stop:
			m_fTimeLineTime = TFLOAT( a_pStart->m_uiTime ) * sm_fFrameTime;
			StopAnimation();
			return;
		case XURXUINamedFrameData::Command_GoTo:
		case XURXUINamedFrameData::Command_GoToAndPlay:
		{
			NamedFrame* pTarget  = FindNamedFrame( a_pStart->m_wszTarget );
			m_pCurrentNamedFrame = pTarget;

			TFLOAT fTime    = TFLOAT( pTarget->m_uiTime ) * sm_fFrameTime;
			m_fTimeLineTime = fTime;

			if ( m_fTimelineLength < fTime )
			{
				StopAnimation();
				return;
			}

			m_fStartTime      = fTime;
			pNextFrame        = FindNextNamedFrame( pTarget );
			m_pNextNamedFrame = pNextFrame;
			SendMessage( pNextFrame->m_eStdNamedFrame, pNextFrame->m_wszName );
			pNextFrame = m_pNextNamedFrame;
			break;
		}
		default:
			goto SetEndFrame;
	}

	m_fNextNamedFrameTime = TFLOAT( pNextFrame->m_uiTime ) * sm_fFrameTime;

SetEndFrame:
	if ( a_pEnd != TNULL )
	{
		m_pEndNamedFrame     = a_pEnd;
		m_fEndNamedFrameTime = TFLOAT( a_pEnd->m_uiTime ) * sm_fFrameTime;
	}

	m_bPlaying = TTRUE;
	Tick( 0.0f );
}

// $deBlob: FUNCTION 006c40b0
void TXUITimelineControl::PlayAnimation( StdNamedFrame a_eStart, StdNamedFrame a_eEnd, TINT a_iIndex )
{
	if ( a_eStart != StdNamedFrame_None )
	{
		while ( TTRUE )
		{
			NamedFrame* pStart = TNULL;
			NamedFrame* pEnd   = TNULL;

			if ( a_eStart != StdNamedFrame_None && a_eEnd != StdNamedFrame_None )
			{
				// The named frames of a transition can have an index ("TransTo2"...)
				if ( a_iIndex == 0 )
				{
					pStart = FindNamedFrame( mas_StandardNamedFrames[ a_eStart ] );
				}
				else
				{
					TWCHAR* wszName = TStringManager::GetTempString16();
					TStringManager::String16Format( wszName, L"%ls%d", mas_StandardNamedFrames[ a_eStart ], a_iIndex );
					pStart = FindNamedFrame( wszName );
				}

				if ( pStart != TNULL && ( TFLOAT( pStart->m_uiTime ) * sm_fFrameTime <= m_fTimelineLength || a_eStart == StdNamedFrame_Normal ) )
				{
					if ( a_iIndex == 0 )
					{
						pEnd = FindNamedFrame( mas_StandardNamedFrames[ a_eEnd ] );
					}
					else
					{
						TWCHAR* wszName = TStringManager::GetTempString16();
						TStringManager::String16Format( wszName, L"%ls%d", mas_StandardNamedFrames[ a_eEnd ], a_iIndex );
						pEnd = FindNamedFrame( wszName );
					}

					if ( pEnd != TNULL && ( TFLOAT( pEnd->m_uiTime ) * sm_fFrameTime <= m_fTimelineLength || a_eStart == StdNamedFrame_Normal ) )
						goto Play;
				}
				else
				{
					pEnd = TNULL;
				}
			}

			// The named frames don't exist, fall back to the frames of a more generic state
			switch ( a_eStart )
			{
				case StdNamedFrame_Normal:
					PlayAnimation_Imp( TNULL, TNULL, TFALSE );
					return;
				default:
				Play:
					if ( pEnd != TNULL )
					{
						PlayAnimation_Imp( pStart, pEnd, TFALSE );
						return;
					}

					if ( pStart != TNULL )
					{
						PlayAnimation_Imp( pStart, TNULL, TFALSE );
						return;
					}

					PlayAnimation_Imp( TNULL, TNULL, TFALSE );
					return;
				case StdNamedFrame_NormalDisable:
				case StdNamedFrame_Focus:
				case StdNamedFrame_KillFocus:
				case StdNamedFrame_NormalCheck:
				case StdNamedFrame_NormalSel:
				case StdNamedFrame_ScrollMore:
					a_eStart = StdNamedFrame_Normal;
					a_eEnd   = StdNamedFrame_EndNormal;
					a_iIndex = 0;
					break;
				case StdNamedFrame_InitFocusDisable:
					a_eStart = StdNamedFrame_FocusDisable;
					a_eEnd   = StdNamedFrame_EndFocusDisable;
					a_iIndex = 0;
					break;
				case StdNamedFrame_Press:
					SendMessage( StdNamedFrame_EndPress, TNULL );
					// fallthrough
				case StdNamedFrame_FocusDisable:
				case StdNamedFrame_InitFocus:
					a_eStart = StdNamedFrame_Focus;
					a_eEnd   = StdNamedFrame_EndFocus;
					a_iIndex = 0;
					break;
				case StdNamedFrame_NormalPress:
					a_eStart = StdNamedFrame_Press;
					a_eEnd   = StdNamedFrame_EndPress;
					a_iIndex = 0;
					break;
				case StdNamedFrame_PressDisable:
					a_eStart = StdNamedFrame_NormalPress;
					a_eEnd   = StdNamedFrame_EndNormalPress;
					a_iIndex = 0;
					break;
				case StdNamedFrame_FocusCheck:
				case StdNamedFrame_NormalCheckDisable:
					a_eStart = StdNamedFrame_NormalCheck;
					a_eEnd   = StdNamedFrame_EndNormalCheck;
					a_iIndex = 0;
					break;
				case StdNamedFrame_InitFocusCheck:
				case StdNamedFrame_FocusCheckDisable:
					a_eStart = StdNamedFrame_FocusCheck;
					a_eEnd   = StdNamedFrame_EndFocusCheck;
					a_iIndex = 0;
					break;
				case StdNamedFrame_InitFocusCheckDisable:
					a_eStart = StdNamedFrame_FocusCheckDisable;
					a_eEnd   = StdNamedFrame_EndFocusCheckDisable;
					a_iIndex = 0;
					break;
				case StdNamedFrame_NormalSelCheck:
				case StdNamedFrame_NormalSelDisable:
					a_eStart = StdNamedFrame_NormalSel;
					a_eEnd   = StdNamedFrame_EndNormalSel;
					a_iIndex = 0;
					break;
				case StdNamedFrame_NormalSelCheckDisable:
					a_eStart = StdNamedFrame_NormalSelDisable;
					a_eEnd   = StdNamedFrame_EndNormalSelDisable;
					a_iIndex = 0;
					break;
				case StdNamedFrame_Scrolling:
				case StdNamedFrame_ScrollMoreDisable:
					a_eStart = StdNamedFrame_ScrollMore;
					a_eEnd   = StdNamedFrame_EndScrollMore;
					a_iIndex = 0;
					break;
				case StdNamedFrame_TransTo:
					SendMessage( StdNamedFrame_EndTransTo, TNULL );
					return;
				case StdNamedFrame_TransFrom:
					SendMessage( StdNamedFrame_EndTransFrom, TNULL );
					return;
				case StdNamedFrame_TransBackTo:
					// No transition: the scene ends it right away
					if ( m_pElement->IsA( &TGetClass( TXUIScene ) ) )
						TXUITimelineControl_SendMessageUp( m_pElement, 7, TNULL );
					return;
				case StdNamedFrame_TransBackFrom:
					if ( m_pElement->IsA( &TGetClass( TXUIScene ) ) )
						TXUITimelineControl_SendMessageUp( m_pElement, 6, TNULL );
					return;
			}
		}
	}

	PlayAnimation_Imp( TNULL, TNULL, TFALSE );
}

// $deBlob: FUNCTION 006c4760
void TXUITimelineControl::ContinueAnimation()
{
	PlayAnimation_Imp( TNULL, TNULL, TTRUE );
}

// $deBlob: FUNCTION 006c45c0
void TXUITimelineControl::StopAnimation()
{
	SetTimeLineTime( m_fTimeLineTime, TFALSE );
	m_bPlaying            = TFALSE;
	m_fTimelineRangeBegin = -1.0f;
	m_fTimelineRangeEnd   = -1.0f;
}

// $deBlob: FUNCTION 006c4630
void TXUITimelineControl::SetTimeLineTime( TFLOAT a_fTime, TBOOL a_bLoop )
{
	if ( m_fTimeLineTime != -1.0f && a_fTime == -1.0f )
		Reset();

	m_fTimeLineTime = a_fTime;

	if ( a_fTime != -1.0f )
	{
		for ( auto it = m_oTimelines.Begin(); it != m_oTimelines.End(); it++ )
		{
			it->SetTime( a_fTime, a_bLoop );
			// NOTE: retail passes 0 to Update
			it->Update();
		}
	}
}

// $deBlob: FUNCTION 006c45f0
void TXUITimelineControl::ResetAnimation()
{
	m_bPlaying            = TFALSE;
	m_fStartTime          = -1.0f;
	m_fNextNamedFrameTime = -1.0f;
	m_fEndNamedFrameTime  = -1.0f;
	m_pCurrentNamedFrame  = TNULL;
	m_pNextNamedFrame     = TNULL;
	m_pEndNamedFrame      = TNULL;
	m_fTimelineRangeBegin = -1.0f;
	m_fTimelineRangeEnd   = -1.0f;
	Reset();
}

// Lets a_pThief (the scene of a transition visual) get the properties animated by the only timeline
// $deBlob: FUNCTION 006c4770
void TXUITimelineControl::SetPropertyThief( TXUIElement* a_pThief )
{
	if ( m_oTimelines.Size() == 1 )
	{
		m_pPropertyThief = a_pThief;
		m_oTimelines.Begin()->m_pVisualElement = a_pThief;
	}
}

// $deBlob: FUNCTION 006c48a0
TXUITimelineControl::StdNamedFrame TXUITimelineControl::GetStdNamedFrame( const TWCHAR* a_wszName )
{
	for ( TINT i = 0; i < StdNamedFrame_NUMOF; i++ )
	{
		// NOTE: compares the prefix only, "TransTo2" is StdNamedFrame_TransTo; retail quirk: the first shorter name
		// wins too ("NormalDisable" is StdNamedFrame_Normal, "EndNormalPress" StdNamedFrame_EndNormal, "EndPressDisable"
		// StdNamedFrame_EndPress)
		if ( TStringManager::String16Compare( mas_StandardNamedFrames[ i ], a_wszName, TStringManager::String16Length( mas_StandardNamedFrames[ i ] ) ) == 0 )
			return i;
	}

	return StdNamedFrame_None;
}

// $deBlob: FUNCTION 006c3f70
TXUITimelineControl::NamedFrame* TXUITimelineControl::FindNamedFrame( const TWCHAR* a_wszName )
{
	if ( a_wszName == TNULL )
		return TNULL;

	for ( auto it = m_oNamedFrames.Begin(); it != m_oNamedFrames.End(); it++ )
	{
		if ( TStringManager::String16Compare( it->m_wszName, a_wszName, -1 ) == 0 )
			return it;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006c3f20
TXUITimelineControl::NamedFrame* TXUITimelineControl::GetNamedFrame( TFLOAT a_fTime, TFLOAT a_fFrameTime )
{
	for ( auto it = m_oNamedFrames.Begin(); it != m_oNamedFrames.End(); it++ )
	{
		if ( TFLOAT( it->m_uiTime ) * a_fFrameTime == a_fTime )
			return it;
	}

	return TNULL;
}

// Returns the named frame after a_pNamedFrame, wraps to the first one
// $deBlob: FUNCTION 006c3fb0
TXUITimelineControl::NamedFrame* TXUITimelineControl::FindNextNamedFrame( NamedFrame* a_pNamedFrame )
{
	auto it = m_oNamedFrames.Begin();

	for ( ; it != m_oNamedFrames.End(); it++ )
	{
		if ( it.Get() == a_pNamedFrame )
			break;
	}

	if ( it == m_oNamedFrames.End() )
		return TNULL;

	auto itNext = it.Next();

	if ( itNext != m_oNamedFrames.End() )
		return itNext;

	return m_oNamedFrames.Begin();
}

// $deBlob: FUNCTION 006c47d0
TXUITimeline* TXUITimelineControl::FindTimeline( const TWCHAR* a_wszName )
{
	for ( auto it = m_oTimelines.Begin(); it != m_oTimelines.End(); it++ )
	{
		if ( TStringManager::String16Compare( it->m_wszName, a_wszName, -1 ) == 0 )
			return it;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006c4050
TFLOAT TXUITimelineControl::GetNamedFrameTime( const TWCHAR* a_wszName )
{
	// NOTE: retail doesn't check the named frame (TXUIScene::GetTimelineTime does)
	NamedFrame* pNamedFrame = FindNamedFrame( a_wszName );
	return TFLOAT( pNamedFrame->m_uiTime ) * sm_fFrameTime;
}

// $deBlob: FUNCTION 006c4810
void TXUITimelineControl::Reset()
{
	for ( auto it = m_oTimelines.Begin(); it != m_oTimelines.End(); it++ )
		it->Reset();

	m_fTimeLineTime = -1.0f;
}

// Sends the message of a named frame to the element (the end of a press, a scroll or a transition, or a custom
// named frame)
// $deBlob: FUNCTION 006c48f0
void TXUITimelineControl::SendMessage( StdNamedFrame a_eFrame, const TWCHAR* a_wszName )
{
	TINT eMessage;

	switch ( a_eFrame )
	{
		case StdNamedFrame_EndPress:
			eMessage = TXUIElement::XUIMessage_EndPress;
			break;
		case StdNamedFrame_EndScrolling:
			eMessage = TXUIElement::XUIMessage_EndScrolling;
			break;
		case StdNamedFrame_EndTransTo:
			if ( !m_pElement->IsA( &TGetClass( TXUIScene ) ) ) return;
			eMessage = TXUIElement::XUIMessage_EndTransTo;
			break;
		case StdNamedFrame_EndTransFrom:
			if ( !m_pElement->IsA( &TGetClass( TXUIScene ) ) ) return;
			eMessage = TXUIElement::XUIMessage_EndTransFrom;
			break;
		case StdNamedFrame_EndTransBackTo:
			if ( !m_pElement->IsA( &TGetClass( TXUIScene ) ) ) return;
			eMessage = TXUIElement::XUIMessage_EndTransBackTo;
			break;
		case StdNamedFrame_EndTransBackFrom:
			if ( !m_pElement->IsA( &TGetClass( TXUIScene ) ) ) return;
			eMessage = TXUIElement::XUIMessage_EndTransBackFrom;
			break;
		case StdNamedFrame_None:
			eMessage = TXUIElement::XUIMessage_NamedFrame;
			break;
		default:
			return;
	}

	TXUITimelineControl_SendMessageUp( m_pElement, eMessage, a_wszName );
}

TOSHI_NAMESPACE_END
