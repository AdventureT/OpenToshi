#include "ToshiPCH.h"
#include "TXUITabScene.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiTabScene" type info
const TXUITypeInfo XURXUITabSceneData::sm_sTypeInfo = { L"XuiTabScene", &XURXUISceneData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUITabSceneData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e7970
const TXUITypeInfo* XURXUITabSceneData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// $deBlob: FUNCTION 006e7980
XURXUITabSceneData::XURXUITabSceneData()
{
	m_uiTabCount     = 1;
	m_uiDefaultTab   = 0;
	m_bWrap          = TFALSE;
	m_bUserInterrupt = TFALSE;
	m_bVerticalTabs  = TFALSE;
	m_bNoAutoHide    = TFALSE;
	m_pClass         = TClass::Find( "TXUITabScene", &TGetClass( TXUIScene ) );
}

// $deBlob: FUNCTION 006e79c0
TBOOL XURXUITabSceneData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUISceneData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e79f0
TUINT32 XURXUITabSceneData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUISceneData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006e7a20
TBOOL XURXUITabSceneData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISceneData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e7a90
TBOOL XURXUITabSceneData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "TabCount", -1 ) == 0 )
	{
		a_rPropIndex = Prop_TabCount;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Wrap", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Wrap;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "UserInterrupt", -1 ) == 0 )
	{
		a_rPropIndex = Prop_UserInterrupt;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "VerticalTabs", -1 ) == 0 )
	{
		a_rPropIndex = Prop_VerticalTabs;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "NoAutoHide", -1 ) == 0 )
	{
		a_rPropIndex = Prop_NoAutoHide;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "DefaultTab", -1 ) == 0 )
	{
		a_rPropIndex = Prop_DefaultTab;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUISceneData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e7b80
TUINT32 XURXUITabSceneData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_Wrap || a_uiPropIndex == Prop_UserInterrupt || a_uiPropIndex == Prop_VerticalTabs || a_uiPropIndex == Prop_NoAutoHide )
			return 1;

		return 4;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISceneData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e7c00
TBOOL XURXUITabSceneData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISceneData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e7c60
TBOOL XURXUITabSceneData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISceneData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e7cc0
TBOOL XURXUITabSceneData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_TabCount ) )
		{
			m_uiTabCount = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Wrap ) )
		{
			m_bWrap = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_UserInterrupt ) )
		{
			m_bUserInterrupt = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_VerticalTabs ) )
		{
			m_bVerticalTabs = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_NoAutoHide ) )
		{
			m_bNoAutoHide = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_DefaultTab ) )
		{
			m_uiDefaultTab = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// TXUITabScene
//-----------------------------------------------------------------------------

TWCHAR TXUITabScene::ms_wszTransTabEnd[ 256 ];

// $deBlob: FUNCTION 006e7d80
// $deBlob: FUNCTION 006e7d90
// $deBlob: FUNCTION 006e7e00
// $deBlob: FUNCTION 006e7e60
// $deBlob: FUNCTION 006e7e70
// $deBlob: FUNCTION 0053f720
// $deBlob: FUNCTION 008593d0
TXUI_DEFINE_CLASS( TXUITabScene );

// $deBlob: FUNCTION 006e7e80
TXUITabScene::TXUITabScene()
{
	m_apTabs        = TNULL;
	m_uiNumTabs     = 0;
	m_uiCurrentTab  = 0;
	m_bVerticalTabs = TFALSE;
	m_bWrap         = TFALSE;
	m_bForwardButtons = TFALSE;
}

// $deBlob: FUNCTION 006e7ed0
// $deBlob: FUNCTION 006e8550
// $deBlob: FUNCTION 006e853b
TXUITabScene::~TXUITabScene()
{
	// The tabs are children of the scene
	for ( TUINT32 i = 0; i < m_uiNumTabs; i++ )
		m_apTabs[ i ] = TNULL;

	operator delete( m_apTabs );
}

// $deBlob: FUNCTION 006e7f20
TBOOL TXUITabScene::SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam )
{
	// The end named frame of the tab transition
	if ( a_eMessage == XUIMessage_NamedFrame && TStringManager::String16Compare( a_wszParam, ms_wszTransTabEnd, -1 ) == 0 )
		return NotifyValueChanged();

	return TXUIControl::SendMessage( a_eMessage, a_wszParam );
}

// EndTransTab
// $deBlob: FUNCTION 006e7f70
TBOOL TXUITabScene::NotifyValueChanged()
{
	if ( m_apTabs != TNULL )
	{
		TINT iController = TXUI::GetSingleton()->GetDefaultController();
		SetFocus( m_apTabs[ m_uiCurrentTab ]->GetFocus( iController ), iController );
	}

	SetXUIStateBits( State_Focus, 0xF, XUIState_TimelineStateShift );
	return TXUIControl::NotifyValueChanged();
}

// $deBlob: FUNCTION 006e7fc0
TXUIElement* TXUITabScene::RayTraceDeepest( const TVector2& a_rPoint, const TVector2& a_rScale, TINT a_iController )
{
	if ( !IsVisible() )
		return TNULL;

	TVector2 vScale( a_rScale.x * m_vScale.GetX(), a_rScale.y * m_vScale.GetY() );
	return m_apTabs[ m_uiCurrentTab ]->RayTraceDeepest( a_rPoint, vScale, a_iController );
}

// $deBlob: FUNCTION 006e8050
TBOOL TXUITabScene::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_iController != TXUI::GetSingleton()->GetDefaultController() )
		return TFALSE;

	if ( GetTimelineState() == State_TransTab )
		return TTRUE;

	if ( m_apTabs == TNULL )
		return TFALSE;

	if ( !m_bForwardButtons && ( a_eButton == XUIButton_Left || a_eButton == XUIButton_Right ) && !m_bVerticalTabs )
	{
		TUINT32 uiNewTab;

		if ( a_eButton == XUIButton_Left )
		{
			// The previous tab
			if ( m_uiCurrentTab != 0 )
				uiNewTab = m_uiCurrentTab - 1;
			else
				uiNewTab = m_bWrap ? m_uiNumTabs - 1 : 0;
		}
		else
		{
			// The next tab
			uiNewTab = m_uiNumTabs - 1;

			if ( m_uiCurrentTab < uiNewTab )
				uiNewTab = m_uiCurrentTab + 1;
			else if ( m_bWrap )
				uiNewTab = 0;
		}

		if ( uiNewTab != m_uiCurrentTab )
		{
			TWCHAR wszStartFrame[ 256 ];
			TStringManager::String16Format( wszStartFrame, 0x100, L"%dTo%d", m_uiCurrentTab + 1, uiNewTab + 1 );
			TStringManager::String16Format( ms_wszTransTabEnd, 0x100, L"%lsEnd", wszStartFrame );

			m_apTabs[ m_uiCurrentTab ]->SetFocus( TFALSE, TFALSE, TTRUE );
			m_apTabs[ uiNewTab ]->SetFocus( TTRUE, TFALSE, TTRUE );
			m_oTimelineControl.PlayAnimation( wszStartFrame, ms_wszTransTabEnd );

			m_uiCurrentTab = uiNewTab;
			SetXUIStateBits( State_TransTab, 0xF, XUIState_TimelineStateShift );
			SetFocus( TNULL, a_iController );
		}

		return TTRUE;
	}

	if ( m_apTabs[ m_uiCurrentTab ]->HandleButton( a_eButton, a_iController ) == TTRUE )
		return TTRUE;

	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// $deBlob: FUNCTION 006e8240
TBOOL TXUITabScene::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIScene::Create( a_rResource, a_pData, TTRUE );

	XURXUITabSceneData* pData = TSTATICCAST( XURXUITabSceneData, a_pData );

	m_uiNumTabs = pData->m_uiTabCount;
	m_apTabs    = TSTATICCAST( TXUIScene*, TMalloc( m_uiNumTabs * sizeof( TXUIScene* ), TXUI::ms_pXUIMemoryBlock ) );

	for ( TUINT32 i = 0; i < m_uiNumTabs; i++ )
	{
		TWCHAR wszName[ 256 ];
		TStringManager::String16Format( wszName, 0x100, L"Tab%d", i + 1 );

		TXUIElement* pElement = FindChildXUIElement( wszName );
		TASSERT( TTRUE == pElement->IsA( &TGetClass( TXUIScene ) ) );

		m_apTabs[ i ] = TSTATICCAST( TXUIScene, pElement );
		m_apTabs[ i ]->SetVisible( TTRUE );
	}

	m_uiCurrentTab  = pData->m_uiDefaultTab;
	m_bVerticalTabs = pData->m_bVerticalTabs == TTRUE;
	m_bWrap         = pData->m_bWrap == TTRUE;

	TXUIScene* pTab = m_apTabs[ m_uiCurrentTab ];

	if ( pTab != TNULL )
	{
		TINT iController = TXUI::GetSingleton()->GetDefaultController();
		SetFocus( pTab, iController );

		if ( GetFocus( iController ) != TNULL )
			GetFocus( iController )->SetFocus( TTRUE, TTRUE, TTRUE );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e8480
void TXUITabScene::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIScene::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006e84f0
void TXUITabScene::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIScene::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

TOSHI_NAMESPACE_END
