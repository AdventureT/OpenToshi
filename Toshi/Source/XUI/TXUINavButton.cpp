#include "ToshiPCH.h"
#include "TXUINavButton.h"
#include "TXUI.h"
#include "TXUIScene.h"

#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiNavButton" type info
const TXUITypeInfo XURXUINavButtonData::sm_sTypeInfo = { L"XuiNavButton", &XURXUIButtonData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUINavButtonData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e39a0
XURXUINavButtonData::XURXUINavButtonData()
{
	m_iSrcTransIndex  = 0;
	m_iDestTransIndex = 0;
	m_uiPressPath     = 0;
	m_bStayVisible    = TFALSE;
	m_pClass          = TClass::Find( "TXUINavButton", &TGetClass( TXUIButton ) );
}

// $deBlob: FUNCTION 006e39e0
TBOOL XURXUINavButtonData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIButtonData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e3a10
TUINT32 XURXUINavButtonData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIButtonData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006e3a40
TBOOL XURXUINavButtonData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIButtonData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e3aa0
TBOOL XURXUINavButtonData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "PressPath", -1 ) == 0 )
	{
		a_rPropIndex = Prop_PressPath;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "StayVisible", -1 ) == 0 )
	{
		a_rPropIndex = Prop_StayVisible;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "SrcTransIndex", -1 ) == 0 )
	{
		a_rPropIndex = Prop_SrcTransIndex;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "DestTransIndex", -1 ) == 0 )
	{
		a_rPropIndex = Prop_DestTransIndex;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIButtonData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e3b50
TUINT32 XURXUINavButtonData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_PressPath )
			return 2;

		return ( a_uiPropIndex == Prop_StayVisible ) ? 1 : 4;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIButtonData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e3bd0
TBOOL XURXUINavButtonData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIButtonData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e3c30
TBOOL XURXUINavButtonData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIButtonData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e3c90
TBOOL XURXUINavButtonData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_PressPath ) )
		{
			m_uiPressPath = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_StayVisible ) )
		{
			m_bStayVisible = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_SrcTransIndex ) )
		{
			m_iSrcTransIndex = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_DestTransIndex ) )
		{
			m_iDestTransIndex = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e3990
const TXUITypeInfo* XURXUINavButtonData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUINavButton
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e3d50
// $deBlob: FUNCTION 006e3d60
// $deBlob: FUNCTION 006e3dc0
// $deBlob: FUNCTION 006e3e20
// $deBlob: FUNCTION 006e3e30
// $deBlob: FUNCTION 0053f570
// $deBlob: FUNCTION 00859310
TXUI_DEFINE_CLASS( TXUINavButton );

// $deBlob: FUNCTION 006e3e40
TXUINavButton::TXUINavButton()
{
	m_wszPressPath    = TNULL;
	m_uiUnknown9C     = 0;
	m_iSrcTransIndex  = 0;
	m_iDestTransIndex = 0;
}

// $deBlob: FUNCTION 006e3e90
// $deBlob: FUNCTION 006e4230
// $deBlob: FUNCTION 006e421b
TXUINavButton::~TXUINavButton()
{
}

// $deBlob: FUNCTION 006e3eb0
TBOOL TXUINavButton::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIButton::Create( a_rResource, a_pData, TTRUE );

	XURXUINavButtonData* pData = TSTATICCAST( XURXUINavButtonData, a_pData );

	m_wszPressPath    = a_rResource.GetString( pData->m_uiPressPath );
	m_iSrcTransIndex  = pData->m_iSrcTransIndex;
	m_iDestTransIndex = pData->m_iDestTransIndex;

	return TTRUE;
}

// $deBlob: FUNCTION 006e3f00
void TXUINavButton::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIButton::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	// NOTE: PressPath and StayVisible aren't animated
	if ( a_uiPropIndex == XURXUINavButtonData::Prop_SrcTransIndex )
	{
		a_fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

		if ( a_fScalar == 0.0f )
		{
			m_iSrcTransIndex = a_uiValue1;
			return;
		}

		TASSERT( a_fScalar == 1.0f );
		m_iSrcTransIndex = a_uiValue2;
	}
	else if ( a_uiPropIndex == XURXUINavButtonData::Prop_DestTransIndex )
	{
		a_fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

		if ( a_fScalar == 0.0f )
		{
			m_iDestTransIndex = a_uiValue1;
			return;
		}

		TASSERT( a_fScalar == 1.0f );
		m_iDestTransIndex = a_uiValue2;
	}
}

// $deBlob: FUNCTION 006e4080
void TXUINavButton::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIButton::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006e40c0
TBOOL TXUINavButton::EndPress()
{
	TBOOL bResult = TXUIButton::EndPress();

	if ( m_wszPressPath != TNULL )
	{
		TString8 strPath( m_wszPressPath );
		strPath.MakeLower();

		TXUIResource* pResource = TXUI::GetSingleton()->FindResource( strPath.GetString() );

		if ( pResource != TNULL )
		{
			TXUIScene* pNewScene = TXUI::GetSingleton()->CreateScene( pResource, 0 );

			if ( pNewScene != TNULL )
			{
				SetFocus( TFALSE, TFALSE, TFALSE );
				pNewScene->SetVisible( TFALSE );

				TXUIScene* pScene       = FindParentScene();
				TXUIScene* pParentScene = pScene->FindParentScene();

				// Put the new scene next to the current one
				if ( pParentScene != TNULL )
				{
					pNewScene->RemoveSelf();
					pParentScene->AddChildTail( pNewScene );
				}

				pScene->SetTransitionIndex( m_iSrcTransIndex );
				pNewScene->SetTransitionIndex( m_iDestTransIndex );
				TXUI::GetSingleton()->TransitionTo( pScene, pNewScene );
			}
		}
	}

	return bResult;
}

// $deBlob: FUNCTION 006e41f0
void TXUINavButton::PlayAnimation( TINT a_iStartFrame, TINT a_iEndFrame )
{
	TXUIElement::PlayAnimation( a_iStartFrame, a_iEndFrame );
}

// $deBlob: FUNCTION 006e4200
void TXUINavButton::StopAnimation()
{
	TXUIElement::StopAnimation();
}

TOSHI_NAMESPACE_END
