#pragma once
#include "Toshi/Xui/XUI.h"
#include "Toshi/Xui/TXUIResource.h"
#include "Toshi/Xui/TXUITimeline.h"
#include "Toshi2/T2GUI/T2GUIElement.h"

#define TXUI_TRANSLATE_TIMELINE_PROP(propName1, propName2, typeOut)     \
	if (TStringManager::String8Compare(propName1, #propName2, -1) == 0) \
	{                                                                   \
		typeOut = PropType_##propName2;                                 \
		return TTRUE;                                                   \
	}

#define TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(propName1, propName2, typeOut, flag) \
	if (TStringManager::String8Compare(propName1, propName2, -1) == 0)           \
	{                                                                            \
		typeOut = flag;                                                          \
		return TTRUE;                                                            \
	}

#define TXUI_TYPEINFO_INIT(class_name)                                     \
protected:                                                                 \
	struct TXUITypeInfo                                                    \
	{                                                                      \
		const TWCHAR*       m_wsClassName;                                 \
		const TXUITypeInfo* m_pParent;                                     \
	};                                                                     \
                                                                           \
private:                                                                   \
	static inline const TXUITypeInfo sm_sTypeInfo = { class_name, TNULL }; \
                                                                           \
public:                                                                    \
	static const TXUITypeInfo* GetTypeInfoStatic()                         \
	{                                                                      \
		return &sm_sTypeInfo;                                              \
	}                                                                      \
                                                                           \
public:                                                                    \
	virtual const TXUITypeInfo* GetTypeInfo() const                        \
	{                                                                      \
		return GetTypeInfoStatic();                                        \
	}

#define TXUI_TYPEINFO(class_name, parent)                                                        \
private:                                                                                         \
	static inline const TXUITypeInfo sm_sTypeInfo = { class_name, parent::GetTypeInfoStatic() }; \
                                                                                                 \
public:                                                                                          \
	static const TXUITypeInfo* GetTypeInfoStatic()                                               \
	{                                                                                            \
		return &sm_sTypeInfo;                                                                    \
	}                                                                                            \
                                                                                                 \
public:                                                                                          \
	virtual const TXUITypeInfo* GetTypeInfo() const                                              \
	{                                                                                            \
		return GetTypeInfoStatic();                                                              \
	}

#define TXUI_PROPTYPE_TO_READFLAG(propType) (1 << propType)
#define TXUI_CHECK_READFLAG(flag, propType) (flag & TXUI_PROPTYPE_TO_READFLAG(propType))

TOSHI_NAMESPACE_START

enum Flags : TUINT8
{
	Flags_BlendModeMask = BITFIELD(3),
	Flags_AnchorMask    = BITFIELD(6)
};

class TXUIElement;

class XURXUIObjectData
{
public:
	typedef TUINT32 PropType;
	enum PropType_ : PropType
	{
		PropType_Id,
		PropType_Width,
		PropType_Height,
		PropType_Position,
		PropType_Scale,
		PropType_Rotation,
		PropType_Opacity,
		PropType_Anchor,
		PropType_Pivot,
		PropType_Show,
		PropType_BlendMode,
		PropType_DisableTimelineRecursion,
		PropType_DesignTime,
		PropType_ColorWriteFlags,
		PropType_ClipChildren,
		PropType_NUMOF,
	};

public:
	XURXUIObjectData()
	{
		m_Index          = 0;
		m_uiTypeIndex    = 0;
		m_NumNamedFrames = 0;
		m_NumTimelines   = 0;
		m_pNamedFrames   = TNULL;
		m_TimelinesData  = TNULL;
		m_Children       = TNULL;
		m_NumChildren    = 0;
		m_pClass         = TNULL;
	}

	virtual ~XURXUIObjectData();

	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData);
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, PropType propType) { return TFALSE; };
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, PropType propType) { return TFALSE; };
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, PropType propType) { return 0; };
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) { return TFALSE; };
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) { return TFALSE; };

	void                     LoadChildren(TXUIResource& resource, TUINT8*& a_pData);
	TBOOL                    LoadNamedFrames(TXUIResource& resource, TUINT8*& a_pData);
	void                     LoadTimelines(TXUIResource& resource, TUINT8*& a_pData);
	class XURXUIElementData* FindChildElementData(TUINT32 a_iStringId);

public:
	XURXUIObjectData**    m_Children;       // 0x4 both
	XURXUINamedFrameData* m_pNamedFrames;   // 0x8 both
	XURXUITimelineData*   m_TimelinesData;  //
	XUIEPTUShort32        m_NumNamedFrames; // 0x10 de blob 0x14 NT08
	TUINT16               m_Index;          // 0x12 de blob 0x1C NT08
	TUINT32               m_uiTypeIndex;    // 0x14 de blob 1E NT08
	TUINT8                m_NumChildren;    // 0x16 de blob 0x10 NT08
	TUINT8                m_NumTimelines;   // 0x17
	const TClass*         m_pClass;         // 0x18
};

class XURXUIElementData : public XURXUIObjectData
{
	TXUI_TYPEINFO_INIT(XUI_CLASS_ELEMENT);

public:
	friend XURXUIObjectData;

public:
	XURXUIElementData()
	{
		m_Width    = T2GUIElement::PackFloat(60.0f);
		m_Height   = T2GUIElement::PackFloat(30.0f);
		m_Opacity  = 255;
		m_Position = -1;
		m_Scale    = -1;
		m_Rotation = -1;
		m_Pivot    = -1;
		m_Flags    = 0x1000;
		m_Id       = 0;
	}

	~XURXUIElementData() = default;

	TBOOL Load(TXUIResource& resource, TUINT8*& a_pData);

	TBOOL TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType);

	TBOOL ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex);

	TBOOL IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) { return propType == PropType_Width || propType == PropType_Height || propType == PropType_Opacity; }

	// No it's not
	TBOOL IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) { return TFALSE; }

	TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
	{
		if (propType != 11 && propType != 9 && propType != 14)
		{
			if (propType != 0) return 4;
			return 2;
		}
		return 1;
	}

	XUIEPTString GetId() const { return m_Id; }

	T2GUIElement::Float GetWidth() const { return m_Width; }

	T2GUIElement::Float GetHeight() const { return m_Height; }

	XUIEPTVector GetPosition() const { return m_Position; }

	XUIEPTVector GetScale() const { return m_Scale; }

	XUIEPTQuaternion GetRotation() const { return m_Rotation; }

	TBOOL IsVisible() const { return m_Flags & 0x1000; }

protected:
	T2GUIElement::Float m_Width;
	T2GUIElement::Float m_Height;
	XUIEPTVector        m_Position;
	XUIEPTVector        m_Scale;
	XUIEPTQuaternion    m_Rotation;
	XUIEPTVector        m_Pivot;
	XUIEPTString        m_Id;
	TUINT32             m_Flags;
	TUINT8              m_Opacity;
};

class TXUIElement
    : public TGenericClassDerived<TXUIElement, TObject, "TXUIElement", TMAKEVERSION(1, 0), TFALSE>
    , public T2GUIElement
{
public:
	static constexpr TFLOAT FLOAT_QUALITY = 127;
	static constexpr TFLOAT FLOAT_SCALE   = 1 / FLOAT_QUALITY;

	using XUIState = TUINT32;

	enum XUIState_ : XUIState
	{
		XUIState_ANCHOR_NONE    = BITFIELD(0),
		XUIState_ANCHOR_LEFT    = BITFIELD(1),
		XUIState_ANCHOR_TOP     = BITFIELD(2),
		XUIState_ANCHOR_RIGHT   = BITFIELD(3),
		XUIState_ANCHOR_BOTTOM  = BITFIELD(4),
		XUIState_ANCHOR_HCENTER = BITFIELD(5),
		XUIState_ANCHOR_VCENTER = BITFIELD(6),
		XUIState_ANCHOR_XSCALE  = BITFIELD(7),
		XUIState_ANCHOR_YSCALE  = BITFIELD(8),

		XUIState_PAUSED              = BITFIELD(9),
		XUIState_NOTIMELINERECURSION = BITFIELD(10),
		XUIState_CLIPCHILDREN        = BITFIELD(16),
		XUIState_STATEMASK           = XUIState_PAUSED | 0b111000000,
		XUIState_OPACITYMASK         = 0b11111110000000000000000000000000,
		XUIState_ALPHAMASK           = 0b1111111100000000000000000,
		XUIState_ANCHORMASK          = 0b0000000000000000000111111,
		XUIState_BLENDMODEMASK       = 0b0000000000001110000000000
	};

public:
	TXUIElement();

	static TFLOAT UnpackFloat(Float value) { return value * FLOAT_SCALE; }

	static Float PackFloat(TFLOAT value) { return static_cast<Float>(value * FLOAT_QUALITY); }

	TFLOAT GetFlags_Opacity() { return UnpackFloat(m_eXUIState >> 0x19); }

	virtual TBOOL SkipRender() override;
	virtual TBOOL IsPaused() const override;
	virtual void  SetHeight(TFLOAT height) override;
	virtual void  SetWidth(TFLOAT width) override;

	virtual void SetXUIAnchoring(TUINT32 a_flags, TUINT32 a_mask = 0x3F)
	{
		TASSERT((a_flags & a_mask) == a_flags);
		m_eXUIState = m_eXUIState & (~a_mask) | a_flags;
	}

	TBOOL Create(TXUIResource& a_rResource, XURXUIElementData* a_pElementData, TBOOL hasChildren);
	void  CreateChildren(TXUIResource& a_rResource, XURXUIElementData* a_pElementData);
	void  UpdateAnchoring(const TVector2& vec);
	TBOOL IsVisible();

private:
	XURXUIObjectData*        m_pObjectData; // 0x40
	const TWCHAR*            m_objectID;    // 0x44
	T2GUITransform::Rotation m_vPosition;   // 0x48
	T2GUITransform::Rotation m_vRotation;   // 0x4C
	T2GUITransform::Rotation m_vScale;      // 0x50
	XUIState                 m_eXUIState;   // 0x5C

	TINT m_iUIDCount; // 0xB4 globs
};

TOSHI_NAMESPACE_END
