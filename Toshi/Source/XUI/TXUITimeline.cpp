#include "ToshiPCH.h"
#include "TXUITimeline.h"
#include "TXUI.h"
#include "TXUITimelineControl.h"
#include "TXUIFigure.h"
#include "TXUIShapeCache.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// Frames are 1/60th of a second
static TCONSTEXPR_VAR TFLOAT TXUITIMELINE_FRAMETIME = 1.0f / 60.0f;

// Directions of the default ease curve tangents
static TVector2 s_vEaseInDir;
static TVector2 s_vEaseOutDir;

//-----------------------------------------------------------------------------
// XURXUITimelineData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e8700
XURXUITimelineData::XURXUITimelineData()
{
	m_pOwnerData         = TNULL;
	m_uiNumTimelineProps = 0;
	m_uiNumKeyframes     = 0;
	m_uiObjectNameIndex  = 0;
	m_aTimelineValues    = TNULL;
	m_pControlledChild   = TNULL;
	m_aKeyframeData      = TNULL;
	m_aTimelineProps     = TNULL;
}

// $deBlob: FUNCTION 006e8730
// $deBlob: FUNCTION 006c23d0
XURXUITimelineData::~XURXUITimelineData()
{
	if ( m_aKeyframeData )
		delete[] m_aKeyframeData;

	operator delete[]( m_aTimelineProps );
	operator delete[]( m_aTimelineValues );
}

// Reads a timeline property id (5, 4 or 3 bytes in the file)
static void XURXUITimelineData_ReadTimelineProp( XURXUITimelineData::TimelineProp& a_rProp, TUINT8*& a_rData )
{
	if ( *a_rData == 0x83 )
	{
		a_rData += 1;
		a_rProp.m_uiProp = PARSEDWORD_BIG( a_rData );
		a_rData += 4;
		a_rProp.m_iIndex = PARSEDWORD_BIG( a_rData );
		a_rData += 4;
	}
	else if ( *a_rData == 0x02 )
	{
		a_rProp.m_uiProp = ( ( ( TUINT32( a_rData[ 1 ] ) | 0x200 ) << 8 | a_rData[ 2 ] ) << 8 ) | a_rData[ 3 ];
		a_rData += 4;
	}
	else
	{
		a_rProp.m_uiProp = 0;
		TUtil::MemCopy( &a_rProp.m_uiProp, a_rData, 3 );
		a_rProp.m_uiProp = PARSEDWORD_BIG( TREINTERPRETCAST( const TUINT8*, &a_rProp.m_uiProp ) );
		a_rData += 3;
	}
}

// $deBlob: FUNCTION 006e8760
TBOOL XURXUITimelineData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	m_uiObjectNameIndex = PARSEWORD_BIG( a_rData );
	a_rData += 2;

	m_uiNumTimelineProps = PARSEWORD_BIG( a_rData + 2 );
	a_rData += 4;

	m_aTimelineProps   = new ( TXUI::ms_pXUIMemoryBlock ) TimelineProp[ m_uiNumTimelineProps ];
	m_pControlledChild = m_pOwnerData->FindChildElementData( m_uiObjectNameIndex );

	for ( TUINT32 i = 0; i < m_uiNumTimelineProps; i++ )
	{
		TimelineProp& rProp = m_aTimelineProps[ i ];
		XURXUITimelineData_ReadTimelineProp( rProp, a_rData );

		TBOOL bValid;

		if ( rProp.IsFigureProp() )
		{
			TASSERT( 0 == TStringManager::String16Compare( a_rResource.GetString( m_pControlledChild->m_uiTypeIndex ), L"XuiFigure", -1 ) );
			bValid = TSTATICCAST( XURXUIFigureData, m_pControlledChild )->ValidateTimelineSubProp( rProp.GetPropIndex(), rProp.GetSubIndex() );
		}
		else
		{
			bValid = m_pControlledChild->ValidateTimelineProp( rProp.GetObjectIndex(), rProp.GetPropIndex() );
		}

		TASSERT( TTRUE == bValid );
	}

	m_uiNumKeyframes = PARSEWORD_BIG( a_rData + 2 );
	TASSERT( m_uiNumKeyframes < 1000 );
	a_rData += 4;

	m_aKeyframeData   = new ( TXUI::ms_pXUIMemoryBlock ) XURXUIKeyframeData[ m_uiNumKeyframes ];
	m_aTimelineValues = TSTATICCAST( TUINT32, TMalloc( m_uiNumTimelineProps * m_uiNumKeyframes * sizeof( TUINT32 ), TXUI::ms_pXUIMemoryBlock ) );

	TUINT32* pValues = m_aTimelineValues;

	for ( TUINT32 i = 0; i < m_uiNumKeyframes; i++ )
	{
		XURXUIKeyframeData& rKeyframe = m_aKeyframeData[ i ];

		rKeyframe.m_uiKeyframeTime = PARSEWORD_BIG( a_rData + 2 );
		TASSERT( m_aKeyframeData[ i ].m_uiKeyframeTime < 5000 );
		a_rData += 4;

		rKeyframe.m_eInterpolation = *a_rData;
		a_rData += 1;
		rKeyframe.m_iEaseIn = *a_rData;
		a_rData += 1;
		rKeyframe.m_iEaseOut = *a_rData;
		a_rData += 1;
		rKeyframe.m_uiEaseScale = *a_rData;
		a_rData += 1;

		rKeyframe.m_pValues = pValues;
		pValues += m_uiNumTimelineProps;

		for ( TUINT32 k = 0; k < m_uiNumTimelineProps; k++ )
		{
			TimelineProp& rProp = m_aTimelineProps[ k ];

			TUINT32 uiPropSize;
			TBOOL   bIsFloat;

			if ( rProp.IsFigureProp() )
			{
				TASSERT( 0 == TStringManager::String16Compare( a_rResource.GetString( m_pControlledChild->m_uiTypeIndex ), L"XuiFigure", -1 ) );

				XURXUIFigureData* pFigureData = TSTATICCAST( XURXUIFigureData, m_pControlledChild );
				uiPropSize                    = pFigureData->GetTimelineSubPropSize( rProp.GetPropIndex(), rProp.GetSubIndex() );
				bIsFloat                      = pFigureData->IsFloatSubPropType( rProp.GetPropIndex(), rProp.GetSubIndex() );
				pFigureData->IsColourSubPropType( rProp.GetPropIndex(), rProp.GetSubIndex() );
			}
			else
			{
				uiPropSize = m_pControlledChild->GetTimelinePropSize( rProp.GetObjectIndex(), rProp.GetPropIndex() );
				bIsFloat   = m_pControlledChild->IsFloatPropType( rProp.GetObjectIndex(), rProp.GetPropIndex() );
				m_pControlledChild->IsColourPropType( rProp.GetObjectIndex(), rProp.GetPropIndex() );
			}

			if ( uiPropSize == 1 )
			{
				rKeyframe.m_pValues[ k ] = *a_rData;
				a_rData += 1;
			}
			else if ( uiPropSize == 2 )
			{
				rKeyframe.m_pValues[ k ] = PARSEWORD_BIG( a_rData );
				a_rData += 2;
			}
			else if ( uiPropSize == 4 )
			{
				// NOTE: both branches read the raw big endian dword (floats are kept as their bits)
				if ( bIsFloat == TTRUE )
					rKeyframe.m_pValues[ k ] = PARSEDWORD_BIG( a_rData );
				else
					rKeyframe.m_pValues[ k ] = PARSEDWORD_BIG( a_rData );

				a_rData += 4;
			}
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e8d80
TUINT32 XURXUITimelineData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData, XURXUIObjectData* a_pOwnerData )
{
	TUINT16 uiObjectNameIndex = PARSEWORD_BIG( a_rData );
	a_rData += 2;

	TUINT32 uiNumTimelineProps = PARSEDWORD_BIG( a_rData );
	a_rData += 4;
	TASSERT( uiNumTimelineProps < 10000 );

	TimelineProp* pProps = new ( TXUI::ms_pXUIMemoryBlock ) TimelineProp[ uiNumTimelineProps ];

	for ( TUINT32 i = 0; i < uiNumTimelineProps; i++ )
		XURXUITimelineData_ReadTimelineProp( pProps[ i ], a_rData );

	TUINT32 uiNumKeyframes = PARSEDWORD_BIG( a_rData );
	TASSERT( uiNumKeyframes < 1000 );
	a_rData += 4;

	TINT iNumValues = ( uiNumTimelineProps + 3 ) * uiNumKeyframes;

	XURXUIElementData* pControlledChild = a_pOwnerData->FindChildElementData( uiObjectNameIndex );

	for ( ; uiNumKeyframes != 0; uiNumKeyframes-- )
	{
		TASSERT( PARSEWORD_BIG( a_rData + 2 ) < 5000 );
		a_rData += 8;

		for ( TUINT32 i = 0; i < uiNumTimelineProps; i++ )
		{
			TimelineProp& rProp = pProps[ i ];
			TUINT32       uiPropSize;

			if ( rProp.IsFigureProp() )
			{
				TASSERT( 0 == TStringManager::String16Compare( a_rResource.GetString( pControlledChild->m_uiTypeIndex ), L"XuiFigure", -1 ) );
				uiPropSize = TSTATICCAST( XURXUIFigureData, pControlledChild )->GetTimelineSubPropSize( rProp.GetPropIndex(), rProp.GetSubIndex() );
			}
			else
			{
				uiPropSize = pControlledChild->GetTimelinePropSize( rProp.GetObjectIndex(), rProp.GetPropIndex() );
			}

			TASSERT( uiPropSize == 1 || uiPropSize == 2 || uiPropSize == 4 );
			a_rData += uiPropSize;
		}
	}

	operator delete( pProps );

	return uiNumTimelineProps * sizeof( TimelineProp ) + 4 + ( iNumValues + 2 ) * 4;
}

//-----------------------------------------------------------------------------
// TXUITimeline
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e9130
TXUITimeline::TXUITimeline()
{
	m_wszName        = TNULL;
	m_pElement       = TNULL;
	m_pVisualElement = TNULL;
	m_fEndTime       = 0.0f;
	m_fTime          = -1.0f;
	m_uiNextXUIKey   = 0;
	m_pResource      = TNULL;
	m_pData          = TNULL;
	m_iEaseIn        = 0;
	m_iEaseOut       = 0;
	m_uiEaseScale    = 50;

	s_vEaseInDir.x = TVector2::VEC_POSX.x + TVector2::VEC_POSY.x;
	s_vEaseInDir.y = TVector2::VEC_POSX.y + TVector2::VEC_POSY.y;
	s_vEaseInDir.Normalize();
	s_vEaseOutDir.x = -s_vEaseInDir.x;
	s_vEaseOutDir.y = -s_vEaseInDir.y;

	m_avEasePoints[ 0 ] = TVector2::VEC_ZERO;
	m_avEasePoints[ 1 ] = s_vEaseInDir;
	m_avEasePoints[ 3 ].m_iX = 0x2000;
	m_avEasePoints[ 3 ].m_iY = 0x2000;
	m_avEasePoints[ 2 ] = s_vEaseOutDir;
}

// $deBlob: FUNCTION 006e92e0
TBOOL TXUITimeline::Create( TXUIResource& a_rResource, XURXUITimelineData* a_pData, TXUIElement* a_pParent )
{
	TASSERT( 0 < a_pData->m_uiObjectNameIndex );
	m_wszName = a_rResource.GetString( a_pData->m_uiObjectNameIndex );

	TASSERT( 0 < a_pData->m_uiObjectNameIndex );
	m_pElement = a_pParent->FindChildXUIElement( a_rResource.GetString( a_pData->m_uiObjectNameIndex ) );

	m_pData     = a_pData;
	m_pResource = &a_rResource;
	m_fEndTime  = TFLOAT( a_pData->m_aKeyframeData[ a_pData->m_uiNumKeyframes - 1 ].m_uiKeyframeTime ) * TXUITIMELINE_FRAMETIME;

	return TTRUE;
}

// $deBlob: FUNCTION 006e93b0
void TXUITimeline::Update()
{
	if ( m_pElement == TNULL && m_pVisualElement == TNULL )
		return;

	if ( m_pData->m_uiNumKeyframes == 0 )
		return;

	if ( m_fTime == -1.0f )
		return;

	TUINT32             uiNextKey  = m_uiNextXUIKey;
	XURXUIKeyframeData* pKeyframes = m_pData->m_aKeyframeData;
	XURXUIKeyframeData& rPrevKey   = pKeyframes[ uiNextKey - 1 ];
	XURXUIKeyframeData& rNextKey   = pKeyframes[ uiNextKey ];
	TUINT8              eInterp    = rPrevKey.m_eInterpolation;

	if ( eInterp == XURXUIKeyframeData::Interpolation_Ease )
	{
		TBOOL bSameEaseIn = m_iEaseIn == rPrevKey.m_iEaseIn;
		if ( !bSameEaseIn ) m_iEaseIn = rPrevKey.m_iEaseIn;

		TBOOL bSameEaseOut = m_iEaseOut == rPrevKey.m_iEaseOut;
		if ( !bSameEaseOut ) m_iEaseOut = rPrevKey.m_iEaseOut;

		if ( m_uiEaseScale != rPrevKey.m_uiEaseScale )
		{
			m_uiEaseScale = rPrevKey.m_uiEaseScale;
			UpdateEase( m_iEaseIn, m_iEaseOut, m_uiEaseScale );
		}
		else if ( !bSameEaseOut || !bSameEaseIn )
		{
			UpdateEase( m_iEaseIn, m_iEaseOut, m_uiEaseScale );
		}
	}

	TASSERT( m_uiNextXUIKey > 0 );

	TUINT32 uiPrevTime = rPrevKey.m_uiKeyframeTime;
	TUINT32 uiNextTime = rNextKey.m_uiKeyframeTime;
	TFLOAT  fPrevTime  = ( TFLOAT( uiPrevTime ) + 0.0f ) * TXUITIMELINE_FRAMETIME;

	if ( m_uiNextXUIKey != 0 && uiNextTime - uiPrevTime == 1 )
		eInterp = XURXUIKeyframeData::Interpolation_None;

	TFLOAT fT = ( m_fTime - fPrevTime ) / ( ( TFLOAT( uiNextTime ) + 0.0f ) * TXUITIMELINE_FRAMETIME - fPrevTime );
	TMath::Clip( fT, 0.0f, 1.0f );

	if ( eInterp == XURXUIKeyframeData::Interpolation_Ease )
	{
		fT = EvaluateEase( m_avEasePoints, 3, fT );
	}
	else if ( eInterp == XURXUIKeyframeData::Interpolation_None )
	{
		fT = 1.0f;

		if ( m_fTime < m_fEndTime )
			fT = 0.0f;
	}

	XURXUITimelineData* pData           = m_pData;
	TUINT16             uiNumProps      = pData->m_uiNumTimelineProps;
	XURXUIElementData*  pControlledData = pData->m_pControlledChild;

	for ( TUINT32 i = 0; i < uiNumProps; i++ )
	{
		XURXUITimelineData::TimelineProp& rProp = pData->m_aTimelineProps[ i ];

		if ( rProp.IsFigureProp() )
		{
			TXUIFigure* pFigure    = TSTATICCAST( TXUIFigure, m_pElement );
			TUINT8      uiSubIndex = rProp.GetSubIndex();
			TUINT8      uiPropIdx  = rProp.GetPropIndex();
			TINT        iIndex;

			if ( uiPropIdx == XURXUIFillData::Prop_Gradient )
			{
				iIndex = rProp.m_iIndex;
				TASSERT( iIndex != -1 );
			}
			else
			{
				iIndex = -1;
			}

			if ( TSTATICCAST( XURXUIFigureData, pControlledData )->IsFloatSubPropType( uiPropIdx, uiSubIndex ) == TTRUE )
			{
				pFigure->UpdateTimelineSubProp( m_pResource, uiPropIdx, uiSubIndex, *TREINTERPRETCAST( TFLOAT*, &rPrevKey.m_pValues[ i ] ), *TREINTERPRETCAST( TFLOAT*, &rNextKey.m_pValues[ i ] ), fT, iIndex );
			}
			else
			{
				pFigure->UpdateTimelineSubProp( m_pResource, uiPropIdx, uiSubIndex, rPrevKey.m_pValues[ i ], rNextKey.m_pValues[ i ], fT, iIndex );
			}
		}
		else
		{
			TUINT8 uiPropIdx   = rProp.GetPropIndex();
			TUINT8 uiObjectIdx = rProp.GetObjectIndex();

			if ( pControlledData->IsFloatPropType( uiObjectIdx, uiPropIdx ) == TTRUE )
			{
				TFLOAT fValue1 = *TREINTERPRETCAST( TFLOAT*, &rPrevKey.m_pValues[ i ] );
				TFLOAT fValue2 = *TREINTERPRETCAST( TFLOAT*, &rNextKey.m_pValues[ i ] );

				if ( m_pVisualElement == TNULL )
					m_pElement->UpdateTimelineProp( m_pResource, uiObjectIdx, uiPropIdx, fValue1, fValue2, fT );
				else
					m_pVisualElement->UpdateTimelineProp( m_pResource, uiObjectIdx + 1, uiPropIdx, fValue1, fValue2, fT );
			}
			else
			{
				TUINT32 uiValue1 = rPrevKey.m_pValues[ i ];
				TUINT32 uiValue2 = rNextKey.m_pValues[ i ];

				if ( m_pVisualElement == TNULL )
					m_pElement->UpdateTimelineProp( m_pResource, uiObjectIdx, uiPropIdx, uiValue1, uiValue2, fT );
				else
					m_pVisualElement->UpdateTimelineProp( m_pResource, uiObjectIdx + 1, uiPropIdx, uiValue1, uiValue2, fT );
			}
		}
	}
}

// $deBlob: FUNCTION 006e9790
void TXUITimeline::Reset()
{
	m_uiNextXUIKey = 0;

	for ( TINT i = 0; i < m_pData->m_uiNumTimelineProps; i++ )
	{
		XURXUITimelineData::TimelineProp& rProp = m_pData->m_aTimelineProps[ i ];

		if ( rProp.IsFigureProp() )
		{
			TXUIFigure*       pFigure     = TSTATICCAST( TXUIFigure, m_pElement );
			XURXUIFigureData* pFigureData = TSTATICCAST( XURXUIFigureData, m_pData->m_pControlledChild );
			TUINT8            uiSubIndex  = rProp.GetSubIndex();
			TUINT8            uiPropIdx   = rProp.GetPropIndex();
			TINT              iIndex;

			if ( uiPropIdx == XURXUIFillData::Prop_Gradient )
			{
				iIndex = rProp.m_iIndex;
				TASSERT( iIndex != -1 );
			}
			else
			{
				iIndex = -1;
			}

			TUINT32* pValues = m_pData->m_aKeyframeData[ 0 ].m_pValues;

			if ( pFigureData->IsFloatSubPropType( uiPropIdx, uiSubIndex ) == TTRUE )
				pFigure->UpdateTimelineSubProp( m_pResource, uiPropIdx, uiSubIndex, *TREINTERPRETCAST( TFLOAT*, &pValues[ i ] ), 0.0f, 0.0f, iIndex );
			else
				pFigure->UpdateTimelineSubProp( m_pResource, uiPropIdx, uiSubIndex, pValues[ i ], 0, 0.0f, iIndex );
		}
		else
		{
			TUINT8 uiObjectIdx = rProp.GetObjectIndex();
			TUINT8 uiPropIdx   = rProp.GetPropIndex();

			TBOOL    bIsFloat = m_pData->m_pControlledChild->IsFloatPropType( uiObjectIdx, uiPropIdx );
			TUINT32* pValues  = m_pData->m_aKeyframeData[ 0 ].m_pValues;

			if ( bIsFloat == TTRUE )
			{
				if ( m_pVisualElement == TNULL )
					m_pElement->UpdateTimelineProp( m_pResource, uiObjectIdx, uiPropIdx, *TREINTERPRETCAST( TFLOAT*, &pValues[ i ] ), 0.0f, 0.0f );
				else
					m_pVisualElement->UpdateTimelineProp( m_pResource, uiObjectIdx + 1, uiPropIdx, *TREINTERPRETCAST( TFLOAT*, &pValues[ i ] ), 0.0f, 0.0f );
			}
			else
			{
				if ( m_pVisualElement == TNULL )
					m_pElement->UpdateTimelineProp( m_pResource, uiObjectIdx, uiPropIdx, pValues[ i ], 0, 0.0f );
				else
					m_pVisualElement->UpdateTimelineProp( m_pResource, uiObjectIdx + 1, uiPropIdx, pValues[ i ], 0, 0.0f );
			}
		}
	}

	m_fTime = -1.0f;
}

// $deBlob: FUNCTION 006e9990
void TXUITimeline::SetTime( TFLOAT a_fTime, TBOOL a_bLoop )
{
	TFLOAT              fFrameTime = TXUITimelineControl::sm_fFrameTime;
	XURXUIKeyframeData* pKeyframes = m_pData->m_aKeyframeData;
	TUINT32             uiLastKey  = m_pData->m_uiNumKeyframes - 1;
	TFLOAT              fLastTime  = TFLOAT( pKeyframes[ uiLastKey ].m_uiKeyframeTime ) * fFrameTime;

	if ( a_fTime <= fLastTime )
		m_fTime = a_fTime;
	else
		m_fTime = ( a_bLoop == TTRUE ) ? fmodf( a_fTime, fLastTime ) : fLastTime;

	TASSERT( m_uiNextXUIKey <= uiLastKey );

	TUINT32 uiKey = 0;

	if ( m_uiNextXUIKey > 1 && m_fTime >= TFLOAT( pKeyframes[ m_uiNextXUIKey - 1 ].m_uiKeyframeTime ) * fFrameTime )
		uiKey = m_uiNextXUIKey - 1;

	uiKey += 1;

	while ( m_fTime >= TFLOAT( pKeyframes[ uiKey ].m_uiKeyframeTime ) * fFrameTime && uiKey < uiLastKey )
		uiKey++;

	m_uiNextXUIKey = uiKey;
	TASSERT( m_uiNextXUIKey > 0 && m_uiNextXUIKey <= uiLastKey );
}

// $deBlob: FUNCTION 006e9af0
void TXUITimeline::UpdateEase( TINT8 a_iEaseIn, TINT8 a_iEaseOut, TUINT8 a_uiEaseScale )
{
	TVector2 vEaseOut = s_vEaseOutDir;
	TVector2 vEaseIn  = s_vEaseInDir;
	TFLOAT   fScale   = TFLOAT( a_uiEaseScale ) / 100.0f;

	static TFLOAT s_fMaxEaseAngle = TMath::PI * 0.25f;

	vEaseIn.Rotate( s_fMaxEaseAngle * ( TFLOAT( a_iEaseIn ) / 100.0f ) );
	vEaseIn.Normalize();
	vEaseIn.x *= fScale;
	vEaseIn.y *= fScale;

	vEaseOut.Rotate( ( TFLOAT( a_iEaseOut ) / 100.0f ) * s_fMaxEaseAngle );
	vEaseOut.x += 1.0f;
	vEaseOut.y += 1.0f;
	vEaseOut.Normalize();

	m_avEasePoints[ 1 ].m_iX = TINT16( TINT( vEaseIn.x * 8192.0f ) );
	m_avEasePoints[ 1 ].m_iY = TINT16( TINT( vEaseIn.y * 8192.0f ) );
	m_avEasePoints[ 2 ].m_iX = TINT16( TINT( vEaseOut.x * fScale * 8192.0f ) );
	m_avEasePoints[ 2 ].m_iY = TINT16( TINT( vEaseOut.y * fScale * 8192.0f ) );
}

// $deBlob: FUNCTION 006e9c70
TFLOAT TXUITimeline::GetEaseScalar( TFLOAT a_fT )
{
	return EvaluateEase( m_avEasePoints, 3, a_fT );
}

// $deBlob: FUNCTION 006e9c90
TFLOAT TXUITimeline::EvaluateEase( const TXUIFixedVector2<8192>* a_pPoints, TINT a_iN, TFLOAT a_fT )
{
	TASSERT( a_iN == 3 );

	// NOTE: only the Y of the control points is used, the X stay 0
	static TVector2 s_avPoints[ 4 ] = { TVector2::VEC_ZERO, TVector2::VEC_ZERO, TVector2::VEC_ZERO, TVector2::VEC_ZERO };

	s_avPoints[ 0 ].y = TFLOAT( a_pPoints[ 0 ].m_iY ) * ( 1.0f / 8192.0f );
	s_avPoints[ 1 ].y = TFLOAT( a_pPoints[ 1 ].m_iY ) * ( 1.0f / 8192.0f );
	s_avPoints[ 2 ].y = TFLOAT( a_pPoints[ 2 ].m_iY ) * ( 1.0f / 8192.0f );
	s_avPoints[ 3 ].y = TFLOAT( a_pPoints[ 3 ].m_iY ) * ( 1.0f / 8192.0f );

	return Bezier( s_avPoints, a_iN, a_fT ).y;
}

TOSHI_NAMESPACE_END
