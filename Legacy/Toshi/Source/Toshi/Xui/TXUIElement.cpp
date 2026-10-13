#include "ToshiPCH.h"
#include "TXUIElement.h"
#include "Toshi/Xui/TXUI.h"
#include "Toshi/Xui/TXUIListItem.h"
#include "XURReader.h"
#include <Toshi/Xui/TXUIVisual.h>

TOSHI_NAMESPACE_BEGIN

// $deBlob: FUNCTION 006be810
XURXUIObjectData::~XURXUIObjectData()
{
	for (TUINT8 i = 0; i < m_NumChildren; i++)
	{
		if (m_Children[i] != TNULL) delete m_Children[i];
	}

	delete[] m_Children;

	if (m_pNamedFrames) delete[] m_pNamedFrames;

	if (m_TimelinesData) delete[] m_TimelinesData;
}

// $deBlob: FUNCTION 006be890
TBOOL XURXUIObjectData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURReader reader(a_pData);
	m_Index = reader.ReadUInt16();
	return TTRUE;
}

// $deBlob: FUNCTION 006be930
void XURXUIObjectData::LoadChildren(TXUIResource& resource, TUINT8*& a_pData)
{
	XURReader reader(a_pData);
	m_NumChildren = reader.ReadUInt8From32();
	m_Children    = new (TXUI::MemoryBlock()) XURXUIObjectData*[m_NumChildren];
	TASSERT(m_Children != TNULL);

	for (size_t i = 0; i < m_NumChildren; i++)
	{
		TUINT16 uiType = reader.ReadUInt16();

		m_Children[i]                = TXUIResource::CreateObjectData(resource, uiType);
		m_Children[i]->m_uiTypeIndex = uiType;
		TUINT8 opcode                = reader.ReadUInt8();

		m_Children[i]->Load(resource, a_pData);

		if (HASFLAG(opcode & 2))
		{
			m_Children[i]->LoadChildren(resource, a_pData);
		}

		if (HASFLAG(opcode & 4) && m_Children[i]->LoadNamedFrames(resource, a_pData) && HASFLAG(opcode & 2))
		{
			m_Children[i]->LoadTimelines(resource, a_pData);
		}
	}
}

// $deBlob: FUNCTION 006beae0
TBOOL XURXUIObjectData::LoadNamedFrames(TXUIResource& resource, TUINT8*& a_pData)
{
	XURReader reader(a_pData);
	m_NumNamedFrames = reader.ReadEPTUShort32();
	TASSERT(m_NumNamedFrames < 1000, "Max of named Frames reached");

	if (m_NumNamedFrames != 0)
	{
		m_pNamedFrames = new (TXUI::MemoryBlock()) XURXUINamedFrameData[m_NumNamedFrames];
		TASSERT(m_pNamedFrames != TNULL);

		for (size_t i = 0; i < m_NumNamedFrames; i++)
		{
			// Order not correct!
			m_pNamedFrames[i].m_Time          = reader.ReadUInt16();
			m_pNamedFrames[i].m_Name          = reader.ReadUInt16From32();
			m_pNamedFrames[i].m_Command       = reader.ReadUInt8();
			m_pNamedFrames[i].m_CommandParams = reader.ReadUInt16();
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006becf0
void XURXUIObjectData::LoadTimelines(TXUIResource& resource, TUINT8*& a_pData)
{
	XURReader reader(a_pData);
	m_NumTimelines = reader.ReadUInt8From32();

	if (m_NumTimelines != 0)
	{
		m_TimelinesData = new (TXUI::MemoryBlock()) XURXUITimelineData[m_NumTimelines];
		TASSERT(m_TimelinesData != TNULL);

		for (size_t i = 0; i < m_NumTimelines; i++)
		{
			m_TimelinesData[i].SetOwner(this);
			m_TimelinesData[i].Load(resource, a_pData);
		}
	}
}

// $deBlob: FUNCTION 006bf190
XURXUIElementData* XURXUIObjectData::FindChildElementData(TUINT32 a_iStringId)
{
	for (size_t i = 0; i < m_NumChildren; i++)
	{
		XURXUIElementData* pElementData = TSTATICCAST(XURXUIElementData, m_Children[i]);

		if (pElementData->GetId() == a_iStringId)
		{
			return pElementData;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 006bf280
TBOOL XURXUIElementData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIObjectData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		XUIEPTFloat    width, height, opacity;
		XUIEPTUShort32 anchor, blendMode;
		XUIEPTBool     show, disableTimelineRecursion, clipChildren, designTime;
		XUIEPTUnsigned colorWriteFlags;

		TBOOL hasId                       = reader.ReadProperty<XUI_EPT_STRING>(PropType_Id, m_Id);
		TBOOL hasWidth                    = reader.ReadProperty<XUI_EPT_FLOAT>(PropType_Width, width);
		TBOOL hasHeight                   = reader.ReadProperty<XUI_EPT_FLOAT>(PropType_Height, height);
		TBOOL hasPosition                 = reader.ReadProperty<XUI_EPT_VECTOR>(PropType_Position, m_Position);
		TBOOL hasScale                    = reader.ReadProperty<XUI_EPT_VECTOR>(PropType_Scale, m_Scale);
		TBOOL hasRotation                 = reader.ReadProperty<XUI_EPT_QUATERNION>(PropType_Rotation, m_Rotation);
		TBOOL hasOpacity                  = reader.ReadProperty<XUI_EPT_FLOAT>(PropType_Opacity, opacity);
		TBOOL hasAnchor                   = reader.ReadProperty<XUI_EPT_USHORT32>(PropType_Anchor, anchor);
		TBOOL hasPivot                    = reader.ReadProperty<XUI_EPT_VECTOR>(PropType_Pivot, m_Pivot);
		TBOOL hasShow                     = reader.ReadProperty<XUI_EPT_BOOL>(PropType_Show, show);
		TBOOL hasBlendMode                = reader.ReadProperty<XUI_EPT_USHORT32>(PropType_BlendMode, blendMode);
		TBOOL hasDisableTimelineRecursion = reader.ReadProperty<XUI_EPT_BOOL>(PropType_DisableTimelineRecursion, disableTimelineRecursion);
		TBOOL hasDesignTime               = reader.ReadProperty<XUI_EPT_BOOL>(PropType_DesignTime, designTime);
		TBOOL hasColorWriteFlags          = reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_ColorWriteFlags, colorWriteFlags);
		;
		TBOOL hasClipChildren = reader.ReadProperty<XUI_EPT_BOOL>(PropType_ClipChildren, clipChildren);

		if (hasWidth)
		{
			m_Width = T2GUIElement::PackFloat(width);
		}

		if (hasHeight)
		{
			m_Height = T2GUIElement::PackFloat(height);
		}

		if (hasOpacity)
		{
			TASSERT(opacity >= 0.0f && opacity <= 1.0f);
			m_Opacity = TUINT8(opacity * 255);
		}

		if (hasAnchor)
		{
			TASSERT(anchor <= Flags_AnchorMask);
			m_Flags |= anchor << 3;
		}

		if (hasShow)
		{
			if (show)
			{
				m_Flags |= 0x1000;
			}
			else
			{
				m_Flags &= ~0x1000;
			}
		}

		if (hasBlendMode)
		{
			m_Flags |= blendMode;
		}

		if (hasDisableTimelineRecursion)
		{
			if (disableTimelineRecursion)
			{
				m_Flags = 0x2000;
			}
			else
			{
				m_Flags &= ~0x2000;
			}
		}

		if (hasClipChildren)
		{
			if (clipChildren)
			{
				m_Flags |= 0x800;
			}
			else
			{
				m_Flags &= ~0x800;
			}
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006bf330
TBOOL XURXUIElementData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, Id, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Width, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Height, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Position, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Scale, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Rotation, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Opacity, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Anchor, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Pivot, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Show, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, BlendMode, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, DisableTimelineRecursion, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, ColorWriteFlags, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, ClipChildren, propType);

	return TFALSE;
}

// $deBlob: FUNCTION 006bf2e0
TBOOL XURXUIElementData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	TASSERT(a_uiObjectIndex == 0);
	return a_uiPropIndex < PropType_NUMOF;
}

// $deBlob: FUNCTION 006bfb20
// TXUIElement

TXUIElement::TXUIElement()
{
	m_pObjectData = TNULL;
	m_Flags |= FLAGS_XUIELEMENT;
	m_vRotation = T2GUITransform::Rotation(0, 0);
	m_vPosition = T2GUITransform::Rotation(0, 0);
	m_vScale    = T2GUITransform::Rotation(0, 0);
	m_eXUIState = 0xc000;
	m_eXUIState = m_eXUIState & 0xfe01ffff | 0x1fe0000;
	m_eXUIState = m_eXUIState & 0x01ffffff | 0xfe000000;
}

// $deBlob: FUNCTION 00626380
TBOOL TXUIElement::SkipRender()
{
	// Skip Rendering since Scale is 0.0f and/or Opacity is 0.0f which you can't see
	if (GetFlags_Opacity() == 0.0f || m_vScale.GetX() == 0.0f || m_vScale.GetY() == 0.0f)
	{
		return TTRUE;
	}
	return T2GUIElement::SkipRender();
}

// $deBlob: FUNCTION 00626410
TBOOL TXUIElement::IsPaused() const
{
	return (m_eXUIState & XUIState_STATEMASK) == XUIState_PAUSED;
}

// $deBlob: FUNCTION 006c0440
void TXUIElement::SetHeight(TFLOAT height)
{
	TVector2 currentDimension;
	GetDimensions(currentDimension.x, currentDimension.y);

	T2GUIElement::SetHeight(height);

	TVector2 newDimension;
	GetDimensions(newDimension.x, newDimension.y);

	if (!currentDimension.IsEqual(newDimension))
	{
		TVector2 combined = currentDimension - newDimension;
		UpdateAnchoring(combined);
	}
}

// $deBlob: FUNCTION 006c0330
void TXUIElement::SetWidth(TFLOAT width)
{
	TVector2 currentDimension;
	GetDimensions(currentDimension.x, currentDimension.y);

	T2GUIElement::SetWidth(width);

	TVector2 newDimension;
	GetDimensions(newDimension.x, newDimension.y);

	if (!currentDimension.IsEqual(newDimension))
	{
		TVector2 combined = currentDimension - newDimension;
		UpdateAnchoring(combined);
	}
}

// $deBlob: FUNCTION 006c1d40
void TXUIElement::UpdateAnchoring(const TVector2& vec)
{
	TIMPLEMENT();
}

// $deBlob: FUNCTION 005805f0
TBOOL TXUIElement::IsVisible()
{
	if (HASFLAG(m_State & FLAGS_VISIBLE) && HASFLAG(m_State & (TUINT8)T2GUIElement::s_uiGlobalVisMask))
	{
		return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006bfd30
TBOOL TXUIElement::Create(TXUIResource& a_rResource, XURXUIElementData* a_pElementData, TBOOL hasChildren)
{
	m_pObjectData = a_pElementData;

	if (GetClass() == TGetClass(TXUICanvas))
	{
		a_rResource.PushID(a_rResource.GetString(a_pElementData->GetId()));
	}

	m_objectID = a_rResource.GetString(a_pElementData->GetId());
	if (TXUIResource::s_bGenerateUIDs && TStringManager::String16Length(m_objectID) != 0)
	{
		if (GetClass()->IsA(TGetClass(TXUIListItem)))
		{
			m_iUIDCount = TXUIResource::s_iUIDCount;
			TXUIResource::s_iUIDCount++;
		}
	}

	m_Width  = a_pElementData->GetWidth();
	m_Height = a_pElementData->GetHeight();

	if (a_pElementData->GetPosition() != -1)
	{
		auto pos = a_rResource.GetVector(a_pElementData->GetPosition());

		m_vPosition.SetX(pos->x);
		m_vPosition.SetY(pos->x);
	}

	if (a_pElementData->GetRotation() != -1)
	{
		//m_Rotation = a_rResource.GetQuat(a_pObjectData->m_Scale);
	}

	if (a_pElementData->GetScale() != -1)
	{
		auto scale = a_rResource.GetVector(a_pElementData->GetScale());
		m_vScale.SetX(scale->x);
		m_vScale.SetY(scale->x);
	}

	SetVisible(a_pElementData->IsVisible());

	if (hasChildren)
	{
		CreateChildren(a_rResource, a_pElementData);
	}

	if (GetClass()->IsExactly(TGetClass(TXUICanvas)))
	{
		a_rResource.PopID();
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c00c0
void TXUIElement::CreateChildren(TXUIResource& a_rResource, XURXUIElementData* a_pElementData)
{
	for (TUINT16 i = 0; i < a_pElementData->m_NumChildren; i++)
	{
		auto child = a_pElementData->m_Children[i];
		if (!child->m_pClass->IsA(TGetClass(TXUIListItem)))
		{
			if (child->m_pClass->IsA(TGetClass(TXUIControl)))
			{
				const TWCHAR* str  = a_rResource.GetString(i);
				TINT          iLen = TStringManager::String16Length(str);
				TASSERT(iLen < 128);
				TCHAR string[128];
				TStringManager::StringUnicodeToChar(string, str, iLen);
				const TClass* tClass = TClass::Find(string, TGetClass(TXUIControl));
			}
		}
	}

	TIMPLEMENT();
}

TOSHI_NAMESPACE_END
