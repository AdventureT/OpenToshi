#pragma once
#include "TXUIResource.h"
#include "GUI/T2GUIElement.h"
#include "Toshi/TObject.h"

#ifdef SendMessage
#  undef SendMessage
#endif

TOSHI_NAMESPACE_START

class TXUIElement;
class TXUIControl;
class XURXUITimelineData;

//-----------------------------------------------------------------------------
// Type info of the XUR data classes (static, returned by XURXUIElementData::GetTypeInfo): the XUI class name of
// the data ("XuiElement", "XuiControl", ...) and the parent's type info. TXUIControl::CreateVisual walks it to find
// a default visual.
//-----------------------------------------------------------------------------
struct TXUITypeInfo
{
	const TWCHAR*       m_wszXuiType; // 0x0
	const TXUITypeInfo* m_pParent;    // 0x4

	const TWCHAR*       GetXuiType() const { return m_wszXuiType; }
	const TXUITypeInfo* GetParent() const { return m_pParent; }
};

//-----------------------------------------------------------------------------
// de Blob packs the element vectors into shorts: TXUIFixedVector2<6> (positions, 1/6 pixel), <256> (scales),
// <32> (pivots), <8192> (TXUITimeline ease curve points).
//-----------------------------------------------------------------------------
template <TINT SCALE>
class TXUIFixedVector2
{
public:
	static const TFLOAT FLOAT_QUALITY;
	static const TFLOAT FLOAT_SCALE;

public:
	// $deBlob: FUNCTION 006c9720
	// $deBlob: FUNCTION 006c9770
	TCONSTEXPR TXUIFixedVector2()
	    : m_iX( 0 ), m_iY( 0 ) {}

	// $deBlob: FUNCTION 006c21b0
	// $deBlob: FUNCTION 006c21f0
	// $deBlob: FUNCTION 006c9730
	TXUIFixedVector2( TFLOAT a_fX, TFLOAT a_fY )
	{
		Set( a_fX, a_fY );
	}

	// $deBlob: FUNCTION 006c2170
	// $deBlob: FUNCTION 006c2230
	// $deBlob: FUNCTION 006c9790
	// $deBlob: FUNCTION 006e9e10
	TXUIFixedVector2( const TVector2& a_rVec )
	{
		Set( a_rVec.x, a_rVec.y );
	}

	// $deBlob: FUNCTION 006c2280
	TXUIFixedVector2( const TXUIFixedVector2& a_rOther )
	{
		m_iX = a_rOther.m_iX;
		m_iY = a_rOther.m_iY;
	}

	// $deBlob: FUNCTION 006c22a0
	// $deBlob: FUNCTION 006c22e0
	// $deBlob: FUNCTION 006c2320
	TXUIFixedVector2& operator=( const TVector2& a_rVec )
	{
		Set( a_rVec.x, a_rVec.y );
		return *this;
	}

	TXUIFixedVector2& operator=( const TXUIFixedVector2& a_rOther )
	{
		m_iX = a_rOther.m_iX;
		m_iY = a_rOther.m_iY;
		return *this;
	}

	// $deBlob: FUNCTION 006c9c50
	// $deBlob: FUNCTION 00550ea0
	// $deBlob: FUNCTION 006c9c80
	// $deBlob: FUNCTION 006e9ec0
	void Set( TFLOAT a_fX, TFLOAT a_fY )
	{
		m_iX = TINT16( TINT( a_fX * FLOAT_QUALITY ) );
		m_iY = TINT16( TINT( a_fY * FLOAT_QUALITY ) );
	}

	// $deBlob: FUNCTION 006c9dd0
	// $deBlob: FUNCTION 006c9df0
	// $deBlob: FUNCTION 006e9ef0
	void SetX( TFLOAT a_fX ) { m_iX = TINT16( TINT( a_fX * FLOAT_QUALITY ) ); }

	// $deBlob: FUNCTION 006c9e10
	// $deBlob: FUNCTION 006c9e30
	// $deBlob: FUNCTION 006e9f10
	void SetY( TFLOAT a_fY ) { m_iY = TINT16( TINT( a_fY * FLOAT_QUALITY ) ); }

	// $deBlob: FUNCTION 006c9b30
	// $deBlob: FUNCTION 005b1ba0
	// $deBlob: FUNCTION 00625f50
	// $deBlob: FUNCTION 006c9b60
	TFLOAT GetX() const { return TFLOAT( m_iX ) * FLOAT_SCALE; }

	// $deBlob: FUNCTION 006c9b90
	// $deBlob: FUNCTION 005b1bd0
	// $deBlob: FUNCTION 00625fa0
	// $deBlob: FUNCTION 006c9bc0
	// $deBlob: FUNCTION 006e9e90
	TFLOAT GetY() const { return TFLOAT( m_iY ) * FLOAT_SCALE; }

	// $deBlob: FUNCTION 006dce80
	TBOOL operator==( const TXUIFixedVector2& a_rOther ) const { return m_iX == a_rOther.m_iX && m_iY == a_rOther.m_iY; }

	// $deBlob: FUNCTION 006dd210
	void operator+=( const TXUIFixedVector2& a_rOther )
	{
		m_iX += a_rOther.m_iX;
		m_iY += a_rOther.m_iY;
	}

	// $deBlob: FUNCTION 006c2600
	// $deBlob: FUNCTION 00625f10
	// $deBlob: FUNCTION 006c2640
	// $deBlob: FUNCTION 006c9a70
	// $deBlob: FUNCTION 006c9af0
	void Get( TVector2& a_rOutVec ) const
	{
		a_rOutVec.x = GetX();
		a_rOutVec.y = GetY();
	}

	TVector2 Get() const { return TVector2( GetX(), GetY() ); }

public:
	TINT16 m_iX; // 0x0
	TINT16 m_iY; // 0x2
};

template <TINT SCALE>
const TFLOAT TXUIFixedVector2<SCALE>::FLOAT_QUALITY = TFLOAT( SCALE );
template <TINT SCALE>
const TFLOAT TXUIFixedVector2<SCALE>::FLOAT_SCALE = 1.0f / FLOAT_QUALITY;

//-----------------------------------------------------------------------------
// A named frame of an element's timelines (XUR: name, time, command, target)
//-----------------------------------------------------------------------------
class XURXUINamedFrameData
{
public:
	enum Command : TUINT8
	{
		Command_Play,
		Command_Stop,
		Command_GoTo,
		Command_GoToAndPlay,
		Command_GoToAndStop,
	};

public:
	// $deBlob: FUNCTION 006e86c0
	XURXUINamedFrameData()
	{
		m_uiTime   = 0;
		m_uiName   = 0;
		m_uiTarget = 0;
		m_eCommand = 0;
	}

	~XURXUINamedFrameData() = default;

public:
	TUINT16 m_uiTime;   // 0x0
	TUINT16 m_uiName;   // 0x2 string index
	TUINT16 m_uiTarget; // 0x4 string index
	TUINT8  m_eCommand; // 0x6
};

//-----------------------------------------------------------------------------
// Base of every XUR data class: the children, named frames and timelines read from the DATA section.
// de Blob adds a size pass (+0x08, CalcPileSize) used to size the resource's memory pile before loading.
//-----------------------------------------------------------------------------
class XURXUIObjectData
{
public:
	// The flags byte in front of each child of a .xur object
	enum ChildFlags : TUINT8
	{
		ChildFlags_HasChildren    = 2,
		ChildFlags_HasNamedFrames = 4,
	};

	XURXUIObjectData();
	virtual ~XURXUIObjectData();                                                                         // +0x00

	virtual TBOOL   Load( TXUIResource& a_rResource, TUINT8*& a_rData );                                 // +0x04
	virtual TUINT32 CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData );                         // +0x08
	virtual TBOOL   ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex );               // +0x0C
	virtual TBOOL   TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ); // +0x10
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex );                // +0x14
	virtual TBOOL   IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex );                    // +0x18
	virtual TBOOL   IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex );                   // +0x1C

	TBOOL LoadChildren( TXUIResource& a_rResource, TUINT8*& a_rData );
	TBOOL LoadNamedFrames( TXUIResource& a_rResource, TUINT8*& a_rData );
	TBOOL LoadTimelines( TXUIResource& a_rResource, TUINT8*& a_rData );

	// The size passes of the Load* functions above
	TUINT32 CalcChildrenPileSize( TXUIResource& a_rResource, TUINT8*& a_rData );
	TUINT32 CalcNamedFramesPileSize( TXUIResource& a_rResource, TUINT8*& a_rData );
	TUINT32 CalcTimelinesPileSize( TXUIResource& a_rResource, TUINT8*& a_rData );

	XURXUIElementData* FindChildElementData( TUINT32 a_uiStringId );
	XURXUIElementData* FindChildElementData( TXUIResource& a_rResource, const TWCHAR* a_wszName );

	XURXUIElementData*    GetChild( TUINT32 a_uiIndex ) const { return m_apChildren[ a_uiIndex ]; }
	TUINT32               GetNumChildren() const { return m_uiNumChildren; }
	XURXUINamedFrameData* GetNamedFrames() const { return m_aNamedFrames; }
	TUINT32               GetNumNamedFrames() const { return m_uiNumNamedFrames; }
	XURXUITimelineData*   GetTimelines() const { return m_aTimelines; }
	TUINT32               GetNumTimelines() const { return m_uiNumTimelines; }
	TUINT16               GetTypeIndex() const { return m_uiTypeIndex; }
	const TClass*         GetClass() const { return m_pClass; }
	// $deBlob: FUNCTION 006be7c0
	void SetClass( const TClass* a_pClass ) { m_pClass = a_pClass; }

public:
	XURXUIElementData**   m_apChildren;        // 0x04
	XURXUINamedFrameData* m_aNamedFrames;      // 0x08
	XURXUITimelineData*   m_aTimelines;        // 0x0C
	TUINT16               m_uiNumNamedFrames;  // 0x10
	TUINT16               m_uiPropLevelMask;   // 0x12 one bit per class level with properties (most derived first)
	TUINT16               m_uiTypeIndex;       // 0x14 string index of the XUI class name
	TUINT8                m_uiNumChildren;     // 0x16
	TUINT8                m_uiNumTimelines;    // 0x17
	const TClass*         m_pClass;            // 0x18 the TClass of the element the data creates
};

class XURXUIElementData : public XURXUIObjectData
{
public:
	// Bits of m_uiFlags
	enum Flags : TUINT16
	{
		Flags_BlendModeMask                 = 0x0007,
		Flags_AnchorShift                   = 3,
		Flags_AnchorMask                    = 0x003F,
		Flags_ClipChildren                  = 0x0800,
		Flags_Show                          = 0x1000,
		Flags_DisableTimelineRecursion      = 0x2000,
		Flags_ClipChildrenShift             = 11,
		Flags_ShowShift                     = 12,
		Flags_DisableTimelineRecursionShift = 13,
	};

	enum Prop
	{
		Prop_Id,
		Prop_Width,
		Prop_Height,
		Prop_Position,
		Prop_Scale,
		Prop_Rotation,
		Prop_Opacity,
		Prop_Anchor,
		Prop_Pivot,
		Prop_Show,
		Prop_BlendMode,
		Prop_DisableTimelineRecursion,
		Prop_DesignTime,
		Prop_ColorWriteFlags,
		Prop_ClipChildren,
		Prop_NUMOF,
	};

public:
	XURXUIElementData();
	~XURXUIElementData() = default;

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;          // +0x04
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;  // +0x08
	virtual TBOOL               ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override; // +0x0C
	virtual TBOOL               TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10
	virtual TUINT32             GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override; // +0x14
	virtual TBOOL               IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;     // +0x18
	virtual TBOOL               IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;    // +0x1C
	virtual const TXUITypeInfo* GetTypeInfo() const;                                                    // +0x20

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

	TUINT16 GetId() const { return m_uiId; }

public:
	static const TXUITypeInfo sm_sTypeInfo;

public:
	TINT16  m_iWidth;    // 0x1C value * 6
	TINT16  m_iHeight;   // 0x1E value * 6
	TINT16  m_iPosition; // 0x20 index in the vector table, -1 if not set
	TINT16  m_iScale;    // 0x22 index in the vector table, -1 if not set
	TINT16  m_iRotation; // 0x24 index in the quaternion (Z rotation) table, -1 if not set
	TINT16  m_iPivot;    // 0x26 index in the vector table, -1 if not set
	TUINT16 m_uiId;      // 0x28 string index
	TUINT16 m_uiFlags;   // 0x2A see Flags
	TUINT8  m_uiOpacity; // 0x2C
};

//-----------------------------------------------------------------------------
// TXUIElement: a T2GUIElement created from XUR data. It also is a TObject (at +0x3C) so the element classes can be
// created by name through the TClass system (ClassOverride).
//-----------------------------------------------------------------------------
class TXUIElement
    : public T2GUIElement
    , public TObject
{
public:
	TDECLARE_CLASS( TXUIElement, TObject );

public:
	// Bits of m_uiXUIState
	using XUIState = TUINT32;
	enum XUIState_ : XUIState
	{
		XUIState_AnchorMask             = 0x0000003F, // 0-5
		XUIState_TimelineStateShift     = 6,          // 6-9
		XUIState_TimelineStateMask      = 0x000003C0,
		XUIState_BlendModeShift         = 10, // 10-12
		XUIState_BlendModeMask          = 0x00001C00,
		XUIState_DirtyShift             = 13, // 13-14
		XUIState_PositionDirty          = 0x00002000,
		XUIState_TransformDirty         = 0x00004000,
		XUIState_TimelineRecursion      = 0x00008000, // 15, !DisableTimelineRecursion
		XUIState_TimelineRecursionShift = 15,
		XUIState_ClipChildren           = 0x00010000, // 16
		XUIState_ClipChildrenShift      = 16,
		XUIState_HierarchicalAlphaShift = 17, // 17-24
		XUIState_HierarchicalAlphaMask  = 0x01FE0000,
		XUIState_OpacityShift           = 25, // 25-31, value * 127
		XUIState_OpacityMask            = 0xFE000000,
	};

	// The bits of m_uiXUIState >> XUIState_DirtyShift
	enum DirtyFlags : TUINT32
	{
		DirtyFlags_Position  = 1,
		DirtyFlags_Transform = 2,
	};

	// Anchor bits (XUI)
	enum Anchor : TUINT32
	{
		Anchor_Left    = BITFLAG( 0 ),
		Anchor_Top     = BITFLAG( 1 ),
		Anchor_Right   = BITFLAG( 2 ),
		Anchor_Bottom  = BITFLAG( 3 ),
		Anchor_XCenter = BITFLAG( 4 ),
		Anchor_YCenter = BITFLAG( 5 ),
	};

	// Timeline state (bits 6-9 of m_uiXUIState), TXUIScene has its own set of values
	enum TimelineState : TUINT32
	{
		TimelineState_None     = 0,
		TimelineState_Focus    = 2,
		TimelineState_Press    = 3,
		TimelineState_Paused   = 8,
		TimelineState_TransTab = 9, // TXUITabScene
	};

	// The buttons TXUI::HandleButton routes to the controls
	using XUIButton = TINT;
	enum XUIButton_ : XUIButton
	{
		XUIButton_Accept,
		XUIButton_Back,
		XUIButton_Up,
		XUIButton_Down,
		XUIButton_Left,
		XUIButton_Right,
	};

	// The messages of SendMessage, sent when the named frame of a control's timeline ends (TXUIControl dispatches them
	// to Press, EndPress, Focus, NavLeft, NavRight, NavUp, NavDown and Back, a TXUIScene to its transition handlers)
	using XUIMessage = TINT;
	enum XUIMessage_ : XUIMessage
	{
		XUIMessage_Press            = 1,
		XUIMessage_EndPress         = 2,
		XUIMessage_Focus            = 3,
		XUIMessage_EndTransFrom     = 4,
		XUIMessage_EndTransTo       = 5,
		XUIMessage_EndTransBackFrom = 6,
		XUIMessage_EndTransBackTo   = 7,
		XUIMessage_EndScrolling     = 8,
		XUIMessage_NamedFrame       = 9, // a custom named frame, the name is the parameter
	};

public:
	TXUIElement();
	~TXUIElement();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL           IsPaused() override;                                    // +0x08
	virtual TBOOL           SkipRender() override;                                  // +0x0C
	virtual void            PreRender() override;                                   // +0x10
	virtual void            Render() override;                                      // +0x14
	virtual void            PostRender() override;                                  // +0x18
	virtual void            SetWidth( TFLOAT a_fWidth ) override;                   // +0x28
	virtual void            SetHeight( TFLOAT a_fHeight ) override;                 // +0x2C
	virtual void            SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) override; // +0x30
	virtual void            SetAlpha( TFLOAT a_fAlpha ) override;                   // +0x40
	virtual TFLOAT          GetAlpha() override;                                    // +0x44
	virtual void            GetScreenTransform( T2GUITransform& a_rOutTransform ) override; // +0x50
	virtual const TVector2& GetPivot() override;                                    // +0x58

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL        Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ); // +0x5C
	virtual void         SetOpacity( TFLOAT a_fOpacity );                                                  // +0x60
	virtual TXUIElement* RayTraceDeepest( const TVector2& a_rPoint, const TVector2& a_rScale, TINT a_iController ); // +0x64
	// NOTE: MSVC puts overloaded virtuals into the vtable in reverse declaration order, so the overload pairs are
	// declared in reverse to get retail's slots (+0x68 frames, +0x6C names; +0x7C indices, +0x80 floats)
	virtual void         PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame );      // +0x6C
	virtual void         PlayAnimation( TINT a_iStartFrame, TINT a_iEndFrame );                            // +0x68
	virtual void         PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd );                              // +0x70
	virtual void         StopAnimation();                                                                  // +0x74
	virtual TBOOL        SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam );                         // +0x78
	virtual void         UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar );     // +0x80
	virtual void         UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ); // +0x7C
	virtual void         SetPresenterData( const TWCHAR* a_wszText, const TWCHAR* a_wszImagePath, TUINT32 a_uiTextIndex, TUINT32 a_uiImageIndex, TINT a_iParam ); // +0x84
	virtual void         SetPresenterText( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam );   // +0x88
	virtual void         SetPresenterText2( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam );  // +0x8C (TXUIScene forwards it to its visual)
	virtual void         SetPresenterImagePath( const TWCHAR* a_wszImagePath, TUINT32 a_uiIndex );        // +0x90
	virtual void         OnCreated();                                                                     // +0x94 called by TXUIResource::CreateScene after Create
	virtual void         OnRayTraceHit( TINT a_iController );                                             // +0x98
	virtual void         SetXUIAnchoring( TUINT32 a_uiAnchoring );                                         // +0x9C
	virtual TBOOL        IsCheckable();                                                                    // +0xA0
	virtual TBOOL        IsEnabled();                                                                      // +0xA4
	virtual TBOOL        IsChecked();                                                                      // +0xA8
	virtual void         ResolveNavigation( TXUIElement* a_pRoot );                                        // +0xAC
	virtual TBOOL        OnNotifyPress( TXUIElement* a_pSource, TINT a_iController );                      // +0xB0
	virtual TBOOL        OnNotifySetFocus( TXUIElement* a_pSource );                                       // +0xB4
	virtual TBOOL        OnNotifyKillFocus( TXUIElement* a_pSource );                                      // +0xB8
	virtual TBOOL        OnNotifyValueChanged( TXUIElement* a_pSource );                                   // +0xBC

	void CreateChildren( TXUIResource& a_rResource, XURXUIObjectData* a_pData );
	void UpdateAnchoring( const TVector2& a_rSizeDelta );
	void UpdateHierarchicalAlpha();

	TBOOL RayTrace( const TVector2& a_rPoint );

	TXUIElement* FindChildXUIElement( const TWCHAR* a_wszName );
	TXUIElement* FindChildXUIElement( TINT a_iUID );
	TXUIControl* FindChildXUIControl( const TWCHAR* a_wszName );
	TXUIControl* FindChildXUIControl( TINT a_iUID );
	TXUIElement* FindParentControl();
	TXUIElement* FindParentScene();

	static void PushClip( TFLOAT a_fWidth, TFLOAT a_fHeight );
	static void PopClip();

	static TUINT32 InterpColour( TUINT32 a_uiColour1, TUINT32 a_uiColour2, TFLOAT a_fScalar );
	static TUINT32 MyMakeARGB( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha );
	static void    MyUnpackARGB( TUINT32 a_uiColour, TFLOAT& a_rAlpha, TFLOAT& a_rRed, TFLOAT& a_rGreen, TFLOAT& a_rBlue );

	// $deBlob: FUNCTION 006c20f0
	static TXUIElement* CastFromTObject( TObject* a_pObject )
	{
		TASSERT( ( a_pObject == TNULL ) || a_pObject->IsA( &TGetClass( TXUIElement ) ) );
		return a_pObject ? TSTATICCAST( TXUIElement, a_pObject ) : TNULL;
	}

	//-----------------------------------------------------------------------------
	// m_uiXUIState helpers (TXUIElement.h:346-348, "(a_flags&a_mask) == a_flags")
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 006c2950
	// $deBlob: FUNCTION 006c29b0
	// $deBlob: FUNCTION 006c2a10
	// $deBlob: FUNCTION 006c2a70
	// $deBlob: FUNCTION 005b9e70
	// $deBlob: FUNCTION 006260d0
	// $deBlob: FUNCTION 005b9f10
	void SetXUIStateBits( TUINT32 a_uiFlags, TUINT32 a_uiMask, TUINT32 a_uiShift )
	{
		TASSERT( ( a_uiFlags & a_uiMask ) == a_uiFlags );
		m_uiXUIState = ( m_uiXUIState & ~( a_uiMask << a_uiShift ) ) | ( a_uiFlags << a_uiShift );
	}

	// $deBlob: FUNCTION 00574f70
	// $deBlob: FUNCTION 00574fc0
	void SetXUIStateFlags( TUINT32 a_uiFlags, TUINT32 a_uiMask, TUINT32 a_uiShift )
	{
		TASSERT( ( a_uiFlags & a_uiMask ) == a_uiFlags );
		m_uiXUIState |= a_uiFlags << a_uiShift;
	}

	// $deBlob: FUNCTION 006c28a0
	// $deBlob: FUNCTION 006c28f0
	void ClearXUIStateFlags( TUINT32 a_uiFlags, TUINT32 a_uiMask, TUINT32 a_uiShift )
	{
		TASSERT( ( a_uiFlags & a_uiMask ) == a_uiFlags );
		m_uiXUIState &= ~( a_uiFlags << a_uiShift );
	}

	// $deBlob: FUNCTION 00625e50
	TFLOAT GetFlags_Opacity() const { return TFLOAT( m_uiXUIState >> XUIState_OpacityShift ) * ( 1.0f / 127.0f ); }

	// NOTE: retail truncates (SetFlags_HierarchicalAlpha rounds)
	// $deBlob: FUNCTION 00626130
	void SetFlags_Opacity( TFLOAT a_fOpacity )
	{
		SetXUIStateBits( TUINT32( TINT( a_fOpacity * 127.0f ) ), 0x7F, XUIState_OpacityShift );
	}

	// $deBlob: FUNCTION 006c2540
	// $deBlob: FUNCTION 006c25b0
	TFLOAT GetFlags_HierarchicalAlpha() const { return TFLOAT( ( m_uiXUIState >> XUIState_HierarchicalAlphaShift ) & 0xFF ) * ( 1.0f / 255.0f ); }

	// $deBlob: FUNCTION 006c2ad0
	void SetFlags_HierarchicalAlpha( TFLOAT a_fAlpha )
	{
		SetXUIStateBits( TUINT32( TMath::Round( a_fAlpha * 255.0f ) ), 0xFF, XUIState_HierarchicalAlphaShift );
	}

	// $deBlob: FUNCTION 006e65a0
	void SetDisableTimelineRecursion( TBOOL a_bDisable ) { m_uiXUIState = ( m_uiXUIState & ~XUIState_TimelineRecursion ) | ( TUINT32( !a_bDisable ) << XUIState_TimelineRecursionShift ); }

	// $deBlob: FUNCTION 005807c0
	void SetPosition( TFLOAT a_fX, TFLOAT a_fY )
	{
		m_vPosition.Set( a_fX, a_fY );
		m_uiXUIState |= XUIState_PositionDirty;
	}

	// NOTE: inlined (AGUICursor sets the pivot of its scene without a dirty flag)
	void SetXUIPivot( TFLOAT a_fX, TFLOAT a_fY ) { m_vPivot.Set( a_fX, a_fY ); }

	// NOTE: inlined (AApplication::OnCreate scales the canvas with it)
	void SetScale( TFLOAT a_fX, TFLOAT a_fY )
	{
		m_vScale.Set( a_fX, a_fY );
		m_uiXUIState |= XUIState_TransformDirty;
	}

	// $deBlob: FUNCTION 00629e90
	void SetZRotation( TFLOAT a_fAngle )
	{
		m_uiXUIState |= XUIState_TransformDirty;
		m_fZRotation = a_fAngle;
	}

	// $deBlob: FUNCTION 006e4c30
	void SetPositionScale( const TVector2& a_rScale ) { m_vPositionScale.Set( a_rScale.x, a_rScale.y ); }

	TFLOAT GetPositionX() const { return m_vPosition.GetX(); }
	TFLOAT GetScaleX() const { return m_vScale.GetX(); }
	TFLOAT GetScaleY() const { return m_vScale.GetY(); }
	TFLOAT GetPositionY() const { return m_vPosition.GetY(); }

	// The scale of the size change of the last child UpdateAnchoring resized
	static const TVector2& GetAnchoringScale();

	// $deBlob: FUNCTION 00625e80
	TUINT32 GetTimelineState() const { return ( m_uiXUIState >> XUIState_TimelineStateShift ) & 0xF; }
	// $deBlob: FUNCTION 006c2530
	TBOOL   IsTimelineRecursionEnabled() const { return ( m_uiXUIState & XUIState_TimelineRecursion ) != 0; }
	TBOOL   IsXUIVisible() const { return IsVisible(); }

	XURXUIObjectData* GetData() const { return m_pData; }
	// $deBlob: FUNCTION 00580470
	const TWCHAR*     GetID() const { return m_wszID; }
	TINT              GetUID() const { return m_iUID; }

	// $deBlob: FUNCTION 006c9dc0
	void SetUID( TINT a_iUID ) { m_iUID = a_iUID; }
	// $deBlob: FUNCTION 006424c0
	TFLOAT            GetZRotation() const { return m_fZRotation; }

	// NOTE: the TObject part at +0x3C
	TObject*       GetTObject() { return this; }
	const TObject* GetTObject() const { return this; }

public:
	static TUINT32 s_uiNumClips;

protected:
	XURXUIObjectData*      m_pData;          // 0x40
	const TWCHAR*          m_wszID;          // 0x44 the element's Id
	TXUIFixedVector2<32>   m_vPivot;         // 0x48
	TXUIFixedVector2<6>    m_vPosition;      // 0x4C
	TXUIFixedVector2<256>  m_vScale;         // 0x50
	TXUIFixedVector2<6>    m_vPositionScale; // 0x54 1.0 by default, scales the timeline positions
	TFLOAT                 m_fZRotation;     // 0x58
	XUIState               m_uiXUIState;     // 0x5C
	TINT                   m_iUID;           // 0x60 -1 if not set
};

TOSHI_NAMESPACE_END
