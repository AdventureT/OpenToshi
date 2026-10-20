#include "pch.h"
#include "AIOTemplateManager.h"

#include <File/TTRB.h>
#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007caae0
AIOTemplateManager::AIOTemplateManager()
{
	TASSERT( ms_pSingleton == TNULL );
	ms_pSingleton = this;

	m_pTRB = TNULL;
}

// $deBlob: FUNCTION 007cae40
// $deBlob: FUNCTION 007cab30
AIOTemplateManager::~AIOTemplateManager()
{
	if ( m_pTRB )
		delete m_pTRB;

	m_pTRB = TNULL;

	TASSERT( ms_pSingleton == this );
	ms_pSingleton = TNULL;
}

// $deBlob: FUNCTION 007cabe0
void AIOTemplateManager::Load( const TCHAR* a_szFileName )
{
	if ( m_pTRB )
		delete m_pTRB;

	m_pTRB = new TTRB;

	TTRB::ERROR eRet = m_pTRB->Load( a_szFileName, 0 );
	TASSERT( eRet == TTRB::ERROR_OK );
}

// Returns the IO data of the template a_szName: the symbol "<name without its 4 character extension>_main" of the TRB
// $deBlob: FUNCTION 007cacd0
void* AIOTemplateManager::GetIOTemplateReference( const TCHAR* a_szName )
{
	TString8 strSymbol( a_szName );
	strSymbol.Delete( strSymbol.Find( "." ), 4 );
	strSymbol += "_main";

	TASSERT( strSymbol.IsIndexValid( 0 ) );
	return m_pTRB->GetSymbolAddress( strSymbol );
}
