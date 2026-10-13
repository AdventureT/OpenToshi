#include "ToshiPCH.h"
#include "TString16.h"

TOSHI_NAMESPACE_START

TString16::TString16()
{
	Reset();
	AllocBuffer(0, TTRUE);
}

TString16::TString16(TUINT32 size)
{
	Reset();
	AllocBuffer(size, TTRUE);
}

TString16& TString16::Concat(const TString16& str, TUINT32 size)
{
	TUINT32 len = str.Length();

	if ((len < size) || (size == -1))
	{
		size = len;
	}

	TUINT32 oldLength = m_iStrLen;
	TWCHAR* oldString = m_pBuffer;

	TBOOL allocated = AllocBuffer(m_iStrLen + size, TFALSE);

	if (allocated)
	{
		// since it has made a new buffer
		// it need to copy the old string
		// to the new buffer

		TStringManager::String16Copy(m_pBuffer, oldString, -1);
	}

	TStringManager::String16Copy(m_pBuffer + oldLength * 2, str.GetString(), size);
	m_pBuffer[m_iStrLen] = 0;

	if (allocated && m_iStrLen != 0)
	{
		TFree(oldString);
	}

	return *this;
}

TString16& TString16::Concat(const TWCHAR* str, TUINT32 size)
{
	TUINT32 len = TStringManager::String16Length(str);

	if ((len < size) || (size == -1))
	{
		size = len;
	}

	TUINT32 oldLength = m_iStrLen;
	TWCHAR* oldString = m_pBuffer;

	TBOOL allocated = AllocBuffer(m_iStrLen + size, TFALSE);

	if (allocated)
	{
		// since it has made a new buffer
		// it need to copy the old string
		// to the new buffer

		TStringManager::String16Copy(m_pBuffer, oldString, -1);
	}

	TStringManager::String16Copy(m_pBuffer + oldLength * 2, str, size);
	m_pBuffer[m_iStrLen] = 0;

	if (allocated && m_iStrLen != 0)
	{
		TFree(oldString);
	}

	return *this;
}

// $deBlob: FUNCTION 006f1650
TBOOL TString16::AllocBuffer(TUINT32 a_iLength, TBOOL freeMemory)
{
	TBOOL   hasChanged    = TFALSE;
	TUINT32 currentLength = Length();

	TASSERT(a_iLength >= 0, "Length can't be less than 0");
	TASSERT(a_iLength <= 0xFFFFFF, "Too big string");

	if (a_iLength != currentLength)
	{
		if (a_iLength == 0)
		{
			if (freeMemory) TFree(m_pBuffer);

			m_pBuffer    = m_aNull;
			m_iExcessLen = 0;

			hasChanged = TTRUE;
		}
		else
		{
			TINT newExcessLen = (currentLength - a_iLength * 2) + m_iExcessLen;

			if (newExcessLen < 0 || newExcessLen > 0xFF)
			{
				if (currentLength != 0 && freeMemory)
				{
					TFree(m_pBuffer);
				}

				m_pBuffer    = (TWCHAR*)TMalloc((a_iLength + 1) * 2);
				m_iExcessLen = 0;

				hasChanged = TTRUE;
			}
			else
			{
				m_iExcessLen = newExcessLen;
				hasChanged   = TFALSE;
			}
		}

		m_iStrLen = a_iLength;
	}

	if (freeMemory) m_pBuffer[0] = '\0';
	return hasChanged;
}

TString16 TString16::Mid(TUINT32 param_1, TUINT32 param_2) const
{
	if (param_2 < 0)
	{
		param_2 = 0;
	}
	else
	{
		if (Length() <= param_2)
		{
			return TString16();
		}
	}

	// Rewrite not correct
	TString16 str = TString16(Length() - param_2);
	TUtil::MemCopy(str.m_pBuffer, GetString(param_2), Length() - param_2);
	m_pBuffer[m_iStrLen - param_2] = 0;

	return str;
}

// $deBlob: FUNCTION 006f0330
void TString16::Copy(const TString16& src, TUINT32 size)
{
	TUINT32 srcLen = src.Length();
	TASSERT(srcLen <= 0xFFFFFF, "Too big string");

	if (srcLen < size || size == -1)
	{
		size = srcLen;
	}

	AllocBuffer(size, TTRUE);
	TUtil::MemCopy(m_pBuffer, src, size);
	m_pBuffer[size] = 0;
}

TOSHI_NAMESPACE_END
