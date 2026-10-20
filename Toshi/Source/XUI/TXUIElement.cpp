#include "ToshiPCH.h"
#include "TXUIElement.h"
#include "TXUI.h"
#include "TXUITimeline.h"
#include "TXUICanvas.h"
#include "TXUIControl.h"
#include "TXUIGroup.h"
#include "TXUIScene.h"
#include "TXUIVisual.h"
#include "TXUIListItem.h"
#include "TXUIScrollEnd.h"
#include "TXUIRadioGroup.h"
#include "TXUIRenderer.h"

#include "Toshi/T2Vector.h"
#include "Math/TVector4.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiElement" type info
const TXUITypeInfo XURXUIElementData::sm_sTypeInfo = { L"XuiElement", TNULL };

// The clip rectangles pushed by PushClip (half sizes)
static T2Vector<TVector2, 10> sm_oClipStack;

// Scale of the size change of the last child UpdateAnchoring resized
static TVector2 s_vAnchoringScale;

const TVector2& TXUIElement::GetAnchoringScale()
{
	return s_vAnchoringScale;
}

//-----------------------------------------------------------------------------
// XURXUIObjectData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006be7e0
XURXUIObjectData::XURXUIObjectData()
{
	m_uiNumNamedFrames = 0;
	m_uiPropLevelMask  = 0;
	m_uiTypeIndex      = 0;
	m_uiNumChildren    = 0;
	m_uiNumTimelines   = 0;
	m_pClass           = TNULL;
	m_apChildren       = TNULL;
	m_aNamedFrames     = TNULL;
	m_aTimelines       = TNULL;
}

// $deBlob: FUNCTION 006be810
// $deBlob: FUNCTION 006c2480
XURXUIObjectData::~XURXUIObjectData()
{
	for ( TINT i = 0; i < m_uiNumChildren; i++ )
	{
		if ( m_apChildren[ i ] )
			delete m_apChildren[ i ];
	}

	operator delete( m_apChildren );

	if ( m_aNamedFrames )
		delete[] m_aNamedFrames;

	if ( m_aTimelines )
		delete[] m_aTimelines;
}

// $deBlob: FUNCTION 006be890
TBOOL XURXUIObjectData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	m_uiPropLevelMask = PARSEWORD_BIG( a_rData );
	a_rData += 2;

	return TTRUE;
}

// $deBlob: FUNCTION 006be910
TUINT32 XURXUIObjectData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIObjectData::Load( a_rResource, a_rData );
	return 0;
}

// $deBlob: FUNCTION 006c2ca0
TBOOL XURXUIObjectData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	return TFALSE;
}

// $deBlob: FUNCTION 006c2bd0
TBOOL XURXUIObjectData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	return TFALSE;
}

// $deBlob: FUNCTION 006c25e0
TUINT32 XURXUIObjectData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	return 0;
}

// $deBlob: FUNCTION 006c2690
TBOOL XURXUIObjectData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	return TFALSE;
}

// $deBlob: FUNCTION 006c2680
TBOOL XURXUIObjectData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	return TFALSE;
}

// $deBlob: FUNCTION 006be930
TBOOL XURXUIObjectData::LoadChildren( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 8 ) );
	m_uiNumChildren = a_rData[ 3 ];
	a_rData += 4;

	m_apChildren = TSTATICCAST( XURXUIElementData*, TMalloc( m_uiNumChildren * sizeof( XURXUIElementData* ), TXUI::ms_pXUIMemoryBlock ) );

	for ( TUINT32 i = 0; i < m_uiNumChildren; i++ )
	{
		TUINT16 uiType = PARSEWORD_BIG( a_rData );
		a_rData += 2;

		m_apChildren[ i ]                = TSTATICCAST( XURXUIElementData, CreateObjectData( a_rResource, uiType ) );
		m_apChildren[ i ]->m_uiTypeIndex = uiType;

		TUINT8 uiFlags = *a_rData;
		a_rData += 1;

		m_apChildren[ i ]->Load( a_rResource, a_rData );

		if ( uiFlags & ChildFlags_HasChildren )
			m_apChildren[ i ]->LoadChildren( a_rResource, a_rData );

		if ( ( uiFlags & ChildFlags_HasNamedFrames ) && m_apChildren[ i ]->LoadNamedFrames( a_rResource, a_rData ) && ( uiFlags & ChildFlags_HasChildren ) )
			m_apChildren[ i ]->LoadTimelines( a_rResource, a_rData );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006beae0
TBOOL XURXUIObjectData::LoadNamedFrames( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TASSERT( PARSEDWORD_BIG( a_rData ) < 1 << 16 );
	m_uiNumNamedFrames = PARSEWORD_BIG( a_rData + 2 );
	TASSERT( m_uiNumNamedFrames < 1000 );
	a_rData += 4;

	if ( m_uiNumNamedFrames != 0 )
	{
		m_aNamedFrames = new ( TXUI::ms_pXUIMemoryBlock ) XURXUINamedFrameData[ m_uiNumNamedFrames ];

		for ( TUINT32 i = 0; i < m_uiNumNamedFrames; i++ )
		{
			m_aNamedFrames[ i ].m_uiName = PARSEWORD_BIG( a_rData );
			a_rData += 2;

			TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 16 ) );
			m_aNamedFrames[ i ].m_uiTime = PARSEWORD_BIG( a_rData + 2 );
			a_rData += 4;

			m_aNamedFrames[ i ].m_eCommand = *a_rData;
			a_rData += 1;

			m_aNamedFrames[ i ].m_uiTarget = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006becf0
TBOOL XURXUIObjectData::LoadTimelines( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 8 ) );
	m_uiNumTimelines = a_rData[ 3 ];
	a_rData += 4;

	if ( m_uiNumTimelines != 0 )
	{
		m_aTimelines = new ( TXUI::ms_pXUIMemoryBlock ) XURXUITimelineData[ m_uiNumTimelines ];

		for ( TUINT32 i = 0; i < m_uiNumTimelines; i++ )
		{
			m_aTimelines[ i ].m_pOwnerData = this;
			m_aTimelines[ i ].Load( a_rResource, a_rData );
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006bee10
TUINT32 XURXUIObjectData::CalcChildrenPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 8 ) );
	m_uiNumChildren = a_rData[ 3 ];
	TASSERT( m_uiNumChildren < 10000 );
	a_rData += 4;

	m_apChildren = TSTATICCAST( XURXUIElementData*, TMalloc( m_uiNumChildren * sizeof( XURXUIElementData* ), TXUI::ms_pXUIMemoryBlock ) );

	TUINT32 uiSize = m_uiNumChildren * sizeof( XURXUIElementData* );

	for ( TUINT32 i = 0; i < m_uiNumChildren; i++ )
		m_apChildren[ i ] = TNULL;

	for ( TUINT32 i = 0; i < m_uiNumChildren; i++ )
	{
		TUINT16 uiType = PARSEWORD_BIG( a_rData );
		a_rData += 2;

		TUINT32 uiObjectSize = GetObjectDataSize( a_rResource, uiType );

		TUINT8 uiFlags = *a_rData;
		a_rData += 1;

		m_apChildren[ i ]                = TSTATICCAST( XURXUIElementData, CreateObjectData( a_rResource, uiType ) );
		m_apChildren[ i ]->m_uiTypeIndex = uiType;

		uiSize += uiObjectSize + m_apChildren[ i ]->CalcPileSize( a_rResource, a_rData );

		if ( uiFlags & ChildFlags_HasChildren )
			uiSize += m_apChildren[ i ]->CalcChildrenPileSize( a_rResource, a_rData );

		if ( uiFlags & ChildFlags_HasNamedFrames )
		{
			uiSize += m_apChildren[ i ]->CalcNamedFramesPileSize( a_rResource, a_rData );

			if ( uiFlags & ChildFlags_HasChildren )
				uiSize += m_apChildren[ i ]->CalcTimelinesPileSize( a_rResource, a_rData );
		}
	}

	return uiSize;
}

// NOTE: also inlined into CalcChildrenPileSize
// $deBlob: FUNCTION 006bf080
TUINT32 XURXUIObjectData::CalcNamedFramesPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiNumNamedFrames = PARSEDWORD_BIG( a_rData );
	TASSERT( uiNumNamedFrames < 1000 );
	a_rData += 4;

	if ( uiNumNamedFrames != 0 )
	{
		// Each named frame is 9 bytes in the file
		a_rData += uiNumNamedFrames * 9;
		return uiNumNamedFrames * sizeof( XURXUINamedFrameData ) + 4;
	}

	return 0;
}

// $deBlob: FUNCTION 006bf110
TUINT32 XURXUIObjectData::CalcTimelinesPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiNumTimelines = PARSEDWORD_BIG( a_rData );
	a_rData += 4;

	if ( uiNumTimelines != 0 )
	{
		TUINT32 uiSize = uiNumTimelines * sizeof( XURXUITimelineData ) + 4;

		for ( ; uiNumTimelines != 0; uiNumTimelines-- )
			uiSize += XURXUITimelineData::CalcPileSize( a_rResource, a_rData, this );

		return uiSize;
	}

	return 0;
}

// $deBlob: FUNCTION 006bf190
XURXUIElementData* XURXUIObjectData::FindChildElementData( TUINT32 a_uiStringId )
{
	for ( TINT i = 0; i < m_uiNumChildren; i++ )
	{
		if ( m_apChildren[ i ]->m_uiId == a_uiStringId )
			return m_apChildren[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 006bf1d0
XURXUIElementData* XURXUIObjectData::FindChildElementData( TXUIResource& a_rResource, const TWCHAR* a_wszName )
{
	for ( TINT i = 0; i < m_uiNumChildren; i++ )
	{
		XURXUIElementData* pChild = m_apChildren[ i ];

		if ( TStringManager::String16Compare( a_rResource.GetString( pChild->m_uiId ), a_wszName, -1 ) == 0 )
			return pChild;
	}

	return TNULL;
}

//-----------------------------------------------------------------------------
// XURXUIElementData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006bf230
XURXUIElementData::XURXUIElementData()
{
	m_iWidth    = 60 * 6;
	m_iHeight   = 30 * 6;
	m_iPosition = -1;
	m_iScale    = -1;
	m_iRotation = -1;
	m_iPivot    = -1;
	m_uiId      = 0;
	m_uiFlags   = Flags_Show;
	m_uiOpacity = 0xFF;
}

// $deBlob: FUNCTION 006bf280
TBOOL XURXUIElementData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIObjectData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006bf2b0
TUINT32 XURXUIElementData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIObjectData::Load( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return 0;
}

// $deBlob: FUNCTION 006bf2e0
TBOOL XURXUIElementData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	TASSERT( a_uiObjectIndex == 0 );
	return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006bf330
TBOOL XURXUIElementData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "Id", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Id;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Width", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Width;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Height", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Height;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Position", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Position;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Scale", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Scale;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Rotation", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Rotation;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Opacity", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Opacity;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Anchor", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Anchor;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Pivot", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Pivot;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Show", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Show;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "BlendMode", -1 ) == 0 )
	{
		a_rPropIndex = Prop_BlendMode;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "DisableTimelineRecursion", -1 ) == 0 )
	{
		a_rPropIndex = Prop_DisableTimelineRecursion;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "ColorWriteFlags", -1 ) == 0 )
	{
		a_rPropIndex = Prop_ColorWriteFlags;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "ClipChildren", -1 ) == 0 )
	{
		a_rPropIndex = Prop_ClipChildren;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006bf510
TUINT32 XURXUIElementData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	TASSERT( a_uiObjectIndex == 0 );

	if ( a_uiPropIndex != Prop_DisableTimelineRecursion && a_uiPropIndex != Prop_Show && a_uiPropIndex != Prop_ClipChildren )
		return ( a_uiPropIndex != Prop_Id ) ? 4 : 2;

	return 1;
}

// $deBlob: FUNCTION 006bf580
TBOOL XURXUIElementData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	TASSERT( a_uiObjectIndex == 0 );
	return a_uiPropIndex == Prop_Width || a_uiPropIndex == Prop_Height || a_uiPropIndex == Prop_Opacity;
}

// $deBlob: FUNCTION 006bf5e0
TBOOL XURXUIElementData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	TASSERT( a_uiObjectIndex == 0 );
	return TFALSE;
}

// $deBlob: FUNCTION 006bf220
const TXUITypeInfo* XURXUIElementData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// $deBlob: FUNCTION 006bf620
TBOOL XURXUIElementData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_Id ) )
		{
			m_uiId = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_Width ) )
		{
			m_iWidth = TINT16( TINT( PARSEFLOAT_BIG( a_rData ) * 6.0f ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Height ) )
		{
			m_iHeight = TINT16( TINT( PARSEFLOAT_BIG( a_rData ) * 6.0f ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Position ) )
		{
			TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 16 ) );
			m_iPosition = PARSEWORD_BIG( a_rData + 2 );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Scale ) )
		{
			TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 16 ) );
			m_iScale = PARSEWORD_BIG( a_rData + 2 );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Rotation ) )
		{
			TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 16 ) );
			m_iRotation = PARSEWORD_BIG( a_rData + 2 );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Opacity ) )
		{
			TFLOAT fOpacity = PARSEFLOAT_BIG( a_rData );
			TASSERT( fOpacity >= 0.0f && fOpacity <= 1.0f );
			m_uiOpacity = TUINT8( TINT( fOpacity * 255.0f ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Anchor ) )
		{
			TASSERT( PARSEDWORD_BIG( a_rData ) <= Flags_AnchorMask );
			m_uiFlags |= PARSEWORD_BIG( a_rData + 2 ) << Flags_AnchorShift;
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Pivot ) )
		{
			TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 16 ) );
			m_iPivot = PARSEWORD_BIG( a_rData + 2 );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Show ) )
		{
			m_uiFlags &= ~Flags_Show;
			m_uiFlags |= ( *a_rData != 0 ) ? Flags_Show : 0;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_BlendMode ) )
		{
			TASSERT( PARSEDWORD_BIG( a_rData ) <= Flags_BlendModeMask );
			m_uiFlags |= PARSEWORD_BIG( a_rData + 2 );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_DisableTimelineRecursion ) )
		{
			m_uiFlags &= ~Flags_DisableTimelineRecursion;
			m_uiFlags |= ( *a_rData != 0 ) ? Flags_DisableTimelineRecursion : 0;
			a_rData += 1;
		}

		// NOTE: DesignTime (bit 12) is never stored in the .xur, ColorWriteFlags is skipped
		if ( uiPropMask & BITFLAG( Prop_ColorWriteFlags ) )
		{
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_ClipChildren ) )
		{
			m_uiFlags &= ~Flags_ClipChildren;
			m_uiFlags |= ( *a_rData != 0 ) ? Flags_ClipChildren : 0;
			a_rData += 1;
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// TXUIElement
//-----------------------------------------------------------------------------

// NOTE: TXUIElement can't be created at runtime but, unlike TDEFINE_CLASS_NORUNTIME, retail's create functions just
// return TNULL. The TClass is registered with an alignment of 1
// $deBlob: FUNCTION 006bfad0
const TClass* TXUIElement::GetClass() const
{
	return &TGetClass( TXUIElement );
}

// $deBlob: FUNCTION 006bfae0
TObject* TXUIElement::CreateTObject()
{
	return TNULL;
}

// $deBlob: FUNCTION 006bfaf0
TObject* TXUIElement::CreateTObjectInPlace( void* a_pPtr )
{
	return TNULL;
}

// $deBlob: FUNCTION 006bfb00
void TXUIElement::InitialiseClass()
{
}

// $deBlob: FUNCTION 006bfb10
void TXUIElement::DeinitialiseClass()
{
}

// $deBlob: FUNCTION 0053ee00
// $deBlob: FUNCTION 00859030
TClass TXUIElement::TClassObjectName = TClass( "TXUIElement", TXUIElement::GetParentClassStatic(), TXUIElement::CreateTObject, TXUIElement::CreateTObjectInPlace, TXUIElement::InitialiseClass, TXUIElement::DeinitialiseClass, 0, 1, sizeof( TXUIElement ), 1 );

TUINT32 TXUIElement::s_uiNumClips = 0;

// $deBlob: FUNCTION 006bfb20
TXUIElement::TXUIElement()
{
	m_pData     = TNULL;
	m_wszID     = TNULL;
	m_vPivot    = TVector2::VEC_ZERO;
	m_vPosition = TVector2::VEC_ZERO;

	m_vScale.m_iX         = 256;
	m_vScale.m_iY         = 256;
	m_vPositionScale.m_iX = 6;
	m_vPositionScale.m_iY = 6;

	m_fZRotation = 0.0f;
	m_iUID       = -1;

	m_uiXUIState = XUIState_TransformDirty | XUIState_TimelineRecursion;
	SetXUIStateBits( TUINT32( TMath::Round( 255.0f ) ), 0xFF, XUIState_HierarchicalAlphaShift );
	SetXUIStateBits( TUINT32( TMath::Round( 127.0f ) ), 0x7F, XUIState_OpacityShift );

	m_eAnchor = T2GUIATTACHMENT_TOPLEFT;
	m_ePivot  = T2GUIATTACHMENT_TOPLEFT;
	m_eFlags |= FLAGS_XUIELEMENT;
}

// $deBlob: FUNCTION 006bfca0
// $deBlob: FUNCTION 006c2450
// $deBlob: FUNCTION 006c2387
TXUIElement::~TXUIElement()
{
	T2GUIElementNode* pNode = m_Children.m_pNext;

	if ( pNode != &m_Children )
	{
		while ( pNode != TNULL )
		{
			T2GUIElement* pChild = TSTATICCAST( T2GUIElement, pNode );
			pChild->RemoveSelf();

			pNode = m_Children.m_pNext;
			if ( pNode == &m_Children ) pNode = TNULL;

			if ( pChild )
				delete pChild;
		}
	}

	RemoveSelf();
}

// $deBlob: FUNCTION 00626410
TBOOL TXUIElement::IsPaused()
{
	return ( m_uiXUIState & XUIState_TimelineStateMask ) == ( TimelineState_Paused << XUIState_TimelineStateShift );
}

// $deBlob: FUNCTION 00626380
TBOOL TXUIElement::SkipRender()
{
	if ( GetFlags_Opacity() != 0.0f && m_vScale.GetX() != 0.0f && m_vScale.GetY() != 0.0f )
		return !( ( m_eFlags & FLAGS_VISIBLE ) && ( m_uiState & s_uiGlobalVisMask ) );

	return TTRUE;
}

// $deBlob: FUNCTION 006c0ef0
void TXUIElement::PreRender()
{
	T2GUITransform oPivotTransform;
	m_vPivot.Get( oPivotTransform.m_Pos );
	TXUI::GetSingleton()->GetRenderer()->PushTransform( oPivotTransform, TVector2::VEC_ZERO, TVector2::VEC_ZERO );

	TUINT32 uiDirty = m_uiXUIState >> XUIState_DirtyShift;

	if ( uiDirty & DirtyFlags_Transform )
	{
		m_oTransform.SetIdentity();
		m_oTransform.Rotate( m_fZRotation );

		T2GUITransform oScale( m_vScale.GetX(), m_vScale.GetY() );
		m_oTransform.PreMultiply( oScale );

		m_oTransform.m_Pos.y = m_vPosition.GetY();
		m_oTransform.m_Pos.x = m_vPosition.GetX();
		m_uiXUIState &= ~( XUIState_PositionDirty | XUIState_TransformDirty );
	}
	else if ( uiDirty & DirtyFlags_Position )
	{
		m_oTransform.m_Pos.y = m_vPosition.GetY();
		m_oTransform.m_Pos.x = m_vPosition.GetX();
		m_uiXUIState &= ~XUIState_PositionDirty;
	}

	UpdateHierarchicalAlpha();
	SetAlpha( GetFlags_HierarchicalAlpha() );
	T2GUIElement::PreRender();
}

// $deBlob: FUNCTION 006bc830
void TXUIElement::Render()
{
	T2GUIElement::Render();
}

// $deBlob: FUNCTION 006c10c0
void TXUIElement::PostRender()
{
	TXUI::GetSingleton()->GetRenderer()->PopTransform();
	T2GUIElement::PostRender();
}

// $deBlob: FUNCTION 006c0330
void TXUIElement::SetWidth( TFLOAT a_fWidth )
{
	TVector2 vOldSize( UnpackFloat( m_iWidth ), UnpackFloat( m_iHeight ) );

	TASSERT( a_fWidth >= 0.0f );
	m_iWidth = PackFloat( a_fWidth );

	TVector2 vNewSize( UnpackFloat( m_iWidth ), UnpackFloat( m_iHeight ) );

	if ( !vOldSize.IsEqual( vNewSize ) )
	{
		TVector2 vDelta( vOldSize.x - vNewSize.x, vOldSize.y - vNewSize.y );
		UpdateAnchoring( vDelta );
	}
}

// $deBlob: FUNCTION 006c0440
void TXUIElement::SetHeight( TFLOAT a_fHeight )
{
	TVector2 vOldSize( UnpackFloat( m_iWidth ), UnpackFloat( m_iHeight ) );

	TASSERT( a_fHeight >= 0.0f );
	m_iHeight = PackFloat( a_fHeight );

	TVector2 vNewSize( UnpackFloat( m_iWidth ), UnpackFloat( m_iHeight ) );

	if ( !vOldSize.IsEqual( vNewSize ) )
	{
		TVector2 vDelta( vOldSize.x - vNewSize.x, vOldSize.y - vNewSize.y );
		UpdateAnchoring( vDelta );
	}
}

// $deBlob: FUNCTION 006c0250
void TXUIElement::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TVector2 vOldSize( UnpackFloat( m_iWidth ), UnpackFloat( m_iHeight ) );

	m_iWidth  = PackFloat( a_fWidth );
	m_iHeight = PackFloat( a_fHeight );

	TVector2 vNewSize( UnpackFloat( m_iWidth ), UnpackFloat( m_iHeight ) );

	if ( !vOldSize.IsEqual( vNewSize ) )
	{
		TVector2 vDelta( vOldSize.x - vNewSize.x, vOldSize.y - vNewSize.y );
		UpdateAnchoring( vDelta );
	}
}

// $deBlob: FUNCTION 00626070
void TXUIElement::SetAlpha( TFLOAT a_fAlpha )
{
	m_uiColour &= 0x00FFFFFF;
	m_uiColour |= TUINT32( a_fAlpha * 255.0f ) << 24;
}

// $deBlob: FUNCTION 00625e20
TFLOAT TXUIElement::GetAlpha()
{
	return TFLOAT( ( m_uiColour >> 24 ) & 0xFF ) / 255.0f;
}

// $deBlob: FUNCTION 006c1960
void TXUIElement::GetScreenTransform( T2GUITransform& a_rOutTransform )
{
	TUINT32 uiDirty = m_uiXUIState >> XUIState_DirtyShift;

	if ( uiDirty & DirtyFlags_Transform )
	{
		m_oTransform.SetIdentity();
		m_oTransform.Rotate( m_fZRotation );

		T2GUITransform oScale( m_vScale.GetX(), m_vScale.GetY() );
		m_oTransform.PreMultiply( oScale );

		m_oTransform.m_Pos.y = m_vPosition.GetY();
		m_oTransform.m_Pos.x = m_vPosition.GetX();
		m_uiXUIState &= ~( XUIState_PositionDirty | XUIState_TransformDirty );
	}
	else if ( uiDirty & DirtyFlags_Position )
	{
		m_oTransform.m_Pos.y = m_vPosition.GetY();
		m_oTransform.m_Pos.x = m_vPosition.GetX();
		m_uiXUIState &= ~XUIState_PositionDirty;
	}

	T2GUIElement::GetScreenTransform( a_rOutTransform );
}

// $deBlob: FUNCTION 00625ea0
const TVector2& TXUIElement::GetPivot()
{
	static TVector2 s_vScratchPivot;
	m_vPivot.Get( s_vScratchPivot );

	return s_vScratchPivot;
}

// $deBlob: FUNCTION 006bfd30
TBOOL TXUIElement::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	m_pData = a_pData;

	const TWCHAR* wszID = a_rResource.GetString( a_pData->m_uiId );

	if ( GetClass() != &TGetClass( TXUICanvas ) )
		a_rResource.PushID( wszID );

	m_wszID = wszID;

	if ( TXUIResource::s_iGenerateUIDs == 0 && TStringManager::String16Length( wszID ) != 0 )
	{
		if ( !IsA( &TGetClass( TXUIListItem ) ) && !IsA( &TGetClass( TXUIScrollEnd ) ) )
			m_iUID = TXUIResource::s_iUIDCount++;
	}

	m_iWidth  = a_pData->m_iWidth;
	m_iHeight = a_pData->m_iHeight;

	if ( a_pData->m_iPosition >= 0 )
		m_vPosition = *a_rResource.GetVector( a_pData->m_iPosition );

	if ( a_pData->m_iRotation >= 0 )
		m_fZRotation = a_rResource.GetZRotation( a_pData->m_iRotation );

	if ( a_pData->m_iScale >= 0 )
		m_vScale = *a_rResource.GetVector( a_pData->m_iScale );

	TFLOAT fOpacity = a_pData->m_uiOpacity * ( 1.0f / 255.0f );
	TMath::Clip( fOpacity, 0.0f, 1.0f );
	SetXUIStateBits( TUINT32( TMath::Round( fOpacity * 127.0f ) ), 0x7F, XUIState_OpacityShift );

	m_uiXUIState = ( m_uiXUIState & ~XUIState_AnchorMask ) | ( ( a_pData->m_uiFlags >> XURXUIElementData::Flags_AnchorShift ) & XUIState_AnchorMask );

	if ( a_pData->m_iPivot >= 0 )
		m_vPivot = *a_rResource.GetVector( a_pData->m_iPivot );

	SetVisible( ( a_pData->m_uiFlags >> XURXUIElementData::Flags_ShowShift ) & 1 );

	m_uiXUIState = ( m_uiXUIState & ~XUIState_BlendModeMask ) | ( ( a_pData->m_uiFlags & XURXUIElementData::Flags_BlendModeMask ) << XUIState_BlendModeShift );
	m_uiXUIState = ( m_uiXUIState & ~XUIState_TimelineRecursion ) | ( ( ~( a_pData->m_uiFlags >> XURXUIElementData::Flags_DisableTimelineRecursionShift ) & 1 ) << XUIState_TimelineRecursionShift );
	m_uiXUIState = ( m_uiXUIState & ~XUIState_ClipChildren ) | ( ( ( a_pData->m_uiFlags >> XURXUIElementData::Flags_ClipChildrenShift ) & 1 ) << XUIState_ClipChildrenShift );

	if ( a_bCreateChildren )
		CreateChildren( a_rResource, a_pData );

	if ( GetClass() != &TGetClass( TXUICanvas ) )
		a_rResource.PopID();

	return TTRUE;
}

// $deBlob: FUNCTION 006c00c0
void TXUIElement::CreateChildren( TXUIResource& a_rResource, XURXUIObjectData* a_pData )
{
	for ( TINT i = 0; i < a_pData->m_uiNumChildren; i++ )
	{
		XURXUIElementData* pChildData = a_pData->m_apChildren[ i ];

		// List items and scroll ends are created by their lists
		if ( pChildData->m_pClass->IsA( &TGetClass( TXUIListItem ) ) || pChildData->m_pClass->IsA( &TGetClass( TXUIScrollEnd ) ) )
			continue;

		TXUIElement* pElement = TNULL;

		if ( pChildData->m_pClass->IsA( &TGetClass( TXUIControl ) ) && TSTATICCAST( XURXUIControlData, pChildData )->m_uiClassOverride != 0 )
		{
			const TWCHAR* wszClassName = a_rResource.GetString( TSTATICCAST( XURXUIControlData, pChildData )->m_uiClassOverride );
			TINT          iLen         = TStringManager::String16Length( wszClassName );
			TASSERT( iLen < 128 );

			TCHAR szClassName[ 128 ];
			TStringManager::StringUnicodeToChar( szClassName, wszClassName, iLen );

			const TClass* pClass = TClass::Find( szClassName, &TGetClass( TXUIControl ) );

			if ( pClass )
				pElement = CastFromTObject( pClass->CreateObject() );
		}

		if ( pElement == TNULL )
		{
			TObject* pObject = pChildData->m_pClass->CreateObject();
			pElement         = pObject ? TSTATICCAST( TXUIElement, pObject ) : TNULL;
		}

		if ( pElement )
			pElement->Create( a_rResource, pChildData, TTRUE );

		AddChildTail( pElement );
		pElement->OnCreated();
	}
}

// $deBlob: FUNCTION 00626220
void TXUIElement::SetOpacity( TFLOAT a_fOpacity )
{
	SetXUIStateBits( TUINT32( TMath::Round( a_fOpacity * 127.0f ) ), 0x7F, XUIState_OpacityShift );
}

// $deBlob: FUNCTION 006c0550
TXUIElement* TXUIElement::RayTraceDeepest( const TVector2& a_rPoint, const TVector2& a_rScale, TINT a_iController )
{
	if ( !( m_eFlags & FLAGS_VISIBLE ) || !( m_uiState & s_uiGlobalVisMask ) || GetFlags_Opacity() == 0.0f )
		return TNULL;

	TVector2 vScale( a_rScale.x * m_vScale.GetX(), a_rScale.y * m_vScale.GetY() );

	if ( TMath::Abs( vScale.x ) < ( 1.0f / 256.0f ) || TMath::Abs( vScale.y ) < ( 1.0f / 256.0f ) )
		return TNULL;

	for ( T2GUIElementNode* pNode = m_Children.m_pPrev; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pPrev )
	{
		TXUIElement* pChild = TSTATICCAST( TXUIElement, TSTATICCAST( T2GUIElement, pNode ) );
		TASSERT( TTRUE == pChild->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) );

		if ( !pChild->IsA( &TGetClass( TXUIVisual ) ) )
		{
			TXUIElement* pResult = pChild->RayTraceDeepest( a_rPoint, vScale, a_iController );

			if ( pResult )
				return pResult;
		}
	}

	if ( IsA( &TGetClass( TXUIControl ) ) && RayTrace( a_rPoint ) == TTRUE )
		return this;

	return TNULL;
}

// $deBlob: FUNCTION 006c06f0
TBOOL TXUIElement::RayTrace( const TVector2& a_rPoint )
{
	TASSERT( TMath::Abs( m_vScale.GetX() ) > 0.0f && TMath::Abs( m_vScale.GetY() ) > 0.0f && IsVisible() && GetFlags_Opacity() > 0.0f );

	T2GUITransform oInvTransform;
	GetInvScreenTransform( oInvTransform );

	TVector2 vLocalPoint;
	vLocalPoint.x = oInvTransform.m_Rot[ 0 ].GetX() * a_rPoint.x + oInvTransform.m_Rot[ 1 ].GetX() * a_rPoint.y + oInvTransform.m_Pos.x;
	vLocalPoint.y = oInvTransform.m_Rot[ 0 ].GetY() * a_rPoint.x + oInvTransform.m_Rot[ 1 ].GetY() * a_rPoint.y + oInvTransform.m_Pos.y;

	return IsPointInside( vLocalPoint );
}

// $deBlob: FUNCTION 006c08d0
void TXUIElement::PlayAnimation( TINT a_iStartFrame, TINT a_iEndFrame )
{
	for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
	{
		T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );

		if ( pGUIElement->GetFlag( FLAGS_XUIELEMENT ) )
		{
			TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

			if ( pChild->IsTimelineRecursionEnabled() )
				pChild->PlayAnimation( a_iStartFrame, a_iEndFrame );
		}
	}
}

// $deBlob: FUNCTION 006c0870
void TXUIElement::PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame )
{
	for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
	{
		T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );

		if ( pGUIElement->GetFlag( FLAGS_XUIELEMENT ) )
		{
			TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

			if ( !pChild->IsA( &TGetClass( TXUIVisual ) ) && pChild->IsTimelineRecursionEnabled() )
				pChild->PlayAnimation( a_wszStartFrame, a_wszEndFrame );
		}
	}
}

// $deBlob: FUNCTION 006c0920
void TXUIElement::PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd )
{
	for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
	{
		T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );

		// NOTE: tests this element's DisableTimelineRecursion, not the child's (PlayAnimation tests the child's)
		if ( pGUIElement->GetFlag( FLAGS_XUIELEMENT ) && IsTimelineRecursionEnabled() )
			TSTATICCAST( TXUIElement, pGUIElement )->PlayTimelineRange( a_fStart, a_fEnd );
	}
}

// $deBlob: FUNCTION 006c0980
void TXUIElement::StopAnimation()
{
	for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
	{
		T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );

		if ( pGUIElement->GetFlag( FLAGS_XUIELEMENT ) && IsTimelineRecursionEnabled() )
			TSTATICCAST( TXUIElement, pGUIElement )->StopAnimation();
	}
}

// $deBlob: FUNCTION 006bc850
TBOOL TXUIElement::SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam )
{
	return TFALSE;
}

// $deBlob: FUNCTION 006c11e0
void TXUIElement::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	TASSERT( a_uiObjectIndex == 0 );

	switch ( a_uiPropIndex )
	{
		case XURXUIElementData::Prop_Position:
		{
			TVector2 vPosition = *a_pResource->GetVector( a_uiValue1 );
			vPosition.Lerp( vPosition, *a_pResource->GetVector( a_uiValue2 ), a_fScalar );
			vPosition.x *= m_vPositionScale.GetX();
			vPosition.y *= m_vPositionScale.GetY();

			TVector2 vCurrent = m_vPosition.Get();

			if ( !vPosition.IsEqual( vCurrent ) )
			{
				m_vPosition = vPosition;
				SetXUIStateFlags( DirtyFlags_Transform, 3, XUIState_DirtyShift );
			}

			break;
		}
		case XURXUIElementData::Prop_Scale:
		{
			TVector2 vScale = *a_pResource->GetVector( a_uiValue1 );
			vScale.Lerp( vScale, *a_pResource->GetVector( a_uiValue2 ), a_fScalar );

			TVector2 vCurrent = m_vScale.Get();

			if ( !vScale.IsEqual( vCurrent ) )
			{
				m_vScale = vScale;
				SetXUIStateFlags( DirtyFlags_Transform, 3, XUIState_DirtyShift );
			}

			break;
		}
		case XURXUIElementData::Prop_Rotation:
		{
			TFLOAT fRotation = TMath::LerpAngle( a_pResource->GetZRotation( a_uiValue1 ), a_pResource->GetZRotation( a_uiValue2 ), a_fScalar );

			if ( fRotation != m_fZRotation )
			{
				m_fZRotation = fRotation;
				SetXUIStateFlags( DirtyFlags_Transform, 3, XUIState_DirtyShift );
			}

			break;
		}
		case XURXUIElementData::Prop_Opacity:
		case XURXUIElementData::Prop_DesignTime:
			break;
		case XURXUIElementData::Prop_Anchor:
		{
			TFLOAT fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

			if ( fScalar != 0.0f )
			{
				TASSERT( a_fScalar == 1.0f );
				SetXUIStateBits( a_uiValue2, XUIState_AnchorMask, 0 );
			}
			else
			{
				SetXUIStateBits( a_uiValue1, XUIState_AnchorMask, 0 );
			}

			break;
		}
		case XURXUIElementData::Prop_Pivot:
		{
			TVector2 vPivot = *a_pResource->GetVector( a_uiValue1 );
			vPivot.Lerp( vPivot, *a_pResource->GetVector( a_uiValue2 ), a_fScalar );
			m_vPivot = vPivot;
			break;
		}
		case XURXUIElementData::Prop_Show:
		{
			TFLOAT fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

			if ( fScalar != 0.0f )
			{
				TASSERT( a_fScalar == 1.0f );
				SetVisible( a_uiValue2 == 1 );
			}
			else
			{
				SetVisible( a_uiValue1 == 1 );
			}

			break;
		}
		case XURXUIElementData::Prop_BlendMode:
		{
			TFLOAT fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

			if ( fScalar != 0.0f )
			{
				TASSERT( a_fScalar == 1.0f );
				SetXUIStateBits( a_uiValue2, 7, XUIState_BlendModeShift );
			}
			else
			{
				SetXUIStateBits( a_uiValue1, 7, XUIState_BlendModeShift );
			}

			break;
		}
		case XURXUIElementData::Prop_DisableTimelineRecursion:
		{
			TFLOAT fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

			if ( fScalar != 0.0f )
			{
				TASSERT( a_fScalar == 1.0f );
				SetXUIStateBits( a_uiValue2 == 0, 1, 15 );
			}
			else
			{
				SetXUIStateBits( a_uiValue1 == 0, 1, 15 );
			}

			break;
		}
		case XURXUIElementData::Prop_ColorWriteFlags:
			TMath::FloorToInt( a_fScalar );
			return;
		case XURXUIElementData::Prop_ClipChildren:
		{
			TFLOAT fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

			if ( fScalar != 0.0f )
			{
				a_uiValue1 = a_uiValue2;
				TASSERT( a_fScalar == 1.0f );
			}

			SetXUIStateBits( a_uiValue1 == 1, 1, 16 );
			break;
		}
	}
}

// $deBlob: FUNCTION 006c10e0
void TXUIElement::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	TASSERT( a_uiObjectIndex == 0 );

	if ( a_uiPropIndex == XURXUIElementData::Prop_Width )
		SetWidth( TMath::LERP( a_fValue1, a_fValue2, a_fScalar ) );
	else if ( a_uiPropIndex == XURXUIElementData::Prop_Height )
		SetHeight( TMath::LERP( a_fValue1, a_fValue2, a_fScalar ) );
	else if ( a_uiPropIndex == XURXUIElementData::Prop_Opacity )
		SetOpacity( TMath::LERP( a_fValue1, a_fValue2, a_fScalar ) );
}

// $deBlob: FUNCTION 0062bca0
void TXUIElement::SetPresenterData( const TWCHAR* a_wszText, const TWCHAR* a_wszImagePath, TUINT32 a_uiTextIndex, TUINT32 a_uiImageIndex, TINT a_iParam )
{
}

// $deBlob: FUNCTION 0062bcc0
void TXUIElement::SetPresenterText( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam )
{
}

// $deBlob: FUNCTION 0062bcd0
void TXUIElement::SetPresenterText2( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam )
{
}

// $deBlob: FUNCTION 0062bcb0
void TXUIElement::SetPresenterImagePath( const TWCHAR* a_wszImagePath, TUINT32 a_uiIndex )
{
}

// $deBlob: FUNCTION 0062bc70
void TXUIElement::OnCreated()
{
}

// $deBlob: FUNCTION 00626050
void TXUIElement::OnRayTraceHit( TINT a_iController )
{
}

// $deBlob: FUNCTION 00626310
void TXUIElement::SetXUIAnchoring( TUINT32 a_uiAnchoring )
{
	SetXUIStateBits( a_uiAnchoring, XUIState_AnchorMask, 0 );
}

// $deBlob: FUNCTION 006bc810
TBOOL TXUIElement::IsCheckable()
{
	return TFALSE;
}

// $deBlob: FUNCTION 006bc820
TBOOL TXUIElement::IsEnabled()
{
	return TTRUE;
}

// $deBlob: FUNCTION 00626010
TBOOL TXUIElement::IsChecked()
{
	return TFALSE;
}

// $deBlob: FUNCTION 006bc840
void TXUIElement::ResolveNavigation( TXUIElement* a_pRoot )
{
}

// $deBlob: FUNCTION 00626ef0
TBOOL TXUIElement::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	return TFALSE;
}

// $deBlob: FUNCTION 00626ee0
TBOOL TXUIElement::OnNotifySetFocus( TXUIElement* a_pSource )
{
	return TFALSE;
}

// $deBlob: FUNCTION 00626ed0
TBOOL TXUIElement::OnNotifyKillFocus( TXUIElement* a_pSource )
{
	return TFALSE;
}

// $deBlob: FUNCTION 00626040
TBOOL TXUIElement::OnNotifyValueChanged( TXUIElement* a_pSource )
{
	return TFALSE;
}

// Returns TTRUE if FindChildXUIElement/Control should look into the children of the element
static TBOOL TXUIElement_IsContainer( TXUIElement* a_pElement )
{
	return a_pElement->IsA( &TGetClass( TXUIGroup ) ) || a_pElement->IsA( &TGetClass( TXUIScene ) ) || a_pElement->IsA( &TGetClass( TXUIRadioGroup ) );
}

// $deBlob: FUNCTION 006c09c0
TXUIElement* TXUIElement::FindChildXUIElement( const TWCHAR* a_wszName )
{
	if ( a_wszName == TNULL )
		return TNULL;

	const TWCHAR* pSeparator = TStringManager::String16FindChar( a_wszName, L'\\' );

	if ( pSeparator == TNULL )
	{
		for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
		{
			T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );
			if ( !pGUIElement->GetFlag( FLAGS_XUIELEMENT ) ) continue;

			TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );
			if ( pChild->IsA( &TGetClass( TXUIVisual ) ) ) continue;

			if ( pChild->m_wszID && TStringManager::String16Compare( pChild->m_wszID, a_wszName, -1 ) == 0 )
				return pChild;

			if ( TXUIElement_IsContainer( pChild ) )
			{
				TXUIElement* pResult = pChild->FindChildXUIElement( a_wszName );
				if ( pResult ) return pResult;
			}
		}
	}
	else
	{
		TWCHAR wszName[ 256 ];
		TINT   iLen = TINT( pSeparator - a_wszName );
		TStringManager::String16Copy( wszName, a_wszName, iLen );
		wszName[ iLen ] = L'\0';

		TXUIElement* pElement = FindChildXUIElement( wszName );

		if ( pElement )
			return pElement->FindChildXUIElement( pSeparator + 1 );
	}

	return TNULL;
}

// $deBlob: FUNCTION 006c0b70
TXUIControl* TXUIElement::FindChildXUIControl( const TWCHAR* a_wszName )
{
	if ( a_wszName == TNULL )
		return TNULL;

	const TWCHAR* pSeparator = TStringManager::String16FindChar( a_wszName, L'\\' );

	if ( pSeparator == TNULL )
	{
		for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
		{
			T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );
			if ( !pGUIElement->GetFlag( FLAGS_XUIELEMENT ) ) continue;

			TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );
			if ( pChild->IsA( &TGetClass( TXUIVisual ) ) ) continue;

			if ( pChild->IsA( &TGetClass( TXUIControl ) ) && TStringManager::String16Compare( pChild->m_wszID, a_wszName, -1 ) == 0 )
				return TSTATICCAST( TXUIControl, pChild );

			if ( TXUIElement_IsContainer( pChild ) )
			{
				TXUIControl* pResult = pChild->FindChildXUIControl( a_wszName );
				if ( pResult ) return pResult;
			}
		}
	}
	else
	{
		TWCHAR wszName[ 256 ];
		TINT   iLen = TINT( pSeparator - a_wszName );
		TStringManager::String16Copy( wszName, a_wszName, iLen );
		wszName[ iLen ] = L'\0';

		TXUIElement* pElement = FindChildXUIElement( wszName );

		if ( pElement )
			return pElement->FindChildXUIControl( pSeparator + 1 );
	}

	return TNULL;
}

// $deBlob: FUNCTION 006c0d30
TXUIElement* TXUIElement::FindChildXUIElement( TINT a_iUID )
{
	if ( a_iUID == -1 )
		return TNULL;

	for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
	{
		T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );
		if ( !pGUIElement->GetFlag( FLAGS_XUIELEMENT ) ) continue;

		TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );
		if ( pChild->IsA( &TGetClass( TXUIVisual ) ) ) continue;

		if ( pChild->m_wszID && a_iUID == pChild->m_iUID )
			return pChild;

		if ( TXUIElement_IsContainer( pChild ) )
		{
			TXUIElement* pResult = pChild->FindChildXUIElement( a_iUID );
			if ( pResult ) return pResult;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 006c0e00
TXUIControl* TXUIElement::FindChildXUIControl( TINT a_iUID )
{
	if ( a_iUID == -1 )
		return TNULL;

	for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
	{
		T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );
		if ( !pGUIElement->GetFlag( FLAGS_XUIELEMENT ) ) continue;

		TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );
		if ( pChild->IsA( &TGetClass( TXUIVisual ) ) ) continue;

		if ( pChild->IsA( &TGetClass( TXUIControl ) ) && a_iUID == pChild->m_iUID )
			return TSTATICCAST( TXUIControl, pChild );

		if ( TXUIElement_IsContainer( pChild ) )
		{
			TXUIControl* pResult = pChild->FindChildXUIControl( a_iUID );
			if ( pResult ) return pResult;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 006c1ab0
TXUIElement* TXUIElement::FindParentControl()
{
	for ( T2GUIElement* pParent = m_pParent; pParent != TNULL && pParent->GetFlag( FLAGS_XUIELEMENT ); pParent = pParent->GetParent() )
	{
		TXUIElement* pElement = TSTATICCAST( TXUIElement, pParent );

		if ( pElement->IsA( &TGetClass( TXUIControl ) ) == TTRUE )
			return pElement;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006c17b0
TUINT32 TXUIElement::InterpColour( TUINT32 a_uiColour1, TUINT32 a_uiColour2, TFLOAT a_fScalar )
{
	TVector4 vColour1;
	vColour1.w = ( TFLOAT( a_uiColour1 & 0xFF ) + 0.0f ) / 255.0f;
	vColour1.z = ( TFLOAT( ( a_uiColour1 >> 8 ) & 0xFF ) + 0.0f ) / 255.0f;
	vColour1.y = ( TFLOAT( ( a_uiColour1 >> 16 ) & 0xFF ) + 0.0f ) / 255.0f;
	vColour1.x = ( TFLOAT( a_uiColour1 >> 24 ) + 0.0f ) / 255.0f;

	TVector4 vColour2;
	vColour2.w = ( TFLOAT( a_uiColour2 & 0xFF ) + 0.0f ) / 255.0f;
	vColour2.z = ( TFLOAT( ( a_uiColour2 >> 8 ) & 0xFF ) + 0.0f ) / 255.0f;
	vColour2.y = ( TFLOAT( ( a_uiColour2 >> 16 ) & 0xFF ) + 0.0f ) / 255.0f;
	vColour2.x = ( TFLOAT( a_uiColour2 >> 24 ) + 0.0f ) / 255.0f;

	vColour1.Lerp4( vColour1, vColour2, a_fScalar );

	return MyMakeARGB( vColour1.y, vColour1.z, vColour1.w, vColour1.x );
}

// NOTE: inline in TXUIElement.h (234-237), the instance lives in the game code
// $deBlob: FUNCTION 00653cc0
TUINT32 TXUIElement::MyMakeARGB( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha )
{
	TASSERT( a_fRed >= 0.0f && a_fRed <= 1.0f );
	TASSERT( a_fGreen >= 0.0f && a_fGreen <= 1.0f );
	TASSERT( a_fBlue >= 0.0f && a_fBlue <= 1.0f );
	TASSERT( a_fAlpha >= 0.0f && a_fAlpha <= 1.0f );

	return ( ( ( TINT( a_fRed * 255.0f ) | TINT( a_fAlpha * 255.0f ) << 8 ) << 8 | TINT( a_fGreen * 255.0f ) ) << 8 ) | TINT( a_fBlue * 255.0f );
}

// $deBlob: FUNCTION 006c2be0
void TXUIElement::MyUnpackARGB( TUINT32 a_uiColour, TFLOAT& a_rAlpha, TFLOAT& a_rRed, TFLOAT& a_rGreen, TFLOAT& a_rBlue )
{
	a_rBlue  = ( TFLOAT( a_uiColour & 0xFF ) + 0.0f ) / 255.0f;
	a_rGreen = ( TFLOAT( ( a_uiColour >> 8 ) & 0xFF ) + 0.0f ) / 255.0f;
	a_rRed   = ( TFLOAT( ( a_uiColour >> 16 ) & 0xFF ) + 0.0f ) / 255.0f;
	a_rAlpha = ( TFLOAT( a_uiColour >> 24 ) + 0.0f ) / 255.0f;
}

// $deBlob: FUNCTION 006c1af0
void TXUIElement::PushClip( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TFLOAT fHalfHeight = a_fHeight * 0.5f;
	TFLOAT fHalfWidth  = a_fWidth * 0.5f;

	TXUI::GetSingleton()->GetRenderer()->SetScissor( -fHalfWidth, fHalfHeight, fHalfWidth, -fHalfHeight );
	sm_oClipStack.PushBack( TVector2( fHalfWidth, fHalfHeight ) );
}

// $deBlob: FUNCTION 006c1bd0
void TXUIElement::PopClip()
{
	TVector2 vClip = sm_oClipStack.Back();
	TXUI::GetSingleton()->GetRenderer()->ClearScissor( -vClip.x, vClip.y, vClip.x, -vClip.y );
	sm_oClipStack.PopBack();

	if ( sm_oClipStack.Size() > 0 )
	{
		TVector2 vPrevClip = sm_oClipStack.Back();
		TXUI::GetSingleton()->GetRenderer()->SetScissor( -vPrevClip.x, vPrevClip.y, vPrevClip.x, -vPrevClip.y );
	}
}

// $deBlob: FUNCTION 006c1d40
void TXUIElement::UpdateAnchoring( const TVector2& a_rSizeDelta )
{
	for ( T2GUIElementNode* pNode = m_Children.m_pNext; pNode != &m_Children && pNode != TNULL; pNode = pNode->m_pNext )
	{
		T2GUIElement* pGUIElement = TSTATICCAST( T2GUIElement, pNode );
		if ( !pGUIElement->GetFlag( FLAGS_XUIELEMENT ) ) continue;

		TXUIElement* pChild = TSTATICCAST( TXUIElement, pGUIElement );

		if ( pChild->IsA( &TGetClass( TXUIVisual ) ) )
		{
			pChild->UpdateAnchoring( a_rSizeDelta );
			continue;
		}

		TFLOAT fWidth, fHeight;
		pChild->GetDimensions( fWidth, fHeight );

		s_vAnchoringScale.x = 0.0f;
		s_vAnchoringScale.y = ( fHeight == 0.0f ) ? 0.0f : 1.0f / fHeight;

		if ( fWidth != 0.0f )
			s_vAnchoringScale.x = 1.0f / fWidth;

		TUINT32 uiAnchoring = pChild->m_uiXUIState;

		if ( uiAnchoring & Anchor_Right )
		{
			if ( !( uiAnchoring & Anchor_Left ) )
			{
				pChild->m_vPosition.Set( pChild->m_vPosition.GetX() - a_rSizeDelta.x, pChild->m_vPosition.GetY() );
				pChild->m_uiXUIState |= XUIState_PositionDirty;
			}
			else
			{
				fWidth = fWidth - a_rSizeDelta.x / pChild->m_vScale.GetX();
			}
		}

		if ( uiAnchoring & Anchor_Bottom )
		{
			if ( !( uiAnchoring & Anchor_Top ) )
			{
				pChild->m_vPosition.Set( pChild->m_vPosition.GetX(), pChild->m_vPosition.GetY() - a_rSizeDelta.y );
				pChild->m_uiXUIState |= XUIState_PositionDirty;
			}
			else
			{
				fHeight = fHeight - a_rSizeDelta.y / pChild->m_vScale.GetY();
			}
		}

		s_vAnchoringScale.x *= fWidth;
		s_vAnchoringScale.y *= fHeight;

		pChild->SetDimensions( fWidth, fHeight );

		TFLOAT fX, fY;

		if ( uiAnchoring & Anchor_XCenter )
		{
			fY = pChild->m_vPosition.GetY();
			fX = pChild->m_vPosition.GetX() - a_rSizeDelta.x * 0.5f;

			if ( uiAnchoring & Anchor_YCenter )
				fY = fY - a_rSizeDelta.y * 0.5f;
		}
		else
		{
			if ( !( uiAnchoring & Anchor_YCenter ) )
				continue;

			fY = pChild->m_vPosition.GetY() - a_rSizeDelta.y * 0.5f;
			fX = pChild->m_vPosition.GetX();
		}

		pChild->m_vPosition.Set( fX, fY );
		pChild->m_uiXUIState |= XUIState_PositionDirty;
	}
}

// $deBlob: FUNCTION 006c2010
void TXUIElement::UpdateHierarchicalAlpha()
{
	T2GUIElement* pParent = m_pParent;

	if ( pParent != TNULL && pParent->GetFlag( FLAGS_XUIELEMENT ) )
	{
		SetFlags_HierarchicalAlpha( TSTATICCAST( TXUIElement, pParent )->GetFlags_HierarchicalAlpha() * GetFlags_Opacity() );
		return;
	}

	SetXUIStateBits( TUINT32( TMath::Round( GetFlags_Opacity() * 255.0f ) ), 0xFF, XUIState_HierarchicalAlphaShift );
}

TOSHI_NAMESPACE_END
