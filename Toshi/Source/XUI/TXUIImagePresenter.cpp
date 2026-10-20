#include "ToshiPCH.h"
#include "TXUIImagePresenter.h"
#include "TXUIImage.h"
#include "TXUI.h"
#include "Toshi/TString8.h"
#include "TXUIRenderer.h"
#include "GUI/T2GUI.h"
#include "GUI/T2GUIMaterial.h"
#include "GUI/T2GUITransform.h"
#include "Render/TTexture.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiImagePresenter" type info
const TXUITypeInfo XURXUIImagePresenterData::sm_sTypeInfo = { L"XuiImagePresenter", &XURXUIElementData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIImagePresenterData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006deb10
XURXUIImagePresenterData::XURXUIImagePresenterData()
{
	m_uiDataAssociation = 0;
	m_uiSizeMode        = 0;
	m_uiBrushFlags      = 0;

	m_pClass = TClass::Find( "TXUIImagePresenter", &TGetClass( TXUIElement ) );
}

// $deBlob: FUNCTION 006deb50
TBOOL XURXUIImagePresenterData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006deb80
TUINT32 XURXUIImagePresenterData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006debb0
TBOOL XURXUIImagePresenterData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006dec20
TBOOL XURXUIImagePresenterData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "SizeMode", -1 ) == 0 )
	{
		a_rPropIndex = Prop_SizeMode;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "DataAssoc", -1 ) == 0 )
	{
		a_rPropIndex = Prop_DataAssociation;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "BrushFlags", -1 ) == 0 )
	{
		a_rPropIndex = Prop_BrushFlags;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006decb0
TUINT32 XURXUIImagePresenterData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return 4;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ded10
TBOOL XURXUIImagePresenterData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ded70
TBOOL XURXUIImagePresenterData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006dedd0
TBOOL XURXUIImagePresenterData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_SizeMode ) )
		{
			m_uiSizeMode = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_DataAssociation ) )
		{
			m_uiDataAssociation = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_BrushFlags ) )
		{
			m_uiBrushFlags = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006deb00
const TXUITypeInfo* XURXUIImagePresenterData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIImagePresenter
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006dee90
// $deBlob: FUNCTION 006deea0
// $deBlob: FUNCTION 006deed0
// $deBlob: FUNCTION 006deef0
// $deBlob: FUNCTION 006def00
// $deBlob: FUNCTION 0053f2f0
// $deBlob: FUNCTION 00859210
TXUI_DEFINE_CLASS( TXUIImagePresenter );

// $deBlob: FUNCTION 006def10
TXUIImagePresenter::TXUIImagePresenter()
{
	m_eSizeMode         = TXUIImage::SizeMode_Tile;
	m_pMaterial         = TNULL;
	m_pTexture          = TNULL;
	m_uiDataAssociation = 0;
	m_uiBrushFlags      = 0;
	m_vCurrentSize      = TVector2::VEC_ZERO;
}

// $deBlob: FUNCTION 006def90
// $deBlob: FUNCTION 006dfb30
// $deBlob: FUNCTION 006dfb1b
TXUIImagePresenter::~TXUIImagePresenter()
{
	if ( m_pMaterial != TNULL )
	{
		m_pMaterial->OnDestroy();
		delete m_pMaterial;
	}
}

// $deBlob: FUNCTION 006defd0
void TXUIImagePresenter::SetImage( const TWCHAR* a_wszImagePath )
{
	if ( a_wszImagePath == TNULL || *a_wszImagePath == L'\0' )
		return;

	// The texture is the lower case file name, without its folders
	TString8 strTextureName( a_wszImagePath );
	TINT     iFileName = strTextureName.FindReverse( '\\', -1 ) + 1;

	if ( iFileName == 0 )
		iFileName = strTextureName.FindReverse( '/', -1 ) + 1;

	TINT iLength   = strTextureName.Length();
	TINT iCount    = iLength - iFileName;
	strTextureName = strTextureName.Mid( iLength - iCount, iCount );
	strTextureName.MakeLower();

	m_pTexture = TTextureManager::GetSingleton()->FindTexture( strTextureName.GetString() );

	if ( m_pMaterial == TNULL )
		m_pMaterial = T2GUI::CreateMaterial( m_pTexture );
	else
		m_pMaterial->SetTextureResource( m_pTexture );

	Init();
}

// $deBlob: FUNCTION 006df0c0
TBOOL TXUIImagePresenter::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, a_bCreateChildren );
	XURXUIImagePresenterData* pData = TSTATICCAST( XURXUIImagePresenterData, a_pData );

	m_eSizeMode         = pData->m_uiSizeMode;
	m_uiColour          = ( TUINT32( GetFlags_Opacity() * 255.0f ) << 24 ) | 0xEBEBEB;
	m_uiDataAssociation = pData->m_uiDataAssociation;
	m_uiBrushFlags      = pData->m_uiBrushFlags;

	return TTRUE;
}

// $deBlob: FUNCTION 006df130
void TXUIImagePresenter::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	if ( a_uiPropIndex == XURXUIImagePresenterData::Prop_SizeMode )
	{
		if ( TFLOAT( TMath::FloorToInt( a_fScalar ) ) == 0.0f )
		{
			m_eSizeMode = a_uiValue1;
			Init();
			return;
		}

		TASSERT( a_fScalar == 1.0f );
		m_eSizeMode = a_uiValue2;
		Init();
	}
	else if ( a_uiPropIndex == XURXUIImagePresenterData::Prop_DataAssociation )
	{
		if ( TFLOAT( TMath::FloorToInt( a_fScalar ) ) == 0.0f )
		{
			m_uiDataAssociation = a_uiValue1;
			return;
		}

		TASSERT( a_fScalar == 1.0f );
		m_uiDataAssociation = a_uiValue2;
	}
	else if ( a_uiPropIndex == XURXUIImagePresenterData::Prop_BrushFlags )
	{
		if ( TFLOAT( TMath::FloorToInt( a_fScalar ) ) == 0.0f )
		{
			m_uiBrushFlags = a_uiValue1;
			return;
		}

		TASSERT( a_fScalar == 1.0f );
		m_uiBrushFlags = a_uiValue2;
	}
}

// $deBlob: FUNCTION 006df340
void TXUIImagePresenter::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006df380
void TXUIImagePresenter::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_iWidth != m_vCurrentSize.m_iX || m_iHeight != m_vCurrentSize.m_iY )
	{
		Init();
		m_vCurrentSize.m_iX = m_iWidth;
		m_vCurrentSize.m_iY = m_iHeight;
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006df3e0
void TXUIImagePresenter::Render()
{
	if ( m_vScale.GetX() == 0.0f || m_vScale.GetY() == 0.0f )
		return;

	TXUIRenderer* pRenderer = TXUI::GetSingleton()->GetRenderer();

	if ( m_pMaterial != TNULL )
	{
		pRenderer->SetMaterial( m_pMaterial );
		pRenderer->SetColour( m_uiColour );

		TFLOAT fWidth  = UnpackFloat( m_iWidth );
		TFLOAT fHeight = UnpackFloat( m_iHeight );

		TVector2 vPos1( fWidth * -0.5f, fHeight * -0.5f );
		TVector2 vPos2( fWidth + vPos1.x, vPos1.y + fHeight );
		pRenderer->RenderRectangle( vPos1, vPos2, m_vUV1, m_vUV2 );
	}
}

// NOTE: same as TXUIImage::Init without the update of m_vCurrentSize
// $deBlob: FUNCTION 006df4d0
void TXUIImagePresenter::Init()
{
	if ( m_pTexture == TNULL )
		return;

	switch ( m_eSizeMode )
	{
		case TXUIImage::SizeMode_Tile:
		{
			m_vUV1.x = 0.0f;
			m_vUV1.y = 0.0f;

			TUINT16 uiTextureWidth  = m_pTexture->GetInfo()->Width2;
			TFLOAT  fScaleX         = GetWidth() / TFLOAT( uiTextureWidth );
			TUINT16 uiTextureHeight = m_pTexture->GetInfo()->Height2;
			m_vUV2.x                = fScaleX;
			m_vUV2.y                = GetHeight() / TFLOAT( uiTextureHeight );
			return;
		}
		case TXUIImage::SizeMode_TextureSize:
		{
			TTexture::Info* pInfo = m_pTexture->GetInfo();
			m_iWidth              = Float( TINT( TFLOAT( pInfo->Width2 ) * 6.0f ) );
			m_iHeight             = Float( TINT( TFLOAT( pInfo->Height2 ) * 6.0f ) );
			TFALLTHROUGH;
		}
		default:
			m_vUV1.x = 0.0f;
			m_vUV1.y = 0.0f;
			m_vUV2.y = 1.0f;
			break;
		case TXUIImage::SizeMode_Centre:
		{
			static TVector2 s_vHalf( 0.5f, 0.5f );

			m_vUV1.x = s_vHalf.x * -1.0f;
			m_vUV1.y = s_vHalf.y * -1.0f;
			m_vUV2   = s_vHalf;

			TUINT16 uiTextureWidth  = m_pTexture->GetInfo()->Width2;
			TFLOAT  fScaleX         = GetWidth() / TFLOAT( uiTextureWidth );
			TUINT16 uiTextureHeight = m_pTexture->GetInfo()->Height2;
			TFLOAT  fScaleY         = GetHeight() / TFLOAT( uiTextureHeight );

			T2GUITransform oTransform;
			oTransform.PreMultiply( fScaleX, fScaleY );

			TVector2 vUV;
			oTransform.Transform( vUV, m_vUV1 );
			m_vUV1 = vUV + s_vHalf;
			oTransform.Transform( vUV, m_vUV2 );
			m_vUV2 = vUV + s_vHalf;
			return;
		}
		case TXUIImage::SizeMode_KeepAspect:
		{
			m_vUV1.x = 0.0f;
			m_vUV1.y = 0.0f;

			TFLOAT  fWidth          = GetWidth();
			TFLOAT  fAspect         = fWidth / GetHeight();
			TUINT16 uiTextureWidth  = m_pTexture->GetInfo()->Width2;
			TUINT16 uiTextureHeight = m_pTexture->GetInfo()->Height2;
			TFLOAT  fScale          = ( GetHeight() * TFLOAT( uiTextureWidth ) ) / ( GetWidth() * TFLOAT( uiTextureHeight ) );

			if ( TFLOAT( uiTextureWidth ) / TFLOAT( uiTextureHeight ) < fAspect )
			{
				m_vUV2.y = 1.0f;
				m_vUV2.x = 1.0f / fScale;
				return;
			}

			m_vUV2.y = fScale;
			break;
		}
		case TXUIImage::SizeMode_CentreCrop:
		{
			static TVector2 s_vHalf( 0.5f, 0.5f );

			m_vUV1.x = s_vHalf.x * -1.0f;
			m_vUV1.y = s_vHalf.y * -1.0f;
			m_vUV2   = s_vHalf;

			T2GUITransform oTransform;

			TFLOAT fWidth         = GetWidth();
			TFLOAT fAspect        = fWidth / GetHeight();
			TFLOAT fTextureAspect = TFLOAT( m_pTexture->GetInfo()->Width2 ) / TFLOAT( m_pTexture->GetInfo()->Height2 );
			TFLOAT fScaleY        = fTextureAspect / fAspect;
			TFLOAT fScaleX;

			if ( fTextureAspect < fAspect )
			{
				fScaleX = 1.0f / fScaleY;
				fScaleY = 1.0f;
			}
			else
			{
				fScaleX = 1.0f;
			}

			oTransform.PreMultiply( fScaleX, fScaleY );

			TVector2 vUV;
			oTransform.Transform( vUV, m_vUV1 );
			m_vUV1 = vUV + s_vHalf;
			oTransform.Transform( vUV, m_vUV2 );
			m_vUV2 = vUV + s_vHalf;
			return;
		}
	}

	m_vUV2.x = 1.0f;
}

TOSHI_NAMESPACE_END
