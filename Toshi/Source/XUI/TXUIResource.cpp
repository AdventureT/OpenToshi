#include "ToshiPCH.h"
#include "TXUIResource.h"
#include "TXUI.h"
#include "TXUIStringTable.h"
#include "TXUICanvas.h"
#include "TXUIScene.h"
#include "TXUITabScene.h"
#include "TXUIFigure.h"
#include "TXUIVisual.h"
#include "TXUIImage.h"
#include "TXUIText.h"
#include "TXUIButton.h"
#include "TXUIGroup.h"
#include "TXUIImagePresenter.h"
#include "TXUITextPresenter.h"
#include "TXUINineGrid.h"
#include "TXUISound.h"
#include "TXUILabel.h"
#include "TXUIEditBox.h"
#include "TXUIMsgBoxButton.h"
#include "TXUINavButton.h"
#include "TXUIBackButton.h"
#include "TXUIRadioButton.h"
#include "TXUICheckBox.h"
#include "TXUIProgressBar.h"
#include "TXUISliderBar.h"
#include "TXUIList.h"
#include "TXUICommonList.h"
#include "TXUIRadioGroup.h"
#include "TXUIScrollEnd.h"
#include "TXUICaret.h"
#include "TXUIListItem.h"
#include "TXUIHTMLPresenter.h"
#include "TXUIHTMLElement.h"
#include "TXUIHTMLControl.h"
#include "TXUIVideo.h"

#include "File/TFile.h"
#include "File/TTRB.h"
#include "Math/TMatrix44.h"
#include "Math/TQuaternion.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TINT TXUIResource::s_iUIDCount     = 0;
TINT TXUIResource::s_iGenerateUIDs = 0;

// $deBlob: FUNCTION 006d78d0
TXUIResource::TXUIResource()
{
	m_pMemoryHeap    = TNULL;
	m_asStringTable  = TNULL;
	m_avVectors      = TNULL;
	m_aqZRotTable    = TNULL;
	m_pCustData      = TNULL;
	m_pRoot          = TNULL;
	m_uiStringCount  = 0;
	m_uiVectorCount  = 0;
	m_uiQuatCount    = 0;
	m_uiCustDataSize = 0;
	m_pStringTable   = TNULL;

	// NOTE: de Blob's TStack starts at 0 (IsEmpty is m_iTop == 0), the engine's starts at -1: push a dummy entry
	m_oIDComparisonStack.PushNull() = TNULL;
}

// $deBlob: FUNCTION 006d7940
// $deBlob: FUNCTION 006be650
TXUIResource::~TXUIResource()
{
	if ( m_pMemoryHeap )
	{
		// Everything was allocated in the pile
		m_oHeader.m_apSections = TNULL;
		TMemory_DestroyHeap( m_pMemoryHeap );
		return;
	}

	for ( TINT i = 0; i < m_uiStringCount; i++ )
		TFree( m_asStringTable[ i ] );

	TFree( m_asStringTable );
	TFree( m_avVectors );
	TFree( m_aqZRotTable );

	if ( m_pRoot )
		delete m_pRoot;

	if ( m_pStringTable )
		delete m_pStringTable;

	TFree( m_pCustData );
}

// Frees the tables and the data of a previous Load
void TXUIResource::Destroy()
{
	for ( TINT i = 0; i < m_uiStringCount; i++ )
		TFree( m_asStringTable[ i ] );

	TFree( m_asStringTable );
	TFree( m_avVectors );
	TFree( m_aqZRotTable );

	if ( m_pRoot )
		delete m_pRoot;
}

// $deBlob: FUNCTION 006d79e0
TUINT32 TXUIResource::CalculatePileSize( TUINT8* a_pData )
{
	ReadHeader( a_pData );

	TINT iSize = m_oHeader.m_uiNumSections * sizeof( XURSection ) + 4;

	for ( TINT i = 0; i < m_oHeader.m_uiNumSections; i++ )
	{
		XURSection& rSection = m_oHeader.m_apSections[ i ];
		TUINT8*     pSection = a_pData + rSection.m_uiOffset;

		switch ( rSection.m_uiSectionID )
		{
			case IDXURQUAT:
				iSize += GetQuatTableSize( pSection, rSection.m_uiSize );
				break;
			case IDXURCUST:
				ReadCustSection( pSection, rSection.m_uiSize );
				iSize += GetCustDataSize( pSection, rSection.m_uiSize );
				break;
			case IDXURDATA:
				iSize += GetDataSectionSize( pSection, rSection.m_uiSize );
				break;
			case IDXURSTRING:
				ReadStringSection( pSection, rSection.m_uiSize );
				iSize += GetStringTableSize( pSection, rSection.m_uiSize );
				break;
			case IDXURVEC:
				iSize += GetVectorTableSize( pSection, rSection.m_uiSize );
				break;
		}
	}

	// The string and custom tables were only needed to size the DATA section
	for ( TINT i = 0; i < m_uiStringCount; i++ )
		operator delete( m_asStringTable[ i ] );

	operator delete( m_asStringTable );
	m_asStringTable  = TNULL;
	m_uiStringCount  = 0;
	m_uiCustDataSize = 0;
	operator delete( m_pCustData );
	m_pCustData = TNULL;
	operator delete( m_oHeader.m_apSections );
	m_oHeader.m_apSections = TNULL;

	return ( iSize + 15 ) & ~15;
}

// $deBlob: FUNCTION 006d7b50
TBOOL TXUIResource::Load( TUINT8* a_pData )
{
	Destroy();

	TBOOL bResult = ReadHeader( a_pData );

	if ( !bResult )
		return bResult;

	for ( TINT i = 0; i < m_oHeader.m_uiNumSections; i++ )
	{
		XURSection& rSection = m_oHeader.m_apSections[ i ];
		TUINT8*     pSection = a_pData + rSection.m_uiOffset;

		switch ( rSection.m_uiSectionID )
		{
			case IDXURQUAT:
				if ( !ReadQuatSection( pSection, rSection.m_uiSize ) ) return TFALSE;
				break;
			case IDXURCUST:
				ReadCustSection( pSection, rSection.m_uiSize );
				break;
			case IDXURDATA:
				if ( !ReadDataSection( pSection, rSection.m_uiSize ) ) return TFALSE;
				break;
			case IDXURSTRING:
				if ( !ReadStringSection( pSection, rSection.m_uiSize ) ) return TFALSE;
				break;
			case IDXURVEC:
				if ( !ReadVectorSection( pSection, rSection.m_uiSize ) ) return TFALSE;
				break;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006d7c80
TBOOL TXUIResource::Load( TBOOL a_bLoadStringTables, const TCHAR* a_szFileName, const TCHAR* a_szStringTableFileName, TBOOL a_bTRB, TTRB* a_pTRB )
{
	Destroy();

	if ( a_bLoadStringTables && a_szStringTableFileName != TNULL )
	{
		m_pStringTable = new ( TXUI::ms_pXUIMemoryBlock ) TXUIStringTable();

		if ( !m_pStringTable->Load( a_szStringTableFileName, a_bTRB, a_pTRB ) )
		{
			if ( m_pStringTable )
				delete m_pStringTable;

			m_pStringTable = TNULL;
		}
	}

	if ( TStringManager::String8FindString( a_szFileName, a_bTRB ? ".trb" : ".xur" ) == TNULL )
		return TFALSE;

	TTRB    oTRB;
	TUINT8* pData = TNULL;

	if ( a_bTRB == TFALSE )
	{
		TFile* pFile = TFileManager::GetSingleton()->CreateFile( a_szFileName, TFILEMODE_READ );

		if ( pFile == TNULL )
		{
			oTRB.Close();
			return TFALSE;
		}

		TUINT32 uiSize = pFile->GetSize();
		pData          = TSTATICCAST( TUINT8, TMalloc( uiSize, TXUI::ms_pXUIMemoryBlock ) );
		pFile->Read( pData, uiSize );
		pFile->Destroy();
	}
	else
	{
		// The .xur is the symbol "<name>_<name>_xur" of a_pTRB or "<name>_xur" of a_szFileName's .trb
		TINT iNameStart = 0;
		TINT iLength    = TStringManager::String8Length( a_szFileName );

		for ( TINT i = iLength - 1; i >= 0; i-- )
		{
			if ( a_szFileName[ i ] == '/' || a_szFileName[ i ] == '\\' )
			{
				iNameStart = i + 1;
				break;
			}
		}

		TINT iFileNameLen = iLength - 4 - iNameStart;
		TASSERT( ( ( iFileNameLen << 1 ) + 5 ) < 128 );

		TCHAR szSymbol[ 128 ];
		TStringManager::String8Copy( szSymbol, a_szFileName + iNameStart, iFileNameLen );
		szSymbol[ iFileNameLen ]     = '_';
		szSymbol[ iFileNameLen + 1 ] = '\0';
		TStringManager::String8Copy( szSymbol + iFileNameLen + 1, a_szFileName + iNameStart, iFileNameLen );
		TStringManager::String8Copy( szSymbol + iFileNameLen * 2 + 1, "_xur", -1 );
		TStringManager::String8ToLowerCase( szSymbol );

		if ( a_pTRB != TNULL )
			pData = TSTATICCAST( TUINT8, a_pTRB->GetSymbolAddress( szSymbol ) );

		if ( pData == TNULL )
		{
			TStringManager::String8Copy( szSymbol, szSymbol + iFileNameLen + 1, iFileNameLen + 4 );
			szSymbol[ iFileNameLen + 4 ] = '\0';

			if ( oTRB.Load( a_szFileName ) != TTRB::ERROR_OK || ( pData = TSTATICCAST( TUINT8, oTRB.GetSymbolAddress( szSymbol ) ) ) == TNULL )
			{
				oTRB.Close();
				return TFALSE;
			}
		}
	}

	if ( ReadHeader( pData ) )
	{
		for ( TINT i = 0; i < m_oHeader.m_uiNumSections; i++ )
		{
			XURSection& rSection = m_oHeader.m_apSections[ i ];
			TUINT8*     pSection = pData + rSection.m_uiOffset;
			TBOOL       bRead    = TTRUE;

			switch ( rSection.m_uiSectionID )
			{
				case IDXURQUAT:
					bRead = ReadQuatSection( pSection, rSection.m_uiSize );
					break;
				case IDXURCUST:
					ReadCustSection( pSection, rSection.m_uiSize );
					break;
				case IDXURDATA:
					bRead = ReadDataSection( pSection, rSection.m_uiSize );
					break;
				case IDXURSTRING:
					bRead = ReadStringSection( pSection, rSection.m_uiSize );
					break;
				case IDXURVEC:
					bRead = ReadVectorSection( pSection, rSection.m_uiSize );
					break;
			}

			if ( bRead == TFALSE )
			{
				if ( a_bTRB == TFALSE )
					TFree( pData );

				oTRB.Close();
				return TFALSE;
			}
		}

		if ( a_bTRB == TFALSE )
			TFree( pData );

		oTRB.Close();
		return TTRUE;
	}

	oTRB.Close();
	return TFALSE;
}

// $deBlob: FUNCTION 006d8130
TXUIScene* TXUIResource::CreateScene( const TWCHAR* a_wszName )
{
	XURXUISceneData* pSceneData = TSTATICCAST( XURXUISceneData, FindScene( a_wszName ) );

	if ( pSceneData == TNULL )
		return TNULL;

	TXUIScene* pScene = TNULL;

	if ( pSceneData->m_uiClassOverride == 0 )
	{
		pScene = new ( TXUI::ms_pXUIMemoryBlock ) TXUIScene();
	}
	else
	{
		const TWCHAR* wszClassName = m_asStringTable[ pSceneData->m_uiClassOverride ];
		TINT          iLen         = TStringManager::String16Length( wszClassName );
		TASSERT( iLen < 128 );

		TCHAR szClassName[ 128 ];
		TStringManager::StringUnicodeToChar( szClassName, wszClassName, iLen );

		const TClass* pClass = TClass::Find( szClassName, &TGetClass( TXUIControl ) );

		if ( pClass != TNULL )
		{
			TMemoryHeap* pOldHeap = TMemory_GetGlobalHeap();
			TMemory_SetGlobalHeap( TXUI::ms_pXUIMemoryBlock );
			pScene = TXUIScene::CastFromTObject( pClass->CreateObject() );
			TMemory_SetGlobalHeap( pOldHeap );
		}
		else
		{
			pScene = new ( TXUI::ms_pXUIMemoryBlock ) TXUIScene();
		}
	}

	s_iUIDCount     = 0;
	s_iGenerateUIDs = 0;

	// NOTE: de Blob's IsEmpty (see the constructor)
	TASSERT( TTRUE == ( m_oIDComparisonStack.Count() == 1 ) );

	pScene->Create( *this, pSceneData, TTRUE );

	s_iUIDCount     = 0;
	s_iGenerateUIDs = 0;

	pScene->OnCreated();

	return pScene;
}

// $deBlob: FUNCTION 006d8300
TXUIScene* TXUIResource::CreateScene( TUINT32 a_uiIndex )
{
	XURXUIElementData* pSceneData = TNULL;

	if ( a_uiIndex < m_pRoot->m_uiNumChildren && m_pRoot->m_apChildren[ a_uiIndex ]->m_pClass->IsA( &TGetClass( TXUIScene ) ) )
		pSceneData = m_pRoot->m_apChildren[ a_uiIndex ];

	if ( pSceneData == TNULL )
	{
		pSceneData = TSTATICCAST( XURXUIElementData, FindFirstScene() );

		if ( pSceneData == TNULL )
			return TNULL;
	}

	return CreateScene( m_asStringTable[ pSceneData->m_uiId ] );
}

// $deBlob: FUNCTION 006d80d0
TXUICanvas* TXUIResource::CreateCanvas()
{
	if ( m_pRoot == TNULL )
		return TNULL;

	TXUICanvas* pCanvas = new ( TXUI::ms_pXUIMemoryBlock ) TXUICanvas();
	pCanvas->Create( *this, TSTATICCAST( XURXUIElementData, m_pRoot ), TFALSE );
	return pCanvas;
}

// $deBlob: FUNCTION 006d8370
TXUIVisual* TXUIResource::CreateVisual( const TWCHAR* a_wszName )
{
	XURXUIElementData* pVisualData = TSTATICCAST( XURXUIElementData, FindVisual( a_wszName ) );

	if ( pVisualData != TNULL )
	{
		TXUIVisual* pVisual = new ( TXUI::ms_pXUIMemoryBlock ) TXUIVisual();

		s_iGenerateUIDs++;
		pVisual->Create( *this, pVisualData, TTRUE );
		s_iGenerateUIDs--;
		TASSERT( s_iGenerateUIDs >= 0 );

		return pVisual;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006d8410
XURXUIObjectData* TXUIResource::FindScene( const TWCHAR* a_wszName )
{
	for ( TUINT32 i = 0; i < m_pRoot->m_uiNumChildren; i++ )
	{
		XURXUIElementData* pChild = m_pRoot->m_apChildren[ i ];

		if ( pChild->m_pClass == &TGetClass( TXUIScene ) && TStringManager::String16Compare( m_asStringTable[ pChild->m_uiId ], a_wszName, -1 ) == 0 )
			return pChild;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006d8470
XURXUIObjectData* TXUIResource::FindScene( TUINT32 a_uiIndex )
{
	if ( a_uiIndex >= m_pRoot->m_uiNumChildren )
		return TNULL;

	if ( m_pRoot->m_apChildren[ a_uiIndex ]->m_pClass->IsA( &TGetClass( TXUIScene ) ) )
		return m_pRoot->m_apChildren[ a_uiIndex ];

	return TNULL;
}

// $deBlob: FUNCTION 006d84d0
XURXUIObjectData* TXUIResource::FindFirstScene()
{
	for ( TINT i = 0; i < m_pRoot->m_uiNumChildren; i++ )
	{
		if ( m_pRoot->m_apChildren[ i ]->m_pClass->IsA( &TGetClass( TXUIScene ) ) )
			return m_pRoot->m_apChildren[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 006d8520
XURXUIObjectData* TXUIResource::FindVisual( const TWCHAR* a_wszName )
{
	for ( TUINT32 i = 0; i < m_pRoot->m_uiNumChildren; i++ )
	{
		XURXUIElementData* pChild = m_pRoot->m_apChildren[ i ];

		if ( pChild->m_pClass == &TGetClass( TXUIVisual ) && TStringManager::String16Compare( m_asStringTable[ pChild->m_uiId ], a_wszName, -1 ) == 0 )
			return pChild;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006d8580
const TVector2* TXUIResource::GetVector( TINT a_iIndex )
{
	TASSERT( a_iIndex >= -1 );
	return ( a_iIndex != -1 ) ? &m_avVectors[ a_iIndex ] : &TVector2::VEC_ZERO;
}

// $deBlob: FUNCTION 006d85e0
TFLOAT TXUIResource::GetZRotation( TINT a_iIndex )
{
	TASSERT( a_iIndex >= -1 );
	return ( a_iIndex != -1 ) ? m_aqZRotTable[ a_iIndex ] : 0.0f;
}

// $deBlob: FUNCTION 006d8640
// $deBlob: FUNCTION 006da030
void TXUIResource::PushID( const TWCHAR* a_wszID )
{
	m_oIDComparisonStack.Push( a_wszID );

	if ( m_ID.Length() != 0 )
		m_ID.Concat( L".", -1 );

	m_ID.Concat( a_wszID, -1 );
}

// $deBlob: FUNCTION 006d8690
// $deBlob: FUNCTION 006d9fc0
void TXUIResource::PopID()
{
	const TWCHAR* wszID = m_oIDComparisonStack.Top();
	m_oIDComparisonStack.Pop();

	TINT iLength = m_ID.Length() - TStringManager::String16Length( wszID ) - 1;
	m_ID         = m_ID.Mid( 0, ( iLength < 1 ) ? 0 : iLength );
}

// $deBlob: FUNCTION 006d8720
const TWCHAR* TXUIResource::LookupStringTable()
{
	if ( m_pStringTable != TNULL )
		return m_pStringTable->Lookup( m_ID.GetString( 0 ) );

	return TNULL;
}

// $deBlob: FUNCTION 006d8750
TBOOL TXUIResource::ReadHeader( TUINT8* a_pData )
{
	m_oHeader.m_uiFileID = PARSEDWORD_BIG( a_pData );
	TASSERT( m_oHeader.m_uiFileID == IDXUR );

	m_oHeader.m_uiVersion     = PARSEDWORD_BIG( a_pData + 4 );
	m_oHeader.m_uiFlags       = PARSEWORD_BIG( a_pData + 8 );
	m_oHeader.m_uiXuiVersion  = PARSEDWORD_BIG( a_pData + 10 );
	m_oHeader.m_uiBinSize     = PARSEDWORD_BIG( a_pData + 14 );
	m_oHeader.m_uiNumSections = PARSEWORD_BIG( a_pData + 18 );
	TASSERT( m_oHeader.m_uiNumSections > 0 );

	m_oHeader.m_apSections = TSTATICCAST( XURSection, TMalloc( m_oHeader.m_uiNumSections * sizeof( XURSection ), TXUI::ms_pXUIMemoryBlock ) );

	// Some files have 0x28 bytes more in the header
	TUINT8* pSections = a_pData + 0x14;
	TUINT32 uiFirstID = PARSEDWORD_BIG( pSections );

	if ( uiFirstID != IDXURSTRING && uiFirstID != IDXURVEC && uiFirstID != IDXURQUAT && uiFirstID != IDXURCUST && uiFirstID != IDXURDATA )
		pSections = a_pData + 0x3C;

	for ( TINT i = 0; i < m_oHeader.m_uiNumSections; i++ )
	{
		m_oHeader.m_apSections[ i ].m_uiSectionID = PARSEDWORD_BIG( pSections );
		TASSERT( m_oHeader.m_apSections[ i ].m_uiSectionID == IDXURSTRING || m_oHeader.m_apSections[ i ].m_uiSectionID == IDXURVEC || m_oHeader.m_apSections[ i ].m_uiSectionID == IDXURQUAT || m_oHeader.m_apSections[ i ].m_uiSectionID == IDXURCUST || m_oHeader.m_apSections[ i ].m_uiSectionID == IDXURDATA );

		m_oHeader.m_apSections[ i ].m_uiOffset = PARSEDWORD_BIG( pSections + 4 );
		m_oHeader.m_apSections[ i ].m_uiSize   = PARSEDWORD_BIG( pSections + 8 );
		pSections += 12;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006d89f0
TBOOL TXUIResource::ReadStringSection( TUINT8* a_pData, TUINT32 a_uiSize )
{
	TASSERT( TNULL == m_asStringTable );

	TUINT8* pEnd    = a_pData + a_uiSize;
	m_uiStringCount = 1;

	for ( TUINT8* pPtr = a_pData; pPtr < pEnd; pPtr += PARSEWORD_BIG( pPtr ) * 2 + 2 )
		m_uiStringCount++;

	m_asStringTable      = TSTATICCAST( TWCHAR*, TMalloc( m_uiStringCount * sizeof( TWCHAR* ), TXUI::ms_pXUIMemoryBlock ) );
	m_asStringTable[ 0 ] = TSTATICCAST( TWCHAR, TMalloc( sizeof( TWCHAR ), TXUI::ms_pXUIMemoryBlock ) );
	TStringManager::String16Copy( m_asStringTable[ 0 ], L"", 1 );

	TUINT8* pPtr = a_pData;

	for ( TINT i = 1; i < m_uiStringCount; i++ )
	{
		TASSERT( pPtr < pEnd );

		TUINT32 uiLength     = PARSEWORD_BIG( pPtr );
		m_asStringTable[ i ] = TSTATICCAST( TWCHAR, TMalloc( uiLength * sizeof( TWCHAR ) + sizeof( TWCHAR ), TXUI::ms_pXUIMemoryBlock ) );
		TStringManager::String16Copy( m_asStringTable[ i ], TREINTERPRETCAST( TWCHAR*, pPtr + 2 ), uiLength );

		// The characters are big endian
		for ( TUINT32 k = 0; k < uiLength; k++ )
			m_asStringTable[ i ][ k ] = PARSEWORD_BIG( TREINTERPRETCAST( TUINT8*, &m_asStringTable[ i ][ k ] ) );

		pPtr += ( uiLength + 1 ) * sizeof( TWCHAR );
		m_asStringTable[ i ][ uiLength ] = L'\0';
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006d8bb0
TBOOL TXUIResource::ReadVectorSection( TUINT8* a_pData, TUINT32 a_uiSize )
{
	// The vectors are stored as 3 floats, only X and Y are kept
	m_uiVectorCount = a_uiSize / 12;
	m_avVectors     = TSTATICCAST( TVector2, TMalloc( m_uiVectorCount * sizeof( TVector2 ), TXUI::ms_pXUIMemoryBlock ) );

	for ( TINT i = 0; i < m_uiVectorCount; i++ )
	{
		m_avVectors[ i ].x = PARSEFLOAT_BIG( a_pData );
		m_avVectors[ i ].y = PARSEFLOAT_BIG( a_pData + 4 );
		a_pData += 12;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006d8c70
TBOOL TXUIResource::ReadQuatSection( TUINT8* a_pData, TUINT32 a_uiSize )
{
	// The quaternions are only kept as their rotation around Z
	m_uiQuatCount = a_uiSize >> 4;
	m_aqZRotTable = TSTATICCAST( TFLOAT, TMalloc( m_uiQuatCount * sizeof( TFLOAT ), TXUI::ms_pXUIMemoryBlock ) );

	for ( TINT i = 0; i < m_uiQuatCount; i++ )
	{
		TQuaternion quat;
		quat.x = PARSEFLOAT_BIG( a_pData );
		quat.y = PARSEFLOAT_BIG( a_pData + 4 );
		quat.z = PARSEFLOAT_BIG( a_pData + 8 );
		quat.w = PARSEFLOAT_BIG( a_pData + 12 );

		TMatrix44 matrix;
		matrix.SetFromQuaternion( quat );

		TVector3 euler;
		matrix.GetEulerXYZ( euler );

		m_aqZRotTable[ i ] = -euler.z;
		TASSERT( m_aqZRotTable[ i ] >= -TMath::PI && m_aqZRotTable[ i ] <= TMath::PI );

		a_pData += 16;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006d8de0
TBOOL TXUIResource::ReadCustSection( TUINT8* a_pData, TUINT32 a_uiSize )
{
	m_pCustData = TSTATICCAST( TUINT8, TMalloc( a_uiSize, TXUI::ms_pXUIMemoryBlock ) );
	TUtil::MemCopy( m_pCustData, a_pData, a_uiSize );
	m_uiCustDataSize = a_uiSize;

	return TTRUE;
}

// $deBlob: FUNCTION 006d8e10
TBOOL TXUIResource::ReadDataSection( TUINT8* a_pData, TUINT32 a_uiSize )
{
	TUINT8* pValidateEnd = a_pData + a_uiSize;

	TUINT16 uiType = PARSEWORD_BIG( a_pData );
	a_pData += 2;
	TASSERT( 0 == TStringManager::String16Compare( GetString( uiType ), L"XuiCanvas", -1 ) );

	m_pRoot = ( uiType == 0 ) ? TNULL : CreateObjectData( *this, m_asStringTable[ uiType ] );

	TUINT8 uiFlags = *a_pData;
	a_pData += 1;

	m_pRoot->Load( *this, a_pData );

	if ( uiFlags & XURXUIElementData::ChildFlags_HasChildren )
		m_pRoot->LoadChildren( *this, a_pData );

	if ( uiFlags & XURXUIElementData::ChildFlags_HasNamedFrames )
	{
		// NOTE: the result of LoadNamedFrames isn't checked
		m_pRoot->LoadNamedFrames( *this, a_pData );

		if ( uiFlags & XURXUIElementData::ChildFlags_HasChildren )
			m_pRoot->LoadTimelines( *this, a_pData );
	}

	TASSERT( pValidateEnd == a_pData );

	return TTRUE;
}

// $deBlob: FUNCTION 006d8f30
TUINT32 TXUIResource::GetStringTableSize( TUINT8* a_pData, TUINT32 a_uiSize )
{
	TUINT8* pEnd         = a_pData + a_uiSize;
	TINT    iStringCount = 1;

	for ( TUINT8* pPtr = a_pData; pPtr < pEnd; pPtr += PARSEWORD_BIG( pPtr ) * 2 + 2 )
		iStringCount++;

	TUINT32 uiSize = iStringCount * 4 + 8;
	TUINT8* pPtr   = a_pData;

	for ( TINT i = 1; i < iStringCount; i++ )
	{
		TASSERT( pPtr < pEnd );

		TUINT32 uiStringSize = PARSEWORD_BIG( pPtr ) * 2;
		pPtr += uiStringSize + 2;
		uiSize += ( uiStringSize + 5 ) & ~3;
	}

	return uiSize;
}

// $deBlob: FUNCTION 006d8ff0
TUINT32 TXUIResource::GetVectorTableSize( TUINT8* a_pData, TUINT32 a_uiSize )
{
	return ( ( a_uiSize / 12 ) * sizeof( TVector2 ) ) + 12;
}

// $deBlob: FUNCTION 006d9010
TUINT32 TXUIResource::GetQuatTableSize( TUINT8* a_pData, TUINT32 a_uiSize )
{
	return ( a_uiSize >> 4 ) * sizeof( TFLOAT ) + 4;
}

// $deBlob: FUNCTION 006d9030
TUINT32 TXUIResource::GetCustDataSize( TUINT8* a_pData, TUINT32 a_uiSize )
{
	return ( a_uiSize + 3 ) & ~3;
}

// $deBlob: FUNCTION 006d9040
TUINT32 TXUIResource::GetDataSectionSize( TUINT8* a_pData, TUINT32 a_uiSize )
{
	TUINT16 uiType = PARSEWORD_BIG( a_pData );
	a_pData += 2;
	TASSERT( 0 == TStringManager::String16Compare( m_asStringTable[ uiType ], L"XuiCanvas", -1 ) );

	TUINT32 uiObjectSize = GetObjectDataSize( *this, m_asStringTable[ uiType ] );

	TUINT8 uiFlags = *a_pData;
	a_pData += 1;

	XURXUIObjectData* pRoot = ( uiType == 0 ) ? TNULL : CreateObjectData( *this, m_asStringTable[ uiType ] );

	TUINT32 uiSize = ( ( uiObjectSize + 15 ) & ~15 ) + pRoot->CalcPileSize( *this, a_pData );

	if ( uiFlags & XURXUIElementData::ChildFlags_HasChildren )
		uiSize += pRoot->CalcChildrenPileSize( *this, a_pData );

	if ( uiFlags & XURXUIElementData::ChildFlags_HasNamedFrames )
	{
		uiSize += pRoot->CalcNamedFramesPileSize( *this, a_pData );

		if ( uiFlags & XURXUIElementData::ChildFlags_HasChildren )
			uiSize += pRoot->CalcTimelinesPileSize( *this, a_pData );
	}

	delete pRoot;

	return ( uiSize + 3 ) & ~3;
}

// $deBlob: FUNCTION 006d9160
XURXUIObjectData* CreateObjectData( TXUIResource& a_rResource, TUINT16 a_uiType )
{
	if ( a_uiType < 1 )
		return TNULL;

	return CreateObjectData( a_rResource, a_rResource.GetString( a_uiType ) );
}

// Creates a data object in TXUI::ms_pXUIMemoryBlock
#define TXUIRESOURCE_CREATEDATA( XUITYPE, DATACLASS )                                \
	if ( TStringManager::String16Compare( a_wszType, XUITYPE, -1 ) == 0 )          \
		return new ( TXUI::ms_pXUIMemoryBlock ) DATACLASS();

// $deBlob: FUNCTION 006d9190
XURXUIObjectData* CreateObjectData( TXUIResource& a_rResource, const TWCHAR* a_wszType )
{
	TXUIRESOURCE_CREATEDATA( L"XuiCanvas", XURXUICanvasData );
	TXUIRESOURCE_CREATEDATA( L"XuiScene", XURXUISceneData );
	TXUIRESOURCE_CREATEDATA( L"XuiTabScene", XURXUITabSceneData );
	TXUIRESOURCE_CREATEDATA( L"XuiFigure", XURXUIFigureData );
	TXUIRESOURCE_CREATEDATA( L"XuiVisual", XURXUIVisualData );
	TXUIRESOURCE_CREATEDATA( L"XuiImage", XURXUIImageData );
	TXUIRESOURCE_CREATEDATA( L"XuiText", XURXUITextData );
	TXUIRESOURCE_CREATEDATA( L"XuiButton", XURXUIButtonData );
	TXUIRESOURCE_CREATEDATA( L"XuiGroup", XURXUIGroupData );
	TXUIRESOURCE_CREATEDATA( L"XuiImagePresenter", XURXUIImagePresenterData );
	TXUIRESOURCE_CREATEDATA( L"XuiTextPresenter", XURXUITextPresenterData );
	TXUIRESOURCE_CREATEDATA( L"XuiNineGrid", XURXUINineGridData );
	TXUIRESOURCE_CREATEDATA( L"XuiSoundXAudio", XURXUISoundXAudioData );
	TXUIRESOURCE_CREATEDATA( L"XuiSoundXACT", XURXUISoundXACTData );
	TXUIRESOURCE_CREATEDATA( L"XuiLabel", XURXUILabelData );
	TXUIRESOURCE_CREATEDATA( L"XuiEdit", XURXUIEditData );
	TXUIRESOURCE_CREATEDATA( L"XuiMessageBoxButton", XURXUIMsgBoxButtonData );
	TXUIRESOURCE_CREATEDATA( L"XuiNavButton", XURXUINavButtonData );
	TXUIRESOURCE_CREATEDATA( L"XuiBackButton", XURXUIBackButtonData );
	TXUIRESOURCE_CREATEDATA( L"XuiRadioButton", XURXUIRadioButtonData );
	TXUIRESOURCE_CREATEDATA( L"XuiCheckbox", XURXUICheckBoxData );
	TXUIRESOURCE_CREATEDATA( L"XuiProgressBar", XURXUIProgressBarData );
	TXUIRESOURCE_CREATEDATA( L"XuiSlider", XURXUISliderBarData );
	TXUIRESOURCE_CREATEDATA( L"XuiList", XURXUIListData );
	TXUIRESOURCE_CREATEDATA( L"XuiCommonList", XURXUICommonListData );
	TXUIRESOURCE_CREATEDATA( L"XuiRadioGroup", XURXUIRadioGroupData );
	TXUIRESOURCE_CREATEDATA( L"XuiScrollEnd", XURXUIScrollEndData );
	TXUIRESOURCE_CREATEDATA( L"XuiCaret", XURXUICaretData );
	TXUIRESOURCE_CREATEDATA( L"XuiListItem", XURXUIListItemData );
	TXUIRESOURCE_CREATEDATA( L"XuiHtmlPresenter", XURXUIHTMLPresenterData );
	TXUIRESOURCE_CREATEDATA( L"XuiHtmlElement", XURXUIHTMLElementData );
	TXUIRESOURCE_CREATEDATA( L"XuiHtmlControl", XURXUIHTMLControlData );
	TXUIRESOURCE_CREATEDATA( L"XuiControl", XURXUIControlData );
	TXUIRESOURCE_CREATEDATA( L"XuiVideo", XURXUIVideoData );

	return TNULL;
}

#undef TXUIRESOURCE_CREATEDATA

// $deBlob: FUNCTION 006d98b0
TUINT32 GetObjectDataSize( TXUIResource& a_rResource, TUINT16 a_uiType )
{
	return ( GetObjectDataSize( a_rResource, a_rResource.GetString( a_uiType ) ) + 15 ) & ~15;
}

// NOTE: the sizes of the data classes of CreateObjectData
// $deBlob: FUNCTION 006d98e0
TUINT32 GetObjectDataSize( TXUIResource& a_rResource, const TWCHAR* a_wszType )
{
	static const struct
	{
		const TWCHAR* wszType;
		TUINT32       uiSize;
	} s_aSizes[] = {
		{ L"XuiCanvas", 0x30 },
		{ L"XuiScene", 0x54 },
		{ L"XuiTabScene", 0x60 },
		{ L"XuiFigure", 0xB8 },
		{ L"XuiVisual", 0x30 },
		{ L"XuiImage", 0x3C },
		{ L"XuiText", 0x4C },
		{ L"XuiButton", 0x48 },
		{ L"XuiGroup", 0x30 },
		{ L"XuiImagePresenter", 0x3C },
		{ L"XuiTextPresenter", 0x4C },
		{ L"XuiNineGrid", 0x48 },
		{ L"XuiSoundXAudio", 0x40 },
		{ L"XuiSoundXACT", 0x44 },
		{ L"XuiLabel", 0x44 },
		{ L"XuiEdit", 0x50 },
		{ L"XuiMessageBoxButton", 0x54 },
		{ L"XuiNavButton", 0x54 },
		{ L"XuiBackButton", 0x48 },
		{ L"XuiRadioButton", 0x48 },
		{ L"XuiCheckbox", 0x48 },
		{ L"XuiProgressBar", 0x50 },
		{ L"XuiSlider", 0x60 },
		{ L"XuiList", 0x48 },
		{ L"XuiCommonList", 0x50 },
		{ L"XuiRadioGroup", 0x44 },
		{ L"XuiScrollEnd", 0x48 },
		{ L"XuiCaret", 0x44 },
		{ L"XuiListItem", 0x58 },
		{ L"XuiHtmlPresenter", 0x34 },
		{ L"XuiHtmlElement", 0x34 },
		{ L"XuiHtmlControl", 0x48 },
		{ L"XuiControl", 0x44 },
		{ L"XuiVideo", 0x3C },
	};

	for ( TUINT32 i = 0; i < TARRAYSIZE( s_aSizes ); i++ )
	{
		if ( TStringManager::String16Compare( a_wszType, s_aSizes[ i ].wszType, -1 ) == 0 )
			return s_aSizes[ i ].uiSize;
	}

	return 0;
}

TOSHI_NAMESPACE_END
