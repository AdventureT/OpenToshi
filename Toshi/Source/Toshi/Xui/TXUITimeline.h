#pragma once
#include "Toshi2/T2SimpleArray.h"
#include "Toshi/Xui/TXUIResource.h"
#include "XUI.h"

TOSHI_NAMESPACE_BEGIN

struct XURXUIKeyframeData
{
	struct Value
	{
		union {
			uint8_t Byte;
			uint16_t Short;
			uint32_t Int;
			float Float;
		};
	};

	Value* m_pValues;
	uint16_t m_uiKeyframeTime;
	uint8_t m_Byte1;
	uint8_t m_Byte2;
	uint8_t m_Byte3;
	uint8_t m_Byte4;

	XURXUIKeyframeData()
	{
		m_pValues = TNULL;
		m_uiKeyframeTime = 0;
		m_Byte1 = 0;
		m_Byte2 = 0;
		m_Byte3 = 0;
		m_Byte4 = 50;
	}
};

class XURXUINamedFrameData
{
public:
	// Order not correct!
	uint32_t m_Name;
	uint16_t m_Time;
	uint16_t m_Command;
	uint8_t m_CommandParams;

	XURXUINamedFrameData()
	{
		m_Name = 0;
		m_Time = 0;
		m_Command = 0;
		m_CommandParams = 0;
	}
};

class XURXUITimelineData
{
public:
	class TimelineProp
	{
	public:
		union {
			struct {
				uint8_t Byte1;
				uint8_t Byte2;
				uint8_t Byte3;
				uint8_t Byte4;
			};

			uint32_t Raw;
		} m_Unk1;

		int32_t m_Unk2;

		TimelineProp()
		{
			m_Unk1.Raw = 0;
			m_Unk2 = -1;
		}
	};

public:
	XURXUITimelineData();
	~XURXUITimelineData();

	TBOOL Load(TXUIResource& a_rXur, uint8_t*& a_pData);

	void SetOwner(XURXUIObjectData* pOwner)
	{
		m_pOwnerData = pOwner;
	}

private:
	XURXUIObjectData* m_pOwnerData;
	XURXUIObjectData* m_pControlledChild; // 0x4 de blob
	T2SimpleArray<XURXUIKeyframeData> m_aKeyframeData; // 0x8 de blob
	TimelineProp* m_aTimelineProps;
	XUIEPTUShort32 m_uiNumTimelineProps;
	XUIEPTUShort32 m_uiNumKeyframes;
	XUIEPTString m_iControlledChildStringID;
	XURXUIKeyframeData::Value* m_aTimelineValues;
};

TOSHI_NAMESPACE_END