#pragma once

TOSHI_NAMESPACE_START

class TTRB;

//-----------------------------------------------------------------------------
// A .xus string table (big endian, "XUIS"): pairs of (localised string, id) looked up by id (0x14 bytes)
//-----------------------------------------------------------------------------
class TXUIStringTable
{
public:
	static TCONSTEXPR_VAR TUINT32 IDXUS = 0x58554953; // 'XUIS' read big endian

	struct XUSHeader
	{
		TUINT32 m_uiFileID;     // 0x0
		TUINT8  m_uiVersion1;   // 0x4
		TUINT8  m_uiVersion2;   // 0x5
		TUINT32 m_uiSize;       // 0x8
		TUINT16 m_uiNumStrings; // 0xC
	};

	struct XUSString
	{
		TWCHAR* m_wszString; // 0x0
		TWCHAR* m_wszID;     // 0x4
	};

public:
	TXUIStringTable();
	~TXUIStringTable();

	TBOOL         Load( const TCHAR* a_szFileName, TBOOL a_bTRB, TTRB* a_pTRB );
	TBOOL         ReadHeader( TUINT8* a_pData );
	TBOOL         ReadStringTable( TUINT8* a_pData );
	const TWCHAR* Lookup( const TWCHAR* a_wszID );

	TUINT32 GetNumStrings() const { return m_oXUSHeader.m_uiNumStrings; }

private:
	XUSHeader  m_oXUSHeader;    // 0x00
	XUSString* m_aStringTable;  // 0x10
};

TOSHI_NAMESPACE_END
