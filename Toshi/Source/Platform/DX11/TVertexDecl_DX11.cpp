#include "ToshiPCH.h"
#include "TVertexDecl_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TVertexDecl_DX11* TVertexDecl_DX11::s_pSingleton;

// $deBlob: FUNCTION 006bac70
void TVertexDecl_DX11::Decl::Init()
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006baca0
void TVertexDecl_DX11::Decl::Deinit()
{
	m_pInputLayout->Release();
	s_pSingleton->RemoveDecl( this );
}

// $deBlob: FUNCTION 006bace0
TVertexDecl_DX11::TVertexDecl_DX11()
{
	m_pHead = TNULL;

	TASSERT( TNULL == s_pSingleton );
	s_pSingleton = this;
}

// $deBlob: FUNCTION 006bad30
// $deBlob: FUNCTION 006aa9e0
TVertexDecl_DX11::~TVertexDecl_DX11()
{
	Decl* pDecl = FindDecl( "rigid" );

	if ( pDecl != TNULL )
	{
		RemoveDecl( pDecl );
		delete pDecl;
	}

	TASSERT( TNULL != s_pSingleton );
	s_pSingleton = TNULL;
}

// $deBlob: FUNCTION 006badd0
TVertexDecl_DX11::Decl* TVertexDecl_DX11::FindDecl( const TCHAR* a_szName )
{
	for ( Decl* pDecl = m_pHead; pDecl != TNULL; pDecl = pDecl->m_pNext )
	{
		if ( TStringManager::String8CompareNoCase( pDecl->m_szName, a_szName ) == 0 )
			return pDecl;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006bae10
TVertexDecl_DX11::Decl* TVertexDecl_DX11::AddDecl( Decl* a_pDecl )
{
	if ( m_pHead == TNULL )
	{
		a_pDecl->m_pNext = TNULL;
		a_pDecl->m_pPrev = TNULL;
		m_pHead          = a_pDecl;
		return a_pDecl;
	}

	Decl* pTail = m_pHead;

	while ( pTail->m_pNext != TNULL )
		pTail = pTail->m_pNext;

	pTail->m_pNext   = a_pDecl;
	a_pDecl->m_pPrev = pTail;
	a_pDecl->m_pNext = TNULL;
	return a_pDecl;
}

// $deBlob: FUNCTION 006bae50
void TVertexDecl_DX11::RemoveDecl( Decl* a_pDecl )
{
	if ( a_pDecl->m_pNext != TNULL )
		a_pDecl->m_pNext->m_pPrev = a_pDecl->m_pPrev;

	if ( a_pDecl->m_pPrev != TNULL )
		a_pDecl->m_pPrev->m_pNext = a_pDecl->m_pNext;

	if ( a_pDecl == m_pHead )
		m_pHead = m_pHead->m_pNext;
}

TOSHI_NAMESPACE_END
