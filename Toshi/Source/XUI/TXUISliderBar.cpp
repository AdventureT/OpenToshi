#include "ToshiPCH.h"
#include "TXUISliderBar.h"
#include "TXUI.h"
#include "TXUIGroup.h"
#include "TXUIInput.h"
#include "TXUIVisual.h"
#include "TXUITimelineControl.h"

#include "Input/TInputDeviceKeyboard.h"
#include "Input/TInputDeviceController.h"
#include "Platform/Windows/Input/TInputDeviceMouse_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiSlider" type info
const TXUITypeInfo XURXUISliderBarData::sm_sTypeInfo = { L"XuiSlider", &XURXUIControlData::sm_sTypeInfo };

// Frames are 1/60th of a second
static TCONSTEXPR_VAR TFLOAT TXUISLIDERBAR_FRAMETIME = 1.0f / 60.0f;

static TXUITimelineControl* GetGroupTimelineControl( TXUIElement* a_pGroup )
{
	return TSTATICCAST( TXUIGroup, a_pGroup )->GetTimelineControl();
}


//-----------------------------------------------------------------------------
// XURXUISliderBarData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e5500
XURXUISliderBarData::XURXUISliderBarData()
{
	m_iRangeMin  = 0;
	m_iRangeMax  = 100;
	m_iValue     = 0;
	m_iStep      = 1;
	m_iAccelInc  = 0;
	m_iAccelTime = 0;
	m_bVertical  = TFALSE;
	m_pClass     = TClass::Find( "TXUISliderBar", &TGetClass( TXUIControl ) );
}

// $deBlob: FUNCTION 006e5560
TBOOL XURXUISliderBarData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e5590
TUINT32 XURXUISliderBarData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006e55c0
TBOOL XURXUISliderBarData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e5630
TBOOL XURXUISliderBarData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "RangeMin", -1 ) == 0 )
	{
		a_rPropIndex = Prop_RangeMin;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "RangeMax", -1 ) == 0 )
	{
		a_rPropIndex = Prop_RangeMax;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Value", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Value;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Step", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Step;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Vertical", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Vertical;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "AccelInc", -1 ) == 0 )
	{
		a_rPropIndex = Prop_AccelInc;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "AccelTime", -1 ) == 0 )
	{
		a_rPropIndex = Prop_AccelTime;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e5740
TUINT32 XURXUISliderBarData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex == Prop_Vertical ) ? 1 : 4;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e57b0
TBOOL XURXUISliderBarData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e5810
TBOOL XURXUISliderBarData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e5870
TBOOL XURXUISliderBarData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_RangeMin ) )
		{
			m_iRangeMin = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_RangeMax ) )
		{
			m_iRangeMax = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Value ) )
		{
			m_iValue = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Step ) )
		{
			m_iStep = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Vertical ) )
		{
			m_bVertical = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_AccelInc ) )
		{
			m_iAccelInc = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_AccelTime ) )
		{
			m_iAccelTime = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		TASSERT( m_iRangeMax > m_iRangeMin );
		TASSERT( m_iValue >= m_iRangeMin && m_iValue <= m_iRangeMax );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e54f0
const TXUITypeInfo* XURXUISliderBarData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUISliderBar
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e5a20
// $deBlob: FUNCTION 006e5a30
// $deBlob: FUNCTION 006e5a60
// $deBlob: FUNCTION 006e5a80
// $deBlob: FUNCTION 006e5a90
// $deBlob: FUNCTION 0053f660
// $deBlob: FUNCTION 00859370
TXUI_DEFINE_CLASS( TXUISliderBar );

// $deBlob: FUNCTION 006e5aa0
TXUISliderBar::TXUISliderBar()
{
	m_iValue           = 0;
	m_iRangeMin        = 0;
	m_iRangeMax        = 0;
	m_iStep            = 1;
	m_pBodyTimeline    = TNULL;
	m_fNormalTime      = 0.0f;
	m_fFocusTime       = 0.0f;
	m_bVertical        = TFALSE;
	m_iAccelInc        = 0;
	m_iAccelTime       = 0;
	m_bCapturing       = TFALSE;
	m_pSliderBody      = TNULL;
	m_pSliderBodyChild = TNULL;
}

// $deBlob: FUNCTION 006e5b40
// $deBlob: FUNCTION 006e6510
// $deBlob: FUNCTION 006e64fb
TXUISliderBar::~TXUISliderBar()
{
	if ( TXUI::GetSingleton()->GetCapturing() == this )
		TXUI::GetSingleton()->SetCapturing( TNULL );
}

// $deBlob: FUNCTION 006e5b70
TBOOL TXUISliderBar::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, TTRUE );

	XURXUISliderBarData* pData = TSTATICCAST( XURXUISliderBarData, a_pData );

	m_iRangeMin = pData->m_iRangeMin;
	m_iRangeMax = pData->m_iRangeMax;
	m_iStep     = pData->m_iStep;
	m_bVertical = pData->m_bVertical == TTRUE;

	TStringManager::String16Format( m_wszValue, 256, L"%d", m_iValue );

	if ( m_pVisual != TNULL )
	{
		m_pVisual->SetPresenterText( m_wszValue, 1, -1 );
		m_pSliderBody = m_pVisual->FindChildXUIElement( L"SliderBody" );

#ifdef DEBLOB_COMMUNITY_PATCH
		if ( m_pSliderBody != TNULL )
			m_pSliderBody->SetDisableTimelineRecursion( TTRUE );
#else
		// NOTE: retail doesn't check m_pSliderBody first
		m_pSliderBody->SetDisableTimelineRecursion( TTRUE );
#endif

		if ( m_pSliderBody != TNULL && m_pSliderBody->IsA( &TGetClass( TXUIGroup ) ) )
		{
			m_pBodyTimeline = GetGroupTimelineControl( m_pSliderBody );
			m_pBodyTimeline->PlayAnimation( TXUITimelineControl::StdNamedFrame_Normal, TXUITimelineControl::StdNamedFrame_EndNormal, 0 );
			m_pBodyTimeline->StopAnimation();

			m_fNormalTime      = m_pBodyTimeline->GetNamedFrameTime( L"Normal" );
			m_fFocusTime       = m_pBodyTimeline->GetNamedFrameTime( L"Focus" );
			m_pSliderBodyChild = m_pSliderBody->ChildBegin();
		}
	}

	SetValue( pData->m_iValue );
	return TTRUE;
}

// $deBlob: FUNCTION 006e5cb0
void TXUISliderBar::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	// NOTE: RangeMin and RangeMax aren't animated, Value is set directly (no SetValue)
	switch ( a_uiPropIndex )
	{
		case XURXUISliderBarData::Prop_Value:
			m_iValue = TINT( TMath::LERP( TFLOAT( a_uiValue1 ), TFLOAT( a_uiValue2 ), a_fScalar ) );
			return;
		case XURXUISliderBarData::Prop_Step:
			m_iStep = TINT( TMath::LERP( TFLOAT( a_uiValue1 ), TFLOAT( a_uiValue2 ), a_fScalar ) );
			return;
		case XURXUISliderBarData::Prop_Vertical:
			a_fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

			// NOTE: retail sets it to the inverted value
			if ( a_fScalar == 0.0f )
			{
				m_bVertical = a_uiValue1 == 0;
				return;
			}

			TASSERT( a_fScalar == 1.0f );
			m_bVertical = a_uiValue2 == 0;
			return;
		case XURXUISliderBarData::Prop_AccelInc:
			m_iAccelInc = TINT( TMath::LERP( TFLOAT( a_uiValue1 ), TFLOAT( a_uiValue2 ), a_fScalar ) );
			return;
		case XURXUISliderBarData::Prop_AccelTime:
			m_iAccelTime = TINT( TMath::LERP( TFLOAT( a_uiValue1 ), TFLOAT( a_uiValue2 ), a_fScalar ) );
			return;
	}
}

// NOTE: retail inlines TXUIControl's version (TXUIElement::UpdateTimelineProp with a_uiObjectIndex - 2)
// $deBlob: FUNCTION 006e5f70
void TXUISliderBar::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006e5fc0
void TXUISliderBar::SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation )
{
	TXUIControl::SetFocus( a_bFocus, a_bNoAnimation, a_bFromNavigation );

	if ( !a_bFocus )
		m_bCapturing = TFALSE;

	UpdateSliderPosition();
}

// $deBlob: FUNCTION 006e5ff0
void TXUISliderBar::Tick( TFLOAT a_fDeltaTime )
{
	TUINT32 uiState = GetTimelineState();

	// Stop dragging once it lost the focus
	if ( uiState != TimelineState_Focus && uiState != TimelineState_None )
		m_bCapturing = TFALSE;

	T2GUIElement::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006e6020
TBOOL TXUISliderBar::HandleButton( TINT a_eButton, TINT a_iController )
{
	TBOOL bHandled = TFALSE;
	TBOOL bIsMouse = TFALSE;

	if ( a_eButton == XUIButton_Accept )
	{
		// The button came from the mouse?
		TInputDevice* pDevice = TInputDevice::GetRegisteredDevices()[ TInputDevice::GetLastXUIDeviceIndex() ];
		bIsMouse              = pDevice->IsA( &TGetClass( TInputDXDeviceMouse ) );

		// NOTE: retail tests the keyboard and the controller too and doesn't use the results
		pDevice->IsA( &TGetClass( TInputDeviceKeyboard ) );
		pDevice->IsA( &TGetClass( TInputDeviceController ) );
	}

	if ( bIsMouse )
	{
		// Start dragging
		if ( !m_bCapturing )
			TXUI::GetSingleton()->SetCapturing( this );

		m_bCapturing = TTRUE;
		bHandled     = TTRUE;

		TXUIInput* pInput = TXUI::GetSingleton()->GetInput();

		if ( pInput->GetRepeatElement() == this )
		{
			HandleAnalog( pInput->GetMouseX(), pInput->GetMouseY(), a_iController );
		}
	}

	if ( m_bVertical == TTRUE )
	{
		if ( a_eButton == XUIButton_Up )
		{
			SetValue( m_iValue - m_iStep );
			UpdateText();
			return TTRUE;
		}

		if ( a_eButton == XUIButton_Down )
		{
			SetValue( m_iStep + m_iValue );
			UpdateText();
			return TTRUE;
		}
	}
	else
	{
		if ( a_eButton == XUIButton_Left )
		{
			SetValue( m_iValue - m_iStep );
			UpdateText();
			return TTRUE;
		}

		if ( a_eButton == XUIButton_Right )
		{
			SetValue( m_iStep + m_iValue );
			UpdateText();
			return TTRUE;
		}
	}

	if ( bHandled == TTRUE )
	{
		UpdateText();
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006e6190
TBOOL TXUISliderBar::HandleButtonRelease( TINT a_eButton, TINT a_iController )
{
	if ( a_eButton == XUIButton_Accept )
	{
		// Stop dragging
		if ( m_bCapturing )
		{
			// NOTE: retail's condition is "TXUI::GetSingleton()->GetCapturing() == this"
			TASSERT( TXUI::GetSingleton()->GetCapturing() == this );
			TXUI::GetSingleton()->SetCapturing( TNULL );
		}

		m_bCapturing = TFALSE;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006e6210
TBOOL TXUISliderBar::HandleAnalog( TINT a_iParam1, TINT a_iParam2, TINT a_iController )
{
	if ( m_bCapturing == TTRUE )
	{
		// Map the mouse position to the slider (its pivot is its center)
		T2GUITransform oTransform;

		TFLOAT fWidth;
		TFLOAT fHeight;
		GetDimensions( fWidth, fHeight );
		GetInvScreenTransform( oTransform );

		TVector2 vPos;
		oTransform.Transform( vPos, TVector2( TFLOAT( a_iParam1 ), TFLOAT( a_iParam2 ) ) );

		TFLOAT fRatio;

		if ( m_bVertical )
			fRatio = ( fHeight * 0.5f + vPos.y ) / fHeight;
		else
			fRatio = ( fWidth * 0.5f + vPos.x ) / fWidth;

		TMath::Clip( fRatio, 0.0f, 1.0f );

		// NOTE: retail ignores m_iRangeMin here
		SetValue( TINT( TFLOAT( m_iRangeMax ) * fRatio + 0.5f ) );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006e6380
TINT TXUISliderBar::GetValue()
{
	return m_iValue;
}

// $deBlob: FUNCTION 006e6390
void TXUISliderBar::SetValue( TINT a_iValue )
{
	if ( a_iValue != m_iValue )
	{
		if ( a_iValue < m_iRangeMin )
			a_iValue = m_iRangeMin;

		m_iValue = a_iValue;

		if ( m_iRangeMax < a_iValue )
			m_iValue = m_iRangeMax;

		UpdateText();
		UpdateSliderPosition();
	}
}

// $deBlob: FUNCTION 006e63e0
void TXUISliderBar::UpdateText()
{
	TStringManager::String16Format( m_wszValue, 256, L"%d", m_iValue );

	if ( m_pVisual != TNULL )
		m_pVisual->SetPresenterText( m_wszValue, 1, -1 );
}

// $deBlob: FUNCTION 006e6420
void TXUISliderBar::UpdateSliderPosition()
{
	if ( m_pSliderBody != TNULL && m_pSliderBodyChild != TNULL )
	{
		TSTATICCAST( TXUIElement, m_pSliderBodyChild )->SetPositionScale( TXUIElement::GetAnchoringScale() );

		TFLOAT fBaseTime = ( GetTimelineState() == TimelineState_Focus ) ? m_fFocusTime : m_fNormalTime;

		// The body timeline has 100 frames from the base time
		m_pBodyTimeline->SetTimeLineTime( ( TFLOAT( m_iValue - m_iRangeMin ) / TFLOAT( m_iRangeMax - m_iRangeMin ) ) * ( TXUISLIDERBAR_FRAMETIME * 100.0f ) + fBaseTime, TFALSE );
	}
}

TOSHI_NAMESPACE_END
