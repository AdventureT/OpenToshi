#include "ToshiPCH.h"
#include "TXUIScene.h"
#include "TXUI.h"
#include "TXUICanvas.h"
#include "TXUIVisual.h"
#include "TXUIBackButton.h"
#include "TXUITabScene.h"
#include "TXUIList.h"
#include "TXUIRadioGroup.h"
#include "TXUITextPresenter.h"
#include "TXUIImagePresenter.h"
#include "Input/TInputInterface.h"
#include "Input/TInputDeviceMouse.h"
#include "Input/TInputDeviceKeyboard.h"
#include "T2Locale/T2Locale.h"

#include <wchar.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiScene" type info
const TXUITypeInfo XURXUISceneData::sm_sTypeInfo = { L"XuiScene", &XURXUIControlData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUISceneData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006cb7f0
const TXUITypeInfo* XURXUISceneData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// $deBlob: FUNCTION 006cb800
XURXUISceneData::XURXUISceneData()
{
	m_uiInterruptTransitions = 0;
	m_uiDefaultFocus         = 0;
	m_bIgnorePresses         = TFALSE;
	m_uiTransFrom            = 0;
	m_uiTransTo              = 0;
	m_uiTransBackFrom        = 0;
	m_uiTransBackTo          = 0;
	m_pClass                 = TClass::Find( "TXUIScene", &TGetClass( TXUIControl ) );
}

// $deBlob: FUNCTION 006cb840
TBOOL XURXUISceneData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006cb870
TUINT32 XURXUISceneData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006cb8a0
TBOOL XURXUISceneData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006cb910
TBOOL XURXUISceneData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "DefaultFocus", -1 ) == 0 )
	{
		a_rPropIndex = Prop_DefaultFocus;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "TransFrom", -1 ) == 0 )
	{
		a_rPropIndex = Prop_TransFrom;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "TransTo", -1 ) == 0 )
	{
		a_rPropIndex = Prop_TransTo;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "TransBackFrom", -1 ) == 0 )
	{
		a_rPropIndex = Prop_TransBackFrom;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "TransBackTo", -1 ) == 0 )
	{
		a_rPropIndex = Prop_TransBackTo;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "InterruptTransitions", -1 ) == 0 )
	{
		a_rPropIndex = Prop_InterruptTransitions;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "IgnorePresses", -1 ) == 0 )
	{
		a_rPropIndex = Prop_IgnorePresses;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006cba20
TUINT32 XURXUISceneData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_IgnorePresses )
			return 1;

		if ( a_uiPropIndex == Prop_DefaultFocus || a_uiPropIndex == Prop_TransFrom || a_uiPropIndex == Prop_TransTo ||
		     a_uiPropIndex == Prop_TransBackFrom || a_uiPropIndex == Prop_TransBackTo )
			return 2;

		return 4;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006cbac0
TBOOL XURXUISceneData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006cbb20
TBOOL XURXUISceneData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006cbb80
TBOOL XURXUISceneData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT8 uiNumProps = *a_rData;
	a_rData += 1;

	if ( uiNumProps != 0 )
	{
		TUINT8 uiPropMask = 0;

		if ( m_uiPropLevelMask > 0 )
		{
			uiPropMask = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_DefaultFocus ) )
		{
			m_uiDefaultFocus = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_TransFrom ) )
		{
			m_uiTransFrom = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_TransTo ) )
		{
			m_uiTransTo = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_TransBackFrom ) )
		{
			m_uiTransBackFrom = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_TransBackTo ) )
		{
			m_uiTransBackTo = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_InterruptTransitions ) )
		{
			m_uiInterruptTransitions = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_IgnorePresses ) )
		{
			m_bIgnorePresses = *a_rData;
			a_rData += 1;
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// TXUIScene
//-----------------------------------------------------------------------------

TWCHAR                               TXUIScene::ms_aPlatformGlyphs[ NUM_PLATFORM_GLYPHS ][ 1 + Platform_NUMOF ];
const TXUIScene::InputBinding*       TXUIScene::ms_pKeyboardBindings = TNULL;
const TXUIScene::InputBinding*       TXUIScene::ms_pXbox360Bindings  = TNULL;
const TXUIScene::InputBinding*       TXUIScene::ms_pPS4Bindings      = TNULL;

TINT TXUIScene::GetPlatform()
{
	return TInputDevice::GetLastGamepadType();
}

// The platform specific elements of the scenes
static const TWCHAR* s_awszPlatformImages[ TXUIScene::Platform_NUMOF ]  = { L"XuiImage1_wii", L"XuiImage1_360", L"XuiImage1_key", L"XuiImage1_ps4" };
static const TWCHAR* s_awszPlatformFigures[ TXUIScene::Platform_NUMOF ] = { L"XuiFigure_wii", L"XuiFigure_360", L"XuiFigure_key", L"XuiFigure_ps4" };

// The labels that get their text from the locale
struct TXUIScene_LocalisedLabel
{
	const TWCHAR* m_wszName;
	TINT          m_iStringId;
};

static const TXUIScene_LocalisedLabel s_aLocalisedLabels[] = {
	{ L"XuiLabel_Skip", 0x369 },
	{ L"XuiLabel_Interact", 0x368 },
	{ L"XuiLabel_ZLock", 0x36B },
	{ L"XuiLabel_Jump", 0x36D },
};

// The elements hidden on every platform but the Wii
static const TWCHAR* s_awszWiiOnlyElements[] = { L"RegJump", L"RegShake", L"RegDrum", L"RegStatic1", L"RegStatic2", L"RegStatic3" };

// The movement glyphs of the strings
static const TWCHAR s_wszMoveGlyph[]        = L"\x89\x8B\x8B\x8B";
static const TWCHAR s_wszLookGlyph[]        = L"\x88\x8B\x8B\x8B";
static const TWCHAR s_wszWASDKeys[]         = L"\x426\x410\x422\x413";
static const TWCHAR s_wszArrowKeys[]        = L"\x439\x438\x43A\x43B";
static const TWCHAR s_wszWASDMoveGlyph[]    = L"\x406\x8B\x8B\x8B";
static const TWCHAR s_wszArrowLookGlyph[]   = L"\x407\x8B\x8B\x8B";

// Returns the button of a binding of the game's controls, -1 if the game didn't install them (retail's bindings are filled by the game's static initializer)
static TINT TXUIScene_GetBinding( const TXUIScene::InputBinding* a_pBindings, TINT a_iIndex )
{
	return ( a_pBindings != TNULL ) ? a_pBindings[ a_iIndex ].m_iButton : -1;
}

// $deBlob: FUNCTION 006cbc80
// $deBlob: FUNCTION 006cbc90
// $deBlob: FUNCTION 006cbcc0
// $deBlob: FUNCTION 006cbce0
// $deBlob: FUNCTION 006cbcf0
// $deBlob: FUNCTION 0053ef40
// $deBlob: FUNCTION 008590b0
TXUI_DEFINE_CLASS( TXUIScene );

// $deBlob: FUNCTION 006cbd00
TXUIScene::TXUIScene()
    : m_oTimelineControl( this )
{
	m_bIgnorePresses = TFALSE;
	m_uiSceneFlags   = SceneFlags_AcceptsInput | SceneFlags_HideAfterTransFrom;

	m_pDefaultFocus          = TNULL;
	m_pBackButton            = TNULL;
	m_wszClassOverride       = TNULL;
	m_pTransTo               = TNULL;
	m_pTransFrom             = TNULL;
	m_pTransBackTo           = TNULL;
	m_pTransBackFrom         = TNULL;
	m_uiInterruptTransitions = 0;
	m_pPrevScene             = TNULL;
	m_iTransitionIndex       = 0;

	for ( TINT i = 0; i < MAXCONTROLLERS; i++ )
	{
		m_apFocus[ i ]     = TNULL;
		m_apLastFocus[ i ] = TNULL;
	}
}

// $deBlob: FUNCTION 006cbdf0
// $deBlob: FUNCTION 006cf320
// $deBlob: FUNCTION 006cf27f
TXUIScene::~TXUIScene()
{
	if ( m_pTransTo != TNULL )
		delete m_pTransTo;

	if ( m_pTransFrom != TNULL )
		delete m_pTransFrom;

	if ( m_pTransBackTo != TNULL )
		delete m_pTransBackTo;

	if ( m_pTransBackFrom != TNULL )
		delete m_pTransBackFrom;

	if ( this == TXUI::GetSingleton()->GetActiveScene() )
		TXUI::GetSingleton()->SetFocussedScene( TNULL );
}

// Creates a transition visual of the scene: the scene steals the properties its only timeline animates
static TXUIVisual* TXUIScene_CreateTransitionVisual( TXUIScene* a_pScene, TXUIResource& a_rResource, TUINT16 a_uiName )
{
	TXUIVisual* pVisual = TXUI::GetSingleton()->CreateVisual( a_rResource.GetString( a_uiName ), TFALSE );

	if ( pVisual != TNULL )
	{
		pVisual->GetTimelineControl()->SetPropertyThief( a_pScene );
		pVisual->GetTimelineControl()->SetLoop( TFALSE );
	}

	return pVisual;
}

// $deBlob: FUNCTION 006cbe70
TBOOL TXUIScene::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, TTRUE );

	XURXUISceneData* pData = TSTATICCAST( XURXUISceneData, a_pData );

	m_uiInterruptTransitions = pData->m_uiInterruptTransitions;
	m_bIgnorePresses         = pData->m_bIgnorePresses;

	if ( pData->m_uiTransFrom != 0 )
		m_pTransFrom = TXUIScene_CreateTransitionVisual( this, a_rResource, pData->m_uiTransFrom );

	if ( pData->m_uiTransTo != 0 )
		m_pTransTo = TXUIScene_CreateTransitionVisual( this, a_rResource, pData->m_uiTransTo );

	if ( pData->m_uiTransBackFrom != 0 )
		m_pTransBackFrom = TXUIScene_CreateTransitionVisual( this, a_rResource, pData->m_uiTransBackFrom );

	if ( pData->m_uiTransBackTo != 0 )
		m_pTransBackTo = TXUIScene_CreateTransitionVisual( this, a_rResource, pData->m_uiTransBackTo );

	if ( pData->m_uiDefaultFocus == 0 )
	{
		// No default focus: the first tab scene, list, radio group or control
		const TClass* apClasses[] = { &TGetClass( TXUITabScene ), &TGetClass( TXUIList ), &TGetClass( TXUIRadioGroup ), &TGetClass( TXUIControl ) };

		for ( TINT i = 0; i < TARRAYSIZE( apClasses ) && m_pDefaultFocus == TNULL; i++ )
		{
			for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
			{
				T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );
				if ( !pGUIElement->GetFlag( FLAGS_XUIELEMENT ) ) continue;

				TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

				if ( pChild->IsA( apClasses[ i ] ) )
				{
					m_pDefaultFocus = TSTATICCAST( TXUIControl, pChild );
					break;
				}
			}
		}
	}
	else
	{
		TXUIElement* pElement = FindChildXUIElement( a_rResource.GetString( pData->m_uiDefaultFocus ) );

		if ( pElement != TNULL )
		{
			TASSERT( pElement->IsA( &TGetClass( TXUIControl ) ) );
			m_pDefaultFocus = TSTATICCAST( TXUIControl, pElement );
		}
	}

	SetFocus( m_pDefaultFocus, TXUI::GetSingleton()->GetDefaultController() );

	if ( pData->m_uiClassOverride != 0 )
		m_wszClassOverride = a_rResource.GetString( pData->m_uiClassOverride );

	// NOTE: retail doesn't check FLAGS_XUIELEMENT here
	for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
	{
		TXUIElement* pChild = TSTATICCAST( TXUIElement, TSTATICCAST( T2GUIElement, pNode ) );

		if ( pChild->IsA( &TGetClass( TXUIBackButton ) ) && m_pBackButton == TNULL )
			m_pBackButton = TSTATICCAST( TXUIBackButton, pChild );

		pChild->ResolveNavigation( this );
	}

	m_oTimelineControl.PlayAnimation( TXUITimelineControl::StdNamedFrame_None, TXUITimelineControl::StdNamedFrame_None, 0 );

	return TTRUE;
}

// $deBlob: FUNCTION 006cc220
void TXUIScene::OnCreated()
{
	UpdatePlatformElements( this, TFALSE );
}

// Ticks a transition visual and stops it at its end, returns TTRUE if it's done
static TBOOL TXUIScene_TickTransitionVisual( TXUIVisual* a_pVisual, TFLOAT a_fDeltaTime )
{
	if ( a_pVisual == TNULL )
		return TTRUE;

	a_pVisual->Tick( a_fDeltaTime );

	TXUITimelineControl& rTimelineControl = *a_pVisual->GetTimelineControl();

	if ( rTimelineControl.GetTimeLineTime() >= rTimelineControl.GetTimelineLength() && rTimelineControl.IsPlaying() == TTRUE )
		a_pVisual->StopAnimation();

	return rTimelineControl.IsPlaying() != TTRUE;
}

// $deBlob: FUNCTION 006cc230
void TXUIScene::Tick( TFLOAT a_fDeltaTime )
{
	UpdatePlatformElements( this, TTRUE );

	TUINT32 uiState = GetTimelineState();
	m_oTimelineControl.Tick( a_fDeltaTime );

	if ( uiState == State_TransFrom )
	{
		TBOOL bVisualDone = TXUIScene_TickTransitionVisual( m_pTransFrom, a_fDeltaTime );

		if ( ( m_uiSceneFlags & SceneFlags_TransitionEnded ) && bVisualDone )
			EndTransFrom();
	}
	else if ( uiState == State_TransTo )
	{
		TBOOL      bVisualDone   = TXUIScene_TickTransitionVisual( m_pTransTo, a_fDeltaTime );
		TXUIScene* pTransToScene = TXUI::GetSingleton()->GetTransToScene();

		if ( pTransToScene != TNULL && pTransToScene == this && ( m_uiSceneFlags & SceneFlags_TransitionEnded ) && bVisualDone )
			EndTransTo();
	}
	else if ( uiState == State_TransBackFrom )
	{
		TBOOL bVisualDone = TXUIScene_TickTransitionVisual( m_pTransBackFrom, a_fDeltaTime );

		if ( ( m_uiSceneFlags & SceneFlags_TransitionEnded ) && bVisualDone )
			EndTransBackFrom();
	}
	else if ( uiState == State_TransBackTo )
	{
		TBOOL bVisualDone = TXUIScene_TickTransitionVisual( m_pTransBackTo, a_fDeltaTime );

		if ( TXUI::GetSingleton()->GetTransBackFromScene() == TNULL && ( m_uiSceneFlags & SceneFlags_TransitionEnded ) && bVisualDone )
			EndTransBackTo();
	}

	T2GUIElement::Tick( a_fDeltaTime );

	if ( m_uiSceneFlags & SceneFlags_Delete )
	{
		m_uiSceneFlags &= ~SceneFlags_Delete;

		TXUIScene* pActiveScene = TXUI::GetSingleton()->GetActiveScene();

		if ( pActiveScene == this )
		{
			TXUI::GetSingleton()->ClearActiveScene();
		}
		else if ( pActiveScene != TNULL )
		{
			for ( TINT i = 0; i < MAXCONTROLLERS; i++ )
			{
				if ( this == pActiveScene->GetFocus( i ) )
					pActiveScene->SetFocus( TNULL, i );
			}
		}

		if ( m_uiSceneFlags & SceneFlags_Cached )
		{
			// Kept for TXUI::CreateScene
			RemoveSelf();
			return;
		}

		delete this;
	}
}

// $deBlob: FUNCTION 006cc550
void TXUIScene::OnMouseOverChange( TXUIElement* a_pElement, TINT a_iController )
{
	TUINT32 uiState = GetTimelineState();

	if ( uiState == State_TransFrom || uiState == State_TransBackFrom )
		return;

	if ( a_pElement == TNULL || a_pElement == this || a_pElement == GetFocus( a_iController ) )
		return;

	if ( a_pElement->IsA( &TGetClass( TXUIControl ) ) && a_pElement->IsEnabled() )
	{
		TXUIControl* pFocus = GetFocus( a_iController );

		if ( pFocus != TNULL )
		{
			TBOOL bFocusedByOther = TFALSE;

			for ( TINT i = 0; i < MAXCONTROLLERS; i++ )
			{
				if ( i != a_iController && pFocus == GetFocus( i ) )
				{
					bFocusedByOther = TTRUE;
					break;
				}
			}

			if ( !bFocusedByOther )
				pFocus->SetFocus( TFALSE, TFALSE, TTRUE );
		}

		SetFocus( TSTATICCAST( TXUIControl, a_pElement ), a_iController );
		GetFocus( a_iController )->SetFocus( TTRUE, TFALSE, TTRUE );
	}
}

// $deBlob: FUNCTION 006cc630
void TXUIScene::TransTo()
{
	if ( GetTimelineState() == State_TransTo )
		return;

	SetVisible( TTRUE );
	SetFocus( TTRUE, TTRUE, TFALSE );
	SetXUIStateBits( State_TransTo, 0xF, XUIState_TimelineStateShift );

	if ( m_pTransTo != TNULL )
	{
		m_pTransTo->PlayAnimation( -1, -1 );
		m_pTransTo->GetTimelineControl()->SetLoop( TFALSE );
	}

	m_uiSceneFlags &= ~SceneFlags_TransitionEnded;
	m_oTimelineControl.PlayAnimation( TXUITimelineControl::StdNamedFrame_TransTo, TXUITimelineControl::StdNamedFrame_EndTransTo, m_iTransitionIndex );

	for ( TINT i = 0; i < MAXCONTROLLERS; i++ )
	{
		if ( GetFocus( i ) != TNULL )
			GetFocus( i )->SetInputIgnored( TTRUE );
	}
}

// $deBlob: FUNCTION 006cc740
void TXUIScene::TransFrom()
{
	if ( GetTimelineState() == State_TransFrom )
		return;

	SetFocus( TFALSE, TFALSE, TTRUE );
	SetXUIStateBits( State_TransFrom, 0xF, XUIState_TimelineStateShift );

	if ( m_pTransFrom != TNULL )
	{
		m_pTransFrom->PlayAnimation( -1, -1 );
		m_pTransFrom->GetTimelineControl()->SetLoop( TFALSE );
	}

	m_uiSceneFlags &= ~SceneFlags_TransitionEnded;
	m_oTimelineControl.PlayAnimation( TXUITimelineControl::StdNamedFrame_TransFrom, TXUITimelineControl::StdNamedFrame_EndTransFrom, m_iTransitionIndex );

	for ( TINT i = 0; i < MAXCONTROLLERS; i++ )
	{
		if ( GetFocus( i ) != TNULL )
			GetFocus( i )->SetInputIgnored( TTRUE );
	}
}

// $deBlob: FUNCTION 006cc850
void TXUIScene::TransBackTo()
{
	if ( GetTimelineState() == State_TransBackTo )
		return;

	SetVisible( TTRUE );
	SetFocus( TTRUE, TFALSE, TFALSE );
	SetXUIStateBits( State_TransBackTo, 0xF, XUIState_TimelineStateShift );

	for ( TINT i = 0; i < MAXCONTROLLERS; i++ )
	{
		if ( GetFocus( i ) != TNULL )
		{
			GetFocus( i )->SetFocus( TTRUE, TTRUE, TTRUE );
			GetFocus( i )->SetInputIgnored( TTRUE );
		}
	}

	if ( m_pTransBackTo != TNULL )
	{
		m_pTransBackTo->PlayAnimation( -1, -1 );
		m_pTransBackTo->GetTimelineControl()->SetLoop( TFALSE );
	}

	m_uiSceneFlags &= ~SceneFlags_TransitionEnded;
	m_oTimelineControl.PlayAnimation( TXUITimelineControl::StdNamedFrame_TransBackTo, TXUITimelineControl::StdNamedFrame_EndTransBackTo, m_iTransitionIndex );
}

// $deBlob: FUNCTION 006cc9c0
void TXUIScene::TransBackFrom()
{
	if ( GetTimelineState() == State_TransBackFrom )
		return;

	SetFocus( TFALSE, TFALSE, TTRUE );
	SetXUIStateBits( State_TransBackFrom, 0xF, XUIState_TimelineStateShift );

	if ( m_pTransBackFrom != TNULL )
	{
		m_pTransBackFrom->PlayAnimation( -1, -1 );
		m_pTransBackFrom->GetTimelineControl()->SetLoop( TFALSE );
	}

	m_uiSceneFlags &= ~SceneFlags_TransitionEnded;
	m_oTimelineControl.PlayAnimation( TXUITimelineControl::StdNamedFrame_TransBackFrom, TXUITimelineControl::StdNamedFrame_EndTransBackFrom, m_iTransitionIndex );

	for ( TINT i = 0; i < MAXCONTROLLERS; i++ )
	{
		if ( GetFocus( i ) != TNULL )
			GetFocus( i )->SetInputIgnored( TTRUE );
	}
}

// $deBlob: FUNCTION 006ccad0
void TXUIScene::EndTransTo()
{
	TASSERT( TXUI::GetSingleton()->GetTransToScene() == this );
	TXUI::GetSingleton()->EndTransTo();

	TXUIScene* pParentScene   = FindParentScene();
	TXUIScene* pFocussedScene = this;

	if ( pParentScene != TNULL )
	{
		pParentScene->SetFocus( this, TXUI::GetSingleton()->GetDefaultController() );
		pFocussedScene = pParentScene;
	}

	TXUI::GetSingleton()->SetFocussedScene( pFocussedScene );

	for ( TINT i = 0; i < MAXCONTROLLERS; i++ )
	{
		if ( GetFocus( i ) != TNULL )
			GetFocus( i )->SetInputIgnored( TFALSE );
	}

	SetXUIStateBits( State_Focus, 0xF, XUIState_TimelineStateShift );
}

// $deBlob: FUNCTION 006ccc00
void TXUIScene::EndTransFrom()
{
	TXUI::GetSingleton()->EndTransFrom();
	SetXUIStateBits( State_Hidden, 0xF, XUIState_TimelineStateShift );

	for ( TINT i = 0; i < MAXCONTROLLERS; i++ )
	{
		if ( GetFocus( i ) != TNULL )
			GetFocus( i )->SetInputIgnored( TFALSE );
	}

	if ( ( m_uiSceneFlags & SceneFlags_HideAfterTransFrom ) && TXUI::GetSingleton()->IsPopup() == TFALSE )
		SetVisible( TFALSE );
}

// $deBlob: FUNCTION 006ccce0
void TXUIScene::EndTransBackTo()
{
	TXUI::GetSingleton()->EndTransBackTo();
	SetVisible( TTRUE );

	TXUIScene* pParentScene   = FindParentScene();
	TXUIScene* pFocussedScene = this;

	if ( pParentScene != TNULL )
	{
		pParentScene->SetFocus( this, TXUI::GetSingleton()->GetDefaultController() );
		pFocussedScene = pParentScene;
	}

	TXUI::GetSingleton()->SetFocussedScene( pFocussedScene );

	for ( TINT i = 0; i < MAXCONTROLLERS; i++ )
	{
		if ( GetFocus( i ) != TNULL )
			GetFocus( i )->SetInputIgnored( TFALSE );
	}

	SetXUIStateBits( State_Focus, 0xF, XUIState_TimelineStateShift );
}

// $deBlob: FUNCTION 006ccdd0
void TXUIScene::EndTransBackFrom()
{
	TASSERT( TXUI::GetSingleton()->GetTransBackFromScene() == this );
	TXUI::GetSingleton()->EndTransBackFrom();

	for ( TINT i = 0; i < MAXCONTROLLERS; i++ )
	{
		if ( GetFocus( i ) != TNULL )
			GetFocus( i )->SetInputIgnored( TFALSE );
	}

	// Deleted by the next Tick
	m_uiSceneFlags |= SceneFlags_Delete;
	SetXUIStateBits( State_None, 0xF, XUIState_TimelineStateShift );
}

// $deBlob: FUNCTION 006cced0
void TXUIScene::PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame )
{
	m_oTimelineControl.PlayAnimation( a_wszStartFrame, a_wszEndFrame );
	TXUIElement::PlayAnimation( a_wszStartFrame, a_wszEndFrame );
}

// $deBlob: FUNCTION 006ccf00
void TXUIScene::PlayAnimation( TINT a_iStartFrame, TINT a_iEndFrame )
{
	m_oTimelineControl.PlayAnimation( a_iStartFrame, a_iEndFrame, 0 );
	TXUIElement::PlayAnimation( a_iStartFrame, a_iEndFrame );
}

// $deBlob: FUNCTION 006ccf30
void TXUIScene::PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd )
{
	m_oTimelineControl.PlayTimelineRange( a_fStart, a_fEnd );
	TXUIElement::PlayTimelineRange( a_fStart, a_fEnd );
}

// $deBlob: FUNCTION 006ccf80
void TXUIScene::StopAnimation()
{
	m_oTimelineControl.StopAnimation();
	TXUIElement::StopAnimation();
}

// $deBlob: FUNCTION 006ccfa0
void TXUIScene::ResetAnimation()
{
	m_oTimelineControl.ResetAnimation();
}

// $deBlob: FUNCTION 006ccfb0
void TXUIScene::SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation )
{
	TUINT32 uiState = GetTimelineState();

	if ( uiState == State_TransFrom || uiState == State_TransBackFrom )
		return;

	TINT iController = TXUI::GetSingleton()->GetDefaultController();

	if ( GetFocus( iController ) != TNULL )
		GetFocus( iController )->SetFocus( TFALSE, TFALSE, TFALSE );

	if ( a_bFocus == TTRUE || a_bNoAnimation == TTRUE )
	{
		// Moves the mouse cursor (without moving it) so the control under it gets the focus
		TInputInterface* pInputInterface = TInputInterface::GetSingleton();

		pInputInterface->GetInputEventEmitter().Throw( TInputInterface::InputEvent( pInputInterface->GetDeviceByIndex<TInputDeviceMouse>( 0 ), TInputDeviceMouse::AXIS_CURSOR, TInputInterface::EVENT_TYPE_MOVED, 0, 0 ) );
	}

	if ( GetFocus( iController ) == TNULL )
		SetFocus( m_pDefaultFocus, iController );

	if ( GetFocus( iController ) != TNULL )
		GetFocus( iController )->SetFocus( a_bFocus, a_bNoAnimation, TTRUE );

	TXUIControl::SetFocus( a_bFocus, TFALSE, TTRUE );
}

// $deBlob: FUNCTION 006cd0b0
TFLOAT TXUIScene::GetTimelineTime( const TWCHAR* a_wszNamedFrame )
{
	TXUITimelineControl::NamedFrame* pNamedFrame = m_oTimelineControl.FindNamedFrame( a_wszNamedFrame );

	if ( pNamedFrame == TNULL )
		return -1.0f;

	return TFLOAT( pNamedFrame->m_uiTime ) * TXUITimelineControl::sm_fFrameTime;
}

// $deBlob: FUNCTION 006cd0f0
void TXUIScene::SetTimelineTime( TFLOAT a_fTime )
{
	if ( a_fTime != m_oTimelineControl.GetTimeLineTime() )
		m_oTimelineControl.SetTimeLineTime( a_fTime, TFALSE );
}

// $deBlob: FUNCTION 006cd120
void TXUIScene::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	TXUIVisual** ppVisual;

	switch ( a_uiPropIndex )
	{
		case XURXUISceneData::Prop_TransFrom:
			ppVisual = &m_pTransFrom;
			break;
		case XURXUISceneData::Prop_TransTo:
			ppVisual = &m_pTransTo;
			break;
		case XURXUISceneData::Prop_TransBackFrom:
			ppVisual = &m_pTransBackFrom;
			break;
		case XURXUISceneData::Prop_TransBackTo:
			ppVisual = &m_pTransBackTo;
			break;
		case XURXUISceneData::Prop_IgnorePresses:
			if ( a_fScalar == 0.0f )
			{
				m_bIgnorePresses = TBOOL( a_uiValue1 );
				return;
			}

			TASSERT( a_fScalar == 1.0f );
			m_bIgnorePresses = TBOOL( a_uiValue2 );
			return;
		default:
			return;
	}

	// The string properties don't interpolate
	TUINT32 uiValue = a_uiValue1;
	a_fScalar       = TFLOAT( TMath::FloorToInt( a_fScalar ) );

	if ( a_fScalar != 0.0f )
	{
		uiValue = a_uiValue2;
		TASSERT( a_fScalar == 1.0f );
	}

	const TWCHAR* wszVisual = a_pResource->GetString( TUINT16( uiValue ) );

#ifdef DEBLOB_COMMUNITY_PATCH
	if ( *ppVisual != TNULL )
		delete *ppVisual;
#else
	// NOTE: retail bug, deletes the TransFrom visual whatever the property is (the old visual leaks and m_pTransFrom
	// is left dangling)
	if ( m_pTransFrom != TNULL )
		delete m_pTransFrom;
#endif

	*ppVisual = TXUI::GetSingleton()->CreateVisual( wszVisual, TFALSE );
	( *ppVisual )->GetTimelineControl()->SetPropertyThief( this );
	( *ppVisual )->GetTimelineControl()->SetLoop( TFALSE );
}

// NOTE: skips the TXUIControl level
// $deBlob: FUNCTION 006cd500
void TXUIScene::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 && a_uiObjectIndex != 1 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 2, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006cd550
TBOOL TXUIScene::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( !( m_uiSceneFlags & SceneFlags_AnyController ) && a_iController != TXUI::GetSingleton()->GetDefaultController() )
		return TFALSE;

	TUINT32 uiState = GetTimelineState();

	if ( uiState == State_TransTo || uiState == State_TransFrom || uiState == State_TransBackFrom || uiState == State_TransBackTo )
		return TFALSE;

	TBOOL bHandled = TFALSE;

	if ( m_pBackButton != TNULL && a_eButton == XUIButton_Back && m_pBackButton->IsEnabled() )
	{
		if ( m_pBackButton->IsVisible() )
			bHandled = m_pBackButton->HandleButton( XUIButton_Back, a_iController );
	}

	TXUIControl* pFocus = GetFocus( a_iController );

	if ( bHandled )
		return TTRUE;

	if ( pFocus == TNULL )
		return TFALSE;

	TXUIControl* pNav;

	switch ( a_eButton )
	{
		case XUIButton_Up: pNav = pFocus->GetNav( Nav_Up ); break;
		case XUIButton_Down: pNav = pFocus->GetNav( Nav_Down ); break;
		case XUIButton_Left: pNav = pFocus->GetNav( Nav_Left ); break;
		case XUIButton_Right: pNav = pFocus->GetNav( Nav_Right ); break;
		default: return TFALSE;
	}

	if ( pNav == TNULL )
		return TFALSE;

	if ( pNav == pFocus )
		return TTRUE;

	pFocus->SetFocus( TFALSE, TFALSE, TTRUE );
	pNav->SetFocus( TTRUE, TFALSE, TTRUE );
	SetFocus( pNav, a_iController );

	if ( TXUIScene* pActiveScene = TXUI::GetSingleton()->GetActiveScene() )
		pActiveScene->SetFocus( pNav, a_iController );

	return TTRUE;
}

// $deBlob: FUNCTION 006cd6a0
void TXUIScene::CreateTimelineControl( TXUIResource& a_rResource, XURXUIObjectData* a_pData )
{
	m_oTimelineControl.Create( a_rResource, a_pData );
}

// $deBlob: FUNCTION 006cd6b0
void TXUIScene::SetPresenterData( const TWCHAR* a_wszText, const TWCHAR* a_wszImagePath, TUINT32 a_uiTextIndex, TUINT32 a_uiImageIndex, TINT a_iParam )
{
	for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
	{
		T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );
		if ( !pGUIElement->GetFlag( FLAGS_XUIELEMENT ) ) continue;

		TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

		if ( pChild->IsA( &TGetClass( TXUITextPresenter ) ) )
		{
			TXUITextPresenter* pTextPresenter = TSTATICCAST( TXUITextPresenter, pChild );

			if ( a_wszText != TNULL && pTextPresenter->GetDataAssociation() == a_uiTextIndex )
			{
				pTextPresenter->SetText( a_wszText, a_iParam );
				pTextPresenter->SetVisible( TTRUE );
			}
			else
			{
				pTextPresenter->SetVisible( TFALSE );
			}
		}
		else if ( pChild->IsA( &TGetClass( TXUIImagePresenter ) ) )
		{
			TXUIImagePresenter* pImagePresenter = TSTATICCAST( TXUIImagePresenter, pChild );

			if ( a_wszImagePath != TNULL && pImagePresenter->GetDataAssociation() == a_uiImageIndex )
			{
				pImagePresenter->SetImage( a_wszImagePath );
				pImagePresenter->SetVisible( TTRUE );
			}
			else
			{
				pImagePresenter->SetVisible( TFALSE );
			}
		}
		else
		{
			pChild->SetPresenterData( a_wszText, a_wszImagePath, a_uiTextIndex, a_uiImageIndex, a_iParam );
		}
	}
}

// $deBlob: FUNCTION 006cd7a0
void TXUIScene::SetPresenterText( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam )
{
	if ( m_pVisual != TNULL )
		m_pVisual->SetPresenterText( a_wszText, a_uiIndex, a_iParam );
}

// $deBlob: FUNCTION 006cd7c0
void TXUIScene::SetPresenterText2( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam )
{
	if ( m_pVisual != TNULL )
		m_pVisual->SetPresenterText2( a_wszText, a_uiIndex, a_iParam );
}

// $deBlob: FUNCTION 006cd7e0
void TXUIScene::SetPresenterImagePath( const TWCHAR* a_wszImagePath, TUINT32 a_uiIndex )
{
	for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
	{
		T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );
		if ( !pGUIElement->GetFlag( FLAGS_XUIELEMENT ) ) continue;

		TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

		if ( pChild->IsA( &TGetClass( TXUIImagePresenter ) ) )
		{
			TXUIImagePresenter* pImagePresenter = TSTATICCAST( TXUIImagePresenter, pChild );

			if ( a_wszImagePath != TNULL && pImagePresenter->GetDataAssociation() == a_uiIndex )
			{
				pImagePresenter->SetImage( a_wszImagePath );
				pImagePresenter->SetVisible( TTRUE );
			}
			else
			{
				pImagePresenter->SetVisible( TFALSE );
			}
		}
		else
		{
			pChild->SetPresenterImagePath( a_wszImagePath, a_uiIndex );
		}
	}
}

// Keeps the scene in TXUI's scene cache (TXUI::CreateScene with a_bCache)
// $deBlob: FUNCTION 006cd870
void TXUIScene::SetCached()
{
	if ( !( m_uiSceneFlags & SceneFlags_Cached ) )
	{
		TXUI::GetSingleton()->AddCachedScene( this );
		m_uiSceneFlags |= SceneFlags_Cached;
	}
}

// $deBlob: FUNCTION 006cd8c0
TXUIElement* TXUIScene::RayTraceDeepest( const TVector2& a_rPoint, const TVector2& a_rScale, TINT a_iController )
{
	if ( !( m_uiSceneFlags & SceneFlags_AcceptsInput ) )
		return TNULL;

	if ( a_iController != TXUI::GetSingleton()->GetDefaultController() && !( m_uiSceneFlags & SceneFlags_AnyController ) )
		return TNULL;

	return TXUIElement::RayTraceDeepest( a_rPoint, a_rScale, a_iController );
}

// Replaces a transition visual, used by the game
// $deBlob: FUNCTION 006cd910
void TXUIScene::SetTransitionVisual( Transition a_eTransition, const TWCHAR* a_wszName )
{
	TXUIVisual* pCurrentVisual;

	switch ( a_eTransition )
	{
		case Transition_From: pCurrentVisual = m_pTransFrom; break;
		case Transition_To: pCurrentVisual = m_pTransTo; break;
		case Transition_BackFrom: pCurrentVisual = m_pTransBackFrom; break;
		case Transition_BackTo: pCurrentVisual = m_pTransBackTo; break;
		default: goto CreateVisual;
	}

	if ( pCurrentVisual != TNULL && TStringManager::String16Compare( pCurrentVisual->GetID(), a_wszName, -1 ) == 0 )
		return;

CreateVisual:
	TXUIVisual* pVisual = TXUI::GetSingleton()->CreateVisual( a_wszName, TFALSE );

	if ( pVisual == TNULL )
		return;

	pVisual->GetTimelineControl()->SetPropertyThief( this );
	pVisual->GetTimelineControl()->SetLoop( TFALSE );

	// NOTE: an invalid a_eTransition leaks the visual
	switch ( a_eTransition )
	{
		case Transition_From:
			if ( m_pTransFrom != TNULL ) delete m_pTransFrom;
			m_pTransFrom = pVisual;
			return;
		case Transition_To:
			if ( m_pTransTo != TNULL ) delete m_pTransTo;
			m_pTransTo = pVisual;
			return;
		case Transition_BackFrom:
			if ( m_pTransBackFrom != TNULL ) delete m_pTransBackFrom;
			m_pTransBackFrom = pVisual;
			return;
		case Transition_BackTo:
			if ( m_pTransBackTo != TNULL ) delete m_pTransBackTo;
			m_pTransBackTo = pVisual;
			return;
	}
}

//-----------------------------------------------------------------------------
// Platform glyphs (de Blob PC port)
//-----------------------------------------------------------------------------

// Shows the elements of the current platform. Called by OnCreated (a_bTick TFALSE) and every Tick.
// $deBlob: FUNCTION 006cda30
void TXUIScene::UpdatePlatformElements( TXUIElement* a_pRoot, TBOOL a_bTick )
{
	// The platform images/figures: shows the ones of the current platform
	// $deBlob: FUNCTION 006ce950
	// $deBlob: FUNCTION 006ce940
	ForEachElement( a_pRoot, TTRUE, [ &a_bTick ]( TXUIElement* a_pElement ) -> TBOOL {
		TINT iPlatform = GetPlatform();

		for ( TINT i = 0; i < Platform_NUMOF; i++ )
		{
			if ( a_pElement->GetID() != TNULL &&
			     ( TStringManager::String16Compare( a_pElement->GetID(), s_awszPlatformImages[ i ], -1 ) == 0 ||
			       TStringManager::String16Compare( a_pElement->GetID(), s_awszPlatformFigures[ i ], -1 ) == 0 ) )
			{
				if ( a_bTick )
					a_pElement->SetVisible( i == iPlatform );
				else
					a_pElement->SetVisible( a_pElement->IsVisible() && i == iPlatform );
			}
		}

		return TTRUE;
	} );

	// The labels of the button prompts get their text from the locale
	// $deBlob: FUNCTION 006ce9e0
	ForEachElement( a_pRoot, TTRUE, []( TXUIElement* a_pElement ) -> TBOOL {
		for ( TUINT32 i = 0; i < TARRAYSIZE( s_aLocalisedLabels ); i++ )
		{
			if ( a_pElement->GetID() != TNULL && TStringManager::String16Compare( a_pElement->GetID(), s_aLocalisedLabels[ i ].m_wszName, -1 ) == 0 )
			{
				TSTATICCAST( TXUIControl, a_pElement )->SetText( T2Locale::GetSingleton()->GetString( s_aLocalisedLabels[ i ].m_iStringId ), -1 );
				return TFALSE;
			}
		}

		return TTRUE;
	} );

	// The skip prompts of the speech bubbles are hidden while the letter box shows its own
	TXUICanvas*  pCanvas    = TXUI::GetSingleton()->GetCanvas();
	TXUIElement* pLetterBox = FindElement( a_pRoot, L"LetterBox", TTRUE );
	TXUIElement* pSkip      = TNULL;
	TBOOL        bShowSkip  = TTRUE;

	if ( pLetterBox != TNULL )
	{
		pSkip = FindElement( pLetterBox, L"Skip", TTRUE );

		if ( pSkip != TNULL && pSkip->IsVisible() && pSkip->IsEnabled() )
			bShowSkip = TFALSE;
	}

	if ( pSkip != TNULL )
	{
		// $deBlob: FUNCTION 006cea60
		// $deBlob: FUNCTION 006cea50
		ForEachElement( pCanvas, TTRUE, [ &bShowSkip ]( TXUIElement* a_pElement ) -> TBOOL {
			if ( a_pElement->GetID() == TNULL )
				return TTRUE;

			if ( TStringManager::String16Compare( a_pElement->GetID(), L"SpeechBubble", -1 ) != 0 &&
			     TStringManager::String16Compare( a_pElement->GetID(), L"SpeechHud", -1 ) != 0 )
				return TTRUE;

			if ( TXUIElement* pLabel = FindElement( a_pElement, L"XuiLabel_Skip", TTRUE ) )
				pLabel->SetVisible( bShowSkip );

			if ( TXUIElement* pImage = FindElement( a_pElement, L"XuiImagePresenter1", TTRUE ) )
				pImage->SetVisible( bShowSkip );

			return TFALSE;
		} );
	}

	if ( GetPlatform() != Platform_Wii )
	{
		for ( TINT i = 0; i < TARRAYSIZE( s_awszWiiOnlyElements ); i++ )
		{
			TXUIElement* pElement = FindElement( a_pRoot, s_awszWiiOnlyElements[ i ], TTRUE );

			if ( pElement != TNULL )
				pElement->SetVisible( TFALSE );
		}
	}
}

// Calls a_rfnCallback for a_pElement, and if it returns TTRUE, for its XUI children (and their visuals if a_bVisuals)
// $deBlob: FUNCTION 006cdb90
TBOOL TXUIScene::ForEachElement( TXUIElement* a_pElement, TBOOL a_bVisuals, const std::function<TBOOL( TXUIElement* )>& a_rfnCallback )
{
	if ( a_pElement == TNULL )
		return TFALSE;

	if ( !a_rfnCallback( a_pElement ) )
		return TFALSE;

	for ( T2GUIElement* pGUIElement = a_pElement->ChildBegin(); pGUIElement != TNULL; pGUIElement = a_pElement->GetNextChild( pGUIElement ) )
	{
		if ( !pGUIElement->GetFlag( FLAGS_XUIELEMENT ) ) continue;

		TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

		if ( a_bVisuals && pChild->IsA( &TGetClass( TXUIControl ) ) )
			ForEachElement( TSTATICCAST( TXUIControl, pChild )->GetVisual(), a_bVisuals, a_rfnCallback );

		ForEachElement( pChild, a_bVisuals, a_rfnCallback );
	}

	return TTRUE;
}

// Copies a_wszString to a_wszOutString replacing the first a_wszFind by a_wszReplace, TNULL if not found
// NOTE: not referenced by retail
// $deBlob: FUNCTION 006cdc40
TWCHAR* TXUIScene::ReplaceString( const TWCHAR* a_wszString, const TWCHAR* a_wszFind, const TWCHAR* a_wszReplace, TWCHAR* a_wszOutString )
{
	const TWCHAR* pFound = wcsstr( a_wszString, a_wszFind );

	if ( pFound == TNULL )
		return TNULL;

	TINT iFindLength    = TINT( wcslen( a_wszFind ) );
	TINT iReplaceLength = TINT( wcslen( a_wszReplace ) );
	TINT iPrefixLength  = TINT( pFound - a_wszString );

	TWCHAR* pOut = a_wszOutString;

	for ( TINT i = 0; i < iPrefixLength; i++ )
		*pOut++ = *a_wszString++;

	for ( TINT i = 0; i < iReplaceLength; i++ )
		*pOut++ = a_wszReplace[ i ];

	for ( const TWCHAR* pRest = a_wszString + iFindLength; *pRest != L'\0'; pRest++ )
		*pOut++ = *pRest;

	*pOut = L'\0';
	return a_wszOutString;
}

// The glyphs of the font that show a button of a platform
// $deBlob: FUNCTION 006cdd10
TBOOL TXUIScene::IsPlatformGlyph( TWCHAR a_wcChar )
{
	return TUINT16( a_wcChar - 0x80 ) <= 0xA || TUINT16( a_wcChar - 1000 ) <= 0x75;
}

// The glyphs of the strings that stand for an input (remapped to the button of the current platform)
// $deBlob: FUNCTION 006cdd40
TBOOL TXUIScene::IsInputGlyph( TWCHAR a_wcChar )
{
	return TUINT16( a_wcChar - 0x8C ) < 6 || a_wcChar == 0x81 || a_wcChar == 0x84 || a_wcChar == 0x88;
}

// $deBlob: FUNCTION 006cdd80
TWCHAR TXUIScene::GetKeyboardGlyph( TINT a_iKey )
{
	switch ( a_iKey )
	{
		case TInputDeviceKeyboard::KEY_ESCAPE: return 0x408;
		case TInputDeviceKeyboard::KEY_TAB: return 0x433;
		case TInputDeviceKeyboard::KEY_Q: return 0x420;
		case TInputDeviceKeyboard::KEY_W: return 0x426;
		case TInputDeviceKeyboard::KEY_E: return 0x414;
		case TInputDeviceKeyboard::KEY_R: return 0x421;
		case TInputDeviceKeyboard::KEY_T: return 0x423;
		case TInputDeviceKeyboard::KEY_Y: return 0x428;
		case TInputDeviceKeyboard::KEY_U: return 0x424;
		case TInputDeviceKeyboard::KEY_I: return 0x418;
		case TInputDeviceKeyboard::KEY_O: return 0x41E;
		case TInputDeviceKeyboard::KEY_P: return 0x41F;
		case TInputDeviceKeyboard::KEY_LCONTROL: return 0x42E;
		case TInputDeviceKeyboard::KEY_A: return 0x410;
		case TInputDeviceKeyboard::KEY_S: return 0x422;
		case TInputDeviceKeyboard::KEY_D: return 0x413;
		case TInputDeviceKeyboard::KEY_F: return 0x415;
		case TInputDeviceKeyboard::KEY_G: return 0x416;
		case TInputDeviceKeyboard::KEY_H: return 0x417;
		case TInputDeviceKeyboard::KEY_J: return 0x419;
		case TInputDeviceKeyboard::KEY_K: return 0x41A;
		case TInputDeviceKeyboard::KEY_L: return 0x41B;
		case TInputDeviceKeyboard::KEY_LSHIFT: return 0x431;
		case TInputDeviceKeyboard::KEY_Z: return 0x429;
		case TInputDeviceKeyboard::KEY_X: return 0x427;
		case TInputDeviceKeyboard::KEY_C: return 0x412;
		case TInputDeviceKeyboard::KEY_V: return 0x425;
		case TInputDeviceKeyboard::KEY_B: return 0x411;
		case TInputDeviceKeyboard::KEY_N: return 0x41D;
		case TInputDeviceKeyboard::KEY_M: return 0x41C;
		case TInputDeviceKeyboard::KEY_RSHIFT: return 0x432;
		case TInputDeviceKeyboard::KEY_MULTIPLY: return 0x43C;
		case TInputDeviceKeyboard::KEY_LMENU:
		case TInputDeviceKeyboard::KEY_RMENU: return 0x430;
		case TInputDeviceKeyboard::KEY_SPACE: return 0x40A;
		case TInputDeviceKeyboard::KEY_CAPITAL: return 0x40C;
		case TInputDeviceKeyboard::KEY_SUBTRACT: return 0x43F;
		case TInputDeviceKeyboard::KEY_ADD: return 0x43E;
		case TInputDeviceKeyboard::KEY_RCONTROL: return 0x42F;
		case TInputDeviceKeyboard::KEY_UP: return 0x439;
		case TInputDeviceKeyboard::KEY_LEFT: return 0x438;
		case TInputDeviceKeyboard::KEY_RIGHT: return 0x43B;
		case TInputDeviceKeyboard::KEY_DOWN: return 0x43A;
		default: return 0x42A;
	}
}

// $deBlob: FUNCTION 006ce050
TWCHAR TXUIScene::GetXbox360Glyph( TINT a_iButton )
{
	switch ( a_iButton )
	{
		case 0: return 0x3FA;
		case 1: return 0x3F8;
		case 2: return 0x3FB;
		case 3: return 0x3F9;
		case 4: return 0x3F4;
		case 5: return 0x3F5;
		case 6: return 0x3FC;
		case 7: return 0x3FF;
		case 8: return 0x3F2;
		case 9: return 0x3F3;
		case 10: return 0x3FD;
		case 11: return 0x3FE;
		default: return 0x42A;
	}
}

// $deBlob: FUNCTION 006ce110
TWCHAR TXUIScene::GetPS4Glyph( TINT a_iButton )
{
	switch ( a_iButton )
	{
		case 0: return 0x45B;
		case 1: return 0x44D;
		case 2: return 0x44C;
		case 3: return 0x45D;
		case 4: return 0x453;
		case 5: return 0x457;
		case 6: return 0x454;
		case 7: return 0x458;
		case 8: return 0x45A;
		case 9: return 0x456;
		case 10: return 0x455;
		case 11: return 0x459;
		default: return 0x42A;
	}
}

// Builds ms_aPlatformGlyphs from the game's input bindings. Called at startup and by the game when the bindings change.
// $deBlob: FUNCTION 006ce1d0
void TXUIScene::InitPlatformGlyphs()
{
	enum
	{
		INPUT,
		WII,
		XBOX360,
		KEYBOARD,
		PS4,
	};

	TWCHAR( &aGlyphs )[ NUM_PLATFORM_GLYPHS ][ 1 + Platform_NUMOF ] = ms_aPlatformGlyphs;

	// The input glyphs and the Wii buttons are fixed
	aGlyphs[ 1 ][ INPUT ]  = 0x8C;
	aGlyphs[ 1 ][ WII ]    = 0x8C;
	aGlyphs[ 14 ][ INPUT ] = 0x90;
	aGlyphs[ 15 ][ INPUT ] = 0x90;
	aGlyphs[ 0 ][ INPUT ]  = 0x8D;
	aGlyphs[ 0 ][ WII ]    = 0x80;
	aGlyphs[ 2 ][ INPUT ]  = 0x8F;
	aGlyphs[ 2 ][ WII ]    = 0x80;
	aGlyphs[ 5 ][ INPUT ]  = 0x81;
	aGlyphs[ 5 ][ WII ]    = 0x81;
	aGlyphs[ 10 ][ INPUT ] = 0x91;
	aGlyphs[ 10 ][ WII ]   = 0x81;
	aGlyphs[ 12 ][ INPUT ] = 0x84;
	aGlyphs[ 12 ][ WII ]   = 0x84;
	aGlyphs[ 16 ][ INPUT ] = 0x8E;
	aGlyphs[ 16 ][ WII ]   = 0x87;

	for ( TINT i = 6; i <= 9; i++ )
	{
		aGlyphs[ i ][ INPUT ] = 0x88;
		aGlyphs[ i ][ WII ]   = 0x88;
	}

	aGlyphs[ 11 ][ WII ] = 0x8A;

	// Xbox 360
	aGlyphs[ 1 ][ XBOX360 ]  = GetXbox360Glyph( TXUIScene_GetBinding( ms_pXbox360Bindings, 1 ) );
	aGlyphs[ 0 ][ XBOX360 ]  = GetXbox360Glyph( TXUIScene_GetBinding( ms_pXbox360Bindings, 0 ) );
	aGlyphs[ 2 ][ XBOX360 ]  = GetXbox360Glyph( TXUIScene_GetBinding( ms_pXbox360Bindings, 2 ) );
	aGlyphs[ 5 ][ XBOX360 ]  = GetXbox360Glyph( TXUIScene_GetBinding( ms_pXbox360Bindings, 5 ) );
	aGlyphs[ 10 ][ XBOX360 ] = GetXbox360Glyph( TXUIScene_GetBinding( ms_pXbox360Bindings, 10 ) );
	aGlyphs[ 11 ][ XBOX360 ] = GetXbox360Glyph( TXUIScene_GetBinding( ms_pXbox360Bindings, 11 ) );
	aGlyphs[ 12 ][ XBOX360 ] = GetXbox360Glyph( TXUIScene_GetBinding( ms_pXbox360Bindings, 12 ) );
	aGlyphs[ 16 ][ XBOX360 ] = GetXbox360Glyph( TXUIScene_GetBinding( ms_pXbox360Bindings, 16 ) );

	for ( TINT i = 6; i <= 9; i++ )
		aGlyphs[ i ][ XBOX360 ] = 0x3F3;

	aGlyphs[ 14 ][ XBOX360 ] = GetXbox360Glyph( TXUIScene_GetBinding( ms_pXbox360Bindings, 14 ) );
	aGlyphs[ 15 ][ XBOX360 ] = GetXbox360Glyph( TXUIScene_GetBinding( ms_pXbox360Bindings, 15 ) );

	for ( TINT i = 17; i <= 20; i++ )
		aGlyphs[ i ][ XBOX360 ] = 0x3F2;

	// Keyboard
	for ( TINT i = 0; i < NUM_PLATFORM_GLYPHS; i++ )
		aGlyphs[ i ][ KEYBOARD ] = GetKeyboardGlyph( TXUIScene_GetBinding( ms_pKeyboardBindings, i ) );

	// PS4
	aGlyphs[ 1 ][ PS4 ]  = GetPS4Glyph( TXUIScene_GetBinding( ms_pPS4Bindings, 1 ) );
	aGlyphs[ 0 ][ PS4 ]  = GetPS4Glyph( TXUIScene_GetBinding( ms_pPS4Bindings, 0 ) );
	aGlyphs[ 2 ][ PS4 ]  = GetPS4Glyph( TXUIScene_GetBinding( ms_pPS4Bindings, 2 ) );
	aGlyphs[ 5 ][ PS4 ]  = GetPS4Glyph( TXUIScene_GetBinding( ms_pPS4Bindings, 5 ) );
	aGlyphs[ 10 ][ PS4 ] = GetPS4Glyph( TXUIScene_GetBinding( ms_pPS4Bindings, 10 ) );
	aGlyphs[ 11 ][ PS4 ] = GetPS4Glyph( TXUIScene_GetBinding( ms_pPS4Bindings, 11 ) );
	aGlyphs[ 12 ][ PS4 ] = GetPS4Glyph( TXUIScene_GetBinding( ms_pPS4Bindings, 12 ) );
	aGlyphs[ 16 ][ PS4 ] = GetPS4Glyph( TXUIScene_GetBinding( ms_pPS4Bindings, 16 ) );

	for ( TINT i = 6; i <= 9; i++ )
		aGlyphs[ i ][ PS4 ] = 0x459;

	aGlyphs[ 14 ][ PS4 ] = GetPS4Glyph( TXUIScene_GetBinding( ms_pPS4Bindings, 14 ) );
	aGlyphs[ 15 ][ PS4 ] = GetPS4Glyph( TXUIScene_GetBinding( ms_pPS4Bindings, 15 ) );

	for ( TINT i = 17; i <= 20; i++ )
		aGlyphs[ i ][ PS4 ] = 0x455;
}

// Builds the platform glyphs at startup (the dynamic initializer calls InitPlatformGlyphs directly)
struct TXUIScene_PlatformGlyphsInit
{
	// $deBlob: FUNCTION 006cf1e0
	TXUIScene_PlatformGlyphsInit() { TXUIScene::InitPlatformGlyphs(); }
};

// $deBlob: FUNCTION 0053ef90
static TXUIScene_PlatformGlyphsInit s_oPlatformGlyphsInit;

// Replaces the movement (and look) glyphs of a_wszString by the keys of the current platform
// $deBlob: FUNCTION 006ce450
void TXUIScene::FixMovementGlyphs( TWCHAR* a_wszString )
{
	if ( GetPlatform() == Platform_Wii )
		return;

	TWCHAR wszMoveKeys[ 5 ] = {
		GetKeyboardGlyph( TXUIScene_GetBinding( ms_pKeyboardBindings, 17 ) ),
		GetKeyboardGlyph( TXUIScene_GetBinding( ms_pKeyboardBindings, 20 ) ),
		GetKeyboardGlyph( TXUIScene_GetBinding( ms_pKeyboardBindings, 18 ) ),
		GetKeyboardGlyph( TXUIScene_GetBinding( ms_pKeyboardBindings, 19 ) ),
		L'\0'
	};

	TWCHAR wszLookKeys[ 5 ] = {
		GetKeyboardGlyph( TXUIScene_GetBinding( ms_pKeyboardBindings, 6 ) ),
		GetKeyboardGlyph( TXUIScene_GetBinding( ms_pKeyboardBindings, 7 ) ),
		0x8B,
		0x8B,
		L'\0'
	};

	if ( GetPlatform() != Platform_Keyboard )
	{
		// The pads have a single glyph for the sticks
		TWCHAR* pMove = wcsstr( a_wszString, s_wszMoveGlyph );

		if ( pMove != TNULL )
			*pMove = ms_aPlatformGlyphs[ 20 ][ 1 + GetPlatform() ];

		TWCHAR* pLook = wcsstr( a_wszString, s_wszLookGlyph );

		if ( pLook != TNULL )
			*pLook = ms_aPlatformGlyphs[ 6 ][ 1 + GetPlatform() ];

		return;
	}

	TWCHAR* pMove = wcsstr( a_wszString, s_wszMoveGlyph );

	if ( pMove != TNULL )
	{
		const TWCHAR* wszKeys;

		if ( wcscmp( wszMoveKeys, s_wszWASDKeys ) == 0 )
			wszKeys = s_wszWASDMoveGlyph;
		else
			wszKeys = ( wcscmp( wszMoveKeys, s_wszArrowKeys ) != 0 ) ? wszMoveKeys : s_wszArrowKeys;

		wcsncpy( pMove, wszKeys, 4 );
	}

	TWCHAR* pLook = wcsstr( a_wszString, s_wszLookGlyph );

	if ( pLook != TNULL )
	{
		const TWCHAR* wszKeys;

		if ( wcscmp( wszLookKeys, s_wszWASDKeys ) == 0 )
			wszKeys = s_wszWASDMoveGlyph;
		else
			wszKeys = ( wcscmp( wszLookKeys, s_wszArrowKeys ) != 0 ) ? wszLookKeys : s_wszArrowLookGlyph;

		wcsncpy( pLook, wszKeys, 4 );
	}
}

// Remaps the input and platform glyphs of a_wszString (in place) to the buttons of the current platform, returns the
// end of the string
// $deBlob: FUNCTION 006ce6d0
TWCHAR* TXUIScene::RemapPlatformGlyphs( TWCHAR* a_wszString )
{
	if ( a_wszString == TNULL )
		return TNULL;

	FixMovementGlyphs( a_wszString );

	TINT iPlatform = GetPlatform();

	for ( TWCHAR* pChar = a_wszString; *pChar != L'\0'; pChar++ )
	{
		TWCHAR wcChar = *pChar;

		if ( TUINT16( wcChar - 0x8C ) < 6 || wcChar == 0x81 || wcChar == 0x84 || wcChar == 0x88 )
		{
			// An input glyph
			for ( TINT i = 0; i < NUM_PLATFORM_GLYPHS; i++ )
			{
				if ( wcChar == ms_aPlatformGlyphs[ i ][ 0 ] )
				{
					*pChar = ms_aPlatformGlyphs[ i ][ 1 + iPlatform ];
					break;
				}
			}
		}
		else if ( TUINT16( wcChar - 0x80 ) < 0xB || TUINT16( wcChar - 1000 ) < 0x76 )
		{
			// The glyph of another platform
			for ( TINT iOtherPlatform = 0; iOtherPlatform < Platform_NUMOF; iOtherPlatform++ )
			{
				if ( iOtherPlatform == iPlatform )
					continue;

				for ( TINT i = 0; i < NUM_PLATFORM_GLYPHS; i++ )
				{
					if ( *pChar == ms_aPlatformGlyphs[ i ][ 1 + iOtherPlatform ] )
					{
						*pChar = ms_aPlatformGlyphs[ i ][ 1 + iPlatform ];
						break;
					}
				}
			}
		}
	}

	return a_wszString + wcslen( a_wszString );
}

// Remaps the glyphs of the text presenters under a_pParent
// NOTE: not referenced by retail
// $deBlob: FUNCTION 006ce7f0
void TXUIScene::RemapPresenterGlyphs( TXUIElement* a_pParent, TINT a_iUnused )
{
	T2GUIElement* pGUIElement = a_pParent->ChildBegin();

	while ( pGUIElement != TNULL )
	{
		if ( !pGUIElement->GetFlag( FLAGS_XUIELEMENT ) )
		{
			pGUIElement = a_pParent->GetNextChild( pGUIElement );
			continue;
		}

		TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

		if ( pChild->IsA( &TGetClass( TXUITextPresenter ) ) )
			RemapPlatformGlyphs( const_cast<TWCHAR*>( TSTATICCAST( TXUITextPresenter, pChild )->GetText() ) );

		T2GUIElement* pNext = a_pParent->GetNextChild( pGUIElement );
		RemapPresenterGlyphs( pChild, a_iUnused );
		pGUIElement = pNext;
	}
}

// Finds the element with the Id a_wszName under a_pRoot (and the visuals of its scenes and back buttons)
// $deBlob: FUNCTION 006ce860
TXUIElement* TXUIScene::FindElement( TXUIElement* a_pRoot, const TWCHAR* a_wszName, TBOOL a_bCheckRoot )
{
	if ( a_bCheckRoot && a_pRoot->GetID() != TNULL && TStringManager::String16Compare( a_pRoot->GetID(), a_wszName, -1 ) == 0 )
		return a_pRoot;

	for ( T2GUIElement* pGUIElement = a_pRoot->ChildBegin(); pGUIElement != TNULL; pGUIElement = a_pRoot->GetNextChild( pGUIElement ) )
	{
		if ( !pGUIElement->GetFlag( FLAGS_XUIELEMENT ) ) continue;

		TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

		if ( pChild->IsA( &TGetClass( TXUIScene ) ) || pChild->IsA( &TGetClass( TXUIBackButton ) ) )
		{
			TXUIVisual* pVisual = TSTATICCAST( TXUIControl, pChild )->GetVisual();

			if ( pVisual != TNULL && pVisual->GetID() != TNULL && TStringManager::String16Compare( pVisual->GetID(), a_wszName, -1 ) == 0 )
				return pVisual;

			if ( a_bCheckRoot && pVisual != TNULL )
			{
				TXUIElement* pResult = FindElement( pVisual, a_wszName, TTRUE );

				if ( pResult != TNULL )
					return pResult;
			}
		}

		TXUIElement* pResult = FindElement( pChild, a_wszName, TTRUE );

		if ( pResult != TNULL )
			return pResult;
	}

	return TNULL;
}

TOSHI_NAMESPACE_END
