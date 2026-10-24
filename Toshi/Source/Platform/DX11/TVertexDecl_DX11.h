#pragma once
#include <d3d11.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: the list of vertex declarations ("tvdc" TAssetInit entries, Source\Render\TVertexDecl_DX11.cpp).
// Created by TRenderDX11's constructor. Not a TSingleton: the class has its own s_pSingleton. On PC no TRB
// creates one (Decl::Init asserts), only the deinit path and the "rigid" cleanup in the destructor are live.
//-----------------------------------------------------------------------------
class TVertexDecl_DX11
{
public:
	struct Decl
	{
		void Init();
		void Deinit();

		const TCHAR*       m_szName;       // 0x00
		void*              m_pUnk;         // 0x04
		ID3D11InputLayout* m_pInputLayout; // 0x08
		Decl*              m_pNext;        // 0x0C
		Decl*              m_pPrev;        // 0x10
	}; // 0x14

public:
	TVertexDecl_DX11();
	~TVertexDecl_DX11();

	Decl* FindDecl( const TCHAR* a_szName );
	Decl* AddDecl( Decl* a_pDecl );
	void  RemoveDecl( Decl* a_pDecl );

	// $deBlob: FUNCTION 006bae90
	Decl* GetHead() const { return m_pHead; }

	// $deBlob: FUNCTION 006ab190
	static TVertexDecl_DX11* GetSingleton() { return s_pSingleton; }

private:
	static TVertexDecl_DX11* s_pSingleton;

private:
	Decl* m_pHead; // 0x00
}; // 0x04

TOSHI_NAMESPACE_END
