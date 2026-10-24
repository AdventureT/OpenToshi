#include "ToshiPCH.h"
#include "TXUIVisual.h"
#include "TXUITextPresenter.h"
#include "TXUIImagePresenter.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiVisual" type info
const TXUITypeInfo XURXUIVisualData::sm_sTypeInfo = { L"XuiVisual", &XURXUIElementData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIVisualData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e17a0
XURXUIVisualData::XURXUIVisualData()
{
	m_pClass = TClass::Find( "TXUIVisual", &TGetClass( TXUIElement ) );
}

// $deBlob: FUNCTION 006e17d0
TBOOL XURXUIVisualData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	a_rData += 1;
	return TTRUE;
}

// $deBlob: FUNCTION 006e17f0
TUINT32 XURXUIVisualData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	a_rData += 1;
	return uiSize;
}

// $deBlob: FUNCTION 006e1810
TBOOL XURXUIVisualData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006e1830
TBOOL XURXUIVisualData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	a_rObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e1850
TUINT32 XURXUIVisualData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );

	return 4;
}

// $deBlob: FUNCTION 006e1870
TBOOL XURXUIVisualData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006e1890
TBOOL XURXUIVisualData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// NOTE: XuiVisual has no properties, it only skips its property count
// $deBlob: FUNCTION 006e18b0
TBOOL XURXUIVisualData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	a_rData += 1;
	return TTRUE;
}

// $deBlob: FUNCTION 006e1790
const TXUITypeInfo* XURXUIVisualData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIVisual
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e18c0
// $deBlob: FUNCTION 006e18d0
// $deBlob: FUNCTION 006e1920
// $deBlob: FUNCTION 006e1960
// $deBlob: FUNCTION 006e1970
// $deBlob: FUNCTION 0053f3e0
// $deBlob: FUNCTION 00859270
TXUI_DEFINE_CLASS( TXUIVisual );

// $deBlob: FUNCTION 006e1980
TXUIVisual::TXUIVisual()
    : m_oTimelineControl( this )
{
}

// $deBlob: FUNCTION 006e19b0
// $deBlob: FUNCTION 006e1e30
// $deBlob: FUNCTION 006e1e1b
TXUIVisual::~TXUIVisual()
{
}

// $deBlob: FUNCTION 006e19d0
TBOOL TXUIVisual::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, TTRUE );
	m_oTimelineControl.Create( a_rResource, a_pData );
	m_oTimelineControl.PlayAnimation( -1, -1, 0 );

	return TTRUE;
}

// $deBlob: FUNCTION 006e1a10
void TXUIVisual::Tick( TFLOAT a_fDeltaTime )
{
	m_oTimelineControl.Tick( a_fDeltaTime );
	T2GUIElement::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006e1a40
void TXUIVisual::SetPresenterData( const TWCHAR* a_wszText, const TWCHAR* a_wszImagePath, TUINT32 a_uiTextIndex, TUINT32 a_uiImageIndex, TINT a_iParam )
{
	T2GUIElementNode* pNode = m_Children.m_pNext;

	if ( pNode != &m_Children && pNode != TNULL )
	{
		do
		{
			T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );

			if ( pGUIElement->GetFlag( FLAGS_XUIELEMENT ) )
			{
				TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

				if ( pChild->IsA( &TGetClass( TXUITextPresenter ) ) )
				{
					TXUITextPresenter* pTextPresenter = TSTATICCAST( TXUITextPresenter, pChild );

					if ( a_wszText != TNULL && pTextPresenter->GetDataAssociation() == a_uiTextIndex )
					{
						pTextPresenter->SetText( a_wszText, a_iParam );
						pChild->SetVisible( TTRUE );
					}
					else
					{
						pChild->SetVisible( TFALSE );
					}
				}
				else if ( pChild->IsA( &TGetClass( TXUIImagePresenter ) ) )
				{
					TXUIImagePresenter* pImagePresenter = TSTATICCAST( TXUIImagePresenter, pChild );

					if ( a_wszImagePath != TNULL && pImagePresenter->GetDataAssociation() == a_uiImageIndex )
					{
						pImagePresenter->SetImage( a_wszImagePath );
						pChild->SetVisible( TTRUE );
					}
					else
					{
						pChild->SetVisible( TFALSE );
					}
				}
				else
				{
					pChild->SetPresenterData( a_wszText, a_wszImagePath, a_uiTextIndex, a_uiImageIndex, a_iParam );
				}
			}

			pNode = pNode->m_pNext;
		} while ( pNode != &m_Children && pNode != TNULL );
	}
}

// $deBlob: FUNCTION 006e1b30
void TXUIVisual::SetPresenterText( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam )
{
	T2GUIElementNode* pNode = m_Children.m_pNext;

	if ( pNode != &m_Children && pNode != TNULL )
	{
		do
		{
			T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );

			if ( pGUIElement->GetFlag( FLAGS_XUIELEMENT ) )
			{
				TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

				if ( pChild->IsA( &TGetClass( TXUITextPresenter ) ) )
				{
					TXUITextPresenter* pTextPresenter = TSTATICCAST( TXUITextPresenter, pChild );

					// NOTE: unlike TXUIGroup, the visual doesn't hide the presenters that don't match
					if ( a_wszText != TNULL && pTextPresenter->GetDataAssociation() == a_uiIndex )
					{
						pTextPresenter->SetText( a_wszText, a_iParam );
						pChild->SetVisible( TTRUE );
					}
				}
				else
				{
					pChild->SetPresenterText( a_wszText, a_uiIndex, a_iParam );
				}
			}

			pNode = pNode->m_pNext;
		} while ( pNode != &m_Children && pNode != TNULL );
	}
}

// NOTE: the parameters of +0x8C are (0x6C value, colours, data association index) for the text presenters, see
// TXUITextPresenter::SetMultiColour (the core declares them as text, index, param)
// $deBlob: FUNCTION 006e1c40
void TXUIVisual::SetPresenterText2( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam )
{
	T2GUIElementNode* pNode = m_Children.m_pNext;

	if ( pNode != &m_Children && pNode != TNULL )
	{
		do
		{
			T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );

			if ( pGUIElement->GetFlag( FLAGS_XUIELEMENT ) )
			{
				TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

				if ( pChild->IsA( &TGetClass( TXUITextPresenter ) ) )
				{
					TXUITextPresenter* pTextPresenter = TSTATICCAST( TXUITextPresenter, pChild );

					if ( pTextPresenter->GetDataAssociation() == TUINT32( a_iParam ) )
						pTextPresenter->SetMultiColour( a_wszText, TREINTERPRETCAST( TUINT32*, a_uiIndex ) );
				}
				else
				{
					pChild->SetPresenterText2( a_wszText, a_uiIndex, a_iParam );
				}
			}

			pNode = pNode->m_pNext;
		} while ( pNode != &m_Children && pNode != TNULL );
	}
}

// $deBlob: FUNCTION 006e1bb0
void TXUIVisual::SetPresenterImagePath( const TWCHAR* a_wszImagePath, TUINT32 a_uiIndex )
{
	T2GUIElementNode* pNode = m_Children.m_pNext;

	if ( pNode != &m_Children && pNode != TNULL )
	{
		do
		{
			T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );

			if ( pGUIElement->GetFlag( FLAGS_XUIELEMENT ) )
			{
				TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

				if ( pChild->IsA( &TGetClass( TXUIImagePresenter ) ) )
				{
					TXUIImagePresenter* pImagePresenter = TSTATICCAST( TXUIImagePresenter, pChild );

					if ( a_wszImagePath != TNULL && pImagePresenter->GetDataAssociation() == a_uiIndex )
					{
						pImagePresenter->SetImage( a_wszImagePath );
						pChild->SetVisible( TTRUE );
					}
					else
					{
						pChild->SetVisible( TFALSE );
					}
				}
				else
				{
					pChild->SetPresenterImagePath( a_wszImagePath, a_uiIndex );
				}
			}

			pNode = pNode->m_pNext;
		} while ( pNode != &m_Children && pNode != TNULL );
	}
}

// $deBlob: FUNCTION 006e1ce0
void TXUIVisual::PlayAnimation( TINT a_iStartFrame, TINT a_iEndFrame )
{
	m_oTimelineControl.PlayAnimation( a_iStartFrame, a_iEndFrame, 0 );
	TXUIElement::PlayAnimation( a_iStartFrame, a_iEndFrame );
}

// $deBlob: FUNCTION 006e1cb0
void TXUIVisual::PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame )
{
	m_oTimelineControl.PlayAnimation( a_wszStartFrame, a_wszEndFrame );
	TXUIElement::PlayAnimation( a_wszStartFrame, a_wszEndFrame );
}

// $deBlob: FUNCTION 006e1d10
void TXUIVisual::StopAnimation()
{
	m_oTimelineControl.StopAnimation();
	TXUIElement::StopAnimation();
}

// NOTE: retail reads the frame time (a TUINT16 at +0x14) from its own named frame; the core's NamedFrame keeps the
// XUR data, which holds the same time
// $deBlob: FUNCTION 006e1d30
TFLOAT TXUIVisual::GetTimelineTime( const TWCHAR* a_wszNamedFrame )
{
	TXUITimelineControl::NamedFrame* pNamedFrame = m_oTimelineControl.FindNamedFrame( a_wszNamedFrame );

	if ( pNamedFrame == TNULL )
		return -1.0f;

	return TFLOAT( pNamedFrame->m_uiTime ) * ( 1.0f / 60.0f );
}

// $deBlob: FUNCTION 006e1d70
void TXUIVisual::SetTimelineTime( TFLOAT a_fTime )
{
	if ( a_fTime != m_oTimelineControl.GetTimeLineTime() )
		m_oTimelineControl.SetTimeLineTime( a_fTime, TFALSE );
}

// $deBlob: FUNCTION 006e1da0
void TXUIVisual::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// $deBlob: FUNCTION 006e1dd0
void TXUIVisual::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

TOSHI_NAMESPACE_END
