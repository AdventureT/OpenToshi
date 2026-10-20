#pragma once
#include "TXUIStringTable.h"
#include "Toshi/TString16.h"
#include "Toshi/TStack.h"
#include "Math/TVector2.h"
#include "Toshi/Endianness.h"

#ifdef FindResource
#  undef FindResource
#endif

TOSHI_NAMESPACE_START

class TTRB;
class TXUIScene;
class TXUIVisual;
class TXUICanvas;
class XURXUIObjectData;
class XURXUIElementData;
class XURXUISceneData;

// NOTE: .xur files are big endian (made by the Xbox 360 XUI tools). The PARSEWORD/PARSEDWORD macros
// read big endian values without advancing the pointer: here they're Endianness.h's *_BIG helpers.

struct XURSection
{
	TUINT32 m_uiSectionID; // 0x0
	TUINT32 m_uiOffset;    // 0x4
	TUINT32 m_uiSize;      // 0x8
};

struct XURHeader
{
	// NOTE: inlined into the TXUIResource constructor
	XURHeader()
	{
		m_apSections    = TNULL;
		m_uiFileID      = 0;
		m_uiVersion     = 0;
		m_uiXuiVersion  = 0;
		m_uiBinSize     = 0;
		m_uiFlags       = 0;
		m_uiNumSections = 0;
	}

	// NOTE: inlined into ~TXUIResource
	~XURHeader()
	{
		TFree( m_apSections );
	}

	XURSection* m_apSections;    // 0x00
	TUINT32     m_uiFileID;      // 0x04
	TUINT32     m_uiVersion;     // 0x08
	TUINT32     m_uiXuiVersion;  // 0x0C
	TUINT32     m_uiBinSize;     // 0x10
	TUINT16     m_uiFlags;       // 0x14
	TUINT16     m_uiNumSections; // 0x16
};

// A loaded .xur: the string, vector, quaternion (stored as Z rotations) and custom data tables plus the tree of
// XURXUI*Data objects (the DATA section) that scenes and visuals are created from.
class TXUIResource
{
public:
	// NOTE: the ids are read big endian
	static TCONSTEXPR_VAR TUINT32 IDXUR       = 0x58554942; // 'XUIB'
	static TCONSTEXPR_VAR TUINT32 IDXURSTRING = 0x5354524E; // 'STRN'
	static TCONSTEXPR_VAR TUINT32 IDXURVEC    = 0x56454354; // 'VECT'
	static TCONSTEXPR_VAR TUINT32 IDXURQUAT   = 0x51554154; // 'QUAT'
	static TCONSTEXPR_VAR TUINT32 IDXURCUST   = 0x43555354; // 'CUST'
	static TCONSTEXPR_VAR TUINT32 IDXURDATA   = 0x44415441; // 'DATA'

public:
	TXUIResource();
	~TXUIResource();

	// Loads a .xur (or the .xur symbol of a .trb) and optionally its string table (.xus/.trb)
	TBOOL Load( TBOOL a_bLoadStringTables, const TCHAR* a_szFileName, const TCHAR* a_szStringTableFileName, TBOOL a_bTRB, TTRB* a_pTRB );
	TBOOL Load( TUINT8* a_pData );

	// Size of the memory pile Load needs for a_pData (de Blob loads every .xur into its own pile)
	TUINT32 CalculatePileSize( TUINT8* a_pData );

	TXUIScene*  CreateScene( TUINT32 a_uiIndex );
	TXUIScene*  CreateScene( const TWCHAR* a_wszName );
	TXUIVisual* CreateVisual( const TWCHAR* a_wszName );
	// A canvas of the resource's root
	TXUICanvas* CreateCanvas();

	XURXUIObjectData* FindScene( TUINT32 a_uiIndex );
	XURXUIObjectData* FindScene( const TWCHAR* a_wszName );
	XURXUIObjectData* FindFirstScene();
	XURXUIObjectData* FindVisual( const TWCHAR* a_wszName );

	const TVector2* GetVector( TINT a_iIndex );
	TFLOAT          GetZRotation( TINT a_iIndex );

	void PushID( const TWCHAR* a_wszID );
	void PopID();

	const TWCHAR* LookupStringTable();

	// $deBlob: FUNCTION 006bc7f0
	const TWCHAR* GetString( TUINT16 a_uiIndex ) const { return m_asStringTable[ a_uiIndex ]; }

	// $deBlob: FUNCTION 006c99c0
	const TUINT8* GetCust( TUINT32 a_uiOffset ) const
	{
		TASSERT( a_uiOffset < m_uiCustDataSize );
		return m_pCustData + a_uiOffset;
	}

	XURXUIObjectData* GetRoot() const { return m_pRoot; }
	TUINT32           GetStringCount() const { return m_uiStringCount; }
	TMemoryHeap*      GetMemoryHeap() const { return m_pMemoryHeap; }
	void              SetMemoryHeap( TMemoryHeap* a_pHeap ) { m_pMemoryHeap = a_pHeap; }

private:
	TBOOL ReadHeader( TUINT8* a_pData );
	TBOOL ReadStringSection( TUINT8* a_pData, TUINT32 a_uiSize );
	TBOOL ReadVectorSection( TUINT8* a_pData, TUINT32 a_uiSize );
	TBOOL ReadQuatSection( TUINT8* a_pData, TUINT32 a_uiSize );
	TBOOL ReadCustSection( TUINT8* a_pData, TUINT32 a_uiSize );
	TBOOL ReadDataSection( TUINT8* a_pData, TUINT32 a_uiSize );

	void Destroy();

	static TUINT32 GetStringTableSize( TUINT8* a_pData, TUINT32 a_uiSize );
	static TUINT32 GetVectorTableSize( TUINT8* a_pData, TUINT32 a_uiSize );
	static TUINT32 GetQuatTableSize( TUINT8* a_pData, TUINT32 a_uiSize );
	static TUINT32 GetCustDataSize( TUINT8* a_pData, TUINT32 a_uiSize );
	TUINT32        GetDataSectionSize( TUINT8* a_pData, TUINT32 a_uiSize );

public:
	static TINT  s_iUIDCount;
	static TINT  s_iGenerateUIDs; // > 0 while creating a visual (asserted as "s_iGenerateUIDs >= 0")

private:
	XURHeader                 m_oHeader;            // 0x00
	TMemoryHeap*              m_pMemoryHeap;        // 0x18 the "xui pile" this resource was loaded into
	TWCHAR**                  m_asStringTable;      // 0x1C
	TVector2*                 m_avVectors;          // 0x20
	TFLOAT*                   m_aqZRotTable;        // 0x24
	TUINT8*                   m_pCustData;          // 0x28
	XURXUIObjectData*         m_pRoot;              // 0x2C
	TINT                      m_uiStringCount;      // 0x30
	TINT                      m_uiVectorCount;      // 0x34
	TINT                      m_uiQuatCount;        // 0x38
	TUINT32                   m_uiCustDataSize;     // 0x3C
	TXUIStringTable*          m_pStringTable;       // 0x40
	TString16                 m_ID;                 // 0x44
	TStack<const TWCHAR*, 32> m_oIDComparisonStack; // 0x50
};

XURXUIObjectData* CreateObjectData( TXUIResource& a_rResource, TUINT16 a_uiType );
XURXUIObjectData* CreateObjectData( TXUIResource& a_rResource, const TWCHAR* a_wszType );

// de Blob: sizes of the objects CreateObjectData creates, for the pile size
TUINT32 GetObjectDataSize( TXUIResource& a_rResource, TUINT16 a_uiType );
TUINT32 GetObjectDataSize( TXUIResource& a_rResource, const TWCHAR* a_wszType );

TOSHI_NAMESPACE_END
