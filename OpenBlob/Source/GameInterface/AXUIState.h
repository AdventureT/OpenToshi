#pragma once

#include "GameInterface/AGameState.h"
#include "Toshi/File/TTRB.h"

constexpr uint32_t BUFFSIZE = 128;

TOBJECT(AXUIState, AGameState, TTRUE)
{

	inline static char s_AssetLoadQueue[BUFFSIZE] = {};
	inline static TBOOL s_bFontLoaded = TFALSE;
	inline static Toshi::TTRB s_FontTRB;

	inline static Toshi::TMemoryHeap* s_pFontTRBMemoryBlock = TNULL;
	static Toshi::TTRB::t_MemoryFuncAlloc FontTRBAllocator;
	static Toshi::TTRB::t_MemoryFuncDealloc FontTRBDeallocator;
public:
	static Toshi::TMemoryHeap* SetFontMemBlock(Toshi::TMemoryHeap* a_pMemHeap);

	static void SetSkin1(const char* a_szTRBFileName, const char* a_szXURFileName);
	static void SetSkin2(const char* a_szTRBFileName, const char* a_szXURFileName);
	static TBOOL InitFont(const char* a_szTRBFileName);
};

