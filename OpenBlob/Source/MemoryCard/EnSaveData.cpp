#include "pch.h"
#include "EnSaveData.h"

TBOOL EnSaveData::Validate()
{
	TINT result = Toshi::TStringManager::String8Compare(m_savedata->header.m_id, "BB", 2);

	if (result == 0 && m_savedata->header.m_size == 3208)
	{
		TUINT32 prevCRC          = m_savedata->header.m_crc;
		m_savedata->header.m_crc = 0;
		m_savedata->header.m_crc = Toshi::TUtil::CRC32(m_buffer, m_size);

		if (m_savedata->header.m_crc != prevCRC) return TFALSE;

		TFLOAT fVal1 = 0.5f;
		TFLOAT fVal2 = 1.0f;
		//const TINT size = sizeof(SaveData);
	}
	return TFALSE;
}
