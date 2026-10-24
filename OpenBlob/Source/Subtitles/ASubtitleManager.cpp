#include "pch.h"
#include "ASubtitleManager.h"
#include "Locale/ALocaleManager.h"

#include <Toshi/T2String8.h>
#include <Toshi/TUtil.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0061b1f0
ASubtitleManager::ASubtitleManager()
{
	m_bPaused         = TFALSE;
	m_pBuffer         = TNULL;
	m_pBufferAllocPtr = TNULL;
	m_pBuffer         = TSTATICCAST( TUINT8, TMemalign( 16, sm_iMaxTRBBufferSize ) );
	m_oTRB.SetMemoryFunctions( TRBAllocator, TRBDeallocator, this );

	for ( TINT i = 0; i < NUM_SUBTITLES; i++ )
		m_oFreeSubList.PushBack( new ASubtitle() );
}

// NOTE: the subtitles still in m_oManagedSubList are popped before StopSubtitle, which then can't find them in a
// list, so they're never moved to m_oFreeSubList nor deleted (a leak; no subtitle is ever played on PC)
// $deBlob: FUNCTION 0061bac0
// $deBlob: FUNCTION 0061b330
ASubtitleManager::~ASubtitleManager()
{
	TASSERT( m_oUnManagedSubList.IsEmpty() );

	while ( !m_oManagedSubList.IsEmpty() )
		StopSubtitle( m_oManagedSubList.PopBack() );

	while ( !m_oFreeSubList.IsEmpty() )
	{
		ASubtitle* pSubtitle = m_oFreeSubList.PopBack();

		if ( pSubtitle != TNULL )
			delete pSubtitle;
	}

	m_oTRB.Close();
	TFree( m_pBuffer );
}

// $deBlob: FUNCTION 0061b490
TBOOL ASubtitleManager::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bPaused != TTRUE )
	{
		for ( auto it = m_oManagedSubList.Begin(); it != m_oManagedSubList.End(); )
		{
			ASubtitle* pSubtitle = it.Get();
			it++;

			if ( !pSubtitle->Update( a_fDeltaTime ) )
				StopSubtitle( pSubtitle );
		}
	}

	return TTRUE;
}

// NOTE: a_iLangId isn't used, retail reads the language of T2Locale
// $deBlob: FUNCTION 0061b540
void ASubtitleManager::LoadSubtitles( TINT a_iLangId )
{
	m_oTRB.Close();
	m_pBufferAllocPtr = m_pBuffer;
	TUtil::MemClear( m_pBuffer, sm_iMaxTRBBufferSize );

	const TCHAR* pszFile;

	switch ( T2Locale::GetSingleton()->GetLanguage() )
	{
		case ALocaleManager::ALANG_ENGLISHUK: pszFile = "Data/Locale/Subtitles/subs_GBR.trb"; break;
		case ALocaleManager::ALANG_DUTCH: pszFile = "Data/Locale/Subtitles/subs_NED.trb"; break;
		case ALocaleManager::ALANG_GERMAN: pszFile = "Data/Locale/Subtitles/subs_GER.trb"; break;
		case ALocaleManager::ALANG_ITALIAN: pszFile = "Data/Locale/Subtitles/subs_ITA.trb"; break;
		case ALocaleManager::ALANG_SPANISH: pszFile = "Data/Locale/Subtitles/subs_SPA.trb"; break;
		case ALocaleManager::ALANG_LATAMSPANISH: pszFile = "Data/Locale/Subtitles/Subs_LAS.trb"; break;
		case ALocaleManager::ALANG_FRENCH: pszFile = "Data/Locale/Subtitles/subs_FRE.trb"; break;
		case ALocaleManager::ALANG_DANISH: pszFile = "Data/Locale/Subtitles/subs_DAN.trb"; break;
		case ALocaleManager::ALANG_NORWEGIAN: pszFile = "Data/Locale/Subtitles/subs_NOR.trb"; break;
		case ALocaleManager::ALANG_SWEDISH: pszFile = "Data/Locale/Subtitles/subs_SWE.trb"; break;
		case ALocaleManager::ALANG_FINNISH: pszFile = "Data/Locale/Subtitles/subs_FIN.trb"; break;
		case ALocaleManager::ALANG_JAPANESE: pszFile = "Data/Locale/Subtitles/subs_JPN.trb"; break;
		case ALocaleManager::ALANG_KOREAN: pszFile = "Data/Locale/Subtitles/subs_KOR.trb"; break;
		default: pszFile = "Data/Locale/Subtitles/subs_ENG.trb"; break;
	}

	TCHAR szFileName[ 256 ];
	T2String8::Format( szFileName, pszFile );

	TTRB::ERROR err = m_oTRB.Load( szFileName );
	TASSERT( err == TTRB::ERROR_OK );

	TINT* piVersion = m_oTRB.CastSymbol<TINT>( "SubtitleVersion" );
	TASSERT( TNULL != piVersion );
	TASSERT( ( *piVersion ) == VERSION );
}

// $deBlob: FUNCTION 0061b880
ASubtitle::SubHeader* ASubtitleManager::GetSubtitleHeader( const TCHAR* a_szName )
{
	return m_oTRB.CastSymbol<ASubtitle::SubHeader>( a_szName );
}

// NOTE: de Blob PC's PlaySubtitle is empty, so no subtitle is ever shown (the movies have burned-in subtitles or none)
// $deBlob: FUNCTION 0061b700
ASubtitle* ASubtitleManager::PlaySubtitle( const TCHAR* a_szName, TUINT32 a_uiFlags, TFLOAT a_fLength, TBOOL a_bUnManaged )
{
	return TNULL;
}

// $deBlob: FUNCTION 0061b710
void ASubtitleManager::StopSubtitle( ASubtitle* a_pSubtitle )
{
	a_pSubtitle->Reset();

	for ( auto it = m_oManagedSubList.Begin(); it != m_oManagedSubList.End(); it++ )
	{
		if ( it.Get() == a_pSubtitle )
		{
			m_oManagedSubList.Transfer( a_pSubtitle, m_oFreeSubList );
			return;
		}
	}

	for ( auto it = m_oUnManagedSubList.Begin(); it != m_oUnManagedSubList.End(); it++ )
	{
		if ( it.Get() == a_pSubtitle )
		{
			m_oUnManagedSubList.Transfer( a_pSubtitle, m_oFreeSubList );
			return;
		}
	}
}

// $deBlob: FUNCTION 0061b890
void* ASubtitleManager::TRBAlloc( TINT a_iSize )
{
	TUINT8* pMem = m_pBufferAllocPtr;
	m_pBufferAllocPtr += a_iSize;
	TASSERT( m_pBufferAllocPtr <= ( m_pBuffer + sm_iMaxTRBBufferSize ) );

	return pMem;
}

// NOTE: the alignment isn't applied
// $deBlob: FUNCTION 0061b8f0
void* __stdcall ASubtitleManager::TRBAllocator( TTRB::AllocType a_eAllocType, TUINT32 a_uiSize, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData )
{
	return TSTATICCAST( ASubtitleManager, a_pUserData )->TRBAlloc( a_uiSize );
}

// $deBlob: FUNCTION 0061b950
void __stdcall ASubtitleManager::TRBDeallocator( TTRB::AllocType a_eAllocType, void* a_pMem, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData )
{
}
