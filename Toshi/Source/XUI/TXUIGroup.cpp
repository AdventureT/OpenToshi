#include "ToshiPCH.h"
#include "TXUIGroup.h"
#include "TXUITextPresenter.h"
#include "TXUIImagePresenter.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiGroup" type info
const TXUITypeInfo XURXUIGroupData::sm_sTypeInfo = { L"XuiGroup", &XURXUIElementData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIGroupData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d48d0
XURXUIGroupData::XURXUIGroupData()
{
	m_pClass = TClass::Find( "TXUIGroup", &TGetClass( TXUIElement ) );
}

// $deBlob: FUNCTION 006d4900
TBOOL XURXUIGroupData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	a_rData += 1;
	return TTRUE;
}

// $deBlob: FUNCTION 006d4920
TUINT32 XURXUIGroupData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	a_rData += 1;
	return uiSize;
}

// $deBlob: FUNCTION 006d4940
TBOOL XURXUIGroupData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006d4960
TBOOL XURXUIGroupData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	a_rObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006d4980
TUINT32 XURXUIGroupData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );

	return 4;
}

// $deBlob: FUNCTION 006d49a0
TBOOL XURXUIGroupData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006d49c0
TBOOL XURXUIGroupData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// NOTE: XuiGroup has no properties, it only skips its property count
// $deBlob: FUNCTION 006d49e0
TBOOL XURXUIGroupData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	a_rData += 1;
	return TTRUE;
}

// $deBlob: FUNCTION 006d48c0
const TXUITypeInfo* XURXUIGroupData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIGroup
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d49f0
// $deBlob: FUNCTION 006d4a00
// $deBlob: FUNCTION 006d4a50
// $deBlob: FUNCTION 006d4a90
// $deBlob: FUNCTION 006d4aa0
// $deBlob: FUNCTION 0053f1e0
// $deBlob: FUNCTION 008591b0
TXUI_DEFINE_CLASS( TXUIGroup );

// $deBlob: FUNCTION 006d4ab0
TXUIGroup::TXUIGroup()
    : m_oTimelineControl( this )
{
}

// $deBlob: FUNCTION 006d4ae0
// $deBlob: FUNCTION 006d4fa0
// $deBlob: FUNCTION 006d4f8b
TXUIGroup::~TXUIGroup()
{
}

// $deBlob: FUNCTION 006d4b00
TBOOL TXUIGroup::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, TTRUE );
	m_oTimelineControl.Create( a_rResource, a_pData );
	m_oTimelineControl.PlayAnimation( -1, -1, 0 );

	return TTRUE;
}

// $deBlob: FUNCTION 006d4b40
void TXUIGroup::Tick( TFLOAT a_fDeltaTime )
{
	m_oTimelineControl.Tick( a_fDeltaTime );
	T2GUIElement::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006d4ba0
void TXUIGroup::PlayAnimation( TINT a_iStartFrame, TINT a_iEndFrame )
{
	m_oTimelineControl.PlayAnimation( a_iStartFrame, a_iEndFrame, 0 );
	TXUIElement::PlayAnimation( a_iStartFrame, a_iEndFrame );
}

// $deBlob: FUNCTION 006d4b70
void TXUIGroup::PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame )
{
	m_oTimelineControl.PlayAnimation( a_wszStartFrame, a_wszEndFrame );
	TXUIElement::PlayAnimation( a_wszStartFrame, a_wszEndFrame );
}

// $deBlob: FUNCTION 006d4bd0
void TXUIGroup::PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd )
{
	m_oTimelineControl.PlayTimelineRange( a_fStart, a_fEnd );
	TXUIElement::PlayTimelineRange( a_fStart, a_fEnd );
}

// $deBlob: FUNCTION 006d4c20
void TXUIGroup::StopAnimation()
{
	m_oTimelineControl.StopAnimation();
	TXUIElement::StopAnimation();
}

// $deBlob: FUNCTION 006d4c40
void TXUIGroup::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// $deBlob: FUNCTION 006d4c70
void TXUIGroup::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006d4cb0
void TXUIGroup::SetPresenterData( const TWCHAR* a_wszText, const TWCHAR* a_wszImagePath, TUINT32 a_uiTextIndex, TUINT32 a_uiImageIndex, TINT a_iParam )
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

// $deBlob: FUNCTION 006d4da0
void TXUIGroup::SetPresenterText( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam )
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

					if ( a_wszText != TNULL && pTextPresenter->GetDataAssociation() == a_uiIndex )
					{
						pTextPresenter->SetText( a_wszText, a_iParam );
						pChild->SetVisible( TTRUE );
					}
					else
					{
						pChild->SetVisible( TFALSE );
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
// $deBlob: FUNCTION 006d4e30
void TXUIGroup::SetPresenterText2( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam )
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
					{
						pTextPresenter->SetMultiColour( a_wszText, TREINTERPRETCAST( TUINT32*, a_uiIndex ) );
						pChild->SetVisible( TTRUE );
					}
					else
					{
						pChild->SetVisible( TFALSE );
					}
				}
				else
				{
					// NOTE: retail passes 0 as the index to the children, not a_iParam
					pChild->SetPresenterText2( a_wszText, a_uiIndex, 0 );
				}
			}

			pNode = pNode->m_pNext;
		} while ( pNode != &m_Children && pNode != TNULL );
	}
}

// $deBlob: FUNCTION 006d4eb0
void TXUIGroup::SetPresenterImagePath( const TWCHAR* a_wszImagePath, TUINT32 a_uiIndex )
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

// $deBlob: FUNCTION 006d4f40
void TXUIGroup::ResolveNavigation( TXUIElement* a_pRoot )
{
	for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
	{
		T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );

		if ( pGUIElement->GetFlag( FLAGS_XUIELEMENT ) )
			TSTATICCAST( TXUIElement, pGUIElement )->ResolveNavigation( a_pRoot );
	}
}

TOSHI_NAMESPACE_END
