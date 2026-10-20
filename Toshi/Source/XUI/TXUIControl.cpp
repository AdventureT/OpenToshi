#include "ToshiPCH.h"
#include "TXUIControl.h"
#include "TXUI.h"
#include "TXUIScene.h"
#include "TXUIVisual.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiControl" type info
const TXUITypeInfo XURXUIControlData::sm_sTypeInfo = { L"XuiControl", &XURXUIElementData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIControlData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006c9ed0
XURXUIControlData::XURXUIControlData()
{
	m_uiClassOverride  = 0;
	m_uiVisual         = 0;
	m_uiText           = 0;
	m_uiImagePath      = 0;
	m_bEnabled         = TTRUE;
	m_bUnfocussedInput = TFALSE;
	m_uiNavLeft        = 0;
	m_uiNavRight       = 0;
	m_uiNavUp          = 0;
	m_uiNavDown        = 0;

	m_pClass = TClass::Find( "TXUIControl", &TGetClass( TXUIElement ) );
}

// $deBlob: FUNCTION 006c9f10
TBOOL XURXUIControlData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006c9f40
TUINT32 XURXUIControlData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006c9f70
TBOOL XURXUIControlData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006c9fe0
TBOOL XURXUIControlData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "ClassOverride", -1 ) == 0 )
	{
		a_rPropIndex = Prop_ClassOverride;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Visual", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Visual;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Enabled", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Enabled;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "UnfocussedInput", -1 ) == 0 )
	{
		a_rPropIndex = Prop_UnfocussedInput;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "NavLeft", -1 ) == 0 )
	{
		a_rPropIndex = Prop_NavLeft;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "NavRight", -1 ) == 0 )
	{
		a_rPropIndex = Prop_NavRight;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "NavUp", -1 ) == 0 )
	{
		a_rPropIndex = Prop_NavUp;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "NavDown", -1 ) == 0 )
	{
		a_rPropIndex = Prop_NavDown;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Text", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Text;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "ImagePath", -1 ) == 0 )
	{
		a_rPropIndex = Prop_ImagePath;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006ca160
TUINT32 XURXUIControlData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_Enabled || a_uiPropIndex == Prop_UnfocussedInput )
			return 1;

		if ( a_uiPropIndex == Prop_ClassOverride || a_uiPropIndex == Prop_Visual || a_uiPropIndex == Prop_NavLeft || a_uiPropIndex == Prop_NavRight ||
		     a_uiPropIndex == Prop_NavUp || a_uiPropIndex == Prop_NavDown || a_uiPropIndex == Prop_Text || a_uiPropIndex == Prop_ImagePath )
			return 2;

		return 4;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ca210
TBOOL XURXUIControlData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ca270
TBOOL XURXUIControlData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006c9ec0
const TXUITypeInfo* XURXUIControlData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// $deBlob: FUNCTION 006ca2d0
TBOOL XURXUIControlData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT8 uiNumProps = *a_rData;
	a_rData += 1;

	if ( uiNumProps != 0 )
	{
		TUINT16 uiPropMask = 0;

		if ( m_uiPropLevelMask > 0 )
		{
			uiPropMask = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_ClassOverride ) )
		{
			m_uiClassOverride = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_Visual ) )
		{
			m_uiVisual = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_Enabled ) )
		{
			m_bEnabled = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_UnfocussedInput ) )
		{
			m_bUnfocussedInput = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_NavLeft ) )
		{
			m_uiNavLeft = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_NavRight ) )
		{
			m_uiNavRight = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_NavUp ) )
		{
			m_uiNavUp = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_NavDown ) )
		{
			m_uiNavDown = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_Text ) )
		{
			m_uiText = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_ImagePath ) )
		{
			m_uiImagePath = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// TXUIControl
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006ca420
// $deBlob: FUNCTION 006ca430
// $deBlob: FUNCTION 006ca460
// $deBlob: FUNCTION 006ca480
// $deBlob: FUNCTION 006ca490
// $deBlob: FUNCTION 0053eef0
// $deBlob: FUNCTION 00859090
TXUI_DEFINE_CLASS( TXUIControl );

// $deBlob: FUNCTION 006ca4a0
TXUIControl::TXUIControl()
{
	m_pVisual          = TNULL;
	m_wszText          = TNULL;
	m_wszImagePath     = TNULL;
	m_uiControlFlags   = ControlFlags_Enabled;
	m_iPressController = -1;

	for ( TINT i = 0; i < Nav_NUMOF; i++ )
	{
		m_awszNav[ i ] = TNULL;
		m_apNav[ i ]   = TNULL;
	}
}

// $deBlob: FUNCTION 006ca520
// $deBlob: FUNCTION 006cb710
// $deBlob: FUNCTION 006cb6fb
TXUIControl::~TXUIControl()
{
	// Forget the scene's focus on this control
	if ( TXUIScene* pActiveScene = TXUI::GetSingleton()->GetActiveScene() )
	{
		for ( TINT i = 0; i < TXUIScene::MAXCONTROLLERS; i++ )
		{
			if ( this == pActiveScene->GetFocus( i ) )
				pActiveScene->SetFocus( TNULL, i );
		}
	}

	if ( m_pVisual )
	{
		m_pVisual->RemoveSelf();

		if ( m_pVisual )
			delete m_pVisual;
	}
}

// $deBlob: FUNCTION 006ca620
TBOOL TXUIControl::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, TTRUE );
	CreateTimelineControl( a_rResource, a_pData );

	XURXUIControlData* pData = TSTATICCAST( XURXUIControlData, a_pData );

	if ( m_pVisual == TNULL || pData->m_uiVisual != TSTATICCAST( XURXUIControlData, m_pData )->m_uiVisual )
		CreateVisual( a_rResource, a_rResource.GetString( pData->m_uiVisual ) );

	if ( pData->m_uiText != 0 )
		SetText( a_rResource.GetString( pData->m_uiText ), -1 );

	if ( pData->m_uiImagePath != 0 )
		SetImagePath( a_rResource.GetString( pData->m_uiImagePath ) );

	if ( pData->m_uiNavLeft != 0 )
		m_awszNav[ Nav_Left ] = a_rResource.GetString( pData->m_uiNavLeft );

	if ( pData->m_uiNavRight != 0 )
		m_awszNav[ Nav_Right ] = a_rResource.GetString( pData->m_uiNavRight );

	if ( pData->m_uiNavUp != 0 )
		m_awszNav[ Nav_Up ] = a_rResource.GetString( pData->m_uiNavUp );

	if ( pData->m_uiNavDown != 0 )
		m_awszNav[ Nav_Down ] = a_rResource.GetString( pData->m_uiNavDown );

	SetUnfocussedInput( pData->m_bUnfocussedInput );

	// NOTE: inlined SetEnabled
	TUINT8 uiFlags = m_uiControlFlags;

	if ( pData->m_bEnabled == TTRUE )
	{
		if ( !( uiFlags & ControlFlags_Enabled ) )
		{
			m_uiControlFlags = uiFlags | ControlFlags_Enabled;
			PlayVisual( TFALSE, TFALSE, TFALSE );
		}
	}
	else if ( uiFlags & ControlFlags_Enabled )
	{
		m_uiControlFlags = uiFlags & ~ControlFlags_Enabled;
		PlayVisual( TFALSE, TFALSE, TFALSE );
	}

	if ( m_pVisual != TNULL )
		PlayVisual( TFALSE, TFALSE, TFALSE );

	return TTRUE;
}

// $deBlob: FUNCTION 006ca760
void TXUIControl::ResolveNavigation( TXUIElement* a_pRoot )
{
	if ( a_pRoot == TNULL )
		return;

	for ( TINT i = 0; i < Nav_NUMOF; i++ )
	{
		if ( m_awszNav[ i ] != TNULL )
		{
			TXUIElement* pElement = a_pRoot->FindChildXUIElement( m_awszNav[ i ] );

			if ( pElement != TNULL )
				m_apNav[ i ] = CastFromTObject( pElement );
		}
	}
}

// $deBlob: FUNCTION 006ca800
TBOOL TXUIControl::EndPress()
{
	TXUI::GetSingleton()->SetInputLocked( TFALSE );
	TASSERT( TTRUE == GetParent()->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) );

	TBOOL        bHandled    = TFALSE;
	TXUIElement* pParent     = TSTATICCAST( TXUIElement, m_pParent );
	TINT         iController = ( m_iPressController == -1 ) ? TXUI::GetSingleton()->GetDefaultController() : m_iPressController;

	while ( pParent != TNULL )
	{
		bHandled = pParent->OnNotifyPress( this, iController );
		pParent  = TSTATICCAST( TXUIElement, pParent->GetParent() );

		if ( pParent == TNULL || !pParent->GetFlag( FLAGS_XUIELEMENT ) || bHandled )
			break;
	}

	m_iPressController = -1;
	return bHandled;
}

// $deBlob: FUNCTION 006ca8a0
TBOOL TXUIControl::Focus()
{
	TASSERT( TTRUE == GetParent()->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) );

	TBOOL        bHandled = TFALSE;
	TXUIElement* pParent  = TSTATICCAST( TXUIElement, m_pParent );

	while ( pParent != TNULL )
	{
		bHandled = pParent->OnNotifySetFocus( this );
		pParent  = TSTATICCAST( TXUIElement, pParent->GetParent() );

		if ( pParent == TNULL || !pParent->GetFlag( FLAGS_XUIELEMENT ) || bHandled )
			break;
	}

	return bHandled;
}

// $deBlob: FUNCTION 006ca910
TBOOL TXUIControl::NotifyValueChanged()
{
	TASSERT( TTRUE == GetParent()->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) );

	TBOOL        bHandled = TFALSE;
	TXUIElement* pParent  = TSTATICCAST( TXUIElement, m_pParent );

	while ( pParent != TNULL )
	{
		bHandled = pParent->OnNotifyValueChanged( this );
		pParent  = TSTATICCAST( TXUIElement, pParent->GetParent() );

		if ( pParent == TNULL || !pParent->GetFlag( FLAGS_XUIELEMENT ) || bHandled )
			break;
	}

	m_iPressController = -1;
	return bHandled;
}

// $deBlob: FUNCTION 006ca980
TBOOL TXUIControl::SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam )
{
	switch ( a_eMessage )
	{
		case XUIMessage_Press: return Press();
		case XUIMessage_EndPress: return EndPress();
		case XUIMessage_Focus: return Focus();
		case XUIMessage_EndTransFrom: return NavLeft();
		case XUIMessage_EndTransTo: return NavRight();
		case XUIMessage_EndTransBackFrom: return NavUp();
		case XUIMessage_EndTransBackTo: return NavDown();
		case XUIMessage_EndScrolling: return Back();
		default: return TFALSE;
	}
}

// $deBlob: FUNCTION 006caa30
void TXUIControl::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	TASSERT( a_uiPropIndex < XURXUIControlData::Prop_NUMOF );

	TFLOAT fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

	switch ( a_uiPropIndex )
	{
		case XURXUIControlData::Prop_Visual:
			if ( fScalar == 0.0f )
			{
				CreateVisual( *a_pResource, a_uiValue1 );
				return;
			}

			TASSERT( a_fScalar == 1.0f );
			CreateVisual( *a_pResource, a_uiValue2 );
			return;
		case XURXUIControlData::Prop_Enabled:
		{
			TINT8 iEnabled = TINT8( a_uiValue1 );

			if ( fScalar != 0.0f )
			{
				iEnabled = TINT8( a_uiValue2 );
				TASSERT( a_fScalar == 1.0f );
			}

			TBOOL bEnabled = IsEnabled();

			if ( iEnabled == 0 )
			{
				if ( bEnabled != TFALSE )
					SetEnabled( iEnabled == 1 );
			}
			else if ( bEnabled == TFALSE )
			{
				SetEnabled( iEnabled == 1 );
			}

			break;
		}
		case XURXUIControlData::Prop_UnfocussedInput:
			if ( fScalar == 0.0f )
			{
				SetUnfocussedInput( TINT8( a_uiValue1 ) != 0 );
				return;
			}

			TASSERT( a_fScalar == 1.0f );
			SetUnfocussedInput( TINT8( a_uiValue2 ) != 0 );
			return;
		case XURXUIControlData::Prop_NavLeft:
		case XURXUIControlData::Prop_NavRight:
		case XURXUIControlData::Prop_NavUp:
		case XURXUIControlData::Prop_NavDown:
		{
			if ( fScalar != 0.0f )
			{
				a_uiValue1 = a_uiValue2;
				TASSERT( a_fScalar == 1.0f );
			}

			TINT iNav        = a_uiPropIndex - XURXUIControlData::Prop_NavLeft;
			m_awszNav[ iNav ] = a_pResource->GetString( TUINT16( a_uiValue1 ) );

			TASSERT( GetParent()->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) );
			m_apNav[ iNav ] = TSTATICCAST( TXUIElement, GetParent() )->FindChildXUIControl( m_awszNav[ iNav ] );
			return;
		}
		case XURXUIControlData::Prop_Text:
			if ( fScalar != 0.0f )
			{
				a_uiValue1 = a_uiValue2;
				TASSERT( a_fScalar == 1.0f );
			}

			if ( m_pVisual != TNULL )
				m_pVisual->SetPresenterText( a_pResource->GetString( TUINT16( a_uiValue1 ) ), 0, -1 );

			break;
		case XURXUIControlData::Prop_ImagePath:
			if ( fScalar != 0.0f )
			{
				a_uiValue1 = a_uiValue2;
				TASSERT( a_fScalar == 1.0f );
			}

			if ( m_pVisual != TNULL )
				m_pVisual->SetPresenterImagePath( a_pResource->GetString( TUINT16( a_uiValue1 ) ), 0 );

			break;
	}
}

// $deBlob: FUNCTION 0062bce0
void TXUIControl::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006cb110
void TXUIControl::Render()
{
	if ( m_pVisual != TNULL )
	{
		if ( !m_pVisual->SkipRender() )
		{
			m_pVisual->PreRender();
			m_pVisual->Render();
			m_pVisual->PostRender();
		}

		// The visual is rendered first, under the children
		TBOOL bVisualVisible = m_pVisual->IsVisible();
		m_pVisual->SetVisible( TFALSE );
		T2GUIElement::Render();
		m_pVisual->SetVisible( bVisualVisible );
		return;
	}

	T2GUIElement::Render();
}

// $deBlob: FUNCTION 006262f0
void TXUIControl::SetWidth( TFLOAT a_fWidth )
{
	TXUIElement::SetWidth( a_fWidth );
}

// $deBlob: FUNCTION 006261b0
void TXUIControl::SetHeight( TFLOAT a_fHeight )
{
	TXUIElement::SetHeight( a_fHeight );
}

// $deBlob: FUNCTION 006260a0
void TXUIControl::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TXUIElement::SetDimensions( a_fWidth, a_fHeight );
}

// $deBlob: FUNCTION 00626000
TBOOL TXUIControl::IsCheckable()
{
	return ( m_uiControlFlags >> 1 ) & 1;
}

// $deBlob: FUNCTION 00626020
TBOOL TXUIControl::IsEnabled()
{
	return m_uiControlFlags & ControlFlags_Enabled;
}

// $deBlob: FUNCTION 00626060
TBOOL TXUIControl::Press()
{
	return TFALSE;
}

// $deBlob: FUNCTION 0062bc40
TBOOL TXUIControl::NavLeft()
{
	return TFALSE;
}

// $deBlob: FUNCTION 0062bc50
TBOOL TXUIControl::NavRight()
{
	return TFALSE;
}

// $deBlob: FUNCTION 0062bc20
TBOOL TXUIControl::NavUp()
{
	return TFALSE;
}

// $deBlob: FUNCTION 0062bc30
TBOOL TXUIControl::NavDown()
{
	return TFALSE;
}

// $deBlob: FUNCTION 00625dd0
TBOOL TXUIControl::Back()
{
	return TFALSE;
}

// $deBlob: FUNCTION 00625f00
const TWCHAR* TXUIControl::GetText()
{
	return m_wszText;
}

// $deBlob: FUNCTION 006262a0
void TXUIControl::SetText( const TWCHAR* a_wszText, TINT a_iParam )
{
	m_wszText = a_wszText;

	if ( m_pVisual != TNULL )
		m_pVisual->SetPresenterText( a_wszText, 0, a_iParam );
}

// $deBlob: FUNCTION 006262d0
void TXUIControl::SetText2( const TWCHAR* a_wszText, TUINT32 a_uiIndex )
{
	if ( m_pVisual != TNULL )
		m_pVisual->SetPresenterText2( a_wszText, a_uiIndex, 0 );
}

// $deBlob: FUNCTION 00625e90
const TWCHAR* TXUIControl::GetImagePath()
{
	return m_wszImagePath;
}

// $deBlob: FUNCTION 006261d0
void TXUIControl::SetImagePath( const TWCHAR* a_wszImagePath )
{
	m_wszImagePath = a_wszImagePath;

	if ( m_pVisual != TNULL )
		m_pVisual->SetPresenterImagePath( a_wszImagePath, 0 );
}

// $deBlob: FUNCTION 0062bc60
TBOOL TXUIControl::HandleButton( TINT a_eButton, TINT a_iController )
{
	return TFALSE;
}

// $deBlob: FUNCTION 00625fd0
TBOOL TXUIControl::HandleButtonRelease( TINT a_eButton, TINT a_iController )
{
	return TFALSE;
}

// $deBlob: FUNCTION 00625fe0
TBOOL TXUIControl::HandleAnalog( TINT a_iParam1, TINT a_iParam2, TINT a_iController )
{
	return TFALSE;
}

// $deBlob: FUNCTION 0062bc10
void TXUIControl::CreateTimelineControl( TXUIResource& a_rResource, XURXUIObjectData* a_pData )
{
}

// $deBlob: FUNCTION 00625ff0
TBOOL TXUIControl::AcceptsUnfocussedInput()
{
	return ( m_uiControlFlags >> 3 ) & 1;
}

// $deBlob: FUNCTION 006cb190
void TXUIControl::CreateVisual( TXUIResource& a_rResource, const TWCHAR* a_wszVisual )
{
	if ( m_pVisual != TNULL )
	{
		delete m_pVisual;
		m_pVisual = TNULL;
	}

	if ( TStringManager::String16Compare( a_wszVisual, L"", -1 ) != 0 )
	{
		// "skin2..." visuals are in the second skin
		TBOOL bSkin2 = TStringManager::String16Compare( a_wszVisual, L"skin2", 4 ) == 0;

		if ( bSkin2 )
			a_wszVisual += 5;

		m_pVisual = TXUI::GetSingleton()->CreateVisual( a_wszVisual, bSkin2 );
	}

	if ( m_pVisual == TNULL )
	{
		// Use the default visual of the XUI class or of one of its parents
		const TXUITypeInfo* pTypeInfo = TSTATICCAST( XURXUIElementData, m_pData )->GetTypeInfo();
		TASSERT( 0 == TStringManager::String16Compare( pTypeInfo->GetXuiType(), a_rResource.GetString( TSTATICCAST( XURXUIControlData, m_pData )->m_uiTypeIndex ) ) );

		for ( ; pTypeInfo != TNULL; pTypeInfo = pTypeInfo->GetParent() )
		{
			m_pVisual = TXUI::GetSingleton()->CreateVisual( pTypeInfo->GetXuiType(), TFALSE );

			if ( m_pVisual != TNULL )
				break;
		}
	}

	if ( m_pVisual != TNULL )
	{
		m_pVisual->SetDimensions( GetWidth(), GetHeight() );
		AddChildTail( m_pVisual );
	}
}

// $deBlob: FUNCTION 006cb790
void TXUIControl::CreateVisual( TXUIResource& a_rResource, TUINT32 a_uiVisualIndex )
{
	if ( m_pVisual == TNULL || a_uiVisualIndex != TSTATICCAST( XURXUIControlData, m_pData )->m_uiVisual )
		CreateVisual( a_rResource, a_rResource.GetString( TUINT16( a_uiVisualIndex ) ) );
}

// $deBlob: FUNCTION 006cb2b0
void TXUIControl::SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation )
{
	TUINT32 uiState = GetTimelineState();

	if ( a_bFocus )
	{
		if ( uiState == TimelineState_Focus )
			return;
	}
	else if ( uiState != TimelineState_Focus )
	{
		return;
	}

	TBOOL bWasPressed = TFALSE;

	if ( a_bFocus )
	{
		bWasPressed  = uiState == TimelineState_Press;
		m_uiXUIState = ( m_uiXUIState & ~0x340 ) | ( TimelineState_Focus << XUIState_TimelineStateShift );

		if ( !bWasPressed )
			SendMessage( XUIMessage_Focus, TNULL );
	}
	else
	{
		m_uiXUIState &= ~XUIState_TimelineStateMask;
	}

	PlayVisual( a_bNoAnimation, a_bFromNavigation, bWasPressed );
}

// $deBlob: FUNCTION 006cb340
void TXUIControl::PlayVisual( TBOOL a_bNoAnimation, TBOOL a_bFromNavigation, TBOOL a_bInit )
{
	TUINT32 uiState = GetTimelineState();

	if ( m_pVisual == TNULL )
		return;

	if ( uiState == TimelineState_Press )
	{
		TBOOL bEnabled1 = IsEnabled();
		TBOOL bEnabled2 = IsEnabled();
		m_pVisual->PlayAnimation( ( bEnabled1 == TFALSE ) * 4 + TXUITimelineControl::StdNamedFrame_Press, ( bEnabled2 == TFALSE ) * 4 + TXUITimelineControl::StdNamedFrame_EndPress );
		return;
	}

	if ( uiState == TimelineState_Focus )
	{
		if ( IsCheckable() && IsChecked() )
		{
			TBOOL bEnabled = IsEnabled();

			if ( a_bNoAnimation )
			{
				TINT iStart = bEnabled ? TXUITimelineControl::StdNamedFrame_InitFocusCheck : TXUITimelineControl::StdNamedFrame_InitFocusCheckDisable;
				TINT iEnd   = IsEnabled() ? TXUITimelineControl::StdNamedFrame_EndInitFocusCheck : TXUITimelineControl::StdNamedFrame_EndInitFocusCheckDisable;
				m_pVisual->PlayAnimation( iStart, iEnd );
				return;
			}

			if ( a_bInit )
			{
				TINT iStart = bEnabled ? TXUITimelineControl::StdNamedFrame_EndFocusCheck : TXUITimelineControl::StdNamedFrame_EndFocusCheckDisable;
				TINT iEnd   = IsEnabled() ? TXUITimelineControl::StdNamedFrame_EndFocusCheck : TXUITimelineControl::StdNamedFrame_EndFocusCheckDisable;
				m_pVisual->PlayAnimation( iStart, iEnd );
				return;
			}

			TINT iStart = bEnabled ? TXUITimelineControl::StdNamedFrame_FocusCheck : TXUITimelineControl::StdNamedFrame_FocusCheckDisable;
			TINT iEnd   = IsEnabled() ? TXUITimelineControl::StdNamedFrame_EndFocusCheck : TXUITimelineControl::StdNamedFrame_EndFocusCheckDisable;
			m_pVisual->PlayAnimation( iStart, iEnd );
			return;
		}

		TBOOL bEnabled = IsEnabled();

		if ( a_bNoAnimation )
		{
			TINT iStart = a_bInit ? ( bEnabled == TFALSE ) * 2 + TXUITimelineControl::StdNamedFrame_EndInitFocus : ( bEnabled == TFALSE ) * 2 + TXUITimelineControl::StdNamedFrame_InitFocus;
			TINT iEnd   = ( IsEnabled() == TFALSE ) * 2 + TXUITimelineControl::StdNamedFrame_EndInitFocus;
			m_pVisual->PlayAnimation( iStart, iEnd );
			return;
		}

		if ( a_bInit )
		{
			TINT iStart = bEnabled ? TXUITimelineControl::StdNamedFrame_EndFocus : TXUITimelineControl::StdNamedFrame_EndFocusDisable;
			TINT iEnd   = IsEnabled() ? TXUITimelineControl::StdNamedFrame_EndFocus : TXUITimelineControl::StdNamedFrame_EndFocusDisable;
			m_pVisual->PlayAnimation( iStart, iEnd );
			return;
		}

		TINT iStart = bEnabled ? TXUITimelineControl::StdNamedFrame_Focus : TXUITimelineControl::StdNamedFrame_FocusDisable;
		TINT iEnd   = IsEnabled() ? TXUITimelineControl::StdNamedFrame_EndFocus : TXUITimelineControl::StdNamedFrame_EndFocusDisable;
		m_pVisual->PlayAnimation( iStart, iEnd );
		return;
	}

	if ( IsCheckable() && IsChecked() )
	{
		TINT iStart = IsEnabled() ? TXUITimelineControl::StdNamedFrame_NormalCheck : TXUITimelineControl::StdNamedFrame_NormalCheckDisable;
		TINT iEnd   = IsEnabled() ? TXUITimelineControl::StdNamedFrame_EndNormalCheck : TXUITimelineControl::StdNamedFrame_EndNormalCheckDisable;
		m_pVisual->PlayAnimation( iStart, iEnd );
		return;
	}

	if ( a_bFromNavigation && IsEnabled() )
	{
		m_pVisual->PlayAnimation( TXUITimelineControl::StdNamedFrame_KillFocus, TXUITimelineControl::StdNamedFrame_EndNormal );
		return;
	}

	TINT iStart = IsEnabled() ? TXUITimelineControl::StdNamedFrame_Normal : TXUITimelineControl::StdNamedFrame_NormalDisable;
	m_pVisual->PlayAnimation( iStart, ( IsEnabled() == TFALSE ) * 2 + TXUITimelineControl::StdNamedFrame_EndNormal );
}

// $deBlob: FUNCTION 006cb650
TXUIScene* TXUIControl::FindParentScene()
{
	for ( T2GUIElement* pParent = m_pParent; pParent != TNULL && pParent->GetFlag( FLAGS_XUIELEMENT ); pParent = pParent->GetParent() )
	{
		TXUIElement* pElement = TSTATICCAST( TXUIElement, pParent );

		if ( pElement->IsA( &TGetClass( TXUIScene ) ) == TTRUE )
			return TSTATICCAST( TXUIScene, pElement );
	}

	return TNULL;
}

// $deBlob: FUNCTION 006cb770
void TXUIControl::SetUnfocussedInput( TBOOL a_bUnfocussedInput )
{
	if ( a_bUnfocussedInput )
		m_uiControlFlags |= ControlFlags_UnfocussedInput;
	else
		m_uiControlFlags &= ~ControlFlags_UnfocussedInput;
}

TOSHI_NAMESPACE_END
