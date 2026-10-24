#pragma once

#include "GameInterface/AGameState.h"
#include "Toshi/File/TTRB.h"

constexpr TUINT32 BUFFSIZE = 128;

TOBJECT(AXUIState, AGameState, TTRUE)
{

	inline static TCHAR       s_AssetLoadQueue[BUFFSIZE] = {};
	inline static TBOOL       s_bFontLoaded              = TFALSE;
	inline static Toshi::TTRB s_FontTRB;
	inline static Toshi::TTRB s_XUITRB;

	inline static Toshi::TMemoryHeap*       s_pFontTRBMemoryBlock = TNULL;
	static Toshi::TTRB::t_MemoryFuncAlloc   FontTRBAllocator;
	static Toshi::TTRB::t_MemoryFuncDealloc FontTRBDeallocator;

public:
	static Toshi::TMemoryHeap* SetFontMemBlock(Toshi::TMemoryHeap * a_pMemHeap);

	static void  SetFrontendCarousel(const TCHAR* a_szTRBFileName, const TCHAR* a_szXURFileName);
	static void  SetSkin1(const TCHAR* a_szTRBFileName, const TCHAR* a_szXURFileName);
	static void  SetSkin2(const TCHAR* a_szTRBFileName, const TCHAR* a_szXURFileName);
	static TBOOL SetCommonSkin(const TCHAR* a_szTRBFileName);
	static TBOOL SetSkinWithFont(const TCHAR* a_szTRBFileName, bool m_bLoad);
	static TBOOL InitFont(const TCHAR* a_szTRBFileName);
};
