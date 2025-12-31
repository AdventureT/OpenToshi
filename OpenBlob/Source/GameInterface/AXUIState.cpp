#include "pch.h"
#include "AXUIState.h"
#include "Toshi/Xui/TXUI.h"
#include "Locale/ALocaleManager.h"
#include "Toshi/Render/TAssetInit.h"
#include "AAssetStreaming.h"
#include "AAssetStreamingJobs.h"

TOSHI_NAMESPACE_USING

TTRB::t_MemoryFuncAlloc AXUIState::FontTRBAllocator = [](TTRB::AllocType alloctype, size_t size, TSHORT unk1, size_t unk2, void* userData) {
	return s_pFontTRBMemoryBlock->Memalign(unk1, size);
};

TTRB::t_MemoryFuncDealloc AXUIState::FontTRBDeallocator = [](TTRB::AllocType alloctype, void* ptr, TSHORT unk1, size_t unk2, void* userData) {
	return TFree(ptr);
};

// $deBlob: FUNCTION 005d33f0
void AXUIState::SetSkin1(const TCHAR* a_szTRBFileName, const TCHAR* a_szXURFileName)
{
	TStringManager::String8Copy(s_AssetLoadQueue, "data/xui/", BUFFSIZE);
	const TINT   iStrLen    = TStringManager::String8Length(s_AssetLoadQueue);
	const TINT   iExcessLen = BUFFSIZE - iStrLen;
	const TCHAR* locale     = ALocaleManager::Instance().GetCurLocaleCode();
	TStringManager::String8Copy(s_AssetLoadQueue + iStrLen, locale, iExcessLen);
	const TINT iLen = TStringManager::String8Length(s_AssetLoadQueue);
	TASSERT(iLen < BUFFSIZE);
	s_AssetLoadQueue[iLen] = '/';
	TStringManager::String8Copy(s_AssetLoadQueue + iLen + 1, a_szTRBFileName, BUFFSIZE - iLen + 1);
	TXUI::GetSingleton()->SetSkin1(s_AssetLoadQueue, a_szXURFileName);
}

// $deBlob: FUNCTION 005d34b0
void AXUIState::SetSkin2(const TCHAR* a_szTRBFileName, const TCHAR* a_szXURFileName)
{
	TStringManager::String8Copy(s_AssetLoadQueue, "data/xui/", BUFFSIZE);
	const TINT   iStrLen    = TStringManager::String8Length(s_AssetLoadQueue);
	const TINT   iExcessLen = BUFFSIZE - iStrLen;
	const TCHAR* locale     = ALocaleManager::Instance().GetCurLocaleCode();
	TStringManager::String8Copy(s_AssetLoadQueue + iStrLen, locale, iExcessLen);
	const TINT iLen = TStringManager::String8Length(s_AssetLoadQueue);
	TASSERT(iLen < BUFFSIZE);
	s_AssetLoadQueue[iLen] = '/';
	TStringManager::String8Copy(s_AssetLoadQueue + iLen + 1, a_szTRBFileName, BUFFSIZE - iLen + 1);
	TXUI::GetSingleton()->SetSkin2(s_AssetLoadQueue, a_szXURFileName);
}

// $deBlob: FUNCTION 005e5e20
TMemoryHeap* AXUIState::SetFontMemBlock(TMemoryHeap* a_pMemHeap)
{
	TMemoryHeap* oldMemBlock = s_pFontTRBMemoryBlock;
	s_pFontTRBMemoryBlock    = a_pMemHeap;
	return oldMemBlock;
}

// $deBlob: FUNCTION 005d3570
TBOOL AXUIState::InitFont(const TCHAR* a_szTRBFileName)
{
	s_FontTRB.SetMemoryFunctions(FontTRBAllocator, FontTRBDeallocator, TNULL);
	TMemoryHeap* oldHeap = SetFontMemBlock(TMemory::GetGlobalHeap());
	if (s_bFontLoaded)
	{
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

void AXUIState::SetFrontendCarousel(const TCHAR* a_szTRBFileName, const TCHAR* a_szXURFileName)
{
	TStringManager::String8Copy(s_AssetLoadQueue, "data/xui/", BUFFSIZE);
	const TINT   iStrLen    = TStringManager::String8Length(s_AssetLoadQueue);
	const TINT   iExcessLen = BUFFSIZE - iStrLen;
	const TCHAR* locale     = ALocaleManager::Instance().GetCurLocaleCode();
	TStringManager::String8Copy(s_AssetLoadQueue + iStrLen, locale, iExcessLen);
	const TINT iLen = TStringManager::String8Length(s_AssetLoadQueue);
	TASSERT(iLen < BUFFSIZE);
	s_AssetLoadQueue[iLen] = '/';
	TStringManager::String8Copy(s_AssetLoadQueue + iLen + 1, a_szTRBFileName, BUFFSIZE - iLen + 1);
	AAssetStreaming* pAssetStreaming = AAssetStreaming::GetSingleton();
	ATRBLoaderJob *job = static_cast<ATRBLoaderJob*>(pAssetStreaming->CancelAllWaitingTerrainJobs());
	job->Init(&s_XUITRB, s_AssetLoadQueue);
	TIMPLEMENT_D("Some kind of callback setting");
	pAssetStreaming->AddMainThreadJob2(job);
}

// $deBlob: FUNCTION 005d3620
TBOOL AXUIState::SetSkinWithFont(const TCHAR* a_szTRBFileName, bool m_bLoad)
{
	static bool g_bFontLoaded = TFALSE;
	static TTRB g_oFontTrb;
	if (g_bFontLoaded)
	{
		TAssetInit::DeinitAssets(g_oFontTrb);
		g_oFontTrb.Close();
		g_bFontLoaded = TFALSE;
	}
	TStringManager::String8Copy(s_AssetLoadQueue, "data/xui/", BUFFSIZE);
	const TINT   iStrLen    = TStringManager::String8Length(s_AssetLoadQueue);
	const TINT   iExcessLen = BUFFSIZE - iStrLen;
	const TCHAR* locale     = ALocaleManager::Instance().GetCurLocaleCode();
	TStringManager::String8Copy(s_AssetLoadQueue + iStrLen, locale, iExcessLen);
	const TINT iLen = TStringManager::String8Length(s_AssetLoadQueue);
	TASSERT(iLen < BUFFSIZE);
	s_AssetLoadQueue[iLen] = '/';
	TStringManager::String8Copy(s_AssetLoadQueue + iLen + 1, a_szTRBFileName, BUFFSIZE - iLen + 1);

	g_oFontTrb.SetMemoryFunctions(FontTRBAllocator, FontTRBDeallocator, TNULL);
	static TMemoryHeap* g_pFontMem;
	g_pFontMem = SetFontMemBlock(TMemory::GetGlobalHeap());
	if (m_bLoad)
	{
		AAssetStreaming* pAssetStreaming = AAssetStreaming::GetSingleton();
		ATRBLoaderJob*   job             = static_cast<ATRBLoaderJob*>(pAssetStreaming->CancelAllWaitingTerrainJobs());
		job->Init(&g_oFontTrb, s_AssetLoadQueue);
		TIMPLEMENT_D("Some kind of callback setting");
		pAssetStreaming->AddMainThreadJob2(job);
		return TTRUE;
	}
	TTRB::ERROR error = g_oFontTrb.Load(s_AssetLoadQueue);
	TASSERT(error == TTRB::ERROR_OK);
	TAssetInit::InitAssets(g_oFontTrb, TTRUE, TFALSE);

	g_bFontLoaded = TRUE;
	SetFontMemBlock(g_pFontMem);
	g_pFontMem = TNULL;
	return TTRUE;
}

TBOOL AXUIState::SetCommonSkin(const TCHAR* a_szTRBFileName)
{
	TStringManager::String8Copy(s_AssetLoadQueue, "data/xui/", BUFFSIZE);
	const TINT   iStrLen    = TStringManager::String8Length(s_AssetLoadQueue);
	const TINT   iExcessLen = BUFFSIZE - iStrLen;
	const TCHAR* locale     = ALocaleManager::Instance().GetCurLocaleCode();
	TStringManager::String8Copy(s_AssetLoadQueue + iStrLen, locale, iExcessLen);
	const TINT iLen = TStringManager::String8Length(s_AssetLoadQueue);
	TASSERT(iLen < BUFFSIZE);
	s_AssetLoadQueue[iLen] = '/';
	TStringManager::String8Copy(s_AssetLoadQueue + iLen + 1, a_szTRBFileName, BUFFSIZE - iLen + 1);

	static bool g_bFontLoaded = TFALSE;
	static TTRB g_oFontTrb;
	g_oFontTrb.SetMemoryFunctions(FontTRBAllocator, FontTRBDeallocator, TNULL);
	TMemoryHeap *pFontMem = SetFontMemBlock(TMemory::GetGlobalHeap());
	if (g_bFontLoaded)
	{
		TAssetInit::DeinitAssets(g_oFontTrb);
		g_oFontTrb.Close();
		g_bFontLoaded = TFALSE;
	}
	TTRB::ERROR error = g_oFontTrb.Load(s_AssetLoadQueue);
	TASSERT(error == TTRB::ERROR_OK);
	TAssetInit::InitAssets(g_oFontTrb, TTRUE, TFALSE);

	g_bFontLoaded = TRUE;
	SetFontMemBlock(pFontMem);
	return TTRUE;
}
