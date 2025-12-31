#pragma once
#include "Toshi2/T2SimpleArray.h"
#include "Toshi/Xui/TXUIResource.h"
#include "XUI.h"

TOSHI_NAMESPACE_BEGIN

struct XURXUIKeyframeData
{
	struct Value
	{
		union
		{
			TUINT8  Byte;
			TUINT16 Short;
			TUINT32 Int;
			TFLOAT  Float;
		};
	};

	Value*  m_pValues;
	TUINT16 m_uiKeyframeTime;
	TUINT8  m_Byte1;
	TUINT8  m_Byte2;
	TUINT8  m_Byte3;
	TUINT8  m_Byte4;

	XURXUIKeyframeData()
	{
		m_pValues        = TNULL;
		m_uiKeyframeTime = 0;
		m_Byte1          = 0;
		m_Byte2          = 0;
		m_Byte3          = 0;
		m_Byte4          = 50;
	}
};

class XURXUINamedFrameData
{
public:
	// Order not correct!
	TUINT32 m_Name;
	TUINT16 m_Time;
	TUINT16 m_Command;
	TUINT8  m_CommandParams;

	// $deBlob: FUNCTION 006e86c0
	XURXUINamedFrameData()
	{
		m_Name          = 0;
		m_Time          = 0;
		m_Command       = 0;
		m_CommandParams = 0;
	}
};

class XURXUITimelineData
{
public:
	class TimelineProp
	{
	public:
		union
		{
			struct
			{
				TUINT8 Byte1;
				TUINT8 Byte2;
				TUINT8 Byte3;
				TUINT8 Byte4;
			};

			TUINT32 Raw;
		} m_Unk1;

		TINT32 m_Unk2;

		TimelineProp()
		{
			m_Unk1.Raw = 0;
			m_Unk2     = -1;
		}
	};

public:
	XURXUITimelineData();
	~XURXUITimelineData();

	TBOOL Load(TXUIResource& a_rXur, TUINT8*& a_pData);

	void SetOwner(XURXUIObjectData* pOwner) { m_pOwnerData = pOwner; }

private:
	XURXUIObjectData*                 m_pOwnerData;
	XURXUIObjectData*                 m_pControlledChild; // 0x4 de blob
	T2SimpleArray<XURXUIKeyframeData> m_aKeyframeData;    // 0x8 de blob
	TimelineProp*                     m_aTimelineProps;
	XUIEPTUShort32                    m_uiNumTimelineProps;
	XUIEPTUShort32                    m_uiNumKeyframes;
	XUIEPTString                      m_iControlledChildStringID;
	XURXUIKeyframeData::Value*        m_aTimelineValues;
};

TOSHI_NAMESPACE_END
