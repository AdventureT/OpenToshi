#pragma once

#include "Toshi2/T2String8.h"
#include "Toshi/Utils/TUtil.h"

class EnSaveData
{
public:
	static constexpr TINT SAVEGAME_SLOT_COUNT = 3;

	// Size should be 0x318 or 792 in decimal
	struct SaveGameSlot
	{
		TBOOL  m_bUsedSlot; // should be 0x0 not sure
		TUINT8 padding[791];
	};

	struct SaveData
	{
		struct Header
		{
			TCHAR   m_id[4]; // 0x0 "BB"
			TUINT32 m_size;  // 0x4
			TINT    m_crc;   // 0x8
		} header;

		SaveGameSlot m_SaveGameSlots[SAVEGAME_SLOT_COUNT]; // 0xC
	};

	TINT      m_size2;    // 0x0
	TINT      m_size;     // 0x4
	TUINT8*   m_buffer;   // 0x8
	SaveData* m_savedata; // 0xC

	TBOOL Validate();
};
