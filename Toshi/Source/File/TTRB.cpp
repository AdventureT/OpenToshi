#include "ToshiPCH.h"
#include "TTRB.h"
#include "TTSF.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

#ifndef TMAKEVERSION
// NOTE: de Blob's name of TVERSION (from the asserts)
#  define TMAKEVERSION( VER_MAJOR, VER_MINOR ) TVERSION( VER_MAJOR, VER_MINOR )
#endif

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00687c00
// $Barnyard: FUNCTION 006ba980
static void* __stdcall DefaultAllocator( TTRB::AllocType alloctype, TUINT32 size, TINT16 alignment, TUINT32 unk2, void* userData )
{
	// NOTE: de Blob honours the section alignment (Barnyard always aligns to 128)
	return TMemalign( alignment, size, TNULL );
}

// $deBlob: FUNCTION 00687c20
// $Barnyard: FUNCTION 006ba9a0
static void __stdcall DefaultDeallocator( TTRB::AllocType alloctype, void* ptr, TINT16 alignment, TUINT32 unk2, void* userData )
{
	TFree( ptr );
}

TTRB::t_MemoryFuncAlloc   TTRB::s_cbDefAllocator        = DefaultAllocator;
TTRB::t_MemoryFuncDealloc TTRB::s_cbDefDeallocator      = DefaultDeallocator;
void*                     TTRB::s_pDefAllocatorUserData = TNULL;

// $deBlob: FUNCTION 006868a0
// $Barnyard: FUNCTION 006ba9e0
TTRB::TTRB()
{
	m_pHeader = TNULL;
	m_SYMB    = TNULL;
	SetMemoryFunctions( s_cbDefAllocator, s_cbDefDeallocator, s_pDefAllocatorUserData );
}

// $deBlob: FUNCTION 006868d0
// $deBlob: FUNCTION 0056d9e0
// $Barnyard: FUNCTION 006baff0
TTRB::~TTRB()
{
	Close();
}

// $deBlob: FUNCTION 006868e0
// $Barnyard: FUNCTION 006bb000
TTRB::ERROR TTRB::Load( const TCHAR* a_szFilePath, TUINT32 a_uiUnknown )
{
	TPROFILER_SCOPE();
	TFile* pFile = TFile::Create( a_szFilePath, TFILEMODE_READ );
	return Load( pFile, a_uiUnknown );
}

// $deBlob: FUNCTION 00686920
TTRB::ERROR TTRB::Load( TFile* a_pFile, TUINT32 a_uiUnknown )
{
	TTSFI oTTSFI;
	ERROR eError;

	if ( oTTSFI.Open( a_pFile ) == ERROR_OK )
	{
		if ( oTTSFI.m_Magic == TFourCC( "TRBF" ) )
		{
			eError = ProcessForm( oTTSFI, TREINTERPRETCAST( TFORM*, &oTTSFI.m_Magic ) ) ? ERROR_OK : ERROR_PARSE_ERROR;
		}
		else
		{
			eError = ERROR_NOT_TRBF;
		}

		oTTSFI.Close();
	}
	else
	{
		eError = ERROR_NO_HEADER;
	}

	return eError;
}

// $deBlob: FUNCTION 00686f10
// $Barnyard: FUNCTION 006baad0
TBOOL TTRB::ProcessForm( TTSFI& ttsf, TFORM* a_pForm )
{
	TPROFILER_SCOPE();

	static TCONSTEXPR_VAR TINT MAX_RELC_NUM_BATCH = 512;
	RELCEntry             relcEntries[ MAX_RELC_NUM_BATCH ];

	TINT32 fileSize = ttsf.m_CurrentHunk.Size - 4;
	TINT32 leftSize = fileSize;
	ttsf.PushForm();

	while ( fileSize > 0 )
	{
		if ( ttsf.ReadHunk() != ERROR_OK ) return TFALSE;

		TUINT32 sectionName = ttsf.m_CurrentHunk.Name;
		TUINT32 sectionSize = ttsf.m_CurrentHunk.Size;
		leftSize -= TAlignNumUp( sectionSize ) + sizeof( Toshi::TTSF::Hunk );

		if ( sectionName == TFourCC( "HEAD" ) )
		{
			TINT numsections = ( sectionSize - 4 ) / 0xC;

			m_pHeader                = static_cast<Header*>( m_MemAllocator( AllocType_Header, GetHeaderSize( numsections ), 0, 0, m_MemUserData ) );
			m_pHeader->m_ui32Version = { 0 };

			ttsf.ReadRaw( &m_pHeader->m_i32SectionCount, sizeof( m_pHeader->m_i32SectionCount ) );

			TASSERT( m_pHeader->m_ui32Version == TMAKEVERSION( 0, 0 ) );
			TASSERT( m_pHeader->m_i32SectionCount == numsections );

			for ( TINT i = 0; i < m_pHeader->m_i32SectionCount; i++ )
			{
				SecInfo* pSect = GetSectionInfo( i );
				ttsf.ReadRaw( pSect, 0xC );
				pSect->m_Unk2 = 0;
			}

			for ( TINT i = 0; i < m_pHeader->m_i32SectionCount; i++ )
			{
				SecInfo* pSect = GetSectionInfo( i );

				if ( pSect->m_Alignment == 0 )
					pSect->m_Alignment = 16;

				pSect->m_Data = m_MemAllocator( AllocType_Data, pSect->m_Size, pSect->m_Alignment, pSect->m_Unk2, m_MemUserData );
			}

			ttsf.SkipHunk();
		}
		else if ( sectionName == TFourCC( "SYMB" ) )
		{
			m_SYMB = static_cast<SYMB*>( m_MemAllocator( AllocType_Symbols, sectionSize, 0, 0, m_MemUserData ) );
			ttsf.ReadHunkData( m_SYMB );
		}
		else if ( sectionName == TFourCC( "SECC" ) )
		{
			for ( TINT i = 0; i < m_pHeader->m_i32SectionCount; i++ )
			{
				SecInfo* pSect = GetSectionInfo( i );

				if ( pSect->m_Data != TNULL )
					ttsf.ReadCompressed( pSect->m_Data, pSect->m_Size );
			}

			ttsf.SkipHunk();
		}
		else if ( sectionName == TFourCC( "RELC" ) )
		{
			TINT32 relocCount   = 0;
			TINT32 iCurReloc    = 0;
			TINT32 readedRelocs = 0;

			ttsf.ReadRaw( &relocCount, sizeof( relocCount ) );

			if ( relocCount > 0 )
			{
				do
				{
					// limit count of RELCs to read
					TINT32 relocReadCount = TMath::Min( relocCount - readedRelocs, MAX_RELC_NUM_BATCH );
					ttsf.ReadRaw( relcEntries, relocReadCount * sizeof( RELCEntry ) );
					iCurReloc    = readedRelocs + relocReadCount;
					readedRelocs = iCurReloc;

					for ( TINT i = 0; i < relocReadCount; i++ )
					{
						auto& relcEntry = relcEntries[ i ];
						auto  hdrx1     = GetSectionInfo( relcEntry.HDRX1 );
						auto  hdrx2     = hdrx1;

						if ( m_pHeader->m_ui32Version.Value > TVERSION( 1, 0 ) )
						{
							hdrx2 = GetSectionInfo( relcEntry.HDRX2 );
						}

						// this won't work in x64 because pointers in TRB files are always 4 bytes
						// need some workaround to support x64 again
						TSTATICASSERT( sizeof( void* ) == 4 );
						TUINTPTR* ptr = TREINTERPRETCAST( TUINTPTR*, TUINTPTR( hdrx1->m_Data ) + relcEntry.Offset );
						*ptr += TUINTPTR( hdrx2->m_Data );
					}
				} while ( iCurReloc < relocCount );
			}

			struct
			{
				TINT32 m_i32RelocCount;
			} oTable = { relocCount };

			TASSERT( iCurReloc == oTable.m_i32RelocCount );
			ttsf.SkipHunk();
		}
		else if ( sectionName == TFourCC( "SECT" ) )
		{
			for ( TINT i = 0; i < m_pHeader->m_i32SectionCount; i++ )
			{
				SecInfo* pSect = GetSectionInfo( i );

				if ( pSect->m_Data != TNULL )
					ttsf.ReadRaw( pSect->m_Data, pSect->m_Size );
			}

			ttsf.SkipHunk();
		}
		else if ( sectionName == TFourCC( "HDRX" ) )
		{
			m_pHeader = TSTATICCAST( Header, m_MemAllocator( AllocType_Header, sectionSize, 0, 0, m_MemUserData ) );
			ttsf.ReadHunkData( m_pHeader );

			for ( TINT i = 0; i < m_pHeader->m_i32SectionCount; i++ )
			{
				SecInfo* pSect = GetSectionInfo( i );

				if ( pSect->m_Alignment == 0 )
					pSect->m_Alignment = 16;

				pSect->m_Data = m_MemAllocator( AllocType_Data, pSect->m_Size, pSect->m_Alignment, pSect->m_Unk2, m_MemUserData );
			}
		}
		else if ( sectionName == TFourCC( "FORM" ) )
		{
			TFORM form;
			ttsf.ReadFORM( &form );

			if ( !ProcessForm( ttsf, &form ) )
				return TFALSE;
		}
		else
		{
			// Unknown section
			ttsf.SkipHunk();
		}

		fileSize = leftSize;
	}

	ttsf.PopForm();
	return TTRUE;
}

// $deBlob: FUNCTION 00686d30
// $Barnyard: FUNCTION 006bafa0
void* TTRB::GetSymbolAddress( const TCHAR* symbName )
{
	TTRBSymbol* pSymbol = GetSymbol( symbName );

	if ( pSymbol != TNULL )
		return GetSymbolAddress( *pSymbol );

	return TNULL;
}

// $deBlob: FUNCTION 00686c30
// $Barnyard: FUNCTION 006baeb0
TINT TTRB::GetSymbolIndex( const TCHAR* symbName )
{
	if ( m_SYMB != TNULL )
	{
		TINT16 hash = HashString( symbName );

		for ( TINT i = 0; i < m_SYMB->m_i32SymbCount; i++ )
		{
			TTRBSymbol* symbol = reinterpret_cast<TTRBSymbol*>( m_SYMB + 1 ) + i;

			if ( symbol->NameHash == hash )
			{
				if ( T2String8::Compare( symbName, GetSymbolName( symbol ), -1 ) == 0 )
				{
					return i;
				}
			}
		}
	}

	return -1;
}

// $deBlob: FUNCTION 006869d0
// $Barnyard: FUNCTION 006baa20
void TTRB::Close()
{
	if ( m_pHeader != TNULL )
	{
		for ( TINT i = 0; i < m_pHeader->m_i32SectionCount; i++ )
		{
			DestroySection( i );
		}

		m_MemDeallocator( AllocType_Header, m_pHeader, 0, 0, m_MemUserData );
		m_pHeader = TNULL;
	}

	DeleteSymbolTable();
}

// $deBlob: FUNCTION 00686a40
void TTRB::DestroySection( TINT index )
{
	TASSERT( index >= 0 );
	TASSERT( index < m_pHeader->m_i32SectionCount );

	SecInfo* pSecInfo = GetSectionInfo( index );
	TASSERT( TNULL != pSecInfo );

	if ( pSecInfo->m_Data != TNULL )
	{
		m_MemDeallocator( AllocType_Data, pSecInfo->m_Data, pSecInfo->m_Alignment, pSecInfo->m_Unk2, m_MemUserData );
		pSecInfo->m_Data = TNULL;
	}
}

// $deBlob: FUNCTION 00686e00
// $Barnyard: FUNCTION 006ba950
void TTRB::SetMemoryFunctions( t_MemoryFuncAlloc allocator, t_MemoryFuncDealloc deallocator, void* userdata )
{
	m_MemAllocator   = allocator;
	m_MemDeallocator = deallocator;
	m_MemUserData    = userdata;
}

// $deBlob: FUNCTION 00686e20
void TTRB::SetDefaultMemoryFunctions( t_MemoryFuncAlloc allocator, t_MemoryFuncDealloc deallocator, void* userdata )
{
	if ( allocator != TNULL )
	{
		TASSERT( TNULL != deallocator );
		s_cbDefDeallocator      = deallocator;
		s_cbDefAllocator        = allocator;
		s_pDefAllocatorUserData = userdata;
	}
	else
	{
		TASSERT( deallocator == TNULL );
		s_cbDefAllocator        = DefaultAllocator;
		s_cbDefDeallocator      = DefaultDeallocator;
		s_pDefAllocatorUserData = TNULL;
	}
}

// $deBlob: FUNCTION 00686b90
// $Barnyard: FUNCTION 006ba8b0
void TTRB::DeleteSymbolTable()
{
	if ( m_SYMB != TNULL )
	{
		m_MemDeallocator( AllocType_Symbols, m_SYMB, 0, 0, m_MemUserData );
		m_SYMB = TNULL;
	}
}

// $deBlob: FUNCTION 00686b60
TINT16 TTRB::HashString( const TCHAR* str )
{
	TINT16 hash = 0;
	TCHAR  character;

	while ( *str != '\0' )
	{
		character = *( str++ );
		hash      = (TINT16)character + hash * 0x1f;
	}

	return hash;
}

// $deBlob: FUNCTION 00686ce0
// $Barnyard: FUNCTION 006ba920
const TCHAR* TTRB::GetSymbolName( TTRBSymbol* symbol ) const
{
	if ( m_SYMB == TNULL )
	{
		return TNULL;
	}

	return reinterpret_cast<const TCHAR*>(
	    reinterpret_cast<uintptr_t>( m_SYMB ) +
	    GetSymbolTableSize( m_SYMB->m_i32SymbCount ) +
	    TINT16( symbol->NameOffset )
	);
}

const TCHAR* TTRB::GetSymbolName( TINT index ) const
{
	if ( m_SYMB == TNULL )
	{
		return TNULL;
	}

	return GetSymbolName( reinterpret_cast<TTRBSymbol*>( m_SYMB + 1 ) + index );
}

// $deBlob: FUNCTION 00686bc0
// $Barnyard: FUNCTION 006ba8f0
TTRB::TTRBSymbol* TTRB::GetSymbol( TINT index ) const
{
	if ( m_SYMB != TNULL && index < m_SYMB->m_i32SymbCount )
	{
		return reinterpret_cast<TTRBSymbol*>( m_SYMB + 1 ) + index;
	}

	return TNULL;
}

// $deBlob: FUNCTION 00686bf0
// $Barnyard: FUNCTION 006baf60
TTRB::TTRBSymbol* TTRB::GetSymbol( const TCHAR* a_symbolName )
{
	TINT index = GetSymbolIndex( a_symbolName );

	if ( index == -1 )
	{
		return TNULL;
	}

	return GetSymbol( index );
}

// $deBlob: FUNCTION 00686d90
TTRB::TTRBSymbol* TTRB::GetSymbol( const void* a_pAddress )
{
	for ( TINT i = 0; i < m_SYMB->m_i32SymbCount; i++ )
	{
		TTRBSymbol* pSymbol = GetSymbol( i );

		if ( pSymbol != TNULL && a_pAddress == GetSymbolAddress( *pSymbol ) )
			return pSymbol;
	}

	return TNULL;
}

TOSHI_NAMESPACE_END
