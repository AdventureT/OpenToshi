#include "ToshiPCH.h"
#include "TXUICommonList.h"
#include "TXUIListItem.h"
#include "TXUIScene.h"
#include "TXUI.h"

#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiCommonList" type info
const TXUITypeInfo XURXUICommonListData::sm_sTypeInfo = { L"XuiCommonList", &XURXUIListData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUICommonListData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d3d50
XURXUICommonListData::XURXUICommonListData()
{
	m_uiItemsText    = 0;
	m_uiItemsImage   = 0;
	m_uiItemsNavPath = 0;

	m_pClass = TClass::Find( "TXUICommonList", &TGetClass( TXUIList ) );
}

// $deBlob: FUNCTION 006d3d80
TBOOL XURXUICommonListData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIListData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006d3db0
TUINT32 XURXUICommonListData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIListData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006d3de0
TBOOL XURXUICommonListData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIListData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d3e50
TBOOL XURXUICommonListData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "ItemsText", -1 ) == 0 )
	{
		a_rPropIndex = Prop_ItemsText;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "ItemsImage", -1 ) == 0 )
	{
		a_rPropIndex = Prop_ItemsImage;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "ItemsNavPath", -1 ) == 0 )
	{
		a_rPropIndex = Prop_ItemsNavPath;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIListData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006d3ee0
TUINT32 XURXUICommonListData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_ItemsText || a_uiPropIndex == Prop_ItemsImage || a_uiPropIndex == Prop_ItemsNavPath )
			return 2;

		return 4;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIListData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d3f60
TBOOL XURXUICommonListData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIListData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d3fc0
TBOOL XURXUICommonListData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIListData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d4020
TBOOL XURXUICommonListData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_ItemsText ) )
		{
			m_uiItemsText = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_ItemsImage ) )
		{
			m_uiItemsImage = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_ItemsNavPath ) )
		{
			m_uiItemsNavPath = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006d3d40
const TXUITypeInfo* XURXUICommonListData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUICommonList
//-----------------------------------------------------------------------------

// Splits a_wszBuffer at each a_wszDelimiter into new strings stored in a_pOutStrings (the text after the last
// delimiter is ignored)
// $deBlob: FUNCTION 006d47c0
static void TokeniseBuffer( const TWCHAR* a_wszBuffer, const TWCHAR* a_wszDelimiter, TWCHAR** a_pOutStrings )
{
	TSIZE         uiDelimiterLength = TStringManager::String16Length( a_wszDelimiter );
	const TWCHAR* wszFound          = TStringManager::String16FindString( a_wszBuffer, const_cast<TWCHAR*>( a_wszDelimiter ) );

	while ( wszFound != TNULL )
	{
		TSIZE   uiSize   = ( ( wszFound - a_wszBuffer ) + 1 ) * sizeof( TWCHAR );
		TWCHAR* wszToken = TSTATICCAST( TWCHAR, TMemoryHeap_Malloc( TXUI::ms_pXUIMemoryBlock, uiSize ) );

		TUtil::MemCopy( wszToken, a_wszBuffer, uiSize );
		wszToken[ uiSize / sizeof( TWCHAR ) - 1 ] = L'\0';
		*( a_pOutStrings++ )                      = wszToken;

		a_wszBuffer = wszFound + uiDelimiterLength;
		wszFound    = TStringManager::String16FindString( a_wszBuffer, const_cast<TWCHAR*>( a_wszDelimiter ) );
	}
}

// $deBlob: FUNCTION 006d40a0
// $deBlob: FUNCTION 006d40b0
// $deBlob: FUNCTION 006d40e0
// $deBlob: FUNCTION 006d4100
// $deBlob: FUNCTION 006d4110
// $deBlob: FUNCTION 0053f190
// $deBlob: FUNCTION 00859190
TXUI_DEFINE_CLASS( TXUICommonList );

// $deBlob: FUNCTION 006d4120
TXUICommonList::TXUICommonList()
{
	m_wszItemsText     = TNULL;
	m_wszItemsImage    = TNULL;
	m_wszItemsNavPath  = TNULL;
	m_awszItemsText    = TNULL;
	m_awszItemsImage   = TNULL;
	m_awszItemsNavPath = TNULL;
}

// $deBlob: FUNCTION 006d4180
// $deBlob: FUNCTION 006d4860
// $deBlob: FUNCTION 006d484b
TXUICommonList::~TXUICommonList()
{
	operator delete( m_wszItemsText );
	operator delete( m_wszItemsImage );
	operator delete( m_wszItemsNavPath );

	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		operator delete( m_awszItemsText[ i ] );
		operator delete( m_awszItemsImage[ i ] );
		operator delete( m_awszItemsNavPath[ i ] );
	}

	operator delete( m_awszItemsText );
	operator delete( m_awszItemsImage );
	operator delete( m_awszItemsNavPath );
}

// $deBlob: FUNCTION 006d4220
TBOOL TXUICommonList::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIList::Create( a_rResource, a_pData, a_bCreateChildren );

	XURXUICommonListData* pCommonListData = TSTATICCAST( XURXUICommonListData, a_pData );
	TXUI*                 pXUI            = TXUI::GetSingleton();

	if ( pCommonListData->m_uiItemsText == 0 )
		return TTRUE;

	const TWCHAR* wszItemsText    = a_rResource.GetString( pCommonListData->m_uiItemsText );
	const TWCHAR* wszItemsImage   = a_rResource.GetString( pCommonListData->m_uiItemsImage );
	const TWCHAR* wszItemsNavPath = a_rResource.GetString( pCommonListData->m_uiItemsNavPath );

	// Copy the strings, one item per line
	m_wszItemsText = new ( TXUI::ms_pXUIMemoryBlock ) TWCHAR[ TStringManager::String16Length( wszItemsText ) + 1 ];
	TStringManager::String16Copy( m_wszItemsText, wszItemsText, -1 );

	for ( const TWCHAR* wszLineEnd = TStringManager::String16FindChar( m_wszItemsText, L'\n' ); wszLineEnd != TNULL; wszLineEnd = TStringManager::String16FindChar( wszLineEnd + 1, L'\n' ) )
		m_uiNumItems++;

	m_wszItemsImage = new ( TXUI::ms_pXUIMemoryBlock ) TWCHAR[ TStringManager::String16Length( wszItemsImage ) + 1 ];
	TStringManager::String16Copy( m_wszItemsImage, wszItemsImage, -1 );

	m_wszItemsNavPath = new ( TXUI::ms_pXUIMemoryBlock ) TWCHAR[ TStringManager::String16Length( wszItemsNavPath ) + 1 ];
	TStringManager::String16Copy( m_wszItemsNavPath, wszItemsNavPath, -1 );

	if ( m_uiNumItems == 0 )
		return TTRUE;

	m_awszItemsText    = new ( TXUI::ms_pXUIMemoryBlock ) TWCHAR*[ m_uiNumItems ];
	m_awszItemsImage   = new ( TXUI::ms_pXUIMemoryBlock ) TWCHAR*[ m_uiNumItems ];
	m_awszItemsNavPath = new ( TXUI::ms_pXUIMemoryBlock ) TWCHAR*[ m_uiNumItems ];

	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		m_awszItemsText[ i ]    = TNULL;
		m_awszItemsImage[ i ]   = TNULL;
		m_awszItemsNavPath[ i ] = TNULL;
	}

	TokeniseBuffer( m_wszItemsText, L"\r\n", m_awszItemsText );
	TokeniseBuffer( m_wszItemsImage, L"\r\n", m_awszItemsImage );
	TokeniseBuffer( m_wszItemsNavPath, L"\r\n", m_awszItemsNavPath );

	// Find the XuiListItem in the visual
	if ( pXUI->GetSkin1Resource() == TNULL && pXUI->GetSkin2Resource() == TNULL )
		return TTRUE;

	XURXUIObjectData* pVisualData = TNULL;

	if ( pCommonListData->m_uiVisual != 0 )
	{
		const TWCHAR* wszVisual = a_rResource.GetString( pCommonListData->m_uiVisual );
		pVisualData             = pXUI->GetSkin1Resource()->FindVisual( wszVisual );

		if ( pVisualData == TNULL )
		{
			// NOTE: unlike TXUIList::Create, "skin2" isn't skipped here
			pVisualData = pXUI->GetSkin2Resource()->FindVisual( wszVisual );

			if ( pVisualData == TNULL )
				return TTRUE;
		}
	}
	else
	{
		// Use the default visual of the XUI class or of one of its parents
		const TXUITypeInfo* pTypeInfo = pCommonListData->GetTypeInfo();
		TASSERT( 0 == TStringManager::String16Compare( pTypeInfo->GetXuiType(), a_rResource.GetString( pCommonListData->m_uiTypeIndex ) ) );

		for ( ; pTypeInfo != TNULL; pTypeInfo = pTypeInfo->GetParent() )
		{
			const TWCHAR* wszXuiType = pTypeInfo->GetXuiType();
			pVisualData              = pXUI->GetSkin1Resource()->FindVisual( wszXuiType );

			if ( pVisualData != TNULL )
				break;

			pVisualData = pXUI->GetSkin2Resource()->FindVisual( wszXuiType );

			if ( pVisualData != TNULL )
				break;
		}

		if ( pVisualData == TNULL )
			return TTRUE;
	}

	for ( TUINT32 i = 0; i < pVisualData->GetNumChildren(); i++ )
	{
		if ( pVisualData->GetChild( i )->GetClass()->IsA( &TGetClass( TXUIListItem ) ) )
		{
			m_pItemData = pVisualData->GetChild( i );
			break;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006d4590
void TXUICommonList::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex == 0 )
	{
		// NOTE: the own properties aren't animated
		TASSERT( a_uiPropIndex < XURXUICommonListData::Prop_NUMOF );
		return;
	}

	TASSERT( a_uiObjectIndex > 0 );
	TXUIList::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// $deBlob: FUNCTION 006d4640
void TXUICommonList::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIList::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// Opens the .xur of the item's nav path in place of the list's scene
// $deBlob: FUNCTION 006d4680
void TXUICommonList::OnItemPressed( TXUIListItem* a_pItem )
{
	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		if ( &m_pItems[ i ] != a_pItem || m_awszItemsNavPath[ i ] == TNULL )
			continue;

		TString8 strNavPath( m_awszItemsNavPath[ i ] );
		strNavPath.MakeLower();

		TXUIResource* pResource = TXUI::GetSingleton()->FindResource( strNavPath.GetString() );

		if ( pResource != TNULL )
		{
			TXUIScene* pScene = TXUI::GetSingleton()->CreateScene( pResource, TUINT32( 0 ) );

			if ( pScene != TNULL )
			{
				pScene->SetVisible( TFALSE );

				TXUIScene* pFromScene   = FindParentScene();
				TXUIScene* pParentScene = pFromScene->FindParentScene();

				if ( pParentScene != TNULL )
				{
					pScene->RemoveSelf();
					pParentScene->AddChildTail( pScene );
				}

				TXUI::GetSingleton()->TransitionTo( pFromScene, pScene );
			}
		}
	}
}

TOSHI_NAMESPACE_END
