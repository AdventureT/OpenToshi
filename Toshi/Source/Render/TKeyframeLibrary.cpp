#include "ToshiPCH.h"
#include "TAnimation.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

static TMemoryHeap* s_pAnimMemHeap;

TSTATICASSERT( sizeof( TKeyframeLibrary ) == 0x44 );

// $deBlob: FUNCTION 0068c290
void SetTAnimMemoryHeap( TMemoryHeap* a_pHeap )
{
	s_pAnimMemHeap = a_pHeap;
}

// $deBlob: FUNCTION 0068c2a0
TMemoryHeap* GetTAnimMemoryHeap()
{
	return s_pAnimMemHeap;
}

// $deBlob: FUNCTION 0068e310
TKeyframeLibrary* __stdcall TKeyframeLibrary::Create( const TCHAR* a_szName, TINT a_iNumTranslations, TINT a_iNumQuaternions, TINT a_iNumScales, const TVector3& a_rSomeVector )
{
	TSIZE uiSize = sizeof( TKeyframeLibrary ) + a_iNumTranslations * sizeof( TAnimVector ) + a_iNumQuaternions * sizeof( TAnimQuaternion ) + a_iNumScales * sizeof( TAnimScale );

	TKeyframeLibrary* pLibrary = TSTATICCAST( TKeyframeLibrary, ( s_pAnimMemHeap != TNULL ) ? TMemoryHeap_Malloc( s_pAnimMemHeap, uiSize ) : TMalloc( uiSize ) );

	if ( pLibrary == TNULL )
		return TNULL;

	// The inlined constructor
	new ( pLibrary ) TQList<TKeyframeLibrary>::TNode();
	pLibrary->m_iReferenceCount  = 0;
	pLibrary->m_iNumTranslations = 0;
	pLibrary->m_iNumQuaternions  = 0;
	pLibrary->m_iNumScales       = 0;
	pLibrary->m_pTranslations    = TNULL;
	pLibrary->m_pQuaternions     = TNULL;
	pLibrary->m_pScales          = TNULL;
	pLibrary->m_pTRB             = TNULL;

	pLibrary->m_szName           = a_szName;
	pLibrary->m_iReferenceCount  = 0;
	pLibrary->m_SomeVector       = a_rSomeVector;
	pLibrary->m_iNumQuaternions  = a_iNumQuaternions;
	pLibrary->m_iNumTranslations = a_iNumTranslations;
	pLibrary->m_pScales          = TREINTERPRETCAST( TAnimScale*, TREINTERPRETCAST( TAnimQuaternion*, TREINTERPRETCAST( TAnimVector*, pLibrary + 1 ) + a_iNumTranslations ) + a_iNumQuaternions );
	pLibrary->m_iNumScales       = a_iNumScales;
	pLibrary->m_pQuaternions     = TREINTERPRETCAST( TAnimQuaternion*, TREINTERPRETCAST( TAnimVector*, pLibrary + 1 ) + a_iNumTranslations );
	pLibrary->m_pTranslations    = TREINTERPRETCAST( TAnimVector*, pLibrary + 1 );

	return pLibrary;
}

// $deBlob: FUNCTION 0068e3e0
TKeyframeLibrary* __stdcall TKeyframeLibrary::CreateFromData( TRBHeader* a_pTRBHeader )
{
	TKeyframeLibrary* pLibrary = TSTATICCAST( TKeyframeLibrary, ( s_pAnimMemHeap != TNULL ) ? TMemoryHeap_Malloc( s_pAnimMemHeap, sizeof( TKeyframeLibrary ) ) : TMalloc( sizeof( TKeyframeLibrary ) ) );

	if ( pLibrary != TNULL )
	{
		// The inlined constructor
		new ( pLibrary ) TQList<TKeyframeLibrary>::TNode();
		pLibrary->m_iReferenceCount  = 0;
		pLibrary->m_iNumTranslations = 0;
		pLibrary->m_iNumQuaternions  = 0;
		pLibrary->m_iNumScales       = 0;
		pLibrary->m_pTranslations    = TNULL;
		pLibrary->m_pQuaternions     = TNULL;
		pLibrary->m_pScales          = TNULL;
		pLibrary->m_pTRB             = TNULL;
	}

	// NOTE: retail doesn't check the allocation here
	pLibrary->m_szName           = a_pTRBHeader->m_szName;
	pLibrary->m_iReferenceCount  = 0;
	pLibrary->m_SomeVector       = a_pTRBHeader->m_SomeVector;
	pLibrary->m_iNumTranslations = a_pTRBHeader->m_iNumTranslations;
	pLibrary->m_iNumQuaternions  = a_pTRBHeader->m_iNumQuaternions;
	pLibrary->m_iNumScales       = a_pTRBHeader->m_iNumScales;
	pLibrary->m_iTranslationSize = a_pTRBHeader->m_iTranslationSize;
	pLibrary->m_iQuaternionSize  = a_pTRBHeader->m_iQuaternionSize;
	pLibrary->m_iScaleSize       = a_pTRBHeader->m_iScaleSize;
	pLibrary->m_pTranslations    = a_pTRBHeader->m_pTranslations;
	pLibrary->m_pQuaternions     = a_pTRBHeader->m_pQuaternions;
	pLibrary->m_pScales          = a_pTRBHeader->m_pScales;

	return pLibrary;
}

// $deBlob: FUNCTION 0068e4a0
TKeyframeLibrary* __stdcall TKeyframeLibrary::CreateFromTRB( TTRB* a_pTRB, const TCHAR* a_szSymbolName )
{
	return CreateFromData( TSTATICCAST( TRBHeader, a_pTRB->GetSymbolAddress( a_szSymbolName ) ) );
}

// $deBlob: FUNCTION 0068e4c0
void TKeyframeLibrary::Delete()
{
	TASSERT( 0 == m_iReferenceCount );

	if ( m_pTRB != TNULL )
	{
		m_pTRB->Close();

		if ( m_pTRB )
			delete m_pTRB;

		m_pTRB = TNULL;
	}

	TFree( this );
}

TOSHI_NAMESPACE_END
