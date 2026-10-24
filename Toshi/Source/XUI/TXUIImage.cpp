#include "ToshiPCH.h"
#include "TXUIImage.h"
#include "TXUI.h"
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

// "XuiImage" type info
const TXUITypeInfo XURXUIImageData::sm_sTypeInfo = { L"XuiImage", &XURXUIElementData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIImageData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006dd890
XURXUIImageData::XURXUIImageData()
{
	m_uiImagePath  = 0;
	m_uiSizeMode   = 0;
	m_uiBrushFlags = 0;

	m_pClass = TClass::Find( "TXUIImage", &TGetClass( TXUIElement ) );
}

// $deBlob: FUNCTION 006dd8d0
TBOOL XURXUIImageData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006dd900
TUINT32 XURXUIImageData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006dd930
TBOOL XURXUIImageData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006dd9a0
TBOOL XURXUIImageData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "SizeMode", -1 ) == 0 )
	{
		a_rPropIndex = Prop_SizeMode;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "ImagePath", -1 ) == 0 )
	{
		a_rPropIndex = Prop_ImagePath;
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

// $deBlob: FUNCTION 006dda30
TUINT32 XURXUIImageData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex != Prop_ImagePath ) ? 4 : 2;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ddaa0
TBOOL XURXUIImageData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ddb00
TBOOL XURXUIImageData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ddb60
TBOOL XURXUIImageData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_ImagePath ) )
		{
			m_uiImagePath = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_BrushFlags ) )
		{
			m_uiBrushFlags = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006dd880
const TXUITypeInfo* XURXUIImageData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIImage
//-----------------------------------------------------------------------------

// Lower case file name (without its folders) of an image path, the name of its texture
// $deBlob: FUNCTION 006de970
static void __stdcall GetTextureName( TCHAR* a_szBuffer, TINT a_iBufferSize, const TWCHAR* a_wszImagePath )
{
	const TWCHAR* wszFileName = T2String16::FindLastChar( a_wszImagePath, L'\\' );

	if ( wszFileName != TNULL || ( wszFileName = T2String16::FindLastChar( a_wszImagePath, L'/' ) ) != TNULL )
		a_wszImagePath = wszFileName + 1;

	T2String16::CopyToChar( a_szBuffer, a_wszImagePath, a_iBufferSize - 1 );
	T2String8::ToLowerCase( a_szBuffer );
}

// $deBlob: FUNCTION 006ddc10
// $deBlob: FUNCTION 006ddc20
// $deBlob: FUNCTION 006ddc50
// $deBlob: FUNCTION 006ddc70
// $deBlob: FUNCTION 006ddc80
// $deBlob: FUNCTION 0053f2a0
// $deBlob: FUNCTION 008591f0
TXUI_DEFINE_CLASS( TXUIImage );

// $deBlob: FUNCTION 006ddc90
TXUIImage::TXUIImage()
{
	m_eSizeMode     = SizeMode_Tile;
	m_pMaterial     = TNULL;
	m_bOwnsMaterial = TTRUE;
	m_wszImagePath  = TNULL;
	m_pTexture      = TNULL;
	m_uiBrushFlags  = 0;
	m_vCurrentSize  = TVector2::VEC_ZERO;
}

// $deBlob: FUNCTION 006ddd10
// $deBlob: FUNCTION 006de9f0
// $deBlob: FUNCTION 006de9e4
TXUIImage::~TXUIImage()
{
	if ( m_pMaterial != TNULL && m_bOwnsMaterial == TTRUE )
	{
		m_pMaterial->OnDestroy();
		delete m_pMaterial;
	}

	m_pTexture = TNULL;
}

// $deBlob: FUNCTION 006ddd60
TBOOL TXUIImage::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, a_bCreateChildren );
	XURXUIImageData* pData = TSTATICCAST( XURXUIImageData, a_pData );

	TTexture* pTexture;

	if ( pData->m_uiImagePath != 0 )
	{
		m_wszImagePath = a_rResource.GetString( pData->m_uiImagePath );

		TCHAR szTextureName[ 256 ];
		GetTextureName( szTextureName, sizeof( szTextureName ), m_wszImagePath );

		pTexture   = TTextureManager::GetSingleton()->FindTexture( szTextureName );
		m_pTexture = pTexture;
	}
	else
	{
		// NOTE: m_pTexture stays TNULL, Init doesn't compute the UVs
		pTexture = TTextureManager::GetSingleton()->GetInvalidTexture();
	}

	m_pMaterial = T2GUI::CreateMaterial( pTexture );
	m_uiColour  = ( TUINT32( GetFlags_Opacity() * 255.0f ) << 24 ) | 0xEBEBEB;

	m_eSizeMode    = pData->m_uiSizeMode;
	m_uiBrushFlags = pData->m_uiBrushFlags;
	Init();

	return TTRUE;
}

// $deBlob: FUNCTION 006dde70
void TXUIImage::SetImagePath( const TWCHAR* a_wszImagePath )
{
	if ( m_wszImagePath == a_wszImagePath )
		return;

	m_wszImagePath = a_wszImagePath;

	TCHAR szTextureName[ 256 ];
	GetTextureName( szTextureName, sizeof( szTextureName ), a_wszImagePath );

	m_pTexture = TTextureManager::GetSingleton()->FindTexture( szTextureName );
	m_pMaterial->SetTextureResource( m_pTexture );
	Init();
}

// $deBlob: FUNCTION 006ddf10
void TXUIImage::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	if ( a_uiPropIndex == XURXUIImageData::Prop_SizeMode )
	{
		if ( TFLOAT( TMath::FloorToInt( a_fScalar ) ) != 0.0f )
		{
			TASSERT( a_fScalar == 1.0f );
			m_eSizeMode = a_uiValue2;
			Init();
			return;
		}

		m_eSizeMode = a_uiValue1;
		Init();
	}
	else if ( a_uiPropIndex == XURXUIImageData::Prop_ImagePath )
	{
		if ( TFLOAT( TMath::FloorToInt( a_fScalar ) ) != 0.0f )
		{
			a_uiValue1 = a_uiValue2;
			TASSERT( a_fScalar == 1.0f );
		}

		SetImagePath( a_pResource->GetString( TUINT16( a_uiValue1 ) ) );
	}
	else if ( a_uiPropIndex == XURXUIImageData::Prop_BrushFlags )
	{
		if ( TFLOAT( TMath::FloorToInt( a_fScalar ) ) != 0.0f )
		{
			TASSERT( a_fScalar == 1.0f );
			m_uiBrushFlags = a_uiValue2;
			return;
		}

		m_uiBrushFlags = a_uiValue1;
	}
}

// $deBlob: FUNCTION 006de120
void TXUIImage::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006de160
void TXUIImage::SetUVs( const TVector2& a_rUV1, const TVector2& a_rUV2 )
{
	m_vUV1 = a_rUV1;
	m_vUV2 = a_rUV2;
}

// $deBlob: FUNCTION 006de190
void TXUIImage::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_iWidth != m_vCurrentSize.m_iX || m_iHeight != m_vCurrentSize.m_iY )
	{
		Init();
		m_vCurrentSize.m_iX = m_iWidth;
		m_vCurrentSize.m_iY = m_iHeight;
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006de1f0
void TXUIImage::PreRender()
{
	TXUIElement::PreRender();
}

// $deBlob: FUNCTION 006de200
void TXUIImage::Render()
{
	if ( m_vScale.GetX() == 0.0f || m_vScale.GetY() == 0.0f )
		return;

	TXUIRenderer* pRenderer = TXUI::GetSingleton()->GetRenderer();
	pRenderer->SetMaterial( m_pMaterial );
	pRenderer->SetColour( m_uiColour );

	TFLOAT fWidth  = UnpackFloat( m_iWidth );
	TFLOAT fHeight = UnpackFloat( m_iHeight );

	TVector2 vPos1( fWidth * -0.5f, fHeight * -0.5f );
	TVector2 vPos2( fWidth + vPos1.x, vPos1.y + fHeight );
	pRenderer->RenderRectangle( vPos1, vPos2, m_vUV1, m_vUV2 );
}

// $deBlob: FUNCTION 006de2f0
void TXUIImage::PostRender()
{
	TXUIElement::PostRender();
}

// $deBlob: FUNCTION 006de300
void TXUIImage::Init()
{
	m_vCurrentSize.m_iX = m_iWidth;
	m_vCurrentSize.m_iY = m_iHeight;

	if ( m_pTexture == TNULL )
		return;

	switch ( m_eSizeMode )
	{
		case SizeMode_Tile:
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
		case SizeMode_TextureSize:
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
		case SizeMode_Centre:
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
		case SizeMode_KeepAspect:
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
		case SizeMode_CentreCrop:
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
