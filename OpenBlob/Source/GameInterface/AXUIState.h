#pragma once

#include "GameInterface/AGameState.h"

constexpr uint32_t BUFFSIZE = 128;

class AXUIState :
	public Toshi::TGenericClassDerived<AXUIState, AGameState, "AXUIState", TMAKEVERSION(1, 0), TTRUE>
{

	struct Asset
	{
		int m_iUnk;                  // 0x00
		float m_fUnk2;               // 0x04
		int m_iUnk3;                 // 0x08
		int m_iUnk4;                 // 0x0A
		const char* m_sResourceName; // 0x10
		const wchar_t* m_sAssetName; // 0x14
		int m_iUnk5;                 // 0x24
	};

	inline static char s_AssetLoadQueue[BUFFSIZE] = {};
public:

	AXUIState() = default;
	AXUIState(Asset & a_rAssets);

	static void SetSkin1(const char* a_szTRBFileName, const char* a_szXURFileName);
	static void SetSkin2(const char* a_szTRBFileName, const char* a_szXURFileName);
};

