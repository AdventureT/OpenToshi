#include "ToshiPCH.h"
#include "TXUIButton.h"
#include "TXUIBackButton.h"

bool Toshi::XURXUIBackButtonData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0) return XURXUIBackButtonData::IsColourPropType(a_uiObjectIndex - 1, propType);
	return false;
}

bool Toshi::XURXUIBackButtonData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0) return XURXUIBackButtonData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	return false;
}

uint32_t Toshi::XURXUIBackButtonData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0) return XURXUIBackButtonData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	return 4;
}

bool Toshi::XURXUIBackButtonData::TranslateTimelineProp(const char* name, uint32_t& param_2, PropType& propType)
{
	name++;
	return XURXUIBackButtonData::TranslateTimelineProp(name, param_2, propType);
}

bool Toshi::XURXUIBackButtonData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t param_2)
{
	if (a_uiObjectIndex != 0) return XURXUIBackButtonData::ValidateTimelineProp(a_uiObjectIndex - 1, param_2);
	return false;
}

bool Toshi::XURXUIBackButtonData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	a_pData++;
	return XURXUIButtonData::Load(resource, a_pData);
}
