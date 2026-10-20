#pragma once
#include "ASubtitle.h"

#include <Toshi/TSingleton.h>
#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Purpose: the subtitles: loads the subtitles TRB of the current language (Data/Locale/Subtitles/subs_*.trb) into its
// own buffer and updates the subtitles being shown. Has a pool of 10 ASubtitle objects. Created by AAppInitState,
// updated by AApplication and stopped by AMovieState.
// NOTE: in de Blob PC PlaySubtitle always returns TNULL, so no subtitle is ever shown
//-----------------------------------------------------------------------------
class ASubtitleManager
    : public Toshi::TSingleton<ASubtitleManager>
{
public:
	static TCONSTEXPR_VAR TINT    VERSION              = 0x100;
	static TCONSTEXPR_VAR TUINT32 sm_iMaxTRBBufferSize = 0x21000;
	static TCONSTEXPR_VAR TINT    NUM_SUBTITLES        = 10;

public:
	ASubtitleManager();
	virtual ~ASubtitleManager(); // +0x00

	TBOOL                 Update( TFLOAT a_fDeltaTime );
	void                  LoadSubtitles( TINT a_iLangId );
	ASubtitle::SubHeader* GetSubtitleHeader( const TCHAR* a_szName );
	ASubtitle*            PlaySubtitle( const TCHAR* a_szName, TUINT32 a_uiFlags, TFLOAT a_fLength, TBOOL a_bUnManaged );
	void                  StopSubtitle( ASubtitle* a_pSubtitle );
	void*                 TRBAlloc( TINT a_iSize );

	// $deBlob: FUNCTION 005cb580
	void SetPaused( TBOOL a_bPaused ) { m_bPaused = a_bPaused; }

	static void* __stdcall TRBAllocator( Toshi::TTRB::AllocType a_eAllocType, TUINT32 a_uiSize, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData );
	static void __stdcall TRBDeallocator( Toshi::TTRB::AllocType a_eAllocType, void* a_pMem, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData );

private:
	Toshi::T2SList<ASubtitle> m_oManagedSubList;   // 0x04 updated by Update
	Toshi::T2SList<ASubtitle> m_oUnManagedSubList; // 0x08
	Toshi::T2SList<ASubtitle> m_oFreeSubList;      // 0x0C
	TBOOL                     m_bPaused;           // 0x10 Update does nothing while set
	Toshi::TTRB               m_oTRB;              // 0x14
	TUINT8*                   m_pBuffer;           // 0x28 sm_iMaxTRBBufferSize bytes for m_oTRB
	TUINT8*                   m_pBufferAllocPtr;   // 0x2C
};

TSTATICASSERT( sizeof( ASubtitleManager ) == 0x30 );
