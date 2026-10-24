#pragma once
#include "TFXConstant_DX11.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: the list of "FX lite" effects ("tfxl" TAssetInit entries, Source\Render\TFXManager_DX11.cpp). Created by
// TRenderDX11's constructor. Not a TSingleton: the class has its own s_pSingleton. On PC the effect shaders are
// never created (TRenderDX11::CreateFXShader and TFXConstant_DX11 assert).
//-----------------------------------------------------------------------------
class TFXManager_DX11
{
public:
	struct FXLite
	{
		void Init();
		void Deinit();

		const TCHAR*      m_szName;        // 0x00 lower case
		void*             m_pShaderData;   // 0x04
		void*             m_pVertexShader; // 0x08
		void*             m_pPixelShader;  // 0x0C
		TFXConstant_DX11* m_pVSConstants;  // 0x10
		TFXConstant_DX11* m_pPSConstants;  // 0x14
		FXLite*           m_pNext;         // 0x18
		FXLite*           m_pPrev;         // 0x1C
	}; // 0x20

public:
	TFXManager_DX11();
	~TFXManager_DX11();

	void*   CreateFX( void* a_pData );
	FXLite* FindFX( const TCHAR* a_szName );
	void    AddFX( FXLite* a_pFX );
	void    RemoveFX( FXLite* a_pFX );

	// $deBlob: FUNCTION 006ba980
	FXLite* GetHead() const { return m_pHead; }

	// $deBlob: FUNCTION 006ab180
	static TFXManager_DX11* GetSingleton() { return s_pSingleton; }

private:
	static TFXManager_DX11* s_pSingleton;

private:
	FXLite* m_pHead; // 0x00
}; // 0x04

TOSHI_NAMESPACE_END
