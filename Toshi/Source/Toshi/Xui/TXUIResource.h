#pragma once
#include "TXUIStringTable.h"
#include "Toshi/File/TFile.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/Core/TStack.h"
#include "Toshi/Strings/TString16.h"
#include "Toshi2/T2String16.h"

TOSHI_NAMESPACE_BEGIN

class XURXUIObjectData;
class TXUIScene;

class TXUIResource
{
	static const TUINT32 IDXUR       = TFourCC("XUIB");
	static const TUINT32 IDXURSTRING = TFourCC("STRN");
	static const TUINT32 IDXURVEC    = TFourCC("VECT");
	static const TUINT32 IDXURQUAT   = TFourCC("QUAT");
	static const TUINT32 IDXURCUST   = TFourCC("CUST");
	static const TUINT32 IDXURDATA   = TFourCC("DATA");

	struct Section
	{
		TUINT32 m_uiSectionID; // 0x0
		TUINT32 m_uiOffset;    // 0x4
		TUINT32 m_uiSize;      // 0x8
	};

	struct XURHeader
	{
		XURHeader()
		{
			m_apSections    = TNULL;
			m_uiFileID      = 0;
			m_uiVersion     = 0;
			m_uiXuiVersion  = 0;
			m_usBinSize     = 0;
			m_uiFlags       = 0;
			m_usNumSections = 0;
		}

		Section* m_apSections;    // 0x00
		TUINT32  m_uiFileID;      // 0x04
		TUINT32  m_uiVersion;     // 0x08
		TUINT32  m_uiXuiVersion;  // 0x0C
		TUINT32  m_usBinSize;     // 0x10
		TUINT16  m_uiFlags;       // 0x12
		TUINT16  m_usNumSections; // 0x14
	};

public:
	TXUIResource()
	{
		m_pMemoryHeap   = TNULL;
		m_asStringTable = TNULL;
		m_avVectors     = TNULL;
		m_aqZRotTable   = TNULL;
		m_pCustData     = TNULL;
		m_pRoot         = TNULL;
		m_uiStringCount = 0;
		m_uiVectorCount = 0;
		m_uiQuatCount   = 0;
		m_pStringTable  = 0;
	}

	~TXUIResource();

	const TWCHAR* GetString(TUINT16 a_uiType) { return m_asStringTable[a_uiType]; }

	TFLOAT GetZRotation(TINT a_iIndex = 0)
	{
		TASSERT(a_iIndex >= -1);
		return (a_iIndex == -1) ? 0 : m_aqZRotTable[a_iIndex];
	}

	const TVector2* GetVector(TINT a_iIndex = 0)
	{
		TASSERT(a_iIndex >= -1);
		return (a_iIndex == -1) ? &TVector2::VEC_ZERO : &m_avVectors[a_iIndex];
	}

	const TUINT8* GetCust(TUINT32 a_uiOffset) const
	{
		TASSERT(a_uiOffset < m_uiCustDataSize);
		return m_pCustData + a_uiOffset;
	}

	TBOOL ReadHeader(TUINT8* buffer);

	// Toshi::TXUIResource::Load(const TCHAR*, const TCHAR*, TBOOL, Toshi::TTRB*)

	// loadStringTables = when TTRUE load .xus/.trb files (StringTables)
	// filename = f.e Data/XUI/%s.trb or Data/XUI/%s.xur
	// fileNameStringTable = f.e %s/StringTables/%s/%s.xus or %s/StringTables/%s/%s.trb
	// loadTrb = when TTRUE load .xur/.trb files (XUIB)
	// unk3 = probably a function
	void Load(TBOOL loadStringTables, const TCHAR* filenameXUIB, const TCHAR* fileNameStringTable, TBOOL loadTrb, void* unk3);

	TBOOL Load(TUINT8* buffer);
	TBOOL ReadDataSection(TUINT8* a_pData, TUINT32 size);
	TBOOL ReadStringSection(TUINT8* buffer, TUINT32 size);
	TBOOL ReadCustSection(TUINT8* buffer, TUINT32 size);
	TBOOL ReadQuatSection(TUINT8* buffer, TUINT32 size);
	TINT  GetStringTableSize(TUINT8* pPtr, TUINT32 size);

	void PushID(const TWCHAR* a_wsID);
	void PopID();

	TXUIStringTable& LookupStringTable();

	TXUIScene*        CreateScene(TUINT32 a_uiIndex);
	TXUIScene*        CreateScene(const TWCHAR* a_wcName);
	XURXUIObjectData* FindScene(TUINT32 a_uiIndex);
	XURXUIObjectData* FindScene(const TWCHAR* a_wcName);
	XURXUIObjectData* FindFirstScene(TUINT32 a_uiIndex);

	static XURXUIObjectData* CreateObjectData(TXUIResource& a_rResource, TUINT16 a_uiType);
	static XURXUIObjectData* CreateObjectData(TXUIResource& a_rResource, const TWCHAR* objectName);

private:
	void Destroy();

private:
	XURHeader                 m_oHeader;            // 0x00
	TMemoryHeap*              m_pMemoryHeap;        // 0x18
	TWCHAR**                  m_asStringTable;      // 0x1C
	TVector2*                 m_avVectors;          // 0x20
	TFLOAT*                   m_aqZRotTable;        // 0x24
	TUINT8*                   m_pCustData;          // 0x28
	XURXUIObjectData*         m_pRoot;              // 0x2C
	TUINT32                   m_uiStringCount;      // 0x30
	TUINT32                   m_uiVectorCount;      // 0x34
	TUINT32                   m_uiQuatCount;        // 0x38
	TUINT32                   m_uiCustDataSize;     // 0x3C
	TXUIStringTable*          m_pStringTable;       // 0x40
	TString16                 m_ID;                 // 0x44
	TStack<const TWCHAR*, 32> m_oIDComparisonStack; // 0x50
public:
	static inline TINT  s_iUIDCount     = 0;
	static inline TBOOL s_bGenerateUIDs = TFALSE;
};

TOSHI_NAMESPACE_END
