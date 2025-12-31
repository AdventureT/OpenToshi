#pragma once
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/File/TTRB.h"

TOSHI_NAMESPACE_START

class T2Locale : public Toshi::TSingleton<T2Locale>
{
public:
	using Lang         = TINT32;
	using LocaleId     = TINT32;
	using LocaleString = const TWCHAR*;

	struct LocaleStrings
	{
		LocaleId      m_numstrings;
		LocaleString* Strings;
	};

public:
	T2Locale(const T2Locale&)  = delete;
	T2Locale(const T2Locale&&) = delete;

	// $deBlob: FUNCTION 00663000
	virtual ~T2Locale(){};
	virtual const TCHAR* GetLanguageFilename(Lang lang) = 0;

	void SetLanguage(Lang langid);

protected:
	T2Locale(TINT langCount, size_t bufferSize, void* buffer);

	LocaleString Get(LocaleId locid)
	{
		if (locid > -1 && locid < m_StringTable->m_numstrings)
		{
			return m_StringTable->Strings[locid];
		}

		return TNULL;
	}

	inline TINT GetNumStrings() const { return m_StringTable->m_numstrings; }

	// $deBlob: FUNCTION 00662fd0
	static void* TRBAllocator(TTRB::AllocType alloctype, size_t size, TINT16 unk, size_t unk2, void* userData) { return static_cast<T2Locale*>(userData)->TRBAlloc(size); }

	// $deBlob: FUNCTION 00662ff0
	static void TRBDeallocator(TTRB::AllocType alloctype, void* ptr, TINT16 unk, size_t unk2, void* userData)
	{
		// T2Locale doesn't have deallocator
	}

	void* TRBAlloc(size_t size)
	{
		void* ptr   = m_BufferPos;
		m_BufferPos = static_cast<TCHAR*>(ptr) + size;
		return ptr;
	}

protected:
	TTRB           m_Locale;      // 0x04
	void*          m_pBuffer;     // 0x18
	void*          m_BufferPos;   // 0x1C
	LocaleStrings* m_StringTable; // 0x20
	Lang           m_LangId;      // 0x24
	TBOOL          m_bOwnsBuffer; // 0x28
};

TOSHI_NAMESPACE_END
