#include "ToshiPCH.h"
#include "TAnimation.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0068e6e0
TKeyframeLibraryManager::TKeyframeLibraryManager()
{
}

// $deBlob: FUNCTION 0068e6f0
TKeyframeLibraryManager::~TKeyframeLibraryManager()
{
	m_List.Clear();
}

// $deBlob: FUNCTION 0068e730
TKeyframeLibrary* TKeyframeLibraryManager::LoadLibrary( const TCHAR* a_szFilePath )
{
	TTRB* pTRB = new TTRB();

	if ( pTRB->Load( a_szFilePath, 0 ) != TTRB::ERROR_OK )
	{
		if ( pTRB )
			delete pTRB;

		return TNULL;
	}

	TKeyframeLibrary* pLibrary = LoadLibrary( TSTATICCAST( TKeyframeLibrary::TRBHeader, pTRB->GetSymbolAddress( "tkey" ) ) );

	if ( pLibrary == TNULL )
	{
		if ( pTRB )
			delete pTRB;

		return TNULL;
	}

	pLibrary->SetTRB( pTRB );
	return pLibrary;
}

// $deBlob: FUNCTION 0068e7c0
TKeyframeLibrary* TKeyframeLibraryManager::LoadLibrary( TKeyframeLibrary::TRBHeader* a_pTRBHeader )
{
	TKeyframeLibrary* pLibrary = TKeyframeLibrary::CreateFromData( a_pTRBHeader );

	if ( pLibrary != TNULL )
	{
		m_List.PushFront( pLibrary );
		m_iNumLibraries++;
	}

	return pLibrary;
}

// $deBlob: FUNCTION 0068e830
TKeyframeLibrary* TKeyframeLibraryManager::LoadLibrary( TTRB* a_pTRB, const TCHAR* a_szSymbolName )
{
	TKeyframeLibrary* pLibrary = TKeyframeLibrary::CreateFromTRB( a_pTRB, a_szSymbolName );

	if ( pLibrary != TNULL )
	{
		m_List.PushFront( pLibrary );
		m_iNumLibraries++;
	}

	return pLibrary;
}

// $deBlob: FUNCTION 0068e8a0
void TKeyframeLibraryManager::UnloadLibrary( TKeyframeLibrary* a_pLibrary )
{
	a_pLibrary->Remove();
	a_pLibrary->Delete();
	m_iNumLibraries--;
}

// $deBlob: FUNCTION 0068e8d0
TKeyframeLibrary* TKeyframeLibraryManager::GetLibrary( const TCHAR* a_szName )
{
	for ( auto it = m_List.Begin(); it != m_List.End(); ++it )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName(), a_szName ) == 0 )
			return it;
	}

	return TNULL;
}

TOSHI_NAMESPACE_END
