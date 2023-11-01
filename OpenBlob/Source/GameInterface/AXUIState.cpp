#include "pch.h"
#include "AXUIState.h"
#include "Toshi/Xui/TXUI.h"
#include "Locale/ALocaleManager.h"


TOSHI_NAMESPACE_USING

AXUIState::AXUIState(Asset& a_rAssets)
{
	TXUI* txui = TXUI::GetSingleton();
	if (a_rAssets.m_iUnk5 == 0) {
		TASSERT(TNULL != a_rAssets.m_sResourceName);
		TXUIResource* res = txui->FindResource(a_rAssets.m_sResourceName);
		if (a_rAssets.m_sAssetName == TNULL) {
			txui->CreateScene(res, 0);
		} else {

		}
	}
}

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

