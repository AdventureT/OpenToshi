#pragma once
#include "TXUIElement.h"
#include "Toshi/T2DList.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// One keyframe of a XUR timeline (0xC bytes): the values of every animated property and the interpolation to the
// next keyframe.
//-----------------------------------------------------------------------------
class XURXUIKeyframeData
{
public:
	enum Interpolation : TUINT8
	{
		Interpolation_Linear,
		Interpolation_None,
		Interpolation_Ease,
	};

public:
	// $deBlob: FUNCTION 006e86d0
	XURXUIKeyframeData()
	{
		m_pValues         = TNULL;
		m_uiKeyframeTime  = 0;
		m_eInterpolation  = 0;
		m_iEaseIn         = 0;
		m_iEaseOut        = 0;
		m_uiEaseScale     = 50;
	}

	// NOTE: non-trivial in retail (the arrays have a count cookie)
	~XURXUIKeyframeData() {}

public:
	TUINT32* m_pValues;        // 0x0 m_uiNumTimelineProps values (in XURXUITimelineData::m_aTimelineValues)
	TUINT16  m_uiKeyframeTime; // 0x4 in frames (60 per second)
	TUINT8   m_eInterpolation; // 0x6
	TINT8    m_iEaseIn;        // 0x7
	TINT8    m_iEaseOut;       // 0x8
	TUINT8   m_uiEaseScale;    // 0x9
};

//-----------------------------------------------------------------------------
// A XUR timeline (0x1C bytes): the animated properties of one child of the owner and its keyframes.
//-----------------------------------------------------------------------------
class XURXUITimelineData
{
public:
	// An animated property (8 bytes). m_uiProp is read big endian: type << 24 | object index << 16 | property index << 8
	// | sub index; figure stroke/fill properties are type 2 or 0x10302/0x10303.
	struct TimelineProp
	{
		TimelineProp()
		{
			m_uiProp = 0;
			m_iIndex = -1;
		}

		TUINT8 GetSubIndex() const { return TUINT8( m_uiProp ); }
		TUINT8 GetPropIndex() const { return TUINT8( m_uiProp >> 8 ); }
		TUINT8 GetObjectIndex() const { return TUINT8( m_uiProp >> 16 ); }
		TUINT8 GetType() const { return TUINT8( m_uiProp >> 24 ); }

		// Stroke/fill properties of a XuiFigure (see XURXUIFigureData)
		TBOOL IsFigureProp() const { return GetType() == 2 || m_uiProp == 0x10302 || m_uiProp == 0x10303; }

		TUINT32 m_uiProp; // 0x0
		TINT    m_iIndex; // 0x4 gradient stop index of figure properties
	};

public:
	XURXUITimelineData();
	~XURXUITimelineData();

	TBOOL Load( TXUIResource& a_rResource, TUINT8*& a_rData );

	static TUINT32 CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData, XURXUIObjectData* a_pOwnerData );

public:
	XURXUIObjectData*   m_pOwnerData;          // 0x00
	XURXUIElementData*  m_pControlledChild;    // 0x04
	XURXUIKeyframeData* m_aKeyframeData;       // 0x08
	TimelineProp*       m_aTimelineProps;      // 0x0C
	TUINT16             m_uiNumTimelineProps;  // 0x10
	TUINT16             m_uiNumKeyframes;      // 0x12
	TUINT16             m_uiObjectNameIndex;   // 0x14 string index of the controlled child's Id
	TUINT32*            m_aTimelineValues;     // 0x18 m_uiNumKeyframes * m_uiNumTimelineProps values
};

//-----------------------------------------------------------------------------
// A running timeline (0x3C bytes), owned by a TXUITimelineControl
//-----------------------------------------------------------------------------
class TXUITimeline : public T2DList<TXUITimeline>::Node
{
public:
	TXUITimeline();

	TBOOL Create( TXUIResource& a_rResource, XURXUITimelineData* a_pData, TXUIElement* a_pParent );
	void  Update();
	void  Reset();
	void  SetTime( TFLOAT a_fTime, TBOOL a_bLoop );
	void  UpdateEase( TINT8 a_iEaseIn, TINT8 a_iEaseOut, TUINT8 a_uiEaseScale );
	TFLOAT GetEaseScalar( TFLOAT a_fT );

	static TFLOAT EvaluateEase( const TXUIFixedVector2<8192>* a_pPoints, TINT a_iN, TFLOAT a_fT );

	// $deBlob: FUNCTION 006c5370
	TFLOAT              GetEndTime() const { return m_fEndTime; }
	TFLOAT              GetTime() const { return m_fTime; }
	XURXUITimelineData* GetData() const { return m_pData; }
	// $deBlob: FUNCTION 006c5360
	const TWCHAR* GetName() const { return m_wszName; }

public:
	const TWCHAR*           m_wszName;          // 0x08 Id of the animated element
	TXUIElement*            m_pElement;         // 0x0C
	TXUIElement*            m_pVisualElement;   // 0x10 set when the timeline animates a control through its visual (object index + 1)
	TFLOAT                  m_fEndTime;         // 0x14
	TFLOAT                  m_fTime;            // 0x18 -1 when stopped
	TUINT32                 m_uiNextXUIKey;     // 0x1C
	TXUIResource*           m_pResource;        // 0x20
	XURXUITimelineData*     m_pData;            // 0x24
	TXUIFixedVector2<8192>  m_avEasePoints[ 4 ]; // 0x28 ease curve control points
	TINT8                   m_iEaseIn;          // 0x38
	TINT8                   m_iEaseOut;         // 0x39
	TUINT8                  m_uiEaseScale;      // 0x3A
};

TOSHI_NAMESPACE_END
