#include "ToshiPCH.h"
#include "T2GUI.h"

#include <Render/TRender.h>
#include <Render/TTexture.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

T2GUI*       T2GUI::s_Instance = TNULL;
TMemoryHeap* T2GUI::s_pMemHeap = TNULL;

// $deBlob: FUNCTION 006f4ef0
T2GUI::T2GUI()
{
	m_pRenderer = TNULL;
	m_pContext1 = TNULL;
	m_pContext2 = TNULL;

	T2GUIContext* pContext = new T2GUIContext();
	m_pContext2            = pContext;
	m_pContext1            = pContext;
}

// $deBlob: FUNCTION 006f4f90
// $deBlob: FUNCTION 006f5480
T2GUI::~T2GUI()
{
	if ( m_pContext2 )
		delete m_pContext2;
}

// $deBlob: FUNCTION 006f4fb0
T2GUI* T2GUI::Open( TMemoryHeap* a_pMemHeap )
{
	s_pMemHeap = a_pMemHeap;
	TASSERT( s_Instance == TNULL );

	s_Instance = new T2GUI();
	return s_Instance;
}

// $deBlob: FUNCTION 006f5020
void T2GUI::Close()
{
	if ( s_Instance )
		delete s_Instance;

	s_Instance = TNULL;
}

// $deBlob: FUNCTION 006f5060
void T2GUI::SetRenderer( T2GUIRenderer* a_pRenderer )
{
	m_pRenderer = a_pRenderer;
}

// $deBlob: FUNCTION 006f5070
T2GUIRenderer* T2GUI::GetRenderer() const
{
	return m_pRenderer;
}

// $deBlob: FUNCTION 006f50a0
void T2GUI::Render()
{
	if ( m_pContext1 && m_pRenderer )
	{
		m_pRenderer->BeginScene();
		T2GUIElement* pRoot = m_pContext1->GetRootElement();
		pRoot->PreRender();
		pRoot->Render();
		pRoot->PostRender();
		m_pRenderer->EndScene();
	}
}

// $deBlob: FUNCTION 006f50e0
T2GUIMaterial* T2GUI::CreateMaterial( TTexture* a_pTexture )
{
	T2GUIMaterial* pMaterial = ( s_pMemHeap != TNULL ) ? new ( s_pMemHeap ) T2GUIMaterial() : new T2GUIMaterial();
	pMaterial->Create();
	pMaterial->SetTextureResource( a_pTexture );

	return pMaterial;
}

// $deBlob: FUNCTION 006f5140
T2GUIMaterial* T2GUI::CreateMaterial( const TCHAR* a_szTextureName )
{
	TTexture* pTexture = GetTexture( a_szTextureName );

	T2GUIMaterial* pMaterial = ( s_pMemHeap != TNULL ) ? new ( s_pMemHeap ) T2GUIMaterial() : new T2GUIMaterial();
	pMaterial->Create();
	pMaterial->SetTextureResource( pTexture );

	return pMaterial;
}

// $deBlob: FUNCTION 006f51b0
void T2GUI::DestroyMaterial( TMaterial* a_pMaterial )
{
	if ( a_pMaterial )
	{
		TASSERT( a_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );
		T2GUIMaterial* pMaterial = TSTATICCAST( T2GUIMaterial, a_pMaterial );
		pMaterial->OnDestroy();
		delete pMaterial;
	}
}

// $deBlob: FUNCTION 006f5220
TTexture* T2GUI::GetTexture( const TCHAR* a_szTextureName )
{
	TCHAR szTextureName[ 128 ];
	TStringManager::String8CopySafe( szTextureName, a_szTextureName, sizeof( szTextureName ) );
	TStringManager::String8ToLowerCase( szTextureName );

	TString8 strTextureName( szTextureName );
	TINT     iSlash = strTextureName.FindReverse( '\\', -1 );
	strTextureName  = strTextureName.Mid( iSlash + 1, strTextureName.Length() - iSlash - 1 );

	return TTextureManager::GetSingleton()->FindTexture( strTextureName.GetString() );
}

// $deBlob: FUNCTION 006f5310
TUINT T2GUI::GetWidth( T2GUIMaterial* a_pMaterial )
{
	TASSERT( a_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );
	return a_pMaterial->GetTextureResource()->GetWidth();
}

// $deBlob: FUNCTION 006f5370
TUINT T2GUI::GetHeight( T2GUIMaterial* a_pMaterial )
{
	TASSERT( a_pMaterial->IsA( &TGetClass( T2GUIMaterial ) ) );
	return a_pMaterial->GetTextureResource()->GetHeight();
}

TOSHI_NAMESPACE_END
