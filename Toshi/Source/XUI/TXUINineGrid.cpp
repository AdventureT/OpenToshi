#include "ToshiPCH.h"
#include "TXUINineGrid.h"
#include "TXUI.h"
#include "Toshi/TString8.h"
#include "TXUIRenderer.h"
#include "GUI/T2GUI.h"
#include "GUI/T2GUIMaterial.h"
#include "Render/TTexture.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiNineGrid" type info
const TXUITypeInfo XURXUINineGridData::sm_sTypeInfo = { L"XuiNineGrid", &XURXUIElementData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUINineGridData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e0a10
XURXUINineGridData::XURXUINineGridData()
{
	m_uiTextureFileName = 0;
	m_bNoCenter         = TFALSE;

	m_pClass = TClass::Find( "TXUINineGrid", &TGetClass( TXUIElement ) );

	m_uiLeftOffset   = 0;
	m_uiTopOffset    = 0;
	m_uiRightOffset  = 0;
	m_uiBottomOffset = 0;
}

// $deBlob: FUNCTION 006e0a60
TBOOL XURXUINineGridData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e0a90
TUINT32 XURXUINineGridData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// NOTE: retail accepts the property indices up to 15, XuiNineGrid only has 6 (Prop_NUMOF)
// $deBlob: FUNCTION 006e0ac0
TBOOL XURXUINineGridData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < 15 ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e0b30
TBOOL XURXUINineGridData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "TextureFileName", -1 ) == 0 )
	{
		a_rPropIndex = Prop_TextureFileName;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "LeftOffset", -1 ) == 0 )
	{
		a_rPropIndex = Prop_LeftOffset;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "TopOffset", -1 ) == 0 )
	{
		a_rPropIndex = Prop_TopOffset;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "RightOffset", -1 ) == 0 )
	{
		a_rPropIndex = Prop_RightOffset;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "BottomOffset", -1 ) == 0 )
	{
		a_rPropIndex = Prop_BottomOffset;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "NoCenter", -1 ) == 0 )
	{
		a_rPropIndex = Prop_NoCenter;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e0c20
TUINT32 XURXUINineGridData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_TextureFileName )
			return 2;

		TUINT32 uiSize = 4;

		if ( a_uiPropIndex == Prop_NoCenter )
			uiSize = 1;

		return uiSize;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e0ca0
TBOOL XURXUINineGridData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006e0cc0
TBOOL XURXUINineGridData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006e0ce0
TBOOL XURXUINineGridData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_TextureFileName ) )
		{
			m_uiTextureFileName = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_LeftOffset ) )
		{
			m_uiLeftOffset = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_TopOffset ) )
		{
			m_uiTopOffset = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_RightOffset ) )
		{
			m_uiRightOffset = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_BottomOffset ) )
		{
			m_uiBottomOffset = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_NoCenter ) )
		{
			m_bNoCenter = *a_rData;
			a_rData += 1;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e0a00
const TXUITypeInfo* XURXUINineGridData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUINineGrid
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e0df0
// $deBlob: FUNCTION 006e0e00
// $deBlob: FUNCTION 006e0e30
// $deBlob: FUNCTION 006e0e50
// $deBlob: FUNCTION 006e0e60
// $deBlob: FUNCTION 0053f390
// $deBlob: FUNCTION 00859250
TXUI_DEFINE_CLASS( TXUINineGrid );

// $deBlob: FUNCTION 006e0e70
TXUINineGrid::TXUINineGrid()
{
	m_pMaterial     = TNULL;
	m_bNoCenter     = TFALSE;
	m_fLeftOffset   = 0.0f;
	m_fTopOffset    = 0.0f;
	m_fRightOffset  = 0.0f;
	m_fBottomOffset = 0.0f;
}

// $deBlob: FUNCTION 006e0ed0
// $deBlob: FUNCTION 006e1700
// $deBlob: FUNCTION 006e16eb
TXUINineGrid::~TXUINineGrid()
{
	if ( m_pMaterial != TNULL )
	{
		m_pMaterial->OnDestroy();
		delete m_pMaterial;
	}
}

// $deBlob: FUNCTION 006e0f10
TBOOL TXUINineGrid::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, TTRUE );
	XURXUINineGridData* pData = TSTATICCAST( XURXUINineGridData, a_pData );

	m_fLeftOffset   = TFLOAT( pData->m_uiLeftOffset );
	m_fTopOffset    = TFLOAT( pData->m_uiTopOffset );
	m_fRightOffset  = TFLOAT( pData->m_uiRightOffset );
	m_fBottomOffset = TFLOAT( pData->m_uiBottomOffset );

	// The texture is the lower case file name, without its folders
	TString8 strTextureName( a_rResource.GetString( pData->m_uiTextureFileName ) );
	TINT     iSlash  = strTextureName.FindReverse( '\\', -1 );
	TINT     iLength = strTextureName.Length();
	TINT     iCount  = iLength - ( iSlash + 1 );
	strTextureName   = strTextureName.Mid( iLength - iCount, iCount );
	strTextureName.MakeLower();

	TTexture* pTexture = TTextureManager::GetSingleton()->FindTexture( strTextureName.GetString() );
	m_pMaterial        = T2GUI::CreateMaterial( pTexture );

	TTexture::Info* pInfo  = pTexture->GetInfo();
	TFLOAT          fLeft  = m_fLeftOffset / TFLOAT( pInfo->Width );
	TFLOAT          fRight = 1.0f - m_fRightOffset / TFLOAT( pInfo->Width );
	TFLOAT          fTop   = m_fTopOffset / TFLOAT( pInfo->Height );
	TFLOAT          fBottom = 1.0f - m_fBottomOffset / TFLOAT( pInfo->Height );

	m_aCells[ Cell_TopLeft ].vUV1.Set( 0.0f, 0.0f );
	m_aCells[ Cell_TopLeft ].vUV2.Set( fLeft, fTop );
	m_aCells[ Cell_Left ].vUV1.Set( 0.0f, fTop );
	m_aCells[ Cell_Left ].vUV2.Set( fLeft, fBottom );
	m_aCells[ Cell_BottomLeft ].vUV1.Set( 0.0f, fBottom );
	m_aCells[ Cell_BottomLeft ].vUV2.Set( fLeft, 1.0f );

	m_aCells[ Cell_Top ].vUV1.Set( fLeft, 0.0f );
	m_aCells[ Cell_Top ].vUV2.Set( fRight, fTop );
	m_aCells[ Cell_Center ].vUV1.Set( fLeft, fTop );
	m_aCells[ Cell_Center ].vUV2.Set( fRight, fBottom );
	m_aCells[ Cell_Bottom ].vUV1.Set( fLeft, fBottom );
	m_aCells[ Cell_Bottom ].vUV2.Set( fRight, 1.0f );

	m_aCells[ Cell_TopRight ].vUV1.Set( fRight, 0.0f );
	m_aCells[ Cell_TopRight ].vUV2.Set( 1.0f, fTop );
	m_aCells[ Cell_Right ].vUV1.Set( fRight, fTop );
	m_aCells[ Cell_Right ].vUV2.Set( 1.0f, fBottom );
	m_aCells[ Cell_BottomRight ].vUV1.Set( fRight, fBottom );
	m_aCells[ Cell_BottomRight ].vUV2.Set( 1.0f, 1.0f );

	SetWidth( GetWidth() );
	SetHeight( GetHeight() );

	m_bNoCenter = pData->m_bNoCenter != 0;

	return TTRUE;
}

// $deBlob: FUNCTION 006e12f0
void TXUINineGrid::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// $deBlob: FUNCTION 006e1320
void TXUINineGrid::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006e1360
void TXUINineGrid::Render()
{
	TXUIRenderer* pRenderer = TXUI::GetSingleton()->GetRenderer();
	pRenderer->SetMaterial( m_pMaterial );
	pRenderer->SetColour( m_uiColour );

	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );

	// The sizes of the columns and rows, the borders keep the size of the texture's borders
	TFLOAT fMiddleWidth  = GetWidth() - m_fLeftOffset - m_fRightOffset;
	TFLOAT fMiddleHeight = GetHeight() - m_fTopOffset - m_fBottomOffset;

	TFLOAT fX0 = fWidth * -0.5f;
	TFLOAT fX1 = fX0 + m_fLeftOffset;
	TFLOAT fX2 = fX1 + fMiddleWidth;
	TFLOAT fX3 = fX2 + m_fRightOffset;
	TFLOAT fY0 = fHeight * -0.5f;
	TFLOAT fY1 = fY0 + m_fTopOffset;
	TFLOAT fY2 = fY1 + fMiddleHeight;
	TFLOAT fY3 = fY2 + m_fBottomOffset;

	// Left column
	pRenderer->RenderRectangle( TVector2( fX0, fY0 ), TVector2( fX1, fY1 ), m_aCells[ Cell_TopLeft ].vUV1, m_aCells[ Cell_TopLeft ].vUV2 );
	pRenderer->RenderRectangle( TVector2( fX0, fY1 ), TVector2( fX1, fY2 ), m_aCells[ Cell_Left ].vUV1, m_aCells[ Cell_Left ].vUV2 );
	pRenderer->RenderRectangle( TVector2( fX0, fY2 ), TVector2( fX1, fY3 ), m_aCells[ Cell_BottomLeft ].vUV1, m_aCells[ Cell_BottomLeft ].vUV2 );

	// Middle column
	pRenderer->RenderRectangle( TVector2( fX1, fY0 ), TVector2( fX2, fY1 ), m_aCells[ Cell_Top ].vUV1, m_aCells[ Cell_Top ].vUV2 );

	if ( !m_bNoCenter )
		pRenderer->RenderRectangle( TVector2( fX1, fY1 ), TVector2( fX2, fY2 ), m_aCells[ Cell_Center ].vUV1, m_aCells[ Cell_Center ].vUV2 );

	pRenderer->RenderRectangle( TVector2( fX1, fY2 ), TVector2( fX2, fY3 ), m_aCells[ Cell_Bottom ].vUV1, m_aCells[ Cell_Bottom ].vUV2 );

	// Right column
	pRenderer->RenderRectangle( TVector2( fX2, fY0 ), TVector2( fX3, fY1 ), m_aCells[ Cell_TopRight ].vUV1, m_aCells[ Cell_TopRight ].vUV2 );
	pRenderer->RenderRectangle( TVector2( fX2, fY1 ), TVector2( fX3, fY2 ), m_aCells[ Cell_Right ].vUV1, m_aCells[ Cell_Right ].vUV2 );
	pRenderer->RenderRectangle( TVector2( fX2, fY2 ), TVector2( fX3, fY3 ), m_aCells[ Cell_BottomRight ].vUV1, m_aCells[ Cell_BottomRight ].vUV2 );

	T2GUIElement::Render();
}

TOSHI_NAMESPACE_END
