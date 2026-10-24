#include "ToshiPCH.h"
#include "TModelManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TModelManager* TModelManager::s_pSingleton;

// $deBlob: FUNCTION 006aefe0
TModelManager::TModelManager()
    : m_pHeadModel( T2ResourcePtr::IDINVALID )
{
	TASSERT( TNULL == s_pSingleton );
	s_pSingleton = this;
}

// $deBlob: FUNCTION 006af040
// $deBlob: FUNCTION 00691740
TModelManager::~TModelManager()
{
	TASSERT( TNULL != s_pSingleton );
	s_pSingleton = TNULL;
}

// $deBlob: FUNCTION 006af0a0
T2ModelPtr TModelManager::FindModel( const TCHAR* a_szName )
{
	TASSERT( T2String8::IsLowerCase( a_szName ) );

	T2ModelPtr pModel = m_pHeadModel;

	while ( pModel )
	{
		if ( TStringManager::String8CompareNoCase( pModel->GetName(), a_szName ) == 0 )
			return pModel;

		pModel = pModel->m_pNextModel;
	}

	return T2ModelPtr( T2ResourcePtr::IDINVALID );
}

// $deBlob: FUNCTION 006af190
T2ModelPtr TModelManager::GetNextModel( T2ModelPtr a_pModel )
{
	if ( a_pModel )
		return a_pModel->m_pNextModel;

	return T2ModelPtr( T2ResourcePtr::IDINVALID );
}

// $deBlob: FUNCTION 006af200
void TModelManager::AddModel( T2ModelPtr a_pModel )
{
	a_pModel->m_pNextModel = m_pHeadModel;
	a_pModel->m_pPrevModel = T2ModelPtr( T2ResourcePtr::IDINVALID );

	if ( m_pHeadModel )
		m_pHeadModel->m_pPrevModel = a_pModel;

	m_pHeadModel = a_pModel;
}

// $deBlob: FUNCTION 006af2b0
void TModelManager::RemoveModel( T2ModelPtr a_pModel )
{
	if ( a_pModel->m_pNextModel )
		a_pModel->m_pNextModel->m_pPrevModel = a_pModel->m_pPrevModel;

	if ( a_pModel->m_pPrevModel )
		a_pModel->m_pPrevModel->m_pNextModel = a_pModel->m_pNextModel;

	if ( a_pModel == m_pHeadModel )
		m_pHeadModel = m_pHeadModel->m_pNextModel;
}

TOSHI_NAMESPACE_END
