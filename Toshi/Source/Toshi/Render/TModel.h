#pragma once

#include "Toshi/File/TFile.h"

namespace Toshi
{
	class TModel
	{
		unsigned int m_Flags; // 0x4
		int m_iLODCount; // 0xC
		void* m_pSkeleton; // 0x14

		struct Header
		{
			uint32_t m_magic;
			uint32_t m_version;
			uint32_t m_remainingBytes;
		};

		struct Header2
		{

		};

		inline bool IsCreated() { return (m_Flags & 1) != 0; }

		inline int GetLODCount() { return m_iLODCount; }

	protected:
		bool LoadTMD(const char*); // JPOG only i think not sure though
	};
}


