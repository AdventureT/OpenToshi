#include "ToshiPCH.h"
#include "TXUIVisual.h"

TOSHI_NAMESPACE_USING

XURXUIVisualData::XURXUIVisualData() : XURXUIElementData()
{
    m_pClass = TClass::Find("TXUIVisual", TGetClass(TXUIElement));
}

TBOOL XURXUIVisualData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
    XURXUIElementData::Load(resource, a_pData);
    a_pData++;
    return TTRUE;
}

TBOOL XURXUIVisualData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
{
    return a_uiObjectIndex != 0 ? XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex) : TFALSE;
}

TBOOL XURXUIVisualData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
{
    return XURXUIElementData::TranslateTimelineProp(name, ++a_uiObjectIndex, propType);
}

uint32_t XURXUIVisualData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
    return a_uiObjectIndex != 0 ? XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType) : 4;
}

TBOOL XURXUIVisualData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
    return a_uiObjectIndex != 0 ? XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType) : TFALSE;
}

TBOOL XURXUIVisualData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
    return a_uiObjectIndex != 0 ? XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType) : TFALSE;
}
