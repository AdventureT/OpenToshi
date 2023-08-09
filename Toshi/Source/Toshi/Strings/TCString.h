#pragma once
#include "TString.h"
#include "TWString.h"
#include "Toshi/Typedefs.h"
#include "Toshi2/T2Allocator.h"

namespace Toshi
{
	class TCString
	{
	public:
		TCString(T2Allocator* allocator = TNULL);
		TCString(TCString&& src, T2Allocator* allocator = TNULL) noexcept;
		TCString(const TCString& src, T2Allocator* allocator = TNULL);
		TCString(const TWString& src, T2Allocator* allocator = TNULL);
		TCString(const char* src, T2Allocator* allocator = TNULL);
		TCString(uint32_t size, T2Allocator* allocator = TNULL);
		~TCString() { FreeBuffer(); }

		void Copy(const TCString& src, uint32_t size = -1) { Copy(src.m_pBuffer, size); }
		void Copy(const TWString& src, uint32_t size = -1);
		void Copy(const char* src, uint32_t size = -1);

		void FreeBuffer();

		// Returns TTRUE if allocated memory
		TBOOL AllocBuffer(uint32_t size, TBOOL freeMemory = TTRUE);

		static TCString Format(const char* a_pcFormat, ...);
		TCString& VFormat(const char* a_pcFormat, char* a_pcArgs);

		void UndoForceSetData() { Reset(); }
		void ForceSetData(char* a_cString, int a_ilength);

		int FindReverse(char a_findChar, int pos) const;

		void Truncate(uint32_t length);

		// Returns position of specified character
		int32_t Find(char character, uint32_t pos = 0) const;

		// Returns position of specified substring
		int32_t Find(const char* substr, uint32_t pos = 0) const;

		// Returns string starting from specified index
		const char* GetString(uint32_t index = 0) const;

		TCString& Concat(const TCString& str, uint32_t size = -1) { return Concat(str.m_pBuffer, size); };
		TCString& Concat(const TWString& src, uint32_t size = -1);
		TCString& Concat(const char* src, uint32_t size = -1);

		int Compare(const char*, int) const;
		int CompareNoCase(const char*, int) const;

		TCString Mid(uint32_t param_1, uint32_t param_2) const;
		TCString Right(int param_1) const { return Mid(param_1, Length() - param_1); }

		TCString& MakeUpper() { _strupr_s(m_pBuffer, m_iStrLen); return *this; }
		TCString& MakeLower() { _strlwr_s(m_pBuffer, m_iStrLen); return *this; }

		uint32_t Length() const { return m_iStrLen; }
		uint8_t ExcessLength() const { return m_iExcessLen; }

		TBOOL IsAllLowerCase() const;
		TBOOL IsAllUpperCase() const;
		TBOOL IsIndexValid(uint32_t index) const { return index >= 0 && index <= Length(); }
		TBOOL IsEmpty() const { return m_iStrLen == 0; }
		TBOOL IsUnicode() const { return TFALSE; } // Who would have known?

	public:
		TCString operator+(char const* cstr) const { TCString str = TCString(*this); return std::move(str.Concat(cstr)); }
		TCString* operator+= (char const* cstr) { Concat(cstr, -1); return this; }
		TCString* operator+= (TCString& str) { Concat(str, -1); return this; }
		char& operator[](int index) { return m_pBuffer[index]; }
		const char& operator[](int index) const { return *GetString(index); }
		operator const char* () const { return m_pBuffer; }
		TBOOL operator!() { return m_iStrLen == 0; }
		TBOOL operator==(const char* cstr) const { return Compare(cstr, -1) == 0; }
		TBOOL operator==(const TCString& str) const { return Compare(str.m_pBuffer, -1) == 0; }
		TBOOL operator!=(const char* cstr) const { return Compare(cstr, -1) != 0; }
		TBOOL operator!=(const TCString& str) const { return Compare(str.m_pBuffer, -1) != 0; }
		TBOOL operator<(const char* cstr) const { return Compare(cstr, -1) > -1; };
		TBOOL operator<(const TCString& str) const { return Compare(str.m_pBuffer, -1) > -1; };
		TBOOL operator<=(const char* cstr) const { return Compare(cstr, -1) > 0; };
		TBOOL operator<=(const TCString& str) const { return Compare(str.m_pBuffer, -1) > 0; };
		//TCString& operator=(const wchar_t* wcstr) { Copy(wcstr, -1); return *this; };
		TCString& operator=(const char* cstr) { Copy(cstr, -1); return *this; };
		TCString& operator=(const TCString& str) { Copy(str, -1); return *this; };

	private:

		typedef T2Allocator* (*func_DefaultAllocatorCB)();

		void Reset()
		{
			m_pBuffer = NullString;
			m_iStrLen = 0;
			m_iExcessLen = 0;
		}

		T2Allocator* GetAllocator()
		{
			return sm_pDefaultAllocatorCB();
		}

		static T2Allocator* GetDefaultAllocatorCB()
		{
			return &T2Allocator::s_GlobalAllocator;
		}

	private:

		static inline func_DefaultAllocatorCB sm_pDefaultAllocatorCB = &GetDefaultAllocatorCB;

		char* m_pBuffer = NullString; // 0x0
		uint8_t m_iExcessLen = 0;     // 0x4
		uint32_t m_iStrLen : 24 = 0;  // 0x5
		T2Allocator* m_pAllocator;    // 0x8
	};
}


