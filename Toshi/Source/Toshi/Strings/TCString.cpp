#include "ToshiPCH.h"
#include "TCString.h"

#include "Toshi/Core/TSystem.h"

namespace Toshi
{
	TCString::TCString(T2Allocator* allocator)
	{
		Reset();
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		AllocBuffer(0, TTRUE);
	}

	TCString::TCString(TCString&& src, T2Allocator* allocator) noexcept
	{
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		TCString::m_iExcessLen = src.m_iExcessLen;
		TCString::m_iStrLen = src.m_iStrLen;
		TCString::m_pBuffer = src.m_pBuffer;
		src.m_iExcessLen = 0;
		src.m_iStrLen = 0;
		src.m_pBuffer = NullString;
	}

	TCString::TCString(const TCString& src, T2Allocator* allocator)
	{
		Reset();
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		Copy(src, -1);
	}

	TCString::TCString(uint32_t size, T2Allocator* allocator)
	{
		Reset();
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		AllocBuffer(size);
	}

	TCString::TCString(const TWString& src, T2Allocator* allocator)
	{
		Reset();
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		Copy(src);
	}

	TCString::TCString(const char* src, T2Allocator* allocator)
	{
		Reset();
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		Copy(src);
	}

	void TCString::Copy(const TWString& src, uint32_t size)
	{
		uint32_t srcLen = src.Length();
		TASSERT(srcLen <= 0xFFFFFF, "Too big string");

		if (srcLen < size || size == -1) { size = srcLen; }

		AllocBuffer(size, TTRUE);
		TStringManager::StringUnicodeToChar(m_pBuffer, src, size);
		m_pBuffer[size] = 0;
	}

	void TCString::Copy(const char* src, uint32_t size)
	{
		if (src != m_pBuffer)
		{
			size_t srcLen = src ? TStringManager::String8Length(src) : 0;
			TASSERT(srcLen <= 0xFFFFFF, "Too big string");

			if (srcLen < size || size == -1)
			{
				size = (uint32_t)srcLen;
			}

			AllocBuffer(size, TTRUE);
			TUtil::MemCopy(m_pBuffer, src, size);

			m_pBuffer[size] = 0;
		}
	}

	int32_t TCString::Find(char character, uint32_t pos) const
	{
		if (!IsIndexValid(pos)) return -1;

		const char* foundAt = strchr(&m_pBuffer[pos], character);
		if (foundAt == TNULL) return -1;

		return (int32_t)(foundAt - m_pBuffer);
	}

	int32_t TCString::Find(const char* substr, uint32_t pos) const
	{
		if (!IsIndexValid(pos)) return -1;

		const char* foundAt = strstr(GetString(pos), substr);
		if (foundAt == TNULL) return -1;

		return (int32_t)(foundAt - m_pBuffer);
	}

	TBOOL TCString::AllocBuffer(uint32_t a_iLength, TBOOL freeMemory)
	{
		TBOOL hasChanged = TFALSE;
		uint32_t currentLength = Length();

		TASSERT(a_iLength >= 0, "Length can't be less than 0");
		TASSERT(a_iLength <= 0xFFFFFF, "Too big string");

		if (a_iLength != currentLength)
		{
			if (a_iLength == 0)
			{
				if (freeMemory) m_pAllocator->Free(m_pBuffer);

				m_pBuffer = NullString;
				m_iExcessLen = 0;

				hasChanged = TTRUE;
			}
			else
			{
				int newExcessLen = (currentLength - a_iLength) + m_iExcessLen;

				if (newExcessLen < 0 || newExcessLen > 0xFF)
				{
					if (currentLength != 0 && freeMemory)
					{
						m_pAllocator->Free(m_pBuffer);
					}

					m_pBuffer = (char*)m_pAllocator->Malloc(a_iLength + 1);
					m_iExcessLen = 0;

					hasChanged = TTRUE;
				}
				else
				{
					m_iExcessLen = newExcessLen;
					hasChanged = TFALSE;
				}
			}

			m_iStrLen = a_iLength;
		}

		if (freeMemory) m_pBuffer[0] = '\0';
		return hasChanged;
	}

	TCString TCString::Format(const char* a_pcFormat, ...)
	{
		char buffer[0x400];
		TCString buffer2;
		va_list args;

		va_start(args, a_pcFormat);

		// TASSERT is only in T2String8
		int iResult = _vsnprintf(buffer, sizeof(buffer), a_pcFormat, args);
		TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem")
		buffer2.Copy(buffer, -1);
		
		return buffer2;
	}

	TCString& TCString::VFormat(const char* a_pcFormat, char* a_pcArgs)
	{
		char buffer[0x400];

		// TASSERT is only in T2String8
		int iResult = _vsnprintf(buffer, sizeof(buffer), a_pcFormat, a_pcArgs);
		TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem")
		Copy(buffer, -1);

		return *this;
	}

	void TCString::ForceSetData(char* a_cString, int a_iLength)
	{
		m_pBuffer = a_cString;

		if (a_iLength < 0)
		{
			m_iStrLen = TStringManager::String8Length(a_cString);
		}
		else
		{
			m_iStrLen = a_iLength;
		}

		m_iExcessLen = 0;
	}

	int TCString::FindReverse(char a_findChar, int pos) const
	{
		if (pos == -1)
		{
			pos = m_iStrLen;
		}
		else
		{
			if (!IsIndexValid(pos)) return -1;
		}


		for (; pos > -1; pos--)
		{
			if (a_findChar == m_pBuffer[pos])
			{
				return pos;
			}
		}

		return -1;
	}

	void TCString::Truncate(uint32_t length)
	{
		if (Length() < length)
		{
			length = Length();
		}

		char* oldBuffer = m_pBuffer;

		TBOOL allocated = AllocBuffer(length, TFALSE);
		if (allocated)
		{
			TStringManager::String8Copy(m_pBuffer, oldBuffer, length);
		}

		m_pBuffer[length] = 0;

		if (allocated && Length() != 0)
		{
			m_pAllocator->Free(oldBuffer);
		}
	}

	void TCString::FreeBuffer()
	{
		if (Length() != 0) m_pAllocator->Free(m_pBuffer);
		Reset();
	}

	const char* TCString::GetString(uint32_t a_iIndex) const
	{
		TASSERT(a_iIndex >= 0 && a_iIndex <= (TINT)m_iStrLen);
		if (IsIndexValid(a_iIndex)) { return &m_pBuffer[a_iIndex]; }
		return TNULL;
	}

	TCString& TCString::Concat(const char* str, uint32_t size)
	{
		uint32_t len = (uint32_t)TStringManager::String8Length(str);
		
		if ((len < size) || (size == -1))
		{
			size = len;
		}

		uint32_t oldLength = m_iStrLen;
		char* oldString = m_pBuffer;

		TBOOL allocated = AllocBuffer(m_iStrLen + size, TFALSE);

		if (allocated)
		{
			// since it has made a new buffer
			// it need to copy the old string
			// to the new buffer

			TStringManager::String8Copy(m_pBuffer, oldString, -1);
		}

		TStringManager::String8Copy(m_pBuffer + oldLength, str, size);
		m_pBuffer[m_iStrLen] = 0;

		if (allocated && oldLength != 0)
		{
			m_pAllocator->Free(oldString);
		}

		return *this;
	}

	int TCString::Compare(const char* a_pcString, int param_2) const
	{
		TASSERT(a_pcString != TNULL, "TCString::CompareNoCase - Passed string cannot be TNULL");
		TASSERT(IsIndexValid(0), "TCString::CompareNoCase - Index 0 is not valid");
		TASSERT(GetString() != TNULL, "TCString::CompareNoCase - String cannot be TNULL");

		if (param_2 != -1)
		{
			return strncmp(GetString(), a_pcString, param_2);
		}

		const char* str = GetString();
		char bVar1 = 0;
		char bVar4 = 0;
		while (TTRUE)
		{
			bVar1 = *str;
			bVar4 = bVar1 < *a_pcString;

			if (bVar1 != *a_pcString) break;
			if (bVar1 == 0) return 0;
			
			bVar1 = str[1];
			bVar4 = bVar1 < a_pcString[1];

			if (bVar1 != a_pcString[1]) break;
			if (bVar1 == 0) return 0;

			str += 2;
			a_pcString += 2;
		}
		return bVar4 | 1;
	}

	int TCString::CompareNoCase(const char* a_pcString, int param_2) const
	{
		TASSERT(a_pcString != TNULL, "TCString::CompareNoCase - Passed string cannot be TNULL");
		TASSERT(IsIndexValid(0), "TCString::CompareNoCase - Index 0 is not valid");
		TASSERT(GetString() != TNULL, "TCString::CompareNoCase - String cannot be TNULL");

		if (param_2 == -1)
		{
			return _stricmp(GetString(), a_pcString);
		}

		TASSERT(IsIndexValid(0), "TCString::CompareNoCase - Index 0 is not valid");
		
		return _strnicmp(GetString(), a_pcString, param_2);
	}

	TCString TCString::Mid(uint32_t param_1, uint32_t param_2) const
	{
		if (param_2 < 0)
		{
			param_2 = 0;
		}
		else if (Length() <= param_2)
		{
			return TCString();
		}

		// Rewrite not correct
		TCString str = TCString(Length() - param_2);
		TUtil::MemCopy(str.m_pBuffer, GetString(param_2), Length() - param_2);
		m_pBuffer[m_iStrLen - param_2] = 0;

		return str;
	}

	TCString& TCString::Concat(const TWString& str, uint32_t size)
	{
		uint32_t len = str.Length();

		if ((len < size) || (size == -1))
		{
			size = len;
		}

		uint32_t oldLength = m_iStrLen;
		char* oldString = m_pBuffer;
		
		TBOOL allocated = AllocBuffer(m_iStrLen + size, TFALSE);
		
		if (allocated)
		{
			// since it has made a new buffer
			// it need to copy the old string
			// to the new buffer

			TStringManager::String8Copy(m_pBuffer, oldString, -1);
		}

		TStringManager::StringUnicodeToChar(m_pBuffer + oldLength, str.GetString(), size);
		m_pBuffer[m_iStrLen] = 0;
		
		if (allocated && m_iStrLen != 0)
		{
			m_pAllocator->Free(oldString);
		}

		return *this;
	}

	TBOOL TCString::IsAllLowerCase() const
	{
		if (m_iStrLen != 0)
		{
			char* iter = m_pBuffer;
			char* endStr = m_pBuffer + m_iStrLen;

			do
			{
				char currentC = *iter++;
				if (currentC > '@' && currentC < '[') return TFALSE;

			} while (iter < endStr);
		}

		return TTRUE;
	}

	TBOOL TCString::IsAllUpperCase() const
	{
		if (m_iStrLen != 0)
		{
			char* iter = m_pBuffer;
			char* endStr = m_pBuffer + m_iStrLen;

			do
			{
				char currentC = *iter++;
				if (currentC > '`' && currentC < '{') return TFALSE;

			} while (iter < endStr);
		}

		return TTRUE;
	}
}