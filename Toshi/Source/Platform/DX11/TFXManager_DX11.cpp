#include "ToshiPCH.h"
#include "TFXManager_DX11.h"
#include "TRender_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TFXManager_DX11* TFXManager_DX11::s_pSingleton;

// $deBlob: FUNCTION 006ba690
void TFXManager_DX11::FXLite::Init()
{
	FXLite* pLoaded = s_pSingleton->FindFX( m_szName );

	if ( pLoaded == TNULL )
	{
		TRenderDX11::Interface()->CreateFXShader( m_pShaderData, &m_pVertexShader );
		TRenderDX11::Interface()->CreateFXShader( m_pShaderData, &m_pPixelShader );

		// NOTE: allocated, never constructed
		m_pVSConstants = TSTATICCAST( TFXConstant_DX11, operator new( sizeof( TFXConstant_DX11 ) ) );
		m_pPSConstants = TSTATICCAST( TFXConstant_DX11, operator new( sizeof( TFXConstant_DX11 ) ) );
		m_pVSConstants->Init( m_pVertexShader );
		m_pPSConstants->Init( m_pPixelShader );
	}
	else
	{
		// Shares the shaders of the effect with the same name
		m_pVertexShader = pLoaded->m_pVertexShader;
		m_pPixelShader  = pLoaded->m_pPixelShader;
		m_pVSConstants  = pLoaded->m_pVSConstants;
		m_pPSConstants  = pLoaded->m_pPSConstants;
	}

	s_pSingleton->AddFX( this );
}

// $deBlob: FUNCTION 006ba740
void TFXManager_DX11::FXLite::Deinit()
{
	s_pSingleton->RemoveFX( this );

	if ( s_pSingleton->FindFX( m_szName ) == TNULL )
	{
		TASSERT( TFALSE );
		operator delete( m_pVSConstants );
		operator delete( m_pPSConstants );
	}
}

// $deBlob: FUNCTION 006ba7d0
void* TFXManager_DX11::CreateFX( void* a_pData )
{
	TASSERT( TFALSE );
	return TNULL;
}

// $deBlob: FUNCTION 006ba810
TFXManager_DX11::TFXManager_DX11()
{
	m_pHead = TNULL;

	TASSERT( TNULL == s_pSingleton );
	s_pSingleton = this;
}

// $deBlob: FUNCTION 006ba860
// $deBlob: FUNCTION 006aa830
TFXManager_DX11::~TFXManager_DX11()
{
	TASSERT( TNULL != s_pSingleton );
	s_pSingleton = TNULL;
}

// $deBlob: FUNCTION 006ba8a0
TFXManager_DX11::FXLite* TFXManager_DX11::FindFX( const TCHAR* a_szName )
{
	TASSERT( T2String8::IsLowerCase( a_szName ) );

	for ( FXLite* pFX = m_pHead; pFX != TNULL; pFX = pFX->m_pNext )
	{
		if ( TStringManager::String8CompareNoCase( pFX->m_szName, a_szName ) == 0 )
			return pFX;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006ba910
void TFXManager_DX11::AddFX( FXLite* a_pFX )
{
	a_pFX->m_pNext = m_pHead;
	a_pFX->m_pPrev = TNULL;

	if ( m_pHead != TNULL )
		m_pHead->m_pPrev = a_pFX;

	m_pHead = a_pFX;
}

// $deBlob: FUNCTION 006ba940
void TFXManager_DX11::RemoveFX( FXLite* a_pFX )
{
	if ( a_pFX->m_pNext != TNULL )
		a_pFX->m_pNext->m_pPrev = a_pFX->m_pPrev;

	if ( a_pFX->m_pPrev != TNULL )
		a_pFX->m_pPrev->m_pNext = a_pFX->m_pNext;

	if ( a_pFX == m_pHead )
		m_pHead = m_pHead->m_pNext;
}

TOSHI_NAMESPACE_END
