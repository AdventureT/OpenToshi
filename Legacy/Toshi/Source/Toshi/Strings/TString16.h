#pragma once
#include "TString.h"

TOSHI_NAMESPACE_START

class TString16
{
public:
	inline TUINT32 Length() const { return m_iStrLen; }

public:
	TString16();
	TString16(TUINT32 size);

	// Returns TTRUE if allocated memory
	TBOOL AllocBuffer(TUINT32 a_iLength, TBOOL freeMemory = TTRUE);

	TString16& Concat(const TString16& str, TUINT32 size);
	TString16& Concat(const TWCHAR* str, TUINT32 size);

	void Copy(const TString16& src, TUINT32 size = -1);

	TString16 Mid(TUINT32 param_1, TUINT32 param_2) const;
	TString16 Left(TUINT32 param_1) { return Mid(param_1, 0); }

	inline TBOOL         IsIndexValid(TUINT32 index) const { return index <= m_iStrLen && index >= 0; }
	inline const TWCHAR* GetString(TUINT32 index = 0) const
	{
		if (!IsIndexValid(index))
		{
			return 0;
		}
		return m_pBuffer + index * 2;
	}
	inline     operator TWCHAR*() const { return m_pBuffer; }
	TString16& operator=(const TString16& str)
	{
		Copy(str, -1);
		return *this;
	};

private:
	void Reset()
	{
		m_pBuffer    = m_aNull;
		m_iStrLen    = 0;
		m_iExcessLen = 0;
	}

	static inline TWCHAR m_aNull[8]       = { 0, 0, 0, 0, 0, 0, 0, 0 };
	TWCHAR*              m_pBuffer        = (TWCHAR*)NullString;
	TUINT8               m_iExcessLen : 8 = 0;
	TUINT32              m_iStrLen : 24   = 0;
};

TOSHI_NAMESPACE_END
