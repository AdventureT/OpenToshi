#pragma once

TOSHI_NAMESPACE_BEGIN

class TXUIStringTable
{
public:
	static const TUINT32 IDXUS = TFourCC("XUIS");

	~TXUIStringTable();

	struct Str
	{
		TINT    value;
		TWCHAR* string;
	};

	Toshi::TXUIStringTable::Str* Lookup(const TWCHAR* a_str);

	struct XUSHeader
	{
		TUINT32 m_uiFileID;
	};

	XUSHeader m_oXUSHeader;       // 0x0
	size_t    m_uiCountOfStrings; // 0xC
	Str**     m_aStringTable;     // 0x10
};

TOSHI_NAMESPACE_END
