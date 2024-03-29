#include "ToshiPCH.h"
#include "TXUIButton.h"
#include "TXUIBackButton.h"

namespace Toshi
{
	TBOOL XURXUIBackButtonData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIButtonData::IsColourPropType(a_uiObjectIndex - 1, propType);
		return TFALSE;
	}

	TBOOL XURXUIBackButtonData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIButtonData::IsFloatPropType(a_uiObjectIndex - 1, propType);
		return TFALSE;
	}

	uint32_t XURXUIBackButtonData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
	{
		if (a_uiObjectIndex != 0) return XURXUIButtonData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
		return 4;
	}

	TBOOL XURXUIBackButtonData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
	{
		a_uiObjectIndex++;
		return XURXUIButtonData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
	}

	TBOOL XURXUIBackButtonData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
	{
		if (a_uiObjectIndex != 0) return XURXUIButtonData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
		return TFALSE;
	}

	TBOOL XURXUIBackButtonData::Load(TXUIResource& resource, uint8_t*& a_pData)
	{
		TBOOL res = XURXUIButtonData::Load(resource, a_pData);
		a_pData++;

		return res;
	}

}