#pragma once
#ifndef __TOSHI_TTRB_H__
#  define __TOSHI_TTRB_H__
#endif

#include "Toshi/Strings/TString8.h"
#include "Toshi/File/TFile.h"

#undef ERROR

TOSHI_NAMESPACE_START

class TTSFI;

class TTRB
{
public:
	typedef TUINT8 ERROR;
	enum ERROR_ : ERROR
	{
		ERROR_OK                   = 0,
		ERROR_NO_HEADER            = 1,
		ERROR_NOT_TRBF             = 2,
		ERROR_PARSE_ERROR          = 3,
		ERROR_WRONG_MAGIC          = 4,
		ERROR_FORM_MAGIC           = 5,
		ERROR_NO_FILE              = 6,
		ERROR_NOT_TRB              = 7,
		ERROR_NO_FILEINFO_ON_STACK = 8,
	};

	typedef TUINT8 AllocType;
	enum AllocType_ : AllocType
	{
		AllocType_Unk0 = 0,
		AllocType_Unk1 = 1,
		AllocType_Unk2 = 2,
	};

	using t_MemoryFuncAlloc   = void* (*)(AllocType alloctype, size_t size, TINT16 unk1, size_t unk2, void* userData);
	using t_MemoryFuncDealloc = void (*)(AllocType alloctype, void* ptr, TINT16 unk1, size_t unk2, void* userData);

	struct SecInfo
	{
		TINT16  m_Unused; // 0x0 (padding)
		TINT16  m_Unk1;   // 0x2
		TUINT32 m_Size;   // 0x4
		void*   m_Data;   // 0x8
		TUINT32 m_Unk2;   // 0xC
	};

	struct Header
	{
		TVersion m_ui32Version;
		TINT32   m_i32SectionCount;
	};

	struct RELCEntry
	{
		TINT16  HDRX1;  // HDRX of pointer
		TINT16  HDRX2;  // HDRX of data
		TUINT32 Offset; // Offset to pointer in HDRX1
	};

	struct SectionFORM
	{
		TUINT32 Unk;
	};

	struct TTRBSymbol
	{
		TUINT16 HDRX;
		TUINT16 NameOffset;
		TUINT16 Padding;
		TINT16  NameHash;
		TUINT32 DataOffset;
	};

	struct SYMB
	{
		TINT32 m_i32SymbCount;
	};

public:
	TTRB();
	~TTRB() { Close(); }

	// Creates TFile and reads file
	ERROR Load(const TCHAR* path);

	// Reads TRB from TFile
	ERROR Load(TFile* file);

	// Returns index of TTRBSymbol
	TINT GetSymbolIndex(const TCHAR* symbName);

	// Returns pointer to data if found and TNULL if not
	void* GetSymbolAddress(const TCHAR* symbName);
	void* GetSymbolAddress(TTRBSymbol& symb) { return static_cast<TCHAR*>(GetSection(symb.HDRX)) + symb.DataOffset; }

	// Destroys TRB file and the content
	void Close();

	static void SetDefaultMemoryFuncs(t_MemoryFuncAlloc allocator, t_MemoryFuncDealloc deallocator, void* allocatorUserData);

	template <typename T> T* CastSymbol(const TCHAR* symbName) { return TSTATICCAST(T, GetSymbolAddress(symbName)); }

	// $deBlob: FUNCTION 00686ee0
	static size_t GetSymbolTableSize(size_t count) { return sizeof(TTRBSymbol) * count + sizeof(SYMB); }

	// $deBlob: FUNCTION 00686ef0
	static size_t GetHeaderSize(size_t count) { return sizeof(SecInfo) * count + sizeof(Header); }

	SecInfo* GetSectionInfoList() const { return reinterpret_cast<SecInfo*>(m_pHeader + 1); }

	SecInfo* GetSectionInfo(TINT index) const { return GetSectionInfoList() + index; }

	TTRBSymbol* GetSymbol(TINT index) const
	{
		if (m_SYMB == TNULL)
		{
			return TNULL;
		}
		else if (index < m_SYMB->m_i32SymbCount)
		{
			return reinterpret_cast<TTRBSymbol*>(m_SYMB + 1) + index;
		}

		return TNULL;
	}

	TTRBSymbol* GetSymbol(TCHAR const* a_symbolName)
	{
		TINT index = GetSymbolIndex(a_symbolName);
		if (index == -1)
		{
			return TNULL;
		}

		return GetSymbol(index);
	}

	const TCHAR* GetSymbolName(TINT index) const
	{
		if (m_SYMB == TNULL)
		{
			return TNULL;
		}

		return GetSymbolName(reinterpret_cast<TTRBSymbol*>(m_SYMB + 1) + index);
	}

	const TCHAR* GetSymbolName(TTRBSymbol* symbol) const
	{
		if (m_SYMB == TNULL)
		{
			return TNULL;
		}

		return reinterpret_cast<const TCHAR*>(reinterpret_cast<uintptr_t>(m_SYMB) + GetSymbolTableSize(m_SYMB->m_i32SymbCount) + symbol->NameOffset);
	}

	// $deBlob: FUNCTION 00686b50
	SYMB* GetSymbolTable() const { return m_SYMB; }

	static TINT16 HashString(const TCHAR* str)
	{
		TINT16 hash = 0;
		TCHAR  character;

		while (*str != '\0')
		{
			character = *(str++);
			hash      = (TINT16)character + hash * 0x1f;
		}

		return hash;
	}

	void SetMemoryFunctions(t_MemoryFuncAlloc allocator, t_MemoryFuncDealloc deallocator, void* userdata)
	{
		// 00686e00
		m_MemAllocator   = allocator;
		m_MemDeallocator = deallocator;
		m_MemUserData    = userdata;
	}

private:
	// Parses sections
	TBOOL ProcessForm(TTSFI& ttsf);

	// Returns pointer to section by index
	inline void* GetSection(TINT index) { return GetSectionInfo(index)->m_Data; }

	void DeleteSymbolTable()
	{
		if (m_SYMB != TNULL)
		{
			m_MemDeallocator(AllocType_Unk2, m_SYMB, 0, 0, m_MemUserData);
			m_SYMB = TNULL;
		}
	}

private:
	static void* s_pDefAllocatorUserData;

private:
	Header*             m_pHeader;        // 0x00
	SYMB*               m_SYMB;           // 0x04
	t_MemoryFuncAlloc   m_MemAllocator;   // 0x08
	t_MemoryFuncDealloc m_MemDeallocator; // 0x0C
	void*               m_MemUserData;    // 0x10
};

TOSHI_NAMESPACE_END
