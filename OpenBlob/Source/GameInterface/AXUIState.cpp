#include "pch.h"
#include "AXUIState.h"
#include "Toshi/Xui/TXUI.h"
#include "Locale/ALocaleManager.h"
#include "Toshi/Render/TAssetInit.h"

TOSHI_NAMESPACE_USING

TTRB::t_MemoryFuncAlloc AXUIState::FontTRBAllocator = [](TTRB::AllocType alloctype, size_t size, short unk1, size_t unk2, void* userData)
{
	return s_pFontTRBMemoryBlock->Memalign(unk1, size);
};

TTRB::t_MemoryFuncDealloc AXUIState::FontTRBDeallocator = [](TTRB::AllocType alloctype, void* ptr, short unk1, size_t unk2, void* userData)
{
	return TFree(ptr);
};

void AXUIState::SetSkin1(const char* a_szTRBFileName, const char* a_szXURFileName)
{
	TStringManager::String8Copy(s_AssetLoadQueue, "data/xui/", BUFFSIZE);
	const int iStrLen = TStringManager::String8Length(s_AssetLoadQueue);
	const int iExcessLen = BUFFSIZE - iStrLen;
	const char* locale = ALocaleManager::Instance().GetCurLocaleCode();
	TStringManager::String8Copy(s_AssetLoadQueue + iStrLen, locale, iExcessLen);
	const int iLen = TStringManager::String8Length(s_AssetLoadQueue);
	TASSERT(iLen < BUFFSIZE);
	s_AssetLoadQueue[iLen] = '/';
	TStringManager::String8Copy(s_AssetLoadQueue + iLen + 1, a_szTRBFileName, BUFFSIZE - iLen + 1);
	TXUI::GetSingleton()->SetSkin1(s_AssetLoadQueue, a_szXURFileName);
}

void AXUIState::SetSkin2(const char* a_szTRBFileName, const char* a_szXURFileName)
{
	TStringManager::String8Copy(s_AssetLoadQueue, "data/xui/", BUFFSIZE);
	const int iStrLen = TStringManager::String8Length(s_AssetLoadQueue);
	const int iExcessLen = BUFFSIZE - iStrLen;
	const char* locale = ALocaleManager::Instance().GetCurLocaleCode();
	TStringManager::String8Copy(s_AssetLoadQueue + iStrLen, locale, iExcessLen);
	const int iLen = TStringManager::String8Length(s_AssetLoadQueue);
	TASSERT(iLen < BUFFSIZE);
	s_AssetLoadQueue[iLen] = '/';
	TStringManager::String8Copy(s_AssetLoadQueue + iLen + 1, a_szTRBFileName, BUFFSIZE - iLen + 1);
	TXUI::GetSingleton()->SetSkin2(s_AssetLoadQueue, a_szXURFileName);
}

TMemoryHeap* AXUIState::SetFontMemBlock(TMemoryHeap* a_pMemHeap)
{
	TMemoryHeap* oldMemBlock = s_pFontTRBMemoryBlock;
	s_pFontTRBMemoryBlock = a_pMemHeap;
	return oldMemBlock;
}

TBOOL AXUIState::InitFont(const char* a_szTRBFileName)
{
	s_FontTRB.SetMemoryFunctions(FontTRBAllocator, FontTRBDeallocator, TNULL);
	TMemoryHeap* oldHeap = SetFontMemBlock(TMemory::GetGlobalHeap());
	if (s_bFontLoaded) {
		TAssetInit::DeinitAssets(s_FontTRB);
		s_FontTRB.Close();
		s_bFontLoaded = TFALSE;
	}
	TTRB::ERROR error = s_FontTRB.Load(a_szTRBFileName);
	TASSERT(error == TTRB::ERROR_OK);
	TAssetInit::InitAssets(s_FontTRB, TTRUE, TFALSE);
	s_bFontLoaded = TRUE;
	SetFontMemBlock(oldHeap);
	return TTRUE;
}
