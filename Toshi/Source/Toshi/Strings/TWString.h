#pragma once
#include "TString.h"
#include "TCString.h"

namespace Toshi
{
	class TWString
	{
	public:
		inline uint32_t Length() const { return m_iStrLen; }
		
	public:
		TWString();
		TWString(uint32_t size);
		TWString(const TCString& a_cString);

		// Returns TTRUE if allocated memory
		TBOOL AllocBuffer(uint32_t a_iLength, TBOOL freeMemory = TTRUE);

		TWString& Concat(const TWString& str, uint32_t size);
		TWString& Concat(const wchar_t* str, uint32_t size);

		void Copy(const TWString& src, uint32_t size = -1);
		void Copy(const TCString& src, uint32_t size = -1);

		TWString Mid(uint32_t param_1, uint32_t param_2) const;
		TWString Left(uint32_t param_1) { return Mid(param_1, 0); }

		inline TBOOL IsIndexValid(uint32_t index) const { return index <= m_iStrLen && index >= 0; }
		inline const wchar_t* GetString(uint32_t index = 0) const { if (!IsIndexValid(index)) { return 0; } return m_pBuffer + index * 2; }
		inline operator wchar_t* () const { return m_pBuffer; }
		TWString& operator=(const TWString& str) { Copy(str, -1); return *this; };
	private:

		void Reset()
		{
			m_pBuffer = m_aNull;
			m_iStrLen = 0;
			m_iExcessLen = 0;
		}

		static inline wchar_t m_aNull[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		wchar_t* m_pBuffer = (wchar_t*)NullString;
		uint8_t m_iExcessLen : 8 = 0;
		uint32_t m_iStrLen : 24 = 0;
	};
}


