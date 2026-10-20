#include "ToshiPCH.h"
#include "TXUIProgressBar.h"
#include "TXUI.h"
#include "TXUIGroup.h"
#include "TXUIVisual.h"
#include "TXUITimelineControl.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiProgressBar" type info
const TXUITypeInfo XURXUIProgressBarData::sm_sTypeInfo = { L"XuiProgressBar", &XURXUIControlData::sm_sTypeInfo };

static TXUITimelineControl* GetGroupTimelineControl( TXUIElement* a_pGroup )
{
	return TSTATICCAST( TXUIGroup, a_pGroup )->GetTimelineControl();
}

static TFLOAT GetTimelineLength( TXUITimelineControl* a_pTimelineControl )
{
	return a_pTimelineControl->GetTimelineLength();
}

//-----------------------------------------------------------------------------
// XURXUIProgressBarData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e42a0
XURXUIProgressBarData::XURXUIProgressBarData()
{
	m_iRangeMin = 0;
	m_iRangeMax = 100;
	m_iValue    = 0;
	m_pClass    = TClass::Find( "TXUIProgressBar", &TGetClass( TXUIControl ) );
}

// $deBlob: FUNCTION 006e42e0
TBOOL XURXUIProgressBarData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e4310
TUINT32 XURXUIProgressBarData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006e4340
TBOOL XURXUIProgressBarData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e43a0
TBOOL XURXUIProgressBarData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
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

	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e4430
TUINT32 XURXUIProgressBarData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		TASSERT( a_uiPropIndex < Prop_NUMOF );
		return 4;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e44d0
TBOOL XURXUIProgressBarData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e4530
TBOOL XURXUIProgressBarData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e4590
TBOOL XURXUIProgressBarData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		TASSERT( m_iRangeMax > m_iRangeMin );
		TASSERT( m_iValue >= m_iRangeMin && m_iValue <= m_iRangeMax );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e4290
const TXUITypeInfo* XURXUIProgressBarData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIProgressBar
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e46b0
// $deBlob: FUNCTION 006e46c0
// $deBlob: FUNCTION 006e46f0
// $deBlob: FUNCTION 006e4710
// $deBlob: FUNCTION 006e4720
// $deBlob: FUNCTION 0053f5c0
// $deBlob: FUNCTION 00859330
TXUI_DEFINE_CLASS( TXUIProgressBar );

// $deBlob: FUNCTION 006e4730
TXUIProgressBar::TXUIProgressBar()
{
	m_iValue             = 0;
	m_iRangeMin          = 0;
	m_iRangeMax          = 0;
	m_pBodyTimeline      = TNULL;
	m_fStartTime         = 0.0f;
	m_pProgressBody      = TNULL;
	m_pProgressBodyChild = TNULL;
}

// $deBlob: FUNCTION 006e4790
// $deBlob: FUNCTION 006e4b80
// $deBlob: FUNCTION 006e4b6b
TXUIProgressBar::~TXUIProgressBar()
{
}

// $deBlob: FUNCTION 006e47b0
TBOOL TXUIProgressBar::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, TTRUE );

	XURXUIProgressBarData* pData = TSTATICCAST( XURXUIProgressBarData, a_pData );

	m_iValue    = pData->m_iValue;
	m_iRangeMin = pData->m_iRangeMin;
	m_iRangeMax = pData->m_iRangeMax;

	TStringManager::String16Format( m_wszValue, 256, L"%d", m_iValue );

	if ( m_pVisual != TNULL )
	{
		m_pVisual->SetPresenterText( m_wszValue, 1, -1 );
		m_pProgressBody = m_pVisual->FindChildXUIElement( L"ProgressBody" );

		if ( m_pProgressBody != TNULL && m_pProgressBody->IsA( &TGetClass( TXUIGroup ) ) )
		{
			m_pBodyTimeline = GetGroupTimelineControl( m_pProgressBody );

			m_pBodyTimeline->m_bEnabled = TFALSE;
			m_pBodyTimeline->StopAnimation();

			m_fStartTime         = m_pBodyTimeline->GetTimeLineTime();
			m_pProgressBodyChild = m_pProgressBody->ChildBegin();
		}

		if ( m_pBodyTimeline != TNULL )
		{
			m_pBodyTimeline->SetTimeLineTime( ( TFLOAT( m_iValue - m_iRangeMin ) / TFLOAT( m_iRangeMax - m_iRangeMin ) ) * GetTimelineLength( m_pBodyTimeline ) + m_fStartTime, TFALSE );

			TStringManager::String16Format( m_wszValue, 256, L"%d", m_iValue );
			m_pVisual->SetPresenterText( m_wszValue, 1, -1 );
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e4910
void TXUIProgressBar::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	// NOTE: only the value is animated
	if ( a_uiPropIndex == XURXUIProgressBarData::Prop_Value )
		SetValue( TINT( TMath::LERP( TFLOAT( a_uiValue1 ), TFLOAT( a_uiValue2 ), a_fScalar ) ) );
}

// NOTE: retail inlines TXUIControl's version (TXUIElement::UpdateTimelineProp with a_uiObjectIndex - 2)
// $deBlob: FUNCTION 006e49f0
void TXUIProgressBar::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006e4a40
void TXUIProgressBar::SetValue( TINT a_iValue )
{
	TMath::Clip( a_iValue, m_iRangeMin, m_iRangeMax );

	if ( m_pBodyTimeline != TNULL && m_pVisual != TNULL && m_iValue != a_iValue )
	{
		m_pBodyTimeline->SetTimeLineTime( ( TFLOAT( a_iValue - m_iRangeMin ) / TFLOAT( m_iRangeMax - m_iRangeMin ) ) * GetTimelineLength( m_pBodyTimeline ) + m_fStartTime, TFALSE );

		TStringManager::String16Format( m_wszValue, 256, L"%d", a_iValue );
		m_pVisual->SetPresenterText( m_wszValue, 1, -1 );
	}

	m_iValue = a_iValue;
}

// $deBlob: FUNCTION 006e4af0
void TXUIProgressBar::Tick( TFLOAT a_fDeltaTime )
{
	TUINT32 uiState = GetTimelineState();

	if ( ( uiState == TimelineState_Focus || uiState == TimelineState_None ) && m_pProgressBody != TNULL && m_pProgressBodyChild != TNULL )
	{
		TSTATICCAST( TXUIElement, m_pProgressBodyChild )->SetPositionScale( TXUIElement::GetAnchoringScale() );
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006e4c00
TINT TXUIProgressBar::GetValue()
{
	return m_iValue;
}

// $deBlob: FUNCTION 006e4bf0
TINT TXUIProgressBar::GetRangeMin()
{
	return m_iRangeMin;
}

// $deBlob: FUNCTION 006e4c80
void TXUIProgressBar::SetRangeMin( TINT a_iRangeMin )
{
	m_iRangeMin = a_iRangeMin;
}

// $deBlob: FUNCTION 006e4be0
TINT TXUIProgressBar::GetRangeMax()
{
	return m_iRangeMax;
}

// $deBlob: FUNCTION 006e4c70
void TXUIProgressBar::SetRangeMax( TINT a_iRangeMax )
{
	m_iRangeMax = a_iRangeMax;
}

TOSHI_NAMESPACE_END
