#pragma once
#include "TXUIElement.h"
#include "Render/TTexture.h"

TOSHI_NAMESPACE_START

class TXUIShape;
class T2GUIMaterial;

//-----------------------------------------------------------------------------
// A point of a XuiFigure's path (0xE): the point and its two Bezier control points, in 1/16 pixels
//-----------------------------------------------------------------------------
struct XUIFigurePoint
{
	// $deBlob: FUNCTION 006c9780
	XUIFigurePoint()
	{
		m_vPoint = TXUIFixedVector2<16>();
		m_vCtrl1 = TXUIFixedVector2<16>();
		m_vCtrl2 = TXUIFixedVector2<16>();
		m_bCurve = TFALSE;
	}

	TBOOL operator==( const XUIFigurePoint& a_rOther ) const;

	// $deBlob: FUNCTION 006dcf10
	TBOOL operator!=( const XUIFigurePoint& a_rOther ) const { return !( *this == a_rOther ); }

	// $deBlob: FUNCTION 006c9ab0
	void GetPoint( TVector2& a_rOutVec ) const { m_vPoint.Get( a_rOutVec ); }

	// $deBlob: FUNCTION 006c9980
	void GetCtrl1( TVector2& a_rOutVec ) const { m_vCtrl1.Get( a_rOutVec ); }

	// $deBlob: FUNCTION 006c9a30
	void GetCtrl2( TVector2& a_rOutVec ) const { m_vCtrl2.Get( a_rOutVec ); }

	// $deBlob: FUNCTION 006c9d50
	void SetPoint( TFLOAT a_fX, TFLOAT a_fY ) { m_vPoint.Set( a_fX, a_fY ); }

	// $deBlob: FUNCTION 006c9cb0
	void SetCtrl1( TFLOAT a_fX, TFLOAT a_fY ) { m_vCtrl1.Set( a_fX, a_fY ); }

	// $deBlob: FUNCTION 006c9d10
	void SetCtrl2( TFLOAT a_fX, TFLOAT a_fY ) { m_vCtrl2.Set( a_fX, a_fY ); }

	// $deBlob: FUNCTION 006c9c10
	TBOOL IsCurve() const { return m_bCurve; }

	// $deBlob: FUNCTION 006c9d80
	void SetCurve( TBOOL a_bCurve ) { m_bCurve = a_bCurve; }

	TXUIFixedVector2<16> m_vPoint; // 0x0
	TXUIFixedVector2<16> m_vCtrl1; // 0x4
	TXUIFixedVector2<16> m_vCtrl2; // 0x8
	TBOOL                m_bCurve; // 0xC set when a control point isn't on the point (more than 1 pixel away)
};

//-----------------------------------------------------------------------------
// XUR data of a figure's stroke (0x24, not an element: a sub object of XURXUIFigureData)
//-----------------------------------------------------------------------------
class XURXUIStrokeData : public XURXUIObjectData
{
public:
	enum Prop
	{
		Prop_StrokeWidth,
		Prop_StrokeColor,
		Prop_NUMOF,
	};

public:
	XURXUIStrokeData();

	// $deBlob: FUNCTION 006c5420
	// $deBlob: FUNCTION 006c9950
	~XURXUIStrokeData() = default;

	virtual TBOOL   Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                         // +0x04
	virtual TUINT32 CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                 // +0x08
	virtual TBOOL   TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10

	// NOTE: the sub objects add one-index variants of the timeline property queries
	virtual TBOOL   ValidateTimelineProp( TUINT32 a_uiPropIndex ); // +0x20
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiPropIndex );  // +0x24
	virtual TBOOL   IsFloatPropType( TUINT32 a_uiPropIndex );      // +0x28
	virtual TBOOL   IsColourPropType( TUINT32 a_uiPropIndex );     // +0x2C

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

public:
	TFLOAT  m_fStrokeWidth;   // 0x1C
	TUINT32 m_uiStrokeColour; // 0x20
};

//-----------------------------------------------------------------------------
// XUR data of a fill's gradient (0x28)
//-----------------------------------------------------------------------------
class XURXUIGradientData : public XURXUIObjectData
{
public:
	// NOTE: TranslateTimelineProp maps "Fill.Gradient.StopPos" to 2 and "Fill.Gradient.StopColor" to 3, but Load,
	// IsColourPropType, IsFloatPropType and TXUIFigure use 2 for the colours and 3 for the positions
	enum Prop
	{
		Prop_Radial,
		Prop_NumStops,
		Prop_StopColours,
		Prop_StopPositions,
		Prop_NUMOF,
	};

public:
	XURXUIGradientData();
	~XURXUIGradientData();

	virtual TBOOL   Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                         // +0x04
	virtual TUINT32 CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                 // +0x08
	virtual TBOOL   TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10

	virtual TBOOL   ValidateTimelineProp( TUINT32 a_uiPropIndex ); // +0x20
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiPropIndex );  // +0x24
	virtual TBOOL   IsFloatPropType( TUINT32 a_uiPropIndex );      // +0x28
	virtual TBOOL   IsColourPropType( TUINT32 a_uiPropIndex );     // +0x2C

public:
	TUINT8   m_uiRadial;         // 0x1C
	TUINT16  m_uiNumStops;       // 0x1E
	TUINT32* m_auiStopColours;   // 0x20
	TFLOAT*  m_afStopPositions;  // 0x24
};

//-----------------------------------------------------------------------------
// XUR data of a figure's fill (0x58)
//-----------------------------------------------------------------------------
class XURXUIFillData : public XURXUIObjectData
{
public:
	enum Prop
	{
		Prop_FillType,
		Prop_FillColor,
		Prop_TextureFileName,
		Prop_Gradient,
		Prop_Translation,
		Prop_Scale,
		Prop_Rotation,
		Prop_WrapX,
		Prop_WrapY,
		Prop_BrushFlags,
		Prop_Unknown10,
		Prop_NUMOF,
	};

	// The fill types (TXUIFigure keeps it in bits 9-11 of its flags)
	enum Fill
	{
		Fill_None,
		Fill_Solid,
		Fill_LinearGradient,
		Fill_RadialGradient,
		Fill_Texture,
		Fill_Mask = 7,
	};

public:
	XURXUIFillData();

	// $deBlob: FUNCTION 006c5990
	// $deBlob: FUNCTION 006c98c0
	~XURXUIFillData() = default;

	virtual TBOOL   Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                         // +0x04
	virtual TUINT32 CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                 // +0x08
	virtual TBOOL   TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10

	virtual TBOOL   ValidateTimelineProp( TUINT32 a_uiPropIndex ); // +0x20
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiPropIndex );  // +0x24
	virtual TBOOL   IsFloatPropType( TUINT32 a_uiPropIndex );      // +0x28
	virtual TBOOL   IsColourPropType( TUINT32 a_uiPropIndex );     // +0x2C

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

public:
	XURXUIGradientData m_oGradient;          // 0x1C
	TUINT32            m_uiFillColour;       // 0x44
	TUINT16            m_uiTextureFileName;  // 0x48 string index
	TUINT16            m_uiFillType;         // 0x4A see Fill
	TINT16             m_iTranslation;       // 0x4C index in the vector table, -1 if not set
	TINT16             m_iScale;             // 0x4E index in the vector table, -1 if not set
	TFLOAT             m_fRotation;          // 0x50 degrees
	TUINT16            m_uiWrapX;            // 0x54
	TUINT16            m_uiWrapY;            // 0x56
};

//-----------------------------------------------------------------------------
// XUR data of a XuiFigure (0xB8)
//-----------------------------------------------------------------------------
class XURXUIFigureData : public XURXUIElementData
{
public:
	enum Prop
	{
		Prop_Stroke,
		Prop_Fill,
		Prop_Closed,
		Prop_Points,
		Prop_NUMOF,
	};

	// The sub objects of the timeline sub properties (the first index of the *SubProp functions)
	enum SubObject
	{
		SubObject_Stroke   = 0,
		SubObject_Fill     = 1,
		SubObject_Gradient = 3,
	};

public:
	XURXUIFigureData();
	~XURXUIFigureData();

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                         // +0x04
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                 // +0x08
	virtual TBOOL               ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                      // +0x0C
	virtual TBOOL               TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10
	virtual TUINT32             GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                       // +0x14
	virtual TBOOL               IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                           // +0x18
	virtual TBOOL               IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                          // +0x1C
	virtual const TXUITypeInfo* GetTypeInfo() const override;                                                                         // +0x20

	// Stroke/fill sub properties animated by the timelines (see XURXUITimelineData::TimelineProp::IsFigureProp)
	virtual TBOOL   ValidateTimelineSubProp( TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex ); // +0x24
	virtual TUINT32 GetTimelineSubPropSize( TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex );  // +0x28
	virtual TBOOL   IsFloatSubPropType( TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex );      // +0x2C
	virtual TBOOL   IsColourSubPropType( TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex );     // +0x30

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

public:
	static const TXUITypeInfo sm_sTypeInfo;

public:
	TXUIFixedVector2<6> m_vDims;        // 0x30 the figure's size when its points were made
	TUINT16             m_uiNumPoints;  // 0x34
	TUINT8              m_uiClosed;     // 0x36
	XURXUIStrokeData    m_oStroke;      // 0x38
	XURXUIFillData      m_oFill;        // 0x5C
	XUIFigurePoint*     m_pPoints;      // 0xB4
};

//-----------------------------------------------------------------------------
// TXUIFigure (0xE0): a filled and/or stroked path (shape tesselated and cached by TXUIShapeCache)
//-----------------------------------------------------------------------------
class TXUIFigure : public TXUIElement
{
public:
	TDECLARE_CLASS( TXUIFigure, TXUIElement );

public:
	// m_uiFigureFlags
	enum FigureFlags : TUINT32
	{
		FigureFlags_Unknown0           = BITFLAG( 0 ),
		FigureFlags_GradientDirty      = BITFLAG( 1 ), // recreate the gradient texture
		FigureFlags_StrokeDirty        = BITFLAG( 2 ), // tesselate the stroke again
		FigureFlags_UVsDirty           = BITFLAG( 3 ), // SetupUVs
		FigureFlags_FillTransformDirty = BITFLAG( 4 ), // rebuild m_oFillTransform from the rotation and scale
		FigureFlags_AaEdgeDirty        = BITFLAG( 5 ), // remake the anti-aliasing edge vertices
		FigureFlags_ColourDirty        = BITFLAG( 6 ), // recolour the anti-aliasing edge
		FigureFlags_Closed             = BITFLAG( 7 ),
		FigureFlags_Curved             = BITFLAG( 8 ), // the first or last point is a curve
		FigureFlags_RadialGradient     = BITFLAG( 9 ), // NOTE: shares the low bit of the fill type field, which SetFillType overwrites
		FigureFlags_FillTypeShift      = 9,            // 9-11, XURXUIFillData::Fill
		FigureFlags_FillTypeMask       = 0x00000E00,
	};

public:
	TXUIFigure();
	~TXUIFigure();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void PreRender() override;                                        // +0x10
	virtual void Render() override;                                           // +0x14
	virtual void SetWidth( TFLOAT a_fWidth ) override;                        // +0x28
	virtual void SetHeight( TFLOAT a_fHeight ) override;                      // +0x2C
	virtual void SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) override; // +0x30
	virtual void SetAlpha( TFLOAT a_fAlpha ) override;                        // +0x40

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;                                                     // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	//-----------------------------------------------------------------------------
	// TXUIFigure
	// NOTE: MSVC puts overloaded virtuals into the vtable in reverse declaration order: retail has the float one at
	// +0xC0 and the TUINT32 one at +0xC4
	//-----------------------------------------------------------------------------
	virtual void UpdateTimelineSubProp( TXUIResource* a_pResource, TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar, TINT a_iIndex ); // +0xC4
	// NOTE: retail takes a_iIndex too (the gradient stop of SubObject_Gradient), the default keeps the callers that don't pass it compiling
	virtual void UpdateTimelineSubProp( TXUIResource* a_pResource, TUINT32 a_uiPropIndex, TUINT32 a_uiSubIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar, TINT a_iIndex = 0 ); // +0xC0

	TBOOL CreateFill( TXUIResource& a_rResource, XURXUIFillData* a_pFillData );
	TBOOL CreateGradient( XURXUIGradientData* a_pGradientData, TBOOL a_bRadial );
	void  CreateFillGradientTexture( TBOOL a_bRadial );
	TBOOL SetupUVs();

	void SetFillStopPos( TUINT32 a_uiIndex, TFLOAT a_fPosition );
	void SetFillStopColour( TUINT32 a_uiIndex, TUINT32 a_uiColour );
	void SetFillRotation( TFLOAT a_fDegrees );
	void SetFillType( XURXUIFillData::Fill a_eType );
	void SetFillTexture( const TWCHAR* a_wszTextureName );

	// NOTE: inlined by retail (ALandmarkInstance)
	void SetFillColour( TUINT32 a_uiColour )
	{
		m_uiFigureFlags |= FigureFlags_ColourDirty;
		m_uiFillColour = a_uiColour;
	}

	static TUINT32 GetGradientColour( TUINT32 a_uiColour1, TUINT32 a_uiColour2, TFLOAT a_fFrac );

	// $deBlob: FUNCTION 006c9a20
	XURXUIFillData::Fill GetFillType() const { return XURXUIFillData::Fill( ( m_uiFigureFlags >> FigureFlags_FillTypeShift ) & 7 ); }

	// $deBlob: FUNCTION 006c9bf0
	TBOOL IsFigureFlagSet( TUINT32 a_uiFlag ) const { return ( m_uiFigureFlags & a_uiFlag ) != 0; }

	// $deBlob: FUNCTION 006c9cf0
	void SetFigureFlag( TUINT32 a_uiFlag, TBOOL a_bSet )
	{
		if ( a_bSet )
			m_uiFigureFlags |= a_uiFlag;
		else
			m_uiFigureFlags &= ~a_uiFlag;
	}

	// $deBlob: FUNCTION 006c9d90
	void SetStrokeColour( TUINT32 a_uiColour ) { m_uiStrokeColour = a_uiColour; }

	// $deBlob: FUNCTION 006c9da0
	void SetStrokeWidth( TFLOAT a_fWidth )
	{
		m_uiFigureFlags |= FigureFlags_StrokeDirty;
		m_fStrokeWidth = a_fWidth;
	}

	TXUIShape* GetShape() const { return m_pShape; }

public:
	static TUINT32 s_uiGradientTextureWidth;  // 32
	static TUINT32 s_uiGradientTextureHeight; // 32, 1 for the linear gradients
	static TBOOL   s_bRenderAaEdge;
	static TBOOL   s_bDebugRender; // renders the outline, the triangles and the stroke as lines

private:
	TUINT32                 m_uiFigureFlags;          // 0x64 see FigureFlags
	TXUIFixedVector2<128>*  m_pUVs;                   // 0x68 one per shape vertex
	T2GUIMaterial*          m_pMaterial;              // 0x6C
	TTexture*               m_pGradientTexture;       // 0x70
	TTexture::Info*         m_pGradientTextureInfo;   // 0x74
	TFLOAT                  m_fStrokeWidth;           // 0x78
	TUINT32                 m_uiStrokeColour;         // 0x7C
	T2GUITransform          m_oFillTransform;         // 0x80 the UV transform (fill rotation, scale and translation)
	TVector2*               m_pStrokeVerts;           // 0x90 4 per shape vertex + 2 (tri strip)
	TUINT16                 m_uiNumStrokeVerts;       // 0x94
	TUINT16                 m_uiNumStops;             // 0x96
	TUINT32*                m_auiFillStopColours;     // 0x98
	TFLOAT*                 m_afFillStopPositions;    // 0x9C
	TXUIFixedVector2<6>     m_vFigureDims;            // 0xA0 the size of the figure when it was created
	TXUIFixedVector2<128>   m_vDimsScale;             // 0xA4 current size / m_vFigureDims
	TXUIFixedVector2<128>   m_vShapeScale;            // 0xA8 m_vFigureDims / the shape's size
	T2GUITransform          m_oUnknownTransform;      // 0xAC unknown, only set to the identity by the constructor
	TXUIShape*              m_pShape;                 // 0xBC from TXUIShapeCache
	TXUIShape*              m_pAaEdgeShape;           // 0xC0 the anti-aliasing edge of the solid fills (owned)
	TUINT32*                m_pGradientTextureData;   // 0xC4
	TFLOAT                  m_fFillRotation;          // 0xC8 radians
	TXUIFixedVector2<128>   m_vFillScale;             // 0xCC
	TTexture*               m_pFillTexture;           // 0xD0
	const TWCHAR*           m_wszFillTextureName;     // 0xD4
	TUINT32                 m_uiFillColour;           // 0xD8
	TUINT16                 m_uiFillWrapX;            // 0xDC
	TUINT16                 m_uiFillWrapY;            // 0xDE
};

TOSHI_NAMESPACE_END
