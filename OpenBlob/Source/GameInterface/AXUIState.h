#pragma once

#include "GameInterface/AGameState.h"

constexpr uint32_t BUFFSIZE = 128;

class AXUIState :
	public Toshi::TGenericClassDerived<AXUIState, AGameState, "ATestState", TMAKEVERSION(1, 0), TTRUE>
{

	inline static char s_AssetLoadQueue[BUFFSIZE] = {};
public:
	static void SetSkin1(const char* a_szTRBFileName, const char* a_szXURFileName);
	static void SetSkin2(const char* a_szTRBFileName, const char* a_szXURFileName);
};

