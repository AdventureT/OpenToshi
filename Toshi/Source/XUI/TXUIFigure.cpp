#include "ToshiPCH.h"
#include "TXUIFigure.h"
#include "TXUI.h"
#include "TXUIRenderer.h"
#include "TXUIShapeCache.h"
#include "GUI/T2GUI.h"
#include "GUI/T2GUIMaterial.h"
#include "GUI/T2GUITransform.h"
#include "Render/TTexture.h"
#include "Platform/DX11/TRender_DX11.h"

#include <malloc.h>
#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiFigure" type info
const TXUITypeInfo XURXUIFigureData::sm_sTypeInfo = { L"XuiFigure", &XURXUIElementData::sm_sTypeInfo };

// Default colour of the strokes and fills (ultramarine)
static TUINT32 s_uiDefaultFigureColour = 0xFF0F0F80;

TUINT32 TXUIFigure::s_uiGradientTextureWidth  = 32;
TUINT32 TXUIFigure::s_uiGradientTextureHeight = 32;
TBOOL   TXUIFigure::s_bRenderAaEdge           = TTRUE;
TBOOL   TXUIFigure::s_bDebugRender            = TFALSE;

//-----------------------------------------------------------------------------
// XURXUIStrokeData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006c53f0
XURXUIStrokeData::XURXUIStrokeData()
{
	m_fStrokeWidth   = 0.0f;
	m_uiStrokeColour = s_uiDefaultFigureColour;
}

// $deBlob: FUNCTION 006c5430
TBOOL XURXUIStrokeData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIObjectData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006c5460
TUINT32 XURXUIStrokeData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIObjectData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006c5490
TBOOL XURXUIStrokeData::ValidateTimelineProp( TUINT32 a_uiPropIndex )
{
	return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006c54a0
TBOOL XURXUIStrokeData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "Stroke.StrokeWidth", -1 ) == 0 )
	{
		a_rPropIndex = Prop_StrokeWidth;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Stroke.StrokeColor", -1 ) == 0 )
	{
		a_rPropIndex = Prop_StrokeColor;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006c54f0
TUINT32 XURXUIStrokeData::GetTimelinePropSize( TUINT32 a_uiPropIndex )
{
	return 4;
}

// $deBlob: FUNCTION 006c5500
TBOOL XURXUIStrokeData::IsFloatPropType( TUINT32 a_uiPropIndex )
{
	return ( a_uiPropIndex == Prop_StrokeWidth ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006c5510
TBOOL XURXUIStrokeData::IsColourPropType( TUINT32 a_uiPropIndex )
{
	return ( a_uiPropIndex == Prop_StrokeColor ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006c5520
TBOOL XURXUIStrokeData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_StrokeWidth ) )
		{
			m_fStrokeWidth = PARSEFLOAT_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_StrokeColor ) )
		{
			m_uiStrokeColour = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUIGradientData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006c55b0
XURXUIGradientData::XURXUIGradientData()
{
	m_uiNumStops      = 0;
	m_auiStopColours  = TNULL;
	m_afStopPositions = TNULL;
	m_uiRadial        = 0;
}

// $deBlob: FUNCTION 006c55e0
// $deBlob: FUNCTION 006c9910
XURXUIGradientData::~XURXUIGradientData()
{
	operator delete( m_auiStopColours );
	operator delete( m_afStopPositions );
}

// $deBlob: FUNCTION 006c5610
TBOOL XURXUIGradientData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIObjectData::Load( a_rResource, a_rData );

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

		if ( uiPropMask & BITFLAG( Prop_Radial ) )
		{
			m_uiRadial = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_NumStops ) )
		{
			TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 16 ) );
			m_uiNumStops = TUINT16( PARSEWORD_BIG( a_rData + 2 ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_StopColours ) )
		{
			TUINT8 uiCount = *a_rData;
			a_rData += 1;

			m_auiStopColours = TSTATICCAST( TUINT32, TMalloc( uiCount * sizeof( TUINT32 ), TXUI::ms_pXUIMemoryBlock ) );

			for ( TUINT32 i = 0; i < uiCount; i++ )
			{
				m_auiStopColours[ i ] = PARSEDWORD_BIG( a_rData );
				a_rData += 4;
			}
		}

		if ( uiPropMask & BITFLAG( Prop_StopPositions ) )
		{
			TUINT8 uiCount = *a_rData;
			a_rData += 1;

			m_afStopPositions = TSTATICCAST( TFLOAT, TMalloc( uiCount * sizeof( TFLOAT ), TXUI::ms_pXUIMemoryBlock ) );

			for ( TUINT32 i = 0; i < uiCount; i++ )
			{
				m_afStopPositions[ i ] = PARSEFLOAT_BIG( a_rData );
				a_rData += 4;
			}
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c57c0
TUINT32 XURXUIGradientData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIObjectData::CalcPileSize( a_rResource, a_rData );

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

		if ( uiPropMask & BITFLAG( Prop_Radial ) )
			a_rData += 1;

		if ( uiPropMask & BITFLAG( Prop_NumStops ) )
			a_rData += 4;

		if ( uiPropMask & BITFLAG( Prop_StopColours ) )
		{
			TUINT32 uiArraySize = *a_rData * sizeof( TUINT32 );
			uiSize += uiArraySize;
			a_rData += uiArraySize + 1;
		}

		if ( uiPropMask & BITFLAG( Prop_StopPositions ) )
		{
			TUINT32 uiArraySize = *a_rData * sizeof( TFLOAT );
			uiSize += uiArraySize;
			a_rData += uiArraySize + 1;
		}
	}

	return uiSize;
}

// $deBlob: FUNCTION 006c5840
TBOOL XURXUIGradientData::ValidateTimelineProp( TUINT32 a_uiPropIndex )
{
	return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006c5850
TBOOL XURXUIGradientData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "Fill.Gradient.Radial", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Radial;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Fill.Gradient.NumStops", -1 ) == 0 )
	{
		a_rPropIndex = Prop_NumStops;
		return TTRUE;
	}

	// NOTE: see Prop
	if ( TStringManager::String8Compare( a_szName, "Fill.Gradient.StopPos", -1 ) == 0 )
	{
		a_rPropIndex = 2;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Fill.Gradient.StopColor", -1 ) == 0 )
	{
		a_rPropIndex = 3;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006c58f0
TUINT32 XURXUIGradientData::GetTimelinePropSize( TUINT32 a_uiPropIndex )
{
	return ( a_uiPropIndex == Prop_Radial ) ? 1 : 4;
}

// $deBlob: FUNCTION 006c5910
TBOOL XURXUIGradientData::IsFloatPropType( TUINT32 a_uiPropIndex )
{
	return ( a_uiPropIndex == Prop_StopPositions ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006c5920
TBOOL XURXUIGradientData::IsColourPropType( TUINT32 a_uiPropIndex )
{
	return ( a_uiPropIndex == Prop_StopColours ) ? TTRUE : TFALSE;
}

//-----------------------------------------------------------------------------
// XURXUIFillData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006c5930
XURXUIFillData::XURXUIFillData()
{
	m_uiFillColour      = s_uiDefaultFigureColour;
	m_iTranslation      = -1;
	m_iScale            = -1;
	m_uiWrapX           = 0;
	m_uiWrapY           = 0;
	m_uiTextureFileName = 0;
	m_uiFillType        = Fill_Solid;
	m_fRotation         = 0.0f;
}

// $deBlob: FUNCTION 006c59d0
TBOOL XURXUIFillData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIObjectData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// NOTE: like retail, it also sets the wraps
// $deBlob: FUNCTION 006c5a00
TUINT32 XURXUIFillData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIObjectData::CalcPileSize( a_rResource, a_rData );

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

		if ( uiPropMask & BITFLAG( Prop_FillType ) )
			a_rData += 4;

		if ( uiPropMask & BITFLAG( Prop_FillColor ) )
			a_rData += 4;

		if ( uiPropMask & BITFLAG( Prop_TextureFileName ) )
			a_rData += 2;

		if ( uiPropMask & BITFLAG( Prop_Gradient ) )
			uiSize += m_oGradient.CalcPileSize( a_rResource, a_rData );

		if ( uiPropMask & BITFLAG( Prop_Translation ) )
			a_rData += 4;

		if ( uiPropMask & BITFLAG( Prop_Scale ) )
			a_rData += 4;

		if ( uiPropMask & BITFLAG( Prop_Rotation ) )
			a_rData += 4;

		if ( uiPropMask & BITFLAG( Prop_WrapX ) )
		{
			m_uiWrapX = TUINT16( PARSEWORD_BIG( a_rData + 2 ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_WrapY ) )
		{
			m_uiWrapY = TUINT16( PARSEWORD_BIG( a_rData + 2 ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_BrushFlags ) )
			a_rData += 4;

		if ( uiPropMask & BITFLAG( Prop_Unknown10 ) )
			a_rData += 4;
	}

	return uiSize;
}

// $deBlob: FUNCTION 006c5af0
TBOOL XURXUIFillData::ValidateTimelineProp( TUINT32 a_uiPropIndex )
{
	return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006c5b00
TBOOL XURXUIFillData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( m_oGradient.TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex ) == TTRUE )
		return TTRUE;

	if ( TStringManager::String8Compare( a_szName, "Fill.FillType", -1 ) == 0 )
	{
		a_rPropIndex = Prop_FillType;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Fill.FillColor", -1 ) == 0 )
	{
		a_rPropIndex = Prop_FillColor;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Fill.TextureFileName", -1 ) == 0 )
	{
		a_rPropIndex = Prop_TextureFileName;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Fill.Translation", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Translation;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Fill.Scale", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Scale;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Fill.Rotation", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Rotation;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Fill.WrapX", -1 ) == 0 )
	{
		a_rPropIndex = Prop_WrapX;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Fill.WrapY", -1 ) == 0 )
	{
		a_rPropIndex = Prop_WrapY;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Fill.BrushFlags", -1 ) == 0 )
	{
		a_rPropIndex = Prop_BrushFlags;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006c5c40
TUINT32 XURXUIFillData::GetTimelinePropSize( TUINT32 a_uiPropIndex )
{
	return ( a_uiPropIndex != Prop_TextureFileName ) ? 4 : 2;
}

// $deBlob: FUNCTION 006c5c60
TBOOL XURXUIFillData::IsFloatPropType( TUINT32 a_uiPropIndex )
{
	return ( a_uiPropIndex == Prop_Rotation ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006c5c70
TBOOL XURXUIFillData::IsColourPropType( TUINT32 a_uiPropIndex )
{
	return ( a_uiPropIndex == Prop_FillColor ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006c5c80
TBOOL XURXUIFillData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT8 uiNumProps = *a_rData;
	a_rData += 1;

	if ( uiNumProps != 0 )
	{
		TUINT32 uiPropMask = 0;

		if ( m_uiPropLevelMask > 0 )
		{
			uiPropMask = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_FillType ) )
		{
			TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 16 ) );
			m_uiFillType = TUINT16( PARSEWORD_BIG( a_rData + 2 ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_FillColor ) )
		{
			m_uiFillColour = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_TextureFileName ) )
		{
			m_uiTextureFileName = TUINT16( PARSEWORD_BIG( a_rData ) );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_Gradient ) )
			m_oGradient.Load( a_rResource, a_rData );

		if ( uiPropMask & BITFLAG( Prop_Translation ) )
		{
			TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 15 ) );
			m_iTranslation = TINT16( PARSEWORD_BIG( a_rData + 2 ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Scale ) )
		{
			TASSERT( PARSEDWORD_BIG( a_rData ) < ( 1 << 15 ) );
			m_iScale = TINT16( PARSEWORD_BIG( a_rData + 2 ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Rotation ) )
		{
			m_fRotation = PARSEFLOAT_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_WrapX ) )
		{
			m_uiWrapX = TUINT16( PARSEWORD_BIG( a_rData + 2 ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_WrapY ) )
		{
			m_uiWrapY = TUINT16( PARSEWORD_BIG( a_rData + 2 ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_BrushFlags ) )
			a_rData += 4;

		if ( uiPropMask & BITFLAG( Prop_Unknown10 ) )
			a_rData += 4;
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XUIFigurePoint
//-----------------------------------------------------------------------------

// The points are equal if their 3 vectors are closer than ~0.22 pixels and they have the same curve flag
// $deBlob: FUNCTION 006c5f10
TBOOL XUIFigurePoint::operator==( const XUIFigurePoint& a_rOther ) const
{
	TFLOAT fDX = m_vPoint.GetX() - a_rOther.m_vPoint.GetX();
	TFLOAT fDY = m_vPoint.GetY() - a_rOther.m_vPoint.GetY();

	if ( fDY * fDY + fDX * fDX <= 0.05f )
	{
		fDX = m_vCtrl1.GetX() - a_rOther.m_vCtrl1.GetX();
		fDY = m_vCtrl1.GetY() - a_rOther.m_vCtrl1.GetY();

		if ( fDY * fDY + fDX * fDX <= 0.05f )
		{
			fDX = m_vCtrl2.GetX() - a_rOther.m_vCtrl2.GetX();
			fDY = m_vCtrl2.GetY() - a_rOther.m_vCtrl2.GetY();

			if ( fDY * fDY + fDX * fDX <= 0.05f )
				return m_bCurve == a_rOther.m_bCurve ? TTRUE : TFALSE;
		}
	}

	return TFALSE;
}

//-----------------------------------------------------------------------------
// XURXUIFigureData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006c6040
const TXUITypeInfo* XURXUIFigureData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// NOTE: like retail, m_uiClosed isn't initialised
// $deBlob: FUNCTION 006c6050
XURXUIFigureData::XURXUIFigureData()
    : m_vDims( TVector2::VEC_ZERO )
{
	m_uiNumPoints = 0;
	m_pPoints     = TNULL;
	m_pClass      = TClass::Find( "TXUIFigure", &TGetClass( TXUIElement ) );
}

// $deBlob: FUNCTION 006c6150
// $deBlob: FUNCTION 006c9890
XURXUIFigureData::~XURXUIFigureData()
{
	operator delete( m_pPoints );
}

// $deBlob: FUNCTION 006c61c0
TBOOL XURXUIFigureData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );

	TUINT8 uiNumProps = *a_rData;
	a_rData += 1;

	if ( uiNumProps != 0 )
	{
		// NOTE: unlike the other classes, the mask is always there
		TUINT8 uiPropMask = *a_rData;
		a_rData += 1;

		if ( uiPropMask & BITFLAG( Prop_Stroke ) )
			m_oStroke.Load( a_rResource, a_rData );

		if ( uiPropMask & BITFLAG( Prop_Fill ) )
			m_oFill.Load( a_rResource, a_rData );

		if ( uiPropMask & BITFLAG( Prop_Closed ) )
		{
			m_uiClosed = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_Points ) )
		{
			// The points are in the custom data: size, the figure's size, the number of points, the points
			TUINT32 uiOffset = PARSEDWORD_BIG( a_rData );
			a_rData += 4;

			const TUINT8* pCustData  = a_rResource.GetCust( uiOffset );
			TUINT32       uiCustSize = PARSEDWORD_BIG( pCustData );
			const TUINT8* pStart     = pCustData + 4;

			m_vDims.Set( PARSEFLOAT_BIG( pCustData + 4 ), PARSEFLOAT_BIG( pCustData + 8 ) );
			pCustData += 12;

			TASSERT( PARSEDWORD_BIG( pCustData ) < ( 1 << 16 ) );
			m_uiNumPoints = TUINT16( PARSEWORD_BIG( pCustData + 2 ) );
			pCustData += 4;

			if ( m_uiNumPoints != 0 )
			{
				m_pPoints = new ( TXUI::ms_pXUIMemoryBlock ) XUIFigurePoint[ m_uiNumPoints ];

				for ( TUINT32 i = 0; i < m_uiNumPoints; i++ )
				{
					m_pPoints[ i ].m_vPoint.Set( PARSEFLOAT_BIG( pCustData + 0 ), PARSEFLOAT_BIG( pCustData + 4 ) );
					m_pPoints[ i ].m_vCtrl1.Set( PARSEFLOAT_BIG( pCustData + 8 ), PARSEFLOAT_BIG( pCustData + 12 ) );
					m_pPoints[ i ].m_vCtrl2.Set( PARSEFLOAT_BIG( pCustData + 16 ), PARSEFLOAT_BIG( pCustData + 20 ) );
					pCustData += 24;
				}

				// A point is a curve if its first control isn't on it or on the previous point's second control
				for ( TINT i = 0; i < m_uiNumPoints; i++ )
				{
					TINT iPrev = i - 1;

					if ( iPrev < 0 )
						iPrev = m_uiNumPoints + iPrev;

					XUIFigurePoint& rPoint = m_pPoints[ i ];
					XUIFigurePoint& rPrev  = m_pPoints[ iPrev ];

					TFLOAT fDX = rPoint.m_vCtrl1.GetX() - rPoint.m_vPoint.GetX();
					TFLOAT fDY = rPoint.m_vCtrl1.GetY() - rPoint.m_vPoint.GetY();

					if ( 1.0f < fDY * fDY + fDX * fDX )
					{
						rPoint.m_bCurve = TTRUE;
						continue;
					}

					fDX = rPoint.m_vCtrl1.GetX() - rPrev.m_vCtrl2.GetX();
					fDY = rPoint.m_vCtrl1.GetY() - rPrev.m_vCtrl2.GetY();

					rPoint.m_bCurve = ( 1.0f < fDY * fDY + fDX * fDX );
				}
			}

			TASSERT( (TUINT32)( pCustData - pStart ) == uiCustSize );
		}
	}

	return TTRUE;
}

// NOTE: like retail, it also sets m_uiClosed and m_uiNumPoints
// $deBlob: FUNCTION 006c66c0
TUINT32 XURXUIFigureData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );

	TUINT8 uiNumProps = *a_rData;
	a_rData += 1;

	if ( uiNumProps != 0 )
	{
		TUINT8 uiPropMask = *a_rData;
		a_rData += 1;

		if ( uiPropMask & BITFLAG( Prop_Stroke ) )
			uiSize += m_oStroke.CalcPileSize( a_rResource, a_rData );

		if ( uiPropMask & BITFLAG( Prop_Fill ) )
			uiSize += m_oFill.CalcPileSize( a_rResource, a_rData );

		if ( uiPropMask & BITFLAG( Prop_Closed ) )
		{
			m_uiClosed = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_Points ) )
		{
			TUINT32 uiOffset = PARSEDWORD_BIG( a_rData );
			a_rData += 4;

			const TUINT8* pCustData = a_rResource.GetCust( uiOffset );
			TASSERT( PARSEDWORD_BIG( pCustData + 12 ) < ( 1 << 16 ) );
			m_uiNumPoints = TUINT16( PARSEWORD_BIG( pCustData + 14 ) );

			if ( m_uiNumPoints != 0 )
				uiSize += m_uiNumPoints * sizeof( XUIFigurePoint ) + 4;
		}
	}

	return uiSize;
}

// $deBlob: FUNCTION 006c6830
TBOOL XURXUIFigureData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006c68a0
TBOOL XURXUIFigureData::ValidateTimelineSubProp( TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex )
{
	if ( a_uiPropIndex == SubObject_Stroke )
		return m_oStroke.ValidateTimelineProp( a_uiSubIndex );

	if ( a_uiPropIndex == SubObject_Gradient )
		return m_oFill.m_oGradient.ValidateTimelineProp( a_uiSubIndex );

	return m_oFill.ValidateTimelineProp( a_uiSubIndex );
}

// $deBlob: FUNCTION 006c68f0
TBOOL XURXUIFigureData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( m_oStroke.TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex ) )
		return TTRUE;

	if ( m_oFill.TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex ) )
		return TTRUE;

	if ( TStringManager::String8Compare( a_szName, "Closed", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Closed;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Points", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Points;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006c6990
TUINT32 XURXUIFigureData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex == Prop_Closed ) ? 1 : 4;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006c6a00
TUINT32 XURXUIFigureData::GetTimelineSubPropSize( TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex )
{
	if ( a_uiPropIndex == SubObject_Stroke )
		return m_oStroke.GetTimelinePropSize( a_uiSubIndex );

	if ( a_uiPropIndex == SubObject_Gradient )
		return m_oFill.m_oGradient.GetTimelinePropSize( a_uiSubIndex );

	return m_oFill.GetTimelinePropSize( a_uiSubIndex );
}

// $deBlob: FUNCTION 006c6a40
TBOOL XURXUIFigureData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006c6aa0
TBOOL XURXUIFigureData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006c6b00
TBOOL XURXUIFigureData::IsFloatSubPropType( TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex )
{
	if ( a_uiPropIndex == SubObject_Stroke )
		return m_oStroke.IsFloatPropType( a_uiSubIndex );

	if ( a_uiPropIndex == SubObject_Gradient )
		return m_oFill.m_oGradient.IsFloatPropType( a_uiSubIndex );

	return m_oFill.IsFloatPropType( a_uiSubIndex );
}

// $deBlob: FUNCTION 006c6b50
TBOOL XURXUIFigureData::IsColourSubPropType( TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex )
{
	if ( a_uiPropIndex == SubObject_Stroke )
		return m_oStroke.IsColourPropType( a_uiSubIndex );

	if ( a_uiPropIndex == SubObject_Gradient )
		return m_oFill.m_oGradient.IsColourPropType( a_uiSubIndex );

	return m_oFill.IsColourPropType( a_uiSubIndex );
}

//-----------------------------------------------------------------------------
// TXUIFigure
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006c6ba0
// $deBlob: FUNCTION 006c6bb0
// $deBlob: FUNCTION 006c6be0
// $deBlob: FUNCTION 006c6c00
// $deBlob: FUNCTION 006c6c10
// $deBlob: FUNCTION 0053eea0
// $deBlob: FUNCTION 00859070
TXUI_DEFINE_CLASS( TXUIFigure );

// NOTE: like retail, m_uiStrokeColour isn't initialised (Create sets it)
// $deBlob: FUNCTION 006c6c20
TXUIFigure::TXUIFigure()
    : m_vFigureDims( TVector2::VEC_ZERO )
    , m_vDimsScale( 1.0f, 1.0f )
    , m_vShapeScale( 1.0f, 1.0f )
    , m_vFillScale( 1.0f, 1.0f )
{
	m_uiFigureFlags        = 0;
	m_pUVs                 = TNULL;
	m_pMaterial            = TNULL;
	m_pGradientTexture     = TNULL;
	m_pGradientTextureInfo = TNULL;
	m_fStrokeWidth         = 0.0f;
	m_pStrokeVerts         = TNULL;
	m_uiNumStrokeVerts     = 0;
	m_uiNumStops           = 0;
	m_auiFillStopColours   = TNULL;
	m_afFillStopPositions  = TNULL;
	m_pShape               = TNULL;
	m_pAaEdgeShape         = TNULL;
	m_pGradientTextureData = TNULL;
	m_fFillRotation        = 0.0f;
	m_uiFigureFlags |= FigureFlags_Unknown0 | ( XURXUIFillData::Fill_Solid << FigureFlags_FillTypeShift );
	m_pFillTexture       = TNULL;
	m_wszFillTextureName = TNULL;
	m_uiFillWrapX        = 0;
	m_uiFillWrapY        = 0;
	m_uiFillColour       = 0xFFFFFFFF;
	m_oFillTransform.SetIdentity();
}

// $deBlob: FUNCTION 006c6d70
// $deBlob: FUNCTION 006c9830
// $deBlob: FUNCTION 006c9828
TXUIFigure::~TXUIFigure()
{
	if ( m_pGradientTexture )
	{
		if ( ID3D11ShaderResourceView* pSRView = m_pGradientTexture->GetHandle() )
			pSRView->Release();

		delete m_pGradientTextureInfo;
		TTextureManager::GetSingleton()->DestroyTexture( m_pGradientTexture );
		m_pGradientTexture = TNULL;
	}

	if ( m_pMaterial )
	{
		m_pMaterial->OnDestroy();
		delete m_pMaterial;
	}

	delete[] m_pUVs;
	delete[] m_auiFillStopColours;
	delete[] m_afFillStopPositions;
	delete[] m_pStrokeVerts;
	TFree( m_pGradientTextureData );

	if ( m_pShape )
		TXUIShapeCache::GetSingleton()->DestroyShape( m_pShape );

	if ( m_pAaEdgeShape )
	{
		delete m_pAaEdgeShape;
		m_pAaEdgeShape = TNULL;
	}
}

// $deBlob: FUNCTION 006c6e50
TBOOL TXUIFigure::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, a_bCreateChildren );

	XURXUIFigureData* pData = TSTATICCAST( XURXUIFigureData, a_pData );

	SetFigureFlag( FigureFlags_Closed, pData->m_uiClosed == 1 );

	TBOOL bRes = CreateFill( a_rResource, &pData->m_oFill );
	TASSERT( TTRUE == bRes );

	m_fStrokeWidth   = pData->m_oStroke.m_fStrokeWidth;
	m_uiStrokeColour = pData->m_oStroke.m_uiStrokeColour;

	m_vFigureDims.m_iX = m_iWidth;
	m_vFigureDims.m_iY = m_iHeight;

	if ( pData->m_pPoints[ 0 ].m_bCurve == TFALSE && pData->m_pPoints[ pData->m_uiNumPoints - 1 ].m_bCurve == TFALSE )
		m_uiFigureFlags &= ~FigureFlags_Curved;
	else
		m_uiFigureFlags |= FigureFlags_Curved;

	m_uiFillWrapX = pData->m_oFill.m_uiWrapX;
	m_uiFillWrapY = pData->m_oFill.m_uiWrapY;

	if ( pData->m_uiNumPoints >= 3 )
	{
		TXUIShapeParams* pParams = new ( TXUI::ms_pXUIMemoryBlock ) TXUIShapeParams;
		pParams->m_pPoints       = pData->m_pPoints;
		pParams->m_uiNumPoints   = pData->m_uiNumPoints;

		m_pShape = TXUIShapeCache::GetSingleton()->CreateShape( pParams );
		m_vShapeScale.Set( m_vFigureDims.GetX() / m_pShape->m_vDims.GetX(), m_vFigureDims.GetY() / m_pShape->m_vDims.GetY() );

		if ( m_fStrokeWidth > 0.0f )
		{
			TASSERT( m_pShape->m_uiNumVerts * 4 + 2 < ( 1 << 16 ) );
			m_uiNumStrokeVerts = TUINT16( m_pShape->m_uiNumVerts * 4 + 2 );
			m_pStrokeVerts     = new ( TXUI::ms_pXUIMemoryBlock ) TVector2[ m_uiNumStrokeVerts ];

			TVector2 vScale( m_vDimsScale.GetX() * m_vShapeScale.GetX(), m_vDimsScale.GetY() * m_vShapeScale.GetY() );
			bRes = m_pShape->TesselateStroke( m_fStrokeWidth, m_pStrokeVerts, vScale );
			TASSERT( TTRUE == bRes );
		}
		else if ( GetFillType() == XURXUIFillData::Fill_Solid )
		{
			TVector2 vScale( m_vDimsScale.GetX() * m_vShapeScale.GetX(), m_vDimsScale.GetY() * m_vShapeScale.GetY() );
			m_pAaEdgeShape = m_pShape->CloneAndMakeAaEdge( vScale, m_uiColour );
		}

		SetupUVs();
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c71e0
void TXUIFigure::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TASSERT( m_vFigureDims.GetX() != 0.0f && m_vFigureDims.GetY() != 0.0f );
	m_vDimsScale.SetX( a_fWidth / m_vFigureDims.GetX() );
	m_vDimsScale.SetY( a_fHeight / m_vFigureDims.GetY() );

	if ( 0.0f < m_fStrokeWidth )
		m_uiFigureFlags |= FigureFlags_StrokeDirty;

	m_uiFigureFlags |= FigureFlags_UVsDirty | FigureFlags_AaEdgeDirty;
	TXUIElement::SetDimensions( a_fWidth, a_fHeight );
}

// $deBlob: FUNCTION 006c72f0
void TXUIFigure::SetFillStopPos( TUINT32 a_uiIndex, TFLOAT a_fPosition )
{
	if ( a_uiIndex < m_uiNumStops && a_fPosition != m_afFillStopPositions[ a_uiIndex ] )
	{
		m_afFillStopPositions[ a_uiIndex ] = a_fPosition;

		XURXUIFillData::Fill eFillType = GetFillType();

		if ( eFillType == XURXUIFillData::Fill_LinearGradient || eFillType == XURXUIFillData::Fill_RadialGradient )
			m_uiFigureFlags |= FigureFlags_GradientDirty;
	}
}

// $deBlob: FUNCTION 006c7340
void TXUIFigure::SetFillStopColour( TUINT32 a_uiIndex, TUINT32 a_uiColour )
{
	if ( a_uiIndex < m_uiNumStops && a_uiColour != m_auiFillStopColours[ a_uiIndex ] )
	{
		m_auiFillStopColours[ a_uiIndex ] = a_uiColour;

		XURXUIFillData::Fill eFillType = GetFillType();

		if ( eFillType == XURXUIFillData::Fill_LinearGradient || eFillType == XURXUIFillData::Fill_RadialGradient )
			m_uiFigureFlags |= FigureFlags_GradientDirty;
	}
}

// $deBlob: FUNCTION 006c7390
void TXUIFigure::SetFillRotation( TFLOAT a_fDegrees )
{
	m_uiFigureFlags |= FigureFlags_UVsDirty | FigureFlags_FillTransformDirty;
	m_fFillRotation = a_fDegrees * ( TMath::PI / 180.0f );
}

// $deBlob: FUNCTION 006c73b0
void TXUIFigure::SetFillType( XURXUIFillData::Fill a_eType )
{
	TUINT32 uiOldFlags = m_uiFigureFlags;

	if ( a_eType == GetFillType() )
		return;

	switch ( a_eType )
	{
		case XURXUIFillData::Fill_None:
			if ( m_pMaterial )
				m_pMaterial->SetTextureResource( TNULL );

			m_uiColour = 0;
			m_uiFigureFlags |= FigureFlags_ColourDirty;
			break;
		case XURXUIFillData::Fill_Solid:
			if ( m_pMaterial )
				m_pMaterial->SetTextureResource( TNULL );

			break;
		case XURXUIFillData::Fill_LinearGradient:
			if ( m_auiFillStopColours && m_afFillStopPositions )
			{
				m_uiFigureFlags = uiOldFlags | FigureFlags_UVsDirty;

				// The radial gradient texture has to be remade
				if ( ( uiOldFlags & FigureFlags_RadialGradient ) || m_pMaterial == TNULL )
					m_uiFigureFlags = uiOldFlags | FigureFlags_UVsDirty | FigureFlags_GradientDirty;

				m_uiColour      = 0xFFFFFFFF;
				m_uiFigureFlags = ( m_uiFigureFlags & ~FigureFlags_RadialGradient ) | FigureFlags_ColourDirty;
			}

			break;
		case XURXUIFillData::Fill_RadialGradient:
			if ( m_auiFillStopColours && m_afFillStopPositions )
			{
				m_uiFigureFlags = uiOldFlags | FigureFlags_UVsDirty;

				// The linear gradient texture has to be remade
				if ( ( uiOldFlags & FigureFlags_RadialGradient ) == 0 || m_pMaterial == TNULL )
					m_uiFigureFlags = uiOldFlags | FigureFlags_UVsDirty | FigureFlags_GradientDirty;

				m_uiFigureFlags |= FigureFlags_RadialGradient | FigureFlags_ColourDirty;
				m_uiColour = 0xFFFFFFFF;
			}

			break;
		case XURXUIFillData::Fill_Texture:
			if ( m_pMaterial )
				m_pMaterial->SetTextureResource( m_pFillTexture );

			m_uiColour = 0xFFFFFFFF;
			m_uiFigureFlags |= FigureFlags_ColourDirty;
			break;
	}

	TASSERT( (TUINT32)a_eType < XURXUIFillData::Fill_Mask );
	m_uiFigureFlags = ( m_uiFigureFlags & ~FigureFlags_FillTypeMask ) | ( a_eType << FigureFlags_FillTypeShift );
}

// Strips the folders of a texture path and lowers it (the texture names of the TRBs)
static void StripFillTextureName( TString8& a_rName )
{
	TINT iSlash  = a_rName.FindReverse( '\\', -1 );
	TINT iLength = a_rName.Length();
	TINT iCount  = iLength - ( iSlash + 1 );
	a_rName      = a_rName.Mid( iLength - iCount, iCount );
	a_rName.MakeLower();
}

// NOTE: like retail, it expects m_pMaterial to exist
// $deBlob: FUNCTION 006c7510
void TXUIFigure::SetFillTexture( const TWCHAR* a_wszTextureName )
{
	if ( a_wszTextureName != m_wszFillTextureName )
	{
		m_uiFigureFlags |= FigureFlags_ColourDirty;
		m_wszFillTextureName = a_wszTextureName;
		m_uiColour           = 0xFFFFFFFF;

		TString8 strName( a_wszTextureName );

		if ( strName.Length() != 0 )
		{
			StripFillTextureName( strName );

			m_pFillTexture = TTextureManager::GetSingleton()->FindTexture( strName.GetString() );
			m_pMaterial->SetTextureResource( m_pFillTexture );
			m_uiFigureFlags |= FigureFlags_UVsDirty;
			return;
		}

		m_pFillTexture = TTextureManager::GetSingleton()->GetInvalidTexture();
	}
}

// $deBlob: FUNCTION 006c7640
void TXUIFigure::SetWidth( TFLOAT a_fWidth )
{
	m_vDimsScale.SetX( a_fWidth / m_vFigureDims.GetX() );

	if ( 0.0f < m_fStrokeWidth )
		m_uiFigureFlags |= FigureFlags_StrokeDirty;

	m_uiFigureFlags |= FigureFlags_UVsDirty | FigureFlags_AaEdgeDirty;
	TXUIElement::SetWidth( a_fWidth );
}

// $deBlob: FUNCTION 006c76a0
void TXUIFigure::SetHeight( TFLOAT a_fHeight )
{
	m_vDimsScale.SetY( a_fHeight / m_vFigureDims.GetY() );

	if ( 0.0f < m_fStrokeWidth )
		m_uiFigureFlags |= FigureFlags_StrokeDirty;

	m_uiFigureFlags |= FigureFlags_UVsDirty | FigureFlags_AaEdgeDirty;
	TXUIElement::SetHeight( a_fHeight );
}

// $deBlob: FUNCTION 006c7700
void TXUIFigure::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == XURXUIFigureData::Prop_Closed )
			SetFigureFlag( FigureFlags_Closed, a_uiValue1 == 1 );

		return;
	}

	TASSERT( a_uiObjectIndex > 0 );
	TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// $deBlob: FUNCTION 006c7790
void TXUIFigure::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex == 0 )
		return;

	TASSERT( a_uiObjectIndex > 0 );
	TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006c7810
void TXUIFigure::UpdateTimelineSubProp( TXUIResource* a_pResource, TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar, TINT a_iIndex )
{
	if ( a_uiPropIndex == XURXUIFigureData::SubObject_Stroke )
	{
		if ( a_uiSubIndex == XURXUIStrokeData::Prop_StrokeColor )
			SetStrokeColour( InterpColour( a_uiValue1, a_uiValue2, a_fScalar ) );
	}
	else if ( a_uiPropIndex == XURXUIFigureData::SubObject_Gradient )
	{
		if ( a_uiSubIndex == XURXUIGradientData::Prop_StopColours )
			SetFillStopColour( TUINT32( a_iIndex ), InterpColour( a_uiValue1, a_uiValue2, a_fScalar ) );
	}
	else
	{
		switch ( a_uiSubIndex )
		{
			case XURXUIFillData::Prop_FillType:
				SetFillType( XURXUIFillData::Fill( a_uiValue1 ) );
				return;
			case XURXUIFillData::Prop_FillColor:
			{
				TUINT32 uiColour = InterpColour( a_uiValue1, a_uiValue2, a_fScalar );
				m_uiFigureFlags |= FigureFlags_ColourDirty;
				m_uiFillColour = uiColour;
				return;
			}
			case XURXUIFillData::Prop_TextureFileName:
				SetFillTexture( a_pResource->GetString( TUINT16( a_uiValue1 ) ) );
				return;
			case XURXUIFillData::Prop_Translation:
			{
				TVector2 vTranslation = *a_pResource->GetVector( TINT( a_uiValue1 ) );
				TVector2 vTo          = *a_pResource->GetVector( TINT( a_uiValue2 ) );
				vTranslation.Lerp( vTranslation, vTo, a_fScalar );

				m_oFillTransform.m_Pos = vTranslation;
				m_uiFigureFlags |= FigureFlags_UVsDirty | FigureFlags_FillTransformDirty;
				return;
			}
			case XURXUIFillData::Prop_Scale:
			{
				TVector2 vScale = *a_pResource->GetVector( TINT( a_uiValue1 ) );
				TVector2 vTo    = *a_pResource->GetVector( TINT( a_uiValue2 ) );
				vScale.Lerp( vScale, vTo, a_fScalar );

				m_vFillScale = vScale;
				m_uiFigureFlags |= FigureFlags_UVsDirty | FigureFlags_FillTransformDirty;
				return;
			}
		}
	}
}

// $deBlob: FUNCTION 006c79f0
void TXUIFigure::UpdateTimelineSubProp( TXUIResource* a_pResource, TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar, TINT a_iIndex )
{
	if ( a_uiPropIndex == XURXUIFigureData::SubObject_Stroke )
	{
		if ( a_uiSubIndex == XURXUIStrokeData::Prop_StrokeWidth )
			SetStrokeWidth( TMath::LERP( a_fValue1, a_fValue2, a_fScalar ) );
	}
	else if ( a_uiPropIndex == XURXUIFigureData::SubObject_Gradient )
	{
		if ( a_uiSubIndex == XURXUIGradientData::Prop_StopPositions )
		{
			TMath::Clip( a_fValue1, 0.0f, 1.0f );
			TMath::Clip( a_fValue2, 0.0f, 1.0f );
			SetFillStopPos( TUINT32( a_iIndex ), TMath::LERP( a_fValue1, a_fValue2, a_fScalar ) );
		}
	}
	else if ( a_uiSubIndex == XURXUIFillData::Prop_Rotation )
	{
		SetFillRotation( TMath::LERP( a_fValue1, a_fValue2, a_fScalar ) );
	}
}

// $deBlob: FUNCTION 006c7b00
void TXUIFigure::SetAlpha( TFLOAT a_fAlpha )
{
	if ( GetFillType() == XURXUIFillData::Fill_Solid && TINT( GetAlpha() * 255.0f ) != TINT( a_fAlpha * 255.0f ) )
		m_uiFigureFlags |= FigureFlags_ColourDirty;

	TXUIElement::SetAlpha( a_fAlpha );
}

// $deBlob: FUNCTION 006c7b80
void TXUIFigure::PreRender()
{
	if ( m_uiFigureFlags & FigureFlags_GradientDirty )
	{
		CreateFillGradientTexture( ( ( m_uiFigureFlags >> FigureFlags_FillTypeShift ) & 1 ) != 0 );
		m_uiFigureFlags &= ~FigureFlags_GradientDirty;
	}

	if ( ( m_uiFigureFlags & FigureFlags_StrokeDirty ) && m_pShape )
	{
		TVector2 vScale( m_vDimsScale.GetX() * m_vShapeScale.GetX(), m_vDimsScale.GetY() * m_vShapeScale.GetY() );
		m_pShape->TesselateStroke( m_fStrokeWidth, m_pStrokeVerts, vScale );
		m_uiFigureFlags &= ~FigureFlags_StrokeDirty;
	}

	if ( m_uiFigureFlags & FigureFlags_FillTransformDirty )
	{
		TVector2 vTranslation = m_oFillTransform.m_Pos;

		m_oFillTransform.SetIdentity();
		m_oFillTransform.Rotate( m_fFillRotation );

		T2GUITransform oScale;
		oScale.m_Pos.x      = 0.0f;
		oScale.m_Pos.y      = 0.0f;
		oScale.m_Rot[ 0 ].m_X = T2GUITransform::Float( TINT( m_vFillScale.GetX() * 256.0f ) );
		oScale.m_Rot[ 0 ].m_Y = 0;
		oScale.m_Rot[ 1 ].m_X = 0;
		oScale.m_Rot[ 1 ].m_Y = T2GUITransform::Float( TINT( m_vFillScale.GetY() * 256.0f ) );
		m_oFillTransform.PreMultiply( oScale );

		m_oFillTransform.m_Pos = vTranslation;
		m_uiFigureFlags &= ~FigureFlags_FillTransformDirty;
	}

	if ( m_uiFigureFlags & FigureFlags_UVsDirty )
	{
		SetupUVs();
		m_uiFigureFlags &= ~FigureFlags_UVsDirty;
	}

	if ( ( m_uiFigureFlags & FigureFlags_AaEdgeDirty ) && m_pShape && m_pAaEdgeShape )
	{
		TVector2 vScale( m_vDimsScale.GetX() * m_vShapeScale.GetX(), m_vDimsScale.GetY() * m_vShapeScale.GetY() );
		m_pShape->MakeAaEdgeVerts( m_pAaEdgeShape, vScale );
		m_uiFigureFlags &= ~FigureFlags_AaEdgeDirty;
	}

	TXUIElement::PreRender();
}

// $deBlob: FUNCTION 006c7df0
void TXUIFigure::Render()
{
	if ( m_vScale.GetX() == 0.0f || m_vScale.GetY() == 0.0f )
		return;

	if ( m_pShape && m_pShape->m_pVerts )
	{
		TASSERT( m_pShape->m_uiNumVerts >= 3 );

		TXUIRenderer* pRenderer = TXUI::GetSingleton()->GetRenderer();
		TFLOAT        fAlpha    = GetAlpha();

		if ( GetFillType() == XURXUIFillData::Fill_Solid )
		{
			TUINT32 uiColour = ( TUINT32( TFLOAT( m_uiFillColour >> 24 ) / 255.0f * fAlpha * 255.0f ) << 24 ) | ( m_uiFillColour & 0x00FFFFFF );

			if ( m_uiColour != uiColour )
			{
				m_uiColour = uiColour;
				m_uiFigureFlags |= FigureFlags_ColourDirty;
			}
		}

		if ( ( m_uiFigureFlags & FigureFlags_ColourDirty ) && m_pShape && m_pAaEdgeShape )
		{
			m_pAaEdgeShape->SetColorAaEdge( m_uiColour );
			m_uiFigureFlags &= ~FigureFlags_ColourDirty;
		}

		pRenderer->SetColour( m_uiColour );
		pRenderer->SetMaterial( ( m_pMaterial && m_pMaterial->GetTextureResource() && GetFillType() != XURXUIFillData::Fill_Solid ) ? m_pMaterial : TNULL );

		TFLOAT fScaleX = m_vShapeScale.GetX() * m_vDimsScale.GetX();
		TFLOAT fScaleY = m_vShapeScale.GetY() * m_vDimsScale.GetY();

		TUINT32 uiNumVerts = m_pShape->m_uiNumVerts;
		TUINT32 uiMaxVerts = uiNumVerts;

		if ( m_pAaEdgeShape )
		{
			TUINT32 uiNumAaVerts = m_pAaEdgeShape->m_uiNumVerts;
			uiMaxVerts           = ( uiNumVerts >= uiNumAaVerts ) ? uiNumVerts : uiNumAaVerts;
		}

		TVector2* pVerts = TSTATICCAST( TVector2, _alloca( uiMaxVerts * sizeof( TVector2 ) ) );
		TVector2* pUVs   = TSTATICCAST( TVector2, _alloca( uiMaxVerts * sizeof( TVector2 ) ) );

		for ( TINT i = 0; i < TINT( uiNumVerts ); i++ )
			m_pShape->m_pVerts[ i ].Get( pVerts[ i ] );

		// The fill
		if ( m_pShape->m_pIndices && ( m_uiFigureFlags & FigureFlags_FillTypeMask ) )
		{
			TVector2* pFillUVs = TNULL;

			if ( m_pMaterial && m_pMaterial->GetTextureResource() && m_pUVs )
			{
				for ( TINT i = 0; i < m_pShape->m_uiNumVerts; i++ )
					m_pUVs[ i ].Get( pUVs[ i ] );

				pFillUVs = pUVs;
			}

			pRenderer->RenderIndexedTriList( pVerts, pFillUVs, m_pShape->m_pIndices, m_pShape->m_uiNumIndices, 2, m_pShape->m_uiNumVerts, fScaleX, fScaleY );
		}

		// The anti-aliasing edge
		if ( s_bRenderAaEdge && m_pAaEdgeShape )
		{
			TVector2* pAaUVs = TNULL;

			if ( m_pAaEdgeShape->m_pUVs )
			{
				for ( TINT i = 0; i < m_pAaEdgeShape->m_uiNumVerts; i++ )
					m_pAaEdgeShape->m_pUVs[ i ].Get( pUVs[ i ] );

				pAaUVs = pUVs;
			}

			for ( TINT i = 0; i < m_pAaEdgeShape->m_uiNumVerts; i++ )
				m_pAaEdgeShape->m_pVerts[ i ].Get( pVerts[ i ] );

			pRenderer->RenderIndexedTriStripColoursList( pVerts, pAaUVs, m_pAaEdgeShape->m_pColours, m_pAaEdgeShape->m_pIndices, m_pAaEdgeShape->m_uiNumIndices, 2, m_pAaEdgeShape->m_uiNumVerts, 1.0f, 1.0f );
		}

		pRenderer->SetMaterial( TNULL );

		// The stroke
		if ( m_pStrokeVerts )
		{
			pRenderer->SetColour( ( TUINT32( fAlpha * 255.0f ) << 24 ) | ( m_uiStrokeColour & 0x00FFFFFF ) );

			if ( m_pStrokeVerts )
			{
				TUINT32 uiNumStrokeVerts;

				if ( m_uiFigureFlags & FigureFlags_Closed )
					uiNumStrokeVerts = m_uiNumStrokeVerts;
				else if ( m_uiFigureFlags & FigureFlags_Curved )
					uiNumStrokeVerts = m_uiNumStrokeVerts - TXUIShape::ms_iSplineSteps * 4 - 1;
				else
					uiNumStrokeVerts = m_uiNumStrokeVerts - 5;

				pRenderer->RenderTriStrip( m_pStrokeVerts, TNULL, uiNumStrokeVerts, 1.0f, 1.0f );
			}
		}

		if ( s_bDebugRender )
		{
			pRenderer->SetColour( 0xFFFFFFFF );

			TVector2* pDebugVerts = TSTATICCAST( TVector2, _alloca( m_pShape->m_uiNumVerts * sizeof( TVector2 ) ) );

			for ( TINT i = 0; i < m_pShape->m_uiNumVerts; i++ )
				m_pShape->m_pVerts[ i ].Get( pDebugVerts[ i ] );

			// The outline
			for ( TUINT32 i = 0; i < TUINT32( m_pShape->m_uiNumVerts - 1 ); i++ )
			{
				TVector2 vStart( pDebugVerts[ i ].x * fScaleX, pDebugVerts[ i ].y * fScaleY );
				TVector2 vEnd( pDebugVerts[ i + 1 ].x * fScaleX, pDebugVerts[ i + 1 ].y * fScaleY );
				pRenderer->RenderLine( vStart, vEnd );
			}

			{
				TVector2 vStart( pDebugVerts[ m_pShape->m_uiNumVerts - 1 ].x * fScaleX, pDebugVerts[ m_pShape->m_uiNumVerts - 1 ].y * fScaleY );
				TVector2 vEnd( pDebugVerts[ 0 ].x * fScaleX, pDebugVerts[ 0 ].y * fScaleY );
				pRenderer->RenderLine( vStart, vEnd );
			}

			// An edge of each triangle
			for ( TINT i = 0; i < m_pShape->m_uiNumTriangles; i++ )
			{
				TUINT16  uiFirst = m_pShape->m_pIndices[ i * 3 ];
				TUINT16  uiThird = m_pShape->m_pIndices[ i * 3 + 2 ];
				TVector2 vEnd( pDebugVerts[ uiThird ].x * fScaleX, pDebugVerts[ uiThird ].y * fScaleY );
				TVector2 vStart( pDebugVerts[ uiFirst ].x * fScaleX, pDebugVerts[ uiFirst ].y * fScaleY );
				pRenderer->RenderLine( vStart, vEnd );
			}

			// The stroke
			if ( m_pStrokeVerts )
			{
				for ( TINT i = 0; i < m_uiNumStrokeVerts - 1; i++ )
					pRenderer->RenderLine( m_pStrokeVerts[ i ], m_pStrokeVerts[ i + 1 ] );

				pRenderer->RenderLine( m_pStrokeVerts[ m_uiNumStrokeVerts - 1 ], m_pStrokeVerts[ 0 ] );
			}
		}

		pRenderer->SetColour( m_uiColour );
		pRenderer->SetMaterial( TNULL );
	}

	T2GUIElement::Render();
}

// $deBlob: FUNCTION 006c85b0
TBOOL TXUIFigure::CreateFill( TXUIResource& a_rResource, XURXUIFillData* a_pFillData )
{
	m_uiFigureFlags |= FigureFlags_ColourDirty;
	m_uiFillColour = a_pFillData->m_uiFillColour;

	TUINT32 uiFillType = a_pFillData->m_uiFillType;

	if ( uiFillType == XURXUIFillData::Fill_LinearGradient || uiFillType == XURXUIFillData::Fill_RadialGradient )
	{
		TBOOL bRes = CreateGradient( &a_pFillData->m_oGradient, uiFillType == XURXUIFillData::Fill_RadialGradient );
		TASSERT( TTRUE == bRes );
	}

	if ( a_pFillData->m_uiTextureFileName != 0 && a_pFillData->m_uiFillType == XURXUIFillData::Fill_Texture )
	{
		m_wszFillTextureName = a_rResource.GetString( a_pFillData->m_uiTextureFileName );

		TString8 strName( m_wszFillTextureName );
		StripFillTextureName( strName );

		m_pFillTexture = TTextureManager::GetSingleton()->FindTexture( strName.GetString() );

		if ( m_pMaterial == TNULL )
			m_pMaterial = T2GUI::CreateMaterial( m_pFillTexture );
		else
			m_pMaterial->SetTextureResource( m_pFillTexture );
	}

	SetFillType( XURXUIFillData::Fill( a_pFillData->m_uiFillType ) );

	m_oFillTransform.SetIdentity();
	m_fFillRotation = a_pFillData->m_fRotation * ( TMath::PI / 180.0f );
	m_oFillTransform.Rotate( m_fFillRotation );

	if ( a_pFillData->m_iScale >= 0 )
	{
		const TVector2* pScale = a_rResource.GetVector( a_pFillData->m_iScale );

		T2GUITransform oScale;
		oScale.m_Pos.x        = 0.0f;
		oScale.m_Pos.y        = 0.0f;
		oScale.m_Rot[ 0 ].m_Y = 0;
		oScale.m_Rot[ 1 ].m_X = 0;
		oScale.m_Rot[ 1 ].m_Y = T2GUITransform::Float( TINT( pScale->y * 256.0f ) );
		oScale.m_Rot[ 0 ].m_X = T2GUITransform::Float( TINT( pScale->x * 256.0f ) );
		m_oFillTransform.PreMultiply( oScale );

		m_vFillScale.Set( pScale->x, pScale->y );
	}

	if ( a_pFillData->m_iTranslation >= 0 )
	{
		const TVector2* pTranslation = a_rResource.GetVector( a_pFillData->m_iTranslation );
		m_oFillTransform.m_Pos       = *pTranslation;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c8850
TBOOL TXUIFigure::CreateGradient( XURXUIGradientData* a_pGradientData, TBOOL a_bRadial )
{
	if ( a_bRadial == TTRUE )
	{
		TASSERT( a_pGradientData->m_uiRadial );
		m_uiFigureFlags |= FigureFlags_RadialGradient;
	}
	else
	{
		m_uiFigureFlags &= ~FigureFlags_RadialGradient;
	}

	m_uiNumStops          = a_pGradientData->m_uiNumStops;
	m_auiFillStopColours  = TSTATICCAST( TUINT32, TMalloc( m_uiNumStops * sizeof( TUINT32 ), TXUI::ms_pXUIMemoryBlock ) );
	m_afFillStopPositions = TSTATICCAST( TFLOAT, TMalloc( m_uiNumStops * sizeof( TFLOAT ), TXUI::ms_pXUIMemoryBlock ) );
	TUtil::MemCopy( m_auiFillStopColours, a_pGradientData->m_auiStopColours, m_uiNumStops * sizeof( TUINT32 ) );
	TUtil::MemCopy( m_afFillStopPositions, a_pGradientData->m_afStopPositions, m_uiNumStops * sizeof( TFLOAT ) );

	CreateFillGradientTexture( ( ( m_uiFigureFlags >> FigureFlags_FillTypeShift ) & 1 ) != 0 );
	return TTRUE;
}

// $deBlob: FUNCTION 006c9e50
static TUINT32 MakeARGB( TUINT32 a_uiRed, TUINT32 a_uiGreen, TUINT32 a_uiBlue, TUINT32 a_uiAlpha )
{
	return ( ( ( ( a_uiRed & 0xFF ) | ( a_uiAlpha << 8 ) ) << 8 | ( a_uiGreen & 0xFF ) ) << 8 ) | ( a_uiBlue & 0xFF );
}

// $deBlob: FUNCTION 006c9e80
static void UnpackARGB( TUINT32 a_uiColour, TUINT32& a_rRed, TUINT32& a_rGreen, TUINT32& a_rBlue, TUINT32& a_rAlpha )
{
	a_rAlpha = a_uiColour >> 24;
	a_rRed   = ( a_uiColour >> 16 ) & 0xFF;
	a_rGreen = ( a_uiColour >> 8 ) & 0xFF;
	a_rBlue  = a_uiColour & 0xFF;
}

// Interpolates the channels with 8 bits of precision
// $deBlob: FUNCTION 006c8960
TUINT32 TXUIFigure::GetGradientColour( TUINT32 a_uiColour1, TUINT32 a_uiColour2, TFLOAT a_fFrac )
{
	if ( a_uiColour1 == a_uiColour2 )
		return a_uiColour2;

	TUINT32 aChannels1[ 4 ];
	TUINT32 aChannels2[ 4 ];
	UnpackARGB( a_uiColour1, aChannels1[ 0 ], aChannels1[ 1 ], aChannels1[ 2 ], aChannels1[ 3 ] );
	UnpackARGB( a_uiColour2, aChannels2[ 0 ], aChannels2[ 1 ], aChannels2[ 2 ], aChannels2[ 3 ] );

	TUINT32 uiFrac = TUINT32( a_fFrac * 255.0f );

	for ( TINT i = 0; i < 4; i++ )
		aChannels1[ i ] = ( ( aChannels2[ i ] - aChannels1[ i ] ) * uiFrac + aChannels1[ i ] * 256 ) >> 8;

	return MakeARGB( aChannels1[ 0 ], aChannels1[ 1 ], aChannels1[ 2 ], aChannels1[ 3 ] );
}

// Fills the gradient texture: a s_uiGradientTextureWidth x 1 strip for the linear gradients, a square for the radial
// ones (the stops go from the centre to the edge)
// NOTE: like retail, an existing texture isn't updated (it's immutable), only its data
// $deBlob: FUNCTION 006c8a90
void TXUIFigure::CreateFillGradientTexture( TBOOL a_bRadial )
{
	TUINT32 uiWidth = s_uiGradientTextureWidth;
	TUINT32 uiHeight;

	if ( m_pGradientTexture == TNULL )
	{
		uiHeight = s_uiGradientTextureHeight;

		if ( a_bRadial == TFALSE )
			uiHeight = 1;
	}
	else
	{
		uiWidth  = m_pGradientTexture->GetWidth();
		uiHeight = m_pGradientTexture->GetHeight();
	}

	TFLOAT fWidth = TFLOAT( TINT( uiWidth ) );

	if ( m_pGradientTextureData == TNULL )
		m_pGradientTextureData = TSTATICCAST( TUINT32, TMalloc( uiHeight * uiWidth * 4, TXUI::ms_pXUIMemoryBlock ) );

	TUINT32* pData = m_pGradientTextureData;

	if ( a_bRadial == TTRUE )
	{
		TFLOAT fY = 0.0f;

		for ( TINT iRow = 0; iRow < TINT( uiHeight ); iRow++ )
		{
			TFLOAT fX = 0.0f;

			if ( 0 < TINT( uiWidth ) )
			{
				TFLOAT fDY = TMath::Abs( fY - TFLOAT( TINT( uiHeight ) ) * 0.5f );

				for ( TINT iCol = 0; iCol < TINT( uiWidth ); iCol++ )
				{
					TFLOAT fDX   = TMath::Abs( fX - fWidth * 0.5f );
					TFLOAT fFrac = TFLOAT( sqrt( (double)( fDX * fDX + fDY * fDY ) ) ) / ( ( fWidth - 1.0f ) * 0.5f );

					TUINT32 uiColour;

					if ( 1.0f <= fFrac || m_afFillStopPositions[ m_uiNumStops - 1 ] <= fFrac )
					{
						uiColour = m_auiFillStopColours[ m_uiNumStops - 1 ];
					}
					else if ( m_afFillStopPositions[ 0 ] < fFrac )
					{
						TINT iSecondStop = 0;

						if ( m_afFillStopPositions[ 0 ] < fFrac )
						{
							do
							{
								iSecondStop++;
							} while ( m_afFillStopPositions[ iSecondStop ] <= fFrac && fFrac != m_afFillStopPositions[ iSecondStop ] );
						}

						TINT iFirstStop = iSecondStop - 1;

						if ( iFirstStop < 0 )
							iFirstStop = 0;

						TASSERT( fFrac >= m_afFillStopPositions[ iFirstStop ] && fFrac <= m_afFillStopPositions[ iSecondStop ] );
						fFrac = ( fFrac - m_afFillStopPositions[ iFirstStop ] ) / ( m_afFillStopPositions[ iSecondStop ] - m_afFillStopPositions[ iFirstStop ] );
						TASSERT( fFrac >= 0.0f && fFrac <= 1.0f );

						uiColour = GetGradientColour( m_auiFillStopColours[ iFirstStop ], m_auiFillStopColours[ iSecondStop ], fFrac );
					}
					else
					{
						uiColour = m_auiFillStopColours[ 0 ];
					}

					fX += 1.0f;
					*pData = uiColour;
					pData++;
				}
			}

			fY += 1.0f;
		}
	}
	else
	{
		for ( TINT iCol = 0; iCol < TINT( uiWidth ); iCol++ )
		{
			TFLOAT fFrac = TFLOAT( iCol ) / fWidth;
			TASSERT( fFrac >= 0.0f && fFrac <= 1.0f );

			TUINT32 uiColour;

			if ( m_afFillStopPositions[ 0 ] < fFrac )
			{
				if ( fFrac < m_afFillStopPositions[ m_uiNumStops - 1 ] )
				{
					TINT iSecondStop = 0;
					TASSERT( iSecondStop < m_uiNumStops );

					if ( m_afFillStopPositions[ 0 ] <= fFrac && fFrac != m_afFillStopPositions[ 0 ] )
					{
						do
						{
							TASSERT( iSecondStop < m_uiNumStops );
							iSecondStop++;
						} while ( m_afFillStopPositions[ iSecondStop ] <= fFrac && fFrac != m_afFillStopPositions[ iSecondStop ] );
					}

					TINT iFirstStop = iSecondStop - 1;

					if ( iFirstStop < 0 )
						iFirstStop = 0;

					TASSERT( iFirstStop < m_uiNumStops );
					TASSERT( iSecondStop < m_uiNumStops );
					TASSERT( fFrac >= m_afFillStopPositions[ iFirstStop ] && fFrac <= m_afFillStopPositions[ iSecondStop ] );
					TASSERT( iFirstStop < m_uiNumStops );
					TASSERT( iSecondStop < m_uiNumStops );
					fFrac = ( fFrac - m_afFillStopPositions[ iFirstStop ] ) / ( m_afFillStopPositions[ iSecondStop ] - m_afFillStopPositions[ iFirstStop ] );
					TASSERT( fFrac >= 0.0f && fFrac <= 1.0f );
					TASSERT( iFirstStop < m_uiNumStops );
					TASSERT( iSecondStop < m_uiNumStops );

					uiColour = GetGradientColour( m_auiFillStopColours[ iFirstStop ], m_auiFillStopColours[ iSecondStop ], fFrac );
				}
				else
				{
					uiColour = m_auiFillStopColours[ m_uiNumStops - 1 ];
				}
			}
			else
			{
				uiColour = m_auiFillStopColours[ 0 ];
			}

			// Same colour for the whole column
			TUINT32* pPixel = pData;

			for ( TINT iRow = 0; iRow < TINT( uiHeight ); iRow++ )
			{
				*pPixel = uiColour;
				pPixel += uiWidth;
			}

			pData++;
		}
	}

	if ( m_pGradientTexture == TNULL )
	{
		m_pGradientTextureInfo              = new ( TXUI::ms_pXUIMemoryBlock ) TTexture::Info();
		m_pGradientTextureInfo->Width       = uiWidth;
		m_pGradientTextureInfo->Height      = uiHeight;
		m_pGradientTextureInfo->MipMapCount = 1;
		m_pGradientTextureInfo->Format      = DXGI_FORMAT_B8G8R8A8_UNORM;
		m_pGradientTextureInfo->SRView      = TRenderDX11::Interface()->CreateTexture( uiWidth, uiHeight, DXGI_FORMAT_B8G8R8A8_UNORM, m_pGradientTextureData, 0, D3D11_USAGE_IMMUTABLE, 0, 1 );

		m_pGradientTexture = TTextureManager::GetSingleton()->CreateTexture( m_pGradientTextureInfo );
	}

	if ( m_pMaterial == TNULL )
		m_pMaterial = T2GUI::CreateMaterial( m_pGradientTexture );
	else
		m_pMaterial->SetTextureResource( m_pGradientTexture );
}

// Computes the UVs of the shape (and of its anti-aliasing edge) from the fill transform: the gradient texture
// covers the figure, the fill texture is repeated at its size
// $deBlob: FUNCTION 006c9270
TBOOL TXUIFigure::SetupUVs()
{
	if ( m_pMaterial == TNULL )
		return TTRUE;

	if ( m_pUVs == TNULL )
		m_pUVs = new ( TXUI::ms_pXUIMemoryBlock ) TXUIFixedVector2<128>[ m_pShape->m_uiNumVerts ];

	TFLOAT fMaxU;
	TFLOAT fMaxV;

	if ( m_pGradientTexture && m_pMaterial->GetTextureResource() == m_pGradientTexture )
	{
		fMaxU = 1.0f;
		fMaxV = 1.0f;

		m_pMaterial->SetTextureAddressV( T2GUIMaterial::TEXTUREADDRESS_CLAMP );
		m_pMaterial->SetTextureAddress( T2GUIMaterial::TEXTUREADDRESS_CLAMP );
	}
	else
	{
		if ( m_pFillTexture == TNULL )
			return TTRUE;

		TTexture* pTexture = m_pMaterial->GetTextureResource();

		if ( pTexture != m_pFillTexture )
			return TTRUE;

		fMaxU = UnpackFloat( m_iWidth ) / TFLOAT( TINT( pTexture->GetWidth() ) );
		fMaxV = UnpackFloat( m_iHeight ) / TFLOAT( TINT( pTexture->GetHeight() ) );

		// U and V from m_uiFillWrapX (2: clamp), then V from m_uiFillWrapY
		T2GUIMaterial::TEXTUREADDRESS eAddress = ( m_uiFillWrapX == 2 ) ? T2GUIMaterial::TEXTUREADDRESS_CLAMP : T2GUIMaterial::TEXTUREADDRESS_WRAP;
		m_pMaterial->SetTextureAddress( eAddress );
		m_pMaterial->SetTextureAddressV( eAddress );
		m_pMaterial->SetTextureAddressV( ( m_uiFillWrapY == 2 ) ? T2GUIMaterial::TEXTUREADDRESS_CLAMP : T2GUIMaterial::TEXTUREADDRESS_WRAP );
	}

	if ( UnpackFloat( m_iWidth ) != 0.0f && UnpackFloat( m_iHeight ) != 0.0f )
	{
		TFLOAT fWidth       = UnpackFloat( m_iWidth );
		TFLOAT fHeight      = UnpackFloat( m_iHeight );
		TFLOAT fDimsScaleX  = m_vDimsScale.GetX();
		TFLOAT fDimsScaleY  = m_vDimsScale.GetY();
		TFLOAT fShapeScaleX = m_vShapeScale.GetX();
		TFLOAT fShapeScaleY = m_vShapeScale.GetY();
		TFLOAT fOffsetX     = fWidth * -0.5f;
		TFLOAT fOffsetY     = fHeight * -0.5f;

		static TVector2 s_vCentre( 0.5f, 0.5f );

		for ( TUINT32 i = 0; i < m_pShape->m_uiNumVerts; i++ )
		{
			TFLOAT fU = ( ( m_pShape->m_pVerts[ i ].GetX() * fShapeScaleX * fDimsScaleX - fOffsetX ) / fWidth ) * fMaxU;
			TFLOAT fV = ( ( m_pShape->m_pVerts[ i ].GetY() * fShapeScaleY * fDimsScaleY - fOffsetY ) / fHeight ) * fMaxV;
			TMath::Clip( fU, 0.0f, fMaxU );
			TMath::Clip( fV, 0.0f, fMaxV );

			TFLOAT fCentredU = fU - s_vCentre.x;
			TFLOAT fX        = m_oFillTransform.m_Rot[ 0 ].GetX() * fCentredU + m_oFillTransform.m_Rot[ 1 ].GetX() * ( fV - s_vCentre.y ) + m_oFillTransform.m_Pos.x + s_vCentre.x;
			TFLOAT fY        = m_oFillTransform.m_Rot[ 0 ].GetY() * fCentredU + m_oFillTransform.m_Rot[ 1 ].GetY() * ( fV - s_vCentre.y ) + m_oFillTransform.m_Pos.y + s_vCentre.y;
			m_pUVs[ i ].Set( fX, fY );
		}

		if ( m_pAaEdgeShape )
			m_pAaEdgeShape->MakeUVsAaEdge( &m_oFillTransform, TVector2( fDimsScaleX, fDimsScaleY ), TVector2( fWidth, fHeight ), fMaxU, fMaxV );

		return TTRUE;
	}

	return TFALSE;
}

TOSHI_NAMESPACE_END
