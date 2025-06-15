#include "ToshiPCH.h"
#include "TTRB.h"
#include "TTSF.h"

TOSHI_NAMESPACE_START

static constexpr TTRB::t_MemoryFuncAlloc s_cbDefAllocator = [](TTRB::AllocType alloctype, size_t size, TINT16 unk1, size_t unk2, void* userData) {
	return TMalloc(size);
};

static constexpr TTRB::t_MemoryFuncDealloc s_cbDefDeallocator = [](TTRB::AllocType alloctype, void* ptr, TINT16 unk1, size_t unk2, void* userData) {
	TFree(ptr);
};

void* TTRB::s_pDefAllocatorUserData = TNULL;

TTRB::TTRB()
{
	m_pHeader = TNULL;
	m_SYMB    = TNULL;

	SetMemoryFunctions(s_cbDefAllocator, s_cbDefDeallocator, s_pDefAllocatorUserData);
}

TTRB::ERROR TTRB::Load(const TCHAR* path)
{
	// FUN_006868e0
	TFile* pFile = TFile::Create(path);
	return Load(pFile);
}

TTRB::ERROR TTRB::Load(TFile* file)
{
	// FUN_00686920
	TTSFI ttsf;
	ERROR error = ttsf.Open(file);

	if (error == ERROR_OK)
	{
		if (ttsf.m_Magic == TFourCC("TRBF"))
		{
			if (ProcessForm(ttsf))
			{
				error = ERROR_OK;
			}
			else
			{
				error = ERROR_PARSE_ERROR;
			}
		}
		else
		{
			error = ERROR_NOT_TRBF;
		}
	}
	else
	{
		error = ERROR_NO_HEADER;
	}

	return error;
}

TBOOL TTRB::ProcessForm(TTSFI& ttsf)
{
	// FUN_00686f10
	static constexpr TUINT32 s_RELCEntriesLimit = 0x200;
	RELCEntry                relcEntries[s_RELCEntriesLimit];

	TINT32 fileSize = ttsf.m_CurrentHunk.Size - 4;
	TINT32 leftSize = fileSize;
	ttsf.PushForm();

	do
	{
		TUINT32 sectionName = 0;
		TUINT32 sectionSize = 0;

		while (TTRUE)
		{
			if (fileSize < 1)
			{
				ttsf.PopForm();
				// FUN_007ebfbf
				return TTRUE;
			}

			TUINT8 readResult = ttsf.ReadHunk();
			if (readResult != ERROR_OK) return TFALSE;

			sectionName = ttsf.m_CurrentHunk.Name;
			sectionSize = ttsf.m_CurrentHunk.Size;
			leftSize -= TMath::AlignNumUp(sectionSize) + sizeof(Toshi::TTSF::Hunk);

			if (TFourCC("HEAD") < sectionName) break;

			if (sectionName == TFourCC("HEAD"))
			{
				TINT numsections = (sectionSize - 4) / 0xC;

				m_pHeader                = static_cast<Header*>(m_MemAllocator(AllocType_Unk0, sizeof(Header) + sizeof(SecInfo) * numsections, 0, 0, m_MemUserData));
				m_pHeader->m_ui32Version = { 0 };

				ttsf.ReadRaw(&m_pHeader->m_i32SectionCount, sizeof(m_pHeader->m_i32SectionCount));

				TASSERT(m_pHeader->m_i32SectionCount == numsections, "HEAD section has wrong num of sections");

				for (TINT i = 0; i < m_pHeader->m_i32SectionCount; i++)
				{
					SecInfo* pSect = GetSectionInfo(i);

					ttsf.ReadRaw(pSect, 0xC);
					pSect->m_Data = m_MemAllocator(AllocType_Unk1, pSect->m_Size, 0, 0, m_MemUserData);
					pSect->m_Unk1 = (pSect->m_Unk1 == 0) ? 16 : pSect->m_Unk1;
					pSect->m_Unk2 = 0;
				}

				ttsf.SkipHunk();
			}
			else if (sectionName == TFourCC("SYMB"))
			{
				m_SYMB = static_cast<SYMB*>(m_MemAllocator(AllocType_Unk2, ttsf.m_CurrentHunk.Size, 0, 0, m_MemUserData));
				ttsf.ReadHunkData(m_SYMB);
			}
			else if (sectionName == TFourCC("SECC"))
			{
				Toshi::TCompress::ms_bIsBigEndian = (CURRENT_ENDIANESS == Endianess_Big);

				for (TINT i = 0; i < m_pHeader->m_i32SectionCount; i++)
				{
					auto* secInfo = GetSectionInfo(i);

					if (secInfo->m_Data != TNULL)
					{
						ttsf.ReadCompressed(secInfo->m_Data, secInfo->m_Size);
					}
				}

				ttsf.SkipHunk();
			}
			else if (sectionName == TFourCC("RELC"))
			{
				TUINT32 relocCount   = 0;
				TUINT32 curReloc     = 0;
				TUINT32 readedRelocs = 0;

				ttsf.ReadRaw(&relocCount, sizeof(relocCount));

				if (relocCount < 1)
				{
					relocCount = 0;
				}
				else
				{
					do
					{
						TUINT32 relocReadCount = relocCount - readedRelocs;

						// limit count of RELCs to read
						relocReadCount = TMath::Min(relocReadCount, s_RELCEntriesLimit);
						ttsf.ReadRaw(relcEntries, relocReadCount << 3);
						curReloc = readedRelocs + relocReadCount;

						auto& header = *m_pHeader;
						for (TUINT32 i = 0; i < relocReadCount; i++)
						{
							auto& relcEntry = relcEntries[i];
							auto  hdrx1     = GetSectionInfo(relcEntry.HDRX1);
							auto  hdrx2     = hdrx1;

							if (m_pHeader->m_ui32Version.Value >= TMAKEVERSION(1, 0))
							{
								hdrx2 = GetSectionInfo(relcEntry.HDRX2);
							}

							// this won't work in x64 because pointers in TRB files are always 4 bytes
							// need some workaround to support x64 again
							uintptr_t* ptr = reinterpret_cast<uintptr_t*>((uintptr_t)hdrx1->m_Data + relcEntry.Offset);
							*ptr += (uintptr_t)hdrx2->m_Data;
						}

						readedRelocs += relocReadCount;
					} while (curReloc < relocCount);
				}

				ttsf.SkipHunk();
			}
			else
			{
				// Unknown section
				ttsf.SkipHunk();

#ifdef TOSHI_DEBUG
				ttsf.LogUnknownSection();
#endif
			}

			fileSize = leftSize;
		}

		if (sectionName != TFourCC("FORM"))
		{
			if (sectionName == TFourCC("SECT"))
			{
				for (TINT i = 0; i < m_pHeader->m_i32SectionCount; i++)
				{
					SecInfo* pSect = GetSectionInfo(i);
					ttsf.ReadRaw(pSect->m_Data, pSect->m_Size);
				}

				ttsf.SkipHunk();
			}
			else if (sectionName == TFourCC("HDRX"))
			{
				m_pHeader = static_cast<Header*>(m_MemAllocator(AllocType_Unk0, sectionSize, 0, 0, m_MemUserData));
				ttsf.ReadHunkData(m_pHeader);

				for (TINT i = 0; i < m_pHeader->m_i32SectionCount; i++)
				{
					SecInfo* pSect = GetSectionInfo(i);
					pSect->m_Unk1  = (pSect->m_Unk1 == 0) ? 16 : pSect->m_Unk1;
					pSect->m_Data  = m_MemAllocator(AllocType_Unk1, pSect->m_Size, pSect->m_Unk1, pSect->m_Unk2, m_MemUserData);
				}
			}
			else
			{
				// Unknown section
				ttsf.SkipHunk();

#ifdef TOSHI_DEBUG
				ttsf.LogUnknownSection();
#endif
			}

			fileSize = leftSize;
		}
	} while (TTRUE);

	SectionFORM form;
	ttsf.ReadFORM(&form);

	TBOOL result = ProcessForm(ttsf);
	fileSize     = leftSize;

	return result;
}

void* TTRB::GetSymbolAddress(const TCHAR* symbName)
{
	// FUN_00686d30
	auto index = GetSymbolIndex(symbName);

	if (m_SYMB != TNULL && index != -1 && index < m_SYMB->m_i32SymbCount)
	{
		auto entry = GetSymbol(index);
		return static_cast<TCHAR*>(GetSection(entry->HDRX)) + entry->DataOffset;
	}

	return TNULL;
}

TINT TTRB::GetSymbolIndex(const TCHAR* symbName)
{
	// 00686c30
	if (m_SYMB != TNULL)
	{
		TINT16 hash = HashString(symbName);

		for (TINT i = 0; i < m_SYMB->m_i32SymbCount; i++)
		{
			auto symbol = GetSymbol(i);

			if (symbol->NameHash == hash)
			{
				if (Toshi::TStringManager::String8Compare(symbName, GetSymbolName(symbol), -1) == 0)
				{
					return i;
				}
			}
		}
	}

	return -1;
}

void TTRB::Close()
{
	// FUN_006869d0
	if (m_pHeader != TNULL)
	{
		for (TINT i = 0; i < m_pHeader->m_i32SectionCount; i++)
		{
			auto sec = GetSectionInfo(i);

			if (sec->m_Data != TNULL)
			{
				m_MemDeallocator(AllocType_Unk1, sec->m_Data, sec->m_Unk1, sec->m_Unk2, m_MemUserData);
			}
		}

		m_MemDeallocator(AllocType_Unk0, m_pHeader, 0, 0, m_MemUserData);
		m_pHeader = TNULL;
	}

	DeleteSymbolTable();
}

void TTRB::SetDefaultMemoryFuncs(t_MemoryFuncAlloc allocator, t_MemoryFuncDealloc deallocator, void* allocatorUserData)
{
	if (allocator != TNULL)
	{
		TASSERT(deallocator != TNULL);
	}
}

TOSHI_NAMESPACE_END
